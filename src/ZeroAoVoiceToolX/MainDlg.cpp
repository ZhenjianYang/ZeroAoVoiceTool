#include "stdafx.h"
#include "resource.h"
#include "MainDlg.h"

#include <ZaConst.h>
#include <ZaConfig.h>
#include <ZaLog.h>
#include <ZaRemote.h>
#include <ZaSound.h>
#include <ZaScenaAnalyzer.h>
#include <ZaVoiceTable.h>
#include <ZaVoicePlayer.h>

LRESULT CMainDlg::OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	// center the dialog on the screen
	CenterWindow();

	// set icons
	HICON hIcon = AtlLoadIconImage(IDR_MAINFRAME, LR_DEFAULTCOLOR, ::GetSystemMetrics(SM_CXICON), ::GetSystemMetrics(SM_CYICON));
	SetIcon(hIcon, TRUE);
	HICON hIconSmall = AtlLoadIconImage(IDR_MAINFRAME, LR_DEFAULTCOLOR, ::GetSystemMetrics(SM_CXSMICON), ::GetSystemMetrics(SM_CYSMICON));
	SetIcon(hIconSmall, FALSE);

	// register object for message filtering and idle updates
	CMessageLoop* pLoop = _Module.GetMessageLoop();
	ATLASSERT(pLoop != NULL);
	pLoop->AddMessageFilter(this);
	pLoop->AddIdleHandler(this);

	UIAddChildWindowContainer(m_hWnd);

	m_status = CMainDlg::Idle;
	m_button_start = this->GetDlgItem(IDC_BUTTON_START);

	s_hWnd_main = this->m_hWnd;
	s_th_monitor = CreateThread(NULL, 0, CMainDlg::Thread_Monitor, NULL, 0, NULL);
	s_th_initplayer = NULL;

	SetWorkPath();
	ZaConfigLoad(DFT_CONFIG_FILE);

	int logparam = 0;
	if (!g_zaConfig->General.OpenDebugLog) {
		logparam = ZALOG_OUT_STDOUT | ZALOG_TYPE_INFO | ZALOG_TYPE_INFO | ZALOG_PARAM_NOPREINFO;
	}
	else {
		logparam = ZALOG_OUT_STDLOG | ZALOG_TYPE_ALL;
	}
	if (g_zaConfig->General.UseLogFile) {
		logparam |= ZALOG_OUT_FILE;
		ZALOG_SETLOGFILE(DFT_LOG_FILE);
	}

	if (g_zaConfig->General.OpenDebugLog) {
		AllocConsole();
		freopen("CONOUT$", "a+", stderr);
	}

	ZALOG_OPEN_WITHPARAM(logparam);

	ZALOG("Zero Ao Voice Tool %s", ZA_VERSION);

	return TRUE;
}

LRESULT CMainDlg::OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	// unregister message filtering and idle updates
	CMessageLoop* pLoop = _Module.GetMessageLoop();
	ATLASSERT(pLoop != NULL);
	pLoop->RemoveMessageFilter(this);
	pLoop->RemoveIdleHandler(this);

	return 0;
}

LRESULT CMainDlg::OnClose(UINT Msg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	this->CloseDialog(0);
	return 0;
}

void CMainDlg::CloseDialog(int nVal)
{
	CloseHandle(s_th_monitor);
	DestroyWindow();
	::PostQuitMessage(nVal);
}

LRESULT CMainDlg::OnBnClickedButtonStart(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	// TODO: 在此添加控件通知处理程序代码
	switch (m_status)
	{
	case CMainDlg::Idle:
		m_button_start.SetWindowTextA("Stop");
		m_status = CMainDlg::WaitingGameStart;
		AddMonitorFunc(CMainDlg::Monitor_GameStart);
		ZALOG("等待游戏运行...");
		break;
	case CMainDlg::WaitingGameStart:
		m_button_start.SetWindowTextA("Start");
		RemoveMonitorFunc(CMainDlg::Monitor_GameStart);
		m_status = CMainDlg::Idle;
		break;
	case CMainDlg::InitVoicePlayer:
		m_button_start.SetWindowTextA("Start");
		m_status = CMainDlg::Idle;
		break;
	case CMainDlg::Running:
		m_button_start.SetWindowTextA("Start");
		m_status = CMainDlg::Idle;
		break;
	default:
		break;
	}
	return 0;
}

LRESULT CMainDlg::OnError(UINT Msg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	return 0;
}
LRESULT CMainDlg::OnStop(UINT Msg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	return 0;
}
LRESULT CMainDlg::OnGameFound(UINT Msg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	RemoveMonitorFunc(CMainDlg::Monitor_GameStart);
	m_gameID = (int)wParam;
	ZALOG("游戏已启动,游戏标题为:%s",
		m_gameID == GAMEID_AO ? A_DFT_WIN_TITLE : Z_DFT_WIN_TITLE);

	if (ZaRemoteInit(m_gameID, (int)m_hWnd, WM_MSG_REMOTEBASE)) {
		::SendMessage(m_hWnd, WM_MSG_ERROR, InitRemoteFailed, 0);
	}
	
	ZaConfigSetActive(m_gameID);
	ZALOG("就绪");

	if (s_th_initplayer) CloseHandle(s_th_initplayer);
	s_th_initplayer = CreateThread(NULL, 0, CMainDlg::Thread_InitVoicePlayer, NULL, 0, NULL);
	m_status = InitVoicePlayer;

	return 0;
}
LRESULT CMainDlg::OnGameExit(UINT Msg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	RemoveMonitorFunc(CMainDlg::Monitor_GameExit);
	ZALOG("游戏已退出！");

	ZaVoicePlayerEnd();
	ZaRemoteEnd();

	m_status = Idle;
	return 0;
}
LRESULT CMainDlg::OnPlayEnd(UINT Msg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	ZaPlayWait();
	if (ZaWaitingNum() == 0) ZaSoundSetStopCallBack();
	return 0;
}
LRESULT CMainDlg::OnInitPlayerEnd(UINT Msg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	CloseHandle(s_th_initplayer); s_th_initplayer = NULL;
	ZALOG("已进入语音播放系统");

	ZALOG("语音文件目录为: %s", g_zaConfig->ActiveGame->VoiceDir.c_str());
	for (unsigned i = 1; i <= g_zaConfig->ActiveGame->VoiceExt.size(); ++i) {
		ZALOG("语音文件后缀%d: %s", i, g_zaConfig->ActiveGame->VoiceExt[i - 1].c_str());
	}
	if (m_gameID == GAMEID_AO && g_zaConfig->ActiveGame->DisableOriginalVoice) {
		ZALOG("启用了禁用原有剧情语音的功能");
	}

	AddMonitorFunc(CMainDlg::Monitor_GameExit);
	memset(&m_zadata, 0, sizeof(m_zadata));
	m_status = Running;
	return 0;
}
LRESULT CMainDlg::OnRLoadScena(UINT Msg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	ZALOG_DEBUG("Msg Received, LoadScena, addr:0x%X", wParam);
	if (m_status != Running) return 0;

	m_zadata.aScena = (unsigned)wParam;
	m_zadata.aScena1 = m_zadata.aScena2 = m_zadata.aCurBlock = m_zadata.aCurText = 0;
	m_zadata.flag = 0;
	return 0;
}
LRESULT CMainDlg::OnRLoadScena1(UINT Msg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	ZALOG_DEBUG("Msg Received, LoadScena1, addr:0x%X", wParam);
	if (m_status != Running) return 0;

	if(m_zadata.aScena1) m_zadata.aScena2 = (unsigned)wParam;
	else m_zadata.aScena1 = (unsigned)wParam;
	return 0;
}
LRESULT CMainDlg::OnRLoadBlock(UINT Msg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	ZALOG_DEBUG("Msg Received, LoadBlock, addr:0x%X", wParam);
	if (m_status != Running) return 0;

	if (m_zadata.aScena) {
		m_zadata.aCurBlock = (unsigned)wParam;

		int errc;
		const char* pScenaName;

		if (!m_zadata.flag) {
			int errc = ZaDetected_LoadScena(m_zadata.aScena, pScenaName);
			if (errc) ::SendMessage(m_hWnd, WM_MSG_ERROR, ReadRemoteDataFailedRunning, 0);

			if (m_zadata.aScena1) {
				errc = ZaDetected_LoadScena1(m_zadata.aScena, pScenaName);
				if (errc) ::SendMessage(m_hWnd, WM_MSG_ERROR, ReadRemoteDataFailedRunning, 0);
			}

			if (m_zadata.aScena2) {
				errc = ZaDetected_LoadScena1(m_zadata.aScena, pScenaName);
				if (errc) ::SendMessage(m_hWnd, WM_MSG_ERROR, ReadRemoteDataFailedRunning, 0);
			}

			m_zadata.flag = 1;
		}

		errc = ZaDetected_LoadBlock(m_zadata.aCurBlock, pScenaName);
		if (errc) ::SendMessage(m_hWnd, WM_MSG_ERROR, ReadRemoteDataFailedRunning, 0);
	}
	return 0;
}
LRESULT CMainDlg::OnRShowText(UINT Msg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	ZALOG_DEBUG("Msg Received, ShowText, addr:0x%X", wParam);
	static char voiceFileName[MAX_LENGTH_VOICE_ID * 2 + 1];
	if (m_status != Running) return 0;

	if (m_zadata.aCurBlock) {
		m_zadata.aCurText = (unsigned)wParam;
		int voiceID;
		bool wait;
		int errc = ZaDetected_ShowText(m_zadata.aCurText, voiceID, wait);
		if (errc) ::SendMessage(m_hWnd, WM_MSG_ERROR, ReadRemoteDataFailedRunning, 0);

		if (voiceID != InValidVoiceId) {
			if (ZaSoundStatus() == ZASOUND_STATUS_STOP
				&& ZaWaitingNum() == 0) {
				wait = false;
			}
			if (!wait) {
				ZaClearWait();
				if (ZaPlayVoice(voiceID, voiceFileName)) {
					ZALOG_DEBUG("Playing %s ...", voiceFileName);
				}
				else {
					ZALOG_ERROR("Play %s failed.", voiceFileName);
				}
			}
			else
			{
				ZaAddToWait(voiceID);
				ZaSoundSetStopCallBack(CMainDlg::PlayEndCallBack);
			}
		}
	}

	return 0;
}

void CMainDlg::SetWorkPath() {
	char buff[1024];
	GetModuleFileName(NULL, (LPSTR)buff, sizeof(buff));
	int p = -1;
	for (int i = 0; buff[i] != 0; ++i)
	{
		if (buff[i] == '\\') p = i;
	}
	if (p != -1) {
		buff[p] = 0;
	}
	else {
		buff[0] = '.'; buff[1] = 0;
	}

	SetCurrentDirectory(buff);
}

void CMainDlg::Monitor_GameStart() {
	static const char* tbuf[] = { Z_DFT_WIN_TITLE, A_DFT_WIN_TITLE };

	int index = ZaCheckGameStart(sizeof(tbuf) / sizeof(*tbuf), tbuf);
	if (index < 0) return;

	if (index == 0 && g_zaConfig->General.Mode != MODE_AO
		|| index == 1 && g_zaConfig->General.Mode != MODE_ZERO)
		::SendMessage(s_hWnd_main,
			WM_MSG_GAMEFOUND,
			index == 0 ? GAMEID_ZERO : GAMEID_AO,
			0
		);
}
void CMainDlg::Monitor_GameExit() {
	if (ZaCheckGameEnd()) {
		::SendMessage(s_hWnd_main,
			WM_MSG_GAMEEXIT,
			0,
			0
		);
	}
}

CMainDlg::MonitorFunc CMainDlg::pmfs[3];
CMainDlg::MonitorFunc *pmf_end = CMainDlg::pmfs + sizeof(CMainDlg::pmfs) / sizeof(*CMainDlg::pmfs);

void CMainDlg::ClearAllMonitorFunc() {
	for (MonitorFunc *p = pmfs; p < pmf_end; ++p)
		*p = NULL;
}
bool CMainDlg::AddMonitorFunc(MonitorFunc func) {
	int count = 0;
	for (MonitorFunc *p = pmfs; p < pmf_end; ++p)
		if (*p == NULL) {
			*p = func; func = NULL;
		}
		else
			++count;
	if (count == 0) {
		ResumeThread(s_th_monitor); 
	}
	return pmf_end - pmfs == count;
}
void CMainDlg::RemoveMonitorFunc(MonitorFunc func) {
	for (MonitorFunc *p = pmfs; p < pmf_end; ++p)
		if (*p == func) {
			*p = NULL;
		}
}
DWORD WINAPI CMainDlg::Thread_Monitor(LPVOID lpParmeter) {
	for (;;)
	{
		int count = 0;
		MonitorFunc func;
		for (MonitorFunc *p = pmfs; p < pmf_end; ++p)
			if (func = *p) { func(); ++count; }

		if (count == 0) SuspendThread(s_th_monitor);
		Sleep(1000);
	}
	return 0;
}

DWORD WINAPI CMainDlg::Thread_InitVoicePlayer(LPVOID lpParmeter) {
	int errc = ZaVoicePlayerInit();
	if (errc) {
		::SendMessage(s_hWnd_main,
			WM_MSG_ERROR,
			CMainDlg::InitVoicePlayerFailed,
			0
		);
	}
	else {
		::SendMessage(s_hWnd_main,
			WM_MSG_INITPLAYEREND,
			0,
			0
		);
	}
	return errc;
}

int CMainDlg::PlayEndCallBack(void *)
{
	::SendMessage(s_hWnd_main, WM_MSG_PLAYEND, 0, 0);
	return 0;
}

HANDLE CMainDlg::s_th_initplayer;
HANDLE CMainDlg::s_th_monitor;
HWND CMainDlg::s_hWnd_main;

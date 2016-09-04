#include "stdafx.h"
#include "resource.h"
#include "MainDlg.h"

#include <ZaConst.h>
#include <ZaConfig.h>
#include <ZaLog.h>
#include <ZaRemote.h>
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
	return 0;
}
LRESULT CMainDlg::OnGameExit(UINT Msg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	return 0;
}
LRESULT CMainDlg::OnPlayEnd(UINT Msg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	return 0;
}
LRESULT CMainDlg::OnInitPlayerEnd(UINT Msg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	return 0;
}
LRESULT CMainDlg::OnRLoadScena(UINT Msg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	return 0;
}
LRESULT CMainDlg::OnRLoadScena1(UINT Msg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	return 0;
}
LRESULT CMainDlg::OnRLoadBlock(UINT Msg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	return 0;
}
LRESULT CMainDlg::OnRShowText(UINT Msg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
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
			index == 0 ? GAMEID_ZERO : GAMEID_ZERO,
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

HANDLE CMainDlg::s_th_initplayer;
HANDLE CMainDlg::s_th_monitor;
HWND CMainDlg::s_hWnd_main;

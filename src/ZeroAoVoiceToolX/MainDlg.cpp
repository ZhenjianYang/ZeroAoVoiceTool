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

	m_status = Status::Idle;

	m_button_start = this->GetDlgItem(IDC_BUTTON_START);
	m_check_autostart = this->GetDlgItem(IDC_CHECK_AUTOSTART);
	
	m_group_zero = this->GetDlgItem(IDC_GROUP_ZERO);
	m_static_zvp = this->GetDlgItem(IDC_STATIC_ZVP);
	m_edit_zvp = this->GetDlgItem(IDC_EDIT_ZVP);
	m_button_zvb = this->GetDlgItem(IDC_BUTTON_ZVB);
	m_static_zve = this->GetDlgItem(IDC_STATIC_ZVE);
	m_edit_zve = this->GetDlgItem(IDC_EDIT_ZVE);
	m_static_zvtp = this->GetDlgItem(IDC_STATIC_ZVTP);
	m_edit_zvtp = this->GetDlgItem(IDC_EDIT_ZVTP);
	m_button_zvtb = this->GetDlgItem(IDC_BUTTON_ZVTB);
	m_static_zvte = this->GetDlgItem(IDC_STATIC_ZVTE);
	m_edit_zvte = this->GetDlgItem(IDC_EDIT_ZVTE);
	m_static_zv = this->GetDlgItem(IDC_STATIC_ZV);
	m_slider_zv = this->GetDlgItem(IDC_SLIDER_ZV);
	m_spin_zv = this->GetDlgItem(IDC_SPIN_ZV);
	m_edit_zv = this->GetDlgItem(IDC_EDIT_ZV);
	m_check_zdov = this->GetDlgItem(IDC_CHECK_ZDOV);

	m_group_ao = this->GetDlgItem(IDC_GROUP_AO);
	m_static_avp = this->GetDlgItem(IDC_STATIC_AVP);
	m_edit_avp = this->GetDlgItem(IDC_EDIT_AVP);
	m_button_avb = this->GetDlgItem(IDC_BUTTON_AVB);
	m_static_ave = this->GetDlgItem(IDC_STATIC_AVE);
	m_edit_ave = this->GetDlgItem(IDC_EDIT_AVE);
	m_static_avtp = this->GetDlgItem(IDC_STATIC_AVTP);
	m_edit_avtp = this->GetDlgItem(IDC_EDIT_AVTP);
	m_button_avtb = this->GetDlgItem(IDC_BUTTON_AVTB);
	m_static_avte = this->GetDlgItem(IDC_STATIC_AVTE);
	m_edit_avte = this->GetDlgItem(IDC_EDIT_AVTE);
	m_static_av = this->GetDlgItem(IDC_STATIC_AV);
	m_slider_av = this->GetDlgItem(IDC_SLIDER_AV);
	m_spin_av = this->GetDlgItem(IDC_SPIN_AV);
	m_edit_av = this->GetDlgItem(IDC_EDIT_AV);
	m_check_adov = this->GetDlgItem(IDC_CHECK_ADOV);

	m_slider_zv.SetRange(0, VOLUME_MAX);
	m_edit_zve.SetReadOnly();
	m_edit_zvte.SetReadOnly();
	m_edit_zv.SetReadOnly();

	m_slider_av.SetRange(0, VOLUME_MAX);
	m_edit_ave.SetReadOnly();
	m_edit_avte.SetReadOnly();
	m_edit_av.SetReadOnly();

	s_hWnd_main = this->m_hWnd;
	s_sign_monitorstop = 0;
	s_event_monitor = ::CreateEvent(NULL, FALSE, FALSE, "Monitor Event");
	s_th_monitor = CreateThread(NULL, 0, CMainDlg::Thread_Monitor, &s_sign_monitorstop, 0, NULL);
	s_th_monitor = NULL;
	s_th_initplayer = NULL;

	SetWorkPath();
	ZaConfigLoad(DFT_CONFIG_FILE);

	LoadConfig();

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
		::DeleteMenu(::GetSystemMenu(GetConsoleWindow(), FALSE), SC_CLOSE, MF_BYCOMMAND);
		freopen("CONOUT$", "a+", stderr);
	}

	ZALOG_OPEN_WITHPARAM(logparam);

	ZALOG_DEBUG("Zero Ao Voice Tool %s", ZA_VERSION);

	ZaSoundInit();
	ZALOG_DEBUG("音频系统已启动");

	if (g_zaConfig->General.AutoStart) {
		::SendMessage(this->m_hWnd, WM_COMMAND, IDC_BUTTON_START, BN_CLICKED);
	}

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
	switch (m_status)
	{
	case Status::Idle:
		break;
	case Status::WaitingGameStart:
		break;
	case Status::InitVoicePlayer:
		if (MessageBox("已启动语音工具，终止并退出？", "询问", MB_ICONQUESTION | MB_YESNO)
			!= IDYES) {
			return 0;
		}
		s_sign_initplayerstop = 1;
		m_status = Status::Exitting;
		return 0;
	case Status::StoppingInitVoicePlayer:
		m_status = Status::Exitting;
		return 0;
	case Status::Running:
		if (MessageBox("已启动语音工具，终止并退出？", "询问", MB_ICONQUESTION | MB_YESNO)
			!= IDYES) {
			return 0;
		}
		ZaVoicePlayerEnd();
		ZALOG_DEBUG("已终止语音系统初试化");
		ZaRemoteEnd();
		ZALOG_DEBUG("已关闭远程进程句柄");
		break;
	case Status::Exitting:
		return 0;
		break;
	default:
		break;
	}

	ZaSoundEnd();
	ZALOG_DEBUG("已关闭音频系统");
	ZALOG_DEBUG("Zero Ao Voice Tool End");

	s_sign_monitorstop = 1;
	::SetEvent(s_event_monitor);
	if (g_zaConfig->General.OpenDebugLog)
		FreeConsole();
	TerminateThread(s_th_monitor, 0);
	CloseHandle(s_th_monitor); s_th_monitor = NULL;

	SaveConfig();

	this->CloseDialog(0);
	return 0;
}

void CMainDlg::CloseDialog(int nVal)
{
	DestroyWindow();
	::PostQuitMessage(nVal);
}

LRESULT CMainDlg::OnBnClickedButtonStart(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& bHandled)
{
	// TODO: 在此添加控件通知处理程序代码
	switch (m_status)
	{
	case Status::Idle:
		SaveConfig();
		m_button_start.SetWindowTextA("Stop");
		m_status = Status::WaitingGameStart;
		AddMonitorFunc(CMainDlg::Monitor_GameStart);
		ZALOG_DEBUG("等待游戏运行...");
		break;
	case Status::StoppingInitVoicePlayer:
		break;
	case Status::WaitingGameStart:
	case Status::InitVoicePlayer:
	case Status::Running:
	default:
		OnStop(WM_MSG_STOP, (WPARAM)m_status, 0, bHandled);
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
	m_status = Status::Idle;
	ClearAllMonitorFunc();

	CMainDlg::Status status = (CMainDlg::Status)wParam;
	switch (status)
	{
	case Status::WaitingGameStart:
		ZALOG_DEBUG("终止等待游戏运行");
		m_button_start.SetWindowTextA("Start");
		break;
	case Status::InitVoicePlayer:
		s_sign_initplayerstop = 1;
		m_status = Status::StoppingInitVoicePlayer;
		m_button_start.EnableWindow(FALSE);
		ZALOG_DEBUG("终止语音系统初试化...");
		break;
	case Status::Running:
		ZaVoicePlayerEnd();
		ZALOG_DEBUG("已退出语音播放系统");
		ZaRemoteEnd();
		ZALOG_DEBUG("已关闭远程进程句柄");
		m_button_start.SetWindowTextA("Start");
		break;
	case Status::Idle:
	case Status::StoppingInitVoicePlayer:
	default:
		throw "错误：此时不应有Idle或StoppingInitVoicePlayer";
	}

	return 0;
}
LRESULT CMainDlg::OnGameFound(UINT Msg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	RemoveMonitorFunc(CMainDlg::Monitor_GameStart);
	if (m_status != Status::WaitingGameStart) return 0;

	m_gameID = (int)wParam;
	ZALOG_DEBUG("游戏已启动,游戏标题为： %s",
		m_gameID == GAMEID_AO ? A_DFT_WIN_TITLE : Z_DFT_WIN_TITLE);

	if (ZaRemoteInit(m_gameID, (int)m_hWnd, WM_MSG_REMOTEBASE)) {
		::SendMessage(m_hWnd, WM_MSG_ERROR, (WPARAM)ErrorType::InitRemoteFailed, 0);
	}
	
	ZaConfigSetActive(m_gameID);
	ZaSoundSetVolumn(g_zaConfig->ActiveGame->Volume);
	ZALOG_DEBUG("就绪");

	s_sign_initplayerstop = 0;
	s_th_initplayer = CreateThread(NULL, 0, CMainDlg::Thread_InitVoicePlayer, &s_sign_initplayerstop, 0, NULL);
	m_status = Status::InitVoicePlayer;

	return 0;
}
LRESULT CMainDlg::OnGameExit(UINT Msg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	RemoveMonitorFunc(CMainDlg::Monitor_GameExit);
	ZALOG_DEBUG("游戏已退出！");

	ZaVoicePlayerEnd();
	ZaRemoteEnd();

	m_button_start.SetWindowTextA("Start");
	m_status = Status::Idle;
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
	if (m_status == Status::StoppingInitVoicePlayer
		|| m_status == Status::Exitting) {
		::SendMessage(m_hWnd, WM_MSG_INITPLAYERSTOPPED, 0, 0);
		return 0;
	}
	else if (m_status != Status::InitVoicePlayer) return 0;

	ZALOG_DEBUG("已进入语音播放系统");

	ZALOG_DEBUG("语音文件目录为: %s", g_zaConfig->ActiveGame->VoiceDir.c_str());
	for (unsigned i = 1; i <= g_zaConfig->ActiveGame->VoiceExt.size(); ++i) {
		ZALOG_DEBUG("语音文件后缀%d: %s", i, g_zaConfig->ActiveGame->VoiceExt[i - 1].c_str());
	}
	if (m_gameID == GAMEID_AO && g_zaConfig->ActiveGame->DisableOriginalVoice) {
		ZALOG_DEBUG("启用了禁用原有剧情语音的功能");
	}

	AddMonitorFunc(CMainDlg::Monitor_GameExit);
	memset(&m_zadata, 0, sizeof(m_zadata));
	m_status = Status::Running;
	return 0;
}
LRESULT CMainDlg::OnInitPlayerStoped(UINT Msg, WPARAM wParam, LPARAM lParam, BOOL & bHandled)
{
	ZaVoicePlayerEnd();
	ZALOG_DEBUG("已终止语音系统初试化");
	ZaRemoteEnd();
	ZALOG_DEBUG("已关闭远程进程句柄");

	m_button_start.SetWindowTextA("Start");
	m_button_start.EnableWindow(TRUE);
	if (m_status == Status::Exitting) {
		m_status = Status::Idle;
		::SendMessage(m_hWnd, WM_CLOSE, 0, 0);
	}
	else {
		m_status = Status::Idle;
	}
	return 0;
}
LRESULT CMainDlg::OnRLoadScena(UINT Msg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	ZALOG_DEBUG("Msg Received, LoadScena, addr:0x%X", wParam);
	if (m_status != Status::Running) return 0;

	m_zadata.aScena = (unsigned)wParam;
	m_zadata.aScena1 = m_zadata.aScena2 = m_zadata.aCurBlock = m_zadata.aCurText = 0;
	m_zadata.flag = 0;
	return 0;
}
LRESULT CMainDlg::OnRLoadScena1(UINT Msg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	ZALOG_DEBUG("Msg Received, LoadScena1, addr:0x%X", wParam);
	if (m_status != Status::Running) return 0;

	if(m_zadata.aScena1) m_zadata.aScena2 = (unsigned)wParam;
	else m_zadata.aScena1 = (unsigned)wParam;
	return 0;
}
LRESULT CMainDlg::OnRLoadBlock(UINT Msg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	ZALOG_DEBUG("Msg Received, LoadBlock, addr:0x%X", wParam);
	if (m_status != Status::Running) return 0;

	if (m_zadata.aScena) {
		m_zadata.aCurBlock = (unsigned)wParam;

		int errc;
		const char* pScenaName;

		if (!m_zadata.flag) {
			int errc = ZaDetected_LoadScena(m_zadata.aScena, pScenaName);
			if (errc) ::SendMessage(m_hWnd, WM_MSG_ERROR, (WPARAM)ErrorType::ReadRemoteDataFailedRunning, 0);

			if (m_zadata.aScena1) {
				errc = ZaDetected_LoadScena1(m_zadata.aScena1, pScenaName);
				if (errc) ::SendMessage(m_hWnd, WM_MSG_ERROR, (WPARAM)ErrorType::ReadRemoteDataFailedRunning, 0);
			}

			if (m_zadata.aScena2) {
				errc = ZaDetected_LoadScena1(m_zadata.aScena2, pScenaName);
				if (errc) ::SendMessage(m_hWnd, WM_MSG_ERROR, (WPARAM)ErrorType::ReadRemoteDataFailedRunning, 0);
			}

			m_zadata.flag = 1;
		}

		errc = ZaDetected_LoadBlock(m_zadata.aCurBlock, pScenaName);
		if (errc) ::SendMessage(m_hWnd, WM_MSG_ERROR, (WPARAM)ErrorType::ReadRemoteDataFailedRunning, 0);
	}
	return 0;
}
LRESULT CMainDlg::OnRShowText(UINT Msg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	ZALOG_DEBUG("Msg Received, ShowText, addr:0x%X", wParam);
	static char voiceFileName[MAX_LENGTH_VOICE_ID * 2 + 1];
	if (m_status != Status::Running) return 0;

	if (m_zadata.aCurBlock) {
		m_zadata.aCurText = (unsigned)wParam;
		int voiceID;
		bool wait;
		int errc = ZaDetected_ShowText(m_zadata.aCurText, voiceID, wait);
		if (errc) ::SendMessage(m_hWnd, WM_MSG_ERROR, (WPARAM)ErrorType::ReadRemoteDataFailedRunning, 0);

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

LRESULT CMainDlg::OnNMCustomdrawSliderV(int idCtrl, LPNMHDR pNMHDR, BOOL& /*bHandled*/)
{
	LPNMCUSTOMDRAW pNMCD = reinterpret_cast<LPNMCUSTOMDRAW>(pNMHDR);
	// TODO: 在此添加控件通知处理程序代码
	CTrackBarCtrl slider = NULL;
	CEdit edit = NULL;
	
	switch (idCtrl)
	{
	case IDC_SLIDER_ZV:
		slider = m_slider_zv;
		edit = m_edit_zv;
		break;
	case IDC_SLIDER_AV:
		slider = m_slider_av;
		edit = m_edit_av;
		break;
	default:
		return 0;
		break;
	}

	CString str;
	str.Format("%d", slider.GetPos());
	edit.SetWindowTextA(str);

	return 0;
}
LRESULT CMainDlg::OnDeltaposSpinV(int idCtrl, LPNMHDR pNMHDR, BOOL& /*bHandled*/)
{
	LPNMUPDOWN pNMUpDown = reinterpret_cast<LPNMUPDOWN>(pNMHDR);
	// TODO: 在此添加控件通知处理程序代码
	CTrackBarCtrl slider = NULL;

	switch (idCtrl)
	{
	case IDC_SPIN_ZV:
		slider = m_slider_zv;
		break;
	case IDC_SPIN_AV:
		slider = m_slider_av;
		break;
	default:
		return 0;
		break;
	}

	int pos = slider.GetPos();
	if (pNMUpDown->iDelta == 1 && pos > slider.GetRangeMin())
	{
		--pos;
	}
	else if (pNMUpDown->iDelta == -1 && pos < slider.GetRangeMax())
	{
		++pos;
	}
	slider.SetPos(pos);

	return 0;
}
LRESULT CMainDlg::OnBnClickedButtonVb(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	// TODO: 在此添加控件通知处理程序代码

	int ids = 0;
	CEdit edit = NULL;

	switch (wID)
	{
	case IDC_BUTTON_ZVB:
		ids = IDS_TEXT_ZVP;
		edit = m_edit_zvp;
		break;
	case IDC_BUTTON_ZVTB:
		ids = IDS_TEXT_ZVTP;
		edit = m_edit_zvtp;
		break;
	case IDC_BUTTON_AVB:
		ids = IDS_TEXT_AVP;
		edit = m_edit_avp;
		break;
	case IDC_BUTTON_AVTB:
		ids = IDS_TEXT_AVTP;
		edit = m_edit_avtp;
		break;
	default:
		return 0;
		break;
	}

	CString title;
	title.LoadStringA(ids);
	CFolderDialog fd(NULL, title);

	if (IDOK == fd.DoModal()) {
		edit.SetWindowTextA(fd.GetFolderPath());
	}

	return 0;
}

void CMainDlg::LoadConfig()
{
	WTL::CString str;

	m_check_autostart.SetCheck(g_zaConfig->General.AutoStart);

	////////////////////////////////////////////////////////////////

	m_edit_zvp.SetWindowTextA(g_zaConfig->Zero.VoiceDir.c_str());
	
	str.Empty();
	for (auto ext : g_zaConfig->Zero.VoiceExt) {
		str += ext.c_str();
		str += " ";
	}
	m_edit_zve.SetWindowTextA(str);
	m_edit_zvtp.SetWindowTextA(g_zaConfig->Zero.VtblDir.c_str());
	m_edit_zvte.SetWindowTextA(g_zaConfig->Zero.VtblExt.c_str());

	m_slider_zv.SetPos(g_zaConfig->Zero.Volume);

	m_check_zdov.SetCheck(g_zaConfig->Zero.DisableOriginalVoice);

	////////////////////////////////////////////////////////////////

	m_edit_avp.SetWindowTextA(g_zaConfig->Ao.VoiceDir.c_str());

	str.Empty();
	for (auto ext : g_zaConfig->Ao.VoiceExt) {
		str += ext.c_str();
		str += " ";
	}
	m_edit_ave.SetWindowTextA(str);
	m_edit_avtp.SetWindowTextA(g_zaConfig->Ao.VtblDir.c_str());
	m_edit_avte.SetWindowTextA(g_zaConfig->Ao.VtblExt.c_str());

	m_slider_av.SetPos(g_zaConfig->Ao.Volume);

	m_check_adov.SetCheck(g_zaConfig->Ao.DisableOriginalVoice);
}

void CMainDlg::SaveConfig()
{
	ZaConfig config;
	ZaConfigSetDefault(&config);
	config.General = g_zaConfig->General;
	config.General.AutoStart = m_check_autostart.GetCheck();

	char buff[1024];
	m_edit_avp.GetWindowTextA(buff, sizeof(buff));
	config.Ao.VoiceDir = buff;
	m_edit_avtp.GetWindowTextA(buff, sizeof(buff));
	config.Ao.VtblDir = buff;
	config.Ao.Volume = m_slider_av.GetPos();
	config.Ao.DisableOriginalVoice = m_check_adov.GetCheck();

	m_edit_zvp.GetWindowTextA(buff, sizeof(buff));
	config.Zero.VoiceDir = buff;
	m_edit_zvtp.GetWindowTextA(buff, sizeof(buff));
	config.Zero.VtblDir = buff;
	config.Zero.Volume = m_slider_zv.GetPos();

	ZaConfigSetConfig(config);
	ZaConfigSave(DFT_CONFIG_FILE);
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
		::SetEvent(s_event_monitor);
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
	while (!*(unsigned*)lpParmeter)
	{
		int count = 0;
		MonitorFunc func;
		for (MonitorFunc *p = pmfs; p < pmf_end; ++p) {
			if (func = *p) {
				func(); 
				++count; 
			}
		}

		if (count == 0) WaitForSingleObject(s_event_monitor, INFINITE);
		else Sleep(1000);
	}
	return 0;
}

DWORD WINAPI CMainDlg::Thread_InitVoicePlayer(LPVOID lpParmeter) {
	int errc = ZaVoicePlayerInit(lpParmeter);
	::SendMessage(s_hWnd_main,
		WM_MSG_INITPLAYEREND,
		errc,
		0);
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
HANDLE CMainDlg::s_event_monitor;
unsigned CMainDlg::s_sign_initplayerstop;
unsigned CMainDlg::s_sign_monitorstop;




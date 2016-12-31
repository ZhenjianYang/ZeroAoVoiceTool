// MainDlg.cpp : implementation of the CMainDlg class
//
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "resource.h"

#include "MainDlg.h"

#include <sstream>

#if !_DEBUG
#define LOG_NOLOG 1
#endif

#include "Log.h"

BOOL CMainDlg::PreTranslateMessage(MSG* pMsg)
{
	return CWindow::IsDialogMessage(pMsg);
}

BOOL CMainDlg::OnIdle()
{
	UIUpdateChildWindows();
	return FALSE;
}

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

	m_check_dov = this->GetDlgItem(IDC_CHECK_DOV);

	m_static_volume = this->GetDlgItem(IDC_STATIC_VOLUME);
	m_slider_volume = this->GetDlgItem(IDC_SLIDER_VOLUME);
	m_edit_volume = this->GetDlgItem(IDC_EDIT_VOLUME);

	m_button_st = this->GetDlgItem(IDC_BUTTON_ST);
	m_button_pre = this->GetDlgItem(IDC_BUTTON_PRE);
	m_button_nxt = this->GetDlgItem(IDC_BUTTON_NEX);
	m_button_lst = this->GetDlgItem(IDC_BUTTON_LST);
	m_button_fnt = this->GetDlgItem(IDC_BUTTON_FNT);
	m_static_cnt = this->GetDlgItem(IDC_STATIC_CNT);
	m_edit_text = this->GetDlgItem(IDC_EDIT_TEXT);

	m_static_info = this->GetDlgItem(IDC_STATIC_INFO);

	SetWorkPath();

	LOG_OPEN;
	LOG("初始化...");

	m_changed_font = false;
	if (!m_config.LoadConfig(CONFIG_FILENAME)) {
		m_config.Reset();
	}
	if (!Za::Main::Init()) {
		MessageBox("读取数据出错！退出！", "错误", MB_OK);
		CloseDialog(-1);
		return TRUE;
	}
	m_pcIn.disableOriVoice = m_config.DisableOriVoice != 0;
	m_pcIn.volume = m_config.Volume;
	Za::Main::SetVoicePlayConfig(m_pcIn);

	LOG("初始化成功！");
	m_status = Status::WaitingGameStart;
	LOG("等待游戏运行...");

	m_gpIn.hMainWindow = (int)m_hWnd;
	m_gpIn.msgId = REMOTE_MSG_ID;
	SetTimer(TIMER_ID, TIMER_TIME);

	if (m_config.DisableOriVoice) {
		LOG("启用了禁用原有语音的功能");
	}
	m_check_dov.SetCheck(m_config.DisableOriVoice);

	m_edit_volume.SetLimitText(3);
	m_slider_volume.SetRangeMax(MAX_VOLUME);
	m_slider_volume.SetRangeMin(0);
	m_slider_volume.SetPos(m_config.Volume);

	HFONT hf = m_edit_text.GetFont();
	GetObject(hf, sizeof(m_lf), &m_lf);
	m_fontcolor = m_config.FontColor;
	//memset(&m_lf, 0, sizeof(m_lf));
	if (m_config.FontName[0]) {
		strcpy(m_lf.lfFaceName, m_config.FontName);
	}
	if (m_config.FontSize) {
		m_lf.lfHeight = -m_config.FontSize;
	}
	if (m_config.FontStyle & FONTSTYLE_ITALIC) m_lf.lfItalic = 1;
	if (m_config.FontStyle & FONTSTYLE_UNDERLINE) m_lf.lfUnderline = 1;
	if (m_config.FontStyle & FONTSTYLE_DELLINE) m_lf.lfStrikeOut = 1;
	if (m_config.FontStyle & FONTSTYLE_WEIGHT) m_lf.lfWeight = m_config.FontStyle & FONTSTYLE_WEIGHT;

	hf = CreateFontIndirect(&m_lf);
	m_edit_text.SetFont(hf);

	tlog.Set(m_config.MaxLogNum);
	tlog.Jump(0);

	//tlog.Add("中文文本样例01\n中文文本样例02\n中文文本样例03\n中文文本样例04\n中文文本样例05\n中文文本样例06\n中文文本样例07\n中文文本样例08",
	//	"日文文本样例01\n日文文本样例02\n日文文本样例03\n日文文本样例04\n日文文本样例05\n日文文本样例06\n日文文本样例07\n日文文本样例08");

	RECT rect_main, rect_edit, rect_info;

	GetWindowRect(&rect_main);
	m_minh = rect_main.bottom - rect_main.top;
	m_minw = rect_main.right - rect_main.left;

	GetClientRect(&rect_main);
	m_edit_text.GetClientRect(&rect_edit);
	m_static_info.GetClientRect(&rect_info);

	m_dy_edit = rect_main.bottom - rect_edit.bottom;

	m_edit_text.GetWindowRect(&rect_edit);
	ScreenToClient(&rect_edit);
	m_x_edit = rect_edit.left;
	m_y_edit = rect_edit.top;

	m_h_info = rect_info.bottom;

	if (m_config.Width && m_config.Height) {
		MoveWindow(m_config.PosX, m_config.PosY, m_config.Width, m_config.Height);
	}

	SetTextCtrl();

	m_static_info.SetWindowTextA("等待游戏运行...");

	return TRUE;
}

LRESULT CMainDlg::OnClose(UINT, WPARAM, LPARAM, BOOL &)
{
	switch (m_status)
	{
	case Status::InitVoicePlayer:
	case Status::Running:
		Za::Main::EndVoiceTables();
		LOG("已结束语音播放");
		Za::Main::CloseGameProcess();
		LOG("已关闭远程访问");
	default:
		break;
	}
	Za::Main::End();
	LOG("已退出语音系统");
	KillTimer(TIMER_ID);

	SaveConfig();

	CloseDialog(0);

	return 0;
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

LRESULT CMainDlg::OnTimer(UINT, WPARAM nID, LPARAM, BOOL &)
{
	if (nID == TIMER_ID) {
		switch (m_status)
		{
		case Status::WaitingGameStart:
			if (Za::Main::OpenGameProcess(m_gpOut, m_gpIn)) {
				m_status = Status::InitVoicePlayer;
				m_vpIn.asyn = true;
				m_vpIn.callBack = nullptr;
				LOG("游戏已运行");
				LOG("游戏标题为:%s", m_gpOut.Title);
				LOG("游戏说明为:%s", m_gpOut.Comment);
				LOG("远程数据地址：0x%08X， 大小：0x%04X", m_gpOut.RemoteDataAddr, m_gpOut.RemoteDataSize);

				LOG("加载语音表...");

				m_static_info.SetWindowTextA("加载语音表...");
				Za::Main::StartVoiceTables(m_vpOut, m_vpIn);
			}
			break;
		case Status::InitVoicePlayer:
			if (m_vpOut.Finished) {
				LOG("已加载语音表");
				LOG("语音表总数：%d", m_vpOut.Count);
				LOG("开始播放语音...");
				m_status = Status::Running;

				std::stringstream ss;
				ss << "正在播放语音..."
					<< " | " << "语音表数：" << m_vpOut.Count
					<< " | " << m_gpOut.Comment;

				m_static_info.SetWindowTextA(ss.str().c_str());

				ss.str("");
				ss << TITLE << " - " << m_gpOut.Title;
				this->SetWindowTextA(ss.str().c_str());
			}
			break;
		case Status::Running:
			if (Za::Main::CheckGameEnd()) {
				LOG("游戏已退出");
				Za::Main::EndVoiceTables();
				LOG("已结束语音播放");
				Za::Main::CloseGameProcess();
				LOG("已关闭远程访问");
				m_status = Status::WaitingGameStart;

				m_static_info.SetWindowTextA("终止，重新等待游戏运行...");
				this->SetWindowTextA(TITLE);
			}
		default:
			break;
		}
	}
	return 0;
}

LRESULT CMainDlg::OnCtrlColorEdit(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL & bHandled)
{
	HBRUSH hbr;
	HDC  hdc = (HDC)wParam;
	HWND  hwnd = (HWND)lParam;

	hbr = (HBRUSH)GetCurrentObject(hdc, OBJ_BRUSH);

	if (hwnd == m_edit_text)
	{
		SetTextColor(hdc, m_fontcolor);
		SetBkMode(hdc, TRANSPARENT);
	}
	else
	{
		hbr = (HBRUSH)GetCurrentObject(NULL, OBJ_BRUSH);
	}
	return (LRESULT)hbr;
}

LRESULT CMainDlg::OnSize(UINT, WPARAM, LPARAM, BOOL &)
{
	RECT rect_main;
	GetClientRect(&rect_main);

	m_edit_text.MoveWindow(m_x_edit, m_y_edit, rect_main.right - m_x_edit * 2, rect_main.bottom - m_dy_edit);
	m_static_info.MoveWindow(0, rect_main.bottom - m_h_info, rect_main.right, m_h_info);

	return 0;
}

LRESULT CMainDlg::OnGetMinMaxInfo(UINT, WPARAM wParam, LPARAM lParam, BOOL &)
{
	MINMAXINFO * info = (MINMAXINFO *)lParam;
	info->ptMinTrackSize.x = m_minw;
	info->ptMinTrackSize.y = m_minh;
	return LRESULT();
}

LRESULT CMainDlg::OnMsgReceived(UINT, WPARAM wParam, LPARAM lParam, BOOL &)
{
	if (m_status != Status::Running) return 0;

	m_msgIn.lparam = lParam;
	m_msgIn.wparam = wParam;
	LOG("收到消息，type=%d, addr=0x%0X", wParam, lParam);
	Za::Main::MessageReceived(m_msgOut, m_msgIn);

	if (m_msgOut.Type != MSGTYPE_SHOWTEXT && m_msgOut.Type != MSGTYPE_PLAYWAIT && m_msgOut.Type != MSGTYPE_LOADSCENA)
		return 0;

	std::stringstream ss;
	ss << "正在播放语音..."
		<< " | " << "语音表数：" << m_vpOut.Count
		<< " | " << m_gpOut.Comment;
	if (m_msgOut.VoiceFileName && m_msgOut.VoiceFileName[0]) {
		LOG("播放语音文件：%s", m_msgOut.VoiceFileName);

		ss << " | " << m_msgOut.VoiceFileName;
	}
	
	if (m_msgOut.CnText && m_msgOut.CnText[0]) {
		LOG("中文对白：\n%s", m_msgOut.CnText);

		tlog.Add(m_msgOut.CnText, m_msgOut.JpText);
		SetTextCtrl();
	}
	if (m_msgOut.JpText && m_msgOut.JpText[0]) {
		LOG("日文对白：\n%s", m_msgOut.JpText);
	}

	m_static_info.SetWindowTextA(ss.str().c_str());

	return 0;
}

LRESULT CMainDlg::OnNMCustomdrawSliderVolume(int /*idCtrl*/, LPNMHDR pNMHDR, BOOL& /*bHandled*/)
{
	LPNMCUSTOMDRAW pNMCD = reinterpret_cast<LPNMCUSTOMDRAW>(pNMHDR);
	// TODO: 在此添加控件通知处理程序代码

	static char buff[10];
	int pos = m_slider_volume.GetPos();
	m_edit_volume.GetWindowTextA(buff, sizeof(buff) - 1);
	if (buff[0] == 0 || pos != atoi(buff)) {
		sprintf(buff, "%d", pos);
		m_edit_volume.SetWindowTextA(buff);
	}

	if (pos != m_pcIn.volume) {
		m_pcIn.volume = pos;
		Za::Main::SetVoicePlayConfig(m_pcIn);
	}

	return 0;
}

LRESULT CMainDlg::OnEnChangeEditVolume(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	// TODO:  如果该控件是 RICHEDIT 控件，它将不
	// 发送此通知，除非重写 __super::OnInitDialog()
	// 函数并调用 CRichEditCtrl().SetEventMask()，
	// 同时将 ENM_CHANGE 标志“或”运算到掩码中。

	// TODO:  在此添加控件通知处理程序代码

	static char buff[10];
	m_edit_volume.GetWindowTextA(buff, sizeof(buff) - 1);
	if (buff[0] == 0 || m_slider_volume.GetPos() == atoi(buff)) {
		return 0;
	}
	int pos;
	sscanf(buff, "%d", &pos);
	m_slider_volume.SetPos(pos);

	if (pos != m_pcIn.volume) {
		m_pcIn.volume = pos;
		Za::Main::SetVoicePlayConfig(m_pcIn);
	}

	return 0;
}

LRESULT CMainDlg::OnBnClickedButtonSt(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	// TODO: 在此添加控件通知处理程序代码
	tlog.Jump(1);
	SetTextCtrl();
	return 0;
}

LRESULT CMainDlg::OnBnClickedButtonPre(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	// TODO: 在此添加控件通知处理程序代码
	tlog.Jump(tlog.CurIdx() - 1);
	SetTextCtrl();
	return 0;
}

LRESULT CMainDlg::OnBnClickedButtonNex(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	// TODO: 在此添加控件通知处理程序代码
	tlog.Jump(tlog.CurIdx() + 1);
	SetTextCtrl();
	return 0;
}

LRESULT CMainDlg::OnBnClickedButtonLst(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	// TODO: 在此添加控件通知处理程序代码
	tlog.Jump(tlog.Count());
	SetTextCtrl();
	return 0;
}

LRESULT CMainDlg::OnBnClickedCheckDov(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	// TODO: 在此添加控件通知处理程序代码
	m_pcIn.disableOriVoice = m_check_dov.GetCheck();
	Za::Main::SetVoicePlayConfig(m_pcIn);

	return 0;
}

LRESULT CMainDlg::OnBnClickedButtonFnt(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	// TODO: 在此添加控件通知处理程序代码
	CFontDialog fd(&m_lf);
	fd.m_cf.rgbColors = m_fontcolor;

	if (fd.DoModal() == IDOK) {
		HFONT hf = m_edit_text.GetFont();
		DeleteObject(hf);
		hf = CreateFontIndirect(&m_lf);

		m_edit_text.SetFont(hf);
		m_fontcolor = fd.GetColor();
		//DeleteObject(hf);

		m_changed_font = true;
	}

	return 0;
}

LRESULT CMainDlg::OnBnClickedButtonClear(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	// TODO: 在此添加控件通知处理程序代码

	tlog.Clear();
	SetTextCtrl();

	return 0;
}

void CMainDlg::CloseDialog(int nVal)
{
	DestroyWindow();
	::PostQuitMessage(nVal);
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

void CMainDlg::SaveConfig()
{
	m_config.Volume = m_pcIn.volume;
	m_config.DisableOriVoice = m_pcIn.disableOriVoice ? 1 : 0;
	RECT rect;
	this->GetWindowRect(&rect);
	m_config.Width = rect.right - rect.left;
	m_config.Height = rect.bottom - rect.top;
	m_config.PosX = rect.left;
	m_config.PosY = rect.top;

	if (m_changed_font) {
		strcpy(m_config.FontName, m_lf.lfFaceName);
		m_config.FontSize = m_lf.lfHeight; if (m_config.FontSize < 0) m_config.FontSize = -m_config.FontSize;
		m_config.FontColor = m_fontcolor;
		m_config.FontStyle = m_lf.lfWeight;
		if (m_lf.lfItalic) m_config.FontStyle |= FONTSTYLE_ITALIC;
		if (m_lf.lfStrikeOut) m_config.FontStyle |= FONTSTYLE_DELLINE;
		if (m_lf.lfUnderline) m_config.FontStyle |= FONTSTYLE_UNDERLINE;
	}
	m_config.SaveConfig(CONFIG_FILENAME);
}

void CMainDlg::SetTextCtrl()
{
	if (tlog.CurIdx() <= 1) {
		m_button_st.EnableWindow(FALSE);
		m_button_pre.EnableWindow(FALSE);
	}
	else {
		m_button_st.EnableWindow(TRUE);
		m_button_pre.EnableWindow(TRUE);
	}

	if (tlog.CurIdx() >= tlog.Count()) {
		m_button_lst.EnableWindow(FALSE);
		m_button_nxt.EnableWindow(FALSE);
	}
	else {
		m_button_lst.EnableWindow(TRUE);
		m_button_nxt.EnableWindow(TRUE);
	}
	m_edit_text.SetWindowTextA(tlog.CurText());

	std::stringstream ss;
	ss << tlog.CurIdx() << '/' << tlog.Count();
	m_static_cnt.SetWindowTextA(ss.str().c_str());
}

void CMainDlg::TextLog::Clear()
{
	_cur = 0;
	_first_idx = 0;
	_logs.clear();
}

void CMainDlg::TextLog::Add(const char * cnText, const char * jpText)
{
	std::stringstream ss;

	int cnt_line = 1;
	if (cnText && cnText[0]) {
		while (*cnText)
		{
			if (*cnText == '\n') {
				ss << "\r";
				cnt_line++;
			}
				
			ss << *cnText;
			++cnText;
		}
		for (int j = cnt_line; j < DFT_LINES_NUM; ++j) {
			ss << "\r\n";
		}
		ss << "\r\n\r\n";
	}
	if (jpText && jpText[0]) {
		while (*jpText)
		{
			if (*jpText == '\n') ss << "\r";

			ss << *jpText;
			++jpText;
		}
	}

	if (_max == 0 || (int)_logs.size() < _max) {
		_logs.push_back(ss.str());
	}
	else {
		_logs[_first_idx] = ss.str();
		++_first_idx;
		if (_first_idx == _max) _first_idx = 0;
	}
	Jump(this->Count());
}

void CMainDlg::TextLog::Jump(int idx)
{
	if (idx > _max) idx = _max;

	_cur = idx;
}

const char * CMainDlg::TextLog::CurText() const
{
	if (_cur == 0) return "";
	return _logs[(_cur - 1 + _first_idx) % _max].c_str();
}


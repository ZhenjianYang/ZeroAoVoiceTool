// MainDlg.cpp : implementation of the CMainDlg class
//
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "resource.h"

#include "aboutdlg.h"
#include "MainDlg.h"

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

	SetWorkPath();
	if (!m_config.LoadConfig(CONFIG_FILENAME)) {
		m_config.Reset();
	}

	LOG_OPEN;

	LOG("��ʼ��...");
	if (!Za::Main::Init()) {
		MessageBox("��ȡ���ݳ����˳���", "����", MB_OK);
		CloseDialog(-1);
		return TRUE;
	}
	LOG("��ʼ���ɹ���");
	m_status = Status::WaitingGameStart;
	LOG("�ȴ���Ϸ����...");

	m_gpIn.hMainWindow = (int)m_hWnd;
	m_gpIn.msgId = REMOTE_MSG_ID;
	SetTimer(TIMER_ID, TIMER_TIME);

	return TRUE;
}

LRESULT CMainDlg::OnClose(UINT, WPARAM, LPARAM, BOOL &)
{
	switch (m_status)
	{
	case Status::InitVoicePlayer:
	case Status::Running:
		Za::Main::EndVoiceTables();
		Za::Main::CloseGameProcess();
	default:
		break;
	}
	Za::Main::End();
	KillTimer(TIMER_ID);

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
				LOG("��Ϸ������");
				LOG("��Ϸ����Ϊ:%s", m_gpOut.Title);
				LOG("��Ϸ˵��Ϊ:%s", m_gpOut.Comment);
				LOG("Զ�����ݵ�ַ��0x%08X�� ��С��0x%04X", m_gpOut.RemoteDataAddr, m_gpOut.RemoteDataSize);

				LOG("����������...");
				Za::Main::StartVoiceTables(m_vpOut, m_vpIn);
			}
			break;
		case Status::InitVoicePlayer:
			if (m_vpOut.Finished) {
				LOG("�Ѽ���������");
				LOG("������������%d", m_vpOut.Count);
				LOG("��ʼ��������...");
				m_status = Status::Running;
			}
			break;
		case Status::Running:
			if (Za::Main::CheckGameEnd()) {
				m_status = Status::WaitingGameStart;
			}
		default:
			break;
		}
	}
	return 0;
}

LRESULT CMainDlg::OnMsgReceived(UINT, WPARAM wParam, LPARAM lParam, BOOL &)
{
	m_msgIn.lparam = lParam;
	m_msgIn.wparam = wParam;
	LOG("�յ���Ϣ��type=%d, addr=0x%0X", wParam, lParam);
	Za::Main::MessageReceived(m_msgOut, m_msgIn);
	if (m_msgOut.VoiceFileName && m_msgOut.VoiceFileName[0]) {
		LOG("���������ļ���%s", m_msgOut.VoiceFileName);
	}
	if (m_msgOut.CnText && m_msgOut.CnText[0]) {
		LOG("���Ķ԰ף�\n%s", m_msgOut.CnText);
	}
	if (m_msgOut.JpText && m_msgOut.JpText[0]) {
		LOG("���Ķ԰ף�\n%s", m_msgOut.JpText);
	}

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

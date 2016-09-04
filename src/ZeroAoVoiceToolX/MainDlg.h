// MainDlg.h : interface of the CMainDlg class
//
/////////////////////////////////////////////////////////////////////////////

#pragma once

#define WM_MSG_BASE 0x800
#define WM_MSG_ERROR (WM_MSG_BASE + 1)
#define WM_MSG_STOP (WM_MSG_BASE + 2)
#define WN_MSG_GAMEFOUND (WM_MSG_BASE + 10)
#define WN_MSG_GAMEEXIT (WM_MSG_BASE + 20)
#define WN_MSG_PLAYEND (WM_MSG_BASE + 30)
#define WN_MSG_INITPLAYEREND (WM_MSG_BASE + 40)
#define WN_MSG_REMOTEBASE (WM_MSG_BASE + 50)
#define WN_MSG_LOADSCENA  (WN_MSG_REMOTEBASE + MSGID_ADDER_LOADSCENA)
#define WN_MSG_LOADSCENA1 (WN_MSG_REMOTEBASE + MSGID_ADDER_LOADSCENA1)
#define WN_MSG_LOADBLOCK  (WN_MSG_REMOTEBASE + MSGID_ADDER_LOADBLOCK)
#define WN_MSG_SHOWTEXT   (WN_MSG_REMOTEBASE + MSGID_ADDER_SHOWTEXT)

class CMainDlg : public CDialogImpl<CMainDlg>, public CUpdateUI<CMainDlg>,
		public CMessageFilter, public CIdleHandler
{
public:
	enum {
		IDD = IDD_MAINDLG,
	};
	enum Status {
		Idle = 0,
		WaitingGameStart,
		InitVoicePlayer,
		Running,
	};
	enum ErrorType {
		Unknow = 0,
		InitVoicePlayerFailed,
	};

	virtual BOOL PreTranslateMessage(MSG* pMsg)
	{
		return CWindow::IsDialogMessage(pMsg);
	}

	virtual BOOL OnIdle()
	{
		UIUpdateChildWindows();
		return FALSE;
	}

	BEGIN_UPDATE_UI_MAP(CMainDlg)
	END_UPDATE_UI_MAP()

	BEGIN_MSG_MAP(CMainDlg)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
		MESSAGE_HANDLER(WM_CLOSE, OnClose)
		COMMAND_HANDLER(IDC_BUTTON_START, BN_CLICKED, OnBnClickedButtonStart)
	END_MSG_MAP()

// Handler prototypes (uncomment arguments if needed):
//	LRESULT MessageHandler(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
//	LRESULT CommandHandler(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
//	LRESULT NotifyHandler(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/)

	LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);

	LRESULT OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnClose(UINT Msg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

	void CloseDialog(int nVal);

	LRESULT OnBnClickedButtonStart(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);

	static void SetWorkPath();

	typedef void(*MonitorFunc)();

	static void Monitor_GameStart();
	static void Monitor_GameExit();

	static MonitorFunc pmfs[];
	static void ClearAllMonitorFunc();
	static bool AddMonitorFunc(MonitorFunc func);
	static void RemoveMonitorFunc(MonitorFunc func);
	static DWORD WINAPI Thread_Monitor(LPVOID lpParmeter);

	static DWORD WINAPI Thread_InitVoicePlayer(LPVOID lpParmeter);

	Status m_status;
	CButton m_button_start;

	static HANDLE s_th_monitor;
	static HWND s_hWnd_main;
	static HANDLE s_th_initplayer;
};

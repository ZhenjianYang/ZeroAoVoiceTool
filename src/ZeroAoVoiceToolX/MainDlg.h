// MainDlg.h : interface of the CMainDlg class
//
/////////////////////////////////////////////////////////////////////////////

#pragma once

#define WM_MSG_BASE 0x800
#define WM_MSG_ERROR (WM_MSG_BASE + 1)
#define WM_MSG_STOP (WM_MSG_BASE + 2)
#define WM_MSG_GAMEFOUND (WM_MSG_BASE + 10)
#define WM_MSG_GAMEEXIT (WM_MSG_BASE + 20)
#define WM_MSG_PLAYEND (WM_MSG_BASE + 30)
#define WM_MSG_INITPLAYEREND (WM_MSG_BASE + 40)
#define WM_MSG_INITPLAYERSTOPPED (WM_MSG_BASE + 41)
#define WM_MSG_REMOTEBASE (WM_MSG_BASE + 50)
#define WM_MSG_LOADSCENA  (WM_MSG_REMOTEBASE + MSGID_ADDER_LOADSCENA)
#define WM_MSG_LOADSCENA1 (WM_MSG_REMOTEBASE + MSGID_ADDER_LOADSCENA1)
#define WM_MSG_LOADBLOCK  (WM_MSG_REMOTEBASE + MSGID_ADDER_LOADBLOCK)
#define WM_MSG_SHOWTEXT   (WM_MSG_REMOTEBASE + MSGID_ADDER_SHOWTEXT)

class CMainDlg : public CDialogImpl<CMainDlg>, public CUpdateUI<CMainDlg>,
		public CMessageFilter, public CIdleHandler
{
public:
	enum {
		IDD = IDD_MAINDLG,
	};
	enum class Status {
		Idle = 0,
		WaitingGameStart,
		InitVoicePlayer,
		StoppingInitVoicePlayer,
		Running,
		Exitting
	};
	enum class ErrorType {
		Unknow = 0,
		InitRemoteFailed,
		InitVoicePlayerFailed,
		ReadRemoteDataFailedRunning
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
		MESSAGE_HANDLER(WM_MSG_ERROR, OnError)
		MESSAGE_HANDLER(WM_MSG_STOP, OnStop)
		MESSAGE_HANDLER(WM_MSG_GAMEFOUND, OnGameFound)
		MESSAGE_HANDLER(WM_MSG_GAMEEXIT, OnGameExit)
		MESSAGE_HANDLER(WM_MSG_PLAYEND, OnPlayEnd)
		MESSAGE_HANDLER(WM_MSG_INITPLAYEREND, OnInitPlayerEnd)
		MESSAGE_HANDLER(WM_MSG_INITPLAYERSTOPPED, OnInitPlayerStoped)
		MESSAGE_HANDLER(WM_MSG_LOADSCENA, OnRLoadScena)
		MESSAGE_HANDLER(WM_MSG_LOADSCENA1, OnRLoadScena1)
		MESSAGE_HANDLER(WM_MSG_LOADBLOCK, OnRLoadBlock)
		MESSAGE_HANDLER(WM_MSG_SHOWTEXT, OnRShowText)
		NOTIFY_HANDLER(IDC_SLIDER_ZV, NM_CUSTOMDRAW, OnNMCustomdrawSliderV)
		NOTIFY_HANDLER(IDC_SPIN_ZV, UDN_DELTAPOS, OnDeltaposSpinV)
		NOTIFY_HANDLER(IDC_SLIDER_AV, NM_CUSTOMDRAW, OnNMCustomdrawSliderV)
		NOTIFY_HANDLER(IDC_SPIN_AV, UDN_DELTAPOS, OnDeltaposSpinV)
		COMMAND_HANDLER(IDC_BUTTON_ZVB, BN_CLICKED, OnBnClickedButtonVb)
		COMMAND_HANDLER(IDC_BUTTON_ZVTB, BN_CLICKED, OnBnClickedButtonVb)
		COMMAND_HANDLER(IDC_BUTTON_AVB, BN_CLICKED, OnBnClickedButtonVb)
		COMMAND_HANDLER(IDC_BUTTON_AVTB, BN_CLICKED, OnBnClickedButtonVb)
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

	LRESULT OnError(UINT Msg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnStop(UINT Msg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnGameFound(UINT Msg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnGameExit(UINT Msg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnPlayEnd(UINT Msg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnInitPlayerEnd(UINT Msg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnInitPlayerStoped(UINT Msg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnRLoadScena(UINT Msg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnRLoadScena1(UINT Msg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnRLoadBlock(UINT Msg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnRShowText(UINT Msg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

	LRESULT OnNMCustomdrawSliderV(int /*idCtrl*/, LPNMHDR pNMHDR, BOOL& /*bHandled*/);
	LRESULT OnDeltaposSpinV(int /*idCtrl*/, LPNMHDR pNMHDR, BOOL& /*bHandled*/);
	LRESULT OnBnClickedButtonVb(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
private:
	void LoadConfig();

public:
	Status m_status;
	CButton m_button_start;

	CStatic m_group_zero;
	CStatic m_static_zvp, m_static_zve, m_static_zvtp, m_static_zvte, m_static_zv;
	CEdit m_edit_zvp, m_edit_zve, m_edit_zvtp, m_edit_zvte, m_edit_zv;
	CButton m_button_zvb, m_button_zvtb;
	CTrackBarCtrl m_slider_zv;
	CUpDownCtrl m_spin_zv;
	CButton m_check_zdov;

	CStatic m_group_ao;
	CStatic m_static_avp, m_static_ave, m_static_avtp, m_static_avte, m_static_av;
	CEdit m_edit_avp, m_edit_ave, m_edit_avtp, m_edit_avte, m_edit_av;
	CButton m_button_avb, m_button_avtb;
	CTrackBarCtrl m_slider_av;
	CUpDownCtrl m_spin_av;
	CButton m_check_adov;

	int m_gameID;
	ZAData m_zadata;

public:
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

	static int PlayEndCallBack(void*);

	static HANDLE s_th_monitor;
	static HWND s_hWnd_main;
	static HANDLE s_th_initplayer;
	static HANDLE s_event_monitor;
	static unsigned s_sign_initplayerstop;
	static unsigned s_sign_monitorstop;
};

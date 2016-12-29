// MainDlg.h : interface of the CMainDlg class
//
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Const.h"
#include "Config.h"

class CMainDlg : public CDialogImpl<CMainDlg>, public CUpdateUI<CMainDlg>,
		public CMessageFilter, public CIdleHandler
{
public:
	enum { IDD = IDD_MAINDLG };

	enum class Status {
		Idle = 0,
		WaitingGameStart,
		InitVoicePlayer,
		Running,
	};

	virtual BOOL PreTranslateMessage(MSG* pMsg);
	virtual BOOL OnIdle();

	BEGIN_UPDATE_UI_MAP(CMainDlg)
	END_UPDATE_UI_MAP()

	BEGIN_MSG_MAP(CMainDlg)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
		MESSAGE_HANDLER(WM_CLOSE, OnClose)
		MESSAGE_HANDLER(WM_TIMER, OnTimer)
		MESSAGE_HANDLER(REMOTE_MSG_ID, OnMsgReceived)
		NOTIFY_HANDLER(IDC_SLIDER_VOLUME, NM_CUSTOMDRAW, OnNMCustomdrawSliderVolume)
		COMMAND_HANDLER(IDC_EDIT_VOLUME, EN_CHANGE, OnEnChangeEditVolume)
	END_MSG_MAP()

// Handler prototypes (uncomment arguments if needed):
//	LRESULT MessageHandler(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
//	LRESULT CommandHandler(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
//	LRESULT NotifyHandler(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/)

	LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnClose(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnTimer(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnMsgReceived(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/);

	void CloseDialog(int nVal);

	void SetWorkPath();
	void SaveConfig();

	Config m_config;
	Status m_status;
	Za::Data::GameProcessIn m_gpIn;
	Za::Data::GameProcessOut m_gpOut;
	Za::Data::VoicePlayerIn m_vpIn;
	Za::Data::VoicePlayerOut m_vpOut;
	Za::Data::PlayConfigIn m_pcIn;
	Za::Data::MessageIn m_msgIn;
	Za::Data::MessageOut m_msgOut;

	CButton m_check_dov;
	CStatic m_static_volume;
	CTrackBarCtrl m_slider_volume;
	CEdit m_edit_volume;

	CButton m_button_st, m_button_pre, m_button_nxt, m_button_lst, m_button_fnt;
	CStatic m_static_cnt;
	CStatic m_static_info;
	CEdit m_edit_text;
	LRESULT OnNMCustomdrawSliderVolume(int /*idCtrl*/, LPNMHDR pNMHDR, BOOL& /*bHandled*/);
	LRESULT OnEnChangeEditVolume(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
};

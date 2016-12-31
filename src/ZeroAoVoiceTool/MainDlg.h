// MainDlg.h : interface of the CMainDlg class
//
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Const.h"
#include "Config.h"

#include <vector>
#include <string>

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

	class TextLog {
		std::vector<std::string> _logs;
		int _max;
		int _cur;
		int _first_idx;

	public:
		TextLog(int max = 0) : _first_idx(0), _cur(0) {  }
		void Set(int max = 0) { _max = max; }
		void Clear();
		void Add(const char* cnText, const char* jpText);
		void Jump(int idx);
		int Count() const { return _logs.size(); }
		int CurIdx() const { return _cur; }
		const char* CurText() const;
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
		MESSAGE_HANDLER(WM_CTLCOLOREDIT, OnCtrlColorEdit)
		MESSAGE_HANDLER(WM_CTLCOLORSTATIC, OnCtrlColorEdit)
		MESSAGE_HANDLER(WM_SIZE, OnSize)
		MESSAGE_HANDLER(WM_GETMINMAXINFO, OnGetMinMaxInfo)
		MESSAGE_HANDLER(REMOTE_MSG_ID, OnMsgReceived)
		NOTIFY_HANDLER(IDC_SLIDER_VOLUME, NM_CUSTOMDRAW, OnNMCustomdrawSliderVolume)
		COMMAND_HANDLER(IDC_EDIT_VOLUME, EN_CHANGE, OnEnChangeEditVolume)
		COMMAND_HANDLER(IDC_BUTTON_ST, BN_CLICKED, OnBnClickedButtonSt)
		COMMAND_HANDLER(IDC_BUTTON_PRE, BN_CLICKED, OnBnClickedButtonPre)
		COMMAND_HANDLER(IDC_BUTTON_NEX, BN_CLICKED, OnBnClickedButtonNex)
		COMMAND_HANDLER(IDC_BUTTON_LST, BN_CLICKED, OnBnClickedButtonLst)
		COMMAND_HANDLER(IDC_CHECK_DOV, BN_CLICKED, OnBnClickedCheckDov)
		COMMAND_HANDLER(IDC_BUTTON_FNT, BN_CLICKED, OnBnClickedButtonFnt)
		COMMAND_HANDLER(IDC_BUTTON_CLEAR, BN_CLICKED, OnBnClickedButtonClear)
	END_MSG_MAP()

// Handler prototypes (uncomment arguments if needed):
//	LRESULT MessageHandler(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
//	LRESULT CommandHandler(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
//	LRESULT NotifyHandler(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/)

	LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnClose(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnTimer(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnCtrlColorEdit(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnSize(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnGetMinMaxInfo(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/);
	LRESULT OnMsgReceived(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/);
	LRESULT OnNMCustomdrawSliderVolume(int /*idCtrl*/, LPNMHDR pNMHDR, BOOL& /*bHandled*/);
	LRESULT OnEnChangeEditVolume(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnBnClickedButtonSt(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnBnClickedButtonPre(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnBnClickedButtonNex(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnBnClickedButtonLst(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnBnClickedCheckDov(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnBnClickedButtonFnt(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnBnClickedButtonClear(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);

	void CloseDialog(int nVal);

	void SetWorkPath();
	void SaveConfig();
	void SetTextCtrl();

	Status m_status;

	Za::Data::GameProcessIn m_gpIn;
	Za::Data::GameProcessOut m_gpOut;
	Za::Data::VoicePlayerIn m_vpIn;
	Za::Data::VoicePlayerOut m_vpOut;
	Za::Data::PlayConfigIn m_pcIn;
	Za::Data::MessageIn m_msgIn;
	Za::Data::MessageOut m_msgOut;

	TextLog tlog;
	LOGFONT m_lf;
	COLORREF m_fontcolor;
	Config m_config;
	bool m_changed_font;

	CButton m_check_dov;
	CStatic m_static_volume;
	CTrackBarCtrl m_slider_volume;
	CEdit m_edit_volume;

	CButton m_button_st, m_button_pre, m_button_nxt, m_button_lst, m_button_fnt;
	CStatic m_static_cnt;
	CStatic m_static_info;
	CEdit m_edit_text;

	int m_dy_edit;
	int m_x_edit, m_y_edit;
	int m_h_info;
	int m_minw, m_minh;
};

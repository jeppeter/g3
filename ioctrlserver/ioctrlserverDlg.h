
// ioctrlserverDlg.h : header file
//

#pragma once


// CioctrlserverDlg dialog
class CioctrlserverDlg : public CDialogEx
{
// Construction
public:
	CioctrlserverDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	enum { IDD = IDD_IOCTRLSERVER_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnSelExe();
	afx_msg void OnSelDll();
	DECLARE_MESSAGE_MAP()
};

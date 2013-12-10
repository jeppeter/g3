
// ioctrlserverDlg.h : header file
//

#pragma once
#include <winsock2.h>
#include <iocapctrl.h>


// CioctrlserverDlg dialog
class CioctrlserverDlg : public CDialogEx
{
// Construction
public:
	CioctrlserverDlg(CWnd* pParent = NULL);	// standard constructor
	virtual ~CioctrlserverDlg();
// Dialog Data
	enum { IDD = IDD_IOCTRLSERVER_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support



private:
	void __StopControl();
	int __StartAccSocket(int port);
	void __CloseReadSock(void);
	int __HandleAccept(void);
	void __GetItemText(UINT id,CString& str);
	DWORD __SocketThread();
	static DWORD WINAPI HandleSocketThread(LPVOID pParam);
	

private:
	CIOController           *m_pIoController;
	SOCKET m_Accsock;
	SOCKET m_Readsock;
// Implementation
protected:
	HICON m_hIcon;
	CString m_strDll;
	CString m_strExe;
	CString m_strParam;
	int m_BufNum;
	int m_BufSectSize;
	int m_TimeWait;
	int m_ListenPort;
	thread_control_t m_ThreadControl;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnSelExe();
	afx_msg void OnSelDll();
	afx_msg void OnStart();
	afx_msg void OnClose();
	afx_msg void OnCancel();
	afx_msg LRESULT OnSocket(WPARAM WParam, LPARAM lParam)  ;
	DECLARE_MESSAGE_MAP()
};

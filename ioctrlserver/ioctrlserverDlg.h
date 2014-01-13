
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
	void __CloseAccSocket();
	void __GetItemText(UINT id,CString& str);
	DWORD __SocketThread();
	static DWORD WINAPI HandleSocketThread(LPVOID pParam);
	unsigned long __ItemAtoi(UINT id,int base=10);
	

private:
	CIOController           *m_pIoController;
	SOCKET m_Accsock;
	SOCKET m_Readsock;
	CString m_strDll;
	CString m_strExe;
	CString m_strParam;	
	HANDLE m_hProc;	
	thread_control_t m_ThreadControl;
// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnSelExe();
	afx_msg void OnSelDll();
	afx_msg void OnStart();
	afx_msg void OnClose();
	afx_msg void OnCancel();
	afx_msg void OnAttach();
	afx_msg LRESULT OnSocket(WPARAM WParam, LPARAM lParam)  ;
	DECLARE_MESSAGE_MAP()
};


// imgctrldemoDlg.h : 头文件
//

#pragma once

#define  SNAPSHOT_TIME_ID   131

// CimgctrldemoDlg 对话框
class CimgctrldemoDlg : public CDialogEx
{
// 构造
public:
	CimgctrldemoDlg(CWnd* pParent = NULL);	// 标准构造函数
	LRESULT OnHotKey(WPARAM wParam, LPARAM lParam);
	CString m_strExe;
	CString m_strDll;
	CString m_strBmp;
	CString m_strParam;

// 对话框数据
	enum { IDD = IDD_IMGCTRLDEMO_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持
private:
	int SnapShort();
	int SnapShot();
private:
	DWORD m_CallProcessId;
	int m_BmpId;
	int m_SnapSecond;
	int m_RealWrite;
	
// 实现
protected:
	HICON m_hIcon;

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnLoad();
	afx_msg void OnSelExe();
	afx_msg void OnSelDll();
	afx_msg void OnSelBmp();
	afx_msg void OnTimer(UINT nEvent);
	DECLARE_MESSAGE_MAP()
};

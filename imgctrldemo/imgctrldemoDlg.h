
// imgctrldemoDlg.h : ͷ�ļ�
//

#pragma once

#define  SNAPSHOT_TIME_ID   131

// CimgctrldemoDlg �Ի���
class CimgctrldemoDlg : public CDialogEx
{
// ����
public:
	CimgctrldemoDlg(CWnd* pParent = NULL);	// ��׼���캯��
	LRESULT OnHotKey(WPARAM wParam, LPARAM lParam);
	CString m_strExe;
	CString m_strDll;
	CString m_strBmp;
	CString m_strParam;

// �Ի�������
	enum { IDD = IDD_IMGCTRLDEMO_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV ֧��
private:
	int SnapShort();
	int SnapShot();
private:
	DWORD m_CallProcessId;
	int m_BmpId;
	int m_SnapSecond;
	int m_RealWrite;
	
// ʵ��
protected:
	HICON m_hIcon;

	// ���ɵ���Ϣӳ�亯��
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

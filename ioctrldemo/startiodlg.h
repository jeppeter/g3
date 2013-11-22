
#ifndef __START_IO_DLG_H__
#define __START_IO_DLG_H__


class CStartIoDlg : public CDialog
{
public:
    CStartIoDlg(CWnd* pParent = NULL);	// ��׼���캯��
    ~CStartIoDlg();
    CString m_strExec;
    CString m_strParam;
    CString m_strDll;
	int m_iBufNum;
	int m_iBufSize;
    int m_iDiK;

    // �Ի�������
    enum { IDD = IDD_START_DIALOG };
protected:
    HICON m_hIcon;

    // ���ɵ���Ϣӳ�亯��
    virtual BOOL OnInitDialog();
    afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
    afx_msg void OnPaint();
    afx_msg HCURSOR OnQueryDragIcon();
    DECLARE_MESSAGE_MAP()
public:
    afx_msg void OnBtnExe();
    afx_msg void OnBtnDll();
	afx_msg void OnOK();
	afx_msg void OnCancel();
};


#endif /*__START_IO_DLG_H__*/


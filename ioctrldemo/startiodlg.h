
#ifndef __START_IO_DLG_H__
#define __START_IO_DLG_H__

class CStartIoDlg : public CDialogEx
{
public:
    CpcmctrldemoDlg(CWnd* pParent = NULL);	// 标准构造函数
    ~CpcmctrldemoDlg()
    {
        this->StopCapper();
    };
    CString m_strExec;
    CString m_strParam;
    CString m_strDll;
    CString m_strDump;
    CString m_strBufNum;
    CString m_strBlockSize;

    // 对话框数据
    enum { IDD = IDD_PCMCTRLDEMO_DIALOG };
};


#endif /*__START_IO_DLG_H__*/


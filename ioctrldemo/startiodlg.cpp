
#include "resource.h"
#include "startiodlg.h"

#ifdef _DEBUG
#pragma comment(lib,"injectctrld.lib")
#pragma comment(lib,"iocapctrld.lib")
#else
#pragma comment(lib,"injectctrl.lib")
#pragma comment(lib,"iocapctrl.lib")
#endif


CStartIoDlg::CStartIoDlg(CWnd* pParent /*=NULL*/)
    : CDialogEx(CStartIoDlg::IDD, pParent)
{
    m_hIcon = AfxGetApp()->LoadIcon(IDI_ICON1);
    m_hProc = NULL;
}

void CStartIoDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CStartIoDlg, CDialogEx)
    ON_WM_SYSCOMMAND()
    ON_WM_QUERYDRAGICON()
    ON_BN_CLICKED(IDC_BTN_EXE,OnBtnExe)
    ON_BN_CLICKED(IDC_BTN_DLL,OnBtnDll)
    ON_BN_CLICKED(IDOK, OnOK)
    ON_BN_CLICKED(IDCANCEL, OnCancel)
END_MESSAGE_MAP()


BOOL CStartIoDlg::OnInitDialog()
{
    CEdit* pEdt=NULL;
    CString fstr;
    CDialogEx::OnInitDialog();

    // 将“关于...”菜单项添加到系统菜单中。

    // IDM_ABOUTBOX 必须在系统命令范围内。


    // 设置此对话框的图标。当应用程序主窗口不是对话框时，框架将自动
    //  执行此操作
    SetIcon(m_hIcon, TRUE);			// 设置大图标
    SetIcon(m_hIcon, FALSE);		// 设置小图标

    // TODO: 在此添加额外的初始化代码
    pEdt = (CEdit*)this->GetDlgItem(IDC_EDT_BUFNUM);
    fstr.Format(TEXT("%d"),10);
    pEdt->SetWindowText(fstr);
    pEdt = (CEdit*)this->GetDlgItem(IDC_EDT_BUFSIZE);
    fstr.Format(TEXT("%x"),10240);
    pEdt->SetWindowText(fstr);

    return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}


void CStartIoDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
    CDialogEx::OnSysCommand(nID, lParam);
}

HCURSOR CStartIoDlg::OnQueryDragIcon()
{
    return static_cast<HCURSOR>(m_hIcon);
}

void CStartIoDlg::OnBtnExe()
{
    CFileDialog fdlg(TRUE,NULL,NULL,OFN_FILEMUSTEXIST|OFN_PATHMUSTEXIST|OFN_READONLY,
                     TEXT("execute files (*.exe)|*.exe||"),NULL);
    CString fname;
    CEdit* pEdt=NULL;
    if(fdlg.DoModal() == IDOK)
    {
        fname = fdlg.GetPathName();
        pEdt = (CEdit*) this->GetDlgItem(IDC_EDT_EXE);
        pEdt->SetWindowText(fname);
    }
}

void CStartIoDlg::OnBtnDll()
{
    CFileDialog fdlg(TRUE,NULL,NULL,OFN_FILEMUSTEXIST|OFN_PATHMUSTEXIST|OFN_READONLY,
                     TEXT("dynamic link library files (*.dll)|*.dll||"),NULL);
    CString fname;
    CEdit* pEdt=NULL;
    if(fdlg.DoModal() == IDOK)
    {
        fname = fdlg.GetPathName();
        pEdt = (CEdit*) this->GetDlgItem(IDC_EDT_DLL);
        pEdt->SetWindowText(fname);
    }
}

void CStartIoDlg::OnOK()
{
    CEdit* pEdt=NULL;
    CString errstr,caption=TEXT("Error"),tmpstr;
    /*now we get the text*/
    pEdt = (CEdit*)this->GetDlgItem(IDC_EDT_EXE);
    pEdt->GetWindowText(this->m_strExec);
    if(this->m_strExec.GetLength() == 0)
    {
        errstr.Format(TEXT("Please Specify Exec"));
        MessageBox(this->m_hWnd,(LPCTSTR)errstr,(LPCTSTR)caption,MB_OK);
        return ;
    }

    pEdt = (CEdit*) this->GetDlgItem(IDC_EDT_DLL);
    pEdt->GetWindowText(this->m_strDll);
    if(this->m_strDll.GetLength() == 0)
    {
        errstr.Format(TEXT("Please Specify Dll"));
        MessageBox(this->m_hWnd,(LPCTSTR)errstr,(LPCTSTR)caption,MB_OK);
        return ;
    }

    pEdt = (CEdit*) this->GetDlgItem(IDC_EDT_PARAM);
    pEdt->GetWindowText(this->m_strParam);

    pEdt = (CEdit*) this->GetDlgItem(IDC_EDT_BUFNUM);
    pEdt->GetWindowText(tmpstr);
    if(tmpstr.GetLength() == 0)
    {
        errstr.Format(TEXT("Please Specify BufNum"));
        MessageBox(this->m_hWnd,(LPCTSTR)errstr,(LPCTSTR)caption,MB_OK);
        return ;
    }

    this->m_iBufNum =_tcstoul(tmpstr, NULL, 10);
    if(this->m_iBufNum <= 0)
    {
        errstr.Format(TEXT("Please Specify BufNum > 0"));
        MessageBox(this->m_hWnd,(LPCTSTR)errstr,(LPCTSTR)caption,MB_OK);
        return ;
    }

    pEdt = (CEdit*) this->GetDlgItem(IDC_EDT_BUFSIZE);
    pEdt->GetWindowText(tmpstr);
    this->m_iBufSize=_tcstoul(tmpstr, NULL, 16);
    if(this->m_iBufSize <= 32)
    {
        errstr.Format(TEXT("Please Specify BufSize > 32"));
        MessageBox(this->m_hWnd,(LPCTSTR)errstr,(LPCTSTR)caption,MB_OK);
        return ;
    }

    CDialogEx::OnOK();
}

void CStartIoDlg::OnCancel()
{
    CDialogEx::OnCancel();
}


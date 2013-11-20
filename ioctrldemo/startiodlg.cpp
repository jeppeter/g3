
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

    // ��������...���˵�����ӵ�ϵͳ�˵��С�

    // IDM_ABOUTBOX ������ϵͳ���Χ�ڡ�


    // ���ô˶Ի����ͼ�ꡣ��Ӧ�ó��������ڲ��ǶԻ���ʱ����ܽ��Զ�
    //  ִ�д˲���
    SetIcon(m_hIcon, TRUE);			// ���ô�ͼ��
    SetIcon(m_hIcon, FALSE);		// ����Сͼ��

    // TODO: �ڴ���Ӷ���ĳ�ʼ������
    pEdt = (CEdit*)this->GetDlgItem(IDC_EDT_BUFNUM);
    fstr.Format(TEXT("%d"),10);
    pEdt->SetWindowText(fstr);
    pEdt = (CEdit*)this->GetDlgItem(IDC_EDT_BUFSIZE);
    fstr.Format(TEXT("%d"),10240);
    pEdt->SetWindowText(fstr);

    return TRUE;  // ���ǽ��������õ��ؼ������򷵻� TRUE
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
}

void CStartIoDlg::OnCancel()
{
}


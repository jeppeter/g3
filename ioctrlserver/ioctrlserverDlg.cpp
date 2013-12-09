
// ioctrlserverDlg.cpp : implementation file
//

#include "stdafx.h"
#include "ioctrlserver.h"
#include "ioctrlserverDlg.h"
#include "afxdialogex.h"
#include <output_debug.h>
#include "resource.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CioctrlserverDlg dialog
#ifdef _DEBUG
#pragma comment(lib,"injectctrld.lib")
#pragma comment(lib,"iocapctrld.lib")
#else
#pragma comment(lib,"injectctrl.lib")
#pragma comment(lib,"iocapctrl.lib")
#endif



CioctrlserverDlg::CioctrlserverDlg(CWnd* pParent /*=NULL*/)
    : CDialogEx(CioctrlserverDlg::IDD, pParent)
{
    m_pIoController = NULL;
    m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}


void CioctrlserverDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CioctrlserverDlg, CDialogEx)
    ON_WM_PAINT()
    ON_WM_CLOSE()
    ON_WM_QUERYDRAGICON()
    ON_COMMAND(IDC_BTN_EXE,OnSelExe)
    ON_COMMAND(IDC_BTN_DLL,OnSelDll)
    ON_COMMAND(IDC_BTN_START,OnStart)
    ON_COMMAND(IDCANCEL,OnCancel)
    ON_MESSAGE(WM_SOCKET,OnSocket)
END_MESSAGE_MAP()


// CioctrlserverDlg message handlers

BOOL CioctrlserverDlg::OnInitDialog()
{
    CEdit* pEdt=NULL;
    CString fmtstr;
    CDialogEx::OnInitDialog();

    // Set the icon for this dialog.  The framework does this automatically
    //  when the application's main window is not a dialog
    SetIcon(m_hIcon, TRUE);			// Set big icon
    SetIcon(m_hIcon, FALSE);		// Set small icon

    // TODO: Add extra initialization here
    pEdt = (CEdit*) this->GetDlgItem(IDC_EDT_BUFNUM);
    fmtstr.Format(TEXT("10"));
    pEdt->SetWindowText(fmtstr);
    pEdt = (CEdit*) this->GetDlgItem(IDC_EDT_BUFSIZE);
    fmtstr.Format(TEXT("400"));
    pEdt->SetWindowText(fmtstr);
    pEdt = (CEdit*) this->GetDlgItem(IDC_EDT_WAITTIME);
    fmtstr.Format(TEXT("1"));
    pEdt->SetWindowText(fmtstr);
    pEdt = (CEdit*) this->GetDlgItem(IDC_EDT_PORT);
    fmtstr.Format(TEXT("3391"));
    pEdt->SetWindowText(fmtstr);

    return TRUE;  // return TRUE  unless you set the focus to a control
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CioctrlserverDlg::OnPaint()
{
    if(IsIconic())
    {
        CPaintDC dc(this); // device context for painting

        SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

        // Center icon in client rectangle
        int cxIcon = GetSystemMetrics(SM_CXICON);
        int cyIcon = GetSystemMetrics(SM_CYICON);
        CRect rect;
        GetClientRect(&rect);
        int x = (rect.Width() - cxIcon + 1) / 2;
        int y = (rect.Height() - cyIcon + 1) / 2;

        // Draw the icon
        dc.DrawIcon(x, y, m_hIcon);
    }
    else
    {
        CDialogEx::OnPaint();
    }
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CioctrlserverDlg::OnQueryDragIcon()
{
    return static_cast<HCURSOR>(m_hIcon);
}

void CioctrlserverDlg::OnSelDll()
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
    return ;
}

void CioctrlserverDlg::OnSelExe()
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
    return ;
}

void CioctrlserverDlg::OnStart()
{
#ifdef _UNICODE
    wchar_t *pDllWide=NULL,*pExeWide=NULL,*pParamWide=NULL;
    int dllwidesize=0,exewidesize=0,paramwidesize=0;
    char *pDllAnsi=NULL,*pExeAnsi=NULL,*pParamAnsi=NULL;
    int dllansisize=0,exeansisize=0,paramansisize=0;
#else
    char *pDllAnsi=NULL,*pExeAnsi=NULL,*pParamAnsi=NULL;
    int dllansisize=0,exeansisize=0,paramansisize=0;
#endif



fail:
#ifdef _UNICODE
#endif
}


void CioctrlserverDlg::__StopControl()
{
    if(this->m_pIoController)
    {
        delete this->m_pIoController;
    }
    this->m_pIoController = NULL;
    return ;
}
void CioctrlserverDlg::OnClose()
{
    this->__StopControl();
    CDialogEx::OnClose();
    return ;
}

void CioctrlserverDlg::OnCancel()
{
    this->__StopControl();
    CDialogEx::OnCancel();
    return;
}

LRESULT CioctrlserverDlg::OnSocket(WPARAM WParam,LPARAM lParam)
{
}


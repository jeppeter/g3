
// ioctrlserverDlg.cpp : implementation file
//

#include "stdafx.h"
#include "ioctrlserver.h"
#include "ioctrlserverDlg.h"
#include "afxdialogex.h"
#include <output_debug.h>
#include "resource.h"
#include <assert.h>
#include <uniansi.h>
#include <dllinsert.h>
#include <injectctrl.h>
#include <procex.h>

#define  WM_SOCKET   (WM_USER+1)

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
    ZeroMemory(&m_ThreadControl,sizeof(m_ThreadControl));
    m_ThreadControl.exited = 1;
    m_pIoController = NULL;
    m_hProc = NULL;
    m_Accsock = INVALID_SOCKET;
    m_Readsock = INVALID_SOCKET;
    m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

CioctrlserverDlg::~CioctrlserverDlg()
{
    this->__StopControl();
    CDialogEx::~CDialogEx();
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
    ON_COMMAND(IDC_BTN_SAVECURSORBMP,OnSelBmp)
    ON_COMMAND(IDC_BTN_START,OnStart)
    ON_COMMAND(IDC_BTN_ATTACH,OnAttach)
    ON_COMMAND(IDC_BTN_SAVECURSORBMP,OnSaveCursorBmp)
    ON_COMMAND(IDCANCEL,OnCancel)
    ON_MESSAGE(WM_SOCKET,OnSocket)
    ON_BN_CLICKED(IDC_CHK_HIDECURSOR, OnHideCursorChk)
    ON_BN_CLICKED(IDC_CHK_SETCURSORPOS,OnSetCursorPosChk)
END_MESSAGE_MAP()


// CioctrlserverDlg message handlers

BOOL CioctrlserverDlg::OnInitDialog()
{
    CEdit* pEdt=NULL;
    CButton *pBtn=NULL;
    CString fmtstr;
    CDialogEx::OnInitDialog();

    // Set the icon for this dialog.  The framework does this automatically
    //  when the application's main window is not a dialog
    SetIcon(m_hIcon, TRUE);			// Set big icon
    SetIcon(m_hIcon, FALSE);		// Set small icon

    // TODO: Add extra initialization here
    pEdt = (CEdit*) this->GetDlgItem(IDC_EDT_BUFNUM);
    fmtstr.Format(TEXT("20"));
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
    pEdt = (CEdit*) this->GetDlgItem(IDC_EDT_ATTACHPID);
    fmtstr.Format(TEXT("0"));
    pEdt->SetWindowText(fmtstr);
    pEdt = (CEdit*) this->GetDlgItem(IDC_EDT_SAVECURSORBMP);
    fmtstr.Format(TEXT(""));
    pEdt->SetWindowText(fmtstr);

    pBtn = (CButton*) this->GetDlgItem(IDC_BTN_HIDECURSOR);
    pBtn->SetCheck(0);
    pBtn = (CButton*) this->GetDlgItem(IDC_BTN_SETCURSORPOS);
    pBtn->SetCheck(1);

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

void CioctrlserverDlg::OnSelBmp()
{
    CFileDialog fdlg(TRUE,NULL,NULL,0,
                     TEXT("bmp files (*.bmp)|*.bmp||"),NULL);
    CString fname;
    CEdit* pEdt=NULL;
    if(fdlg.DoModal() == IDOK)
    {
        CString bmpstr;
        fname = fdlg.GetPathName();
        pEdt = (CEdit*) this->GetDlgItem(IDC_EDT_BMP);
        pEdt->SetWindowText(fname);
    }
    return ;
}


unsigned long CioctrlserverDlg::__ItemAtoi(UINT id,int base)
{
    CString lstr;
    unsigned long lret;
#ifdef _UNICODE
    wchar_t *pEndPtr;
#else
    char* pEndPtr;
#endif
    this->__GetItemText(id,lstr);
#ifdef _UNICODE
    lret = wcstoul((LPCWSTR)lstr,&pEndPtr,base);
#else
    lret = strtoul((LPCSTR)lstr,&pEndPtr,base);
#endif
    return lret;
}

int CioctrlserverDlg::__GetCheck(UINT id)
{
    CButton* pBtn=NULL;

    pBtn = (CButton*)this->GetDlgItem(id);
    return pBtn->GetCheck();
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

void CioctrlserverDlg::__GetItemText(UINT id,CString & str)
{
    CEdit* pEdt=NULL;

    pEdt = (CEdit*)this->GetDlgItem(id);
    assert(pEdt);
    pEdt->GetWindowText(str);
    return ;
}

void CioctrlserverDlg::OnStart()
{
    char *pDllAnsi=NULL,*pExeAnsi=NULL,*pParamAnsi=NULL;
    int dllansisize=0,exeansisize=0,paramansisize=0;
    char *pCommandAnsi=NULL;
    char *pPartDll=NULL;
    int commandsize=0;
    CString errstr;
    int ret;
    int pid=0;
    CString istr;
    unsigned long bufsectsize,bufnum,waittime,listenport;
    BOOL bret;
    int hidecursor=0,enablesetcursorpos=0;

    this->__GetItemText(IDC_EDT_EXE,this->m_strExe);
    this->__GetItemText(IDC_EDT_PARAM,this->m_strParam);
    this->__GetItemText(IDC_EDT_DLL,this->m_strDll);

    if(this->m_strExe.GetLength() == 0)
    {
        errstr.Format(TEXT("Exe Must choose"));
        this->MessageBox((LPCTSTR)errstr,TEXT("Error"),MB_OK);
        ret = ERROR_INVALID_PARAMETER;
        goto fail;
    }

    if(this->m_strDll.GetLength() == 0)
    {
        errstr.Format(TEXT("Dll Must choose"));
        this->MessageBox((LPCTSTR)errstr,TEXT("Error"),MB_OK);
        ret = ERROR_INVALID_PARAMETER;
        goto fail;
    }
#ifdef _UNICODE
    ret = UnicodeToAnsi((wchar_t*)((LPCWSTR)this->m_strExe),&pExeAnsi,&exeansisize);
    if(ret < 0)
    {
        ret = LAST_ERROR_CODE();
        goto fail;
    }
    ret = UnicodeToAnsi((wchar_t*)((LPCWSTR)this->m_strDll),&pDllAnsi,&dllansisize);
    if(ret < 0)
    {
        ret = LAST_ERROR_CODE();
        goto fail;
    }

    if(this->m_strParam.GetLength() > 0)
    {
        ret = UnicodeToAnsi((wchar_t*)((LPCWSTR)this->m_strParam),&pParamAnsi,&paramansisize);
        if(ret < 0)
        {
            ret = LAST_ERROR_CODE();
            goto fail;
        }
    }
#else
    pExeAnsi = (LPCSTR)this->m_strExe;
    pDllAnsi = (LPCSTR)this->m_strDll;
    if(this->m_strParam.GetLength() != 0)
    {
        pParamAnsi = (LPCSTR)this->m_strParam;
    }
#endif

    hidecursor = this->__GetCheck(IDC_CHK_HIDECURSOR);
    enablesetcursorpos = this->__GetCheck(IDC_CHK_SETCURSORPOS);

    commandsize = strlen(pExeAnsi) + 1;
    if(pParamAnsi)
    {
        commandsize += 2;
        commandsize += strlen(pParamAnsi) + 1;
    }

    pCommandAnsi = (char*)calloc(1,commandsize);
    if(pCommandAnsi == NULL)
    {
        ret = LAST_ERROR_CODE();
        goto fail;
    }

    strncpy_s(pCommandAnsi,commandsize,pExeAnsi,_TRUNCATE);
    if(pParamAnsi)
    {
        strncat_s(pCommandAnsi,commandsize," ",_TRUNCATE);
        strncat_s(pCommandAnsi,commandsize,pParamAnsi,_TRUNCATE);
    }

    pPartDll = strrchr(pDllAnsi,'\\');
    if(pPartDll)
    {
        pPartDll += 1;
    }
    else
    {
        pPartDll = pDllAnsi;
    }
    bufnum = this->__ItemAtoi(IDC_EDT_BUFNUM,10);
    bufsectsize = this->__ItemAtoi(IDC_EDT_BUFSIZE,16);
    waittime = this->__ItemAtoi(IDC_EDT_WAITTIME,10);
    listenport = this->__ItemAtoi(IDC_EDT_PORT,10);

    if(bufnum == 0)
    {
        ret = ERROR_INVALID_PARAMETER;
        errstr.Format(TEXT("bufnum == 0"));
        this->MessageBox((LPCTSTR)errstr,TEXT("Error"),MB_OK);
        goto fail;
    }

    if(bufsectsize < 256)
    {
        ret = ERROR_INVALID_PARAMETER;
        errstr.Format(TEXT("bufsectsize(%d) < 256"),bufsectsize);
        this->MessageBox((LPCTSTR)errstr,TEXT("Error"),MB_OK);
        goto fail;
    }

    if(waittime == 0)
    {
        ret = ERROR_INVALID_PARAMETER;
        errstr.Format(TEXT("waittime == 0"));
        this->MessageBox((LPCTSTR)errstr,TEXT("Error"),MB_OK);
        goto fail;
    }

    if(listenport == 0 || listenport >= (1 << 16))
    {
        ret = ERROR_INVALID_PARAMETER;
        errstr.Format(TEXT("listen port(%d) not valid"),listenport);
        this->MessageBox((LPCTSTR)errstr,TEXT("Error"),MB_OK);
        goto fail;
    }

    /*to stop  the control for the next control*/
    this->__StopControl();

    /*now we should start for the command*/
    ret = LoadInsert(NULL,pCommandAnsi,pDllAnsi,pPartDll);
    if(ret < 0)
    {
        ret = LAST_ERROR_CODE();
        errstr.Format(TEXT("Load Insert (%s) Dll(%s) (%s) Error(%d)\n"),pCommandAnsi,pDllAnsi,pPartDll,ret);
        this->MessageBox((LPCTSTR)errstr,TEXT("Error"),MB_OK);
        goto fail;
    }
    pid = ret;

    Sleep(waittime*1000);

    this->m_hProc = OpenProcess(PROCESS_VM_OPERATION | PROCESS_VM_READ | PROCESS_VM_WRITE | PROCESS_QUERY_INFORMATION | PROCESS_CREATE_THREAD, FALSE, pid);
    if(this->m_hProc == NULL)
    {
        ret = LAST_ERROR_CODE();
        errstr.Format(TEXT("Could not open processid(%d) Error(%d)"),pid,ret);
        this->MessageBox((LPCTSTR)errstr,TEXT("Error"),MB_OK);
        goto fail;
    }

    this->m_pIoController = new CIOController();
    bret = this->m_pIoController->Start(this->m_hProc,bufnum,bufsectsize);
    if(!bret)
    {
        ret = LAST_ERROR_CODE();
        errstr.Format(TEXT("Could Start(%d:%d) Error(%d)"),bufnum,bufsectsize,ret);
        this->MessageBox((LPCTSTR)errstr,TEXT("Error"),MB_OK);
        goto fail;
    }

    ret = this->__StartAccSocket(listenport);
    if(ret < 0)
    {
        ret = LAST_ERROR_CODE();
        errstr.Format(TEXT("Could Listen(%d) Error(%d)"),listenport,ret);
        this->MessageBox((LPCTSTR)errstr,TEXT("Error"),MB_OK);
        goto fail;
    }

    ret = StartThreadControl(&(this->m_ThreadControl),CioctrlserverDlg::HandleSocketThread,this,1);
    if(ret < 0)
    {
        ret = LAST_ERROR_CODE();
        errstr.Format(TEXT("Start Thread Error(%d)"),ret);
        this->MessageBox((LPCTSTR)errstr,TEXT("Error"),MB_OK);
        goto fail;
    }


    bret = this->m_pIoController->EnableSetCursorPos(enablesetcursorpos ? TRUE : FALSE);
    if(!bret)
    {
        ret = LAST_ERROR_CODE();
        errstr.Format(TEXT("%s setcursor Error(%d)"),enablesetcursorpos ? TEXT("Enable") : TEXT("Disable") , ret);
        this->MessageBox((LPCTSTR)errstr,TEXT("Error"),MB_OK);
        goto fail;
    }

    bret = this->m_pIoController->HideCursor(hidecursor ? TRUE : FALSE);
    if(!bret)
    {
        ret = LAST_ERROR_CODE();
        errstr.Format(TEXT("%s hidecursor Error(%d)"),hidecursor ? TEXT("Enable") : TEXT("Disable") , ret);
        this->MessageBox((LPCTSTR)errstr,TEXT("Error"),MB_OK);
        goto fail;
    }


    /*all is ok*/
    SetLastError(0);
    return ;
fail:
    if(pCommandAnsi)
    {
        free(pCommandAnsi);
    }
    pCommandAnsi = NULL;
    commandsize = 0;
#ifdef _UNICODE
    UnicodeToAnsi(NULL,&pDllAnsi,&dllansisize);
    UnicodeToAnsi(NULL,&pExeAnsi,&exeansisize);
    UnicodeToAnsi(NULL,&pParamAnsi,&paramansisize);
#endif
    this->__StopControl();
    SetLastError(ret);
    return ;
}


void CioctrlserverDlg::__StopControl()
{
    StopThreadControl(&(this->m_ThreadControl));
    if(this->m_pIoController)
    {
        delete this->m_pIoController;
    }
    this->m_pIoController = NULL;

    if(this->m_Readsock != INVALID_SOCKET)
    {
        closesocket(this->m_Readsock);
    }
    this->m_Readsock = INVALID_SOCKET;
    if(this->m_Accsock != INVALID_SOCKET)
    {
        closesocket(this->m_Accsock);
    }
    this->m_Accsock = INVALID_SOCKET;
    if(this->m_hProc)
    {
        CloseHandle(this->m_hProc);
    }
    this->m_hProc = NULL;

    return ;
}


void CioctrlserverDlg::__CloseAccSocket()
{
    if(this->m_Accsock != INVALID_SOCKET)
    {
        closesocket(this->m_Accsock);
    }
    this->m_Accsock = INVALID_SOCKET;
    return ;
}

int CioctrlserverDlg::__StartAccSocket(int port)
{
    int ret;
    struct sockaddr_in saddr;
    u_long nonblock;
    assert(this->m_Accsock == INVALID_SOCKET);

    this->m_Accsock = socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);

    if(this->m_Accsock == INVALID_SOCKET)
    {
        ret = WSAGetLastError() ? WSAGetLastError() : 1;
        ERROR_INFO("Socket Error(%d)\n",ret);
        goto fail;
    }
    memset(&saddr,0,sizeof(saddr));
    saddr.sin_family = AF_INET;
    saddr.sin_addr.s_addr = inet_addr("0.0.0.0");
    saddr.sin_port = htons(port);

    ret = bind(this->m_Accsock,(struct sockaddr*)&saddr,sizeof(saddr));
    if(ret == SOCKET_ERROR)
    {
        ret = WSAGetLastError() ? WSAGetLastError() : 1;
        ERROR_INFO("Bind port(%d) Error(%d)\n",port,ret);
        goto fail;
    }

    ret = listen(this->m_Accsock,5);
    if(ret == SOCKET_ERROR)
    {
        ret = WSAGetLastError() ? WSAGetLastError() : 1;
        ERROR_INFO("Listen Error(%d)\n",ret);
        goto fail;
    }

#if 1
    nonblock = 1;
    ret= ioctlsocket(this->m_Accsock,FIONBIO,&nonblock);
    if(ret != 0)
    {
        ret = WSAGetLastError() ? WSAGetLastError() : 1;
        ERROR_INFO("Nonblock Set Error(%d)\n",ret);
        goto fail;
    }
#endif

    SetLastError(0);
    return 0;

fail:
    this->__CloseAccSocket();
    SetLastError(ret);
    return -ret;
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
    return 0;
}

void CioctrlserverDlg::OnAttach()
{
    CString errstr;
    int ret;
    unsigned long bufsectsize,bufnum,listenport,pid;
    BOOL bret;
    int hidecursor=0,enablesetcursorpos=0;


    bufnum = this->__ItemAtoi(IDC_EDT_BUFNUM,10);
    pid = this->__ItemAtoi(IDC_EDT_ATTACHPID,10);
    bufsectsize = this->__ItemAtoi(IDC_EDT_BUFSIZE,16);
    listenport = this->__ItemAtoi(IDC_EDT_PORT,10);



    if(pid == 0)
    {
        ret = ERROR_INVALID_PARAMETER;
        errstr.Format(TEXT("Pid == 0"));
        this->MessageBox((LPCTSTR)errstr,TEXT("Error"),MB_OK);
        goto fail;
    }



    if(bufnum == 0)
    {
        ret = ERROR_INVALID_PARAMETER;
        errstr.Format(TEXT("bufnum == 0"));
        this->MessageBox((LPCTSTR)errstr,TEXT("Error"),MB_OK);
        goto fail;
    }

    if(bufsectsize < 256)
    {
        ret = ERROR_INVALID_PARAMETER;
        errstr.Format(TEXT("bufsectsize(%d) < 256"),bufsectsize);
        this->MessageBox((LPCTSTR)errstr,TEXT("Error"),MB_OK);
        goto fail;
    }

    if(listenport == 0 || listenport >= (1 << 16))
    {
        ret = ERROR_INVALID_PARAMETER;
        errstr.Format(TEXT("listen port(%d) not valid"),listenport);
        this->MessageBox((LPCTSTR)errstr,TEXT("Error"),MB_OK);
        goto fail;
    }

    hidecursor = this->__GetCheck(IDC_CHK_HIDECURSOR);
    enablesetcursorpos = this->__GetCheck(IDC_CHK_SETCURSORPOS);

    /*to stop  the control for the next control*/
    this->__StopControl();

    /*now we should start for the command*/

    this->m_hProc = OpenProcess(PROCESS_VM_OPERATION | PROCESS_VM_READ | PROCESS_VM_WRITE | PROCESS_QUERY_INFORMATION | PROCESS_CREATE_THREAD, FALSE, pid);
    if(this->m_hProc == NULL)
    {
        ret = LAST_ERROR_CODE();
        if(ret != ERROR_ACCESS_DENIED)
        {
            errstr.Format(TEXT("Could not open processid(%d) Error(%d)"),pid,ret);
            this->MessageBox((LPCTSTR)errstr,TEXT("Error"),MB_OK);
            goto fail;
        }

        /*now we try again ,so do the debug privilege promotion*/
        ret = EnableCurrentDebugPriv();
        if(ret < 0)
        {
            errstr.Format(TEXT("Can not Enable Debug Privilege Error(%d)"),ret);
            this->MessageBox((LPCTSTR)errstr,TEXT("Error"),MB_OK);
            goto fail;
        }
        this->m_hProc = OpenProcess(PROCESS_VM_OPERATION | PROCESS_VM_READ | PROCESS_VM_WRITE | PROCESS_QUERY_INFORMATION | PROCESS_CREATE_THREAD, FALSE, pid);
        if(this->m_hProc == NULL)
        {
            ret = LAST_ERROR_CODE();
            errstr.Format(TEXT("Could not open processid(%d) On Debug Privilege Error(%d)"),pid,ret);
            this->MessageBox((LPCTSTR)errstr,TEXT("Error"),MB_OK);
            goto fail;
        }
    }

    this->m_pIoController = new CIOController();
    bret = this->m_pIoController->Start(this->m_hProc,bufnum,bufsectsize);
    if(!bret)
    {
        ret = LAST_ERROR_CODE();
        errstr.Format(TEXT("Could Start(%d:%d) Error(%d)"),bufnum,bufsectsize,ret);
        this->MessageBox((LPCTSTR)errstr,TEXT("Error"),MB_OK);
        goto fail;
    }

    ret = this->__StartAccSocket(listenport);
    if(ret < 0)
    {
        ret = LAST_ERROR_CODE();
        errstr.Format(TEXT("Could Listen(%d) Error(%d)"),listenport,ret);
        this->MessageBox((LPCTSTR)errstr,TEXT("Error"),MB_OK);
        goto fail;
    }

    ret = StartThreadControl(&(this->m_ThreadControl),CioctrlserverDlg::HandleSocketThread,this,1);
    if(ret < 0)
    {
        ret = LAST_ERROR_CODE();
        errstr.Format(TEXT("Start Thread Error(%d)"),ret);
        this->MessageBox((LPCTSTR)errstr,TEXT("Error"),MB_OK);
        goto fail;
    }

    bret = this->m_pIoController->EnableSetCursorPos(enablesetcursorpos ? TRUE : FALSE);
    if(!bret)
    {
        ret = LAST_ERROR_CODE();
        errstr.Format(TEXT("%s setcursor Error(%d)"),enablesetcursorpos ? TEXT("Enable") : TEXT("Disable") , ret);
        this->MessageBox((LPCTSTR)errstr,TEXT("Error"),MB_OK);
        goto fail;
    }

    bret = this->m_pIoController->HideCursor(hidecursor ? TRUE : FALSE);
    if(!bret)
    {
        ret = LAST_ERROR_CODE();
        errstr.Format(TEXT("%s hidecursor Error(%d)"),hidecursor ? TEXT("Enable") : TEXT("Disable") , ret);
        this->MessageBox((LPCTSTR)errstr,TEXT("Error"),MB_OK);
        goto fail;
    }

    /*all is ok*/
    SetLastError(0);
    return ;
fail:
    this->__StopControl();
    SetLastError(ret);
    return ;
}


DWORD CioctrlserverDlg::__SocketThread()
{
    HANDLE *pWaitHandle=NULL;
    int waitnum=0;
    int waitsize=3;
    int ret,res;
    DWORD dret;
    SOCKET rsock=INVALID_SOCKET;
    struct sockaddr_in saddr;
    int socklen;
    DEVICEEVENT devevent;
    char* pBuf;
    int hasrecv=0;
    u_long nonblock=1;
    BOOL bret;
    pWaitHandle =(HANDLE*) calloc(waitsize,sizeof(*pWaitHandle));
    if(pWaitHandle == NULL)
    {
        ret = LAST_ERROR_CODE();
        goto out;
    }
    pWaitHandle[1] = WSA_INVALID_EVENT;
    pWaitHandle[2] = WSA_INVALID_EVENT;

    pWaitHandle[0] = this->m_ThreadControl.exitevt;
    pWaitHandle[1] = WSACreateEvent();
    if(pWaitHandle[1] == WSA_INVALID_EVENT)
    {
        ret = WSAGetLastError() ? WSAGetLastError() : 1;
        ERROR_INFO("Create Accept Event Error(%d)\n",ret);
        goto out;
    }
    ret = WSAEventSelect(this->m_Accsock,pWaitHandle[1],FD_ACCEPT|FD_CLOSE);
    if(ret == SOCKET_ERROR)
    {
        ret = WSAGetLastError() ? WSAGetLastError() : 1;
        ERROR_INFO("Select Accept Event Error(%d)\n",ret);
        goto out;
    }
    waitnum = 2;
    DEBUG_INFO("Start Thread Control\n");

    while(this->m_ThreadControl.running)
    {
        //DEBUG_INFO("waitnum %d\n",waitnum);
        dret = WaitForMultipleObjects(waitnum,pWaitHandle,FALSE,INFINITE);
        //DEBUG_INFO("wait return %d waitnum %d\n",dret,waitnum);
        if(dret == WAIT_OBJECT_0)
        {
            ERROR_INFO("NOTIFY EXIT\n");
        }
        else if(dret ==(WAIT_OBJECT_0+1))
        {
            bret = WSAResetEvent(pWaitHandle[1]);
            if(!bret)
            {
                res = WSAGetLastError() ? WSAGetLastError() : 1;
                ERROR_INFO("Reset Event Error(%d)\n",res);
            }
            socklen = sizeof(saddr);
            rsock = accept(this->m_Accsock,(struct sockaddr*)&saddr,&socklen);
            if(rsock == INVALID_SOCKET)
            {
                ret = WSAGetLastError() ? WSAGetLastError() : 1;
                ERROR_INFO("accept Error(%d)\n",ret);
                continue;
            }
            DEBUG_INFO("accept return %d\n",rsock);
            /*now it is coming ,so we should close the handle*/
            if(pWaitHandle[2] != WSA_INVALID_EVENT)
            {
                /*close the old socket and event*/
                WSACloseEvent(pWaitHandle[2]);
            }
            pWaitHandle[2] = WSA_INVALID_EVENT;
            hasrecv = 0;
            waitnum = 2;
            /*now to test if the socket close*/
            if(this->m_Readsock != INVALID_SOCKET)
            {
                closesocket(this->m_Readsock);
            }
            DEBUG_INFO("Accept Socket(%d)\n",rsock);

            this->m_Readsock = rsock;
            rsock = INVALID_SOCKET;
            nonblock = 1;
            ret= ioctlsocket(this->m_Readsock, FIONBIO, &nonblock);
            if(ret != NO_ERROR)
            {
                ret = WSAGetLastError() ? WSAGetLastError() : 1;
                ERROR_INFO("Set Nonblock Error(%d)\n",ret);
                closesocket(this->m_Readsock);
                this->m_Readsock = INVALID_SOCKET;
                waitnum = 2;
                continue;
            }

            pWaitHandle[2] = WSACreateEvent();
            if(pWaitHandle[2] == WSA_INVALID_EVENT)
            {
                ret = WSAGetLastError() ? WSAGetLastError() : 1;
                ERROR_INFO("Create Event Error(%d)\n",ret);
                closesocket(this->m_Readsock);
                this->m_Readsock = INVALID_SOCKET;
                waitnum = 2;
                continue;
            }
            ret = WSAEventSelect(this->m_Readsock,pWaitHandle[2],FD_READ|FD_CLOSE);
            if(ret == SOCKET_ERROR)
            {
                ret = WSAGetLastError() ? WSAGetLastError() : 1;
                ERROR_INFO("Select Event Error(%d)\n",ret);
                closesocket(this->m_Readsock);
                this->m_Readsock = INVALID_SOCKET;
                WSACloseEvent(pWaitHandle[2]);
                pWaitHandle[2] = WSA_INVALID_EVENT;
                waitnum = 2;
                continue;
            }
            /*now all is ok ,so wait for 3 */
            waitnum = 3;
        }
        else if(dret == (WAIT_OBJECT_0+2))
        {
            bret = WSAResetEvent(pWaitHandle[2]);
            if(!bret)
            {
                res = WSAGetLastError() ? WSAGetLastError() : 1;
                ERROR_INFO("Reset Event Error(%d)\n",res);
            }

            pBuf = (char*)&devevent;
            pBuf += hasrecv;
            ret = recv(this->m_Readsock,pBuf,(sizeof(devevent)-hasrecv),0);
            if(ret == 0 || ret == SOCKET_ERROR)
            {
                res = WSAGetLastError() ? WSAGetLastError() : 1;
                ERROR_INFO("Read return (%d) error (%d)\n",ret,res);
                closesocket(this->m_Readsock);
                this->m_Readsock = INVALID_SOCKET;
                WSACloseEvent(pWaitHandle[2]);
                pWaitHandle[2] = WSA_INVALID_EVENT;
                waitnum = 2;
                continue;
            }

            hasrecv += ret;
            if(hasrecv == sizeof(devevent))
            {
                /*receive one device event so ,put it to the handle*/
                assert(this->m_pIoController);
                this->m_pIoController->PushEvent(&devevent);
                hasrecv = 0;
            }
        }
        else if(dret == WAIT_FAILED)
        {
            ret = LAST_ERROR_CODE();
            ERROR_INFO("Wait Error(%d)\n",ret);
            goto out;
        }
    }


    /*just ok exit*/
    ret = 0;

out:

    if(rsock != INVALID_SOCKET)
    {
        closesocket(rsock);
    }
    rsock = INVALID_SOCKET;

    if(pWaitHandle)
    {
        if(pWaitHandle[1] != WSA_INVALID_EVENT)
        {
            WSACloseEvent(pWaitHandle[1]);
        }
        pWaitHandle[1] = WSA_INVALID_EVENT;
        if(pWaitHandle[2] != WSA_INVALID_EVENT)
        {
            WSACloseEvent(pWaitHandle[2]);
        }
        pWaitHandle[2] = WSA_INVALID_EVENT;
        free(pWaitHandle);
    }
    pWaitHandle = NULL;
    waitnum = 0;
    waitsize = 0;
    SetLastError(ret);
    this->m_ThreadControl.exited = 1;
    return ret;
}


DWORD CioctrlserverDlg::HandleSocketThread(LPVOID pParam)
{
    CioctrlserverDlg* pThis = (CioctrlserverDlg*)pParam;
    return pThis->__SocketThread();
}

void CioctrlserverDlg::OnSetCursorPosChk()
{
    int enablesetcursorpos=0;
    BOOL bret;
    CString errstr;
    int ret;
    if(this->m_pIoController)
    {
        enablesetcursorpos = this->__GetCheck(IDC_CHK_SETCURSORPOS);

        bret = this->m_pIoController->EnableSetCursorPos(enablesetcursorpos ? TRUE : FALSE);
        if(!bret)
        {
            ret = LAST_ERROR_CODE();
            errstr.Format(TEXT("%s setcursor Error(%d)"),enablesetcursorpos ? TEXT("Enable") : TEXT("Disable") , ret);
            this->MessageBox((LPCTSTR)errstr,TEXT("Error"),MB_OK);
        }
    }

    return ;
}

void CioctrlserverDlg::OnHideCursorChk()
{
    int hidecursor=0;
    BOOL bret;
    CString errstr;
    int ret;
    if(this->m_pIoController)
    {
        hidecursor = this->__GetCheck(IDC_CHK_HIDECURSOR);
        bret = this->m_pIoController->HideCursor(hidecursor ? TRUE : FALSE);
        if(!bret)
        {
            ret = LAST_ERROR_CODE();
            errstr.Format(TEXT("%s hidecursor Error(%d)"),hidecursor ? TEXT("Enable") : TEXT("Disable") , ret);
            this->MessageBox((LPCTSTR)errstr,TEXT("Error"),MB_OK);
        }
    }

    return ;
}


int CioctrlserverDlg::__SaveBmpFile(char * fname,PVOID pInfo,UINT infolen,PVOID pData,UINT datalen)
{
    FILE *fp=NULL;
    int ret;
    int totallen = 0;
    BITMAPFILEHEADER bfheader;

    fopen_s(&fp,fname,"w");
    if(fp == NULL)
    {
        ret = LAST_ERROR_CODE();
        ERROR_INFO("can not open (%s) Error(%d)\n",fname,ret);
        goto fail;
    }

    ZeroMemory(&bfheader,sizeof(bfheader));
    bfheader.bfType = 'M' << 8 | 'B';
    bfheader.bfSize = sizeof(bfheader) + infolen + datalen;
    bfheader.bfOffBits = sizeof(bfheader) + infolen ;

    ret = fwrite(&bfheader,sizeof(bfheader),1,fp);
    if(ret != 1)
    {
        ret = LAST_ERROR_CODE();
        ERROR_INFO("write(%s)bfheader Error(%d)\n",fname,ret);
        goto fail;
    }
    totallen += sizeof(bfheader);

    ret = fwrite(pInfo,infolen,1,fp);
    if(ret != 1)
    {
        ret = LAST_ERROR_CODE();
        ERROR_INFO("write(%s)info Error(%d)\n",fname,ret);
        goto fail;
    }
    totallen += infolen;

    ret = fwrite(pData,datalen,1,fp);
    if(ret != 1)
    {
        ret = LAST_ERROR_CODE();
        ERROR_INFO("write(%s)data Error(%d)\n",fname,ret);
        goto fail;
    }

    totallen += datalen;
    if(fp)
    {
        fclose(fp);
    }
    fp = NULL;

    return totallen;
fail:
    if(fp)
    {
        fclose(fp);
    }
    fp = NULL;
    SetLastError(ret);
    return -ret;
}

void CioctrlserverDlg::OnSaveCursorBmp()
{
    CString basename;
    CString errstr;
    char* basenameansi=NULL;
#ifdef _UNICODE
    int basenamesize=0;
#endif
    std::auto_ptr<char> pMaskFile2(new char[256]),pColorFile2(new char[256]);
    char *pMaskFile=pMaskFile2.get(),*pColorFile = pColorFile2.get();
    int ret;
    BOOL bret;
    PVOID pMaskBuffer=NULL,pMaskInfo=NULL,pColorBuffer=NULL,pColorInfo=NULL;
    UINT maskbufsize=0,maskinfosize=0,colorbufsize=0,colorinfosize=0;
    UINT maskbuflen,maskinfolen,colorbuflen,colorinfolen;

    this->__GetItemText(IDC_EDT_SAVECURSORBMP,basename);
    if(basename.GetLength() == 0)
    {
        ret = ERROR_INVALID_PARAMTER;
        errstr.Format(TEXT("Please Set savebmp name"));
        this->MessageBox((LPCTSTR)errstr,TEXT("Error"),MB_OK);
        goto fail ;
    }
    if(this->m_pIoController == NULL)
    {
        ret = ERROR_BAD_ENVIRONMENT;
        errstr.Format(TEXT("Inject Exe Not Running"));
        this->MessageBox((LPCTSTR)errstr,TEXT("Error"),MB_OK);
        goto fail ;
    }

    bret = this->m_pIoController->GetCursorBitmap(&pColorInfo,&colorinfosize,&colorinfolen,
            &pColorBuffer,&colorbufsize,&colorbuflen,
            &pMaskInfo,&maskinfosize,&maskbuflen,
            &pMaskBuffer,&maskbufsize,&maskbuflen);
    if(!bret)
    {
        ret = LAST_ERROR_CODE();
        errstr.Format(TEXT("Can not Get(%d) cursor Error(%d)"),GetProcessId(this->m_hProc),ret);
        this->MessageBox((LPCTSTR)errstr,TEXT("Error"),MB_OK);
        goto fail ;
    }


#ifdef _UNICODE
    ret = UnicodeToAnsi((LPCWSTR)basename,&basenameansi,&basenamesize);
    if(ret < 0)
    {
        ret = LAST_ERROR_CODE();
        errstr.Format(TEXT("could not format basename Error(%d)"),ret);
        this->MessageBox((LPCTSTR)errstr,TEXT("Error"),MB_OK);
        goto fail ;
    }
#else
    basenameansi = (LPCSTR)basename;
#endif

    _snprintf_s(pMaskFile,256,_TRUNCATE,"%smask.bmp",basenameansi);
    _snprintf_s(pColorFile,256,_TRUNCATE,"%scolor.bmp",basenameansi);

    ret = this->__SaveBmpFile(pMaskFile,pMaskInfo,maskinfolen,pMaskBuffer,maskbuflen);
    if(ret < 0)
    {
        ret = LAST_ERROR_CODE();
        errstr.Format(TEXT("Write(%s) File Error(%d)"),pMaskFile,ret);
        this->MessageBox((LPCTSTR)errstr,TEXT("Error"),MB_OK);
        goto fail ;
    }

	ret = this->__SaveBmpFile(pColorFile,pColorInfo,colorinfolen,pColorBuffer,colorbuflen);
    if(ret < 0)
    {
        ret = LAST_ERROR_CODE();
        errstr.Format(TEXT("Write(%s) File Error(%d)"),pColorFile,ret);
        this->MessageBox((LPCTSTR)errstr,TEXT("Error"),MB_OK);
        goto fail ;
    }

#ifdef _UNICODE
    UnicodeToAnsi(NULL,&basenameansi,&basenamesize);
#endif
    if(pColorInfo)
    {
        free(pColorInfo);
    }
    pColorInfo = NULL;
    if(pColorBuffer)
    {
        free(pColorBuffer);
    }
    pColorBuffer = NULL;
    if(pMaskInfo)
    {
        free(pMaskInfo);
    }
    pMaskInfo = NULL;
    if(pMaskBuffer)
    {
        free(pMaskBuffer);
    }
    pMaskBuffer = NULL;
    return ;
fail:
#ifdef _UNICODE
    UnicodeToAnsi(NULL,&basenameansi,&basenamesize);
#endif
    if(pColorInfo)
    {
        free(pColorInfo);
    }
    pColorInfo = NULL;
    if(pColorBuffer)
    {
        free(pColorBuffer);
    }
    pColorBuffer = NULL;
    if(pMaskInfo)
    {
        free(pMaskInfo);
    }
    pMaskInfo = NULL;
    if(pMaskBuffer)
    {
        free(pMaskBuffer);
    }
    pMaskBuffer = NULL;
    SetLastError(ret);
    return ;

}




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
#ifdef _UNICODE
    wchar_t *pEndPtr;
#else
    char* pEndPtr;
#endif

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

    commandsize = strlen(pExeAnsi) + 1;
    if(pParamAnsi)
    {
        commandsize += 2;
        commandsize += strlen(pParamAnsi) + 1;
    }

    pCommandAnsi = calloc(1,commandsize);
    if(pCommandAnsi == NULL)
    {
        ret = LAST_ERROR_CODE();
        goto fail;
    }

    strncpy_s(pCommandAnsi,commandsize,pExeAnsi,_TRUNCATE);
    if(pParamAnsi)
    {
        strncat_s(pCommandAnsi,commandsize," ");
        strncat_s(pCommandAnsi,commandsize,pParamAnsi);
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
    bufsectsize = this->__ItemAtoi(IDC_EDT_BUFSECTSIZE,16);
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
        ERROR_INFO("Load Insert (%s) Dll(%s) (%s) Error(%d)\n",pCommandAnsi,pDllAnsi,pPartDll,ret);
        goto fail;
    }






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
    struct sockaddr saddr;
    int socklen;
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

    ret = bind(this->m_Accsock,&saddr,sizeof(saddr));
    if(ret == SOCKET_ERROR)
    {
        ret = WSAGetLastError() ? WSAGetLastError() : 1;
        ERROR_INFO("Bind port(%d) Error(%d)\n",port,ret);
        goto fail;
    }

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


DWORD CioctrlserverDlg::__SocketThread()
{
    HANDLE *pWaitHandle=NULL;
    int waitnum=0;
    int waitsize=3;
    int ret;
    DWORD dret;
    SOCKET rsock=INVALID_SOCKET;
    struct sockaddr_in saddr;
    int socklen;
    DEVICEEVENT devevent;
    char* pBuf;
    int hasrecv=0;
    pWaitHandle = calloc(waitsize,sizeof(*pWaitHandle));
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

    while(this->m_ThreadControl.running)
    {
        dret = WaitForMulitpleObjectEx(waitnum,pWaitHandle,FALSE,INFINITE,TRUE);
        if(dret == WAIT_OJBECT_0)
        {
            ERROR_INFO("NOTIFY EXIT\n");
        }
        else if(dret == WAIT_OBJECT_1)
        {
            socklen = sizeof(saddr);
            rsock = accept(this->m_Accsock,&saddr,&socklen);
            if(rsock == INVALID_SOCKET)
            {
                ret = WSAGetLastError() ? WSAGetLastError() : 1;
                ERROR_INFO("accept Error(%d)\n",ret);
                continue;
            }
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

            this->m_Readsock = rsock;
            rsock = INVALID_SOCKET;

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
            ret = WSASelectEvent(this->m_Readsock,pWaitHandle[2],FD_READ|FD_CLOSE);
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
        else if(dret == WAIT_OBJECT_3)
        {
            pBuf = (char*)&devevent;
            pBuf += hasrecv;
            ret = recv(this->m_Readsock,pBuf,(sizeof(devevent)-hasrecv),MSG_PEEK);
            if(ret == 0 || ret == SOCKET_ERROR)
            {
                ret = WSAGetLastError() ? WSAGetLastError() : 1;
                ERROR_INFO("read error (%d)\n",ret);
                closesocket(this->m_Readsock);
                this->m_Readsock = INVALID_SOCKET;
                WSACloseEvent(pWaitHandle[2]);
                pWaitHandle[2] = WSA_INVALID_EVENT;
                waitnum = 2;
                continue;
            }
            ret = recv(this->m_Readsock,pBuf,(sizeof(devevent)-hasrecv),0);
            if(ret == 0 || ret == SOCKET_ERROR)
            {
                ret = WSAGetLastError() ? WSAGetLastError() : 1;
                ERROR_INFO("read error (%d)\n",ret);
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

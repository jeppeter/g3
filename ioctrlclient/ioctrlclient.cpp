// ioctrlclient.cpp : 定义应用程序的入口点。
//

#include "stdafx.h"
#include "ioctrlclient.h"
#include <output_debug.h>
#include <iocapcommon.h>
#include <memory>
#include <vector>
#include <Winsock.h>
#include <uniansi.h>
#define  DIRECTINPUT_VERSION   0x800
#include <dinput.h>

#pragma comment(lib,"dinput8.lib")
#pragma comment(lib,"dxguid.lib")
#pragma comment(lib,"Ws2_32.lib")

#define MAX_LOADSTRING 100

#define WM_SOCKET   (WM_USER + 1)
#define SOCKET_TIMEOUT   3000
#define SOCKET_TM_EVENTID 10003

static char g_Host[256];
static int g_Port;
static int g_EscapeKey=DIK_RCONTROL;
static SOCKET g_Socket=INVALID_SOCKET;
static int g_Connected=0;
static int g_writesize=0;
static HWND g_hWnd=NULL;
static std::vector<DEVICEEVENT> st_DevEvent;

// 全局变量:
HINSTANCE hInst;								// 当前实例
TCHAR szTitle[MAX_LOADSTRING];					// 标题栏文本
TCHAR szWindowClass[MAX_LOADSTRING];			// 主窗口类名

// 此代码模块中包含的函数的前向声明:
ATOM				MyRegisterClass(HINSTANCE hInstance);
BOOL				InitInstance(HINSTANCE, int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK	About(HWND, UINT, WPARAM, LPARAM);

BOOL UpdateCodeMessage()
{
    return TRUE;
}


int APIENTRY _tWinMain(_In_ HINSTANCE hInstance,
                       _In_opt_ HINSTANCE hPrevInstance,
                       _In_ LPTSTR    lpCmdLine,
                       _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // TODO:  在此放置代码。
    MSG msg;
    HACCEL hAccelTable;
    WSADATA wsaData;
    int ret;
    std::auto_ptr<TCHAR> pErrStr2(new TCHAR[256]);
    TCHAR *pErrStr=pErrStr2.get();

    hInst = hInstance;
    // 初始化全局字符串
    LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadString(hInstance, IDC_IOCTRLCLIENT, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // 执行应用程序初始化:
    if(!InitInstance(hInstance, nCmdShow))
    {
        return FALSE;
    }

    ret = WSAStartup(MAKEWORD(2, 2),&wsaData);
    if(ret != NO_ERROR)
    {
        ret = WSAGetLastError() ? WSAGetLastError() : 1;
#ifdef _UNICODE
        _snwprintf_s(pErrStr,256,_TRUNCATE,TEXT("Init Sock Error(%d)"),ret);
#else
        _snprintf_s(pErrStr,256,_TRUNCATE,"Init Sock Error(%d)",ret);
#endif
        ::MessageBox(g_hWnd,pErrStr,TEXT("Error"),MB_OK);
        return FALSE;
    }

    hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_IOCTRLCLIENT));

    ZeroMemory(&msg,sizeof(msg));
    // 主消息循环:
    while(msg.message != WM_QUIT)
    {
        if(PeekMessage(&msg,0,0,0,PM_REMOVE))
        {
            if(!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
            {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
        }
        UpdateCodeMessage();
    }

    WSACleanup();

    return (int) msg.wParam;
}



//
//  函数:  MyRegisterClass()
//
//  目的:  注册窗口类。
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEX wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style			= CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc	= WndProc;
    wcex.cbClsExtra		= 0;
    wcex.cbWndExtra		= 0;
    wcex.hInstance		= hInstance;
    wcex.hIcon			= LoadIcon(hInstance, MAKEINTRESOURCE(IDI_IOCTRLCLIENT));
    wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
    wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName	= MAKEINTRESOURCE(IDC_IOCTRLCLIENT);
    wcex.lpszClassName	= szWindowClass;
    wcex.hIconSm		= LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassEx(&wcex);
}

//
//   函数:  InitInstance(HINSTANCE, int)
//
//   目的:  保存实例句柄并创建主窗口
//
//   注释:
//
//        在此函数中，我们在全局变量中保存实例句柄并
//        创建和显示主程序窗口。
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
    HWND hWnd;

    hInst = hInstance; // 将实例句柄存储在全局变量中

    hWnd = CreateWindow(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
                        CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, NULL, NULL, hInstance, NULL);

    if(!hWnd)
    {
        return FALSE;
    }

    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);
    g_hWnd = hWnd;

    return TRUE;
}

#ifdef _UNICODE
int StringLength(wchar_t* pString)
{
    return wcslen(pString);
}
BOOL GetDialogItemString(HWND hwndDlg,int nIDDlgItem,wchar_t *pString,int count)
{
    HWND hCtrlItem=NULL;
    int ret;
    hCtrlItem = ::GetDlgItem(hwndDlg,nIDDlgItem);
    if(!hCtrlItem)
    {
        ret = LAST_ERROR_CODE();
        ERROR_INFO("Get <0x%08x>:%d DlgItem Error(%d)\n",hwndDlg,nIDDlgItem,ret);
        SetLastError(ret);
        return FALSE;
    }

    ZeroMemory(pString,count*sizeof(pString[0]));
    SetLastError(0);
    ret = ::GetWindowText(hCtrlItem,pString,count);
    if(ret == 0 && GetLastError() != 0)
    {
        ret = LAST_ERROR_CODE();
        ERROR_INFO("Get <0x%08x>:%d WindowText Error(%d)\n",hwndDlg,nIDDlgItem,ret);
        SetLastError(ret);
    }

    return TRUE;

}

BOOL SetDialogItemString(HWND hwndDlg,int nIDDlgItem,wchar_t *pString)
{
    HWND hCtrlItem=NULL;
    int ret;
    BOOL bret;
    hCtrlItem = ::GetDlgItem(hwndDlg,nIDDlgItem);
    if(!hCtrlItem)
    {
        ret = LAST_ERROR_CODE();
        ERROR_INFO("Get <0x%08x>:%d DlgItem Error(%d)\n",hwndDlg,nIDDlgItem,ret);
        SetLastError(ret);
        return FALSE;
    }

    bret = SetWindowText(hCtrlItem,pString);
    if(!bret)
    {
        ret = LAST_ERROR_CODE();
        ERROR_INFO("Set <0x%08x>:%d WindowText Error(%d)\n",hwndDlg,nIDDlgItem,ret);
        SetLastError(ret);
        return FALSE;
    }

    return TRUE;
}

BOOL InsertComboString(HWND hwndDlg,int nIDDlgItem,int idx,wchar_t *pString)
{
    HWND hCtrlItem=NULL;
    int ret;
    hCtrlItem = ::GetDlgItem(hwndDlg,nIDDlgItem);
    if(!hCtrlItem)
    {
        ret = LAST_ERROR_CODE();
        ERROR_INFO("Get <0x%08x>:%d DlgItem Error(%d)\n",hwndDlg,nIDDlgItem,ret);
        SetLastError(ret);
        return FALSE;
    }

    ret = SendMessage(hCtrlItem, CB_INSERTSTRING,idx,(LPARAM)pString);
    if(ret == CB_ERR)
    {
        ret = LAST_ERROR_CODE();
        ERROR_INFO("InsertString <0x%08x>:%d  Error(%d) CB_ERR(%d)\n",hwndDlg,nIDDlgItem,idx,ret,CB_ERR);
        SetLastError(ret);
        return FALSE;
    }

    return TRUE;
}




int SprintfString(wchar_t* pString ,int count,const wchar_t* pfmt,...)
{
    int ret;
    va_list ap;

    va_start(ap,pfmt);

    ret =  _vsnwprintf_s(pString,count,_TRUNCATE,pfmt,ap);

    DEBUG_INFO("%S\n",pString);
    return ret;
}


#else

int StringLength(char * pString)
{
    return strlen(pString);
}

BOOL GetDialogItemString(HWND hwndDlg,int nIDDlgItem,char *pString,int count)
{
    HWND hCtrlItem=NULL;
    int ret;
    hCtrlItem = ::GetDlgItem(hwndDlg,nIDDlgItem);
    if(!hCtrlItem)
    {
        ret = LAST_ERROR_CODE();
        ERROR_INFO("Get <0x%08x>:%d DlgItem Error(%d)\n",hwndDlg,nIDDlgItem,ret);
        SetLastError(ret);
        return FALSE;
    }

    ZeroMemory(pString,count*sizeof(pString[0]));
    SetLastError(0);
    ret = ::GetWindowText(hCtrlItem,pString,count);
    if(ret == 0 && GetLastError() != 0)
    {
        ret = LAST_ERROR_CODE();
        ERROR_INFO("Get <0x%08x>:%d WindowText Error(%d)\n",hwndDlg,nIDDlgItem,ret);
        SetLastError(ret);
    }

    return TRUE;
}

BOOL SetDialogItemString(HWND hwndDlg,int nIDDlgItem,char *pString)
{
    HWND hCtrlItem=NULL;
    int ret;
    BOOL bret;
    hCtrlItem = ::GetDlgItem(hwndDlg,nIDDlgItem);
    if(!hCtrlItem)
    {
        ret = LAST_ERROR_CODE();
        ERROR_INFO("Get <0x%08x>:%d DlgItem Error(%d)\n",hwndDlg,nIDDlgItem,ret);
        SetLastError(ret);
        return FALSE;
    }

    bret = SetWindowText(hCtrlItem,pString);
    if(!bret)
    {
        ret = LAST_ERROR_CODE();
        ERROR_INFO("Set <0x%08x>:%d WindowText Error(%d)\n",hwndDlg,nIDDlgItem,ret);
        SetLastError(ret);
        return FALSE;
    }

    return TRUE;
}




void SprintfString(wchar_t* pString ,int count,const wchar_t* pfmt,...)
{
    int ret;
    va_list ap;

    va_start(ap,fmt);

    ret = _vsnprintf_s(pString,count,_TRUNCATE,pfmt,ap);

    DEBUG_INFO("%s\n",pString);
    return ret;
}

#endif /*_UNICODE*/

BOOL SetComboSel(HWND hwndDlg,int nIDDlgItem,int idx)
{
    HWND hCtrlItem=NULL;
    int ret;
    hCtrlItem = ::GetDlgItem(hwndDlg,nIDDlgItem);
    if(!hCtrlItem)
    {
        ret = LAST_ERROR_CODE();
        ERROR_INFO("Get <0x%08x>:%d DlgItem Error(%d)\n",hwndDlg,nIDDlgItem,ret);
        SetLastError(ret);
        return FALSE;
    }

    ret = SendMessage(hCtrlItem,CB_SETCURSEL,idx,0);
    if(ret == CB_ERR)
    {
        ret = LAST_ERROR_CODE();
        SendMessage(hCtrlItem,CB_SETCURSEL,0,0);
        ERROR_INFO("SetCurSel <0x%08x>:%d Idx(%d) CB_ERR(%d)  Error(%d)\n",hwndDlg,nIDDlgItem,idx,CB_ERR,ret);
        SetLastError(ret);
        return FALSE;
    }

    return TRUE;
}


int GetComboSel(HWND hwndDlg,int nIDDlgItem)
{
    HWND hCtrlItem=NULL;
    int ret;
    hCtrlItem = ::GetDlgItem(hwndDlg,nIDDlgItem);
    if(!hCtrlItem)
    {
        ret = LAST_ERROR_CODE();
        ERROR_INFO("Get <0x%08x>:%d DlgItem Error(%d)\n",hwndDlg,nIDDlgItem,ret);
        SetLastError(ret);
        return -1;
    }

    ret = SendMessage(hCtrlItem,CB_GETCURSEL,0,0);
    if(ret == CB_ERR)
    {
        ret = LAST_ERROR_CODE();
        ERROR_INFO("GetCurSel <0x%08x>  Error(%d)\n",hwndDlg,nIDDlgItem,ret);
        SetLastError(ret);
        return -1;
    }

    return ret;
}


BOOL InitializeConnectDialog(HWND hwnd)
{
    BOOL bret;
    int ret;
#ifdef _UNICODE
    std::auto_ptr<wchar_t> pChar2(new wchar_t[256]);
    wchar_t* pChar = pChar2.get();
#else
    std::auto_ptr<char> pChar2(new char[256]);
    char* pChar = pChar2.get();
#endif

    SprintfString(pChar,256,TEXT("3391"));
    bret = SetDialogItemString(hwnd,IDC_EDT_PORT,pChar);
    if(!bret)
    {
        ret = LAST_ERROR_CODE();
        ERROR_INFO("Set IDC_EDT_PORT Error(%d)\n",ret);
        goto fail;
    }
    SprintfString(pChar,256,TEXT("RCONTROL"));
    bret = InsertComboString(hwnd,IDC_COMBO_ESCAPE,0,pChar);
    if(!bret)
    {
        ret = LAST_ERROR_CODE();
        ERROR_INFO("Insert IDC_COMBO_ESCAPE Error(%d)\n",ret);
        goto fail;
    }
    SprintfString(pChar,256,TEXT("RWIN"));
    bret = InsertComboString(hwnd,IDC_COMBO_ESCAPE,1,pChar);
    if(!bret)
    {
        ret = LAST_ERROR_CODE();
        ERROR_INFO("Insert IDC_COMBO_ESCAPE Error(%d)\n",ret);
        goto fail;
    }
    SprintfString(pChar,256,TEXT("RSHIFT"));
    bret = InsertComboString(hwnd,IDC_COMBO_ESCAPE,2,pChar);
    if(!bret)
    {
        ret = LAST_ERROR_CODE();
        ERROR_INFO("Insert IDC_COMBO_ESCAPE Error(%d)\n",ret);
        goto fail;
    }
    SetComboSel(hwnd,IDC_COMBO_ESCAPE,0);

    SetLastError(0);
    return TRUE;
fail:
    SetLastError(ret);
    return FALSE;
}

BOOL GetConnectParam(HWND hwnd)
{
    BOOL bret;
    int ret;
    char* pHostAnsi=NULL;
    int hostansisize=0;
    int port;
#ifdef _UNICODE
    std::auto_ptr<wchar_t> pChar2(new wchar_t[256]),pErrStr2(new wchar_t[256]);
    wchar_t* pChar = pChar2.get();
    wchar_t* pErrStr = pErrStr2.get();
#else
    std::auto_ptr<char> pChar2(new char[256]),pErrStr2(new char[256]);
    char* pChar = pChar2.get();
    char *pErrStr = pErrStr2.get();
#endif

    bret = GetDialogItemString(hwnd,IDC_EDT_HOST,pChar,256);
    if(!bret)
    {
        ret = LAST_ERROR_CODE();
        SprintfString(pErrStr,256,TEXT("Get IDC_EDT_HOST Error(%d)"),ret);
        ::MessageBox(hwnd,pErrStr,TEXT("Error"),MB_OK);
        goto fail;
    }
#ifdef _UNICODE
    ret = UnicodeToAnsi(pChar,&pHostAnsi,&hostansisize);
    if(ret < 0)
    {
        ret = LAST_ERROR_CODE();
        goto fail;
    }
#else
    pHostAnsi = pChar;
    hostansisize = strlen(pHostAnsi) + 1;
#endif

    if(strlen(pHostAnsi) == 0)
    {
        ret = ERROR_INVALID_PARAMETER;
        SprintfString(pErrStr,256,TEXT("Host must specify"));
        ::MessageBox(hwnd,pErrStr,TEXT("Error"),MB_OK);
        goto fail;
    }

    strncpy_s(g_Host,sizeof(g_Host),pHostAnsi,_TRUNCATE);
    bret = GetDialogItemString(hwnd,IDC_EDT_PORT,pChar,256);
    if(!bret)
    {
        ret = LAST_ERROR_CODE();
        SprintfString(pErrStr,256,TEXT("Get IDC_EDT_PORT Error(%d)"),ret);
        ::MessageBox(hwnd,pErrStr,TEXT("Error"),MB_OK);
        goto fail;
    }

#ifdef _UNICODE
    port = _wtoi(pChar);
#else
    port = atoi(pChar);
#endif
    if(port == 0 || port >= (1<<16))
    {
        ret = ERROR_INVALID_PARAMETER;
        SprintfString(pErrStr,256,TEXT("Port (%d) Not valid"),port);
        ::MessageBox(hwnd,pErrStr,TEXT("Error"),MB_OK);
        goto fail;
    }
    g_Port = port;


    port = GetComboSel(hwnd,IDC_COMBO_ESCAPE);
    if(port < 0)
    {
        ret =LAST_ERROR_CODE();
        SprintfString(pErrStr,256,TEXT("Get IDC_COMBO_ESCAPE Error(%d)"),ret);
        ::MessageBox(hwnd,pErrStr,TEXT("Error"),MB_OK);
        goto fail;
    }

    if(port == 0)
    {
        g_EscapeKey = DIK_RCONTROL;
    }
    else if(port == 1)
    {
        g_EscapeKey = DIK_RWIN;
    }
    else if(port ==2)
    {
        g_EscapeKey = DIK_RSHIFT;
    }



    SetLastError(ret);
    return TRUE;
fail:
#ifdef _UNICODE
    UnicodeToAnsi(NULL,&pHostAnsi,&hostansisize);
#else
    pHostAnsi = NULL;
    hostansisize = 0;
#endif
    SetLastError(ret);
    return FALSE;
}


BOOL CALLBACK ConnectDialogProc(HWND hwndDlg,
                                UINT message,
                                WPARAM wParam,
                                LPARAM lParam)
{
    BOOL bret=FALSE;
    switch(message)
    {
    case WM_INITDIALOG:
        bret = InitializeConnectDialog(hwndDlg);
        break;
    case WM_CLOSE:
        EndDialog(hwndDlg,0);
        bret = TRUE;
        break;
    case WM_COMMAND:
        switch(LOWORD(wParam))
        {
        case IDCANCEL:
            EndDialog(hwndDlg,wParam);
            bret = TRUE;
            break;
        case IDOK:
            bret = GetConnectParam(hwndDlg);
            if(bret)
            {
                EndDialog(hwndDlg,wParam);
                bret = TRUE;
            }
            break;
        }
        break;
    default:
        break;
    }

    return bret;
}



void StopConnect(HWND hwnd)
{

    if(g_Socket != INVALID_SOCKET)
    {
        closesocket(g_Socket);
    }
    g_Socket= INVALID_SOCKET;
    g_Connected = 0;
    return ;
}


BOOL StartConnect(HWND hwnd)
{
    int ret;
    struct sockaddr_in saddr;
    u_long nonblock;
    UINT_PTR timeret;
    StopConnect(hwnd);

    /*now first to make socket*/
    g_Socket = socket(AF_INET,SOCK_STREAM,0);
    if(g_Socket == INVALID_SOCKET)
    {
        ret = WSAGetLastError() ? WSAGetLastError() : 1;
        ERROR_INFO("Socket Stream Error(%d)\n",ret);
        goto fail;
    }
    nonblock = 1;
    ret = ioctlsocket(g_Socket,FIONBIO,&nonblock);
    if(ret != NO_ERROR)
    {
        ret = WSAGetLastError() ? WSAGetLastError() : 1;
        ERROR_INFO("ioctl nonblock Error(%d)\n",ret);
        goto fail;
    }

    ZeroMemory(&saddr,sizeof(saddr));
    saddr.sin_family = AF_INET;
    saddr.sin_addr.s_addr = inet_atoi(g_Host);
    saddr.sin_port = htons(g_Port);

    ret = connect(g_Socket,&saddr,sizeof(saddr));
    if(ret == SOCKET_ERROR)
    {
        ret=WSAGetLastError() ? WSAGetLastError() : 1;
        if(ret != WSAEWOULDBLOCK  &&
                ret != WSAEINPROGRESS)
        {
            ERROR_INFO("connect (%s:%d) Error(%d)\n",g_Host,g_Port,ret);
            goto fail;
        }
    }

    ret = WSAAsyncSelect(g_Socket,hwnd,WM_SOCKET,FD_CONNECT|FD_WRITE|FD_CLOSE);
    if(ret != NO_ERROR)
    {
        ret = WSAGetLastError() ? WSAGetLastError() : 1;
        ERROR_INFO("Set hwnd(0x%08x) Select Error(%d)\n",hwnd,ret);
        goto fail;
    }

    timeret = SetTimer(hwnd,SOCKET_TM_EVENTID,SOCKET_TIMEOUT,NULL);
    if(timeret == 0)
    {
        ret = LAST_ERROR_CODE();
        goto fail;
    }


    SetLastError(0);
    return TRUE;
fail:
    KillTimer(hwnd,SOCKET_TM_EVENTID);
    StopConnect(hwnd);
    SetLastError(ret);
    return FALSE;
}


BOOL CallConnectFunction(HWND hwnd)
{
    INT_PTR nRet;
    nRet = DialogBox(hInst,MAKEINTRESOURCE(IDD_DLG_CONNECT),hwnd,ConnectDialogProc);
    if(nRet == IDOK)
    {
        ::MessageBox(hwnd,TEXT("Connect"),TEXT("Notice"),MB_OK);
    }

    return TRUE;
}

void CallStopFunction(HWND hwnd)
{
    return ;
}

//
//  函数:  WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  目的:    处理主窗口的消息。
//
//  WM_COMMAND	- 处理应用程序菜单
//  WM_PAINT	- 绘制主窗口
//  WM_DESTROY	- 发送退出消息并返回
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    int wmId, wmEvent;
    PAINTSTRUCT ps;
    HDC hdc;

    switch(message)
    {
    case WM_COMMAND:
        wmId    = LOWORD(wParam);
        wmEvent = HIWORD(wParam);
        // 分析菜单选择:
        switch(wmId)
        {
        case IDM_EXIT:
            DestroyWindow(hWnd);
            break;
        case ID_CONTROL_CONNECT:
            CallConnectFunction(hWnd);
            break;
        case ID_CONTROL_DISCONNECT:
            CallStopFunction(hWnd);
            break;
        case ID_CONTROL_EXIT:
            PostQuitMessage(0);
            break;
        default:
            return DefWindowProc(hWnd, message, wParam, lParam);
        }
        break;
    case WM_PAINT:
        hdc = BeginPaint(hWnd, &ps);
        // TODO:  在此添加任意绘图代码...
        EndPaint(hWnd, &ps);
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

// “关于”框的消息处理程序。
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch(message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if(LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}

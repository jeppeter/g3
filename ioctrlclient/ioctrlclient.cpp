// ioctrlclient.cpp : ����Ӧ�ó������ڵ㡣
//

#include "stdafx.h"
#include "ioctrlclient.h"

#define MAX_LOADSTRING 100

// ȫ�ֱ���:
HINSTANCE hInst;								// ��ǰʵ��
TCHAR szTitle[MAX_LOADSTRING];					// �������ı�
TCHAR szWindowClass[MAX_LOADSTRING];			// ����������

// �˴���ģ���а����ĺ�����ǰ������:
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

    // TODO:  �ڴ˷��ô��롣
    MSG msg;
    HACCEL hAccelTable;

    hInst = hInstance;
    // ��ʼ��ȫ���ַ���
    LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadString(hInstance, IDC_IOCTRLCLIENT, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // ִ��Ӧ�ó����ʼ��:
    if(!InitInstance(hInstance, nCmdShow))
    {
        return FALSE;
    }

    hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_IOCTRLCLIENT));

    ZeroMemory(&msg,sizeof(msg));
    // ����Ϣѭ��:
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

    return (int) msg.wParam;
}



//
//  ����:  MyRegisterClass()
//
//  Ŀ��:  ע�ᴰ���ࡣ
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
//   ����:  InitInstance(HINSTANCE, int)
//
//   Ŀ��:  ����ʵ�����������������
//
//   ע��:
//
//        �ڴ˺����У�������ȫ�ֱ����б���ʵ�������
//        ��������ʾ�����򴰿ڡ�
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
    HWND hWnd;

    hInst = hInstance; // ��ʵ������洢��ȫ�ֱ�����

    hWnd = CreateWindow(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
                        CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, NULL, NULL, hInstance, NULL);

    if(!hWnd)
    {
        return FALSE;
    }

    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);

    return TRUE;
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
		bret = TRUE;
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
				EndDialog(hwndDlg,wParam);
				bret = TRUE;
				break;
			}
		break;
	default:
		break;
    }

    return bret;
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
//  ����:  WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  Ŀ��:    ���������ڵ���Ϣ��
//
//  WM_COMMAND	- ����Ӧ�ó���˵�
//  WM_PAINT	- ����������
//  WM_DESTROY	- �����˳���Ϣ������
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
        // �����˵�ѡ��:
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
        // TODO:  �ڴ���������ͼ����...
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

// �����ڡ������Ϣ�������
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

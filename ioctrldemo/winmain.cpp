
#include <Windows.h>


int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,LPSTR lpCmdLine, int nShowCmd)
{
    HMENU hMenu=NULL;

    WNDCLASSEX wndClass = { 0 };
    wndClass.cbSize = sizeof(WNDCLASSEX) ;
    wndClass.style = CS_HREDRAW | CS_VREDRAW;
    wndClass.lpfnWndProc = WndProc;
    wndClass.cbClsExtra		= 0;
    wndClass.cbWndExtra		= 0;
    wndClass.hInstance = hInstance;
    wndClass.hCursor = LoadCursor(NULL, IDC_ARROW);
    wndClass.hbrBackground=(HBRUSH)GetStockObject(GRAY_BRUSH);
    wndClass.lpszMenuName = NULL;
    wndClass.lpszClassName = _T("IOControlDemo");

    if(!RegisterClassEx(&wndClass))				//����괰�ں���Ҫ�Դ��������ע�ᣬ�������ܴ��������͵Ĵ���
        return -1;
    hMenu = ::LoadMenu(hInstance, MAKEINTRESOURCE(IDR_MAIN_MENU));
    if(hMenu == NULL)
    {
        return -3;
    }

    return 0;
}


LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)     //���ڹ��̺���WndProc
{
    switch(message)				
    {

    case WM_DESTROY:			
        PostQuitMessage(0);		
        break;					
    case WM_COMMAND:
        switch(wParam)
        {
        case ID_START_IO_INJECT:
            break;
        }
        break;

    default:						
        return DefWindowProc(hwnd, message, wParam, lParam);		
    }

    return 0;					
}


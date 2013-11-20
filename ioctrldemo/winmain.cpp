
#include <Windows.h>


int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,LPSTR lpCmdLine, int nShowCmd)
{
    return 0;
}


LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)     //窗口过程函数WndProc
{
    switch(message)				//switch语句开始
    {

    case WM_DESTROY:				//窗口销毁消息
        PostQuitMessage(0);		//向系统表明有个线程有终止请求。用来响应WM_DESTROY消息
        break;						//跳出该switch语句
    case WM_COMMAND:
        switch(wParam)
        {
        case ID_MESSAGE_COMMAND:
            MessageBox(hwnd,TEXT("Call Message"),TEXT("Notify"),MB_OK);
            break;
        }
        break;

    default:						//若上述case条件都不符合，则执行该default语句
        return DefWindowProc(hwnd, message, wParam, lParam);		//调用缺省的窗口过程来为应用程序没有处理的窗口消息提供缺省的处理。
    }

    return 0;					//正常退出
}


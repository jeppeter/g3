
#include <Windows.h>


int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,LPSTR lpCmdLine, int nShowCmd)
{
    return 0;
}


LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)     //���ڹ��̺���WndProc
{
    switch(message)				//switch��俪ʼ
    {

    case WM_DESTROY:				//����������Ϣ
        PostQuitMessage(0);		//��ϵͳ�����и��߳�����ֹ����������ӦWM_DESTROY��Ϣ
        break;						//������switch���
    case WM_COMMAND:
        switch(wParam)
        {
        case ID_MESSAGE_COMMAND:
            MessageBox(hwnd,TEXT("Call Message"),TEXT("Notify"),MB_OK);
            break;
        }
        break;

    default:						//������case�����������ϣ���ִ�и�default���
        return DefWindowProc(hwnd, message, wParam, lParam);		//����ȱʡ�Ĵ��ڹ�����ΪӦ�ó���û�д���Ĵ�����Ϣ�ṩȱʡ�Ĵ���
    }

    return 0;					//�����˳�
}


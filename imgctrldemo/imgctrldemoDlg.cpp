
// imgctrldemoDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "imgctrldemo.h"
#include "imgctrldemoDlg.h"
#include "afxdialogex.h"
#include <output_debug.h>
#include <uniansi.h>
#include <dllinsert.h>
#include <imgcapctrl.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#ifdef _DEBUG
#pragma comment(lib,"injectctrld.lib")
#pragma comment(lib,"imgcapctrld.lib")
#else
#pragma comment(lib,"injectctrl.lib")
#pragma comment(lib,"imgcapctrl.lib")
#endif

#define LAST_ERROR_RETURN()  (GetLastError() ? GetLastError() : 1)
typedef unsigned long ptr_t;

extern "C" int D3DHook_CaptureImageBuffer(HANDLE hProc,char* strDllName,char * data, int len, int * format, int * width, int * height);

// 用于应用程序“关于”菜单项的 CAboutDlg 对话框

class CAboutDlg : public CDialogEx
{
public:
    CAboutDlg();

// 对话框数据
    enum { IDD = IDD_ABOUTBOX };

protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

// 实现
protected:
    DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CimgctrldemoDlg 对话框




CimgctrldemoDlg::CimgctrldemoDlg(CWnd* pParent /*=NULL*/)
    : CDialogEx(CimgctrldemoDlg::IDD, pParent)
{
    m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
#ifdef _UNICODE
    m_strExe = L"";
    m_strDll = L"";
    m_strBmp = L"";
    m_strParam = L"";
#else
    m_strExe = "";
    m_strDll = "";
    m_strBmp = "";
    m_strParam = "";
#endif
    m_CallProcessId = 0;
    m_BmpId = 0;
    m_SnapSecond = 0;
    m_RealWrite = 0;
}

void CimgctrldemoDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CimgctrldemoDlg, CDialogEx)
    ON_WM_PAINT()
    ON_WM_QUERYDRAGICON()
    ON_COMMAND(ID_BTN_LOAD,OnLoad)
    ON_COMMAND(ID_BTN_SEL_EXE,OnSelExe)
    ON_COMMAND(ID_BTN_SEL_DLL,OnSelDll)
    ON_COMMAND(ID_BTN_SEL_BMP,OnSelBmp)
    ON_MESSAGE(WM_HOTKEY,OnHotKey)
    ON_WM_TIMER()
END_MESSAGE_MAP()


// CimgctrldemoDlg 消息处理程序

BOOL CimgctrldemoDlg::OnInitDialog()
{
    CComboBox * pCombo=NULL;
    unsigned int i;
    CString str;
    CButton *pCheck=NULL;
    CDialogEx::OnInitDialog();

    // 将“关于...”菜单项添加到系统菜单中。

    // IDM_ABOUTBOX 必须在系统命令范围内。
    ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
    ASSERT(IDM_ABOUTBOX < 0xF000);

    CMenu* pSysMenu = GetSystemMenu(FALSE);
    if(pSysMenu != NULL)
    {
        BOOL bNameValid;
        CString strAboutMenu;
        bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
        ASSERT(bNameValid);
        if(!strAboutMenu.IsEmpty())
        {
            pSysMenu->AppendMenu(MF_SEPARATOR);
            pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
        }
    }

    // 设置此对话框的图标。当应用程序主窗口不是对话框时，框架将自动
    //  执行此操作
    SetIcon(m_hIcon, TRUE);			// 设置大图标
    SetIcon(m_hIcon, FALSE);		// 设置小图标

    // TODO: 在此添加额外的初始化代码
    /*set the select mask*/
    pCombo = (CComboBox*)this->GetDlgItem(IDC_COMBO_CHAR);
    for(i=0; i<26; i++)
    {
        str.Format(TEXT("%c"),'A'+i);
        pCombo->InsertString(i,str);
    }

    pCheck = (CButton*) this->GetDlgItem(IDC_CHECK_CTRL);
    pCheck->SetCheck(BST_UNCHECKED);
    pCheck = (CButton*) this->GetDlgItem(IDC_CHECK_ALT);
    pCheck->SetCheck(BST_UNCHECKED);
    pCheck = (CButton*) this->GetDlgItem(IDC_CHECK_WIN);
    pCheck->SetCheck(BST_UNCHECKED);
    pCheck = (CButton*) this->GetDlgItem(IDC_CHECK_SHIFT);
    pCheck->SetCheck(BST_UNCHECKED);
    pCheck = (CButton*)this->GetDlgItem(IDC_CHECK_TIMER);
    pCheck->SetCheck(BST_UNCHECKED);
    pCheck = (CButton*)this->GetDlgItem(IDC_CHECK_REALWRITE);
    pCheck->SetCheck(BST_UNCHECKED);

    return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}


// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CimgctrldemoDlg::OnPaint()
{
    if(IsIconic())
    {
        CPaintDC dc(this); // 用于绘制的设备上下文

        SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

        // 使图标在工作区矩形中居中
        int cxIcon = GetSystemMetrics(SM_CXICON);
        int cyIcon = GetSystemMetrics(SM_CYICON);
        CRect rect;
        GetClientRect(&rect);
        int x = (rect.Width() - cxIcon + 1) / 2;
        int y = (rect.Height() - cyIcon + 1) / 2;

        // 绘制图标
        dc.DrawIcon(x, y, m_hIcon);
    }
    else
    {
        CDialogEx::OnPaint();
    }
}

#define  CAPTURE_HOTKEY_ID      131


//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR CimgctrldemoDlg::OnQueryDragIcon()
{
    return static_cast<HCURSOR>(m_hIcon);
}


void CimgctrldemoDlg::OnLoad()
{
    DEBUG_INFO("\n");
    /*now we should test for the job*/
    char *pDllName=NULL,*pFullDllName=NULL,*pExecName=NULL,*pBmpFile=NULL,*pParam = NULL,*pNum=NULL;
    int fulldllnamesize=0,execnamesize=0,bmpfilesize=0,paramsize=0,numsize=0;
    int ret;
    CEdit* pEdt=NULL;
    CButton *pCheck=NULL;
    CComboBox *pCombo=NULL;
    int ctrlcheck=BST_UNCHECKED,altcheck=BST_UNCHECKED,wincheck=BST_UNCHECKED,shiftcheck=BST_UNCHECKED,charsel=-1;
    char* pCommandLine=NULL;
    int commandlinesize=0;
    int timercheck=0;
    int edttimer=0;
    CString errstr,numstr;
    UINT hkmask,hkvk;
    BOOL bret;

    /*now to get the string*/
    pEdt = (CEdit*) this->GetDlgItem(IDC_EDT_EXE);
    pEdt->GetWindowText(m_strExe);
    pEdt = (CEdit*) this->GetDlgItem(IDC_EDT_DLL);
    pEdt->GetWindowText(m_strDll);
    pEdt = (CEdit*) this->GetDlgItem(IDC_EDT_BMP);
    pEdt->GetWindowText(m_strBmp);
    pEdt = (CEdit*) this->GetDlgItem(IDC_EDT_PARAM);
    pEdt->GetWindowText(m_strParam);

    pCheck = (CButton*) this->GetDlgItem(IDC_CHECK_CTRL);
    ctrlcheck=pCheck->GetCheck();

    pCheck = (CButton*) this->GetDlgItem(IDC_CHECK_ALT);
    altcheck=pCheck->GetCheck();
    pCheck = (CButton*) this->GetDlgItem(IDC_CHECK_WIN);
    wincheck=pCheck->GetCheck();
    pCheck = (CButton*) this->GetDlgItem(IDC_CHECK_SHIFT);
    shiftcheck=pCheck->GetCheck();

    pCombo = (CComboBox*) this->GetDlgItem(IDC_COMBO_CHAR);
    charsel = pCombo->GetCurSel();

    pCheck = (CButton*)this->GetDlgItem(IDC_CHECK_TIMER);
    timercheck = pCheck->GetCheck();
    if(timercheck)
    {
        pEdt = (CEdit*) this->GetDlgItem(IDC_EDIT_TIMER);
        pEdt->GetWindowText(numstr);

    }
    pCheck = (CButton*)this->GetDlgItem(IDC_CHECK_REALWRITE);
    m_RealWrite = pCheck->GetCheck() ? 1 : 0;


#ifdef _UNICODE
    ret = UnicodeToAnsi((wchar_t*)((LPCWSTR)m_strExe),&pExecName,&execnamesize);
    if(ret < 0)
    {
        goto out;
    }
    ret = UnicodeToAnsi((wchar_t*)((const WCHAR*)m_strDll),&pFullDllName,&fulldllnamesize);
    if(ret < 0)
    {
        goto out;
    }

    ret = UnicodeToAnsi((wchar_t*)((const WCHAR*)m_strBmp),&pBmpFile,&bmpfilesize);
    if(ret < 0)
    {
        goto out;
    }

    ret = UnicodeToAnsi((wchar_t*)((const WCHAR*)m_strParam),&pParam,&paramsize);
    if(ret < 0)
    {
        goto out;
    }

    if(timercheck)
    {
        ret = UnicodeToAnsi(((wchar_t*)((const WCHAR*)numstr)),&pNum,&numsize);
        if(ret < 0)
        {
            goto out;
        }
        m_SnapSecond = atoi(pNum);
    }

#else
    pExecName = (const char*)m_strExe;
    pFullDllName = (const char*) m_strDll;
    pBmpFile = (const char*) m_strBmp;
    pParam = (const char*)m_strParam;
#endif
    pDllName = strrchr(pFullDllName,'\\');
    if(pDllName)
    {
        pDllName += 1;
    }
    else
    {
        pDllName = pFullDllName;
    }

    DEBUG_INFO("\n");

    /*now we should start the command */
    if(pExecName == NULL|| strlen(pExecName) == 0)
    {
        AfxMessageBox(TEXT("Must Specify Exec"));
        goto out;
    }
    DEBUG_INFO("\n");

    if(pBmpFile == NULL|| strlen(pBmpFile)==0)
    {
        AfxMessageBox(TEXT("Must Specify Bmp file"));
        goto out;
    }

    if(pDllName == NULL || pFullDllName == NULL || strlen(pFullDllName) == 0)
    {
        errstr.Format(TEXT("Can not Run(%s)"),pCommandLine);
        AfxMessageBox(errstr);
        goto out;
    }
    DEBUG_INFO("\n");



    DEBUG_INFO("exename (%s)\n",pExecName);
    DEBUG_INFO("dllname (%s)\n",pDllName);
    DEBUG_INFO("param (%s)\n",pParam);
    DEBUG_INFO("bmpfile (%s)\n",pBmpFile);
    DEBUG_INFO("FullDllName (%s)\n",pFullDllName);

    /*now to start for the running*/
    commandlinesize = strlen(pExecName);
    if(strlen(pParam))
    {
        commandlinesize += 1;
        commandlinesize += strlen(pParam);
    }

    commandlinesize += 1;

    pCommandLine = new char[commandlinesize];
    if(strlen(pParam))
    {
        ret = _snprintf_s(pCommandLine,commandlinesize,commandlinesize,"%s %s",pExecName,pParam);
    }
    else
    {
        ret = _snprintf_s(pCommandLine,commandlinesize,commandlinesize,"%s",pExecName);
    }

    ret = LoadInsert(NULL,pCommandLine,pFullDllName,pDllName);
    if(ret < 0)
    {
        errstr.Format(TEXT("Can not run(%s)"),pCommandLine);
        AfxMessageBox(errstr);
        goto out;
    }
    m_BmpId = 0;
    m_CallProcessId=ret;

    /*now to register hotkey ,first to unregister hotkey*/
    UnregisterHotKey(this->m_hWnd,CAPTURE_HOTKEY_ID);
    this->KillTimer(SNAPSHOT_TIME_ID);
    if(timercheck)
    {
        if(m_SnapSecond == 0 || m_SnapSecond > 60000)
        {
            AfxMessageBox(TEXT("Time should be > 0 && <= 60000"));
            goto out;
        }
        this->SetTimer(SNAPSHOT_TIME_ID,m_SnapSecond ,NULL);
    }
    else
    {
        if(ctrlcheck != BST_CHECKED && altcheck != BST_CHECKED &&
                wincheck != BST_CHECKED)
        {
            AfxMessageBox(TEXT("Ctrl Alt Win Must specify one"));
            goto out;
        }

        if(charsel == CB_ERR || charsel < 0 || charsel >= 26)
        {
            AfxMessageBox(TEXT("Must Select one char"));
            goto out;
        }
        hkmask = 0;
        if(altcheck)
        {
            hkmask |= MOD_ALT;
        }
        if(ctrlcheck)
        {
            hkmask |= MOD_CONTROL;
        }
        if(wincheck)
        {
            hkmask |= MOD_WIN;
        }
        if(shiftcheck)
        {
            hkmask |= MOD_SHIFT;
        }

        /*this is for VK_A*/
        hkvk = 0x41;
        hkvk += charsel;

        bret = RegisterHotKey(this->m_hWnd,CAPTURE_HOTKEY_ID,hkmask,hkvk);
        if(!bret)
        {
            errstr.Format(TEXT("Register Hotkey Error %d"),GetLastError());
            AfxMessageBox(errstr);
            UnregisterHotKey(this->m_hWnd,CAPTURE_HOTKEY_ID);
            goto out;
        }
    }
    /*all is ok*/

out:
    if(pCommandLine)
    {
        delete [] pCommandLine;
    }
    pCommandLine = NULL;

#ifdef _UNICODE
    /*pDllName is the pointer in the pFullDllName so do not free two times*/
    UnicodeToAnsi(NULL,&pFullDllName,&fulldllnamesize);
    UnicodeToAnsi(NULL,&pExecName,&execnamesize);
    UnicodeToAnsi(NULL,&pBmpFile,&bmpfilesize);
    UnicodeToAnsi(NULL,&pParam,&paramsize);
    UnicodeToAnsi(NULL,&pNum,&numsize);
#endif
    return;
}


void CimgctrldemoDlg::OnSelExe()
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

void CimgctrldemoDlg::OnSelDll()
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

void CimgctrldemoDlg::OnSelBmp()
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

}

int CimgctrldemoDlg::SnapShot()
{
    /*now to get top window*/
    char *pDllName=NULL,*pFullDllName=NULL,*pBmpFile=NULL;
    int fulldllnamesize=0,execnamesize=0,bmpfilesize=0,paramsize=0;
    unsigned int processid;
    int ret=-1;
    CString errstr;
    CString strFormatBmp;
    HANDLE hProc=NULL;
    int getlen=0,writelen = 0;
    HANDLE hFile=NULL;
    char* pData=NULL;
    int datalen = 0x800000;
    int format,width,height;
    DWORD curret;
    BOOL bret;
    CImgCapController *pImgCap=NULL;
    int curstamp;
    processid = m_CallProcessId;
    //DEBUG_INFO("\n");


#ifdef _UNICODE
    //DEBUG_INFO("\n");
    ret = UnicodeToAnsi((wchar_t*)((const WCHAR*)m_strDll),&pFullDllName,&fulldllnamesize);
    if(ret < 0)
    {
        goto out;
    }

#else
    pFullDllName = (const char*) m_strDll;

#endif
    //DEBUG_INFO("\n");
    pDllName = strrchr(pFullDllName,'\\');
    if(pDllName)
    {
        pDllName += 1;
    }
    else
    {
        pDllName = pFullDllName;
    }
    //DEBUG_INFO("\n");

    pData = (char*)malloc(datalen);
    if(pData == NULL)
    {
        ret = LAST_ERROR_RETURN();
        ret = -ret;
        ERROR_INFO("could not open datalen %d\n",datalen);
        goto out;
    }
    //DEBUG_INFO("\n");


    hProc = OpenProcess(PROCESS_VM_OPERATION | PROCESS_QUERY_INFORMATION,FALSE,processid);
    if(hProc==NULL)
    {
        ret = LAST_ERROR_RETURN();
        ret = -ret;
        ERROR_INFO("could not open process (%d) (%d)\n",processid,-ret);
        goto out;
    }

    DEBUG_INFO("open process(%d) hProc 0x%08lx\n",processid,hProc);
    pImgCap = new CImgCapController();
    bret = pImgCap->Start(hProc,pDllName,20);
    if(!bret)
    {
        ret = LAST_ERROR_RETURN();
        goto out;
    }




    bret = pImgCap->CapImage((uint8_t*)pData,datalen,&format,&width,&height,&curstamp,&getlen);
    if(!bret)
    {
        ret = LAST_ERROR_RETURN();
        goto out;
    }

#ifdef _UNICODE
    strFormatBmp.Format(TEXT("%s.%d.bmp"),(const WCHAR*)m_strBmp,m_BmpId);
    m_BmpId ++;
    ret = UnicodeToAnsi((wchar_t*)((const WCHAR*)strFormatBmp),&pBmpFile,&bmpfilesize);
    if(ret < 0)
    {
        goto out;
    }
    DEBUG_INFO("pBmpFile %s (%S)\n",pBmpFile,(const WCHAR*)m_strBmp);
    hFile = CreateFile((wchar_t*)((const WCHAR*)strFormatBmp),GENERIC_WRITE | GENERIC_READ , FILE_SHARE_READ ,NULL,
                       OPEN_ALWAYS,FILE_ATTRIBUTE_NORMAL,NULL);
#else
    strFormatBmp.Format(TEXT("%s.%d.bmp"),(const char*)m_strBmp,m_BmpId);
    m_BmpId ++;
    pBmpFile = (const char*) m_strBmp;
    hFile = CreateFile(pBmpFile,GENERIC_WRITE | GENERIC_READ , FILE_SHARE_READ ,NULL,
                       OPEN_ALWAYS,FILE_ATTRIBUTE_NORMAL,NULL);
#endif
    if(hFile == NULL)
    {
        ret = LAST_ERROR_RETURN();
        ret = -ret;
        ERROR_INFO("could not open %s file for write (%d)\n",pBmpFile,-ret);
        goto out;
    }
    DEBUG_INFO("\n");



    if(this->m_RealWrite)
    {
        writelen = 0;
        while(writelen < getlen)
        {
            bret = WriteFile(hFile,(LPCVOID)((ptr_t)pData+writelen),(getlen-writelen),&curret,NULL);
            if(!bret)
            {
                ret = LAST_ERROR_RETURN();
                ret = -ret;
                goto out;
            }

            writelen += curret;
        }
    }
    ret = 0;


out:
    if(pImgCap)
    {
        delete pImgCap;
    }
    pImgCap = NULL;
#ifdef _UNICODE
    UnicodeToAnsi(NULL,&pFullDllName,&fulldllnamesize);
    UnicodeToAnsi(NULL,&pBmpFile,&bmpfilesize);
#endif
    if(hProc)
    {
        CloseHandle(hProc);
    }
    hProc = NULL;
    if(hFile)
    {
        CloseHandle(hFile);
    }
    hFile = NULL;
    if(pData)
    {
        free(pData);
    }
    pData = NULL;
    datalen = 0;
    return -ret;
}

LRESULT CimgctrldemoDlg::OnHotKey(WPARAM wParam, LPARAM lParam)
{
    if(wParam == CAPTURE_HOTKEY_ID)
    {
        SnapShot();
    }
    return 0;
}

void CimgctrldemoDlg::OnTimer(UINT nEvent)
{
    //DEBUG_INFO("Timer++++++++++\n");
    if(nEvent == SNAPSHOT_TIME_ID)
    {
        SnapShot();
    }

    CDialogEx::OnTimer(nEvent);
}





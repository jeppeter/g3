
// pcmctrldemo.h : PROJECT_NAME Ӧ�ó������ͷ�ļ�
//

#pragma once

#ifndef __AFXWIN_H__
	#error "�ڰ������ļ�֮ǰ������stdafx.h�������� PCH �ļ�"
#endif

#include "resource.h"		// ������


// CpcmctrldemoApp:
// �йش����ʵ�֣������ pcmctrldemo.cpp
//

class CpcmctrldemoApp : public CWinApp
{
public:
	CpcmctrldemoApp();

// ��д
public:
	virtual BOOL InitInstance();

// ʵ��

	DECLARE_MESSAGE_MAP()
};

extern CpcmctrldemoApp theApp;
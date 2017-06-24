
// ComboBox_ListBox.h : PROJECT_NAME 应用程序的主头文件
//

#pragma once

#ifndef __AFXWIN_H__
	#error "在包含此文件之前包含“stdafx.h”以生成 PCH 文件"
#endif

#include "resource.h"		// 主符号


// CComboBox_ListBoxApp: 
// 有关此类的实现，请参阅 ComboBox_ListBox.cpp
//

class CComboBox_ListBoxApp : public CWinApp
{
public:
	CComboBox_ListBoxApp();

// 重写
public:
	virtual BOOL InitInstance();

// 实现

	DECLARE_MESSAGE_MAP()
};

extern CComboBox_ListBoxApp theApp;
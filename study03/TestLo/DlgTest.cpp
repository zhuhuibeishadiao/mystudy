// DlgTest.cpp : 实现文件
//

#include "stdafx.h"
#include "DlgTest.h"
#include "afxdialogex.h"
#include "resource.h"


// CDlgTest 对话框

IMPLEMENT_DYNAMIC(CDlgTest, CDialogEx)

CDlgTest::CDlgTest(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_DLGTEST, pParent)
{

}

CDlgTest::~CDlgTest()
{
}

void CDlgTest::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CDlgTest, CDialogEx)
END_MESSAGE_MAP()


// CDlgTest 消息处理程序

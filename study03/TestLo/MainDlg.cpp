
// MainDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "MainApp.h"
#include "MainDlg.h"
#include "afxdialogex.h"
#include "CommonMFC/mfc_listview.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// 用于应用程序“关于”菜单项的 CAboutDlg 对话框

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

// 实现
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CMainCDlg 对话框



CMainCDlg::CMainCDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_TEMPMFC_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CMainCDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CMainCDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BUTTON1, &CMainCDlg::OnBnClickedButton1)
	ON_BN_CLICKED(IDC_BUTTON2, &CMainCDlg::OnBnClickedButton2)
	ON_BN_CLICKED(IDC_BUTTON3, &CMainCDlg::OnBnClickedButton3)
	ON_BN_CLICKED(IDC_Test, &CMainCDlg::OnBnClickedTest)
	ON_BN_CLICKED(IDC_BUTTON5, &CMainCDlg::OnBnClickedButton5)
END_MESSAGE_MAP()


// CMainCDlg 消息处理程序

BOOL CMainCDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 将“关于...”菜单项添加到系统菜单中。

	// IDM_ABOUTBOX 必须在系统命令范围内。
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// 设置此对话框的图标。  当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	// TODO: 在此添加额外的初始化代码
	usr::util::alloc_cmd_window();

	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

void CMainCDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。  对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CMainCDlg::OnPaint()
{
	if (IsIconic())
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

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR CMainCDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


//MFC中的控制台分配
void CMainCDlg::OnBnClickedButton1()
{
	// TODO: 在此添加控件通知处理程序代码

	usr::util::alloc_cmd_window();
	int i = 10;
	std::cout << "分配控制台，打印10次:" << std::endl;
	while (i--)
	{
		printf("hello world\n");
		Sleep(500);
	}
	FreeConsole();
}


//获得文件路径
void CMainCDlg::OnBnClickedButton2()
{
	//filter格式:"显示的给用户看的\0真正规则\0"
	WCHAR szFileName[MAX_PATH];
	usr::util::GetOpenName(GetModuleHandle(nullptr),
		szFileName, L"全部文件\0*.*\0ini文件\0*.ini\0", L"你没眼睛？");
	printf("%ws\n", szFileName);
}

//获得保存路径
void CMainCDlg::OnBnClickedButton3()
{
	// TODO: 在此添加控件通知处理程序代码
	WCHAR szFileName[MAX_PATH];
	usr::util::GetSaveName(GetModuleHandle(NULL), szFileName, L"INI(*.INI)\0*.INI\0", L"眼睛");
	printf("%ws\n", szFileName);
}

//不定参数例子
//count一共有几个参数
void foo(int count, ...)
{
	int index = 0;
	int arg;
	//步骤1：
	va_list	va;
	//步骤2：
	va_start(va, count);

	//步骤3
	for (int i = 0; i < count; i++)
	{
		//获取的是count后面的第一个数字
		arg = va_arg(va, int);
		printf("%d\n", arg);
	}
	//清理
	va_end(va);
}

void CMainCDlg::OnBnClickedTest()
{
	//foo(5, 1, 2, 3, 4, 5);
}

#include "DlgListCtrl.h"
#include "DlgTest.h"
void CMainCDlg::OnBnClickedButton5()
{
	// TODO: 在此添加控件通知处理程序代码
	CDlgListCtrl dlg;
	dlg.DoModal();

	//CDlgTest dlg1;
	//dlg1.DoModal();
}

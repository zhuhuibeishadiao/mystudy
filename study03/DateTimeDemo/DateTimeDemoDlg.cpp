
// DateTimeDemoDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "DateTimeDemo.h"
#include "DateTimeDemoDlg.h"
#include "afxdialogex.h"

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


// CDateTimeDemoDlg 对话框



CDateTimeDemoDlg::CDateTimeDemoDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_DATETIMEDEMO_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CDateTimeDemoDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_DATE_BEGIN, m_date_begin);
	DDX_Control(pDX, IDC_DATE_END, m_date_end);
	DDX_Control(pDX, IDC_EDIT_OUTPUT, m_edit_output);
}

BEGIN_MESSAGE_MAP(CDateTimeDemoDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_NOTIFY(DTN_DATETIMECHANGE, IDC_DATE_BEGIN, &CDateTimeDemoDlg::OnDatetimechangeDateBegin)
	ON_NOTIFY(DTN_DATETIMECHANGE, IDC_DATE_END, &CDateTimeDemoDlg::OnDatetimechangeDateEnd)
END_MESSAGE_MAP()


// CDateTimeDemoDlg 消息处理程序

BOOL CDateTimeDemoDlg::OnInitDialog()
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
	m_date_begin.SetFormat(L"yyyy-MM-dd HH:mm:ss");
	m_date_end.SetFormat(L"yyyy-MM-dd HH:mm:ss");



	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

void CDateTimeDemoDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

void CDateTimeDemoDlg::OnPaint()
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
HCURSOR CDateTimeDemoDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


//时间选择器Demo
void CDateTimeDemoDlg::OnDatetimechangeDateBegin(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMDATETIMECHANGE pDTChange = reinterpret_cast<LPNMDATETIMECHANGE>(pNMHDR);
	// TODO: 在此添加控件通知处理程序代码
	//1.获取当前事件
	SYSTEMTIME st_begin, st_end;
	m_date_begin.GetTime(&st_begin);

	//2.获取另一个控件事件
	m_date_end.GetTime(&st_end);

	//使用FILETIME计算
	FILETIME ft_bein, ft_end;
	SystemTimeToFileTime(&st_begin, &ft_bein);
	SystemTimeToFileTime(&st_end, &ft_end);
	
	ULARGE_INTEGER u_begin, u_end, u_span;
	u_begin.u.HighPart = ft_bein.dwHighDateTime;
	u_begin.u.LowPart = ft_bein.dwLowDateTime;

	u_end.u.HighPart = ft_end.dwHighDateTime;
	u_end.u.LowPart = ft_end.dwLowDateTime;

	
	if (u_end.QuadPart >= u_begin.QuadPart)
	{
		u_span.QuadPart = u_end.QuadPart - u_begin.QuadPart;
	}
	else
	{
		u_span.QuadPart = u_begin.QuadPart - u_end.QuadPart;
	}
	
	
	ULONGLONG ull_second = u_span.QuadPart / 10000000;
	int n_minute = ull_second / 60;
	int n_hours = n_minute / 60;
	int n_days = n_hours / 24;
	CString strOutput;
	//strOutput.Format(L"begin:%lu, %lu\r\nend:%lu, %lu", u_begin.HighPart, u_begin.LowPart, u_end.HighPart, u_end.LowPart);
	

	strOutput.Format(L"秒:%I64u\r\n分钟:%u\r\n小时:%u\r\n天：%u", ull_second, n_minute, n_hours, n_days);

	SetDlgItemTextW(IDC_EDIT_OUTPUT, strOutput);
	*pResult = 0;
}


void CDateTimeDemoDlg::OnDatetimechangeDateEnd(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMDATETIMECHANGE pDTChange = reinterpret_cast<LPNMDATETIMECHANGE>(pNMHDR);
	// TODO: 在此添加控件通知处理程序代码
	//1.获取当前事件
	SYSTEMTIME st_begin, st_end;
	m_date_begin.GetTime(&st_begin);

	//2.获取另一个控件事件
	m_date_end.GetTime(&st_end);

	//使用FILETIME计算
	FILETIME ft_bein, ft_end;
	SystemTimeToFileTime(&st_begin, &ft_bein);
	SystemTimeToFileTime(&st_end, &ft_end);

	ULARGE_INTEGER u_begin, u_end, u_span;
	u_begin.u.HighPart = ft_bein.dwHighDateTime;
	u_begin.u.LowPart = ft_bein.dwLowDateTime;

	u_end.u.HighPart = ft_end.dwHighDateTime;
	u_end.u.LowPart = ft_end.dwLowDateTime;


	if (u_end.QuadPart >= u_begin.QuadPart)
	{
		u_span.QuadPart = u_end.QuadPart - u_begin.QuadPart;
	}
	else
	{
		u_span.QuadPart = u_begin.QuadPart - u_end.QuadPart;
	}


	ULONGLONG ull_second = u_span.QuadPart / 10000000;
	int n_minute = ull_second / 60;
	int n_hours = n_minute / 60;
	int n_days = n_hours / 24;
	CString strOutput;
	//strOutput.Format(L"begin:%lu, %lu\r\nend:%lu, %lu", u_begin.HighPart, u_begin.LowPart, u_end.HighPart, u_end.LowPart);


	strOutput.Format(L"秒:%I64u\r\n分钟:%u\r\n小时:%u\r\n天：%u", ull_second, n_minute, n_hours, n_days);

	SetDlgItemTextW(IDC_EDIT_OUTPUT, strOutput);
	*pResult = 0;
}

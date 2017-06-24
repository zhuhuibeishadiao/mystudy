
// CheckBoxDemoDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "CheckBoxDemo.h"
#include "CheckBoxDemoDlg.h"
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


// CCheckBoxDemoDlg 对话框



CCheckBoxDemoDlg::CCheckBoxDemoDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_CHECKBOXDEMO_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CCheckBoxDemoDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_CHECK_RED, m_chkRed);
	DDX_Control(pDX, IDC_CHECK_BLUE, m_chkBlue);
	DDX_Control(pDX, IDC_CHECK_GREEN, m_chkGreen);
}

BEGIN_MESSAGE_MAP(CCheckBoxDemoDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_ERASEBKGND()
	ON_BN_CLICKED(IDC_CHECK_RED, &CCheckBoxDemoDlg::OnBnClickedCheckRed)
	ON_BN_CLICKED(IDC_CHECK_GREEN, &CCheckBoxDemoDlg::OnBnClickedCheckGreen)
	ON_BN_CLICKED(IDC_CHECK_BLUE, &CCheckBoxDemoDlg::OnBnClickedCheckBlue)
	ON_BN_CLICKED(IDC_RADIO_RECT, &CCheckBoxDemoDlg::OnBnClickedRadioRect)
	ON_BN_CLICKED(IDC_RADIO_CIRCLE, &CCheckBoxDemoDlg::OnBnClickedRadioCircle)
END_MESSAGE_MAP()


// CCheckBoxDemoDlg 消息处理程序

BOOL CCheckBoxDemoDlg::OnInitDialog()
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

	m_chkRed.SetCheck(TRUE);
	static_cast<CButton *>(GetDlgItem(IDC_RADIO_RECT))->SetCheck(TRUE);

	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

void CCheckBoxDemoDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

void CCheckBoxDemoDlg::OnPaint()
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
HCURSOR CCheckBoxDemoDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



BOOL CCheckBoxDemoDlg::OnEraseBkgnd(CDC* pDC)
{
	//本来在最后，这里提上来，等绘制完，再返回bRet
	auto  bRet = CDialogEx::OnEraseBkgnd(pDC);

	//添加好变量，检查是否被选中
	auto nRed = m_chkRed.GetCheck() ? 255 : 0;
	auto nGreen = m_chkGreen.GetCheck() ? 255 : 0;
	auto nBlue = m_chkBlue.GetCheck() ? 255 : 0;
	COLORREF crColor = RGB(nRed, nGreen, nBlue);

	//定义画刷，及其颜色，这是PDC的画刷
	CBrush brush;
	brush.CreateSolidBrush(crColor);
	auto oldBrush = pDC->SelectObject(&brush);

	RECT rect = { 0, 0, 100, 100 };
	auto radioRect = (CButton *)GetDlgItem(IDC_RADIO_RECT);
	auto radioCircle = static_cast<CButton *>(GetDlgItem(IDC_RADIO_CIRCLE));
	if (radioRect->GetCheck())
	{
		pDC->Rectangle(&rect);
	}
	else if(radioCircle->GetCheck())
	{
		pDC->Ellipse(&rect);
	}

	//还原老画刷
	pDC->SelectObject(oldBrush);
	return bRet;
}


void CCheckBoxDemoDlg::OnBnClickedCheckRed()
{
	// TODO: 在此添加控件通知处理程序代码
	Invalidate();
}


void CCheckBoxDemoDlg::OnBnClickedCheckGreen()
{
	// TODO: 在此添加控件通知处理程序代码
	Invalidate();
}


void CCheckBoxDemoDlg::OnBnClickedCheckBlue()
{
	// TODO: 在此添加控件通知处理程序代码
	Invalidate();
}


void CCheckBoxDemoDlg::OnBnClickedRadioRect()
{
	// TODO: 在此添加控件通知处理程序代码
	Invalidate();
}


void CCheckBoxDemoDlg::OnBnClickedRadioCircle()
{
	// TODO: 在此添加控件通知处理程序代码
	Invalidate();
}

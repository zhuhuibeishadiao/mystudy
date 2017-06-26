
// TreeCtrlDemoDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "TreeCtrlDemo.h"
#include "TreeCtrlDemoDlg.h"
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


// CTreeCtrlDemoDlg 对话框



CTreeCtrlDemoDlg::CTreeCtrlDemoDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_TREECTRLDEMO_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CTreeCtrlDemoDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_TREE1, m_tree);
}

BEGIN_MESSAGE_MAP(CTreeCtrlDemoDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	
	ON_BN_CLICKED(IDC_BUTTON_INSERT, &CTreeCtrlDemoDlg::OnBnClickedButtonInsert)
	ON_BN_CLICKED(IDC_BUTTON_DEL, &CTreeCtrlDemoDlg::OnBnClickedButtonDel)
	ON_BN_CLICKED(IDC_UPDATE, &CTreeCtrlDemoDlg::OnBnClickedUpdate)
	ON_NOTIFY(TVN_SELCHANGED, IDC_TREE, &CTreeCtrlDemoDlg::OnSelchangedTree)
END_MESSAGE_MAP()


// CTreeCtrlDemoDlg 消息处理程序

BOOL CTreeCtrlDemoDlg::OnInitDialog()
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

	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

void CTreeCtrlDemoDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

void CTreeCtrlDemoDlg::OnPaint()
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
HCURSOR CTreeCtrlDemoDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}




//treeDemo增删查改
//增按钮
void CTreeCtrlDemoDlg::OnBnClickedButtonInsert()
{
	// TODO: 在此添加控件通知处理程序代码
	CString str;
	GetDlgItemTextW(IDC_EDIT_INPUT, str);

	HTREEITEM hTreeItem = m_tree.GetSelectedItem();
	if (!hTreeItem)
		hTreeItem = TVI_ROOT;
	
	TVINSERTSTRUCT ts = {};
	ts.hParent = hTreeItem;
	ts.item.pszText = (LPWSTR)(LPCTSTR)str;  //这样也可以转换
	ts.item.mask = TVIF_TEXT;
	ts.hInsertAfter = TVI_LAST;
	

	HTREEITEM hNewItem = m_tree.InsertItem(&ts);
	m_tree.SelectItem(hNewItem);
	m_tree.EnsureVisible(hNewItem);	
}

//删按钮
void CTreeCtrlDemoDlg::OnBnClickedButtonDel()
{
	// TODO: 在此添加控件通知处理程序代码
	HTREEITEM hTreeItem = m_tree.GetSelectedItem();
	if (!hTreeItem)
		return;
	//
	HTREEITEM hParent = m_tree.GetParentItem(hTreeItem);
	m_tree.DeleteItem(hTreeItem);
	m_tree.SelectItem(hParent);
}

//点击直接查
void CTreeCtrlDemoDlg::OnSelchangedTree(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMTREEVIEW pNMTreeView = reinterpret_cast<LPNMTREEVIEW>(pNMHDR);
	// TODO: 在此添加控件通知处理程序代码
	HTREEITEM hTreeItem = m_tree.GetSelectedItem();
	if (hTreeItem)
	{
		CString str = m_tree.GetItemText(hTreeItem);
		SetDlgItemTextW(IDC_EDIT_INPUT, str);
	}
	*pResult = 0;
}


//改按钮
void CTreeCtrlDemoDlg::OnBnClickedUpdate()
{
	// TODO: 在此添加控件通知处理程序代码
	HTREEITEM hTreeItem = m_tree.GetSelectedItem();
	if (hTreeItem)
	{
		CString str;
		GetDlgItemTextW(IDC_EDIT_INPUT, str);
		m_tree.SetItemText(hTreeItem, str);
	}
}



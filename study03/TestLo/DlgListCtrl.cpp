// DlgListCtrl.cpp : 实现文件
//

#include "stdafx.h"
#include "resource.h"
#include "DlgListCtrl.h"
#include "afxdialogex.h"
#include "CommonLib/usr_ini_file.h"


// CDlgListCtrl 对话框

IMPLEMENT_DYNAMIC(CDlgListCtrl, CDialogEx)

CDlgListCtrl::CDlgListCtrl(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_LISTDEMO, pParent)
{

	EnableAutomation();

}

CDlgListCtrl::~CDlgListCtrl()
{
}

void CDlgListCtrl::OnFinalRelease()
{
	// 释放了对自动化对象的最后一个引用后，将调用
	// OnFinalRelease。  基类将自动
	// 删除该对象。  在调用该基类之前，请添加您的
	// 对象所需的附加清理代码。

	CDialogEx::OnFinalRelease();
}

void CDlgListCtrl::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LISTDEMO, m_list);
}


BEGIN_MESSAGE_MAP(CDlgListCtrl, CDialogEx)
	ON_BN_CLICKED(IDC_BUTTON1, &CDlgListCtrl::OnBnClickedButton1)
	ON_BN_CLICKED(IDC_BUTTON2, &CDlgListCtrl::OnBnClickedButton2)
	ON_BN_CLICKED(IDC_BUTTON3, &CDlgListCtrl::OnBnClickedButton3)
	ON_NOTIFY(NM_RCLICK, IDC_LISTDEMO, &CDlgListCtrl::OnRclickListdemo)
	ON_COMMAND(ID__SDF, &CDlgListCtrl::MenuAdd)
	ON_COMMAND(ID__SDF32772, &CDlgListCtrl::MenuDel)
	ON_BN_CLICKED(IDC_BUTTON5, &CDlgListCtrl::OnBnClickedButton5)
	ON_BN_CLICKED(IDC_BUTTON4, &CDlgListCtrl::OnBnClickedButton4)
END_MESSAGE_MAP()

BEGIN_DISPATCH_MAP(CDlgListCtrl, CDialogEx)
END_DISPATCH_MAP()

// 注意: 我们添加 IID_IDlgListCtrl 支持
//  以支持来自 VBA 的类型安全绑定。  此 IID 必须同附加到 .IDL 文件中的
//  调度接口的 GUID 匹配。

// {8872B7B7-9466-43BB-8D62-E902A25B11AA}
static const IID IID_IDlgListCtrl =
{ 0x8872B7B7, 0x9466, 0x43BB, { 0x8D, 0x62, 0xE9, 0x2, 0xA2, 0x5B, 0x11, 0xAA } };

BEGIN_INTERFACE_MAP(CDlgListCtrl, CDialogEx)
	INTERFACE_PART(CDlgListCtrl, IID_IDlgListCtrl, Dispatch)
END_INTERFACE_MAP()





BOOL CDlgListCtrl::OnInitDialog()
{
	CDialogEx::OnInitDialog();
	// TODO:  在此添加额外的初始化
	m_listview = mfc::listview(&m_list);

	return TRUE; 
}


void CDlgListCtrl::OnBnClickedButton1()
{
	// TODO: 在此添加控件通知处理程序代码
	//若要多选，需要在init去掉
	mfc::_listview_list SelList;
	m_listview.getSelected(SelList);
}


void CDlgListCtrl::OnBnClickedButton2()
{
	// TODO: 在此添加控件通知处理程序代码
	m_listview.delSelected();
}


void CDlgListCtrl::OnBnClickedButton3()
{
	// TODO: 在此添加控件通知处理程序代码
	int i = rand();
	CString str;
	str.Format(L"%d", i);
	m_listview.insert(str , str, str, str);
}


void CDlgListCtrl::OnRclickListdemo(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	// TODO: 在此添加控件通知处理程序代码
	CMenu menu;
	POINT pt;
	menu.LoadMenu(IDR_MENU1);
	GetCursorPos(&pt);
	menu.GetSubMenu(0)->TrackPopupMenu(TPM_LEFTALIGN | TPM_LEFTBUTTON
		| TPM_RIGHTBUTTON, pt.x, pt.y, this, NULL);
	*pResult = 0;
}


void CDlgListCtrl::MenuAdd()
{
	// TODO: 在此添加命令处理程序代码
	MessageBox(L"Menu Add");
}


void CDlgListCtrl::MenuDel()
{
	// TODO: 在此添加命令处理程序代码
	MessageBox(L"Menu Del");
}

//保存ini
void CDlgListCtrl::OnBnClickedButton5()
{
	// TODO: 在此添加控件通知处理程序代码
	//步骤1.获得保存路径
	WCHAR filePath[MAX_PATH] = {};
	if (!usr::util::GetSaveName(GetModuleHandle(NULL), filePath, L"INI(*.INI)\0*.ini\0", L"保存"))
	{
		return;
	}
	
	//步骤2：获得列表控件中的列和每行数据
	
	mfc::_listview_list _save;
	std::vector<_tstring> sublist;
	m_listview.save(_save);
	m_listview.get_column(sublist);

	//步骤3：把列表中的数据拷贝到file中
	auto file = usr::ini_file(filePath);
	file.set(_save);
	file.set_sub_list(sublist);

	//wcscat_s(filePath, L".ini");
	//调用下file_sava
	file.save_ini();
	AfxMessageBox(filePath);
}

//加载ini
void CDlgListCtrl::OnBnClickedButton4()
{
	// TODO: 在此添加控件通知处理程序代码
	//步骤1：获得文件卢进
	WCHAR filePath[MAX_PATH] = {};
	if (!usr::util::GetOpenName(GetModuleHandle(NULL), filePath, L"INI(*.ini)\0*.ini\0", L"选择"))
		return;

	//步骤2,获得列信息
	m_listview.clear();
	auto file = usr::ini_file(filePath);
	std::vector<_tstring> subs;
	file.get_sub_list(subs);

	//步骤3，设置list_view的列
	for (auto sub : subs)
	{
		m_listview.add_column(sub);
	}

	//加载数据
	m_listview.load(file.get());

}

#pragma once
#include "CommonMFC/mfc_listview.h"

// CDlgListCtrl 对话框

class CDlgListCtrl : public CDialogEx
{
	DECLARE_DYNAMIC(CDlgListCtrl)

public:
	CDlgListCtrl(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CDlgListCtrl();

	virtual void OnFinalRelease();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_LISTDEMO };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
	DECLARE_DISPATCH_MAP()
	DECLARE_INTERFACE_MAP()
public:
	CListCtrl m_list;
	mfc::listview m_listview;
	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedButton1();
	afx_msg void OnBnClickedButton2();
	afx_msg void OnBnClickedButton3();
	afx_msg void OnRclickListdemo(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void MenuAdd();
	afx_msg void MenuDel();
	afx_msg void OnBnClickedButton5();
	afx_msg void OnBnClickedButton4();
};

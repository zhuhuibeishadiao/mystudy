
// CheckBoxDemoDlg.h : 头文件
//

#pragma once
#include "afxwin.h"


// CCheckBoxDemoDlg 对话框
class CCheckBoxDemoDlg : public CDialogEx
{
// 构造
public:
	CCheckBoxDemoDlg(CWnd* pParent = NULL);	// 标准构造函数

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_CHECKBOXDEMO_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持


// 实现
protected:
	HICON m_hIcon;

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	CButton m_chkRed;
	CButton m_chkBlue;
	CButton m_chkGreen;
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnBnClickedCheckRed();
	afx_msg void OnBnClickedCheckGreen();
	afx_msg void OnBnClickedCheckBlue();
	afx_msg void OnBnClickedRadioRect();
	afx_msg void OnBnClickedRadioCircle();
};


// DateTimeDemoDlg.h : 头文件
//

#pragma once
#include "afxdtctl.h"
#include "afxwin.h"


// CDateTimeDemoDlg 对话框
class CDateTimeDemoDlg : public CDialogEx
{
// 构造
public:
	CDateTimeDemoDlg(CWnd* pParent = NULL);	// 标准构造函数

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DATETIMEDEMO_DIALOG };
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
	CDateTimeCtrl m_date_begin;
	CDateTimeCtrl m_date_end;
	CEdit m_edit_output;
	afx_msg void OnDatetimechangeDateBegin(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnDatetimechangeDateEnd(NMHDR *pNMHDR, LRESULT *pResult);
};

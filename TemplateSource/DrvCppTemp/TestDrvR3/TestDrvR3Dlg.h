
// TestDrvR3Dlg.h : 头文件
//

#pragma once


// CTestDrvR3Dlg 对话框
class CTestDrvR3Dlg : public CDialogEx
{
// 构造
public:
	CTestDrvR3Dlg(CWnd* pParent = NULL);	// 标准构造函数

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_TESTDRVR3_DIALOG };
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
	CString m_ioctl;
	afx_msg void OnBnClickedButton5();
};

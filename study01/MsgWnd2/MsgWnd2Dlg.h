
// MsgWnd2Dlg.h : 头文件
//

#pragma once


// CMsgWnd2Dlg 对话框
class CMsgWnd2Dlg : public CDialogEx
{
// 构造
public:
	CMsgWnd2Dlg(CWnd* pParent = NULL);	// 标准构造函数

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_MSGWND2_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持


// 实现
protected:
	HICON m_hIcon;

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
	
protected:
	afx_msg LRESULT SendWnd2Fun(WPARAM wpD, LPARAM lpD);//接收消息程序，步骤2：定义消息函数
};

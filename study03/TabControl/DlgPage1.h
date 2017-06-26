#pragma once


// CDlgPage1 对话框

class CDlgPage1 : public CDialogEx
{
	DECLARE_DYNAMIC(CDlgPage1)

public:
	CDlgPage1(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CDlgPage1();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG_PAGE1 };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
};

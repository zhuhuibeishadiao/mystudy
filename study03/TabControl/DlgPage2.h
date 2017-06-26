#pragma once


// CDlgPage2 对话框

class CDlgPage2 : public CDialogEx
{
	DECLARE_DYNAMIC(CDlgPage2)

public:
	CDlgPage2(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CDlgPage2();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG_PAGE2 };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
};

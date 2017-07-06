#pragma once


// CDlgTest 对话框

class CDlgTest : public CDialogEx
{
	DECLARE_DYNAMIC(CDlgTest)

public:
	CDlgTest(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CDlgTest();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DLGTEST };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
};

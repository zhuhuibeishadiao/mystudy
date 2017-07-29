
// MainDlg.cpp : 实现文件
//

#include <boost/date_time/posix_time/posix_time.hpp>
#include "stdafx.h"
#include "MainApp.h"
#include "MainDlg.h"
#include "afxdialogex.h"
#include "CommonMFC/mfc_listview.h"
#include <boost/bind.hpp>
#include "AsioClient.h"
#include "AsioServer.h"




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


// CMainCDlg 对话框



CMainCDlg::CMainCDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_TEMPMFC_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CMainCDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CMainCDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BUTTON1, &CMainCDlg::OnBnClickedButton1)
	ON_BN_CLICKED(IDC_BUTTON2, &CMainCDlg::OnBnClickedButton2)
	ON_BN_CLICKED(IDC_BUTTON3, &CMainCDlg::OnBnClickedButton3)
	ON_BN_CLICKED(IDC_BUTTON4, &CMainCDlg::OnBnClickedButton4)
	ON_BN_CLICKED(IDC_BUTTON5, &CMainCDlg::OnBnClickedButton5)
	ON_BN_CLICKED(IDC_BUTTON6, &CMainCDlg::OnBnClickedButton6)
	ON_BN_CLICKED(IDC_BUTTON7, &CMainCDlg::OnBnClickedButton7)
	ON_BN_CLICKED(IDC_BUTTON8, &CMainCDlg::OnBnClickedButton8)
	ON_BN_CLICKED(IDC_BUTTON9, &CMainCDlg::OnBnClickedButton9)
	ON_BN_CLICKED(IDC_BUTTON10, &CMainCDlg::OnBnClickedButton10)
END_MESSAGE_MAP()


// CMainCDlg 消息处理程序

BOOL CMainCDlg::OnInitDialog()
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
	usr::util::alloc_cmd_window();


	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

void CMainCDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

void CMainCDlg::OnPaint()
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
HCURSOR CMainCDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


//////////////////////////////////////////////////////////////////////////
void CMainCDlg::OnBnClickedButton1()
{
	// TODO: 同步
	asio::io_service io;
	asio::deadline_timer timer(io, boost::posix_time::seconds(2));
	timer.wait();
	printf("你好\n");
}

//////////////////////////////////////////////////////////////////////////
void CMainCDlg::OnBnClickedButton2()
{
	// TODO: 异步无参
	asio::io_service io;
	asio::deadline_timer t(io, boost::posix_time::seconds(2));

	t.async_wait([=](const asio::error_code& err) {
		printf("操你大爷\n");
	});

	printf("我先草你妈，然后你在说话 ====》");
	io.run();
}

//////////////////////////////////////////////////////////////////////////
void AsioPrint(const asio::error_code&, asio::deadline_timer* t, int* count)
{
	if (*count < 5)
	{
		std::cout << *count << std::endl;
		++(*count);
		t->expires_at(t->expires_at() + boost::posix_time::seconds(1));
		t->async_wait(boost::bind(AsioPrint, asio::placeholders::error, t, count));
	}
}

void CMainCDlg::OnBnClickedButton3()
{
	// TODO: 异步多参
	asio::io_service io;

	int count = 0;
	asio::deadline_timer t(io, boost::posix_time::seconds(1));
	t.async_wait(boost::bind(AsioPrint, asio::placeholders::error, &t, &count));
	//t.async_wait(AsioPrint); 会出错，一定要bind

	io.run();
	std::cout << "Final count is " << count << std::endl;
}

//////////////////////////////////////////////////////////////////////////
class AsioPrintCls
{
public:
	AsioPrintCls(asio::io_service &io) : m_timer(io, boost::posix_time::seconds(1)), m_count(0)
	{
		m_timer.async_wait(boost::bind(&AsioPrintCls::print, this));
	}
	~AsioPrintCls() { std::cout << "Final count is " << m_count << std::endl; };

	void print()
	{
		if (m_count < 5)
		{
			std::cout << m_count << std::endl;
			++m_count;

			m_timer.expires_at(m_timer.expires_at() + boost::posix_time::seconds(1));
			m_timer.async_wait(boost::bind(&AsioPrintCls::print, this));
		}
	}
private:
	asio::deadline_timer m_timer;
	int m_count;
};



void CMainCDlg::OnBnClickedButton4()
{
	// TODO:异步，类函数
	asio::io_service io;
	AsioPrintCls pnt(io);
	io.run();
}

//////////////////////////////////////////////////////////////////////////

class printer
{
public:
	printer(asio::io_service& io) : m_strand(io),
		m_timer1(io, boost::posix_time::seconds(1)),
		m_timer2(io, boost::posix_time::seconds(1)),
		m_count(0)
	{
		m_timer1.async_wait(m_strand.wrap(boost::bind(&printer::print1, this)));
		m_timer2.async_wait(m_strand.wrap(boost::bind(&printer::print2, this)));
	}
	~printer() { std::cout << "Final count is " << m_count << std::endl; };

	void print1()
	{
		if (m_count < 10)
		{
			std::cout << "Timer 1: " << m_count << std::endl;
			++m_count;

			m_timer1.expires_at(m_timer1.expires_at() + boost::posix_time::seconds(1));
			m_timer1.async_wait(m_strand.wrap(boost::bind(&printer::print1, this)));
		}
	}

	void print2()
	{
		if (m_count < 10)
		{
			std::cout << "Timer 2: " << m_count << std::endl;
			++m_count;

			m_timer2.expires_at(m_timer2.expires_at() + boost::posix_time::seconds(1));
			m_timer2.async_wait(m_strand.wrap(boost::bind(&printer::print2, this)));
		}
	}
private:
	asio::io_service::strand m_strand;
	asio::deadline_timer m_timer1;
	asio::deadline_timer m_timer2;
	int m_count;
};


void CMainCDlg::OnBnClickedButton5()
{
	// TODO: 在此添加控件通知处理程序代码
	asio::io_service io;
	printer p(io);
	asio::thread t1(boost::bind(&asio::io_service::run, &io));
	io.run();
	t1.join();
}

//////////////////////////////////////////////////////////////////////////
void CMainCDlg::OnBnClickedButton6()
{
	// Step 1. Assume that the client application has already
	// obtained the DNS name and protocol port number and
	// represented them as strings.
	std::string host = "www.youku.com";
	std::string port_num = "11180";
	asio::error_code ec;
	// Step 2.
	asio::io_service ios;
	// Step 3. Creating a query.
	asio::ip::tcp::resolver::query resolver_query(host,
		port_num, asio::ip::tcp::resolver::query::numeric_service);
	// Step 4. Creating a resolver.
	asio::ip::tcp::resolver resolver(ios);
	// Step 5.
	asio::ip::tcp::resolver::iterator it =
		resolver.resolve(resolver_query, ec);
	// Handling errors if any.
	if (ec.value() != 0) {
		// Failed to resolve the DNS name. Breaking execution.
		std::cout << "Failed to resolve a DNS name."
			<< "Error code = " << ec.value()
			<< ". Message = " << ec.message();
	}

	asio::ip::tcp::resolver::iterator it_end;
	for (; it != it_end; ++it) {
		asio::ip::tcp::endpoint ep = it->endpoint();
		auto ip = ep.address();
		std::string ipStr = ip.to_string();
		printf("%s\n", ipStr.c_str());
	}
}
//////////////////////////////////////////////////////////////////////////
DWORD WINAPI AsioTCPServer1(LPVOID lpParameter)
{
	unsigned short port_num = 3333;
	try {
		Server srv;
		srv.Start(port_num);
		std::this_thread::sleep_for(std::chrono::seconds(60));
		srv.Stop();
	}
	catch (asio::system_error&e) {
		std::cout << "Error occured! Error code = "
			<< e.code() << ". Message: "
			<< e.what();
	}
	return 0;
}

void CMainCDlg::OnBnClickedButton7()
{
	// TODO: TCP服务端
	::CreateThread(NULL, 0, AsioTCPServer1, NULL, 0, NULL);
}

//////////////////////////////////////////////////////////////////////////
DWORD WINAPI AsioTcpClient1(LPVOID lpParameter)
{
	const std::string raw_ip_address = "127.0.0.1";
	const unsigned short port_num = 3333;

	try
	{
		SyncTCPClient client(raw_ip_address, port_num);
		client.connet();

		std::cout << "Sending request to the server..." << std::endl;

		std::string response = client.Foo(12);

		std::cout << "Response received: " << response << std::endl;
		client.close();
	}
	catch (asio::system_error& ec)
	{
		std::cout << "Error occured! Error code = " << ec.code() <<
			". Message: " << ec.what();
	}
	return 0;
}

void CMainCDlg::OnBnClickedButton8()
{
	// TODO: 在此添加控件通知处理程序代码
	::CreateThread(NULL, 0, AsioTcpClient1, NULL, 0, NULL);
}

//////////////////////////////////////////////////////////////////////////
void CMainCDlg::OnBnClickedButton9()
{
	// TODO: 在此添加控件通知处理程序代码

}

//////////////////////////////////////////////////////////////////////////
void CMainCDlg::OnBnClickedButton10()
{
	// TODO: 在此添加控件通知处理程序代码
}

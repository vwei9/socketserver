
// socketserverDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "ComSocket.h"
#include "socketserver.h"
#include "socketserverDlg.h"
#include "afxdialogex.h"
#include "afxwin.h"
#include "Protocol.h"  //用于转换分析


#ifdef _DEBUG
#define new DEBUG_NEW
#endif




// 用于应用程序“关于”菜单项的 CAboutDlg 对话框

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// 对话框数据
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

// 实现
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CsocketserverDlg 对话框



CsocketserverDlg::CsocketserverDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CsocketserverDlg::IDD, pParent)
{
	/******用于按钮和显示的一些变量****/
	//m_chat = _T("Please Click the Button to Start the SOCKTESERVER...");
	//m_port = 3100;
	m_con = new CComSocket();
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CsocketserverDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	/********此处为对话框与变量的关联*********/
	DDX_Control(pDX, IDC_EDIT_CHAT, m_Chat);
	//DDX_Control(pDX, IDC_EDIT_PORT, m_Port);
	DDX_Text(pDX, IDC_EDIT_PORT, m_port);
	DDV_MinMaxUInt(pDX, m_port, 1024, 65535);

}

BEGIN_MESSAGE_MAP(CsocketserverDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()

	ON_BN_CLICKED(IDC_BTN_START, &CsocketserverDlg ::OnBtnStart)
	ON_BN_CLICKED(IDC_BTN_OPEN, &CsocketserverDlg::OnBtnOpen)
	ON_BN_CLICKED(IDC_BTN_CLOSE, &CsocketserverDlg::OnBtnClose)

	ON_EN_CHANGE(IDC_EDIT_CHAT, &CsocketserverDlg::OnEnChangeEditChat)
END_MESSAGE_MAP()


// CsocketserverDlg 消息处理程序

BOOL CsocketserverDlg::OnInitDialog()
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

	// TODO:  在此添加额外的初始化代码



	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

void CsocketserverDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

void CsocketserverDlg::OnPaint()
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
HCURSOR CsocketserverDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


/************此处开始添加函数*********/

DWORD WINAPI ThreadFunc(void* Thread)
{
	CsocketserverDlg *dlg = (CsocketserverDlg*)Thread;
	while (!dlg->m_bClose)
	{
		dlg->period();
		Sleep(1000);
	}
	return 0;
}


int CsocketserverDlg::period()
{
	
		CComSocket socket;
		CProtocol protocol;
         
		GetPort();//从编辑框中读取端口号
		socket.Open();
		char buf[1024];

		
		if (!m_bFile)//判断是否成功打开文件
		{
			CString output = _T("文件打开失败，请选择正确raw文件");
			OutPutWindow(output);
			return -1;
		}
		
		std::string file = m_strFilePath.GetBuffer();
		protocol.Open(file);
		while (true)
		{
			if (protocol.IsOver())
			{
				break;
			}
			int iRev = socket.Rev(buf, 1024, 10);
			if (iRev > 0)
			{
				protocol.Rev(buf, iRev);
			}

			iRev = protocol.Send(buf);
			socket.Send(buf, iRev, 10);
			Sleep(250);
		}
		CString output2 = _T("数据发送结束");
	
}

void CsocketserverDlg::OnBtnStart()
{
	m_bStart = !m_bStart;
	if (m_bStart == true)
	{   
		m_bClose = false;
		m_Thread = ::CreateThread
			(NULL, 0, ThreadFunc, this, 0, &m_dwThreadId);
		
		SetDlgItemText(IDC_BTN_START, _T("关闭服务"));
	}
	else
	{   
		m_bClose = true;
		Sleep(500);
		::CloseHandle(m_Thread);
		m_con->Close();

		m_Thread = 0;
		SetDlgItemText(IDC_BTN_START, _T("打开服务"));
	}
}

void CsocketserverDlg::GetPort()
{
	port = GetDlgItemInt(IDC_EDIT_PORT);
}

void CsocketserverDlg::OnBtnClose()
{
	// TODO:  在此添加控件通知处理程序代码
	if (m_Thread)
	{
		m_bClose = true;
		Sleep(3000);
		::CloseHandle(m_Thread);
		m_con->Close();
	}

	CDialogEx::OnCancel();
}

void CsocketserverDlg::OnClose()
{
	m_bClose = true;
	Sleep(3000);
	::CloseHandle(m_Thread);

	m_con->Close();
	CDialogEx::OnClose();
}

//用于显示listbox
void CsocketserverDlg::OutPutWindow(CString strOutput)
{
	CString str; //获取系统时间 　　
	CTime tm; tm = CTime::GetCurrentTime();
	str = tm.Format("%Y-%m-%d %H:%M:%S ");

	str += strOutput;
	str += _T("\r\n");
	int  nLength = m_Chat.SendMessage(WM_GETTEXTLENGTH);

	if (nLength > 1024 * 10)
	{
		m_Chat.SetWindowText(_T(""));
		nLength = m_Chat.SendMessage(WM_GETTEXTLENGTH);
	}

	m_Chat.SetSel(nLength, nLength);
	m_Chat.ReplaceSel(str);

}

/**************用来打开文件*************/
void CsocketserverDlg::OnBtnOpen()
{

	TCHAR szFilter[] = _T("原始文件(*.raw)|*.raw|所有文件(*.*)|*.*||");
	// 构造打开文件对话框   
	CFileDialog fileDlg(TRUE, _T("raw"), NULL, 0, szFilter, this);


	// 显示打开文件对话框   
	if (IDOK == fileDlg.DoModal())
	{
		// 如果点击了文件对话框上的“打开”按钮，则将选择的文件路径显示到编辑框里   
		m_strFilePath = fileDlg.GetPathName();
		SetDlgItemText(IDC_EDIT_FILE, m_strFilePath);//显示文件的路径

	}

	m_bFile = true;


}

void CsocketserverDlg::OnEnChangeEditChat()
{
	// TODO:  如果该控件是 RICHEDIT 控件，它将不
	// 发送此通知，除非重写 CDialogEx::OnInitDialog()
	// 函数并调用 CRichEditCtrl().SetEventMask()，
	// 同时将 ENM_CHANGE 标志“或”运算到掩码中。

	// TODO:  在此添加控件通知处理程序代码
}

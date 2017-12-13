
// socketserverDlg.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "ComSocket.h"
#include "socketserver.h"
#include "socketserverDlg.h"
#include "afxdialogex.h"
#include "afxwin.h"
#include "Protocol.h"  //����ת������


#ifdef _DEBUG
#define new DEBUG_NEW
#endif




// ����Ӧ�ó��򡰹��ڡ��˵���� CAboutDlg �Ի���

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// �Ի�������
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��

// ʵ��
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


// CsocketserverDlg �Ի���



CsocketserverDlg::CsocketserverDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CsocketserverDlg::IDD, pParent)
{
	/******���ڰ�ť����ʾ��һЩ����****/
	//m_chat = _T("Please Click the Button to Start the SOCKTESERVER...");
	//m_port = 3100;
	m_con = new CComSocket();
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CsocketserverDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	/********�˴�Ϊ�Ի���������Ĺ���*********/
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


// CsocketserverDlg ��Ϣ�������

BOOL CsocketserverDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// ��������...���˵�����ӵ�ϵͳ�˵��С�

	// IDM_ABOUTBOX ������ϵͳ���Χ�ڡ�
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

	// ���ô˶Ի����ͼ�ꡣ  ��Ӧ�ó��������ڲ��ǶԻ���ʱ����ܽ��Զ�
	//  ִ�д˲���
	SetIcon(m_hIcon, TRUE);			// ���ô�ͼ��
	SetIcon(m_hIcon, FALSE);		// ����Сͼ��

	// TODO:  �ڴ���Ӷ���ĳ�ʼ������



	return TRUE;  // ���ǽ��������õ��ؼ������򷵻� TRUE
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

// �����Ի��������С����ť������Ҫ����Ĵ���
//  �����Ƹ�ͼ�ꡣ  ����ʹ���ĵ�/��ͼģ�͵� MFC Ӧ�ó���
//  �⽫�ɿ���Զ���ɡ�

void CsocketserverDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // ���ڻ��Ƶ��豸������

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// ʹͼ���ڹ����������о���
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// ����ͼ��
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

//���û��϶���С������ʱϵͳ���ô˺���ȡ�ù��
//��ʾ��
HCURSOR CsocketserverDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


/************�˴���ʼ��Ӻ���*********/

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
         
		GetPort();//�ӱ༭���ж�ȡ�˿ں�
		socket.Open();
		char buf[1024];

		
		if (!m_bFile)//�ж��Ƿ�ɹ����ļ�
		{
			CString output = _T("�ļ���ʧ�ܣ���ѡ����ȷraw�ļ�");
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
		CString output2 = _T("���ݷ��ͽ���");
	
}

void CsocketserverDlg::OnBtnStart()
{
	m_bStart = !m_bStart;
	if (m_bStart == true)
	{   
		m_bClose = false;
		m_Thread = ::CreateThread
			(NULL, 0, ThreadFunc, this, 0, &m_dwThreadId);
		
		SetDlgItemText(IDC_BTN_START, _T("�رշ���"));
	}
	else
	{   
		m_bClose = true;
		Sleep(500);
		::CloseHandle(m_Thread);
		m_con->Close();

		m_Thread = 0;
		SetDlgItemText(IDC_BTN_START, _T("�򿪷���"));
	}
}

void CsocketserverDlg::GetPort()
{
	port = GetDlgItemInt(IDC_EDIT_PORT);
}

void CsocketserverDlg::OnBtnClose()
{
	// TODO:  �ڴ���ӿؼ�֪ͨ����������
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

//������ʾlistbox
void CsocketserverDlg::OutPutWindow(CString strOutput)
{
	CString str; //��ȡϵͳʱ�� ����
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

/**************�������ļ�*************/
void CsocketserverDlg::OnBtnOpen()
{

	TCHAR szFilter[] = _T("ԭʼ�ļ�(*.raw)|*.raw|�����ļ�(*.*)|*.*||");
	// ������ļ��Ի���   
	CFileDialog fileDlg(TRUE, _T("raw"), NULL, 0, szFilter, this);


	// ��ʾ���ļ��Ի���   
	if (IDOK == fileDlg.DoModal())
	{
		// ���������ļ��Ի����ϵġ��򿪡���ť����ѡ����ļ�·����ʾ���༭����   
		m_strFilePath = fileDlg.GetPathName();
		SetDlgItemText(IDC_EDIT_FILE, m_strFilePath);//��ʾ�ļ���·��

	}

	m_bFile = true;


}

void CsocketserverDlg::OnEnChangeEditChat()
{
	// TODO:  ����ÿؼ��� RICHEDIT �ؼ���������
	// ���ʹ�֪ͨ��������д CDialogEx::OnInitDialog()
	// ���������� CRichEditCtrl().SetEventMask()��
	// ͬʱ�� ENM_CHANGE ��־�������㵽�����С�

	// TODO:  �ڴ���ӿؼ�֪ͨ����������
}

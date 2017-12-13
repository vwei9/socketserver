
// socketserverDlg.h : 头文件
//

#pragma once
#include "ComSocket.h"
#include"stdafx.h" //是一个预处理文件，有这个文件那么程序将自动调用#include<iostream>和<stdlib.h>
#include "afxwin.h"



// CsocketserverDlg 对话框
class CsocketserverDlg : public CDialogEx
{
// 构造
public:
	CsocketserverDlg(CWnd* pParent = NULL);	// 标准构造函数

	/*****这里是新加的变量******/
	bool m_bStart;
	bool m_bClose;
	bool m_bFile;
	UINT m_port;
	UINT port;

	CString m_strFilePath;
    
	CEdit m_Chat;
	//CEdit m_Port;

	int period();
	void GetPort();

// 对话框数据
	enum { IDD = IDD_SOCKETSERVER_DIALOG };

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

	/*****这个开始连接按钮****/
	afx_msg void OnBtnStart();
	afx_msg void OnBtnOpen();
	afx_msg void OnBtnClose();


	void OutPutWindow(CString strOutput);

	DECLARE_MESSAGE_MAP()

private:
	HANDLE m_Thread;
	DWORD m_dwThreadId;

	CCom * m_con;

public:
	afx_msg void OnClose();

	afx_msg void OnEnChangeEditChat();
};

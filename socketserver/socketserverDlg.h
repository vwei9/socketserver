
// socketserverDlg.h : ͷ�ļ�
//

#pragma once
#include "ComSocket.h"
#include"stdafx.h" //��һ��Ԥ�����ļ���������ļ���ô�����Զ�����#include<iostream>��<stdlib.h>
#include "afxwin.h"



// CsocketserverDlg �Ի���
class CsocketserverDlg : public CDialogEx
{
// ����
public:
	CsocketserverDlg(CWnd* pParent = NULL);	// ��׼���캯��

	/*****�������¼ӵı���******/
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

// �Ի�������
	enum { IDD = IDD_SOCKETSERVER_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV ֧��


// ʵ��
protected:
	HICON m_hIcon;

	// ���ɵ���Ϣӳ�亯��
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();

	/*****�����ʼ���Ӱ�ť****/
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

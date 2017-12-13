
#pragma once
#include <WINSOCK2.H>
#include "CCom.h"
#pragma comment(lib,"WS2_32.lib") 

#define   DATA_BUFSIZE   1024 

// �����׽�����Ϣ
typedef   struct   _SOCKET_INFORMATION   {
	WSABUF   DataBufRead;                       // ����������ݻ������Ľṹ�壬�����������ĳ��Ⱥ�����
	WSABUF   DataBufSend;                       // ����������ݻ������Ľṹ�壬�����������ĳ��Ⱥ�����
	SOCKET   Socket;                           // ��ͻ��˽���ͨ�ŵ��׽���
} SOCKET_INFORMATION, *LPSOCKET_INFORMATION;

class CComSocket : public CCom
{
public:
	CComSocket();

	int Open();

	virtual int Send(char* buf, int iSize,long time);

	virtual int Rev(char* buf, int iMaxSize,long time);

	virtual int Close();

private:
	BOOL  CreateSocketInformation(SOCKET   s);
	void  FreeSocketInformation(DWORD   Index);
private:

	DWORD   TotalSockets ;															// ��¼����ʹ�õ��׽���������
	LPSOCKET_INFORMATION   SocketArray[FD_SETSIZE];            // ����Socket��Ϣ��������飬FD_SETSIZE��ʾSELECTģ�������������׽�������

	SOCKET   ListenSocket;                    // �����׽���
	SOCKET   AcceptSocket;                    // ��ͻ��˽���ͨ�ŵ��׽���
	SOCKADDR_IN   InternetAddr;            // �������ĵ�ַ
	WSADATA   wsaData;                        // ���ڳ�ʼ���׽��ֻ���
	INT   Ret;                                            // WinSock API�ķ���ֵ
	FD_SET   WriteSet;                            // ��ȡ��д�Ե��׽��ּ���
	FD_SET   ReadSet;                            // ��ȡ�ɶ��Ե��׽��ּ���
	DWORD   Total;                                // ���ھ���״̬���׽�������
	DWORD   SendBytes;                        // ���͵��ֽ���
	DWORD   RecvBytes;                        // ���յ��ֽ���
};
#include "stdafx.h"
#include "ComSocket.h"
#include <stdio.h>
#include <iostream>

using namespace std;

CComSocket::CComSocket() : TotalSockets(0)
{

}

int CComSocket::Open()
{
	USHORT port = 3100;
	// ��ʼ��WinSock����
	if ((Ret = WSAStartup(MAKEWORD(2, 2), &wsaData)) != 0)
	{
		printf("WSAStartup()   failed   with   error   %d\n", Ret);
		WSACleanup();
		return -1;
	}
	// �������ڼ������׽��� 
	if ((ListenSocket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED)) == INVALID_SOCKET)
	{
		printf("WSASocket()   failed   with   error   %d\n", WSAGetLastError());
		return -1;
	}
	// ���ü�����ַ�Ͷ˿ں�
	InternetAddr.sin_family = AF_INET;
	InternetAddr.sin_addr.S_un.S_addr = htonl(INADDR_ANY);
	InternetAddr.sin_port = htons(port);
	// �󶨼����׽��ֵ����ص�ַ�Ͷ˿�
	if (bind(ListenSocket, (PSOCKADDR)&InternetAddr, sizeof(InternetAddr)) == SOCKET_ERROR)
	{
		printf("bind()   failed   with   error   %d\n", WSAGetLastError());
		return -1;
	}
	// ��ʼ����
	if (listen(ListenSocket, 5))
	{
		printf("listen()   failed   with   error   %d\n", WSAGetLastError());
		return -1;
	}
	// ����Ϊ������ģʽ
	ULONG NonBlock = 1;
	if (ioctlsocket(ListenSocket, FIONBIO, &NonBlock) == SOCKET_ERROR)
	{
		printf("ioctlsocket() failed with error %d\n", WSAGetLastError());
		return -1;
	}
	CreateSocketInformation(ListenSocket);// ΪListenSocket�׽��ִ�����Ӧ��SOCKET_INFORMATION,��ListenSocket��ӵ�SocketArray������
	return 0;
}

int CComSocket::Send(char* buf, int iSize,long time)
{
	// ��SocketArray�����е������׽�����ӵ�WriteSet��ReadSet������,SocketArray�����б����ż����׽��ֺ�������ͻ��˽���ͨ�ŵ��׽���
	// �����Ϳ���ʹ��select()�ж��ĸ��׽����н������ݻ��߶�ȡ/д������
	FD_ZERO(&WriteSet);// ׼����������I/O֪ͨ�Ķ�/д�׽��ּ���
	for (DWORD i = 0; i < TotalSockets; i++)
	{
		LPSOCKET_INFORMATION SocketInfo = SocketArray[i];
		FD_SET(SocketInfo->Socket, &WriteSet);							//��˵����socket�ж����������������ǿͻ��˷���ġ�
	}
	timeval timeout;
	timeout.tv_sec = 0;
	timeout.tv_usec = time;

	// �ж϶�/д�׽��ּ����о������׽���    
	if ((Total = select(0, NULL, &WriteSet, NULL, &timeout)) == SOCKET_ERROR)//��NULL���βδ���Timeout����������ʱ��ṹ�����ǽ�select��������״̬��һ���ȵ������ļ�������������ĳ���ļ������������仯Ϊֹ.��������ͣ������ȴ��ͻ�����Ӧ
	{
		printf("select()   returned   with   error   %d\n", WSAGetLastError());
		return -1;
	}
	else if (Total == 0)
	{
		return -1;
	}
	else
	{
		// ���δ��������׽��֡�����������һ����Ӧ�������������ӿͻ����յ����ַ����ٷ��ص��ͻ��ˡ�
		for (DWORD i = 0; i < TotalSockets; i++)
		{
			LPSOCKET_INFORMATION SocketInfo = SocketArray[i];            // SocketInfoΪ��ǰҪ������׽�����Ϣ

			// �����ǰ�׽�����WriteSet�����У���������׽��ֵ��ڲ����ݻ������������ݿ��Է���
			if (FD_ISSET(SocketInfo->Socket, &WriteSet))
			{
				Total--;            // ����һ�����ھ���״̬���׽���
				if (iSize > 0)        // �������Ҫ���͵����ݣ���������
				{
					SocketInfo->DataBufSend.buf = buf;
					SocketInfo->DataBufSend.len = iSize;
					if (WSASend(SocketInfo->Socket, &(SocketInfo->DataBufSend), 1, &SendBytes, 0, NULL, NULL) == SOCKET_ERROR)
					{
						// ����������WSAEWOULDBLOCK��ʾ��û�����ݣ������ʾ�����쳣
						if (WSAGetLastError() != WSAEWOULDBLOCK)
						{
							printf("WSASend()   failed   with   error   %d\n", WSAGetLastError());
							FreeSocketInformation(i);        // �ͷ��׽�����Ϣ
						}
						continue;
					}
					else
					{
						return SendBytes;
					}
				}
			}
		}
		return 0;
	}
	return -1;
}

int CComSocket::Rev(char* buf, int iMaxSize,long time)
{
	FD_ZERO(&ReadSet);// ׼����������I/O֪ͨ�Ķ�/д�׽��ּ���

	// ��SocketArray�����е������׽�����ӵ�WriteSet��ReadSet������,SocketArray�����б����ż����׽��ֺ�������ͻ��˽���ͨ�ŵ��׽���
	// �����Ϳ���ʹ��select()�ж��ĸ��׽����н������ݻ��߶�ȡ/д������
	for (DWORD i = 0; i < TotalSockets; i++)
	{
		LPSOCKET_INFORMATION SocketInfo = SocketArray[i];
		FD_SET(SocketInfo->Socket, &ReadSet);							//��˵����socket�ж����������������ǿͻ��˷���ġ�
	}
	timeval timeout;
	timeout.tv_sec = 0;
	timeout.tv_usec = time;
	// �ж϶�/д�׽��ּ����о������׽���    
	if ((Total = select(0, &ReadSet, NULL, NULL, &timeout)) == SOCKET_ERROR)//��NULL���βδ���Timeout����������ʱ��ṹ�����ǽ�select��������״̬��һ���ȵ������ļ�������������ĳ���ļ������������仯Ϊֹ.��������ͣ������ȴ��ͻ�����Ӧ
	{
		printf("select()   returned   with   error   %d\n", WSAGetLastError());
		return -1;
	}
	else if (Total == 0)
	{
		return 0;
	}
	else
	{
		// ���δ��������׽��֡�����������һ����Ӧ�������������ӿͻ����յ����ַ����ٷ��ص��ͻ��ˡ�
		for (DWORD i = 0; i < TotalSockets; i++)
		{
			LPSOCKET_INFORMATION SocketInfo = SocketArray[i];            // SocketInfoΪ��ǰҪ������׽�����Ϣ
			// �жϵ�ǰ�׽��ֵĿɶ��ԣ����Ƿ��н��������������߿��Խ�������
			if (FD_ISSET(SocketInfo->Socket, &ReadSet))
			{
				if (SocketInfo->Socket == ListenSocket)        // ���ڼ����׽�����˵���ɶ���ʾ���µ���������
				{
					Total--;    // �������׽��ּ�1
					// �����������󣬵õ���ͻ��˽���ͨ�ŵ��׽���AcceptSocket
					if ((AcceptSocket = accept(ListenSocket, NULL, NULL)) != INVALID_SOCKET)
					{
						// �����׽���AcceptSocketΪ������ģʽ
						// �����������ڵ���WSASend()������������ʱ�Ͳ��ᱻ����
						ULONG NonBlock = 1;
						if (ioctlsocket(AcceptSocket, FIONBIO, &NonBlock) == SOCKET_ERROR)
						{
							printf("ioctlsocket()   failed   with   error   %d\n", WSAGetLastError());
							return -1;
						}
						// �����׽�����Ϣ����ʼ��LPSOCKET_INFORMATION�ṹ�����ݣ���AcceptSocket��ӵ�SocketArray������
						if (CreateSocketInformation(AcceptSocket) == FALSE)
							return -1;
					}
					else
					{
						if (WSAGetLastError() != WSAEWOULDBLOCK)
						{
							printf("accept()   failed   with   error   %d\n", WSAGetLastError());
							return -1;
						}
					}
				}
				else   // ��������
				{
					Total--;                // ����һ�����ھ���״̬���׽���
					SocketInfo->DataBufRead.buf = buf;								// ��ʼ��������λ��
					SocketInfo->DataBufRead.len = iMaxSize;					    // ��ʼ������������
					// ��������
					DWORD  Flags = 0;
					if (WSARecv(SocketInfo->Socket, &(SocketInfo->DataBufRead), 1, &RecvBytes, &Flags, NULL, NULL) == SOCKET_ERROR)
					{
						// ����������WSAEWOULDBLOCK��ʾ��û�����ݣ������ʾ�����쳣
						if (WSAGetLastError() != WSAEWOULDBLOCK)
						{
							printf("WSARecv()   failed   with   error   %d\n", WSAGetLastError());
							FreeSocketInformation(i);        // �ͷ��׽�����Ϣ
						}
						continue;
					}
					else   // ��������
					{
						if (RecvBytes == 0)                                    // ������յ�0���ֽڣ����ʾ�Է��ر�����
						{
							FreeSocketInformation(i);
							continue;
						}
						else
						{
							memcpy(buf, SocketInfo->DataBufRead.buf, RecvBytes);
							return RecvBytes;
						}
					}
				}
			}
		}
	}
	return 0;
}

int CComSocket::Close()
{
	// ���δ��������׽��֡�����������һ����Ӧ�������������ӿͻ����յ����ַ����ٷ��ص��ͻ��ˡ�
	for (DWORD i = 0; i < TotalSockets; i++)
	{
		FreeSocketInformation(i);
	}
	WSACleanup();
	return 0;
}

// ����SOCKET��Ϣ
BOOL   CComSocket::CreateSocketInformation(SOCKET   s)
{
	LPSOCKET_INFORMATION   SI;                                        // ���ڱ����׽��ֵ���Ϣ       
	//   printf("Accepted   socket   number   %d\n",   s);            // ���ѽ��ܵ��׽��ֱ��
	// ΪSI�����ڴ�ռ�
	if ((SI = (LPSOCKET_INFORMATION)GlobalAlloc(GPTR, sizeof(SOCKET_INFORMATION))) == NULL)
	{
		printf("GlobalAlloc()   failed   with   error   %d\n", GetLastError());
		return   FALSE;
	}
	// ��ʼ��SI��ֵ    
	SI->Socket = s;

	// ��SocketArray����������һ����Ԫ�أ����ڱ���SI���� 
	SocketArray[TotalSockets] = SI;
	TotalSockets++;                        // �����׽�������

	return(TRUE);
}

// ������SocketArray��ɾ��ָ����LPSOCKET_INFORMATION����
void  CComSocket::FreeSocketInformation(DWORD   Index)
{
	LPSOCKET_INFORMATION SI = SocketArray[Index];    // ��ȡָ��������Ӧ��LPSOCKET_INFORMATION����
	DWORD   i;

	closesocket(SI->Socket);       // �ر��׽���
	GlobalFree(SI);   // �ͷ�ָ��LPSOCKET_INFORMATION������Դ
	// ��������index���������Ԫ��ǰ��
	if (Index != (TotalSockets - 1))
	{
		for (i = Index; i < TotalSockets; i++)
		{
			SocketArray[i] = SocketArray[i + 1];
		}
	}

	TotalSockets--;        // �׽���������1
}
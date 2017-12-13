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
	// 初始化WinSock环境
	if ((Ret = WSAStartup(MAKEWORD(2, 2), &wsaData)) != 0)
	{
		printf("WSAStartup()   failed   with   error   %d\n", Ret);
		WSACleanup();
		return -1;
	}
	// 创建用于监听的套接字 
	if ((ListenSocket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED)) == INVALID_SOCKET)
	{
		printf("WSASocket()   failed   with   error   %d\n", WSAGetLastError());
		return -1;
	}
	// 设置监听地址和端口号
	InternetAddr.sin_family = AF_INET;
	InternetAddr.sin_addr.S_un.S_addr = htonl(INADDR_ANY);
	InternetAddr.sin_port = htons(port);
	// 绑定监听套接字到本地地址和端口
	if (bind(ListenSocket, (PSOCKADDR)&InternetAddr, sizeof(InternetAddr)) == SOCKET_ERROR)
	{
		printf("bind()   failed   with   error   %d\n", WSAGetLastError());
		return -1;
	}
	// 开始监听
	if (listen(ListenSocket, 5))
	{
		printf("listen()   failed   with   error   %d\n", WSAGetLastError());
		return -1;
	}
	// 设置为非阻塞模式
	ULONG NonBlock = 1;
	if (ioctlsocket(ListenSocket, FIONBIO, &NonBlock) == SOCKET_ERROR)
	{
		printf("ioctlsocket() failed with error %d\n", WSAGetLastError());
		return -1;
	}
	CreateSocketInformation(ListenSocket);// 为ListenSocket套接字创建对应的SOCKET_INFORMATION,把ListenSocket添加到SocketArray数组中
	return 0;
}

int CComSocket::Send(char* buf, int iSize,long time)
{
	// 将SocketArray数组中的所有套接字添加到WriteSet和ReadSet集合中,SocketArray数组中保存着监听套接字和所有与客户端进行通信的套接字
	// 这样就可以使用select()判断哪个套接字有接入数据或者读取/写入数据
	FD_ZERO(&WriteSet);// 准备用于网络I/O通知的读/写套接字集合
	for (DWORD i = 0; i < TotalSockets; i++)
	{
		LPSOCKET_INFORMATION SocketInfo = SocketArray[i];
		FD_SET(SocketInfo->Socket, &WriteSet);							//这说明该socket有读操作。而读操作是客户端发起的。
	}
	timeval timeout;
	timeout.tv_sec = 0;
	timeout.tv_usec = time;

	// 判断读/写套接字集合中就绪的套接字    
	if ((Total = select(0, NULL, &WriteSet, NULL, &timeout)) == SOCKET_ERROR)//将NULL以形参传入Timeout，即不传入时间结构，就是将select置于阻塞状态，一定等到监视文件描述符集合中某个文件描述符发生变化为止.服务器会停到这里等待客户端相应
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
		// 依次处理所有套接字。本服务器是一个回应服务器，即将从客户端收到的字符串再发回到客户端。
		for (DWORD i = 0; i < TotalSockets; i++)
		{
			LPSOCKET_INFORMATION SocketInfo = SocketArray[i];            // SocketInfo为当前要处理的套接字信息

			// 如果当前套接字在WriteSet集合中，则表明该套接字的内部数据缓冲区中有数据可以发送
			if (FD_ISSET(SocketInfo->Socket, &WriteSet))
			{
				Total--;            // 减少一个处于就绪状态的套接字
				if (iSize > 0)        // 如果有需要发送的数据，则发送数据
				{
					SocketInfo->DataBufSend.buf = buf;
					SocketInfo->DataBufSend.len = iSize;
					if (WSASend(SocketInfo->Socket, &(SocketInfo->DataBufSend), 1, &SendBytes, 0, NULL, NULL) == SOCKET_ERROR)
					{
						// 错误编码等于WSAEWOULDBLOCK表示暂没有数据，否则表示出现异常
						if (WSAGetLastError() != WSAEWOULDBLOCK)
						{
							printf("WSASend()   failed   with   error   %d\n", WSAGetLastError());
							FreeSocketInformation(i);        // 释放套接字信息
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
	FD_ZERO(&ReadSet);// 准备用于网络I/O通知的读/写套接字集合

	// 将SocketArray数组中的所有套接字添加到WriteSet和ReadSet集合中,SocketArray数组中保存着监听套接字和所有与客户端进行通信的套接字
	// 这样就可以使用select()判断哪个套接字有接入数据或者读取/写入数据
	for (DWORD i = 0; i < TotalSockets; i++)
	{
		LPSOCKET_INFORMATION SocketInfo = SocketArray[i];
		FD_SET(SocketInfo->Socket, &ReadSet);							//这说明该socket有读操作。而读操作是客户端发起的。
	}
	timeval timeout;
	timeout.tv_sec = 0;
	timeout.tv_usec = time;
	// 判断读/写套接字集合中就绪的套接字    
	if ((Total = select(0, &ReadSet, NULL, NULL, &timeout)) == SOCKET_ERROR)//将NULL以形参传入Timeout，即不传入时间结构，就是将select置于阻塞状态，一定等到监视文件描述符集合中某个文件描述符发生变化为止.服务器会停到这里等待客户端相应
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
		// 依次处理所有套接字。本服务器是一个回应服务器，即将从客户端收到的字符串再发回到客户端。
		for (DWORD i = 0; i < TotalSockets; i++)
		{
			LPSOCKET_INFORMATION SocketInfo = SocketArray[i];            // SocketInfo为当前要处理的套接字信息
			// 判断当前套接字的可读性，即是否有接入的连接请求或者可以接收数据
			if (FD_ISSET(SocketInfo->Socket, &ReadSet))
			{
				if (SocketInfo->Socket == ListenSocket)        // 对于监听套接字来说，可读表示有新的连接请求
				{
					Total--;    // 就绪的套接字减1
					// 接受连接请求，得到与客户端进行通信的套接字AcceptSocket
					if ((AcceptSocket = accept(ListenSocket, NULL, NULL)) != INVALID_SOCKET)
					{
						// 设置套接字AcceptSocket为非阻塞模式
						// 这样服务器在调用WSASend()函数发送数据时就不会被阻塞
						ULONG NonBlock = 1;
						if (ioctlsocket(AcceptSocket, FIONBIO, &NonBlock) == SOCKET_ERROR)
						{
							printf("ioctlsocket()   failed   with   error   %d\n", WSAGetLastError());
							return -1;
						}
						// 创建套接字信息，初始化LPSOCKET_INFORMATION结构体数据，将AcceptSocket添加到SocketArray数组中
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
				else   // 接收数据
				{
					Total--;                // 减少一个处于就绪状态的套接字
					SocketInfo->DataBufRead.buf = buf;								// 初始化缓冲区位置
					SocketInfo->DataBufRead.len = iMaxSize;					    // 初始化缓冲区长度
					// 接收数据
					DWORD  Flags = 0;
					if (WSARecv(SocketInfo->Socket, &(SocketInfo->DataBufRead), 1, &RecvBytes, &Flags, NULL, NULL) == SOCKET_ERROR)
					{
						// 错误编码等于WSAEWOULDBLOCK表示暂没有数据，否则表示出现异常
						if (WSAGetLastError() != WSAEWOULDBLOCK)
						{
							printf("WSARecv()   failed   with   error   %d\n", WSAGetLastError());
							FreeSocketInformation(i);        // 释放套接字信息
						}
						continue;
					}
					else   // 接收数据
					{
						if (RecvBytes == 0)                                    // 如果接收到0个字节，则表示对方关闭连接
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
	// 依次处理所有套接字。本服务器是一个回应服务器，即将从客户端收到的字符串再发回到客户端。
	for (DWORD i = 0; i < TotalSockets; i++)
	{
		FreeSocketInformation(i);
	}
	WSACleanup();
	return 0;
}

// 创建SOCKET信息
BOOL   CComSocket::CreateSocketInformation(SOCKET   s)
{
	LPSOCKET_INFORMATION   SI;                                        // 用于保存套接字的信息       
	//   printf("Accepted   socket   number   %d\n",   s);            // 打开已接受的套接字编号
	// 为SI分配内存空间
	if ((SI = (LPSOCKET_INFORMATION)GlobalAlloc(GPTR, sizeof(SOCKET_INFORMATION))) == NULL)
	{
		printf("GlobalAlloc()   failed   with   error   %d\n", GetLastError());
		return   FALSE;
	}
	// 初始化SI的值    
	SI->Socket = s;

	// 在SocketArray数组中增加一个新元素，用于保存SI对象 
	SocketArray[TotalSockets] = SI;
	TotalSockets++;                        // 增加套接字数量

	return(TRUE);
}

// 从数组SocketArray中删除指定的LPSOCKET_INFORMATION对象
void  CComSocket::FreeSocketInformation(DWORD   Index)
{
	LPSOCKET_INFORMATION SI = SocketArray[Index];    // 获取指定索引对应的LPSOCKET_INFORMATION对象
	DWORD   i;

	closesocket(SI->Socket);       // 关闭套接字
	GlobalFree(SI);   // 释放指定LPSOCKET_INFORMATION对象资源
	// 将数组中index索引后面的元素前移
	if (Index != (TotalSockets - 1))
	{
		for (i = Index; i < TotalSockets; i++)
		{
			SocketArray[i] = SocketArray[i + 1];
		}
	}

	TotalSockets--;        // 套接字总数减1
}
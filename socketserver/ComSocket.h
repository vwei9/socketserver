
#pragma once
#include <WINSOCK2.H>
#include "CCom.h"
#pragma comment(lib,"WS2_32.lib") 

#define   DATA_BUFSIZE   1024 

// 定义套接字信息
typedef   struct   _SOCKET_INFORMATION   {
	WSABUF   DataBufRead;                       // 定义接收数据缓冲区的结构体，包括缓冲区的长度和内容
	WSABUF   DataBufSend;                       // 定义接收数据缓冲区的结构体，包括缓冲区的长度和内容
	SOCKET   Socket;                           // 与客户端进行通信的套接字
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

	DWORD   TotalSockets ;															// 记录正在使用的套接字总数量
	LPSOCKET_INFORMATION   SocketArray[FD_SETSIZE];            // 保存Socket信息对象的数组，FD_SETSIZE表示SELECT模型中允许的最大套接字数量

	SOCKET   ListenSocket;                    // 监听套接字
	SOCKET   AcceptSocket;                    // 与客户端进行通信的套接字
	SOCKADDR_IN   InternetAddr;            // 服务器的地址
	WSADATA   wsaData;                        // 用于初始化套接字环境
	INT   Ret;                                            // WinSock API的返回值
	FD_SET   WriteSet;                            // 获取可写性的套接字集合
	FD_SET   ReadSet;                            // 获取可读性的套接字集合
	DWORD   Total;                                // 处于就绪状态的套接字数量
	DWORD   SendBytes;                        // 发送的字节数
	DWORD   RecvBytes;                        // 接收的字节数
};
#include "stdafx.h"
#include "./Protocol.h"


#define GUDINGSHI_PACKAGE_SIZE 783

CProtocol::CProtocol()
{
	bSend = false;
}

int CProtocol::Open(std::string name)
{
	file.open(name, ios::binary | ios::in);
	if (file.is_open() == true)
	{
		return 0;
	}
	return -1;
}


int CProtocol::Send(char* buf)
{
	if (bSend)
	{
		if (file.is_open() == true && !file.eof())
		{
			file.read(buf, GUDINGSHI_PACKAGE_SIZE);	// 最后一行会多读一次，但因为在加入点时会因为编码器数值相同而删除，因此不影响
			return GUDINGSHI_PACKAGE_SIZE;
		}
	}

	return 0;
}

bool CProtocol::IsOver()
{
	if (file.is_open() == true && !file.eof())
	{
		return false;
	}
	else
	{
		return true;
	}
}


int CProtocol::Rev(char* buf, int size)
{
	if (!bSend)
	{
		char str[] = { "start connection" };
		if (sizeof(str) - 1 == size)
		{
			for (int i = 0; i < size; i++)
			{
				if (str[i] != buf[i])
				{
					return 0;
				}
			}
		}
		printf("开始\n");
		bSend = true;
	}
	else
	{
		char str[] = { "end connection" };
		if (sizeof(str) - 1== size)
		{
			for (int i = 0; i < size; i++)
			{
				if (str[i] != buf[i])
				{
					return 0;
				}
			}
		}
		printf("结束\n");
		bSend = true;
	}

	return 0;
}
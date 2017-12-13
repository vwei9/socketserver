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
			file.read(buf, GUDINGSHI_PACKAGE_SIZE);	// ���һ�л���һ�Σ�����Ϊ�ڼ����ʱ����Ϊ��������ֵ��ͬ��ɾ������˲�Ӱ��
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
		printf("��ʼ\n");
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
		printf("����\n");
		bSend = true;
	}

	return 0;
}
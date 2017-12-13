#pragma once
#include <string>
#include <fstream>
using namespace  std;


//! Э�������
class CProtocol
{
public:
	CProtocol();

	int Open(std::string name);

	int Send(char* buf);
	int Rev(char* buf,int size);

	bool IsOver();
private:
	std::fstream file;

	string strLog;

	bool bSend;
};
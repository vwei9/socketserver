#pragma once
class CCom
{
public:
	virtual int Open() = 0;
	virtual int Send(char* buf, int iSize, long time) = 0;
	virtual int Rev(char* buf, int iMaxSize, long time) = 0;
	virtual int Close() = 0;
};
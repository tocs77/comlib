#pragma once
#include<string>
#include<Windows.h>

#ifndef IComPort_ConfigH
#define IComPort_ConfigH

static const std::wstring portNumberMask = L"\\\\.\\COM";

enum class baudrates {
	BR_110 = 110, BR_300 = 300, BR_600 = 600, BR_1200 = 1200, BR_2400 = 2400,
	BR_4800 = 4800, BR_9600 = 9600, BR_14400 = 14400, BR_19200 = 19200,
	BR_38400 = 38400, BR_56000 = 56000, BR_57600 = 57600, BR_115200 = 115200,
	BR_128000 = 128000, BR_25600 = 256000
};

//-----------------------------------------------------------------------------
class IComPort_Config
{


public:

	IComPort_Config();
	void setBaudrate(baudrates bd);
	void setPort(INT portNumber);
	void setWriteTimeOut(INT timeOutVal);
	void setReadTimeOut(INT timeOutVal);


protected:

	baudrates baudrate;
	std::wstring portNumber;
	INT iWriteTimeOut;
	INT iReadTimeOut;

};
//-----------------------------------------------------------------------------
#endif
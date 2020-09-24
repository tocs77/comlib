#include <string>
#include<Windows.h>

#include "IComPort_Config.h"

IComPort_Config::IComPort_Config() {
	baudrate = baudrates::BR_9600;
	portNumber = L"";
	iWriteTimeOut = 300;
	iReadTimeOut = 300;
}


void IComPort_Config::setBaudrate(baudrates bd) {
	baudrate = bd;
};

void IComPort_Config::setPort(INT portNumber) {
	this->portNumber = portNumberMask + std::to_wstring(portNumber);
}

void IComPort_Config::setWriteTimeOut(INT timeOutvalue) {
	iWriteTimeOut = timeOutvalue;
}

void IComPort_Config::setReadTimeOut(INT timeOutvalue) {
	iReadTimeOut = timeOutvalue;
}
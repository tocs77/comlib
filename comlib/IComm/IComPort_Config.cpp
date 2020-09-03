#include <string>
#include<Windows.h>

#include "IComPort_Config.h"

IComPort_Config::IComPort_Config() {
	baudrate = baudrates::BR_9600;
	portNumber = L"";
}


void IComPort_Config::setBaudrate(baudrates bd) {
	baudrate = bd;
};

void IComPort_Config::setPort(int portNumber) {
	this->portNumber = portNumberMask + std::to_wstring(portNumber);
}
#include <iostream>

#include "CommAsync/CommPortAsync.h"
#include "Logger/Logger.h"
//#include "CommAsync/CommPortAsync.cpp"


int main() {
	Logger::startLog("Log.txt");
	TCommPortAsync tCom;
	//tCom.setBaudrate(baudrates::BR_115200);
	//tCom.setPort(14);
	//tCom.Connect();
	
	Logger::logInfo("Something went wrong.");

	Logger::logWarning("Something went wrong.");
}

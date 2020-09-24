#include <iostream>

#include "CommAsync/CommPortAsync.h"
#include "Logger/Logger.h"


int main() {
	Logger::startLog("Log.txt");
	TCommPortAsync tCom;
	tCom.setBaudrate(baudrates::BR_115200);
	tCom.setPort(3);
	tCom.Connect();
	tCom.Start();

	const int writeLenght = 5;

	BYTE writeBuffer[writeLenght]{ 'T', 'e', 's' ,'t', '\n' };
	DWORD dwBytesWritten = writeLenght;

	BYTE* resultBuf = nullptr;
	DWORD* resultLength = nullptr;

	Sleep(2000);

	Logger::logInfo("Делаем запрос в основном потоке");
	tCom.WriteRead(writeBuffer, writeLenght, resultBuf, resultLength);
	Sleep(3000);
}

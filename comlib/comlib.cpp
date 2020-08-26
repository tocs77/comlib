//#include <iostream>
//#include <Windows.h>
//
//#include <string>
//
//
//
//
//void ReadCOM(HANDLE& hSerial)
//{
//	DWORD iSize;
//	char sReceivedChar;
//	while (true)
//	{
//		ReadFile(hSerial, &sReceivedChar, 1, &iSize, 0);  // Get one byte
//		if (iSize > 0)   // If got something then show
//			std::cout << sReceivedChar;
//	}
//}
//
//int main() {
//
//	LPCWSTR szPortName = L"\\\\.\\COM2";
//	HANDLE serialPort = CreateFile(szPortName,
//		GENERIC_READ | GENERIC_WRITE,  // access ( read and write)
//		0,                           // (share) 0:cannot share the COM port
//		0,                           // security  (None)
//		OPEN_EXISTING,               // creation : open_existing
//		FILE_FLAG_OVERLAPPED,        // we want overlapped operation
//		0                            // no templates file for COM port...
//	);
//
//	if (serialPort != INVALID_HANDLE_VALUE) {
//		std::cout << "open!\n";
//	}
//	else {
//		if (GetLastError() == ERROR_FILE_NOT_FOUND)
//		{
//			std::cout << "serial port does not exist.\n";
//		}
//		std::cout << "some other error occurred.\n";
//	}
//
//	DCB dcb = { 0 };
//	dcb.DCBlength = sizeof(DCB);
//
//	if (!::GetCommState(serialPort, &dcb))
//	{
//		std::cout << "CSerialCommHelper : Failed to Get Comm State Reason: " << GetLastError() << std::endl;
//			return E_FAIL;
//	}
//
//	dcb.BaudRate = CBR_115200;
//	dcb.ByteSize = 8;
//	dcb.StopBits = ONESTOPBIT;
//	dcb.Parity = NOPARITY;
//	if (!SetCommState(serialPort, &dcb))
//	{
//		std::cout << "error setting serial port state\n";
//	}
//
//	while (1)
//	{
//		ReadCOM(serialPort);
//	}
//	return 0;
//}
//

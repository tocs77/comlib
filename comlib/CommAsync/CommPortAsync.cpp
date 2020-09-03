//#pragma on
//#pragma hdrstop
#include <iostream>
#include "CommPortAsync.h"

//#pragma package(smart_init)
//---------------------------------------------------------------------------

TCommPortAsync::TCommPortAsync()
{
	hPort = INVALID_HANDLE_VALUE;
	for (int i = 0; i < EVENT_NUMBER; i++) {
		events[i] = CreateEvent(NULL, TRUE, FALSE, NULL);
	}
	readHandle = NULL;
}



TCommPortAsync::~TCommPortAsync()
{
	//readHandle->detach();
	Disconnect();
	for (int i = 0; i < EVENT_NUMBER; i++) {
		CloseHandle(events[i]);
	}

}

BOOL TCommPortAsync::Connect() {
	Disconnect();
	bool result;

	hPort =
		CreateFile(
			(LPCWSTR)portNumber.c_str(),     //port name 
			GENERIC_READ | GENERIC_WRITE,   // acces to read and write
			0,                              // shared mode, should be zero for com port
			NULL,                           // security for file, should be zero for com port
			OPEN_EXISTING,                  // for port open existing
			FILE_FLAG_OVERLAPPED,          // FILE_FLAG_OVERLAPPED for async work
			NULL);                          // template file, should be zero for com port


	if (hPort == INVALID_HANDLE_VALUE) {
		return FALSE;
	}

	result = SetCommMask(hPort, EV_RXCHAR | EV_TXEMPTY);
	if (!result) {
		return FALSE;
	}
	SetupComm(hPort, 1500, 1500);

	COMMTIMEOUTS CommTimeOuts;
	CommTimeOuts.ReadIntervalTimeout = 20;
	CommTimeOuts.ReadTotalTimeoutMultiplier = 10;
	CommTimeOuts.ReadTotalTimeoutConstant = 70;
	CommTimeOuts.WriteTotalTimeoutMultiplier = 4;
	CommTimeOuts.WriteTotalTimeoutConstant = 100;

	if (!SetCommTimeouts(hPort, &CommTimeOuts)) {
		CloseHandle(hPort);
		hPort = INVALID_HANDLE_VALUE;
		return FALSE;
	}

	DCB ComDCM;

	memset(&ComDCM, 0, sizeof(ComDCM));
	ComDCM.DCBlength = sizeof(DCB);
	GetCommState(hPort, &ComDCM);
	ComDCM.BaudRate = static_cast<DWORD>(baudrate);
	ComDCM.ByteSize = 8;
	ComDCM.Parity = NOPARITY;
	ComDCM.StopBits = ONESTOPBIT;
	ComDCM.fAbortOnError = TRUE;
	ComDCM.fDtrControl = DTR_CONTROL_DISABLE;
	ComDCM.fRtsControl = RTS_CONTROL_DISABLE;
	ComDCM.fBinary = TRUE;
	ComDCM.fParity = FALSE;
	ComDCM.fInX = FALSE;
	ComDCM.fOutX = FALSE;
	ComDCM.XonChar = 0;
	ComDCM.XoffChar = (unsigned char)0xFF;
	ComDCM.fErrorChar = FALSE;
	ComDCM.fNull = FALSE;
	ComDCM.fOutxCtsFlow = FALSE;
	ComDCM.fOutxDsrFlow = FALSE;
	ComDCM.XonLim = 128;
	ComDCM.XoffLim = 128;

	if (!SetCommState(hPort, &ComDCM)) {
		CloseHandle(hPort);
		hPort = INVALID_HANDLE_VALUE;
		return FALSE;
	}
	return true;
}

void TCommPortAsync::Disconnect() {

	if (hPort != INVALID_HANDLE_VALUE)
	{
		CloseHandle(hPort);
		hPort = INVALID_HANDLE_VALUE;
	}

}


BOOL __stdcall TCommPortAsync::IsConnected() {
	return hPort != INVALID_HANDLE_VALUE;
}

HRESULT __stdcall TCommPortAsync::WriteRead(
	BYTE const* const pOutBuffer,
	int iRequestLenght,
	BYTE* pInBuffer,
	int* pAnswerLength
) {
	return S_OK;
}

// „итаем данные, которые мы не запрашивали (если есть)
// ≈сли выполнено успешно возвращаем S_OK = 0x0 иначе стандартный или свой код ошибки
HRESULT __stdcall TCommPortAsync::ReadUnsolicitedData(
	BYTE* pInBuffer,
	int* pDataLength
) {
	return S_OK;
}


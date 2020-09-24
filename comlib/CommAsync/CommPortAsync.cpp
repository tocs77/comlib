//#pragma on
//#pragma hdrstop
#include <iostream>
#include <memory>
#include "CommPortAsync.h"
#include "../Logger/Logger.h"

//#pragma package(smart_init)
//---------------------------------------------------------------------------

TCommPortAsync::TCommPortAsync()
{
	hPort = INVALID_HANDLE_VALUE;
	for (int i = 0; i < EVENT_NUMBER; i++) {
		events[i] = CreateEvent(NULL, TRUE, FALSE, NULL);
	}
	readHandle = NULL;
	readBuffer = {};
}


TCommPortAsync::~TCommPortAsync()
{
	SetEvent(events[FINISH_PORT_READ_EVENT]);
	readHandle->join();
	Disconnect();
	for (int i = 0; i < EVENT_NUMBER; i++) {
		CloseHandle(events[i]);
	}
	for (unsigned int i = 0; i < readBuffer.size(); i++) {

		Logger::logInfo("��������� ������: " + Logger::ByteArrayToHex(readBuffer[i].data.data(), readBuffer[i].size));
	}
	Logger::logInfo("���������� ����� ��������");

}

void TCommPortAsync::Start() {
	readHandle = new std::thread(&TCommPortAsync::readLoop, this);
}

BOOL TCommPortAsync::Connect() {
	Disconnect();
	bool result;

	hPort =
		CreateFile(
			(LPCWSTR)portNumber.c_str(),     //��� �����
			GENERIC_READ | GENERIC_WRITE,   // ������ �� ������ � ������
			0,                              // shared mode ������ ���� 0 ��� ������ ������
			NULL,                           // security for file, ��� com ����� ����� ���� 0
			OPEN_EXISTING,                  // ��� ����� ������� ������������
			FILE_FLAG_OVERLAPPED,          // FILE_FLAG_OVERLAPPED ��� ����������� ������ � ������
			NULL);                          // template file, ��� com ����� ������ ���� 0


	if (hPort != INVALID_HANDLE_VALUE) {
		Logger::logInfo("���� " + std::string(portNumber.begin(), portNumber.end()) + " ������");
	}
	else {
		DWORD errCode;
		errCode = GetLastError();
		switch (errCode)
		{
		case ERROR_FILE_NOT_FOUND:
			Logger::logError("���� " + std::string(portNumber.begin(), portNumber.end()) + " �� ����������.");
			break;
		case ERROR_ACCESS_DENIED:
			Logger::logError("���� " + std::string(portNumber.begin(), portNumber.end()) + " ����� ������ �����������.");
			break;
		default:
			Logger::logError("������ � �����  " + std::to_string(errCode) + " ��������� ��� �������� ����� " +
				std::string(portNumber.begin(), portNumber.end()));
			break;
		}

		return false;
	}

	result = SetCommMask(hPort, EV_RXCHAR | EV_TXEMPTY);
	if (!result) {
		hPort = INVALID_HANDLE_VALUE;
		Logger::logError("�� ��������������� ����� �����  " + std::string(portNumber.begin(), portNumber.end()));
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
		Logger::logError("�� ��������������� �������� ����� " + std::string(portNumber.begin(), portNumber.end()));
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
		Logger::logError("�� ��������������� ��������� �����  " + std::string(portNumber.begin(), portNumber.end()));
		hPort = INVALID_HANDLE_VALUE;
		return FALSE;
	}
	Logger::logInfo("���� " + std::string(portNumber.begin(), portNumber.end()) + " ��������������� �������");
	return true;
}

BOOL TCommPortAsync::Disconnect() {

	if (hPort == INVALID_HANDLE_VALUE) {
		return TRUE;
	}

	if (CloseHandle(hPort)) {
		hPort = INVALID_HANDLE_VALUE;
		return TRUE;
	};

	Logger::logError("�� ������� ������� ���� " + std::string(portNumber.begin(), portNumber.end()));
	return FALSE;

}


BOOL __stdcall TCommPortAsync::IsConnected() {
	return hPort != INVALID_HANDLE_VALUE;
}


void __stdcall TCommPortAsync::readLoop() {
	DWORD dwRead, temp, btr;
	DWORD waitResult = 0, read = 0, state = 0;

	OVERLAPPED osReader = { 0 }; // ��������� ��� ����������� ������ � COM ������
	COMSTAT comstat;
	osReader.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

	HANDLE portEvents[3];		//������� ������������� ��� ����������� ������

	portEvents[0] = osReader.hEvent;
	portEvents[1] = events[STOP_PORT_READ_EVENT];
	portEvents[2] = events[FINISH_PORT_READ_EVENT];



	bool readResult, readNext;

	SetEvent(events[START_PORT_READ_EVENT]);
	Logger::logInfo("readLoop started");

	while (true) {
		waitResult = WaitForSingleObject(events[START_PORT_READ_EVENT], 0);
		if (waitResult != WAIT_OBJECT_0) {
			waitResult = WaitForSingleObject(events[START_PORT_READ_EVENT], INFINITE);
			if (osReader.hEvent) SetEvent(osReader.hEvent);
		}

		readNext = true;
		while (readNext) {

			WaitCommEvent(hPort, &state, &osReader);
			waitResult = WaitForMultipleObjects(3, portEvents, FALSE, INFINITE);
			switch (waitResult)
			{
			case  WAIT_OBJECT_0:  //�������� ��������� � ������ ������
				if (GetOverlappedResult(hPort, &osReader, &read, FALSE)) {

					if ((state & EV_RXCHAR) != 0) {
						ClearCommError(hPort, &temp, &comstat);               //����� ��������� ��������� COMSTAT
						btr = comstat.cbInQue;                                  //� �������� �� �� ���������� �������� ������
						if (btr)                                                 //���� ������������� ���� ����� ��� ������
						{
							readBuffer.push_back(DataStorage{ btr, std::vector<BYTE>(btr) }); //��������� � ����� ����� ������� ������
							readResult = ReadFile(hPort, readBuffer.back().data.data(), readBuffer.back().size, &dwRead, &osReader);    //��������� ����� �� ����� � ����� ���������
							if (readResult) {
								Logger::logInfo("readLoop ��������� " + std::to_string(readBuffer.back().size) + " ����");
							}
							else {
								readBuffer.pop_back(); // ������� ������� ������, ���� �� ������� ������
								Logger::logWarning("readLoop �� ������� ������� ������");
							}
						}
					}
				}
				break;
			case WAIT_OBJECT_0 + 1: //�������� ��������� � ���������� ������ ������
				Logger::logInfo("����������� ����������� ������ ������");
				CancelIoEx(hPort, &osReader);
				readNext = false;
				break;

			case WAIT_OBJECT_0 + 2: //�������� ��������� � ���������� ������. ������� �� ������� readLoop
				Logger::logInfo("readLoop ����������");
				return;
			default:
				Logger::logWarning("readLoop ����������� ��������� ");

				break;
			}
		}
	}
}


HRESULT __stdcall TCommPortAsync::WriteRead(
	BYTE const* const pOutBuffer,
	int iRequestLenght,
	BYTE* pInBuffer,
	DWORD* pAnswerLength
) {
	DWORD dwRead, dwWritedLength, temp, btr;
	BOOL result;
	HRESULT writeReadResult = S_OK;

	OVERLAPPED osReader = { 0 };
	COMSTAT comstat;
	DWORD read;
	osReader.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);


	if (osReader.hEvent == NULL) {
		Logger::logError("������ �������� ������� ��� ������");
		return S_FALSE;
	}

	unsigned long state = 0;

	SetEvent(events[STOP_PORT_READ_EVENT]);			// ��������� ������� ��� ��������� ������ ������������� ������ �� �����
	ResetEvent(events[START_PORT_READ_EVENT]);		//���������� ������� ��� ������ ������ ������������� ������


	DWORD dwErrorFlags;
	result = WriteFile(hPort, pOutBuffer, iRequestLenght, &dwWritedLength, &osReader);
	if (!result) // ���� ��������� ������
	{
		dwErrorFlags = GetLastError();
		if (dwErrorFlags == ERROR_IO_PENDING)
		{
			WaitCommEvent(hPort, &state, &osReader);
			result = WaitForSingleObject(osReader.hEvent, iWriteTimeOut);
			if (result == WAIT_OBJECT_0) {
				if (GetOverlappedResult(hPort, &osReader, &read, FALSE)) {
					if ((state & EV_TXEMPTY) != 0) {
						Logger::logInfo("������ �������� � ���� �������");
					}
				}
			}
			else {
				Logger::logWarning("������ �������� ������");
				writeReadResult = S_FALSE;
			}
		}
		else {
			Logger::logWarning("��� ������ ��������� ������ " + std::to_string(dwErrorFlags));
			writeReadResult = S_FALSE;
		}
	}
	else {

		Logger::logInfo("��������� ������ ������� ����� " + std::to_string(result));
	}


	if (writeReadResult != S_OK) {  //���� ��� ������ ��������� ������, ��������� ���� ������ � �������
		ResetEvent(events[STOP_PORT_READ_EVENT]);
		SetEvent(events[START_PORT_READ_EVENT]);
		return writeReadResult;
	}
	//--------------------------------------
	// ������ � ���� ��������, ���� �����

	WaitCommEvent(hPort, &state, &osReader);
	result = WaitForSingleObject(osReader.hEvent, iReadTimeOut);
	if (result == WAIT_OBJECT_0) {
		if (GetOverlappedResult(hPort, &osReader, &read, FALSE)) {

			if ((state & EV_RXCHAR) != 0) {
				ClearCommError(hPort, &temp, &comstat);               //����� ��������� ��������� COMSTAT
				btr = comstat.cbInQue;                                  //� �������� �� �� ���������� �������� ������
				if (btr)                                                 //���� ������������� ���� ����� ��� ������
				{
					pInBuffer = new BYTE[btr];
					*pAnswerLength = btr;
					result = ReadFile(hPort, pInBuffer, btr, &dwRead, &osReader);    //��������� ����� �� ����� � ����� ���������
					if (result) {
						Logger::logInfo("������ ��������� �������");
					}
					else {
						Logger::logError("�� ������� ��������� ������");
					}
				}
			}
		}
	}
	else {
		Logger::logWarning("��� ������ ��� ������");
		writeReadResult = S_FALSE;
	}

	ResetEvent(events[STOP_PORT_READ_EVENT]);
	SetEvent(events[START_PORT_READ_EVENT]);

	return writeReadResult;

}

// ������ ������, ������� �� �� ����������� (���� ����)
// ���� ��������� ������� ���������� S_OK = 0x0 ����� ����������� ��� ���� ��� ������
HRESULT __stdcall TCommPortAsync::ReadUnrequestedData(
	std::unique_ptr<BYTE*> pInBuffer,
	int* pDataLength
) {
	return S_OK;
}


#include "tty.h"
#include <iostream>
#include <assert.h>
#include <windows.h>
#include <thread>
#include <functional>


static int TIMEOUT = 1000;

TTY::TTY() {
	m_Handle = INVALID_HANDLE_VALUE;
	for (int i = 0; i < EVENT_NUMBER; i++) {
		events[i] = CreateEvent(NULL, TRUE, FALSE, NULL);
	}
}

TTY::~TTY() {
	Disconnect();
	for (int i = 0; i < EVENT_NUMBER; i++) {
		CloseHandle(events[i]);
	}
	readHandle->detach();
}


bool TTY::IsOK() const {
	return m_Handle != INVALID_HANDLE_VALUE;
}

bool TTY::Connect(LPCWSTR port, int baudrate) {

	Disconnect();
	bool result;

	m_Handle =
		CreateFile(
			port,                           //port name 
			GENERIC_READ | GENERIC_WRITE,   // acces to read and write
			0,                              // shared mode, should be zero for com port
			NULL,                           // security for file, should be zero for com port
			OPEN_EXISTING,                  // for port open existing
			FILE_FLAG_OVERLAPPED,          // FILE_FLAG_OVERLAPPED for async work
			NULL);                          // template file, should be zero for com port


	if (m_Handle != INVALID_HANDLE_VALUE) {
		std::wcout << "Port " << port << " opened" << std::endl;
	}
	else {
		DWORD errCode;
		errCode = GetLastError();
		if (errCode == ERROR_FILE_NOT_FOUND)
		{
			std::cout << "serial port does not exist.\n";
		}
		std::cout << "some other error occurred.\t" << errCode << std::endl;
		return false;
	}
	result = SetCommMask(m_Handle, EV_RXCHAR | EV_TXEMPTY);
	if (!result) {
		std::cout << "Error set mask " << std::endl;
	}
	SetupComm(m_Handle, 1500, 1500);

	COMMTIMEOUTS CommTimeOuts;
	CommTimeOuts.ReadIntervalTimeout = 20;
	CommTimeOuts.ReadTotalTimeoutMultiplier = 10;
	CommTimeOuts.ReadTotalTimeoutConstant = 70;
	CommTimeOuts.WriteTotalTimeoutMultiplier = 4;
	CommTimeOuts.WriteTotalTimeoutConstant = TIMEOUT;

	if (!SetCommTimeouts(m_Handle, &CommTimeOuts)) {
		CloseHandle(m_Handle);
		m_Handle = INVALID_HANDLE_VALUE;
		throw TTYException();
	}

	DCB ComDCM;

	memset(&ComDCM, 0, sizeof(ComDCM));
	ComDCM.DCBlength = sizeof(DCB);
	GetCommState(m_Handle, &ComDCM);
	ComDCM.BaudRate = DWORD(baudrate);
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

	if (!SetCommState(m_Handle, &ComDCM)) {
		CloseHandle(m_Handle);
		m_Handle = INVALID_HANDLE_VALUE;
		throw TTYException();
	}
	return true;
}

void TTY::Disconnect() {

	if (m_Handle != INVALID_HANDLE_VALUE)
	{
		CloseHandle(m_Handle);
		m_Handle = INVALID_HANDLE_VALUE;
	}

}

void TTY::Write(const std::vector<unsigned char>& data) {

	if (m_Handle == INVALID_HANDLE_VALUE) {
		throw TTYException();
	}

	DWORD feedback;
	if (!WriteFile(m_Handle, &data[0], (DWORD)data.size(), &feedback, 0) || feedback != (DWORD)data.size()) {
		CloseHandle(m_Handle);
		m_Handle = INVALID_HANDLE_VALUE;
		throw TTYException();
	}

	// In some cases it's worth uncommenting
	//FlushFileBuffers(m_Handle);

}


// TTY::Read function
void TTY::Read() {


	if (m_Handle == INVALID_HANDLE_VALUE) {
		throw TTYException();
	}

	DWORD dwRead, temp, btr;
	OVERLAPPED osReader = { 0 };
	COMSTAT comstat;

	osReader.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

	char data[500];

	if (osReader.hEvent == NULL) {
		std::cout << "Error crate osReader event" << std::endl;
		throw TTYException();
	}

	HANDLE portEvents[2];


	portEvents[0] = osReader.hEvent;
	portEvents[1] = events[STOP_PORT_READ_EVENT];


	unsigned long wait = 0, read = 0, state = 0;

	bool w;

	SetEvent(events[START_PORT_READ_EVENT]);

	while (true) {
		w = WaitForSingleObject(events[START_PORT_READ_EVENT], 0);
		if (w != WAIT_OBJECT_0) {
			std::cout << "Wait for begin working " << std::endl;
			w = WaitForSingleObject(events[START_PORT_READ_EVENT], INFINITE);
			SetEvent(osReader.hEvent);
			std::cout << "Stopped waiting " << std::endl;
		}
		std::cout << "Begin to listen " << std::endl;


		while (true) {


			//std::cout << "Listen to port in Read()..." << std::endl;
			WaitCommEvent(m_Handle, &state, &osReader);
			wait = WaitForMultipleObjects(2, portEvents, FALSE, INFINITE);
			//wait = WaitForSingleObject(osReader.hEvent, INFINITE);
			if (wait == WAIT_OBJECT_0) {
				//std::cout << "Got wait " << wait << " Got state " << state << std::endl;
				if (GetOverlappedResult(m_Handle, &osReader, &read, FALSE)) {

					if ((state & EV_RXCHAR) != 0) {
						ClearCommError(m_Handle, &temp, &comstat);               //нужно заполнить структуру COMSTAT
						btr = comstat.cbInQue;                                  //и получить из неё количество принятых байтов
						if (btr)                                                 //если действительно есть байты для чтения
						{
							w = ReadFile(m_Handle, data, btr, &dwRead, &osReader);    //прочитать байты из порта в буфер программы
							data[dwRead] = 0;
							std::cout << "TTY::Read()  " << data << std::endl;
						}
					}

				}
			}
			else if (wait == WAIT_OBJECT_0 + 1) {
				std::cout << "Got wait 1 for BREAK!!!!!!!!!!!" << wait << std::endl;
				CancelIoEx(m_Handle, &osReader);
				break;
			}
			else {
				std::cout << "Got unknown wait" << wait << std::endl;
			}

		}
	}
}


// TTY::WriteAndRead
HRESULT TTY::WriteAndRead(char* data) {
	std::cout << "-------------------------------------------------------\n";
	DWORD dwRead, temp, btr;
	BOOL result;


	OVERLAPPED osReader = { 0 };
	COMSTAT comstat;
	DWORD read;
	osReader.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);


	if (osReader.hEvent == NULL) {
		std::cout << "Error crate osReader event" << std::endl;
		throw TTYException();
	}
	unsigned long state = 0;


	SetEvent(events[STOP_PORT_READ_EVENT]);
	ResetEvent(events[START_PORT_READ_EVENT]);


	CHAR buffer[5]{ 'T', 'e', 's' ,'t', '\n' };
	DWORD dwBytesWritten = 5;
	DWORD dwErrorFlags;
	std::cout << "Prepare to write " << std::endl;
	result = WriteFile(m_Handle, buffer, dwBytesWritten, &dwBytesWritten, &osReader);
	if (!result)
	{
		std::cout << "Got result in writing " << std::endl;
		dwErrorFlags = GetLastError();
		if (dwErrorFlags == ERROR_IO_PENDING)
		{
			std::cout << "Got result in ERROR_IO_PENDING" << std::endl;
			WaitCommEvent(m_Handle, &state, &osReader);
			result = WaitForSingleObject(osReader.hEvent, 300);
			if (result == WAIT_OBJECT_0) {
				std::cout << "Good wait result " << result << std::endl;
				if (GetOverlappedResult(m_Handle, &osReader, &read, FALSE)) {
					if ((state & EV_TXEMPTY) != 0) {
						std::cout << "Write success";
					}
				}
			}
			else {
				std::cout << "Bad wait result " << result << std::endl;
			}

		}
		else {
			std::cout << "Got error in writing " << dwErrorFlags << std::endl;
		}
	}
	else {

		std::cout << "Got immediate writing result " << result << std::endl;
	}
	std::cout << "End writing " << std::endl;




	WaitCommEvent(m_Handle, &state, &osReader);
	result = WaitForSingleObject(osReader.hEvent, 300);
	std::cout << "Result in writeAndRead " << result << std::endl;
	if (result == WAIT_OBJECT_0) {
		if (GetOverlappedResult(m_Handle, &osReader, &read, FALSE)) {

			if ((state & EV_RXCHAR) != 0) {
				ClearCommError(m_Handle, &temp, &comstat);               //нужно заполнить структуру COMSTAT
				btr = comstat.cbInQue;                                  //и получить из неё количество принятых байтов
				if (btr)                                                 //если действительно есть байты для чтения
				{
					result = ReadFile(m_Handle, data, btr, &dwRead, &osReader);    //прочитать байты из порта в буфер программы
					data[dwRead] = 0;
					std::cout << "TTY::WriteAndRead  " << data << std::endl;
				}
			}
		}
	}
	else {
		std::cout << "no data in port " << std::endl;
		return S_FALSE;
	}


	ResetEvent(events[STOP_PORT_READ_EVENT]);
	SetEvent(events[START_PORT_READ_EVENT]);
	std::cout << "-------------------------------------------------------\n\n";

	return S_OK;
}

void TTY::Start() {
	readHandle = new std::thread(&TTY::Read, this);
}


//int main(int argc, char* argv[])
//{
//	TTY tty;
//	if (!tty.Connect(L"\\\\.\\COM14", CBR_115200)) {
//		return 1;
//	}
//
//	tty.Start();
//
//	char buff[500];
//
//	for (int i = 2; i < 8; i++) {
//
//
//
//
//		if (i > 3 && i < 7) {
//			if (tty.WriteAndRead(buff) == S_OK) {
//				std::cout << "Make write read " << i << " " << buff << std::endl; //output t
//			}
//			else {
//				std::cout << "No result in client query" << std::endl;
//			}
//
//		}
//
//		std::cout << "Do some stuff " << i << std::endl; //output t
//		Sleep(1000);
//
//	}
//	return EXIT_SUCCESS;
//}
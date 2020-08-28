#include "tty.h"
#include <iostream>
#include <assert.h>
#include <windows.h>
#include <thread>
#include <functional>

using namespace std;

static int TIMEOUT = 1000;

TTY::TTY() {
	m_Handle = INVALID_HANDLE_VALUE;
	checkData = true;
}

TTY::~TTY() {
	Disconnect();
}

void TTY::setCheckData(bool val) {
	checkData = val;
}

bool TTY::IsOK() const {
	return m_Handle != INVALID_HANDLE_VALUE;
}

bool TTY::Connect(LPCWSTR port, int baudrate) {

	Disconnect();

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
		std::wcout << "Port " << *port << " opened";
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

	SetCommMask(m_Handle, EV_RXCHAR);
	SetupComm(m_Handle, 1500, 1500);

	COMMTIMEOUTS CommTimeOuts;
	CommTimeOuts.ReadIntervalTimeout = 1;
	CommTimeOuts.ReadTotalTimeoutMultiplier = 1;
	CommTimeOuts.ReadTotalTimeoutConstant = 1;
	CommTimeOuts.WriteTotalTimeoutMultiplier = 0;
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

void TTY::Write(const vector<unsigned char>& data) {

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
// TTY::Read info
void TTY::Read() {


	if (m_Handle == INVALID_HANDLE_VALUE) {
		throw TTYException();
	}

	DWORD dwRead;
	OVERLAPPED osReader = { 0 };

	osReader.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

	char data[100];

	if (osReader.hEvent == NULL) {
		std::cout << "Error crate osReader event" << std::endl;
		throw TTYException();
	}

	//unsigned char* buf = &data[0];
	//DWORD READ_BUF_SIZE = (DWORD)data.size();

	unsigned long wait = 0, read = 0, state = 0;


	if (SetCommMask(m_Handle, EV_RXCHAR)) {
		while (true) {

			WaitCommEvent(m_Handle, &state, &osReader);
			wait = WaitForSingleObject(osReader.hEvent, INFINITE);
			std::cout << "Got wait " << wait << std::endl;
			if (!checkData) {
				std::cout << "Data reading locked" << std::endl;
				continue;
			}

			if (wait == WAIT_OBJECT_0) {
				//begin read data
				ReadFile(m_Handle, data, 99, &dwRead, &osReader);
				wait = WaitForSingleObject(osReader.hEvent, 100);
				std::cout << "Got wait 2: " << wait << std::endl;

				if (wait == WAIT_OBJECT_0) {
					if (GetOverlappedResult(m_Handle, &osReader, &read, FALSE));
				}
				data[dwRead] = 0;
				std::cout << "TTY::Read()  " << data << std::endl;
				ResetEvent(osReader.hEvent);
			}


			Sleep(500);
		}


	}

}

// TTY::Read info
void TTY::WriteAndRead(char* data) {
	DWORD dwRead;
	BOOL res;
	res = ReadFile(m_Handle, data, 499, &dwRead, NULL);
	if (!res) {
		std::cout << "Got error in WriteAndRead " << GetLastError() << std::endl;
	}
	cout << "Got result in WriteAnd Read " << res << std::endl;
	data[dwRead] = 0;
}


int main(int argc, char* argv[])
{
	TTY tty;
	if (!tty.Connect(L"\\\\.\\COM2", CBR_115200)) {
		return 1;
	}

	char buff[500];

	std::thread th(&TTY::Read, std::ref(tty));

	for (int i = 0; i < 10; i++) {



		//if (tty.Read(buff)) {
		//	std::cout << "Got result on step: " << i << " " << buff << std::endl; //output text
		//}
		//else
		//{
		//	std::cout << "No result on step: " << i << std::endl; //output t
		//}
		if (i > 3 && i < 7) {
			tty.setCheckData(false);
			tty.WriteAndRead(buff);
			std::cout << "Make write read " << i << " " << buff << std::endl; //output t
		}

		if (i > 7) tty.setCheckData(true);

		std::cout << "Do some stuff " << i << std::endl; //output t
		Sleep(1000);

	}
	th.detach();
	return EXIT_SUCCESS;
}
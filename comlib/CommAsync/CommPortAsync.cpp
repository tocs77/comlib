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

		Logger::logInfo("Считанные данные: " + Logger::ByteArrayToHex(readBuffer[i].data.data(), readBuffer[i].size));
	}
	Logger::logInfo("Деструктор порта выполнен");

}

void TCommPortAsync::Start() {
	readHandle = new std::thread(&TCommPortAsync::readLoop, this);
}

BOOL TCommPortAsync::Connect() {
	Disconnect();
	bool result;

	hPort =
		CreateFile(
			(LPCWSTR)portNumber.c_str(),     //имя порта
			GENERIC_READ | GENERIC_WRITE,   // доступ на запись и чтение
			0,                              // shared mode должен быть 0 пор делить нельзя
			NULL,                           // security for file, для com порта долже быть 0
			OPEN_EXISTING,                  // для порта открыть существующий
			FILE_FLAG_OVERLAPPED,          // FILE_FLAG_OVERLAPPED для асинхронной работы с портом
			NULL);                          // template file, для com порта должен быть 0


	if (hPort != INVALID_HANDLE_VALUE) {
		Logger::logInfo("Порт " + std::string(portNumber.begin(), portNumber.end()) + " открыт");
	}
	else {
		DWORD errCode;
		errCode = GetLastError();
		switch (errCode)
		{
		case ERROR_FILE_NOT_FOUND:
			Logger::logError("Порт " + std::string(portNumber.begin(), portNumber.end()) + " не существует.");
			break;
		case ERROR_ACCESS_DENIED:
			Logger::logError("Порт " + std::string(portNumber.begin(), portNumber.end()) + " занят другим приложением.");
			break;
		default:
			Logger::logError("Ошибка с кодом  " + std::to_string(errCode) + " произошла при открытии порта " +
				std::string(portNumber.begin(), portNumber.end()));
			break;
		}

		return false;
	}

	result = SetCommMask(hPort, EV_RXCHAR | EV_TXEMPTY);
	if (!result) {
		hPort = INVALID_HANDLE_VALUE;
		Logger::logError("Не устанавливается маска порта  " + std::string(portNumber.begin(), portNumber.end()));
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
		Logger::logError("Не устанавливаются таймауты порта " + std::string(portNumber.begin(), portNumber.end()));
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
		Logger::logError("Не устанавливаются настройки порта  " + std::string(portNumber.begin(), portNumber.end()));
		hPort = INVALID_HANDLE_VALUE;
		return FALSE;
	}
	Logger::logInfo("Порт " + std::string(portNumber.begin(), portNumber.end()) + " сконфигурирован успешно");
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

	Logger::logError("Не удалось закрыть порт " + std::string(portNumber.begin(), portNumber.end()));
	return FALSE;

}


BOOL __stdcall TCommPortAsync::IsConnected() {
	return hPort != INVALID_HANDLE_VALUE;
}


void __stdcall TCommPortAsync::readLoop() {
	DWORD dwRead, temp, btr;
	DWORD waitResult = 0, read = 0, state = 0;

	OVERLAPPED osReader = { 0 }; // Стурктура для асинхронной работы с COM портом
	COMSTAT comstat;
	osReader.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

	HANDLE portEvents[3];		//События отслеживаемые при циклическом чтении

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
			case  WAIT_OBJECT_0:  //Получено сообщение о чтении данных
				if (GetOverlappedResult(hPort, &osReader, &read, FALSE)) {

					if ((state & EV_RXCHAR) != 0) {
						ClearCommError(hPort, &temp, &comstat);               //нужно заполнить структуру COMSTAT
						btr = comstat.cbInQue;                                  //и получить из неё количество принятых байтов
						if (btr)                                                 //если действительно есть байты для чтения
						{
							readBuffer.push_back(DataStorage{ btr, std::vector<BYTE>(btr) }); //Добавляем в буфер новый элемент данных
							readResult = ReadFile(hPort, readBuffer.back().data.data(), readBuffer.back().size, &dwRead, &osReader);    //прочитать байты из порта в буфер программы
							if (readResult) {
								Logger::logInfo("readLoop прочитано " + std::to_string(readBuffer.back().size) + " байт");
							}
							else {
								readBuffer.pop_back(); // Удаляем элемент данных, если не удалось чтение
								Logger::logWarning("readLoop не удалось считать данные");
							}
						}
					}
				}
				break;
			case WAIT_OBJECT_0 + 1: //Получено сообщение о прерывании чтения данных
				Logger::logInfo("Остановлено циклическое чтение данных");
				CancelIoEx(hPort, &osReader);
				readNext = false;
				break;

			case WAIT_OBJECT_0 + 2: //Получено сообщение о завершении работы. Выходим из функции readLoop
				Logger::logInfo("readLoop остановлен");
				return;
			default:
				Logger::logWarning("readLoop неизвестное сообщение ");

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
		Logger::logError("Ошибка создания события для чтения");
		return S_FALSE;
	}

	unsigned long state = 0;

	SetEvent(events[STOP_PORT_READ_EVENT]);			// Установка события для остановки чтения незапрошенных данных из порта
	ResetEvent(events[START_PORT_READ_EVENT]);		//Сбрасываем событие для начала чтения незапрошенных данных


	DWORD dwErrorFlags;
	result = WriteFile(hPort, pOutBuffer, iRequestLenght, &dwWritedLength, &osReader);
	if (!result) // Ждем окончания записи
	{
		dwErrorFlags = GetLastError();
		if (dwErrorFlags == ERROR_IO_PENDING)
		{
			WaitCommEvent(hPort, &state, &osReader);
			result = WaitForSingleObject(osReader.hEvent, iWriteTimeOut);
			if (result == WAIT_OBJECT_0) {
				if (GetOverlappedResult(hPort, &osReader, &read, FALSE)) {
					if ((state & EV_TXEMPTY) != 0) {
						Logger::logInfo("Данные записаны в порт успешно");
					}
				}
			}
			else {
				Logger::logWarning("Ошибка ожидания записи");
				writeReadResult = S_FALSE;
			}
		}
		else {
			Logger::logWarning("При записи произошла ошибка " + std::to_string(dwErrorFlags));
			writeReadResult = S_FALSE;
		}
	}
	else {

		Logger::logInfo("Результат записи получен сразу " + std::to_string(result));
	}


	if (writeReadResult != S_OK) {  //Если при записи произошла ошибка, запускаем цикл чтения и выходим
		ResetEvent(events[STOP_PORT_READ_EVENT]);
		SetEvent(events[START_PORT_READ_EVENT]);
		return writeReadResult;
	}
	//--------------------------------------
	// Запись в порт окончена, ждем ответ

	WaitCommEvent(hPort, &state, &osReader);
	result = WaitForSingleObject(osReader.hEvent, iReadTimeOut);
	if (result == WAIT_OBJECT_0) {
		if (GetOverlappedResult(hPort, &osReader, &read, FALSE)) {

			if ((state & EV_RXCHAR) != 0) {
				ClearCommError(hPort, &temp, &comstat);               //нужно заполнить структуру COMSTAT
				btr = comstat.cbInQue;                                  //и получить из неё количество принятых байтов
				if (btr)                                                 //если действительно есть байты для чтения
				{
					pInBuffer = new BYTE[btr];
					*pAnswerLength = btr;
					result = ReadFile(hPort, pInBuffer, btr, &dwRead, &osReader);    //прочитать байты из порта в буфер программы
					if (result) {
						Logger::logInfo("Данные прочитаны успешно");
					}
					else {
						Logger::logError("Не удалось прочитать данные");
					}
				}
			}
		}
	}
	else {
		Logger::logWarning("Нет данных для чтения");
		writeReadResult = S_FALSE;
	}

	ResetEvent(events[STOP_PORT_READ_EVENT]);
	SetEvent(events[START_PORT_READ_EVENT]);

	return writeReadResult;

}

// Читаем данные, которые мы не запрашивали (если есть)
// Если выполнено успешно возвращаем S_OK = 0x0 иначе стандартный или свой код ошибки
HRESULT __stdcall TCommPortAsync::ReadUnrequestedData(
	std::unique_ptr<BYTE*> pInBuffer,
	int* pDataLength
) {
	return S_OK;
}


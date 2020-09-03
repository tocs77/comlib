#pragma once

#ifndef CommPortAsyncH
#define CommPortAsyncH
//-----------------------------------------------------------------------------
#include <thread>

#include "../IComm/ICommInterface.h"
#include "../IComm/ICommInterface_Statistics.h"
#include "../IComm/IComPort_Config.h"
//-----------------------------------------------------------------------------
constexpr int STOP_PORT_READ_EVENT = 0;
constexpr int  START_PORT_READ_EVENT = 1;
constexpr int EVENT_NUMBER = 2;

class TCommPortAsync :
	public ICommInterface,
	public ICommInterface_Statistics,
	public IComPort_Config
{
private:
	HANDLE hPort;
	HANDLE events[EVENT_NUMBER];
	std::thread* readHandle;
	void Disconnect();
public:
	TCommPortAsync();
	virtual ~TCommPortAsync();
	BOOL __stdcall IsConnected();
	BOOL __stdcall Connect();
	HRESULT __stdcall WriteRead(BYTE const* const pOutBuffer, int iRequestLenght, BYTE* pInBuffer, int* pAnswerLength);
	HRESULT __stdcall ReadUnsolicitedData(BYTE* pInBuffer, int* pDataLength);


	/*
		// Или, если не получится тормозить флагом, сделать Метод
		void __stdcall Stop();
	*/
	// Здесь перечисляем все интерфейсные функции  

};
//-----------------------------------------------------------------------------
#endif
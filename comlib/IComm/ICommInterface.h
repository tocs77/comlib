#pragma once

#ifndef ICommInterfaceH
#define ICommInterfaceH
//-----------------------------------------------------------------------------
#include <vector>
#include <Windows.h>
// #include "WinNT.h"  // Должен включиться автоматически при включении "Windows.h"
//-----------------------------------------------------------------------------
class ICommInterface
{
public:
	virtual BOOL __stdcall Connect() = 0;
	virtual BOOL __stdcall Disconnect() = 0;
	virtual BOOL __stdcall IsConnected() = 0;


	// Обработка запрос-ответ
	// Если выполнено успешно возвращаем S_OK = 0x0 иначе стандартный или свой код ошибки
	virtual HRESULT __stdcall WriteRead(
		BYTE const* const pOutBuffer,
		int iRequestLenght,
		BYTE* pInBuffer,
		DWORD* pAnswerLength
	) = 0;

	// Читаем данные, которые мы не запрашивали (если есть)
	// Если выполнено успешно возвращаем S_OK = 0x0 иначе стандартный или свой код ошибки
	virtual HRESULT __stdcall ReadUnrequestedData(
		std::unique_ptr<BYTE*> pInBuffer,
		int* pDataLength
	) = 0;

	//-----------------------------------------------------------
	// Получить среднее время задержки в канале (если не было обмена то 0)
	DWORD __stdcall GetAvgChannelDelay_mS();
	// Сбросить среднее время задержки в канале
	void __stdcall ResetAvgChannelDelay();

protected:
	struct DataStorage {
		DWORD size;
		std::vector<BYTE> data;
	};

	std::vector<DataStorage> readBuffer;
};
//-----------------------------------------------------------------------------

#endif




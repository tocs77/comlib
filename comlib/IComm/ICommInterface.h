#pragma once

#ifndef ICommInterfaceH
#define ICommInterfaceH
//-----------------------------------------------------------------------------
#include "Windows.h"
// #include "WinNT.h"  // Должен включиться автоматически при включении "Windows.h"
//-----------------------------------------------------------------------------
class ICommInterface
{
  public:
    BOOL __stdcall Connect();
    BOOL __stdcall Disconnect();
    BOOL __stdcall IsConnected();


    // Обработка запрос-ответ
    // Если выполнено успешно возвращаем S_OK = 0x0 иначе стандартный или свой код ошибки
    virtual HRESULT __stdcall WriteRead(
        BYTE const*const pOutBuffer,
        int iRequestLenght,
        BYTE* pInBuffer,
        int* pAnswerLength
          ) = 0;
    
    // Читаем данные, которые мы не запрашивали (если есть)
    // Если выполнено успешно возвращаем S_OK = 0x0 иначе стандартный или свой код ошибки
    virtual HRESULT __stdcall ReadUnsolicitedData(
      BYTE* pInBuffer,
      int* pDataLength
    ) = 0;
    
    //-----------------------------------------------------------
    // Получить среднее время задержки в канале (если не было обмена то 0)
    DWORD __stdcall GetAvgChannelDelay_mS();    
    // Сбросить среднее время задержки в канале
    void __stdcall ResetAvgChannelDelay();
};
//-----------------------------------------------------------------------------
#endif




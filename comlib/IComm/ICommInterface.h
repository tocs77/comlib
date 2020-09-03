#pragma once

#ifndef ICommInterfaceH
#define ICommInterfaceH
//-----------------------------------------------------------------------------
#include "Windows.h"
// #include "WinNT.h"  // ������ ���������� ������������� ��� ��������� "Windows.h"
//-----------------------------------------------------------------------------
class ICommInterface
{
  public:
    BOOL __stdcall Connect();
    BOOL __stdcall Disconnect();
    BOOL __stdcall IsConnected();


    // ��������� ������-�����
    // ���� ��������� ������� ���������� S_OK = 0x0 ����� ����������� ��� ���� ��� ������
    virtual HRESULT __stdcall WriteRead(
        BYTE const*const pOutBuffer,
        int iRequestLenght,
        BYTE* pInBuffer,
        int* pAnswerLength
          ) = 0;
    
    // ������ ������, ������� �� �� ����������� (���� ����)
    // ���� ��������� ������� ���������� S_OK = 0x0 ����� ����������� ��� ���� ��� ������
    virtual HRESULT __stdcall ReadUnsolicitedData(
      BYTE* pInBuffer,
      int* pDataLength
    ) = 0;
    
    //-----------------------------------------------------------
    // �������� ������� ����� �������� � ������ (���� �� ���� ������ �� 0)
    DWORD __stdcall GetAvgChannelDelay_mS();    
    // �������� ������� ����� �������� � ������
    void __stdcall ResetAvgChannelDelay();
};
//-----------------------------------------------------------------------------
#endif




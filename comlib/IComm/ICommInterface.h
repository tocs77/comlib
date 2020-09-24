#pragma once

#ifndef ICommInterfaceH
#define ICommInterfaceH
//-----------------------------------------------------------------------------
#include <vector>
#include <Windows.h>
// #include "WinNT.h"  // ������ ���������� ������������� ��� ��������� "Windows.h"
//-----------------------------------------------------------------------------
class ICommInterface
{
public:
	virtual BOOL __stdcall Connect() = 0;
	virtual BOOL __stdcall Disconnect() = 0;
	virtual BOOL __stdcall IsConnected() = 0;


	// ��������� ������-�����
	// ���� ��������� ������� ���������� S_OK = 0x0 ����� ����������� ��� ���� ��� ������
	virtual HRESULT __stdcall WriteRead(
		BYTE const* const pOutBuffer,
		int iRequestLenght,
		BYTE* pInBuffer,
		DWORD* pAnswerLength
	) = 0;

	// ������ ������, ������� �� �� ����������� (���� ����)
	// ���� ��������� ������� ���������� S_OK = 0x0 ����� ����������� ��� ���� ��� ������
	virtual HRESULT __stdcall ReadUnrequestedData(
		std::unique_ptr<BYTE*> pInBuffer,
		int* pDataLength
	) = 0;

	//-----------------------------------------------------------
	// �������� ������� ����� �������� � ������ (���� �� ���� ������ �� 0)
	DWORD __stdcall GetAvgChannelDelay_mS();
	// �������� ������� ����� �������� � ������
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




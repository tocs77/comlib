#pragma once
#ifndef TTY_H
#define TTY_H

#define NOMINMAX //����� API windows ��������� ������� min � max, ������������� � std::max � std::min � vector
#include <windows.h>

#include <vector>
#include <string>

using namespace std;

struct TTY {

    TTY();
    virtual ~TTY();

    bool IsOK() const;
    void setCheckData(bool);

    bool Connect(LPCWSTR port, int baudrate);
    void Disconnect();

    virtual void Write(const vector<unsigned char>& data);
    virtual void Read();
    virtual void WriteAndRead(char* data);

private:

    HANDLE m_Handle;
    bool checkData;

};

struct TTYException {
};

#endif
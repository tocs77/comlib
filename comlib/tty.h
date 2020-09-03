#pragma once

#ifndef TTY_H
#define TTY_H

#define NOMINMAX //иначе API windows определит макросы min и max, конфликтующие с std::max и std::min в vector
constexpr int STOP_PORT_READ_EVENT = 0;
constexpr int  START_PORT_READ_EVENT = 1;
constexpr int EVENT_NUMBER = 2;

#include <windows.h>
#include <thread>
#include <vector>
#include <string>


struct TTY {

    TTY();
    virtual ~TTY();

    bool IsOK() const;

    bool Connect(LPCWSTR port, int baudrate);
    void Disconnect();

    virtual void Write(const std::vector<unsigned char>& data);
    virtual void Read();
    virtual HRESULT WriteAndRead(char* data);
    virtual void Start();

private:

    HANDLE m_Handle;
    HANDLE events[EVENT_NUMBER];
    std::thread* readHandle;

};

struct TTYException {
};

#endif
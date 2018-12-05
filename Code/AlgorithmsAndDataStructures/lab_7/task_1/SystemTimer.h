#pragma once

#define NOMINMAX

#include "Windows.h"

class SystemTimer
{
public:
    SystemTimer()
    {
        QueryPerformanceFrequency(&m_frequency);
    }

    long double Now() {
        LARGE_INTEGER now;
        QueryPerformanceCounter(&now);
        return static_cast<long double>(now.QuadPart) / m_frequency.QuadPart;
    }


private:
    LARGE_INTEGER m_frequency;
};

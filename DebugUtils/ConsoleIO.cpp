#include "ConsoleIO.h"

#include <iostream>


// TEMP
#include <Windows.h>
#include "AutoCloseHandle.hpp"

std::wstring ConsoleIO::read()
{
    return std::wstring();
}

void ConsoleIO::write(const std::wstring& data)
{
    std::wcout << data << std::endl;
}

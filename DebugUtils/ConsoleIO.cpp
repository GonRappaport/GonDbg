#include "ConsoleIO.h"

#include <iostream>
#include <sstream>


// TEMP
#include <Windows.h>
#include "AutoCloseHandle.hpp"

std::wstring ConsoleIO::read()
{
    wchar_t line[1024];
    std::wcin.getline(line, ARRAYSIZE(line));
    return std::wstring(line);
}

void ConsoleIO::write(const std::wstring& data)
{
    std::wcout << data << std::endl;
}

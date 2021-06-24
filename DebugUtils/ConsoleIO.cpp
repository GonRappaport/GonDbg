#include "ConsoleIO.h"

#include <iostream>


// TEMP
#include <Windows.h>
#include "AutoCloseHandle.hpp"

std::wstring ConsoleIO::read()
{
    std::wstring input;
    std::wcin >> input;
    return input;
}

void ConsoleIO::write(const std::wstring& data)
{
    std::wcout << data << std::endl;
}

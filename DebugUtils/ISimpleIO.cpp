#include "ISimpleIO.h"

#include <cstdarg>
#include <cstring>
#include <vector>

void ISimpleIO::write_formatted(const std::wstring_view fmt, ...)
{
    va_list args;
    va_start(args, fmt);

    write(_format(fmt, args));

    va_end(args);
}

std::wstring ISimpleIO::format(const std::wstring_view fmt, ...) const
{
    va_list args;
    va_start(args, fmt);

    std::wstring formatted_output(_format(fmt, args));

    va_end(args);

    return formatted_output;
}

std::wstring ISimpleIO::_format(const std::wstring_view& fmt, va_list args) const
{
    int formatted_length = _vscwprintf(fmt.data(), args);
    if (formatted_length <= 0)
    {
        throw std::exception("String formatting failed");
    }

    std::vector<wchar_t> output(static_cast<size_t>(formatted_length) + 1);

    if (_vsnwprintf_s(output.data(), output.size(), output.size(), fmt.data(), args) != formatted_length)
    {
        throw std::exception("Unexpected string formatting length");
    }

    return std::wstring(output.data());
}

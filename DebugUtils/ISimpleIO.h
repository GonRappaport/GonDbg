#pragma once

#include <Windows.h>
#include <string>
#include <vector>

class ISimpleIO
{
public:
	virtual std::wstring read() = 0;
	virtual void write(const std::wstring&) = 0;

	virtual void write_formatted(const std::wstring_view, ...);

	virtual std::wstring format(const std::wstring_view, ...) const;
	virtual std::wstring format_bytes(const std::vector<BYTE>) const;

	virtual ~ISimpleIO() = default;

private:
	std::wstring _format(const std::wstring_view&, va_list) const;
};


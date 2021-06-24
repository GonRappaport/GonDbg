#pragma once

#include <string>

class ISimpleIO
{
public:
	virtual std::wstring read() = 0;
	virtual void write(const std::wstring&) = 0;
	virtual void write_formatted(const std::wstring_view, ...);

	virtual std::wstring format(const std::wstring_view, ...) const;

	virtual ~ISimpleIO() = default;

private:
	std::wstring _format(const std::wstring_view&, va_list) const;
};


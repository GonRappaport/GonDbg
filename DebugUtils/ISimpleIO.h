#pragma once

#include <Windows.h>
#include <string>
#include <vector>

class ICtrlHandler
{
public:
	virtual bool handle_control(const DWORD ctrl_type) = 0;
};

class ISimpleIO
{
public:
	virtual std::wstring read() = 0;
	virtual void write(const std::wstring&) = 0;
	virtual std::wstring prompt(const std::wstring&) = 0;
	virtual void register_break_handler(ICtrlHandler*) = 0;
	virtual void unregister_break_handler() = 0;

	virtual void write_formatted(const std::wstring_view, ...);

	static std::wstring format(const std::wstring_view, ...);
	static std::wstring format_bytes(const std::vector<BYTE>);

	virtual ~ISimpleIO() = default;

private:
	static std::wstring _format(const std::wstring_view&, va_list);
};


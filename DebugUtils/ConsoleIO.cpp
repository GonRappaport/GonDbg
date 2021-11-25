#include "ConsoleIO.h"

#include <iostream>
#include <sstream>

#include "Exceptions.h"

bool ConsoleIO::m_registered_handler = false;
std::list<ICtrlHandler*> ConsoleIO::m_registrations;

std::wstring ConsoleIO::read()
{
	// In case someone pressed ctrl+c while input is requested
	if (!std::wcin.good())
	{
		std::wcin.clear();
	}
    wchar_t line[1024];
    std::wcin.getline(line, ARRAYSIZE(line));
    return std::wstring(line);
}

void ConsoleIO::write(const std::wstring& data)
{
    std::wcout << data << std::endl;
}

std::wstring ConsoleIO::prompt(const std::wstring& data)
{
    std::wcout << data.c_str(); // Not including newline
    return read();
}

void ConsoleIO::register_break_handler(ICtrlHandler* handler)
{
	// TODO: Consider adding a wrapper for that for safe removal
	m_handler = handler;
	ConsoleIO::m_registrations.push_back(handler);
	if (!ConsoleIO::m_registered_handler)
	{
		if (!SetConsoleCtrlHandler(ctrl_handler, true))
		{
			throw WinAPIException("SetConsoleCtrlHandler failed");
		}
		ConsoleIO::m_registered_handler = true;
	}
}

BOOL WINAPI ConsoleIO::ctrl_handler(DWORD ctrl_type)
{
	bool result = false;

	// TODO: Add synchronization
	for (auto handler : ConsoleIO::m_registrations)
	{
		// If one of the handlers handled the event, stop processing other handlers that weren't registered by us
		result |= handler->handle_control(ctrl_type);
	}

	return result;
}

void ConsoleIO::unregister_break_handler()
{
	if (nullptr != m_handler)
	{
		ConsoleIO::m_registrations.remove(m_handler);
		if (ConsoleIO::m_registrations.empty())
		{
			SetConsoleCtrlHandler(ctrl_handler, false); // TODO: See what to do if it fails
			ConsoleIO::m_registered_handler = false;
		}
	}
}

ConsoleIO::~ConsoleIO()
{
	try
	{
		// TODO: Is it ok to use it here? It's a virtual function
		unregister_break_handler();
	}
	catch (...)
	{

	}
}

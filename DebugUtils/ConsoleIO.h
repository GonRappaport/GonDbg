#pragma once

#include <list>

#include "ISimpleIO.h"

class ConsoleIO: 
	public ISimpleIO
{
public:
	ConsoleIO() :
		m_handler(nullptr)
	{}

	virtual ~ConsoleIO();

	virtual std::wstring read();
	virtual void write(const std::wstring&);
	virtual std::wstring prompt(const std::wstring&);
	virtual void register_break_handler(ICtrlHandler*);
	virtual void unregister_break_handler();

	ConsoleIO(const ConsoleIO&) = delete;
	ConsoleIO& operator=(const ConsoleIO&) = delete;

	ConsoleIO(ConsoleIO&& cch) noexcept :
		m_handler(std::exchange(cch.m_handler, nullptr))
	{}
	ConsoleIO&& operator=(ConsoleIO&&) = delete;

private:
	ICtrlHandler* m_handler;
	static std::list<ICtrlHandler*> m_registrations;
	static bool m_registered_handler;

	static BOOL WINAPI ctrl_handler(DWORD ctrl_type);
};


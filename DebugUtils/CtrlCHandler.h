#pragma once

#include <Windows.h>
#include <list>

class ICtrlHandler
{
public:
	virtual bool handle_control(const DWORD ctrl_type) = 0;
};

// TODO: Rename this and the files to ConsoleCtrlHandler
class CtrlCHandler
{
public:
	explicit CtrlCHandler(ICtrlHandler* handler);

	CtrlCHandler(const CtrlCHandler&) = delete;
	CtrlCHandler& operator=(const CtrlCHandler&) = delete;

	CtrlCHandler(CtrlCHandler&& cch) noexcept :
		m_handler(cch.m_handler)
	{
		cch.m_handler = nullptr;
	}

	~CtrlCHandler();

private:
	ICtrlHandler* m_handler;

	// Registrations
	// TODO: Implement an interface that only requires one function (ctrl_handler or w/e) and keep them in the list. Then call them in order.
	// Then, have Debugger inherit from that interface and implement that function. Easy.
	static std::list<ICtrlHandler*> m_registrations;
	static bool m_registered_handler;
	
	static BOOL WINAPI ctrl_handler(DWORD ctrl_type);
};


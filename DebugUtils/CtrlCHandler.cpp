#include "CtrlCHandler.h"

#include <exception>

bool CtrlCHandler::m_registered_handler = false;
std::list<ICtrlHandler*> CtrlCHandler::m_registrations;

BOOL WINAPI CtrlCHandler::ctrl_handler(DWORD ctrl_type)
{
	bool result = false;

	for (auto handler : CtrlCHandler::m_registrations)
	{
		// If one of the handlers handled the event, stop processing other handlers that weren't registered by us
		result |= handler->handle_control(ctrl_type);
	}

	return result;
}

CtrlCHandler::CtrlCHandler(ICtrlHandler* handler):
	m_handler(handler)
{
	// TODO: Consider adding a wrapper for that for safe removal
	CtrlCHandler::m_registrations.push_back(handler);
	if (!CtrlCHandler::m_registered_handler)
	{
		if (!SetConsoleCtrlHandler(ctrl_handler, true))
		{
			throw std::exception("SetConsoleCtrlHandler failed");
		}
		CtrlCHandler::m_registered_handler = true;
	}
}

CtrlCHandler::~CtrlCHandler()
{
	try
	{
		if (nullptr != m_handler)
		{
			CtrlCHandler::m_registrations.remove(m_handler);
			if (CtrlCHandler::m_registrations.empty())
			{
				SetConsoleCtrlHandler(ctrl_handler, false); // TODO: See what to do if it fails
				CtrlCHandler::m_registered_handler = false;
			}
		}
	}
	catch (...)
	{

	}
}

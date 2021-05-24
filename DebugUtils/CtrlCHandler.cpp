#include "CtrlCHandler.h"

#include <exception>

CtrlCHandler::m_registered_handler = false;

BOOL WINAPI ctrl_handler(DWORD ctrl_type)
{

}

CtrlCHandler::CtrlCHandler(PFN_CTRLHANDLER handler, void* context):
	m_handler(handler),
	m_context(context)
{
	// TODO: Consider adding a wrapper for that for safe removal
	if (!SetConsoleCtrlHandler(m_handler, true))
	{
		throw std::exception("SetConsoleCtrlHandler failed");
	}
}

CtrlCHandler::~CtrlCHandler()
{
	try
	{
		SetConsoleCtrlHandler(m_handler, false); // TODO: See what to do if it fails
	}
	catch (...)
	{

	}
}

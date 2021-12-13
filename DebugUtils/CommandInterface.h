#pragma once

#include <Windows.h>

enum class CommandResponse : DWORD
{
	NoResponse,
	ContinueExecution,
	ContinueHandled,
	ContinueUnhandled,
	ContinueDelayed // Unsupported // TODO: DBG_REPLY_LATER
};
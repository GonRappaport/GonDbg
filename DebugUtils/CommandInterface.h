#pragma once

#include <Windows.h>

enum class CommandResponse : DWORD
{
	NoResponse,
	ContinueExecution,
	ContinueHandled,
	ContinueUnhandled
};
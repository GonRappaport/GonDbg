#pragma once

#include "Debugger.h"

namespace DebuggerCommands
{
	DWORD help(std::wstring params, Debugger& debugger);
	DWORD go(std::wstring params, Debugger& debugger);
	DWORD list_modules(std::wstring params, Debugger& debugger);
	DWORD read_memory(std::wstring params, Debugger& debugger);
	DWORD quit(std::wstring params, Debugger& debugger);
	DWORD get_symbol_name(std::wstring params, Debugger& debugger);

	std::vector<std::pair<std::wstring, CommandInterface>> get_commands();
}
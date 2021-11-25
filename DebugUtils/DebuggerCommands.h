#pragma once

#include "Debugger.h"

namespace DebuggerCommands
{
	DWORD help(const std::wstring& params, Debugger& debugger);
	DWORD go(const std::wstring& params, Debugger& debugger);
	DWORD list_modules(const std::wstring& params, Debugger& debugger);
	DWORD read_memory(const std::wstring& params, Debugger& debugger);
	DWORD quit(const std::wstring& params, Debugger& debugger);
	DWORD get_symbol_name(const std::wstring& params, Debugger& debugger);

	std::vector<std::pair<std::wstring, CommandInterface>> get_commands();
}
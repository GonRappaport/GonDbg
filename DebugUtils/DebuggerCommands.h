#pragma once

#include "Debugger.h"

namespace DebuggerCommands
{
	// TODO: Sort that to categores
	CommandResponse help(const std::wstring& params, Debugger& debugger);
	CommandResponse go(const std::wstring& params, Debugger& debugger);
	CommandResponse list_modules(const std::wstring& params, Debugger& debugger);
	CommandResponse read_memory(const std::wstring& params, Debugger& debugger);
	CommandResponse quit(const std::wstring& params, Debugger& debugger);
	CommandResponse get_symbol_name(const std::wstring& params, Debugger& debugger);
	CommandResponse step(const std::wstring& params, Debugger& debugger);
	CommandResponse list_threads(const std::wstring& params, Debugger& debugger);

	std::vector<std::pair<std::wstring, CommandInterface>> get_commands();
}
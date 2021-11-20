#include "DebuggerCommands.h"

// TODO: Define the enum for the magic number 1 here used to indicate run another command. Also, make it the default command ret value

DWORD DebuggerCommands::help(std::wstring params, Debugger& debugger)
{
	UNREFERENCED_PARAMETER(params);

	debugger.get_io_handler()->write(L"Supported commands:");
	for (const auto& command : get_commands())
	{
		debugger.get_io_handler()->write_formatted(L"%s", 
			command.first.c_str());
	}
	
	return 1;
}

DWORD DebuggerCommands::go(std::wstring params, Debugger& debugger)
{
	UNREFERENCED_PARAMETER(params);
	UNREFERENCED_PARAMETER(debugger);
    return DBG_EXCEPTION_NOT_HANDLED;
}

DWORD DebuggerCommands::list_modules(std::wstring params, Debugger& debugger)
{
	UNREFERENCED_PARAMETER(params);

	auto loaded_modules = debugger.get_symbol_finder().get_loaded_modules();
	for (auto m : loaded_modules)
	{
		debugger.get_io_handler()->write_formatted(L"0x%llX (0x%08lX): %s",
			static_cast<DWORD64>(m.get_image_base()),
			m.get_image_size(),
			m.get_image_name().c_str());
	}

	return 1;
}

DWORD DebuggerCommands::read_memory(std::wstring params, Debugger& debugger)
{
	const SIZE_T DEFAULT_READ_LENGTH = 0x40;
	RemotePointer address;
	SIZE_T length = DEFAULT_READ_LENGTH;

	int read_values = swscanf_s(params.c_str(), L"%Ix %Ix", &address, &length);

	switch (read_values)
	{
	case 1:
		__fallthrough;
	case 2:
		break;
	default:
		debugger.get_io_handler()->write(L"Invalid syntax. Usage: db <address> [L<length>]");
		return 1;
	}

	// TODO: Add support for invalid addresses (An exception). Maybe wrap this entire function with try
	auto data = debugger.get_process()->read_memory(address, length);
	debugger.get_io_handler()->write(debugger.get_io_handler()->format_bytes(data).c_str());

	return 1;
}

DWORD DebuggerCommands::quit(std::wstring params, Debugger& debugger)
{
	UNREFERENCED_PARAMETER(params);
	UNREFERENCED_PARAMETER(debugger);
	throw std::exception("Exiting debugger");
}

DWORD DebuggerCommands::get_symbol_name(std::wstring params, Debugger& debugger)
{
	RemotePointer address;

	int read_values = swscanf_s(params.c_str(), L"%Ix", &address);
	switch (read_values)
	{
	case 1:
		break;
	default:
		debugger.get_io_handler()->write(L"Invalid syntax. Usage: x <address>");
		return 1;
	}

	debugger.get_io_handler()->write(debugger.get_symbol_finder().get_symbol(address));

	return 1;
}

std::vector<std::pair<std::wstring, CommandInterface>> DebuggerCommands::get_commands()
{
	return {
		std::make_pair(L"help", help),
		std::make_pair(L"g", go),
		std::make_pair(L"lm", list_modules),
		std::make_pair(L"db", read_memory),
		std::make_pair(L"exit", quit),
		std::make_pair(L"x", get_symbol_name),
	};
}

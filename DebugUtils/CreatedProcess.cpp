#include "CreatedProcess.h"

AutoCloseHandle CreatedProcess::_s_create_process(const std::wstring& exe_path, std::wstring command_line)
{
	STARTUPINFOW startup_info = { 0 };
	PROCESS_INFORMATION process_info = { 0 };

	startup_info.cb = sizeof(startup_info);

	if (!CreateProcessW(exe_path.c_str(),
		(command_line.length() > 0) ? (command_line.data()) : (nullptr),
		nullptr,
		nullptr,
		false,
		CREATE_NEW_CONSOLE | DEBUG_ONLY_THIS_PROCESS/* TODO: Support debugging child processes */,
		nullptr,
		nullptr,
		&startup_info,
		&process_info))
	{
		throw std::exception("Process creation failed");
	}

	CloseHandle(process_info.hThread);

	return AutoCloseHandle(process_info.hProcess);
}

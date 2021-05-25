#include "Process.h"

HANDLE Process::get_process_handle()
{
	return m_handle.get_value();
}

std::vector<BYTE> Process::read_memory(PVOID base_address, SIZE_T size)
{
	UNREFERENCED_PARAMETER(base_address);
	UNREFERENCED_PARAMETER(size);
	return std::vector<BYTE>();
}

void Process::write_memory(PVOID base_address, std::vector<BYTE> data)
{
	UNREFERENCED_PARAMETER(base_address);
	UNREFERENCED_PARAMETER(data);
}

AutoCloseHandle&& Process::_s_create_process(const std::wstring& exe_path, std::wstring command_line)
{
	STARTUPINFOW startup_info = { 0 };
	PROCESS_INFORMATION process_info = { 0 };

	startup_info.cb = sizeof(startup_info);

	if (!CreateProcessW(exe_path.c_str(),
		(command_line.length() > 0) ? (command_line.data()) : (nullptr),
		nullptr,
		nullptr,
		false,
		DEBUG_ONLY_THIS_PROCESS /* TODO: Support debugging child processes */,
		nullptr,
		nullptr,
		&startup_info,
		&process_info))
	{
		throw std::exception("Process creation failed");
	}

	CloseHandle(process_info.hThread);

	return std::move(AutoCloseHandle(process_info.hProcess));
}

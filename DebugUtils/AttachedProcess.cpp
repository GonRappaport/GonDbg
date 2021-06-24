#include "AttachedProcess.h"

HANDLE AttachedProcess::get_process_handle() const
{
	return m_handle.get_value();
}

DWORD AttachedProcess::get_process_id() const
{
	if (0 == m_process_id)
	{
		// TODO: Or can it? ;)
		throw std::exception("Process ID can't be 0");
	}
	return m_process_id;
}

bool AttachedProcess::is_64_bit() const
{
	return m_64_bit;
}

// TODO: Combine this and Process.h to inherit from IRunningProcess which will implement these functions and similar. This is in contrast to RecordedProcess which is for TTD
std::vector<BYTE> AttachedProcess::read_memory(PVOID base_address, SIZE_T size)
{
	SIZE_T bytes_read = 0;
	std::vector<BYTE> data(size);

	if ((!ReadProcessMemory(m_handle.get_value(), base_address, data.data(), data.size(), &bytes_read)) ||
		(bytes_read != size))
	{
		throw std::exception("ReadProcessMemory failed");
	}

	return data;
}

void AttachedProcess::write_memory(PVOID base_address, std::vector<BYTE> data)
{
	UNREFERENCED_PARAMETER(base_address);
	UNREFERENCED_PARAMETER(data);
}

AutoCloseHandle AttachedProcess::_s_attach_process(const DWORD process_id)
{
	// TODO: I request more rights here than are requested by DebugActiveProcess. Maybe minimize it? Or allow it to not be used?
	AutoCloseHandle process_handle(OpenProcess(PROCESS_ALL_ACCESS, false, process_id));

	if (!DebugActiveProcess(process_id))
	{
		throw std::exception("DebugActiveProcess failed");
	}

	return std::move(process_handle);
}

bool AttachedProcess::_s_is_64_bit(const HANDLE process_handle)
{
	SYSTEM_INFO info;

	GetNativeSystemInfo(&info);

	if (info.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_INTEL)
	{
		return false;
	}

	BOOL is_wow64 = FALSE;
	HMODULE kernel32 = GetModuleHandleA("kernel32.dll");
	if (nullptr == kernel32)
	{
		throw std::exception("GetModuleHandleA failed");
	}

	auto is_wow64_process = reinterpret_cast<decltype(IsWow64Process)*>(GetProcAddress(kernel32, "IsWow64Process"));
	if (nullptr == is_wow64_process)
	{
		// For 32bit systems, it's possible for this function to be missing. However, we validated that we're running under 64bit Windows.
		throw std::exception("GetProcAddress failed");
	}

	if (!is_wow64_process(process_handle, &is_wow64))
	{
		throw std::exception("IsWow64Process failed");
	}

	return !is_wow64;
}
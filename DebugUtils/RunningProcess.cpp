#include "RunningProcess.h"

bool RunningProcess::_s_is_64_bit(const HANDLE process_handle)
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

HANDLE RunningProcess::get_process_handle() const
{
	return m_handle.get_value();
}

DWORD RunningProcess::get_process_id() const
{
	if (0 == m_process_id)
	{
		// TODO: Or can it? ;)
		throw std::exception("Process ID can't be 0");
	}
	return m_process_id;
}

bool RunningProcess::is_64_bit() const
{
	return m_64_bit;
}

std::vector<BYTE> RunningProcess::read_memory(RemotePointer base_address, SIZE_T size)
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

void RunningProcess::write_memory(RemotePointer base_address, std::vector<BYTE> data)
{
	UNREFERENCED_PARAMETER(base_address);
	UNREFERENCED_PARAMETER(data);
}
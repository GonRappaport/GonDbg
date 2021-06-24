#include "Process.h"

// Copied from wdm.h
#define PAGE_SIZE (0x1000)
// TODO: Convert to a good C++ function
#define PAGE_ALIGN(Va) ((PVOID)((ULONG_PTR)(Va) & ~(PAGE_SIZE - 1)))
#define PAGE_ALIGN_NEXT_PAGE(Va) ((PVOID)((ULONG_PTR)PAGE_ALIGN((Va)) + PAGE_SIZE))

HANDLE Process::get_process_handle() const
{
	return m_handle.get_value();
}

DWORD Process::get_process_id() const
{
	if (0 == m_process_id)
	{
		throw std::exception("Process ID can't be 0");
	}
	return m_process_id;
}

bool Process::is_64_bit() const
{
	return m_64_bit;
}

std::vector<BYTE> Process::read_memory(PVOID base_address, SIZE_T size)
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

std::vector<BYTE> Process::_read_page(PVOID base_address)
{
	const SIZE_T bytes_to_read = PAGE_SIZE - (reinterpret_cast<SIZE_T>(base_address) & 0xFFF);

	return read_memory(base_address, bytes_to_read);
}

void Process::write_memory(PVOID base_address, std::vector<BYTE> data)
{
	UNREFERENCED_PARAMETER(base_address);
	UNREFERENCED_PARAMETER(data);
}

PVOID Process::read_pointer(PVOID base_address)
{
	const uint8_t pointer_size = m_64_bit ? 8 : 4;
	// TODO: No support for 32bit process debugging 64bit, so we're safe
	PVOID pointer_value;

	// TODO: Turn this into an assert
	if (pointer_size > sizeof(pointer_value))
	{
		throw std::exception("Impossible pointer sizes");
	}

	auto pointer_data = read_memory(base_address, pointer_size);

	if (m_64_bit)
	{
		pointer_value = reinterpret_cast<PVOID>(*(reinterpret_cast<uint64_t*>(pointer_data.data())));
	}
	else
	{
		pointer_value = reinterpret_cast<PVOID>(*(reinterpret_cast<uint32_t*>(pointer_data.data())));
	}

	return pointer_value;
}

std::string Process::read_string(PVOID base_address)
{
	std::vector<BYTE> raw_data;

	do
	{
		auto new_data = _read_page(base_address);
		// Just to improve performance
		raw_data.reserve(raw_data.size() + std::distance(new_data.begin(), new_data.end()));
		raw_data.insert(raw_data.end(), new_data.begin(), new_data.end());
		base_address = PAGE_ALIGN_NEXT_PAGE(base_address);
	} while (strnlen(reinterpret_cast<char*>(raw_data.data()), raw_data.size()) == raw_data.size());

	return std::string(reinterpret_cast<char*>(raw_data.data()));
}

std::wstring Process::read_wstring(PVOID base_address)
{
	std::vector<BYTE> raw_data;

	do
	{
		auto new_data = _read_page(base_address);
		// Just to improve performance
		raw_data.reserve(raw_data.size() + std::distance(new_data.begin(), new_data.end()));
		raw_data.insert(raw_data.end(), new_data.begin(), new_data.end());
		// TODO: To improve performance in case of large strings, perform the null-search only in new_data, not the entire string
		base_address = PAGE_ALIGN_NEXT_PAGE(base_address);
		// TODO: Assert raw_data.size() divides by 2. And overall prettify this function
	} while ((wcsnlen(reinterpret_cast<wchar_t*>(raw_data.data()), raw_data.size() / sizeof(wchar_t)) * sizeof(wchar_t)) == raw_data.size());

	return std::wstring(reinterpret_cast<wchar_t*>(raw_data.data()));
}

std::string Process::read_string_capped(PVOID base_address, SIZE_T max_length)
{
	auto raw_data = read_memory(base_address, max_length);
	return std::string(reinterpret_cast<char*>(raw_data.data()));
}

std::wstring Process::read_wstring_capped(PVOID base_address, SIZE_T max_length)
{
	auto raw_data = read_memory(base_address, max_length);
	return std::wstring(reinterpret_cast<wchar_t*>(raw_data.data()));
}

AutoCloseHandle Process::_s_create_process(const std::wstring& exe_path, std::wstring command_line)
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

	return AutoCloseHandle(process_info.hProcess);
}

bool Process::_s_is_64_bit(const HANDLE process_handle)
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

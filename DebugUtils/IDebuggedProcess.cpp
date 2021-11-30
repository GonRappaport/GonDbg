#include "IDebuggedProcess.h"

// Copied from wdm.h
#define PAGE_SIZE (0x1000)
// TODO: Convert to a good C++ function
#define PAGE_ALIGN(Va) ((RemotePointer)((ULONG_PTR)(Va) & ~(PAGE_SIZE - 1)))
#define PAGE_ALIGN_NEXT_PAGE(Va) ((RemotePointer)((ULONG_PTR)PAGE_ALIGN((Va)) + PAGE_SIZE))

std::vector<BYTE> IDebuggedProcess::_read_page(RemotePointer base_address)
{
	const SIZE_T bytes_to_read = PAGE_SIZE - (static_cast<DWORD64>(base_address) & 0xFFF);

	return read_memory(base_address, bytes_to_read);
}

RemotePointer IDebuggedProcess::read_pointer(RemotePointer base_address)
{
	const uint8_t pointer_size = is_64_bit() ? 8 : 4;
	// TODO: No support for 32bit process debugging 64bit, so we're safe
	RemotePointer pointer_value;

	// TODO: Turn this into an assert
	if (pointer_size > sizeof(pointer_value))
	{
		throw std::exception("Impossible pointer sizes");
	}

	auto pointer_data = read_memory(base_address, pointer_size);

	if (is_64_bit())
	{
		pointer_value = static_cast<DWORD64>(*(reinterpret_cast<uint64_t*>(pointer_data.data())));
	}
	else
	{
		pointer_value = static_cast<DWORD64>(*(reinterpret_cast<uint32_t*>(pointer_data.data())));
	}

	return pointer_value;
}

std::string IDebuggedProcess::read_string(RemotePointer base_address)
{
	std::vector<BYTE> raw_data;

	do
	{
		auto new_data = _read_page(base_address);
		// Just to improve performance
		// TODO: Switch to using some adaptation over iostream or std::string with append/push_back
		raw_data.reserve(raw_data.size() + std::distance(new_data.begin(), new_data.end()));
		raw_data.insert(raw_data.end(), new_data.begin(), new_data.end());
		base_address = PAGE_ALIGN_NEXT_PAGE(base_address);
	} while (strnlen(reinterpret_cast<char*>(raw_data.data()), raw_data.size()) == raw_data.size());

	return std::string(reinterpret_cast<char*>(raw_data.data()));
}

std::wstring IDebuggedProcess::read_wstring(RemotePointer base_address)
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

std::string IDebuggedProcess::read_string_capped(RemotePointer base_address, SIZE_T max_length)
{
	auto raw_data = read_memory(base_address, max_length);
	return std::string(reinterpret_cast<char*>(raw_data.data()));
}

std::wstring IDebuggedProcess::read_wstring_capped(RemotePointer base_address, SIZE_T max_length)
{
	auto raw_data = read_memory(base_address, max_length);
	return std::wstring(reinterpret_cast<wchar_t*>(raw_data.data()));
}
#include "IDebuggedProcess.h"

// Copied from wdm.h
#define PAGE_SIZE (0x1000)
// TODO: Convert to a good C++ function
#define PAGE_ALIGN(Va) ((PVOID)((ULONG_PTR)(Va) & ~(PAGE_SIZE - 1)))
#define PAGE_ALIGN_NEXT_PAGE(Va) ((PVOID)((ULONG_PTR)PAGE_ALIGN((Va)) + PAGE_SIZE))

std::vector<BYTE> IDebuggedProcess::_read_page(PVOID base_address)
{
	const SIZE_T bytes_to_read = PAGE_SIZE - (reinterpret_cast<SIZE_T>(base_address) & 0xFFF);

	return read_memory(base_address, bytes_to_read);
}

PVOID IDebuggedProcess::read_pointer(PVOID base_address)
{
	const uint8_t pointer_size = is_64_bit() ? 8 : 4;
	// TODO: No support for 32bit process debugging 64bit, so we're safe
	PVOID pointer_value;

	// TODO: Turn this into an assert
	if (pointer_size > sizeof(pointer_value))
	{
		throw std::exception("Impossible pointer sizes");
	}

	auto pointer_data = read_memory(base_address, pointer_size);

	if (is_64_bit())
	{
		pointer_value = reinterpret_cast<PVOID>(*(reinterpret_cast<uint64_t*>(pointer_data.data())));
	}
	else
	{
		pointer_value = reinterpret_cast<PVOID>(*(reinterpret_cast<uint32_t*>(pointer_data.data())));
	}

	return pointer_value;
}

std::string IDebuggedProcess::read_string(PVOID base_address)
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

std::wstring IDebuggedProcess::read_wstring(PVOID base_address)
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

std::string IDebuggedProcess::read_string_capped(PVOID base_address, SIZE_T max_length)
{
	auto raw_data = read_memory(base_address, max_length);
	return std::string(reinterpret_cast<char*>(raw_data.data()));
}

std::wstring IDebuggedProcess::read_wstring_capped(PVOID base_address, SIZE_T max_length)
{
	auto raw_data = read_memory(base_address, max_length);
	return std::wstring(reinterpret_cast<wchar_t*>(raw_data.data()));
}
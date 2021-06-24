#pragma once

#include <Windows.h>
#include <vector>
#include <string>

/*
class RemotePointer
{
public:
	RemotePointer(void* ptr)

private:
	
};
*/

class IDebuggedProcess
{
public:
	virtual HANDLE get_process_handle() const = 0;
	virtual DWORD get_process_id() const = 0;
	virtual bool is_64_bit() const = 0;
	// No need to use PVOID64 and such as it's impossible to debug 64-bit processes from a 32-bit debugger
	// TODO: Consider switching from PVOID to not accidently access this as a pointer in the local process
	virtual std::vector<BYTE> read_memory(PVOID base_address, SIZE_T size) = 0;
	virtual void write_memory(PVOID base_address, std::vector<BYTE> data) = 0;
	// TODO: That requires you to keep data of if the process is 32 or 64 bit
	virtual PVOID read_pointer(PVOID base_address);
	virtual std::string read_string(PVOID base_address);
	virtual std::wstring read_wstring(PVOID base_address);
	virtual std::string read_string_capped(PVOID base_address, SIZE_T max_length);
	virtual std::wstring read_wstring_capped(PVOID base_address, SIZE_T max_length);

	virtual ~IDebuggedProcess() = default;

protected:
	virtual std::vector<BYTE> _read_page(PVOID base_address);
};
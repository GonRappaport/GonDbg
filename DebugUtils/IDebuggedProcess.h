#pragma once

#include <Windows.h>
#include <vector>

class IDebuggedProcess
{
public:
	virtual HANDLE get_process_handle() = 0;
	// No need to use PVOID64 and such as it's impossible to debug 64-bit processes from a 32-bit debugger
	// TODO: Consider switching from PVOID to not accidently access this as a pointer in the local process
	virtual std::vector<BYTE> read_memory(PVOID base_address, SIZE_T size) = 0;
	virtual void write_memory(PVOID base_address, std::vector<BYTE> data) = 0;

	virtual ~IDebuggedProcess() = default;
};
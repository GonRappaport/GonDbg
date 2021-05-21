#pragma once

#include <Windows.h>
#include <string>

namespace ProcUtils
{
	const DWORD process_name_to_pid(const std::wstring& process_name);
}

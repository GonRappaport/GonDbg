#include "Debugger.h"
#include "ProcUtils.h"

void Debugger::debug()
{
}

DebuggerPtr DebuggerUtils::attach(const DWORD pid, bool is_invasive)
{
	return std::make_unique<Debugger>();
}

DebuggerPtr DebuggerUtils::attach(const std::wstring& process_name, bool is_invasive)
{
	DWORD pid = ProcUtils::process_name_to_pid(process_name);
	return DebuggerUtils::attach(pid, is_invasive);
}

DebuggerPtr DebuggerUtils::create(const std::wstring& exe_path)
{
	return std::make_unique<Debugger>();
}
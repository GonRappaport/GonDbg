#include "Debugger.h"
#include "ProcUtils.h"
#include "Process.h"

void Debugger::debug()
{
	DEBUG_EVENT debug_event;
	
	while (true)
	{
		if (!wait_for_debug_event(&debug_event, INFINITE))
		{
			throw std::exception("WaitForDebugEvent(Ex) failed"); // TODO: Add GetLastError everywhere
		}
		DWORD continue_status = dispatch_debug_event(debug_event);
		UNREFERENCED_PARAMETER(continue_status);
		ContinueDebugEvent(debug_event.dwProcessId, debug_event.dwThreadId, DBG_EXCEPTION_NOT_HANDLED);
	}
}

DWORD Debugger::dispatch_debug_event(const DEBUG_EVENT& debug_event)
{
	switch (debug_event.dwDebugEventCode)
	{
	case EXCEPTION_DEBUG_EVENT:
		return dispatch_exception(debug_event);
	case CREATE_THREAD_DEBUG_EVENT:
		return dispatch_thread_creation(debug_event);
	case CREATE_PROCESS_DEBUG_EVENT:
		return dispatch_process_creation(debug_event);
	case EXIT_THREAD_DEBUG_EVENT:
		return dispatch_thread_termination(debug_event);
	case EXIT_PROCESS_DEBUG_EVENT:
		return dispatch_process_termination(debug_event);
	case LOAD_DLL_DEBUG_EVENT:
		return dispatch_module_load(debug_event);
	case UNLOAD_DLL_DEBUG_EVENT:
		return dispatch_module_unload(debug_event);
	case OUTPUT_DEBUG_STRING_EVENT:
		return dispatch_debug_string(debug_event);
	case RIP_EVENT:
		return dispatch_rip(debug_event);
	default:
		throw std::exception("Unknown debug event");
	}
}

DWORD Debugger::dispatch_exception(const DEBUG_EVENT& debug_event)
{
	UNREFERENCED_PARAMETER(debug_event);
	return 0;
}

DWORD Debugger::dispatch_thread_creation(const DEBUG_EVENT& debug_event)
{
	UNREFERENCED_PARAMETER(debug_event);
	return 0;
}

DWORD Debugger::dispatch_process_creation(const DEBUG_EVENT& debug_event)
{
	UNREFERENCED_PARAMETER(debug_event);
	return 0;
}

DWORD Debugger::dispatch_thread_termination(const DEBUG_EVENT& debug_event)
{
	UNREFERENCED_PARAMETER(debug_event);
	return 0;
}

DWORD Debugger::dispatch_process_termination(const DEBUG_EVENT& debug_event)
{
	UNREFERENCED_PARAMETER(debug_event);
	return 0;
}

DWORD Debugger::dispatch_module_load(const DEBUG_EVENT& debug_event)
{
	UNREFERENCED_PARAMETER(debug_event);
	return 0;
}

DWORD Debugger::dispatch_module_unload(const DEBUG_EVENT& debug_event)
{
	UNREFERENCED_PARAMETER(debug_event);
	return 0;
}

DWORD Debugger::dispatch_debug_string(const DEBUG_EVENT& debug_event)
{
	UNREFERENCED_PARAMETER(debug_event);
	return 0;
}

DWORD Debugger::dispatch_rip(const DEBUG_EVENT& debug_event)
{
	UNREFERENCED_PARAMETER(debug_event);
	return 0;
}

bool Debugger::handle_control(const DWORD ctrl_type)
{
	// TODO: See what to do if it fails. Is it even the same thread?
	if (ctrl_type == CTRL_C_EVENT)
	{
		DebugBreakProcess(m_debugged_process->get_process_handle());
		// TODO: Return true to avoid the process from terminating?
	}
	return false;
}

PFN_WAITFORDEBUGEVENT Debugger::_cache_wait_for_debug_event()
{
	HMODULE kernel32 = GetModuleHandleA("kernel32.dll");
	if (nullptr == kernel32)
	{
		throw std::exception("Couldn't find kernel32.dll");
	}

	// WaitForDebugEventEx is the preferred function to use but it only exists on Windows 10
	FARPROC retval = GetProcAddress(kernel32, "WaitForDebugEventEx");
	if (nullptr == retval)
	{
		return WaitForDebugEvent;
	}

	return reinterpret_cast<PFN_WAITFORDEBUGEVENT>(retval);
}

Debugger&& Debugger::attach_to_process(const DWORD pid, bool is_invasive)
{
	UNREFERENCED_PARAMETER(pid);
	UNREFERENCED_PARAMETER(is_invasive);
	throw std::exception();
}

Debugger&& Debugger::attach_to_process(const std::wstring& process_name, bool is_invasive)
{
	DWORD pid = ProcUtils::process_name_to_pid(process_name);
	return Debugger::attach_to_process(pid, is_invasive);
}

Debugger&& Debugger::debug_new_process(const std::wstring& exe_path)
{
	return std::move(Debugger(std::move(std::make_unique<Process>(exe_path)), GetCurrentThreadId()));
}
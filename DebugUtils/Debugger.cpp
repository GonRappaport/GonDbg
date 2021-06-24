#include <optional>

#include "Debugger.h"
#include "ProcUtils.h"
#include "Process.h"

// TODO: Replace this with io.h
#include <iostream>

void Debugger::debug()
{
	DEBUG_EVENT debug_event;
	
	try
	{
		while (true)
		{
			if (!wait_for_debug_event(&debug_event, INFINITE))
			{
				throw std::exception("WaitForDebugEvent(Ex) failed"); // TODO: Add GetLastError everywhere
			}
			DWORD continue_status = dispatch_debug_event(debug_event);
			UNREFERENCED_PARAMETER(continue_status);
			// TODO: For time travel debugging, hook this function with one that does nothing (I think) to simply "debug" the trace. You'll also hook the wait function
			ContinueDebugEvent(debug_event.dwProcessId, debug_event.dwThreadId, DBG_EXCEPTION_NOT_HANDLED);
		}
	}
	catch (const std::exception& e)
	{
		if (0 != memcmp(e.what(), "Debugged process died", sizeof("Debugged process died")))
		{
			throw;
		}
	}
}

std::wstring Debugger::_read_remote_string(PVOID base_address, bool is_unicode)
{
	if (nullptr != base_address)
	{
		auto remote_address = m_debugged_process->read_pointer(base_address);
		if (nullptr != remote_address)
		{
			if (is_unicode)
			{
				auto s = m_debugged_process->read_wstring(remote_address);
				return s;
			}
			else
			{
				auto remote_ascii_name = m_debugged_process->read_string(remote_address);
				m_io_handler->write_formatted(L"Ascii string: %s\n", remote_ascii_name.c_str());
				return std::wstring(remote_ascii_name.begin(), remote_ascii_name.end());
			}
		}
	}

	// TODO: Throwing an exception would be more annoying to implement
	return std::wstring(L"");
}

DWORD Debugger::dispatch_debug_event(const DEBUG_EVENT& debug_event)
{
	if (debug_event.dwProcessId != m_debugged_process->get_process_id())
	{
		// TODO: Debugging multiple processes is not yet supported
		throw std::exception("Unexpected process ID!");
	}
	// TODO: Create a class that describes a debug event and allows reading the strings (Only thing shared between them is the thread id and process id).
	// TODO: When reading strings, cache them (std::optional where if has_value() == false, you read). Do the read as lazy.
	switch (debug_event.dwDebugEventCode)
	{
	case EXCEPTION_DEBUG_EVENT:
	{
		ExceptionDebugEvent event_data(debug_event, *this);
		return dispatch_exception(event_data);
	}
	case CREATE_THREAD_DEBUG_EVENT:
	{
		CreateThreadDebugEvent event_data(debug_event, *this);
		return dispatch_thread_creation(event_data);
	}
	case CREATE_PROCESS_DEBUG_EVENT:
	{
		CreateProcessDebugEvent event_data(debug_event, *this);
		return dispatch_process_creation(event_data);
	}
	case EXIT_THREAD_DEBUG_EVENT:
	{
		ExitThreadDebugEvent event_data(debug_event, *this);
		return dispatch_thread_termination(event_data);
	}
	case EXIT_PROCESS_DEBUG_EVENT:
	{
		ExitProcessDebugEvent event_data(debug_event, *this);
		return dispatch_process_termination(event_data);
	}
	case LOAD_DLL_DEBUG_EVENT:
	{
		LoadDllDebugEvent event_data(debug_event, *this);
		return dispatch_module_load(event_data);
	}
	case UNLOAD_DLL_DEBUG_EVENT:
	{
		UnloadDllDebugEvent event_data(debug_event, *this);
		return dispatch_module_unload(event_data);
	}
	case OUTPUT_DEBUG_STRING_EVENT:
	{
		DebugStringDebugEvent event_data(debug_event, *this);
		return dispatch_debug_string(event_data);
	}
	case RIP_EVENT:
	{
		RipDebugEvent event_data(debug_event, *this);
		return dispatch_rip(event_data);
	}
	default:
		throw std::exception("Unknown debug event");
	}
}

DWORD Debugger::dispatch_exception(ExceptionDebugEvent& debug_event)
{
	m_io_handler->write_formatted(L"Exception raised. Thread ID: %lu, First chance: %i, Exception Code: 0x%08lX, Exception Address: 0x%p",
		debug_event.get_thread_id(),
		debug_event.is_first_chance(),
		debug_event.get_exception_code(),
		debug_event.get_exception_address());
	return 0;
}

DWORD Debugger::dispatch_thread_creation(CreateThreadDebugEvent& debug_event)
{
	m_io_handler->write_formatted(L"Thread created. Thread ID: %lu, Start Address: 0x%p",
		debug_event.get_thread_id(),
		debug_event.get_start_address());
	// TODO: Cache the thread handle in the threads list
	return 0;
}

DWORD Debugger::dispatch_process_creation(CreateProcessDebugEvent& debug_event)
{
	m_io_handler->write_formatted(L"Process created. Process ID: %lu, process name: %ws",
		debug_event.get_process_id(),
		debug_event.get_image_path().c_str());
	// TODO: Cache the process handles
	return 0;
}

DWORD Debugger::dispatch_thread_termination(ExitThreadDebugEvent& debug_event)
{
	m_io_handler->write_formatted(L"Thread died. Thread ID: %lu, exit code: %lu",
		debug_event.get_thread_id(),
		debug_event.get_exit_code());
	return 0;
}

DWORD Debugger::dispatch_process_termination(ExitProcessDebugEvent& debug_event)
{
	m_io_handler->write_formatted(L"Process died. Process ID: %lu, exit code: %lu",
		debug_event.get_process_id(),
		debug_event.get_exit_code());
	throw std::exception("Debugged process died");
	return 0;
}

DWORD Debugger::dispatch_module_load(LoadDllDebugEvent& debug_event)
{
	m_io_handler->write_formatted(L"Module loaded. Module address: 0x%p, module name: %ws",
		debug_event.get_image_base(),
		debug_event.get_image_path().c_str());
	return 0;
}

DWORD Debugger::dispatch_module_unload(UnloadDllDebugEvent& debug_event)
{
	m_io_handler->write_formatted(L"Module unloaded. Module address: 0x%p",
		debug_event.get_image_base());
	return 0;
}

DWORD Debugger::dispatch_debug_string(DebugStringDebugEvent& debug_event)
{
	m_io_handler->write_formatted(L"Debug string output: %ws", 
		debug_event.get_debug_string().c_str());
	return 0;
}

DWORD Debugger::dispatch_rip(RipDebugEvent& debug_event)
{
	m_io_handler->write_formatted(L"RIP raised");
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

Debugger Debugger::attach_to_process(const DWORD pid, bool is_invasive, std::shared_ptr<ISimpleIO> io_handler)
{
	UNREFERENCED_PARAMETER(pid);
	UNREFERENCED_PARAMETER(is_invasive);
	UNREFERENCED_PARAMETER(io_handler);
	throw std::exception();
}

Debugger Debugger::attach_to_process(const std::wstring& process_name, bool is_invasive, std::shared_ptr<ISimpleIO> io_handler)
{
	DWORD pid = ProcUtils::process_name_to_pid(process_name);
	return Debugger::attach_to_process(pid, is_invasive, io_handler);
}

Debugger Debugger::debug_new_process(const std::wstring& exe_path, std::shared_ptr<ISimpleIO> io_handler)
{
	return Debugger(std::make_unique<Process>(exe_path), GetCurrentThreadId(), io_handler);
}
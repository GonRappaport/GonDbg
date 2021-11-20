#include <optional>

#include "Debugger.h"
#include "ProcUtils.h"
#include "Process.h"
#include "AttachedProcess.h"
#include "Command.h"

void Debugger::debug()
{
	m_io_handler->register_break_handler(this);
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
			while (continue_status == 1) // TODO: Define enum
			{
				continue_status = handle_user_command();
			}
			// TODO: For time travel debugging, hook this function with one that does nothing (I think) to simply "debug" the trace. You'll also hook the wait function
			if (!ContinueDebugEvent(debug_event.dwProcessId, debug_event.dwThreadId, continue_status))
			{
				throw std::exception("ContinueDebugEvent failed");
			}
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
	m_io_handler->write_formatted(L"Exception raised. Thread ID: %lu, First chance: %i, Exception Code: 0x%08lX, Exception Address: 0x%llX",
		debug_event.get_thread_id(),
		debug_event.is_first_chance(),
		debug_event.get_exception_code(),
		static_cast<DWORD64>(debug_event.get_exception_address()));
	if (debug_event.is_debug_break())
	{
		return 1;
	}
	return DBG_EXCEPTION_NOT_HANDLED;
}

DWORD Debugger::dispatch_thread_creation(CreateThreadDebugEvent& debug_event)
{
	m_io_handler->write_formatted(L"Thread created. Thread ID: %lu, Start Address: 0x%llX",
		debug_event.get_thread_id(),
		static_cast<DWORD64>(debug_event.get_start_address()));
	// TODO: Cache the thread handle in the threads list
	return DBG_EXCEPTION_NOT_HANDLED;
}

DWORD Debugger::dispatch_process_creation(CreateProcessDebugEvent& debug_event)
{
	m_io_handler->write_formatted(L"Process created. Process ID: %lu, process name: %ws, thread ID: %lu",
		debug_event.get_process_id(),
		debug_event.get_image_path().c_str(),
		debug_event.get_thread_id());

	m_symbol_finder.load_module(debug_event.get_file_handle(), debug_event.get_image_path(), debug_event.get_image_base());

	// TODO: Cache the process handles
	return DBG_EXCEPTION_NOT_HANDLED;
}

DWORD Debugger::dispatch_thread_termination(ExitThreadDebugEvent& debug_event)
{
	m_io_handler->write_formatted(L"Thread died. Thread ID: %lu, exit code: %lu",
		debug_event.get_thread_id(),
		debug_event.get_exit_code());
	return DBG_EXCEPTION_NOT_HANDLED;
}

DWORD Debugger::dispatch_process_termination(ExitProcessDebugEvent& debug_event)
{
	m_io_handler->write_formatted(L"Process died. Process ID: %lu, exit code: %lu",
		debug_event.get_process_id(),
		debug_event.get_exit_code());
	throw std::exception("Debugged process died");
	return DBG_EXCEPTION_NOT_HANDLED;
}

DWORD Debugger::dispatch_module_load(LoadDllDebugEvent& debug_event)
{
	m_io_handler->write_formatted(L"Module loaded. Module address: 0x%llX, module name: %ws",
		static_cast<DWORD64>(debug_event.get_image_base()),
		debug_event.get_image_path().c_str());

	m_symbol_finder.load_module(debug_event.get_file_handle(), debug_event.get_image_path(), debug_event.get_image_base());

	return DBG_EXCEPTION_NOT_HANDLED;
}

DWORD Debugger::dispatch_module_unload(UnloadDllDebugEvent& debug_event)
{
	m_io_handler->write_formatted(L"Module unloaded. Module address: 0x%llX",
		static_cast<DWORD64>(debug_event.get_image_base()));
	m_symbol_finder.unload_module(debug_event.get_image_base());
	return DBG_EXCEPTION_NOT_HANDLED;
}

DWORD Debugger::dispatch_debug_string(DebugStringDebugEvent& debug_event)
{
	if (debug_event.is_relevant())
	{
		m_io_handler->write_formatted(L"Debug string output: %ws",
			debug_event.get_debug_string().c_str());
	}
	return DBG_EXCEPTION_NOT_HANDLED;
}

DWORD Debugger::dispatch_rip(RipDebugEvent& debug_event)
{
	m_io_handler->write_formatted(L"RIP raised. Error: 0x%08lX, type: %lu",
		debug_event.get_error(),
		debug_event.get_type());
	return DBG_EXCEPTION_NOT_HANDLED;
}

DWORD Debugger::handle_user_command()
{
	Command command(m_io_handler->prompt(L"GonDBG>"));

	// TODO: Commands starting with '.' are debugger settings related functions
	// TODO: Commands starting with '!' are extension functions
	// TODO: The plugin name before ! is optional. If it appears, it's from an explicit plugin. Otherwise, search all plugins
	// TODO: Other commands are normal debug commands
	
	if (0 == command.get_command_name().find(L"g"))
	{
		return DBG_EXCEPTION_NOT_HANDLED;
	}
	else if (0 == command.get_command_name().find(L"db"))
	{
		const SIZE_T DEFAULT_READ_LENGTH = 0x40;
		RemotePointer address;
		SIZE_T length = DEFAULT_READ_LENGTH;

		int read_values = swscanf_s(command.get_command_params().c_str(), L"%Ix %Ix", &address, &length);
		
		switch (read_values)
		{
		case 1:
			__fallthrough;
		case 2:
			break;
		default:
			m_io_handler->write(L"Invalid syntax. Usage: db <address> [L<length>]");
			return 1;
		}

		// TODO: Add support for invalid addresses (An exception). Maybe wrap this entire function with try
		auto data = m_debugged_process->read_memory(address, length);
		m_io_handler->write(m_io_handler->format_bytes(data).c_str());
	}
	else if (0 == command.get_command_name().find(L"exit"))
	{
		throw std::exception("Exiting debugger");
	}
	else if (0 == command.get_command_name().find(L"x"))
	{
		RemotePointer address;

		int read_values = swscanf_s(command.get_command_params().c_str(), L"%Ix", &address);
		switch (read_values)
		{
		case 1:
			break;
		default:
			m_io_handler->write(L"Invalid syntax. Usage: x <address>");
			return 1;
		}

		m_io_handler->write(m_symbol_finder.get_symbol(address));
	}
	else if (0 == command.get_command_name().find(L"lm"))
	{
		auto loaded_modules = m_symbol_finder.get_loaded_modules();
		for (auto m : loaded_modules)
		{
			m_io_handler->write_formatted(L"0x%llX (0x%08lX): %s",
				static_cast<DWORD64>(m.get_image_base()),
				m.get_image_size(),
				m.get_image_name().c_str());
		}
	}
	else
	{
		m_io_handler->write(L"Unrecognized command");
	}

	return 1;
}

bool Debugger::handle_control(const DWORD ctrl_type)
{
	// TODO: See what to do if it fails. Is it even the same thread?
	if (ctrl_type == CTRL_C_EVENT)
	{
		DebugBreakProcess(m_debugged_process->get_process_handle());
		// TODO: Return true to avoid the process from terminating?
		return true;
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

Debugger Debugger::debug_new_process(const std::wstring& exe_path, std::shared_ptr<ISimpleIO> io_handler)
{
	return Debugger(std::make_shared<Process>(exe_path), GetCurrentThreadId(), io_handler);
}

Debugger Debugger::attach_to_process(const DWORD pid, std::shared_ptr<ISimpleIO> io_handler)
{
	return Debugger(std::make_shared<AttachedProcess>(pid), GetCurrentThreadId(), io_handler);
}

Debugger Debugger::attach_to_process(const std::wstring& process_name, std::shared_ptr<ISimpleIO> io_handler)
{
	DWORD pid = ProcUtils::process_name_to_pid(process_name);
	return Debugger::attach_to_process(pid, io_handler);
}

Debugger Debugger::attach_to_process_no_inject(const DWORD pid, std::shared_ptr<ISimpleIO> io_handler)
{
	UNREFERENCED_PARAMETER(pid);
	UNREFERENCED_PARAMETER(io_handler);
	throw std::exception();
}

Debugger Debugger::attach_to_process_no_inject(const std::wstring& process_name, std::shared_ptr<ISimpleIO> io_handler)
{
	DWORD pid = ProcUtils::process_name_to_pid(process_name);
	return Debugger::attach_to_process_no_inject(pid, io_handler);
}

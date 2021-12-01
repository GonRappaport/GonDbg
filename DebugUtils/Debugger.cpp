#include <optional>

#include "Debugger.h"
#include "ProcUtils.h"
#include "CreatedProcess.h"
#include "AttachedProcess.h"
#include "DebuggerCommands.h"
#include "Exceptions.h"

Debugger::Debugger(std::shared_ptr<IDebuggedProcess> debugged_process, const DWORD debugging_thread_id, std::shared_ptr<ISimpleIO> io_handler) :
	m_debugged_process(debugged_process),
	m_io_handler(io_handler),
	m_debugger_tid(debugging_thread_id),
	wait_for_debug_event(_cache_wait_for_debug_event()),
	m_symbol_finder(debugged_process),
	m_commands()/*,
	m_threads()*/
{
	for (const auto& command : DebuggerCommands::get_commands())
	{
		m_commands.register_command(command.first, command.second);
	}
}

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
				throw WinAPIException("WaitForDebugEvent(Ex) failed"); // TODO: Add GetLastError everywhere
			}
			DWORD continue_status = dispatch_debug_event(debug_event);
			while (continue_status == 1) // TODO: Define enum
			{
				continue_status = handle_user_command();
			}
			// TODO: For time travel debugging, hook this function with one that does nothing (I think) to simply "debug" the trace. You'll also hook the wait function
			if (!ContinueDebugEvent(debug_event.dwProcessId, debug_event.dwThreadId, continue_status))
			{
				throw WinAPIException("ContinueDebugEvent failed");
			}
		}
	}
	catch (const DebuggingEnd&)
	{}
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
		ExceptionDebugEvent event_data(debug_event, m_debugged_process);
		return dispatch_exception(event_data);
	}
	case CREATE_THREAD_DEBUG_EVENT:
	{
		CreateThreadDebugEvent event_data(debug_event, m_debugged_process);
		return dispatch_thread_creation(event_data);
	}
	case CREATE_PROCESS_DEBUG_EVENT:
	{
		CreateProcessDebugEvent event_data(debug_event, m_debugged_process);
		return dispatch_process_creation(event_data);
	}
	case EXIT_THREAD_DEBUG_EVENT:
	{
		ExitThreadDebugEvent event_data(debug_event, m_debugged_process);
		return dispatch_thread_termination(event_data);
	}
	case EXIT_PROCESS_DEBUG_EVENT:
	{
		ExitProcessDebugEvent event_data(debug_event, m_debugged_process);
		return dispatch_process_termination(event_data);
	}
	case LOAD_DLL_DEBUG_EVENT:
	{
		LoadDllDebugEvent event_data(debug_event, m_debugged_process);
		return dispatch_module_load(event_data);
	}
	case UNLOAD_DLL_DEBUG_EVENT:
	{
		UnloadDllDebugEvent event_data(debug_event, m_debugged_process);
		return dispatch_module_unload(event_data);
	}
	case OUTPUT_DEBUG_STRING_EVENT:
	{
		DebugStringDebugEvent event_data(debug_event, m_debugged_process);
		return dispatch_debug_string(event_data);
	}
	case RIP_EVENT:
	{
		RipDebugEvent event_data(debug_event, m_debugged_process);
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
	m_threads.emplace_back(debug_event);
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
	// TODO: Remove thread or mark as dead
	return DBG_EXCEPTION_NOT_HANDLED;
}

DWORD Debugger::dispatch_process_termination(ExitProcessDebugEvent& debug_event)
{
	m_io_handler->write_formatted(L"Process died. Process ID: %lu, exit code: %lu",
		debug_event.get_process_id(),
		debug_event.get_exit_code());
	// TODO: Make new exception type
	throw DebuggingEnd();
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
	std::wstring command_line = m_io_handler->prompt(L"GonDBG>");

	const auto command_params = CommandsRegistration::s_parse_command(command_line);

	try
	{
		const RegisteredCommand& command_data = m_commands.get_command(command_params.first);
		return command_data.m_implementation(command_params.second, *this);
	}
	catch (const CommandNotFoundException&)
	{
		m_io_handler->write(L"Unknown command");
		return 1;
	}
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
	return Debugger(std::make_shared<CreatedProcess>(exe_path), GetCurrentThreadId(), io_handler);
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

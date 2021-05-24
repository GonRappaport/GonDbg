#include <Windows.h>
#include <memory>
#include <string>
#include <vector>

#include "AutoCloseHandle.hpp"
#include "CtrlCHandler.h"

using PFN_WAITFORDEBUGEVENT = decltype(&WaitForDebugEvent);

#pragma once
class Debugger
{
public:
	Debugger(const HANDLE debugged_process_handle, const DWORD debugging_thread_id) :
		m_debuggee_process_handle(debugged_process_handle),
		m_break_handler(dispatch_user_breakins_handler, this),
		m_debugger_tid(debugging_thread_id),
		wait_for_debug_event(_cache_wait_for_debug_event())/*,
		m_threads(),
		m_modules()*/
	{};

	void debug();

private:
	// Initial data
	// Only the thread that initiated debugging can wait for debug events for a given process
	const AutoCloseHandle m_debuggee_process_handle;
	const CtrlCHandler m_break_handler;
	const DWORD m_debugger_tid;
	const PFN_WAITFORDEBUGEVENT wait_for_debug_event;

	// Runtime-gathered data
	//std::vector<CreatedThread> m_threads;
	//std::vector<LoadedModules> m_modules;

	static PFN_WAITFORDEBUGEVENT _cache_wait_for_debug_event();

	DWORD dispatch_debug_event(const DEBUG_EVENT& debug_event);
	DWORD dispatch_exception(const DEBUG_EVENT& debug_event);
	DWORD dispatch_thread_creation(const DEBUG_EVENT& debug_event);
	DWORD dispatch_process_creation(const DEBUG_EVENT& debug_event);
	DWORD dispatch_thread_termination(const DEBUG_EVENT& debug_event);
	DWORD dispatch_process_termination(const DEBUG_EVENT& debug_event);
	DWORD dispatch_module_load(const DEBUG_EVENT& debug_event);
	DWORD dispatch_module_unload(const DEBUG_EVENT& debug_event);
	DWORD dispatch_debug_string(const DEBUG_EVENT& debug_event);
	DWORD dispatch_rip(const DEBUG_EVENT& debug_event);

	BOOL dispatch_user_breakins(DWORD dwCtrlType);

	static BOOL WINAPI dispatch_user_breakins_handler(DWORD ctrl_type, void* context);
};

using DebuggerPtr = std::unique_ptr<Debugger>;

namespace DebuggerUtils
{
	DebuggerPtr attach(const DWORD pid, bool is_invasive);
	DebuggerPtr attach(const std::wstring&, bool is_invasive);

	DebuggerPtr create(const std::wstring& exe_path);
}
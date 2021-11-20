#pragma once

#include <Windows.h>
#include <memory>
#include <string>
#include <vector>

#include "AutoCloseHandle.hpp"
#include "IDebuggedProcess.h"
#include "ISimpleIO.h"
#include "DebugEvents.h"
#include "SymbolFinder.h"
#include "CommandsRegistration.h"

using PFN_WAITFORDEBUGEVENT = decltype(&WaitForDebugEvent);

class ExceptionDebugEvent;
class CreateThreadDebugEvent;
class CreateProcessDebugEvent;
class ExitThreadDebugEvent;
class ExitProcessDebugEvent;
class LoadDllDebugEvent;
class UnloadDllDebugEvent;
class DebugStringDebugEvent;
class RipDebugEvent;
class Debugger :
	public ICtrlHandler
{
public:
	Debugger(std::shared_ptr<IDebuggedProcess> debugged_process, const DWORD debugging_thread_id, std::shared_ptr<ISimpleIO> io_handler);

	Debugger(const Debugger&) = delete;
	Debugger& operator=(const Debugger&) = delete;

	Debugger(Debugger&& dbg) noexcept :
		m_debugged_process(dbg.m_debugged_process),
		m_io_handler(std::move(dbg.m_io_handler)),
		m_debugger_tid(dbg.m_debugger_tid),
		wait_for_debug_event(dbg.wait_for_debug_event),
		m_symbol_finder(std::move(dbg.m_symbol_finder)),
		m_commands(std::move(dbg.m_commands))/*,
		m_threads(std::move(dbg.m_threads))*/
	{}

	virtual ~Debugger() = default;

	void debug();

	static Debugger attach_to_process_no_inject(const DWORD pid, std::shared_ptr<ISimpleIO> io_handler);
	static Debugger attach_to_process_no_inject(const std::wstring& process_name, std::shared_ptr<ISimpleIO> io_handler);
	static Debugger attach_to_process(const DWORD pid, std::shared_ptr<ISimpleIO> io_handler);
	static Debugger attach_to_process(const std::wstring&, std::shared_ptr<ISimpleIO> io_handler);
	static Debugger debug_new_process(const std::wstring& exe_path, std::shared_ptr<ISimpleIO> io_handler);

	// TODO: This is exported so commands can use it. It's bad, get rid of it
	const std::shared_ptr<IDebuggedProcess> get_process() { return m_debugged_process; }
	const std::shared_ptr<ISimpleIO> get_io_handler() { return m_io_handler; }
	const SymbolFinder& get_symbol_finder() { return m_symbol_finder; }
	const CommandsRegistration& get_registered_commands() { return m_commands; }

private:
	// Initial data
	// Only the thread that initiated debugging can wait for debug events for a given process
	std::shared_ptr<IDebuggedProcess> m_debugged_process;
	std::shared_ptr<ISimpleIO> m_io_handler;
	const DWORD m_debugger_tid;
	const PFN_WAITFORDEBUGEVENT wait_for_debug_event;
	SymbolFinder m_symbol_finder;
	CommandsRegistration m_commands;

	// Runtime-gathered data
	//std::vector<CreatedThread> m_threads;

	static PFN_WAITFORDEBUGEVENT _cache_wait_for_debug_event();

	DWORD dispatch_debug_event(const DEBUG_EVENT& debug_event);
	DWORD dispatch_exception(ExceptionDebugEvent& debug_event);
	DWORD dispatch_thread_creation(CreateThreadDebugEvent& debug_event);
	DWORD dispatch_process_creation(CreateProcessDebugEvent& debug_event);
	DWORD dispatch_thread_termination(ExitThreadDebugEvent& debug_event);
	DWORD dispatch_process_termination(ExitProcessDebugEvent& debug_event);
	DWORD dispatch_module_load(LoadDllDebugEvent& debug_event);
	DWORD dispatch_module_unload(UnloadDllDebugEvent& debug_event);
	DWORD dispatch_debug_string(DebugStringDebugEvent& debug_event);
	DWORD dispatch_rip(RipDebugEvent& debug_event);

	DWORD handle_user_command();

	virtual bool handle_control(const DWORD ctrl_type);

	std::wstring _read_remote_string(RemotePointer base_address, bool is_unicode);

	friend class DebugEvent;
};
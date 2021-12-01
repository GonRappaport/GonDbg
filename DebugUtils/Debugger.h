#pragma once
// TODO: This wasn't compiled for x86 and has some warnings and errors

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
#include "CreatedThread.h"

using PFN_WAITFORDEBUGEVENT = decltype(&WaitForDebugEvent);

class ICallbackContext
{
public:
	ICallbackContext() = default;
	virtual ~ICallbackContext() = default;
};
// Returns true if it wishes to stay in the list, false to be removed.
using ExceptionCallback = std::function<bool(const ExceptionDebugEvent&, std::shared_ptr<ICallbackContext>)>;
using ThreadCreationCallback = std::function<bool(const CreateThreadDebugEvent&, std::shared_ptr<ICallbackContext>)>;

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
		m_commands(std::move(dbg.m_commands)),
		m_threads(std::move(dbg.m_threads)),
		m_exception_callbacks(std::move(dbg.m_exception_callbacks)),
		m_thread_creation_callbacks(std::move(dbg.m_thread_creation_callbacks)),
		m_current_thread_id(std::exchange(dbg.m_current_thread_id, 0))
	{}

	virtual ~Debugger() = default;

	void debug();

	static Debugger attach_to_process_no_inject(const DWORD pid, std::shared_ptr<ISimpleIO> io_handler);
	static Debugger attach_to_process_no_inject(const std::wstring& process_name, std::shared_ptr<ISimpleIO> io_handler);
	static Debugger attach_to_process(const DWORD pid, std::shared_ptr<ISimpleIO> io_handler);
	static Debugger attach_to_process(const std::wstring&, std::shared_ptr<ISimpleIO> io_handler);
	static Debugger debug_new_process(const std::wstring& exe_path, std::shared_ptr<ISimpleIO> io_handler);

	const std::shared_ptr<IDebuggedProcess> get_process() const { return m_debugged_process; }
	const std::shared_ptr<ISimpleIO> get_io_handler() const { return m_io_handler; }
	const SymbolFinder& get_symbol_finder() const { return m_symbol_finder; }
	const CommandsRegistration& get_registered_commands() const { return m_commands; }
	std::list<CreatedThread>& get_threads() { return m_threads; } // Not returning const as commands may alter thread status
	DWORD get_current_thread_id() const { return m_current_thread_id; }

	void register_exception_callback(ExceptionCallback callback, std::shared_ptr<ICallbackContext> context) { m_exception_callbacks.emplace_back(std::make_pair(callback, context)); }
	void register_thread_creation_callback(ThreadCreationCallback callback, std::shared_ptr<ICallbackContext> context) { m_thread_creation_callbacks.emplace_back(std::make_pair(callback, context)); }

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
	std::list<CreatedThread> m_threads;
	std::list<std::pair<ExceptionCallback, std::shared_ptr<ICallbackContext>>> m_exception_callbacks;
	std::list<std::pair<ThreadCreationCallback, std::shared_ptr<ICallbackContext>>> m_thread_creation_callbacks;
	DWORD m_current_thread_id;

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
};
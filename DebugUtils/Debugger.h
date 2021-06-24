#pragma once

#include <Windows.h>
#include <memory>
#include <string>
#include <vector>

#include "AutoCloseHandle.hpp"
#include "CtrlCHandler.h"
#include "IDebuggedProcess.h"
#include "ISimpleIO.h"
#include "DebugEvents.h"

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
	Debugger(std::unique_ptr<IDebuggedProcess>&& debugged_process, const DWORD debugging_thread_id, std::shared_ptr<ISimpleIO> io_handler) :
		m_debugged_process(std::move(debugged_process)),
		m_io_handler(io_handler),
		m_break_handler(this),
		m_debugger_tid(debugging_thread_id),
		wait_for_debug_event(_cache_wait_for_debug_event())/*,
		m_threads(),
		m_modules()*/
	{};

	Debugger(const Debugger&) = delete;
	Debugger& operator=(const Debugger&) = delete;

	Debugger(Debugger&& dbg) noexcept :
		m_debugged_process(std::move(dbg.m_debugged_process)),
		m_io_handler(std::move(dbg.m_io_handler)),
		m_break_handler(std::move(dbg.m_break_handler)),
		m_debugger_tid(dbg.m_debugger_tid),
		wait_for_debug_event(dbg.wait_for_debug_event)/*,
		m_threads(std::move(dbg.m_threads)),
		m_modules(std::move(dbg.m_modules))*/
	{}

	void debug();

	static Debugger attach_to_process(const DWORD pid, bool is_invasive, std::shared_ptr<ISimpleIO> io_handler);
	static Debugger attach_to_process(const std::wstring&, bool is_invasive, std::shared_ptr<ISimpleIO> io_handler);
	static Debugger debug_new_process(const std::wstring& exe_path, std::shared_ptr<ISimpleIO> io_handler);

private:
	// Initial data
	// Only the thread that initiated debugging can wait for debug events for a given process
	std::unique_ptr<IDebuggedProcess> m_debugged_process;
	std::shared_ptr<ISimpleIO> m_io_handler;
	CtrlCHandler m_break_handler;
	const DWORD m_debugger_tid;
	const PFN_WAITFORDEBUGEVENT wait_for_debug_event;

	// Runtime-gathered data
	//std::vector<CreatedThread> m_threads;
	//std::vector<LoadedModules> m_modules;

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

	virtual bool handle_control(const DWORD ctrl_type);

	std::wstring _read_remote_string(PVOID base_address, bool is_unicode);

	friend class DebugEvent;
};
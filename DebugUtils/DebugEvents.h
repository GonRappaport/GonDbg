#pragma once

#include <Windows.h>
#include <string>
#include <optional>
#include <memory>

#include "IDebuggedProcess.h"
#include "AutoCloseHandle.hpp"

class DebugEvent
{
public:
	// TODO: Instead of keeping Debugger, keep IDebuggedProcessPtr. It both solves your circular dependency (Maybe, see that you include headers correctly. As of now, seems right) and makes more sense
	// TODO: It will also be better since once you support multiple processes, you could relate each debug event to a specific process in Debugger, instead of doing that here
	DebugEvent(const DEBUG_EVENT& debug_event, std::shared_ptr<IDebuggedProcess>& process) :
		m_thread_id(debug_event.dwThreadId),
		m_process_id(debug_event.dwProcessId),
		m_process(process)
	{}
	virtual ~DebugEvent() = default;
	DebugEvent(const DebugEvent&) = delete;
	DebugEvent& operator=(const DebugEvent&) = delete;
	DebugEvent(DebugEvent&&) = delete;
	DebugEvent& operator=(DebugEvent&&) = delete;

	DWORD get_thread_id() const { return m_thread_id; }
	DWORD get_process_id() const { return m_process_id; }
	std::shared_ptr <IDebuggedProcess> get_process() const { return m_process; }

protected:
	const DWORD m_thread_id;
	const DWORD m_process_id;
	std::shared_ptr<IDebuggedProcess> m_process;

	std::wstring _read_remote_string(const RemotePointer base_address, const bool is_unicode);
	std::wstring _deref_read_remote_string(const RemotePointer base_address, const bool is_unicode);
};

class ExceptionDebugEvent final :
	public DebugEvent
{
public:
	ExceptionDebugEvent(const DEBUG_EVENT& debug_event, std::shared_ptr<IDebuggedProcess>& process):
		DebugEvent(debug_event, process),
		// TODO: Check if it's EXCEPTION_RECORD32 or 64
		m_exception_record(debug_event.u.Exception.ExceptionRecord),
		m_first_chance(debug_event.u.Exception.dwFirstChance)
	{}
	virtual ~ExceptionDebugEvent() = default;

	DWORD get_exception_code() const { return m_exception_record.ExceptionCode; }
	RemotePointer get_exception_address() const { return RemotePointer(m_exception_record.ExceptionAddress); }
	bool is_first_chance() const { return m_first_chance; }
	bool is_debug_break() const { return m_exception_record.ExceptionCode == EXCEPTION_BREAKPOINT; }
	bool is_single_step() const { return m_exception_record.ExceptionCode == EXCEPTION_SINGLE_STEP; }
	bool is_trap() const;

private:
	const EXCEPTION_RECORD m_exception_record;
	const bool m_first_chance;
};

class CreateThreadDebugEvent final :
	public DebugEvent
{
public:
	CreateThreadDebugEvent(const DEBUG_EVENT& debug_event, std::shared_ptr<IDebuggedProcess>& process) :
		DebugEvent(debug_event, process),
		m_handle(debug_event.u.CreateThread.hThread),
		m_local_base(debug_event.u.CreateThread.lpThreadLocalBase),
		m_start_address(debug_event.u.CreateThread.lpStartAddress)
	{}
	virtual ~CreateThreadDebugEvent() = default;

	HANDLE get_handle() const { return m_handle; }
	RemotePointer get_local_base() const { return m_local_base; }
	RemotePointer get_start_address() const { return m_start_address; }

private:
	// NOTE: This shouldn't be closed as it will be closed by the system after the EXIT_THREAD debug event
	const HANDLE m_handle;
	const RemotePointer m_local_base;
	const RemotePointer m_start_address;
};

class CreateProcessDebugEvent final :
	public DebugEvent
{
public:
	CreateProcessDebugEvent(const DEBUG_EVENT& debug_event, std::shared_ptr<IDebuggedProcess>& process) :
		DebugEvent(debug_event, process),
		m_file_handle(debug_event.u.CreateProcessInfo.hFile),
		m_process_handle(debug_event.u.CreateProcessInfo.hProcess),
		m_thread_handle(debug_event.u.CreateProcessInfo.hThread),
		m_image_base(debug_event.u.CreateProcessInfo.lpBaseOfImage),
		m_start_address(debug_event.u.CreateProcessInfo.lpStartAddress),
		m_image_path_data(std::make_pair(debug_event.u.CreateProcessInfo.lpImageName, debug_event.u.CreateProcessInfo.fUnicode))
	{}
	virtual ~CreateProcessDebugEvent() = default;

	const std::wstring& get_image_path() 
	{
		if (!m_image_path.has_value())
		{
			m_image_path = _deref_read_remote_string(m_image_path_data.first, m_image_path_data.second);
		}
		return m_image_path.value();
	}

	HANDLE get_file_handle() const { return m_file_handle.get_value(); }
	// TODO: Is this legit?
	AutoCloseHandle detach_file_handle() { return std::move(m_file_handle); }

	const RemotePointer get_image_base() const { return m_image_base; }
	HANDLE get_thread_handle() const { return m_thread_handle; }

private:
	AutoCloseHandle m_file_handle;
	// NOTE: These shouldn't be closed as they will be closed by the system after the EXIT_PROCESS debug event
	const HANDLE m_process_handle;
	const HANDLE m_thread_handle;
	const RemotePointer m_image_base;
	const RemotePointer m_start_address;
	const std::pair<RemotePointer, bool> m_image_path_data;
	std::optional<std::wstring> m_image_path;
};

class ExitThreadDebugEvent final :
	public DebugEvent
{
public:
	ExitThreadDebugEvent(const DEBUG_EVENT& debug_event, std::shared_ptr<IDebuggedProcess>& process) :
		DebugEvent(debug_event, process),
		m_exit_code(debug_event.u.ExitThread.dwExitCode)
	{}
	virtual ~ExitThreadDebugEvent() = default;

	DWORD get_exit_code() const { return m_exit_code; }

private:
	const DWORD m_exit_code;
};

class ExitProcessDebugEvent final :
	public DebugEvent
{
public:
	ExitProcessDebugEvent(const DEBUG_EVENT& debug_event, std::shared_ptr<IDebuggedProcess>& process) :
		DebugEvent(debug_event, process),
		m_exit_code(debug_event.u.ExitProcess.dwExitCode)
	{}
	virtual ~ExitProcessDebugEvent() = default;

	DWORD get_exit_code() const { return m_exit_code; }

private:
	const DWORD m_exit_code;
};

class LoadDllDebugEvent final :
	public DebugEvent
{
public:
	LoadDllDebugEvent(const DEBUG_EVENT& debug_event, std::shared_ptr<IDebuggedProcess>& process) :
		DebugEvent(debug_event, process),
		m_file_handle(debug_event.u.LoadDll.hFile),
		m_image_base(debug_event.u.LoadDll.lpBaseOfDll),
		m_debug_info_offset(debug_event.u.LoadDll.dwDebugInfoFileOffset),
		m_debug_info_size(debug_event.u.LoadDll.nDebugInfoSize),
		m_image_path_data(std::make_pair(debug_event.u.LoadDll.lpImageName, debug_event.u.LoadDll.fUnicode))
	{}
	virtual ~LoadDllDebugEvent() = default;

	RemotePointer get_image_base() const { return m_image_base; }

	const std::wstring& get_image_path()
	{
		if (!m_image_path.has_value())
		{
			m_image_path = _deref_read_remote_string(m_image_path_data.first, m_image_path_data.second);
		}
		return m_image_path.value();
	}

	HANDLE get_file_handle() const { return m_file_handle.get_value(); }
	// TODO: Is this legit?
	AutoCloseHandle detach_file_handle() { return std::move(m_file_handle); }

private:
	AutoCloseHandle m_file_handle;
	const RemotePointer m_image_base;
	const DWORD m_debug_info_offset;
	const DWORD m_debug_info_size;
	const std::pair<RemotePointer, bool> m_image_path_data;
	std::optional<std::wstring> m_image_path;
};

class UnloadDllDebugEvent final :
	public DebugEvent
{
public:
	UnloadDllDebugEvent(const DEBUG_EVENT& debug_event, std::shared_ptr<IDebuggedProcess>& process) :
		DebugEvent(debug_event, process),
		m_image_base(debug_event.u.UnloadDll.lpBaseOfDll)
	{}
	virtual ~UnloadDllDebugEvent() = default;

	RemotePointer get_image_base() const { return m_image_base; }

private:
	const RemotePointer m_image_base;
};

class DebugStringDebugEvent final :
	public DebugEvent
{
public:
	DebugStringDebugEvent(const DEBUG_EVENT& debug_event, std::shared_ptr<IDebuggedProcess>& process) :
		DebugEvent(debug_event, process),
		m_string_data(std::make_pair(debug_event.u.DebugString.lpDebugStringData, debug_event.u.DebugString.fUnicode)),
		m_string_length(debug_event.u.DebugString.nDebugStringLength)
	{}
	virtual ~DebugStringDebugEvent() = default;

	const std::wstring& get_debug_string()
	{
		if (!m_string.has_value())
		{
			m_string = _read_remote_string(m_string_data.first, m_string_data.second);
		}
		return m_string.value();
	}

	bool is_relevant() const;

private:
	const std::pair<RemotePointer, bool> m_string_data;
	// TODO: DebugStringLength is ignored, as it may also truncate my output. Add an upper cap with it to read_string
	const WORD m_string_length;
	std::optional<std::wstring> m_string;
};

class RipDebugEvent final :
	public DebugEvent
{
public:
	RipDebugEvent(const DEBUG_EVENT& debug_event, std::shared_ptr<IDebuggedProcess>& process) :
		DebugEvent(debug_event, process),
		m_error(debug_event.u.RipInfo.dwError),
		m_type(debug_event.u.RipInfo.dwType)
	{}
	virtual ~RipDebugEvent() = default;

	DWORD get_error() const { return m_error; }
	DWORD get_type() const { return m_type; }

private:
	const DWORD m_error;
	const DWORD m_type;
};

#pragma once

#include "DebugEvents.h"
#include "Exceptions.h"

class CreatedThread
{
public:
	CreatedThread(const CreateThreadDebugEvent& debug_event):
		m_thread_handle(debug_event.get_handle()), // Note: this is a partial  privilege handle
		m_thread_id(debug_event.get_thread_id())
	{}
	CreatedThread(const CreateProcessDebugEvent& debug_event) :
		m_thread_handle(debug_event.get_thread_handle()), // Note: this is a partial  privilege handle
		m_thread_id(debug_event.get_thread_id())
	{}
	~CreatedThread() = default;

	CreatedThread(const CreatedThread&) = delete;
	CreatedThread& operator=(const CreatedThread&) = delete;
	CreatedThread(CreatedThread&&) noexcept = delete;
	CreatedThread&& operator=(CreatedThread&&) noexcept = delete;

	DWORD get_thread_id() const { return m_thread_id; }

	// TODO: Consider this returning some "cookie" class to un-suspend a thread even on exceptions
	void suspend();
	void resume();
	bool is_suspended();
	void set_trap_flag();
	void clear_trap_flag();
	bool is_trap_flag_set();

private:
	AutoCloseHandle m_thread_handle;
	const DWORD m_thread_id;
};

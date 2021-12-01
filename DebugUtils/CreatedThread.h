#pragma once

#include "DebugEvents.h"

class CreatedThread
{
public:
	CreatedThread(const CreateThreadDebugEvent& debug_event):
		m_thread_handle(debug_event.get_handle()), // Note: this is a partial  privilege handle
		m_thread_id(debug_event.get_thread_id())
	{}
	~CreatedThread() = default;

	DWORD get_thread_id() const { return m_thread_id; }

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


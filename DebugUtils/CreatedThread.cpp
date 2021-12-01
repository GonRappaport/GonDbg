#include "CreatedThread.h"

const DWORD TRAP_FLAG = 0x100;

void CreatedThread::suspend()
{
	// TODO: Add support for Wow64SuspendThread
	if (-1 == SuspendThread(m_thread_handle.get_value()))
	{
		throw WinAPIException("SuspendThread failed");
	}
}

void CreatedThread::resume()
{
	if (-1 == ResumeThread(m_thread_handle.get_value()))
	{
		throw WinAPIException("ResumeThread failed");
	}
}

void CreatedThread::set_trap_flag()
{
	// TODO: Support WOW64_CONTEXT
	CONTEXT thread_context = { 0 };
	thread_context.ContextFlags = CONTEXT_CONTROL;
	if (!GetThreadContext(m_thread_handle.get_value(), &thread_context))
	{
		throw WinAPIException("GetThreadContext failed");
	}
	thread_context.EFlags |= TRAP_FLAG;
	if (!SetThreadContext(m_thread_handle.get_value(), &thread_context))
	{
		throw WinAPIException("SetThreadContext failed");
	}
}

void CreatedThread::clear_trap_flag()
{
	// TODO: Support WOW64_CONTEXT
	CONTEXT thread_context = { 0 };
	thread_context.ContextFlags = CONTEXT_CONTROL;
	if (!GetThreadContext(m_thread_handle.get_value(), &thread_context))
	{
		throw WinAPIException("GetThreadContext failed");
	}
	thread_context.EFlags &= ~TRAP_FLAG;
	if (!SetThreadContext(m_thread_handle.get_value(), &thread_context))
	{
		throw WinAPIException("SetThreadContext failed");
	}
}

bool CreatedThread::is_trap_flag_set()
{
	// TODO: Support WOW64_CONTEXT
	CONTEXT thread_context = { 0 };
	thread_context.ContextFlags = CONTEXT_CONTROL;
	if (!GetThreadContext(m_thread_handle.get_value(), &thread_context))
	{
		throw WinAPIException("GetThreadContext failed");
	}
	return thread_context.EFlags & TRAP_FLAG;
}

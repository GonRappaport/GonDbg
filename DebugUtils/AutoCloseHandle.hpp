#pragma once

#include <Windows.h>
#include <exception>

template <HANDLE invalid_value>
class AutoCloseHandleImpl
{
public:
	explicit AutoCloseHandleImpl(const HANDLE handle) :
		m_handle(_validate_handle(handle))
	{}

	AutoCloseHandleImpl(const AutoCloseHandleImpl&) = delete;
	AutoCloseHandleImpl& operator=(const AutoCloseHandleImpl&) = delete;

	AutoCloseHandleImpl(AutoCloseHandleImpl&& ach) noexcept:
		m_handle(_validate_handle(ach.m_handle)) // TODO: Declared as noexcept but _validate_handle may throw... Can a class be moved twice?
	{
		ach.m_handle = invalid_value;
	}

	~AutoCloseHandleImpl()
	{
		try
		{
			if (m_handle != invalid_value)
			{
				CloseHandle(m_handle);
			}
		}
		catch (...)
		{

		}
	}

	HANDLE get_value() const { return m_handle; }


private:
	HANDLE _validate_handle(const HANDLE handle)
	{
		if (handle == invalid_value)
		{
			throw std::exception("Bad handle value");
		}
		return handle;
	}
	HANDLE m_handle;
};

using AutoCloseHandle = AutoCloseHandleImpl<nullptr>;
using AutoCloseFileHandle = AutoCloseHandleImpl<INVALID_HANDLE_VALUE>;

#pragma once

#include <Windows.h>
#include <exception>
#include <functional>

template <HANDLE invalid_value>
class AutoCloseHandleImpl
{
public:
	explicit AutoCloseHandleImpl(const HANDLE handle, std::function<void(HANDLE)> closing_function=CloseHandle) :
		m_handle(_validate_handle(handle)),
		m_closer(closing_function)
	{}

	AutoCloseHandleImpl(const AutoCloseHandleImpl&) = delete;
	AutoCloseHandleImpl& operator=(const AutoCloseHandleImpl&) = delete;

	AutoCloseHandleImpl(AutoCloseHandleImpl&& ach) noexcept:
		m_handle(_validate_handle(ach.m_handle)) // TODO: Declared as noexcept but _validate_handle may throw... Can a class be moved twice?
	{
		ach.m_handle = invalid_value; // TODO: std::exchange?
	}

	~AutoCloseHandleImpl()
	{
		try
		{
			if (m_handle != invalid_value)
			{
				m_closer(m_handle);
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
	const std::function<void(HANDLE)> m_closer;
};

using AutoCloseHandle = AutoCloseHandleImpl<nullptr>;
using AutoCloseFileHandle = AutoCloseHandleImpl<INVALID_HANDLE_VALUE>;

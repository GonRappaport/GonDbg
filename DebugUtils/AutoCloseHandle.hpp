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

	~AutoCloseHandleImpl()
	{
		try
		{
			CloseHandle(m_handle);
		}
		catch (...)
		{

		}
	}

	const HANDLE get_value() const { return m_handle; }


private:
	const HANDLE _validate_handle(const HANDLE handle)
	{
		if (handle == invalid_value)
		{
			throw std::exception("Bad handle value");
		}
		return handle;
	}
	const HANDLE m_handle;
};

using AutoCloseHandle = AutoCloseHandleImpl<nullptr>;
using AutoCloseFileHandle = AutoCloseHandleImpl<INVALID_HANDLE_VALUE>;

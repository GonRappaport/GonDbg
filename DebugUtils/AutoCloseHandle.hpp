#pragma once

#include <Windows.h>
#include <exception>
#include <functional>

// TODO: Improve this template to be able to capture the type of the function
template <typename closeable_type, closeable_type invalid_value, BOOL __stdcall closing_function(closeable_type)>
class AutoCloseHandleImpl
{
public:
	AutoCloseHandleImpl(const closeable_type handle) :
		m_handle(_validate_handle(handle))
	{}

	// TODO: Consider making that ref-counting to allow for copy constructions. But maybe a different class so it won't be implicit
	// TODO: Since that doesn't work only with HANDLE now, maybe overide this function for HANDLE specifically and use refcounting. Or pass a refcount-increasing function
	AutoCloseHandleImpl(const AutoCloseHandleImpl&) = delete;
	AutoCloseHandleImpl& operator=(const AutoCloseHandleImpl&) = delete;

	AutoCloseHandleImpl(AutoCloseHandleImpl&& ach) noexcept:
		m_handle(std::exchange(ach.m_handle, invalid_value)) // Not calling _validate_handle as it may throw and the handle was already validated anyway
	{}

	~AutoCloseHandleImpl()
	{
		try
		{
			if (m_handle != invalid_value)
			{
				closing_function(m_handle);
			}
		}
		catch (...)
		{

		}
	}

	closeable_type get_value() const { return m_handle; }

private:
	closeable_type _validate_handle(const closeable_type handle)
	{
		if (handle == invalid_value)
		{
			throw std::exception("Bad handle value");
		}
		return handle;
	}
	closeable_type m_handle;
};

using AutoCloseHandle = AutoCloseHandleImpl<HANDLE, nullptr, CloseHandle>;
using AutoCloseFileHandle = AutoCloseHandleImpl<HANDLE, INVALID_HANDLE_VALUE, CloseHandle>;
using AutoFreeLibrary = AutoCloseHandleImpl<HMODULE, nullptr, FreeLibrary>;
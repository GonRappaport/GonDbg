#pragma once

#include <Windows.h>
#include <exception>
#include <functional>

// TODO: Maybe add the closer function to the template instead of the constructor
template <typename closeable_type, closeable_type invalid_value>
class AutoCloseHandleImpl
{
public:
	AutoCloseHandleImpl(const closeable_type handle, std::function<void(closeable_type)> closing_function) :
		m_handle(_validate_handle(handle)),
		m_closer(closing_function)
	{}

	// TODO: I don't think it really specializes, it's just weird and wrong probably. Probably added bugs here
	// Specilization for HANDLE
	explicit AutoCloseHandleImpl(const HANDLE handle):
		m_handle(_validate_handle(handle)),
		m_closer(CloseHandle)
	{}

	// Specilization for HMODULE
	explicit AutoCloseHandleImpl(const HMODULE handle) :
		m_handle(_validate_handle(handle)),
		m_closer(FreeLibrary)
	{}

	// TODO: Consider making that ref-counting to allow for copy constructions. But maybe a different class so it won't be implicit
	// TODO: Since that doesn't work only with HANDLE now, maybe overide this function for HANDLE specifically and use refcounting.
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
	const std::function<void(closeable_type)> m_closer;
};

using AutoCloseHandle = AutoCloseHandleImpl<HANDLE, nullptr>;
using AutoCloseFileHandle = AutoCloseHandleImpl<HANDLE, INVALID_HANDLE_VALUE>;
using AutoFreeLibrary = AutoCloseHandleImpl<HMODULE, nullptr>;
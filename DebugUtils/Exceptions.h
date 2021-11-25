#pragma once

#include <Windows.h>
#include <exception>

class WinAPIException :
	public std::exception
{
public:
	WinAPIException() noexcept:
		std::exception(),
		m_last_error(GetLastError())
	{}

	explicit WinAPIException(const DWORD error) noexcept:
		// TODO: FormatMessageA?
		std::exception(),
		m_last_error(error)
	{}

	explicit WinAPIException(const char* message) noexcept:
		std::exception(message),
		m_last_error(GetLastError())
	{}

	WinAPIException(const char* message, const DWORD error) noexcept:
		std::exception(message),
		m_last_error(error)
	{}

	WinAPIException(const WinAPIException& other) noexcept:
		std::exception(),
		m_last_error(other.m_last_error)
	{}

	virtual ~WinAPIException() = default;

	DWORD get_error() const noexcept { return m_last_error; }

private:
	const DWORD m_last_error;
};
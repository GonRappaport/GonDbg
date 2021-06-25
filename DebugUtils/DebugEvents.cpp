#include "DebugEvents.h"

std::wstring DebugEvent::_read_remote_string(const RemotePointer base_address, const bool is_unicode)
{
	if (!base_address.is_null())
	{
		if (is_unicode)
		{
			return this->m_debugger.m_debugged_process->read_wstring(base_address);
		}
		else
		{
			auto remote_ascii_name = this->m_debugger.m_debugged_process->read_string(base_address);
			return std::wstring(remote_ascii_name.begin(), remote_ascii_name.end());
		}
	}

	// TODO: Throwing an exception would be more annoying to implement
	return std::wstring(L"");
}

std::wstring DebugEvent::_deref_read_remote_string(const RemotePointer base_address, const bool is_unicode)
{
	if (!base_address.is_null())
	{
		auto remote_address = this->m_debugger.m_debugged_process->read_pointer(base_address);
		return _read_remote_string(remote_address, is_unicode);
	}

	// TODO: Throwing an exception would be more annoying to implement
	return std::wstring(L"");
}

bool DebugStringDebugEvent::is_relevant() const
{
	// TODO: Improve that according to what windbg does
	return m_string_data.second;
}
#pragma once

#include "IDebuggedProcess.h"
#include "AutoCloseHandle.hpp"

class RunningProcess :
    public IDebuggedProcess
{
public:
	explicit RunningProcess(AutoCloseHandle handle) :
		m_handle(std::move(handle)),
		// Note: Accesing m_handle and not handle since handle was moved to m_handle
		m_process_id(GetProcessId(m_handle.get_value())),
		m_64_bit(_s_is_64_bit(m_handle.get_value()))
	{}

	RunningProcess(AutoCloseHandle handle, const DWORD pid) :
		m_handle(std::move(handle)),
		m_process_id(pid),
		// Note: Accesing m_handle and not handle since handle was moved to m_handle
		m_64_bit(_s_is_64_bit(m_handle.get_value()))
	{}

	RunningProcess(const RunningProcess&) = delete;
	RunningProcess& operator=(const RunningProcess&) = delete;

	RunningProcess(RunningProcess&& p) noexcept :
		m_handle(std::move(p.m_handle)),
		m_process_id(p.m_process_id),
		m_64_bit(p.m_64_bit)
	{}

	virtual ~RunningProcess() = default;

	virtual std::vector<BYTE> read_memory(RemotePointer base_address, SIZE_T size);
	virtual void write_memory(RemotePointer base_address, std::vector<BYTE> data);

	virtual HANDLE get_process_handle() const;
	virtual DWORD get_process_id() const;
	virtual bool is_64_bit() const;

protected:
	AutoCloseHandle m_handle;
	const DWORD m_process_id;
	const bool m_64_bit;

	static bool _s_is_64_bit(const HANDLE);
};


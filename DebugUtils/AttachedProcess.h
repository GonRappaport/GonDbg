#pragma once

#include "IDebuggedProcess.h"
#include "AutoCloseHandle.hpp"

// TODO:DebugActiveProcessStop/DebugSetProcessKillOnExit
class AttachedProcess :
	public IDebuggedProcess
{
public:
	explicit AttachedProcess(const DWORD process_id) :
		m_handle(_s_attach_process(process_id)),
		m_process_id(process_id),
		m_64_bit(_s_is_64_bit(m_handle.get_value()))
	{}

	AttachedProcess(const AttachedProcess&) = delete;
	AttachedProcess& operator=(const AttachedProcess&) = delete;

	AttachedProcess(AttachedProcess&& p) noexcept :
		m_handle(std::move(p.m_handle)),
		m_process_id(p.m_process_id),
		m_64_bit(p.m_64_bit)
	{}

	virtual ~AttachedProcess() = default;

	virtual HANDLE get_process_handle() const;
	virtual DWORD get_process_id() const;
	virtual bool is_64_bit() const;
	virtual std::vector<BYTE> read_memory(RemotePointer base_address, SIZE_T size);
	virtual void write_memory(RemotePointer base_address, std::vector<BYTE> data);

private:
	AutoCloseHandle m_handle;
	const DWORD m_process_id;
	const bool m_64_bit;

	static AutoCloseHandle _s_attach_process(const DWORD process_id);
	static bool _s_is_64_bit(const HANDLE);
};


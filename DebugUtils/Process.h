#pragma once

#include "IDebuggedProcess.h"
#include "AutoCloseHandle.hpp"

class Process :
    public IDebuggedProcess
{
public:
    explicit Process(const std::wstring& exe_path) :
		m_handle(_s_create_process(exe_path, L"")),
		m_process_id(GetProcessId(m_handle.get_value())),
		m_64_bit(_s_is_64_bit(m_handle.get_value()))
	{}
	Process(const std::wstring& exe_path, const std::wstring& command_line) :
		m_handle(_s_create_process(exe_path, command_line)),
		m_process_id(GetProcessId(m_handle.get_value())),
		m_64_bit(_s_is_64_bit(m_handle.get_value()))
	{}

	Process(const Process&) = delete;
	Process& operator=(const Process&) = delete;

	Process(Process&& p) noexcept :
		m_handle(std::move(p.m_handle)),
		m_process_id(p.m_process_id),
		m_64_bit(p.m_64_bit)
	{}

	virtual ~Process() = default;

	virtual HANDLE get_process_handle() const;
	virtual DWORD get_process_id() const;
	virtual bool is_64_bit() const;
	virtual std::vector<BYTE> read_memory(PVOID base_address, SIZE_T size);
	virtual void write_memory(PVOID base_address, std::vector<BYTE> data);

private:
    AutoCloseHandle m_handle;
	const DWORD m_process_id;
	const bool m_64_bit;

	static AutoCloseHandle _s_create_process(const std::wstring& exe_path, std::wstring command_line);
	static bool _s_is_64_bit(const HANDLE);
};


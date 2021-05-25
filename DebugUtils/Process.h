#pragma once

#include <string>

#include "IDebuggedProcess.h"
#include "AutoCloseHandle.hpp"

class Process :
    public IDebuggedProcess
{
public:
    explicit Process(const std::wstring& exe_path) :
		m_handle(_s_create_process(exe_path, L""))
	{}
	Process(const std::wstring& exe_path, const std::wstring& command_line) :
		m_handle(_s_create_process(exe_path, command_line))
	{}

	Process(const Process&) = delete;
	Process& operator=(const Process&) = delete;

	Process(Process&& p) noexcept :
		m_handle(std::move(p.m_handle))
	{}

	virtual ~Process() = default;

	virtual HANDLE get_process_handle();
	virtual std::vector<BYTE> read_memory(PVOID base_address, SIZE_T size);
	virtual void write_memory(PVOID base_address, std::vector<BYTE> data);

private:
    AutoCloseHandle m_handle;

	static AutoCloseHandle&& _s_create_process(const std::wstring& exe_path, std::wstring command_line);
};


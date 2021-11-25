#pragma once

#include "RunningProcess.h"
#include "AutoCloseHandle.hpp"

class CreatedProcess :
    public RunningProcess
{
public:
    explicit CreatedProcess(const std::wstring& exe_path) :
		RunningProcess(_s_create_process(exe_path, L""))
	{}
	CreatedProcess(const std::wstring& exe_path, const std::wstring& command_line) :
		RunningProcess(_s_create_process(exe_path, command_line))
	{}

	CreatedProcess(const CreatedProcess&) = delete;
	CreatedProcess& operator=(const CreatedProcess&) = delete;

	CreatedProcess(CreatedProcess&& p) noexcept = default;

	virtual ~CreatedProcess() = default;

private:
	static AutoCloseHandle _s_create_process(const std::wstring& exe_path, std::wstring command_line);
};


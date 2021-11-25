#pragma once

#include "RunningProcess.h"
#include "AutoCloseHandle.hpp"

// TODO:DebugActiveProcessStop/DebugSetProcessKillOnExit
class AttachedProcess :
	public RunningProcess
{
public:
	explicit AttachedProcess(const DWORD process_id) :
		RunningProcess(_s_attach_process(process_id), process_id)
	{}

	AttachedProcess(const AttachedProcess&) = delete;
	AttachedProcess& operator=(const AttachedProcess&) = delete;

	AttachedProcess(AttachedProcess&& p) noexcept = default;

	virtual ~AttachedProcess() = default;

private:
	static AutoCloseHandle _s_attach_process(const DWORD process_id);
};


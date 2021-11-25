#include "AttachedProcess.h"

AutoCloseHandle AttachedProcess::_s_attach_process(const DWORD process_id)
{
	// TODO: I request more rights here than are requested by DebugActiveProcess. Maybe minimize it? Or allow it to not be used?
	AutoCloseHandle process_handle(OpenProcess(PROCESS_ALL_ACCESS, false, process_id));

	if (!DebugActiveProcess(process_id))
	{
		throw std::exception("DebugActiveProcess failed");
	}

	// TODO: Is that necessary?
	return std::move(process_handle);
}
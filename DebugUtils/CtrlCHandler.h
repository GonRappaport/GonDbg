#pragma once

#include <Windows.h>
#include <vector>

typedef BOOL (PFN_CTRLHANDLER*)(DWORD ctrl_type, void* context);

class CtrlCHandler
{
public:
	explicit CtrlCHandler(PFN_CTRLHANDLER handler, void* context);
	~CtrlCHandler();

private:
	const PFN_CTRLHANDLER m_handler;
	const void* m_context;
	static std::vector<std::tuple<PFN_CTRLHANDLER, void*>> m_registrations;
	static bool m_registered_handler;
};


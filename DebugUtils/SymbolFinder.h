#pragma once

#include <memory>

#include "IDebuggedProcess.h"

class SymbolFinder
{
public:
	SymbolFinder(std::shared_ptr<IDebuggedProcess> process);
	~SymbolFinder();

	std::wstring get_symbol(RemotePointer address);

private:
	std::shared_ptr<IDebuggedProcess> m_process;
};


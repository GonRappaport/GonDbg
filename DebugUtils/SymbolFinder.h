#pragma once

#include <memory>
#include <list>

#include "IDebuggedProcess.h"
#include "LoadedModule.h"

class SymbolFinder
{
public:
	SymbolFinder(std::shared_ptr<IDebuggedProcess> process);
	~SymbolFinder();

	SymbolFinder(const SymbolFinder&) = delete;
	SymbolFinder& operator=(const SymbolFinder&) = delete;

	SymbolFinder(SymbolFinder&& sf) noexcept :
		m_process(sf.m_process),
		m_owner(std::exchange(sf.m_owner, false)),
		m_modules(std::move(sf.m_modules))
	{}
	SymbolFinder&& operator=(SymbolFinder&& sf) = delete;/*noexcept
	{
		m_process = sf.m_process;
		m_owner = std::exchange(sf.m_owner, false);
		m_modules = sf.m_modules;
	}*/

	std::wstring get_symbol(RemotePointer address) const;

	void load_module(HANDLE file_handle, const std::wstring& image_name, const RemotePointer image_base);
	void unload_module(const RemotePointer image_base);

	const std::list<LoadedModule>& get_loaded_modules() const { return m_modules; }

private:
	std::shared_ptr<IDebuggedProcess> m_process;
	bool m_owner;
	std::list<LoadedModule> m_modules;

	const LoadedModule& _get_module_by_address(RemotePointer address) const;
};


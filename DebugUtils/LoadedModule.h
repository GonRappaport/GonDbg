#pragma once

#include <Windows.h>
// TODO: Make sure you bring the DLL with you, or compile with the oldest version of dbghelp.h
#include <DbgHelp.h>
#include "IDebuggedProcess.h"

class LoadedModule
{
public:
	LoadedModule(const PIMAGEHLP_MODULEW64 module_info, const HANDLE process_handle);
	~LoadedModule() = default;

	const std::wstring& get_image_name() const { return m_image_name; }
	const RemotePointer get_image_base() const { return m_image_base; }
	const DWORD get_image_size() const { return m_image_size; }
	bool is_address_in_image(RemotePointer address) const;

	// TODO: Doesn't work with unloaded modules
	bool operator==(const LoadedModule& other) const { return m_image_base == other.m_image_base; }

private:
	const RemotePointer m_image_base;
	const DWORD m_image_size;
	const std::wstring m_image_name;
	const std::wstring m_image_path;
	//const std::wstring m_pdb_name;
	SYM_TYPE m_symbols_type;

	static std::wstring _s_get_pdb_path(const PIMAGEHLP_MODULEW64 module_info, const HANDLE process_handle);
};

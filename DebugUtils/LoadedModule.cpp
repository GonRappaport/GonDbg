#include "LoadedModule.h"

#include "Exceptions.h"

#include <Psapi.h>

LoadedModule::LoadedModule(const PIMAGEHLP_MODULEW64 module_info, const HANDLE process_handle):
	m_image_base(module_info->BaseOfImage),
	m_image_size(module_info->ImageSize),
	m_symbols_type(module_info->SymType),
	m_image_name(_s_resolve_module_name(module_info, process_handle)),
	m_image_path(module_info->ImageName)/*,
	m_pdb_name(_s_get_pdb_path(module_info, process_handle))*/
{}

bool LoadedModule::is_address_in_image(RemotePointer address) const
{
	return ((address >= m_image_base) &&
		(address < (m_image_base + m_image_size)));
}

std::wstring LoadedModule::_s_get_pdb_path(const PIMAGEHLP_MODULEW64 module_info, const HANDLE process_handle)
{
	if (module_info->ModuleName[0] == UNICODE_NULL)
	{
		return L"";
	}
	std::vector<wchar_t> pdb_path_raw(MAX_PATH);
	std::vector<wchar_t> dbg_path_raw(MAX_PATH);
	if (!SymGetSymbolFileW(process_handle, nullptr, module_info->ImageName, sfImage, pdb_path_raw.data(), pdb_path_raw.size(), dbg_path_raw.data(), dbg_path_raw.size()))
	{
		throw WinAPIException("SymGetSymbolFileW failed");
	}
	return std::wstring(pdb_path_raw.data());
}

std::wstring LoadedModule::_s_resolve_module_name(const PIMAGEHLP_MODULEW64 module_info, const HANDLE process_handle)
{
	if (module_info->ModuleName[0] != UNICODE_NULL)
	{
		return module_info->ModuleName;
	}

	wchar_t process_name_buffer[MAX_PATH] = { 0 };
	
	DWORD name_length = GetModuleBaseNameW(process_handle, 
		reinterpret_cast<HMODULE>(module_info->BaseOfImage), 
		process_name_buffer, 
		ARRAYSIZE(process_name_buffer));
	if (0 == name_length)
	{
		// TODO: That's a hack as it seems both ntdlll and the loaded module fail to have their names retrieved :(
		return L"NTDLL";
	}

	return std::wstring(process_name_buffer);
}

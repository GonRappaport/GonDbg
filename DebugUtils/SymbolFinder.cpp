#include <Windows.h>
#include <DbgHelp.h>

#include "SymbolFinder.h"
#include "ISimpleIO.h"

SymbolFinder::SymbolFinder(std::shared_ptr<IDebuggedProcess> process):
    m_process(process),
    m_owner(true)
{
    if (!SymInitializeW((HANDLE)process->get_process_handle(), nullptr, false))
    {
        throw std::exception("SymInitializeW failed");
    }

    DWORD path_length = GetEnvironmentVariableW(L"_NT_SYMBOL_PATH", nullptr, 0);
    if (0 == path_length)
    {
        throw std::exception("GetEnvironmentVariableW failed");
    }

    std::vector<wchar_t> symbol_path(path_length);
    if ((path_length-1) != GetEnvironmentVariableW(L"_NT_SYMBOL_PATH", symbol_path.data(), static_cast<DWORD>(symbol_path.size())))
    {
        throw std::exception("GetEnvironmentVariableW failed");
    }

    if (!SymSetSearchPathW(process->get_process_handle(), symbol_path.data()))
    {
        throw std::exception("SymGetSearchPathW failed");
    }
}

SymbolFinder::~SymbolFinder()
{
    try
    {
        if (m_owner)
        {
            SymCleanup(m_process->get_process_handle());
        }
    }
    catch(...)
    { }
}

std::wstring SymbolFinder::get_symbol(RemotePointer address) const
{
    DWORD64 displacement;
    std::vector<BYTE> symbol_info_raw(sizeof(SYMBOL_INFOW) +( (MAX_SYM_NAME + 1) * sizeof(wchar_t)));
    PSYMBOL_INFOW symbol_info = reinterpret_cast<PSYMBOL_INFOW>(symbol_info_raw.data());

    symbol_info->SizeOfStruct = sizeof(*symbol_info);
    symbol_info->MaxNameLen = MAX_SYM_NAME;

    if (!SymFromAddrW(m_process->get_process_handle(), address, &displacement, symbol_info))
    {
        throw std::exception("SymFromAddrW failed");
    }

    return ISimpleIO::format(L"%s!%s+0x%llX",
        _get_module_by_address(address).get_image_name().c_str(),
        symbol_info->Name,
        displacement);
}

void SymbolFinder::load_module(HANDLE file_handle, const std::wstring& image_name, const RemotePointer image_base)
{
    if (0 == SymLoadModuleExW(m_process->get_process_handle(),
        file_handle,
        image_name.c_str(),
        nullptr,
        static_cast<DWORD64>(image_base),
        0,
        nullptr,
        0))
    {
        throw std::exception("SymLoadModuleExW failed");
    }

    IMAGEHLP_MODULEW64 module_info = { 0 };
    module_info.SizeOfStruct = sizeof(module_info);
    if (!SymGetModuleInfoW64(m_process->get_process_handle(), image_base, &module_info))
    {
        throw std::exception("SymGetModuleInfoW64 failed");
    }

    m_modules.emplace_back(&module_info, m_process->get_process_handle());
}

void SymbolFinder::unload_module(const RemotePointer image_base)
{
    bool found = false;

    // TODO: Place in unloaded modules list
    for (const LoadedModule& loaded_module : m_modules)
    {
        if (loaded_module.is_address_in_image(image_base))
        {
            found = true;
            m_modules.remove(loaded_module);
            break;
        }
    }

    if (!found)
    {
        throw std::exception("Unloaded module not recognized");
    }

    if (!SymUnloadModule64(m_process->get_process_handle(), image_base))
    {
        throw std::exception("SymUnloadModule64 failed");
    }
}

const LoadedModule& SymbolFinder::_get_module_by_address(RemotePointer address) const
{
    for (const LoadedModule& loaded_module : m_modules)
    {
        if (loaded_module.is_address_in_image(address))
        {
            return loaded_module;
        }
    }
    throw std::exception("Address not in any module");
}

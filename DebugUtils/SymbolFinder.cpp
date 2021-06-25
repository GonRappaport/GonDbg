#include <Windows.h>
#include <DbgHelp.h>

#include "SymbolFinder.h"

SymbolFinder::SymbolFinder(std::shared_ptr<IDebuggedProcess> process):
    m_process(process)
{
    if (!SymInitializeW(process->get_process_handle(), nullptr, false))
    {
        throw std::exception("SymInitializeW failed");
    }
}

SymbolFinder::~SymbolFinder()
{
    try
    {
        SymCleanup(m_process->get_process_handle());
    }
    catch(...)
    { }
}

std::wstring SymbolFinder::get_symbol(RemotePointer address)
{
    //SymFromAddrW(m_process->get_process_handle(), address, &displacement, &symbol_info);
    return L"";
}

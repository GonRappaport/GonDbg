#include "LoadedLibrary.h"
#include "Exceptions.h"

FARPROC LoadedLibrary::get_export(WORD ordinal) const
{
    FARPROC func_address = GetProcAddress(m_handle.get_value(), MAKEINTRESOURCEA(ordinal));
    if (nullptr == func_address)
    {
        // TODO: Special exception for func not found?
        throw WinAPIException("GetProcAddress failed");
    }
    return func_address;
}

FARPROC LoadedLibrary::get_export(std::string name) const
{
    FARPROC func_address = GetProcAddress(m_handle.get_value(), name.c_str());
    if (nullptr == func_address)
    {
        // TODO: Special exception for func not found?
        throw WinAPIException("GetProcAddress failed");
    }
    return func_address;
}

std::wstring LoadedLibrary::_s_resolve_full_path(const std::wstring& library_path, const std::vector<std::wstring>& search_paths)
{
    std::vector<AutoCloseHandleImpl<DLL_DIRECTORY_COOKIE, nullptr, RemoveDllDirectory>> added_directories;

    added_directories.reserve(search_paths.size());

    for (const std::wstring& path : search_paths)
    {
        added_directories.emplace_back(AddDllDirectory(path.c_str()));
    }

    // TODO: Loading it twice to get the file path is somewhat of a security issue, but I don't really care right now.
    AutoFreeLibrary loaded_library (LoadLibraryExW(library_path.c_str(),
        nullptr,
        LOAD_LIBRARY_SEARCH_DEFAULT_DIRS | LOAD_LIBRARY_AS_DATAFILE | LOAD_LIBRARY_AS_IMAGE_RESOURCE));

    DWORD file_path_length = GetModuleFileNameW(loaded_library.get_value(), nullptr, 0);
    DWORD last_error(GetLastError());
    if (ERROR_INSUFFICIENT_BUFFER != last_error)
    {
        throw WinAPIException("GetModuleFileNameW failed", last_error);
    }

    std::vector<wchar_t> file_path(static_cast<size_t>(file_path_length) + 1);
    if (file_path_length != GetModuleFileNameW(loaded_library.get_value(), file_path.data(), file_path_length))
    {
        throw WinAPIException("GetModuleFileNameW failed");
    }

    return std::wstring(file_path.data());
}

// TODO: Doesn't support Windows XP
// TODO: For Windows Vista & Windows 7, requires a KB
AutoFreeLibrary LoadedLibrary::_s_load_library_extra_paths(const std::wstring& library_path, const std::vector<std::wstring>& search_paths)
{
    std::vector<AutoCloseHandleImpl<DLL_DIRECTORY_COOKIE, nullptr, RemoveDllDirectory>> added_directories;

    added_directories.reserve(search_paths.size());

    for (const std::wstring& path : search_paths)
    {
        added_directories.emplace_back(AddDllDirectory(path.c_str()));
    }

    return AutoFreeLibrary(LoadLibraryExW(library_path.c_str(),
        nullptr,
        LOAD_LIBRARY_SEARCH_DEFAULT_DIRS));
}

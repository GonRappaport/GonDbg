#pragma once

#include "AutoCloseHandle.hpp"

class LoadedLibrary
{
public:
	explicit LoadedLibrary(const std::wstring& library_path):
		m_path(library_path),
		m_handle(LoadLibraryW(library_path.c_str()))
	{}
	LoadedLibrary(const std::wstring& library_path, const std::vector<std::wstring> search_paths):
		m_path(_s_resolve_full_path(library_path, search_paths)),
		m_handle(_s_load_library_extra_paths(library_path, search_paths))
	{}
	~LoadedLibrary() = default;

	LoadedLibrary(const LoadedLibrary&) = delete;
	LoadedLibrary& operator=(const LoadedLibrary&) = delete;
	LoadedLibrary(LoadedLibrary&& ll) noexcept :
		m_path(ll.m_path),
		m_handle(std::move(ll.m_handle))
	{}
	LoadedLibrary& operator=(LoadedLibrary&& lp) = delete;

	std::wstring get_path() const { return m_path; }

	FARPROC get_export(WORD ordinal) const;
	FARPROC get_export(std::string name) const;

private:
	const std::wstring& m_path;
	AutoFreeLibrary m_handle;

	std::wstring _s_resolve_full_path(const std::wstring& library_path, const std::vector<std::wstring>& search_paths);
	AutoFreeLibrary _s_load_library_extra_paths(const std::wstring& library_path, const std::vector<std::wstring>& search_paths);
};


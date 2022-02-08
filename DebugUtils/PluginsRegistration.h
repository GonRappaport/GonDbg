#pragma once

#include <functional>

#include "LoadedLibrary.h"

const std::string DEFAULT_PLUGIN_INIT_FUNCTION("module_init");

class LoadedPlugin
{
public:
	// TODO: Maybe add an optional environment variable that will be used as path for plugins, which you'll use with the appropriate WinAPI to search for in that path
	// TODO: Why is everything here by reference??
	LoadedPlugin(const std::wstring& plugin_name, const std::wstring& plugin_path, const std::string& init_function_name=DEFAULT_PLUGIN_INIT_FUNCTION) :
		m_plugin_path(plugin_path),
		m_plugin(_s_load_plugin(plugin_path, init_function_name)),
		m_plugin_name(plugin_name)
	{}
	~LoadedPlugin() = default;

	LoadedPlugin(const LoadedPlugin&) = delete;
	LoadedPlugin& operator=(const LoadedPlugin&) = delete;
	LoadedPlugin(LoadedPlugin&& lp) noexcept :
		m_plugin_path(lp.m_plugin_path),
		m_plugin(std::move(lp.m_plugin))
	{}
	LoadedPlugin& operator=(LoadedPlugin&& lp) = delete;

	const std::wstring& get_plugin_name() const { return m_plugin_name;	}
	const std::wstring& get_plugin_path() const { return m_plugin_path; }
	// TODO: How will plugins export commands? By an export? By registering to the debugger directly? By passing some struct?

private:
	const std::wstring m_plugin_path;
	const std::wstring m_plugin_name;
	LoadedLibrary m_plugin;

	LoadedLibrary _s_load_plugin(const std::wstring& plugin_path, const std::string& init_function_name);
};

class PluginsRegistration
{
public:
	PluginsRegistration() = default;
	~PluginsRegistration() = default;

	PluginsRegistration(const PluginsRegistration&) = delete;
	PluginsRegistration& operator=(const PluginsRegistration&) = delete;
	// TODO: is this legit that it's the default? Or do I need to implement that?
	PluginsRegistration(PluginsRegistration&&) noexcept = default;
	PluginsRegistration& operator=(PluginsRegistration&&) = default;

	const LoadedPlugin& get_plugin_by_name(const std::wstring& plugin_name);
	void load_plugin(const std::wstring& plugin_path, const std::wstring& plugin_name);

private:
	std::list<LoadedPlugin> m_loaded_plugins;
};


#include "PluginsRegistration.h"
#include "Exceptions.h"

// TODO: Understand what's the proper way to transfer C++ objects between modules which may be compiled on different C++ standards or even C or other compilers.
// TODO: And add a calling convention?
using init_function_type = std::function<void(void)>;

LoadedLibrary LoadedPlugin::_s_load_plugin(const std::wstring& plugin_path, const std::string& init_function_name)
{
    LoadedLibrary loaded_module(plugin_path);

    init_function_type init_function(loaded_module.get_export(init_function_name));
    // TODO: That's temporary
    init_function();

    return loaded_module;
}

const LoadedPlugin& PluginsRegistration::get_plugin_by_name(const std::wstring& plugin_name)
{
    for (const auto& plugin : m_loaded_plugins)
    {
        // TODO: Consider making that case insensitive. Also for debugger commands
        if (plugin.get_plugin_name() == plugin_name)
        {
            return plugin;
        }
    }

    throw std::exception("Plugin not found");
}

void PluginsRegistration::load_plugin(const std::wstring& plugin_path, const std::wstring& plugin_name)
{
    m_loaded_plugins.emplace_back(plugin_name, plugin_path);
}

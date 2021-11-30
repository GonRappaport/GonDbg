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

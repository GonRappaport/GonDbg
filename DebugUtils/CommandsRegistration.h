#pragma once

#include <Windows.h>
#include <string>
#include <functional>
#include <optional>

// Forward reference
class Debugger;
using CommandInterface = std::function<DWORD(std::wstring, Debugger&)>;

class RegisteredCommand
{
public:
	RegisteredCommand(CommandInterface impl, const std::wstring& command_name) :
		m_implementation(impl),
		m_command_name(command_name)
	{}
	RegisteredCommand(CommandInterface impl, const std::wstring& command_name, const std::wstring& plugin_name) :
		m_implementation(impl),
		m_command_name(command_name),
		m_plugin_name(plugin_name)
	{}
	const CommandInterface m_implementation;
	const std::wstring m_command_name;
	const std::optional<std::wstring> m_plugin_name;
};

class CommandsRegistration
{
public:
	CommandsRegistration() = default;
	~CommandsRegistration() = default;

	CommandsRegistration(const CommandsRegistration&) = default;
	CommandsRegistration& operator=(const CommandsRegistration&) = default;
	CommandsRegistration(CommandsRegistration&&) noexcept = default;
	CommandsRegistration& operator=(CommandsRegistration&&) = default;

	const RegisteredCommand& get_command(const std::wstring command_name);

	static std::pair<const std::wstring, const std::wstring> s_parse_command(const std::wstring& command_line);

	// TODO: Commands starting with '.' are debugger settings related functions
	// TODO: Commands starting with '!' are extension functions
	// TODO: The plugin name before ! is optional. If it appears, it's from an explicit plugin. Otherwise, search all plugins
	// TODO: Other commands are normal debug commands

	// This is for use by GonDBG
	// TODO: Consider wrapping this with #ifdef GONDBG
	void register_command(const std::wstring command_name, const CommandInterface command_impl);
	// This is for use by extensions
	void register_command(const std::wstring command_name, const std::wstring plugin_name, const CommandInterface command_impl);

private:
	std::unordered_map<std::wstring, RegisteredCommand> m_registered_commands;
};


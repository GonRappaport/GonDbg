#pragma once

#include <string_view>
#include <optional>
#include <unordered_map>

#include "Debugger.h"

using CommandInterface = std::function<void(std::wstring, Debugger&)>;

class RegisteredCommand
{
public:
	RegisteredCommand(CommandInterface impl, const std::wstring& command_name):
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

class Command
{
public:
	explicit Command(const std::wstring_view command_line);
	Command(const Command&) = default;
	Command& operator=(const Command&) = default;
	Command(Command&&) = default;
	Command& operator=(Command&&) = default;
	
	const std::wstring& get_command_name() const { return m_command.m_command_name; }
	const std::wstring& get_command_params() const { return m_raw_params; }
	const bool is_plugin_command() const { return m_command.m_plugin_name.has_value(); }
	const std::wstring& get_plugin_name() const { return m_command.m_plugin_name.value(); }

	void execute(Debugger& debugger);

	// This is for use by GonDBG
	// TODO: Consider wrapping this with #ifdef GONDBG
	static void s_register_command(const std::wstring command_name, const CommandInterface command_impl);
	// This is for use by extensions
	static void s_register_command(const std::wstring command_name, const std::wstring plugin_name, const CommandInterface command_impl);

private:
	static std::wstring _s_get_params(const std::wstring_view& command_line);
	static const RegisteredCommand& _s_get_command(const std::wstring_view& command_line);

	const std::wstring m_raw_params;
	const RegisteredCommand& m_command;

	static std::unordered_map<std::wstring, RegisteredCommand> m_registered_commands;
};


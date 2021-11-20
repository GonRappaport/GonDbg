#include "CommandsRegistration.h"

#include <sstream>

// Copied from the documentation of isspace
static const std::wstring WHITE_SPACES(L" \t\n\v\f\r");

std::pair<const std::wstring, const std::wstring> CommandsRegistration::s_parse_command(const std::wstring& command_line)
{
	size_t command_name_length = command_line.find_first_of(WHITE_SPACES);
	std::wstring command_params(L"");

	if (command_name_length == std::wstring::npos)
	{
		// It's a command with no params
		command_name_length = command_line.length();
	}

	std::wstring command_name(command_line.substr(0, command_name_length));
	std::wstring raw_params(command_line.substr(command_name_length));

	return std::make_pair(command_name, raw_params);
}

const RegisteredCommand& CommandsRegistration::get_command(const std::wstring command_name)
{
	if (L'!' == command_name.front())
	{
		// This is a plugin command but we don't know where from. Start iterating
		for (const auto& command : m_registered_commands)
		{
			if (command.second.m_plugin_name.has_value() &&
				command.second.m_command_name == command_name.substr(1))
			{
				return command.second;
			}
		}
	}
	return m_registered_commands.at(command_name);
}

void CommandsRegistration::register_command(const std::wstring command_name, const CommandInterface command_impl)
{
	if (m_registered_commands.find(command_name) != m_registered_commands.end())
	{
		throw std::exception("The same command was registered twice");
	}
	m_registered_commands.emplace(std::make_pair(command_name,
		RegisteredCommand(command_impl, command_name)));
}

void CommandsRegistration::register_command(const std::wstring command_name, const std::wstring plugin_name, const CommandInterface command_impl)
{
	std::wstringstream command_key;
	command_key << plugin_name << L"!" << command_name;
	if (m_registered_commands.find(command_key.str()) != m_registered_commands.end())
	{
		throw std::exception("The same command was registered twice");
	}
	m_registered_commands.emplace(std::make_pair(command_key.str(),
		RegisteredCommand(command_impl, command_name, plugin_name)));
}

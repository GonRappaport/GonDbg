#include "Command.h"

#include <sstream>

// Copied from the documentation of isspace
static const std::wstring WHITE_SPACES(L" \t\n\v\f\r");

std::unordered_map<std::wstring, RegisteredCommand> Command::m_registered_commands;

std::wstring Command::_s_get_params(const std::wstring_view& command_line)
{
	size_t command_name_length = command_line.find_first_of(WHITE_SPACES);

	if (command_name_length == std::wstring::npos)
	{
		// It's a command with no params
		return L"";
	}

	return std::wstring(command_line.substr(command_name_length));
}

const RegisteredCommand& Command::_s_get_command(const std::wstring_view& command_line)
{
	size_t command_name_length = command_line.find_first_of(WHITE_SPACES);

	if (command_name_length == std::wstring::npos)
	{
		// It's a command with no params
		command_name_length = command_line.length();
	}

	size_t plugin_name_length = command_line.find_first_of(L'!', 0);

	std::wstring command_name;
	std::optional<std::wstring> plugin_name;

	if ((plugin_name_length == std::wstring::npos) ||
		(plugin_name_length > command_name_length))
	{
		// This isn't a plugin command
		plugin_name_length = 0;
		command_name = command_line.substr(0, command_name_length);

		return m_registered_commands.at(command_name);
	}
	else if (plugin_name_length == 0)
	{
		// This is a plugin command but we need to find out ourselves from which plugin
		for (const auto& command : m_registered_commands)
		{
			if (command.second.m_plugin_name.has_value() &&
				command.second.m_command_name == command_name)
			{
				return command.second;
			}
		}
	}
	else
	{
		// This is a plugin command
		plugin_name = command_line.substr(0, plugin_name_length);
		command_name = command_line.substr(plugin_name_length + 1, command_name_length - (plugin_name_length + 1));
		// Construct the command key
		std::wstringstream command_key;
		command_key << plugin_name.value() << L"!" << command_name;
		return m_registered_commands.at(command_key.str());
	}
}

Command::Command(const std::wstring_view command_line):
	m_raw_params(_s_get_params(command_line)),
	m_command(_s_get_command(command_line))
{}

void Command::execute(Debugger& debugger)
{
}

void Command::s_register_command(const std::wstring command_name, const CommandInterface command_impl)
{
	if (m_registered_commands.find(command_name) != m_registered_commands.end())
	{
		throw std::exception("The same command was registered twice");
	}
	m_registered_commands.emplace(std::make_pair(command_name,
		RegisteredCommand(command_impl, command_name)));
}

void Command::s_register_command(const std::wstring command_name, const std::wstring plugin_name, const CommandInterface command_impl)
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

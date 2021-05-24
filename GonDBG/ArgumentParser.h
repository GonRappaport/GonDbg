#pragma once

#include <Windows.h>
#include <Shlwapi.h>
#include <string>
#include <vector>
#include <optional>

enum class DebugTargetType
{
	Attach,
	NonInvasiveAttach,
	Create
};

enum class DebugeeIdentifier
{
	Pid,
	ProcessName,
	FilePath
};

class ProgramParameters
{
public:
	ProgramParameters(
		const DebugTargetType debugee_type,
		const std::wstring_view& debugee_path)
		: m_debugee_type(debugee_type),
		m_debugee_identifier((debugee_type == DebugTargetType::Create) ? (DebugeeIdentifier::FilePath) : (DebugeeIdentifier::ProcessName)),
		m_debugee_path(debugee_path)
	{};

	ProgramParameters(
		const DebugTargetType debugee_type,
		const DWORD debugee_pid)
		: m_debugee_type(debugee_type),
		m_debugee_identifier(DebugeeIdentifier::Pid),
		m_debugee_pid(debugee_pid)
	{};

	// Getters
	const std::wstring& get_debugee_path() const
	{
		// TODO: This if is literally what value() does. Consider removing
		if (!m_debugee_path.has_value())
		{
			throw std::exception("No debugee path given");
		}
		return m_debugee_path.value();
	}

	const DWORD get_debugee_pid() const
	{
		if (!m_debugee_pid.has_value())
		{
			throw std::exception("No debugee PID given");
		}
		return m_debugee_pid.value();
	}

	const DebugTargetType get_debugee_type() const { return m_debugee_type; };
	const DebugeeIdentifier get_debugee_identifier() const { return m_debugee_identifier; }

	__declspec(property (get = get_debugee_type)) const DebugTargetType debugee_type;
	__declspec(property (get = get_debugee_identifier)) const DebugeeIdentifier debugee_identifier;
	__declspec(property (get = get_debugee_path)) const std::wstring& debugee_path;
	__declspec(property (get = get_debugee_pid)) const DWORD debugee_pid;

private:
	const DebugTargetType m_debugee_type;
	const DebugeeIdentifier m_debugee_identifier;

	const std::optional<const std::wstring> m_debugee_path;

	const std::optional<const DWORD> m_debugee_pid;
};

namespace ArgumentParser
{
	const 
	ProgramParameters
	Parse(const std::vector<std::wstring_view>& raw_input);
};

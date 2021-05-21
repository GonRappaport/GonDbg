#include "ArgumentParser.h"

const
ProgramParameters
ArgumentParser::Parse(const std::vector<std::wstring_view>& raw_input)
{
	DebugTargetType debugee_type;
	unsigned int next_index = 0;

	// Check how a process should be debugged (See DebugTargetType)
	// TODO: Make wrappers for string parsing (Also for the PID parsing below)
	if (0 == _wcsicmp(raw_input[next_index].data(), L"attach"))
	{
		debugee_type = DebugTargetType::Attach;
		next_index++;
		if (0 == _wcsicmp(raw_input[next_index].data(), L"--no-inject"))
		{
			debugee_type = DebugTargetType::NonInvasiveAttach;
			next_index++;
		}
	}
	else if (0 == _wcsicmp(raw_input[next_index].data(), L"create"))
	{
		debugee_type = DebugTargetType::Create;
		next_index++;
	}
	else
	{
		throw std::exception("Unknown debugee type received");
	}

	// Check if the last argument is a PID or a path/process name
	int iPid = -1;
	C_ASSERT(sizeof(iPid) == sizeof(DWORD));
	if (StrToIntExW(raw_input[next_index].data(), STIF_DEFAULT, &iPid))
	{
		// TODO: For future, do better input parameter parsing, as a process can be named 1235, without .exe.
		if (debugee_type == DebugTargetType::Create)
		{
			throw std::exception("Can't create a process with a PID");
		}
		return ProgramParameters(debugee_type, static_cast<DWORD>(iPid));
	}

	return ProgramParameters(debugee_type, raw_input[next_index]);
}
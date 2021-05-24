#include <Windows.h>
#include <string>
#include <vector>
#include <iostream>

#include "ArgumentParser.h"
#include "Debugger.h"

static
INT
SafeMain(
	INT iArgc,
	PCWSTR* ppwszArgv
)
{
	try
	{
		const std::vector<std::wstring_view> raw_parameters(
			ppwszArgv + 1, // Skip the debugger path
			ppwszArgv + iArgc
		);

		auto params = ArgumentParser::Parse(raw_parameters);

		DebuggerPtr debugger;

		bool is_invasive = true;
		switch (params.debugee_type)
		{
		case DebugTargetType::NonInvasiveAttach:
			is_invasive = false;
			__fallthrough;
		case DebugTargetType::Attach:
			if (params.debugee_identifier == DebugeeIdentifier::FilePath)
			{
				debugger = DebuggerUtils::attach(params.debugee_path, is_invasive);
			}
			else
			{
				debugger = DebuggerUtils::attach(params.debugee_pid, is_invasive);
			}
			break;
		case DebugTargetType::Create:
			debugger = DebuggerUtils::create(params.debugee_path);
			break;
		}

		debugger->debug();

		return 0;
	}
	catch (const std::exception& e)
	{
		std::wcout << e.what();
		return -1;
	}
	catch (...)
	{
		std::wcout << L"Unknwon exception";
		return -2;
	}
}

EXTERN_C
{

INT
wmain(
	INT iArgc,
	PCWSTR * ppwszArgv
)
{
	__try
	{
		return SafeMain(iArgc, ppwszArgv);
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		return -1;
	}
}

}
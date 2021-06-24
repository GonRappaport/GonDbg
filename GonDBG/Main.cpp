#include <Windows.h>
#include <string>
#include <vector>
#include <iostream>

#include "ArgumentParser.h"
#include "Debugger.h"
#include "ConsoleIO.h"

static
INT
SafeMain(
	INT iArgc,
	PCWSTR* ppwszArgv
)
{
	try
	{
		std::shared_ptr<ConsoleIO> io = std::make_shared<ConsoleIO>();

		if (iArgc == 1)
		{
			io->write_formatted(L"Usage: %s <create|attach [--no-inject]> <PID/Path>", ppwszArgv[0]);
			return 1;
		}

		const std::vector<std::wstring_view> raw_parameters(
			ppwszArgv + 1, // Skip the debugger path
			ppwszArgv + iArgc
		);

		auto params = ArgumentParser::Parse(raw_parameters);

		std::unique_ptr<Debugger> debugger;

		switch (params.debugee_type)
		{
		case DebugTargetType::NonInvasiveAttach:
			if (params.debugee_identifier == DebugeeIdentifier::FilePath)
			{
				debugger = std::make_unique<Debugger>(Debugger::attach_to_process_no_inject(params.debugee_path, io));
			}
			else
			{
				debugger = std::make_unique<Debugger>(Debugger::attach_to_process_no_inject(params.debugee_pid, io));
			}
			break;
		case DebugTargetType::Attach:
			if (params.debugee_identifier == DebugeeIdentifier::FilePath)
			{
				debugger = std::make_unique<Debugger>(Debugger::attach_to_process(params.debugee_path, io));
			}
			else
			{
				debugger = std::make_unique<Debugger>(Debugger::attach_to_process(params.debugee_pid, io));
			}
			break;
		case DebugTargetType::Create:
			debugger = std::make_unique<Debugger>(Debugger::debug_new_process(params.debugee_path, io));
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
		// TODO: You got this when passing gondbg.exe attach 1
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
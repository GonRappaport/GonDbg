#include <Windows.h>
#include <memory>
#include <string>

#pragma once
class Debugger
{
public:

	void debug();

private:

};

using DebuggerPtr = std::unique_ptr<Debugger>;

namespace DebuggerUtils
{
	DebuggerPtr attach(const DWORD pid, bool is_invasive);
	DebuggerPtr attach(const std::wstring&, bool is_invasive);

	DebuggerPtr create(const std::wstring& exe_path);
}
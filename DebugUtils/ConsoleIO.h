#pragma once

#include "ISimpleIO.h"

class ConsoleIO: 
	public ISimpleIO
{
public:
	ConsoleIO() {}

	virtual ~ConsoleIO() = default;

	virtual std::wstring read();
	virtual void write(const std::wstring&);
};


#pragma once

#include <Windows.h>
#include <vector>
#include <string>

class RemotePointer
{
public:
	RemotePointer() :
		m_address(0)
	{}
	explicit RemotePointer(const PVOID ptr):
		m_address(reinterpret_cast<DWORD64>(ptr))
	{}

	explicit RemotePointer(const DWORD64 addr):
		m_address(addr)
	{}

	RemotePointer(const RemotePointer & rp) :
		m_address(rp.m_address)
	{}

	RemotePointer& operator=(const PVOID ptr)
	{
		m_address = reinterpret_cast<DWORD64>(ptr);
		return *this;
	}

	RemotePointer& operator=(const DWORD64 addr)
	{
		m_address = addr;
		return *this;
	}

	RemotePointer& operator=(const RemotePointer & rp)
	{
		m_address = rp.m_address;
		return *this;
	}

	operator PVOID() const { return reinterpret_cast<PVOID>(m_address); }
	operator DWORD64() const { return m_address; }
	DWORD64* operator&() { return &m_address; }

	inline bool operator< (const RemotePointer& rhs) const { return (m_address < rhs.m_address); }
	inline bool operator> (const RemotePointer& rhs) const { return rhs < (*this); }
	inline bool operator<=(const RemotePointer& rhs) const { return !((*this) > rhs); }
	inline bool operator>=(const RemotePointer& rhs) const { return !((*this) < rhs); }
	inline bool operator==(const RemotePointer& rhs) const { return (m_address == rhs.m_address); }
	inline bool operator!=(const RemotePointer& rhs) const { return !((*this) == rhs); }

	bool is_null() const { return 0 == m_address; }

private:
	DWORD64 m_address;
};

class IDebuggedProcess
{
public:
	virtual HANDLE get_process_handle() const = 0;
	virtual DWORD get_process_id() const = 0;
	virtual bool is_64_bit() const = 0;
	// No need to use PVOID64 and such as it's impossible to debug 64-bit processes from a 32-bit debugger
	// TODO: Or is it? ;)
	virtual std::vector<BYTE> read_memory(RemotePointer base_address, SIZE_T size) = 0;
	virtual void write_memory(RemotePointer base_address, std::vector<BYTE> data) = 0;
	// TODO: That requires you to keep data of if the process is 32 or 64 bit
	virtual RemotePointer read_pointer(RemotePointer base_address);
	virtual std::string read_string(RemotePointer base_address);
	virtual std::wstring read_wstring(RemotePointer base_address);
	virtual std::string read_string_capped(RemotePointer base_address, SIZE_T max_length);
	virtual std::wstring read_wstring_capped(RemotePointer base_address, SIZE_T max_length);

	virtual ~IDebuggedProcess() = default;

protected:
	virtual std::vector<BYTE> _read_page(RemotePointer base_address);
};
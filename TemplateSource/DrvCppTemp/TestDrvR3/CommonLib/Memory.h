#pragma once
//#include "stdafx.h"

namespace usr::util
{
	using memory_block = struct 
	{
		PVOID Address;
		SIZE_T Size;
	};
	class Memory
	{
	public:
		Memory(){};
		~Memory() {};
		Memory(Process _process) {
			_Process = _process;
		};
		Memory & operator = (const Memory &_mem)
		{
			this->_Process = _mem._Process;
			return *this;
		}
	private:
		Process _Process;
	private:
		bool is_local() {
			return (_Process.get() == GetCurrentProcess());
		}
	private:
		template<typename Type>
		Type read(DWORD_PTR Address)
		{
			auto _handle = _Process.get();
			Type ret = Type(0);
			SIZE_T dwRet = 0;
			auto bret = ReadProcessMemory(_handle, (LPCVOID)Address, &ret,
				sizeof(Type), &dwRet);
			if (bret
				&& dwRet==sizeof(Type))
			{
				return ret;
			}
			DebugBreak();
			return ret;
		}
		template<typename T>
		bool write(DWORD_PTR Address, T Value)
		{
			auto _handle = _Process.get();
			SIZE_T dwRet = 0;
			auto bret = WriteProcessMemory(_handle,
				(LPVOID)Address,
				&Value,
				sizeof(T),
				&dwRet);
			if (bret
				&& dwRet == sizeof(T))
			{
				return true;
			}
			DebugBreak();
			return false;
		}
	public:
		template<typename T,typename...ARGS>
		T read_memory(ARGS&&...args)
		{
			auto SIZE_ARGS = sizeof...(args);
			DWORD_PTR offsets[] = { (DWORD_PTR)std::forward<ARGS>(args)... };
			T ret = T(0);
			DWORD_PTR address = 0;
			if (is_local())
			{
				for (size_t i = 0; i < SIZE_ARGS - 1; i++)
				{
					address += offsets[i];
					__try {
						address = *(DWORD_PTR*)address;
					}
					__except (1)
					{
						return ret;
					}
				}
				address += offsets[SIZE_ARGS - 1];
				ret = *(T*)address;
			}
			else
			{
				for (size_t i = 0; i < SIZE_ARGS - 1; i++)
				{
					address += offsets[i];
					address = read<DWORD_PTR>(address);
				}
				address += offsets[SIZE_ARGS - 1];
				ret = read<T>(address);
			}
			return ret;
		}
		template<typename T,typename...ARGS>
		bool write_memory(T arg1, ARGS...args)
		{
			auto SIZE_ARGS = sizeof...(args);
			DWORD_PTR offsets[] = { (DWORD_PTR)std::forward<ARGS>(args)... };
			DWORD_PTR address = 0;
			if (is_local())
			{
				for (size_t i = 0; i < SIZE_ARGS - 1; i++)
				{
					address += offsets[i];
					__try {
						address = *(DWORD_PTR*)address;
					}
					__except (1)
					{
						return false;
					}
				}
				address += offsets[SIZE_ARGS - 1];
				*(T*)address = arg1;
				return true;
			}
			else
			{
				for (size_t i = 0; i < SIZE_ARGS - 1; i++)
				{
					address += offsets[i];
					address = read<DWORD_PTR>(address);
				}
				address += offsets[SIZE_ARGS - 1];
				return write(address,arg1);
			}
			return false;
		}
	private:
		bool is_address_readable(PVOID Address, SIZE_T Size)
		{
			auto _handle = GetCurrentProcess();
			if (!is_local())_handle = _Process.get();
			MEMORY_BASIC_INFORMATION mbi = {};
			auto ret = VirtualQueryEx(_handle, Address, &mbi, sizeof(MEMORY_BASIC_INFORMATION));
			if (ret != 0)
			{
				if (!(mbi.Protect&PAGE_NOACCESS))
				{
					return true;
				}
			}
			SIZE_T dwRet = 0;
			BYTE Temp = 0;
			auto b_ret = ReadProcessMemory(_handle,
				Address, &Temp, sizeof(Temp), &dwRet);
			if (dwRet == sizeof(Temp) && b_ret)
			{
				return true;
			}
			if (is_local())
			{
				auto bBad = IsBadReadPtr(Address, Size);
				if (!bBad)
				{
					return true;
				}
			}
			return false;
		}
	private:
		void get_memory_range(std::vector<memory_block>&_memlist)
		{
			SYSTEM_INFO si = {};
			GetSystemInfo(&si);
			auto _handle = GetCurrentProcess();
			if (!is_local())
			{
				_handle = _Process.get();
			}
			PUCHAR addr = 0;
			auto max_addr = reinterpret_cast<PUCHAR>(si.lpMaximumApplicationAddress);
			while (addr<max_addr)
			{
				MEMORY_BASIC_INFORMATION meminfo = {};
				if (VirtualQueryEx(_handle, addr, &meminfo, sizeof(meminfo)) == 0)
				{
					addr += si.dwPageSize;
					DebugBreak();
					continue;
				}
				if (!(meminfo.Protect & PAGE_NOACCESS))
				{
					memory_block block = {};
					block.Address = meminfo.BaseAddress;
					block.Size = meminfo.RegionSize;
					_memlist.push_back(block);
				}
				addr=(PUCHAR)meminfo.BaseAddress+meminfo.RegionSize;
			}
		}
	private:
		bool read_memory(PVOID Address, SIZE_T Size, std::vector<BYTE> &_out)
		{
			auto _handle = is_local() ? GetCurrentProcess() : _Process.get();
			SIZE_T dwRet = 0;
			_out.resize(Size);
			std::uninitialized_fill_n(_out.begin(), Size,0);
			auto bRet = ReadProcessMemory(_handle,
				Address, &_out[0], Size, &dwRet);
			if (bRet)
			{
				return true;
			}
			return false;
		}
	private:
		//KMP 无wildcard的
		void kmp_nwc_getnext(std::vector<int>&_next,PBYTE _sig,SIZE_T _sig_size)
		{
			_next.resize(_sig_size);
			_next[0] = 0;
			SIZE_T j = 0;
			for (SIZE_T i = 1; i < _sig_size; i++)
			{
				while (j > 0 && _sig[j] != _sig[i])
				{
					j = _next[j - 1];
				}
				if (_sig[j] == _sig[i])
					j++;

				_next[i] = j;
			}
		}
		SIZE_T kmp_nwc_search0(std::vector<int> _next, std::vector<BYTE> _code, PBYTE _sig, SIZE_T _sig_size)
		{
			size_t j = 0;
			for (size_t i = 0; i < _code.size(); i++)
			{
				while (j > 0 && _code[i] != _sig[j])
					j = _next[j - 1];
				if (_code[i] == _sig[j])
					j++;
				if (j == _sig_size)
					return i - _sig_size + 1;
			}
			return -1;
		}
		SIZE_T kmp_nwc_search(std::vector<BYTE> _code, PBYTE _sig, SIZE_T _sig_size)
		{
			std::vector<int>_next = {};
			kmp_nwc_getnext(_next, _sig, _sig_size);
			return kmp_nwc_search0(_next, _code, _sig, _sig_size);
		}
		//KMP 支持WildCard = ?的
		void kmp_wc_getnext(std::vector<int>&_next, PBYTE _sig,PBYTE _mask,SIZE_T _sig_size)
		{
			_next.resize(_sig_size);
			_next[0] = 0;
			auto charMatch = [&](SIZE_T a1,SIZE_T a2) {
				if (_mask[a1] == '?' || _mask[a2] == '?')
					return true;
				return _sig[a1] == _sig[a2];
			};
			for (SIZE_T i = 1; i < _sig_size; ++i) {
				SIZE_T k = i - 1;
				while (k > 0 && !charMatch(i, _next[k])) k = _next[k - 1];
				if (_sig[i] == _sig[_next[k]]) _next[i] = _next[k] + 1;
				else _next[i] = 0;
			}
		}
		//_mask = "xxxx?xxx"
		SIZE_T kmp_wc_search(std::vector<BYTE> _code, PBYTE _sig, PBYTE _mask, SIZE_T _sig_size)
		{
			std::vector<int>_next = {};
			kmp_wc_getnext(_next, _sig, _mask, _sig_size);
			auto charMatch = [&](SIZE_T a1, SIZE_T a2) {
				if (_mask[a2] == '?')
					return true;
				return _code[a1] == _sig[a2];
			};
			size_t i, j;
			for (i = j = 0; i < _code.size(); ) {
				if (charMatch(i, j)) 
				{
					++i, ++j;
					if (j >= _sig_size) {  // found patt
						return  (i - _sig_size);
					}
				}
				else {
					if (j == 0) ++i;
					else j = _next[j - 1];
				}
			}
			return -1;
		}
	//public:
	//	SIZE_T kmp_wc_search(PBYTE _Code,SIZE_T _Size,PBYTE _sig, PBYTE _mask, SIZE_T _sig_size)
	//	{
	//		std::vector<BYTE> code = {};
	//		code.resize(_Size);
	//		RtlCopyMemory(&code[0], _Code, _Size);
	//		return kmp_wc_search(code, _sig, _mask, _sig_size);
	//	}
	public:
		//Scan内存的XXX
		//返回地址列表std::vector<DWORD_PTR> m_address?
		bool scan_memory(
			PBYTE _pattern,
			PCHAR _matchMagic,
			SIZE_T _pattern_size,
			DWORD_PTR &find_address)
		{
			std::vector<memory_block> _memblock;
			get_memory_range(_memblock);
			for (auto mem_block : _memblock)
			{
				auto bret = scan_memory(mem_block.Address,
					mem_block.Size,
					_pattern,
					_matchMagic,
					_pattern_size,
					find_address);
				if (bret)
				{
					if (!is_local())
					{
						return true;
					}
					if (find_address != (DWORD_PTR)_pattern)
					{
						return true;
					}
				}
			}
			return false;
		}
		bool scan_memory(PVOID BaseAddress,
			SIZE_T ScanSize,
			PBYTE _pattern,
			PCHAR _matchMagic,
			SIZE_T _pattern_size,
			DWORD_PTR &find_address)
		{
			std::vector<BYTE> block = {};
			if (!read_memory(BaseAddress, ScanSize, block))
			{
				return false;
			}
			auto si = kmp_wc_search(block, _pattern,(PBYTE)_matchMagic,_pattern_size);
			if (si != (SIZE_T)-1)
			{
				find_address = (DWORD_PTR)BaseAddress + si;
				return true;
			}
			return false;
		}
		bool scan_memory(PVOID BaseAddress,
			SIZE_T ScanSize,
			PBYTE _pattern,
			SIZE_T _pattern_size,
			DWORD_PTR &find_address)
		{
			std::vector<BYTE> block = {};
			if (!read_memory(BaseAddress,ScanSize,block))
			{
				return false;
			}
			auto si = kmp_nwc_search(block, _pattern, _pattern_size);
			if (si!=(SIZE_T)-1)
			{
				find_address = (DWORD_PTR)BaseAddress + si;
				return true;
			}
			return false;
		}
		bool scan_memory(PBYTE _pattern,
			SIZE_T _pattern_size,
			DWORD_PTR &find_address)
		{
			std::vector<memory_block> _memblock = {};
			get_memory_range(_memblock);
			for (auto mem_block:_memblock)
			{
				find_address = 0;
				auto bret = scan_memory(mem_block.Address,
					mem_block.Size,
					_pattern,
					_pattern_size,
					find_address);
				if (bret)
				{
					//OutputDebugStringA((char*)find_address);
					if (!is_local())
					{
						return true;
					}
					if (find_address != (DWORD_PTR)_pattern)
					{
						return true;
					}
				//	return true;
				}
			}
			return false;
		}
	public:
		PVOID alloc_mem(SIZE_T _size) {
			if (is_local())
			{
				return VirtualAlloc(nullptr, _size, MEM_COMMIT, PAGE_EXECUTE_READWRITE);
			}
			else
				return VirtualAllocEx(_Process.get(), nullptr, _size, MEM_COMMIT, PAGE_EXECUTE_READWRITE);
		}
		bool write(PVOID _address, PVOID _buffer, SIZE_T _size)
		{
			SIZE_T dwWrite = 0;
			auto bRet = WriteProcessMemory(_Process.get(), _address, _buffer, _size, &dwWrite);
			if (bRet&&dwWrite==_size)
			{
				return true;
			}
			return false;
		}
	};
}
#pragma once
namespace ddk::util
{
	class nt_syscall :public Singleton<nt_syscall>
	{
	public:
		nt_syscall()
		{
			PrevModeOffset = GetPreviousModeOffset();
			build_syscall_table();
		}
		~nt_syscall()
		{

		}
	private:
		//完整load syscall table全部ntdll上的nt函數(Zw不要)
		void build_syscall_table()
		{
			auto NtdllBase = ddk::util::load_dll(std::wstring(L"\\SystemRoot\\System32\\ntdll.dll"));
			auto exit_1 = std::experimental::make_scope_exit([&]() {
				if (NtdllBase)
					ddk::util::free_dll(NtdllBase); });
			auto get_syscall_number = [=](auto FuncRva) 
			{
				if (FuncRva)
				{
					PUCHAR Func = (PUCHAR)NtdllBase + FuncRva;
#ifdef _X86_
					// check for mov eax,imm32
					if (*Func == 0xB8)
					{
						// return imm32 argument (syscall numbr)
						return *(PULONG)((PUCHAR)Func + 1);
					}
#elif _AMD64_
					// check for mov eax,imm32
					if (*(Func + 3) == 0xB8)
					{
						// return imm32 argument (syscall numbr)
						return *(PULONG)(Func + 4);
					}
#endif
				}
				return DWORD(-1);
			};
			if (NtdllBase)
			{
				auto RVATOVA = [](auto _base_, auto _offset_) {
					return ((PUCHAR)(_base_)+(ULONG)(_offset_));
				};
				__try
				{
					PIMAGE_EXPORT_DIRECTORY pExport = NULL;

					PIMAGE_NT_HEADERS32 pHeaders32 = (PIMAGE_NT_HEADERS32)
						((PUCHAR)NtdllBase + ((PIMAGE_DOS_HEADER)NtdllBase)->e_lfanew);

					if (pHeaders32->FileHeader.Machine == IMAGE_FILE_MACHINE_I386)
					{
						// 32-bit image
						if (pHeaders32->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress)
						{
							pExport = (PIMAGE_EXPORT_DIRECTORY)RVATOVA(
								NtdllBase,
								pHeaders32->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress
							);
						}
					}
					else if (pHeaders32->FileHeader.Machine == IMAGE_FILE_MACHINE_AMD64)
					{
						// 64-bit image
						PIMAGE_NT_HEADERS64 pHeaders64 = (PIMAGE_NT_HEADERS64)
							((PUCHAR)NtdllBase + ((PIMAGE_DOS_HEADER)NtdllBase)->e_lfanew);

						if (pHeaders64->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress)
						{
							pExport = (PIMAGE_EXPORT_DIRECTORY)RVATOVA(
								NtdllBase,
								pHeaders64->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress
							);
						}
					}

					if (pExport)
					{
						PULONG AddressOfFunctions = (PULONG)RVATOVA(NtdllBase, pExport->AddressOfFunctions);
						PSHORT AddrOfOrdinals = (PSHORT)RVATOVA(NtdllBase, pExport->AddressOfNameOrdinals);
						PULONG AddressOfNames = (PULONG)RVATOVA(NtdllBase, pExport->AddressOfNames);
						ULONG i = 0;
						for (i = 0; i < pExport->NumberOfFunctions; i++)
						{
							auto func_name = std::string((char *)RVATOVA(NtdllBase, AddressOfNames[i]));
							
							if(func_name.size()>2
								&&func_name.at(0)=='N'&&func_name.at(1)=='t')
							{
								auto syscall_id = get_syscall_number(AddressOfFunctions[AddrOfOrdinals[i]]);
								auto pfn = get_ssdt_function_address(syscall_id);
								if (pfn)
								{
									LOG_DEBUG("load %s %p\r\n", func_name.c_str(), pfn);
									_funcs.insert(std::make_pair(func_name, pfn));
								}		
							}
						}
					}
				}
				__except (EXCEPTION_EXECUTE_HANDLER)
				{

				}
			}
		}
	private:
		ULONG PrevModeOffset;
		std::unordered_map<std::string, PVOID> _funcs;
	public:
		inline PVOID getSysCall(std::string name)
		{
			auto pfn= ddk::util::get_proc_address(name);
			if (pfn)
			{
				_funcs.insert(std::make_pair(name, pfn));
			}
			return pfn;
		}
	public:
		template<typename T>
		inline T get(const std::string& name)
		{
			auto iter = _funcs.find(name);
			if (iter != _funcs.end())
				return reinterpret_cast<T>(iter->second);
			else
			{
				auto pfn = getSysCall(name);
				if (pfn)
				{
					return  reinterpret_cast<T>(pfn);
				}
			}
			return reinterpret_cast<T>(nullptr);
		}
		template<typename T, typename... Args>
		inline NTSTATUS safeNativeCall(const std::string& name, Args&&... args)
		{
			auto pfn = nt_syscall::get<T>(name);
			return pfn ? pfn(std::forward<Args>(args)...) : STATUS_ORDINAL_NOT_FOUND;
		}
		template<typename T, typename... Args>
		inline NTSTATUS safeSysCall(const std::string& name, Args&&... args)
		{
			auto pfn = nt_syscall::get<T>(name);
			PUCHAR pPrevMode = (PUCHAR)PsGetCurrentThread() + GetPreviousModeOffset();
			UCHAR prevMode = *pPrevMode;
			*pPrevMode = KernelMode;
			auto ret = pfn ? pfn(std::forward<Args>(args)...) : STATUS_ORDINAL_NOT_FOUND;
			*pPrevMode = prevMode;
			return ret;
		}
		template<typename T, typename... Args>
		inline auto safeCall(const std::string& name, Args&&... args) -> typename std::result_of<T(Args...)>::type
		{
			auto pfn = nt_syscall::get<T>(name);
			return pfn ? pfn(std::forward<Args>(args)...) : (std::result_of<T(Args...)>::type)(0);
		}
	};
};

#define GET_IMPORT(name) (ddk::util::nt_syscall::getInstance().get<fn ## name>( #name ))
#define SAFE_NATIVE_CALL(name, ...) (ddk::util::nt_syscall::getInstance().safeNativeCall<fn ## name>( #name, __VA_ARGS__ ))
#define SAFE_CALL(name, ...) (ddk::util::nt_syscall::getInstance().safeCall<fn ## name>( #name, __VA_ARGS__ ))
#define SAFE_SYSCALL(name, ...) (ddk::util::nt_syscall::getInstance().safeSysCall<fn ## name>( #name, __VA_ARGS__ ))

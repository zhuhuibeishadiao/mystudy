#pragma once
#include "stdafx.h"
#include "BaseHook.h"

namespace ddk
{
	typedef struct _SERVICE_DESCRIPTOR_TABLE {

		PLONG32 ServiceTable;
		PULONG  CounterTable;
		ULONG   TableSize;
		PUCHAR  ArgumentTable;

	} SERVICE_DESCRIPTOR_TABLE, *PSERVICE_DESCRIPTOR_TABLE;

	typedef struct _SERVICE_DESCRIPTOR_TABLE_SHADOW {
		PLONG32 SsdtServiceTable;
		PULONG  SsdtCounterTable;
		ULONG	SsdtTableSize;
		PUCHAR  SsdtArgumentTable;

		PLONG32 ServiceTable;
		PULONG  CounterTable;
		ULONG	TableSize;
		PUCHAR  ArgumentTable;
	} SERVICE_DESCRIPTOR_TABLE_SHADOW, *PSERVICE_DESCRIPTOR_TABLE_SHADOW;

	class syscall64_hook :public Singleton<syscall64_hook>
	{
	public:
		syscall64_hook()
		{
			install_hook = false;
			new_syscall64 = nullptr;
			old_syscall64 = __readmsr(0xc0000082);
			LOG_DEBUG("%p\r\n", PVOID(old_syscall64));
			install();
		}
	private:
		void install()
		{
			init_syscall64_hook();
			if (install_hook)
			{
				LOG_DEBUG("my syscall64 = %p\r\n", new_syscall64);
				ddk::util::ForEachProcessors(std::bind(&syscall64_hook::hook_syscall64, this, std::placeholders::_1), nullptr);
			}
		}
	private:
		NTSTATUS hook_syscall64(void * context)
		{
			__writemsr(0xc0000082, (ULONG64)new_syscall64);
			return STATUS_SUCCESS;
		}
		NTSTATUS restore_syscall64(void* context)
		{
			__writemsr(0xc0000082, old_syscall64);
			return STATUS_SUCCESS;
		}
	public:
		~syscall64_hook()
		{
			ddk::util::ForEachProcessors(std::bind(&syscall64_hook::restore_syscall64, this, std::placeholders::_1), nullptr);
			if (install_hook)
			{
				if (new_syscall64)
				{
					ExFreePool(new_syscall64);
				}
			}
		}
	private:
		void init_syscall64_hook()
		{
			if (install_hook)
			{
				return;
			}
			new_syscall64 = setup_mysyscall64();
			if (new_syscall64)
			{
				install_hook = true;
			}
			return;
		}
		PVOID setup_mysyscall64()
		{
			PUCHAR pKiSystemCall64 = (PUCHAR)__readmsr(0xc0000082);
			SIZE_T syscall64length = 0;
			ULONG_PTR p;
			if (!pKiSystemCall64)
			{
				LOG_DEBUG("no Syscall64\r\n");
				return nullptr;
			}

			if (!syscall64length)
				syscall64length = 0x1000 - sizeof(SERVICE_DESCRIPTOR_TABLE_SHADOW) * 3;

			p = (ULONG_PTR)ExAllocatePoolWithTag(NonPagedPool, 0x80000, 'nxhk');
			if (!p)
			{
				LOG_DEBUG("no Buffer\r\n");
				return nullptr;
			}
			RtlZeroMemory((PVOID)p, 0x80000);

			RtlCopyMemory((PVOID)p, pKiSystemCall64, syscall64length);
			LOG_DEBUG("p : %p, %p, KiSystemCall64 length = %x\n",
				(PULONG_PTR)p,
				pKiSystemCall64,
				syscall64length);

			pMySsdt = (PSERVICE_DESCRIPTOR_TABLE)(p + 0x1000 - \
				sizeof(SERVICE_DESCRIPTOR_TABLE_SHADOW) * 3);
			pMyShadowSsdt = (PSERVICE_DESCRIPTOR_TABLE_SHADOW)(p + 0x1000 - \
				sizeof(SERVICE_DESCRIPTOR_TABLE_SHADOW) * 2);

			pMyFilter = (PSERVICE_DESCRIPTOR_TABLE)(p + 0x1000 - \
				sizeof(SERVICE_DESCRIPTOR_TABLE_SHADOW));

			Trampoline = (PUCHAR)(p + 0x1000);
			Address = (PULONG_PTR)(p + 0x2000);
			//
			SsdtTable = (PLONG32)(p + 0x3000);
			SsdtTrampoline = p + 0x3000 + 0x1000 * 4;
			SsdtAddress = (PULONG64)(p + 0x3000 + 0x1000 * 4 + 0x1000 * 16);
			//
			ShadowSsdtTable = (PLONG32)((ULONG_PTR)SsdtAddress + 0x1000 * 8);
			ShadowSsdtTrampoline = (ULONG_PTR)ShadowSsdtTable + 0x1000 * 4;
			ShadowSsdtAddress = (PULONG64)(ShadowSsdtTrampoline + 0x1000 * 16);

			if (FixMySyscall64(p, pKiSystemCall64, syscall64length))
			{
				ExFreePool((PVOID)p);
				return nullptr;
			}
			return (PVOID)p;
		}
	private:
		NTSTATUS FixMySyscall64(
			ULONG_PTR pMySysCall64,
			PUCHAR pKiSystemCall64,
			SIZE_T Length)
		{
			// lea r10,[nt!KeServiceDescriptorTable]
			// lea r11,[nt!KeServiceDescriptorTableShadow]
			// lea rdi,[nt!KeServiceDescriptorTableShadow]
			ULONG64 leaR10Offset = 0, leaR11Offset = 0, leaRdiOffset = 0;
			ULONG64 leaR11Offset2 = 0;
			ULONG64 i;

			for (i = 0; i < Length; i++)
			{
				// Fix opcode call
				// Change it to jmp ptr xxxxxxxx
				if (pKiSystemCall64[i] == 0xE8)
				{
					FixOpcodeCallInDriver(
						(ULONG_PTR)pKiSystemCall64,
						pMySysCall64,
						i,
						Address++);
				}

				// Fix FF15 call  qword ptr [fffffa80`010320d0]
				else if (pKiSystemCall64[i] == 0xFF &&
					pKiSystemCall64[i + 1] == 0x15)
				{
					ULONG_PTR p = pMySysCall64 + i;
					LONG32 Rel32 = *(PLONG32)(p + 2);

					*Address = Rel32 + (ULONG_PTR)pKiSystemCall64 + i + 6;
					if (MmIsAddressValid((PVOID)(*Address)))
					{
						LOG_DEBUG("found %p : 0xFF15 call\n", p);
						*Address = *(PULONG_PTR)(*Address);
						*(PLONG32)(p + 2) = (LONG32)((PUCHAR)Address - (PUCHAR)p - 6);
						Address++;
					}
				}

				// Fix opcode cmp rsi, qword ptr [nt!MmUserProbeAddress]
				// Change it to a call xxxxxxxx
				else if (pKiSystemCall64[i] == 0x48 &&
					pKiSystemCall64[i + 1] == 0x3B &&
					pKiSystemCall64[i + 2] == 0x35)
				{
					FixOpcodeCmpRsiInDriver(
						pMySysCall64,
						i,
						Address++);
				}

				// Fix opcode cmovae rsi,qword ptr [nt!MmUserProbeAddress]
				// Change it to call xxxxxxxx
				else if (pKiSystemCall64[i] == 0x48 &&
					pKiSystemCall64[i + 1] == 0x0f &&
					pKiSystemCall64[i + 2] == 0x43 &&
					pKiSystemCall64[i + 3] == 0x35)
				{
					FixOpcodeCmovaeRsiInDriver(
						pMySysCall64,
						i,
						Address++);
				}

				// Fix opcode test dword ptr [nt!PerfGlobalGroupMask],40h
				// Change it to a call xxxxxxxx
				else if (pKiSystemCall64[i] == 0xF7 &&
					pKiSystemCall64[i + 1] == 0x05 &&
					pKiSystemCall64[i + 6] == 0x40)
				{
					FixOpcodeTestInDriver(
						(ULONG_PTR)pKiSystemCall64,
						pMySysCall64,
						i,
						Address++);
				}

				// Fix lea r10,[nt!KeServiceDescriptorTable]
				// and lea r11,[nt!KeServiceDescriptorTableShadow]
				else if (pKiSystemCall64[i] == 0x4c &&
					pKiSystemCall64[i + 1] == 0x8d &&
					pKiSystemCall64[i + 7] == 0x4c &&
					pKiSystemCall64[i + 8] == 0x8d)
				{
					UCHAR *p = (UCHAR *)(pMySysCall64 + i + 14);

					// Save their rel32
					leaR10Offset = i;
					leaR11Offset = i + 7;

					//=======================
					*Address = (ULONG_PTR)(&pKiSystemCall64[i] + 14);

					// jmp qword ptr
					*p = 0xFF;
					*(p + 1) = 0x25;
					*(PLONG32)(p + 2) = (LONG32)((PUCHAR)Address - p - 6);

					Address++;
				}

				// Fix lea rdi,[nt!KeServiceDescriptorTableShadow]
				else if (pKiSystemCall64[i] == 0x48 &&
					pKiSystemCall64[i + 1] == 0x8d &&
					pKiSystemCall64[i + 2] == 0x3d)
				{
					leaRdiOffset = i;
				}
				//Fix lea     r11, KeServiceDescriptorTableFilter
				else if (pKiSystemCall64[i] == 0x4C &&
					pKiSystemCall64[i + 1] == 0x8d &&
					pKiSystemCall64[i + 7] == 0x4d)
				{
					leaR11Offset2 = i;
				}
			}
			if (!leaR11Offset || !leaR10Offset)
			{
				LOG_DEBUG("ssdt table found failed\r\n");
				return STATUS_NOT_FOUND;
			}

			{
				return RebuildSystemServiceTable(
					(ULONG_PTR)pKiSystemCall64,
					pMySysCall64,
					leaR10Offset,
					leaR11Offset,
					leaRdiOffset,
					leaR11Offset2);
			}

			return STATUS_SUCCESS;
		}
	private:
		NTSTATUS RebuiltSsdt(PSERVICE_DESCRIPTOR_TABLE SysSsdt)
		{
			LONG32 Rel32;
			ULONG32 i;

			if (!SysSsdt)
				return STATUS_NOT_FOUND;

			memset(pMySsdt, 0, sizeof(SERVICE_DESCRIPTOR_TABLE_SHADOW));

			// Fill SSDT in driver
			pMySsdt->ServiceTable = SsdtTable;
			// pMySsdt->ServiceTable = SysSsdt->ServiceTable;
			pMySsdt->ArgumentTable = SysSsdt->ArgumentTable;
			pMySsdt->CounterTable = SysSsdt->CounterTable;
			pMySsdt->TableSize = SysSsdt->TableSize;


			//LOG_DEBUG("\n===============SSDT===============\n");

			// Fill ssdt rel32
			for (i = 0; i < SysSsdt->TableSize; i++)
			{
				if (!ddk::util::IsWindowsVistaOrGreater())
					SsdtAddress[i] = ((LONG)SysSsdt->ServiceTable[i] & 0xFFFFFFF0) + (ULONG_PTR)SysSsdt->ServiceTable;
				else
					SsdtAddress[i] = ((LONG)SysSsdt->ServiceTable[i] >> 4) + (ULONG_PTR)SysSsdt->ServiceTable;

				// jmp qword ptr
				*(PUSHORT)SsdtTrampoline = 0x25FF;

				// rel32
				Rel32 = (LONG32)((ULONG_PTR)&SsdtAddress[i] - SsdtTrampoline - 6);
				*(PLONG32)(SsdtTrampoline + 2) = Rel32;

				if (!ddk::util::IsWindowsVistaOrGreater())
					SsdtTable[i] = ((LONG)(SsdtTrampoline - (ULONG_PTR)SsdtTable));
				else
					SsdtTable[i] = ((LONG)(SsdtTrampoline - (ULONG_PTR)SsdtTable)) << 4;

				SsdtTable[i] |= SysSsdt->ServiceTable[i] & 0xF;

				SsdtTrampoline += 0x10;
				//  LOG_DEBUG("%d 0x%llX\n", i, SsdtAddress[i]);
			}

			return STATUS_SUCCESS;
		}

		PEPROCESS LookupAWin32Process()
		{
			PEPROCESS EProcess;
			ULONG pid;

			for (pid = 4; pid < 5000; pid += 4)
			{
				if (PsLookupProcessByProcessId((HANDLE)pid, &EProcess) == STATUS_SUCCESS)
				{
					if (PsGetProcessWin32Process(EProcess))
					{
						return EProcess;
					}
				}
			}

			return NULL;
		}
		NTSTATUS
			RebuildFltSsdt(
				PSERVICE_DESCRIPTOR_TABLE sysFltSsdt
			)
		{
			if (!sysFltSsdt)
			{
				return STATUS_NOT_FOUND;
			}

			pMyFilter->ServiceTable = sysFltSsdt->ServiceTable;
			pMyFilter->ArgumentTable = sysFltSsdt->ArgumentTable;
			pMyFilter->CounterTable = sysFltSsdt->CounterTable;
			pMyFilter->TableSize = sysFltSsdt->TableSize;
			//²»×ö¸´ÖÆ
			return STATUS_SUCCESS;
		}
		NTSTATUS
			RebuiltShadowSsdt(
				PSERVICE_DESCRIPTOR_TABLE_SHADOW SysShadowSsdt)
		{
			PEPROCESS eprocess;
			KAPC_STATE ApcState;
			//
			LONG32 Rel32;
			ULONG32 i;

			if (!SysShadowSsdt)
				return STATUS_NOT_FOUND;

			eprocess = LookupAWin32Process();
			if (!eprocess)
			{
				return STATUS_NOT_FOUND;
			}
			KeStackAttachProcess(eprocess, &ApcState);  // Attach to a GUI process to obtain shadow ssdt address

			pMyShadowSsdt->SsdtServiceTable = SsdtTable;
			//pMyShadowSsdt->SsdtServiceTable = SysShadowSsdt->SsdtServiceTable;
			pMyShadowSsdt->SsdtArgumentTable = SysShadowSsdt->SsdtArgumentTable;
			pMyShadowSsdt->SsdtCounterTable = SysShadowSsdt->SsdtCounterTable;
			pMyShadowSsdt->SsdtTableSize = SysShadowSsdt->SsdtTableSize;
			//
			pMyShadowSsdt->ServiceTable = ShadowSsdtTable;
			//pMyShadowSsdt->ServiceTable = SysShadowSsdt->ServiceTable;
			pMyShadowSsdt->ArgumentTable = SysShadowSsdt->ArgumentTable;
			pMyShadowSsdt->CounterTable = SysShadowSsdt->CounterTable;
			pMyShadowSsdt->TableSize = SysShadowSsdt->TableSize;

			//LOG_DEBUG("\n===============ShadowSSDT===============\n");

			// Fill shadow ssdt rel32
			for (i = 0; i < SysShadowSsdt->TableSize; i++)
			{
				if (!ddk::util::IsWindowsVistaOrGreater())
					ShadowSsdtAddress[i] = ((LONG)SysShadowSsdt->ServiceTable[i] & 0xFFFFFFF0) + (ULONG_PTR)SysShadowSsdt->ServiceTable;
				else
					ShadowSsdtAddress[i] = ((LONG)SysShadowSsdt->ServiceTable[i] >> 4) + (ULONG_PTR)SysShadowSsdt->ServiceTable;

				// jmp qword ptr
				*(PUSHORT)ShadowSsdtTrampoline = 0x25FF;

				// rel32
				Rel32 = (LONG32)((ULONG_PTR)&ShadowSsdtAddress[i] - ShadowSsdtTrampoline - 6);
				*(PLONG32)(ShadowSsdtTrampoline + 2) = Rel32;

				if (!ddk::util::IsWindowsVistaOrGreater())
					ShadowSsdtTable[i] = ((LONG)(ShadowSsdtTrampoline - (ULONG_PTR)ShadowSsdtTable));
				else
					ShadowSsdtTable[i] = ((LONG)(ShadowSsdtTrampoline - (ULONG_PTR)ShadowSsdtTable)) << 4;

				ShadowSsdtTable[i] |= SysShadowSsdt->ServiceTable[i] & 0xF;

				ShadowSsdtTrampoline += 0x10;
				// LOG_DEBUG("%d 0x%llX\n", i, ShadowSsdtAddress[i]);
			}

			KeUnstackDetachProcess(&ApcState);

			return STATUS_SUCCESS;
		}
	private:
		NTSTATUS RebuildSystemServiceTable(
			ULONG_PTR pKiSystemCall64,
			ULONG_PTR pMySysCall64,
			ULONG_PTR leaR10Offset,
			ULONG_PTR leaR11Offset,
			ULONG_PTR leaRdiOffset,
			ULONG_PTR leaR11Offset2)
		{
			LONG32 Rel32;
			ULONG_PTR SysSsdt;
			ULONG_PTR SysShadowSsdt;
			ULONG_PTR FltSsdt;
			// Obtain SSDT address
			Rel32 = *(PLONG32)(pKiSystemCall64 + leaR10Offset + 3);
			SysSsdt = pKiSystemCall64 + leaR10Offset + 7 + Rel32;
			if (Rel32)
			{
				RebuiltSsdt((PSERVICE_DESCRIPTOR_TABLE)SysSsdt);
				Rel32 = (LONG32)((ULONG_PTR)pMySsdt - (pMySysCall64 + leaR10Offset) - 7);
				*(PLONG32)(pMySysCall64 + leaR10Offset + 3) = Rel32;
			}
			else
			{
				return STATUS_NOT_FOUND;
			}

			// and Shadow SSDT
			Rel32 = *(PLONG32)(pKiSystemCall64 + leaR11Offset + 3);
			SysShadowSsdt = pKiSystemCall64 + leaR11Offset + 7 + Rel32;
			if (Rel32)
			{
				RebuiltShadowSsdt((PSERVICE_DESCRIPTOR_TABLE_SHADOW)SysShadowSsdt);

				Rel32 = (LONG32)((ULONG_PTR)pMyShadowSsdt - (pMySysCall64 + leaR11Offset) - 7);
				*(PLONG32)(pMySysCall64 + leaR11Offset + 3) = Rel32;
			}
			else
			{
				return STATUS_NOT_FOUND;
			}

			// Fix lea rdi, 
			if (leaRdiOffset != 0)
			{
				Rel32 = *(PLONG32)(pMySysCall64 + leaRdiOffset + 3);
				if (Rel32)
				{
					Rel32 = (LONG32)((ULONG_PTR)pMyShadowSsdt + 0x20 - (pMySysCall64 + leaRdiOffset) - 7);
					*(PLONG32)(pMySysCall64 + leaRdiOffset + 3) = Rel32;
				}
				else
				{
					return STATUS_NOT_FOUND;
				}
			}
			if (leaR11Offset2 != 0)
			{
				Rel32 = *(PLONG32)(pMySysCall64 + leaR11Offset2 + 3);
				FltSsdt = pKiSystemCall64 + leaR11Offset2 + 7 + Rel32;
				if (Rel32)
				{
					RebuildFltSsdt((PSERVICE_DESCRIPTOR_TABLE)FltSsdt);
					Rel32 = (LONG32)((ULONG_PTR)pMyFilter + 0x20 - (pMySysCall64 + leaR11Offset2) - 7);
					*(PLONG32)(pMySysCall64 + leaR11Offset2 + 3) = Rel32;
				}
				else
				{
					return STATUS_NOT_FOUND;
				}
			}
			return STATUS_SUCCESS;
		}
	private:
		bool install_hook;
		ULONG64 old_syscall64;
		PVOID new_syscall64;
	private:
		VOID FixOpcodeCallInDriver(
			ULONG_PTR pKiSystemCall64,
			ULONG_PTR pMySysCall64,
			ULONG64 Offset,
			PULONG_PTR pAddress)
		{
			ULONG_PTR p = pMySysCall64 + Offset;
			LONG32 Rel32;
			ULONG_PTR TargetAddress;

			LOG_DEBUG("found %p : 0xE8 call\n", p);

			Rel32 = *(PLONG32)(p + 1);

			TargetAddress = Rel32 + pKiSystemCall64 + Offset + 5;

			if (MmIsAddressValid((PVOID)TargetAddress))
			{
				// target - opcode rel32
				*pAddress = TargetAddress;

				// jmp qword ptr
				*Trampoline = 0xFF;
				*(Trampoline + 1) = 0x25;

				Rel32 = (LONG32)((PUCHAR)pAddress - Trampoline - 6);
				*(PLONG32)(Trampoline + 2) = Rel32;

				*(PLONG32)(p + 1) = (LONG32)(Trampoline - (PUCHAR)p - 5);

				Trampoline += 6;
			}
			else
			{
				LOG_DEBUG("From %p Call - MmIsAddressValid() failed : %p\n", p, TargetAddress);
			}
		}
		VOID FixOpcodeCmpRsiInDriver(
			ULONG_PTR pMySysCall64,
			ULONG64 Offset,
			PULONG_PTR pAddress)
		{
			ULONG_PTR p = pMySysCall64 + Offset;
			LONG32 Rel32;

			LOG_DEBUG("found %p : cmp rsi, qword ptr [nt!MmUserProbeAddress]\n", p);

			// push rax
			*Trampoline = 0x50;
			//mov rax, qword ptr
			*(PULONG_PTR)(Trampoline + 1) = 0x058B48;

			// rel32
			*pAddress = (ULONG_PTR)&MmUserProbeAddress;
			Rel32 = (LONG32)((PUCHAR)pAddress - Trampoline - 8);
			*(PLONG32)(Trampoline + 4) = Rel32;

			// cmp rsi,qword ptr [rax]
			*(PULONG_PTR)(Trampoline + 8) = 0x303B48;
			// pop rax
			*(PUCHAR)(Trampoline + 11) = 0x58;
			// ret
			*(PUCHAR)(Trampoline + 12) = 0xC3;

			// call
			*(PUCHAR)p = 0xE8;
			*(PLONG32)(p + 1) = (LONG32)(Trampoline - (PUCHAR)p - 5);

			// nop 2 bytes
			*(PUSHORT)(p + 5) = 0x9090;

			Trampoline += 13;
		}
		VOID FixOpcodeCmovaeRsiInDriver(
			ULONG_PTR pMySysCall64,
			ULONG64 Offset,
			PULONG_PTR pAddress)
		{
			ULONG_PTR p = pMySysCall64 + Offset;
			LONG32 Rel32;

			LOG_DEBUG("found %p : cmovae rsi,qword ptr [nt!MmUserProbeAddress]\n", p);

			// push rax
			*Trampoline = 0x50;
			//mov rax, qword ptr
			*(PULONG_PTR)(Trampoline + 1) = 0x058B48;

			// rel32
			*pAddress = (ULONG_PTR)&MmUserProbeAddress;
			Rel32 = (LONG32)((PUCHAR)pAddress - Trampoline - 8);
			*(PLONG32)(Trampoline + 4) = Rel32;

			// cmovae  rsi,qword ptr [rax]
			*(PULONG_PTR)(Trampoline + 8) = 0x30430F48;
			// pop rax
			*(PUCHAR)(Trampoline + 12) = 0x58;
			// ret
			*(PUCHAR)(Trampoline + 13) = 0xC3;

			// call
			*(PUCHAR)p = 0xE8;
			*(PLONG32)(p + 1) = (LONG32)(Trampoline - (PUCHAR)p - 5);

			// nop 3 bytes
			*(PUSHORT)(p + 5) = 0x9090;
			*(PUCHAR)(p + 7) = 0x90;

			Trampoline += 14;
		}
		VOID FixOpcodeTestInDriver(
			ULONG_PTR pKiSystemCall64,
			ULONG_PTR pMySysCall64,
			ULONG64 Offset,
			PULONG_PTR pAddress)
		{
			ULONG_PTR p = pMySysCall64 + Offset;
			LONG32 Rel32;

			LOG_DEBUG("found %p : test dword ptr [nt!PerfGlobalGroupMask],40h\n", p);

			Rel32 = *(PULONG32)(p + 2);

			// push rax
			*Trampoline = 0x50;
			// mov rax, qword ptr
			*(PULONG_PTR)(Trampoline + 1) = 0x058B48;

			// rel32
			*pAddress = pKiSystemCall64 + Offset + 10 + Rel32;
			Rel32 = (LONG32)((PUCHAR)pAddress - Trampoline - 8);
			*(PLONG32)(Trampoline + 4) = Rel32;

			// test dword ptr [rax],40h
			*(PULONG_PTR)(Trampoline + 8) = 0x4000F7;
			*(PULONG_PTR)(Trampoline + 11) = 0;
			// pop rax
			*(PUCHAR)(Trampoline + 14) = 0x58;
			// ret
			*(PUCHAR)(Trampoline + 15) = 0xC3;

			// call
			*(PUCHAR)p = 0xE8;
			*(PLONG32)(p + 1) = (LONG32)(Trampoline - (PUCHAR)p - 5);

			// nop 5 bytes
			*(PULONG32)(p + 5) = 0x90909090;
			*(PUCHAR)(p + 9) = 0x90;

			Trampoline += 16;
		}
	private:
		PSERVICE_DESCRIPTOR_TABLE pMySsdt;
		PSERVICE_DESCRIPTOR_TABLE_SHADOW pMyShadowSsdt;
		PSERVICE_DESCRIPTOR_TABLE pMyFilter;
		PUCHAR Trampoline;
		PULONG_PTR Address;
		PLONG32   SsdtTable;
		ULONG_PTR SsdtTrampoline;        // FF25 XXXXXXXX
		PULONG64  SsdtAddress;
		PLONG32   ShadowSsdtTable;
		ULONG_PTR ShadowSsdtTrampoline;  // FF25 XXXXXXXX
		PULONG64  ShadowSsdtAddress;
	public:
		ULONG64 GetSyscall64() {
			return old_syscall64;
		}
		PVOID getNewSyscall64()
		{
			return new_syscall64;
		}
	public:
		bool hook_ssdt(ULONG Index, PVOID NewFunction, PVOID *OldFunction)
		{
			if (!install_hook)
			{
				return false;
			}
			auto _idx = Index & 0xFFF;
			PVOID _old = nullptr;
			KIRQL old_irql;
			ddk::cpu_lock _lock;
			_lock.lock();
			old_irql = KeRaiseIrqlToDpcLevel();
			if (Index >= 0x1000)
			{
				_old = PVOID(ShadowSsdtAddress[_idx]);
				ShadowSsdtAddress[_idx] = (ULONG64)NewFunction;
			}
			else
			{
				_old = PVOID(SsdtAddress[_idx]);
				SsdtAddress[_idx] = (ULONG64)NewFunction;
			}
			if (OldFunction)
			{
				*OldFunction = _old;
			}
			KeLowerIrql(old_irql);
			_lock.unlock();
			return true;
		}
	};
};
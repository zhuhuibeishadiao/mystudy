#include "stdafx.h"
#include "BaseHook.h"
#ifdef _WIN64
namespace Detours
{
	namespace X64
	{
		#define HEADER_MAGIC		'@D64'

		#define MAX_INSTRUCT_SIZE	0x100

		#define RAX_JUMP_LENGTH_64	0x0C	// mov rax, <addr>; jmp rax;
		#define PUSH_RET_LENGTH_64	0x0E	// push <low 32 addr>; mov [rsp+4h], <hi 32 addr>; ret;

		#define ALIGN_64(x)			Internal::AlignAddress((uint64_t)(x), 16);

		
		uint8_t *DetourFunction(uint8_t *Target, uint8_t *Detour, X64Option Options)
		{
			// Decode the actual assembly
			_DecodedInst decodedInstructions[DISASM_MAX_INSTRUCTIONS];
			uint32_t decodedInstructionsCount = 0;

			//distorm_decode64不能满足reasm问题解决
			_DecodeResult res = distorm_decode64((_OffsetType)Target, (const unsigned char *)Target, 32, Decode64Bits, decodedInstructions, DISASM_MAX_INSTRUCTIONS, &decodedInstructionsCount);

			// Check if decoding failed
			if (res != DECRES_SUCCESS)
				return nullptr;

			// Calculate the hook length from options
			uint32_t totalInstrSize = 0;
			uint32_t neededSize		= DetourGetHookLength(Options);
			uint32_t copyInsCount = 0;
			// Calculate how many instructions are needed to place the jump
			for (uint32_t i = 0; i < decodedInstructionsCount; i++)
			{
				totalInstrSize += decodedInstructions[i].size;
				copyInsCount++;
				if (totalInstrSize >= neededSize)
					break;
			}

			// Unable to find a needed length
			if (totalInstrSize < neededSize)
				return nullptr;

			// Allocate the trampoline data
			uint32_t allocSize = 0;
			allocSize += sizeof(JumpTrampolineHeader);	// Base structure
			allocSize += totalInstrSize;				// Size of the copied instructions
			allocSize += MAX_INSTRUCT_SIZE;				// See PUSH_RET_LENGTH_64
			allocSize += MAX_INSTRUCT_SIZE;				// See PUSH_RET_LENGTH_64
			allocSize += 0x64;							// Padding for any memory alignment

			/*ddk::mem_util::MM_SYSTEM_ALLOC _allocMem = {};
			if (!ddk::mem_util::alloc_mem((PVOID)Target, allocSize, &_allocMem))
			{
				return nullptr;
			}*/
			auto MapAddress = ddk::mem_util::MmAllocMemoryForHook((PVOID)Target, allocSize);
			if (!MapAddress)
			{
				return nullptr;
			}
			uint8_t *jumpTrampolinePtr = (uint8_t *)MapAddress;

			LOG_DEBUG("allocate %p\r\n", jumpTrampolinePtr);
			if (!jumpTrampolinePtr)
				return nullptr;

			// Fill out the header
			JumpTrampolineHeader *header = (JumpTrampolineHeader *)jumpTrampolinePtr;

			header->Magic				= HEADER_MAGIC;
			header->Random				= uint32_t((DWORD64)PsGetCurrentProcessId() + (DWORD64)PsGetCurrentThreadId() + totalInstrSize);

			//header->hSection = _allocMem.hSection;
			//header->MapAddress = _allocMem.MapAddress;
			//header->SectionObject = _allocMem.SectionObject;

			header->CodeOffset			= Target;
			header->DetourOffset		= Detour;

			header->InstructionLength	= totalInstrSize;
			header->InstructionOffset	= ALIGN_64(jumpTrampolinePtr + sizeof(JumpTrampolineHeader));

			header->TrampolineLength	= MAX_INSTRUCT_SIZE;
			header->TrampolineOffset	= ALIGN_64(header->InstructionOffset + header->InstructionLength + MAX_INSTRUCT_SIZE + header->TrampolineLength);

			// Copy the fixed instructions over
			//memcpy(header->InstructionOffset, Target, header->InstructionLength);
			// Apply any changes because of the different IP
			if(1)
			{
				header->backup_size = totalInstrSize;
				memcpy(header->backup_code, Target, totalInstrSize);

				BYTE *data = Reassembler::Reassemble64((uint64_t)header->InstructionOffset, decodedInstructions, copyInsCount, &header->InstructionLength);

				if (!data)
				{
					//ExFreePool(jumpTrampolinePtr, 0, MEM_RELEASE);
					return nullptr;
				}
				// Copy the fixed instructions over
				memcpy(header->InstructionOffset, data, header->InstructionLength);
				//memcpy(header->InstructionOffset, Target, header->InstructionLength);

				// Free unneeded data
				Reassembler::Free(data);
			}
			// Write the assembly in the allocation block
			DetourWriteStub(header);

			bool result = true;

			switch (Options)
			{
			case X64Option::USE_RAX_JUMP:	result = DetourWriteRaxJump(header);break;
			case X64Option::USE_PUSH_RET:	result = DetourWritePushRet(header);break;
			default:						result = false;						break;
			}

			// If an operation failed free the memory and exit
			if (!result)
			{
				//VirtualFree(jumpTrampolinePtr, 0, MEM_RELEASE);
				return nullptr;
			}

			// Force flush any possible CPU cache
			Internal::FlushCache(Target, totalInstrSize);
			Internal::FlushCache(jumpTrampolinePtr, allocSize);

			// Set read/execution on the page
			//DWORD dwOld = 0;
			//VirtualProtect(jumpTrampolinePtr, allocSize, PAGE_EXECUTE_READ, &dwOld);

			return header->InstructionOffset;
		}

		bool DetourRemove(uint8_t *Trampoline)
		{
			JumpTrampolineHeader *header = (JumpTrampolineHeader *)(Trampoline - sizeof(JumpTrampolineHeader));

			if (header->Magic != HEADER_MAGIC)
				return false;

			// Rewrite the backed-up code
			if (!Internal::WriteMemory(header->CodeOffset, header->backup_code, header->backup_size))
			{
				LOG_DEBUG("Failed WriteMemory\r\n");
				return false;
			}
			Internal::FlushCache(header->CodeOffset, header->InstructionLength);
			//VirtualFree(header, 0, MEM_RELEASE);

			/*ddk::mem_util::MM_SYSTEM_ALLOC _allocmem = {};
			_allocmem.hSection = header->hSection;
			_allocmem.MapAddress = header->MapAddress;
			_allocmem.SectionObject = header->SectionObject;

			ddk::mem_util::free_alloc(&_allocmem);*/

			return true;
		}

		uint8_t *DetourVTable(uint8_t *Target, uint8_t *Detour, uint32_t TableIndex)
		{
			// Each function is stored in an array - also get a copy of the original
			uint8_t *virtualPointer = (Target + (TableIndex * sizeof(ULONGLONG)));
			uint8_t *original		= *(uint8_t **)virtualPointer;

			if (!Internal::AtomicCopy4X8(virtualPointer, Detour, sizeof(ULONGLONG)))
				return nullptr;

			return original;
		}

		bool VTableRemove(uint8_t *Target, uint8_t *Function, uint32_t TableIndex)
		{
			// Reverse VTable detour
			return DetourVTable(Target, Function, TableIndex) != nullptr;
		}

		uint8_t *DetourIAT(uint8_t *TargetModule, uint8_t *Detour, const char *ImportModule, const char *API)
		{
			return Internal::IATHook(TargetModule, ImportModule, API, Detour);
		}

		void DetourWriteStub(JumpTrampolineHeader *Header)
		{
			/********** Allocated code block modifications **********/

			// Determine where the 'unhooked' part of the function starts
			uint8_t *unhookStart = (Header->CodeOffset + Header->InstructionLength);

			// Jump to hooked function (Backed up instructions)
			uint8_t *binstr_ptr = (Header->InstructionOffset + Header->InstructionLength);

			AsmGen hookGen(binstr_ptr, ASMGEN_64);
			hookGen.AddCode("push 0x%X",						((uint64_t)unhookStart) & 0xFFFFFFFF);
			hookGen.AddCode("mov dword ptr ss:[rsp+0x4], 0x%X", ((uint64_t)unhookStart) >> 32);
			hookGen.AddCode("ret");
			hookGen.WriteStreamTo(binstr_ptr);

			// Jump to user function (Write the trampoline)
			AsmGen userGen(Header->TrampolineOffset, ASMGEN_64);
			userGen.AddCode("push 0x%X",						((uint64_t)Header->DetourOffset) & 0xFFFFFFFF);
			userGen.AddCode("mov dword ptr ss:[rsp+0x4], 0x%X", ((uint64_t)Header->DetourOffset) >> 32);
			userGen.AddCode("ret");
			userGen.WriteStreamTo(Header->TrampolineOffset);
		}

		bool DetourWriteRaxJump(JumpTrampolineHeader *Header)
		{
			AsmGen gen(Header->CodeOffset, ASMGEN_64);

			// Jump to trampoline (from hooked function)
			gen.AddCode("mov rax, 0x%llx", Header->TrampolineOffset);
			gen.AddCode("jmp rax");

			return Internal::WriteMemory(Header->CodeOffset, gen.GetStream(), gen.GetStreamLength());
		}

		bool DetourWritePushRet(JumpTrampolineHeader *Header)
		{
			AsmGen gen(Header->CodeOffset, ASMGEN_64);

			// Jump with PUSH/RET (push low32, [rsp+4h] = hi32)
			gen.AddCode("push 0x%X",						((uint64_t)Header->TrampolineOffset) & 0xFFFFFFFF);
			gen.AddCode("mov dword ptr ss:[rsp+0x4], 0x%X", ((uint64_t)Header->TrampolineOffset) >> 32);
			gen.AddCode("ret");

			return Internal::WriteMemory(Header->CodeOffset, gen.GetStream(), gen.GetStreamLength());
		}

		uint32_t DetourGetHookLength(X64Option Options)
		{
			uint32_t size = 0;

			switch(Options)
			{
			case X64Option::USE_RAX_JUMP:	size += RAX_JUMP_LENGTH_64;	break;
			case X64Option::USE_PUSH_RET:	size += PUSH_RET_LENGTH_64;	break;
			default:						size = 0;					break;
			}

			return size;
		}
	}
}
#endif // _WIN64
#include "stdafx.h"

namespace ddk::ntos
{
	KDDEBUGGER_DATA64 _kdblock = {};
	PDWORD64 KiWaitNever_ = nullptr;
	PBYTE KdpDataBlockEncoded_ = nullptr;
	PDWORD64 KiWaitAlways_ = nullptr;
	bool init_kdbgblock = false;
	
	DWORD64 uncipherData(DWORD64 data, 
		DWORD64 KiWaitNever,
		DWORD64 KiWaitAlways,
		DWORD64 KdpDataBlockEncoded)
	{
		data = data^KiWaitNever;
		data = RotateLeft64(data, KiWaitNever & 0xFF);
		data = data^KdpDataBlockEncoded;
		data = RtlUlonglongByteSwap(data);
		data = data^KiWaitAlways;
		return data;
	}

	PKDDEBUGGER_DATA64 get_kdblock()
	{
		if (init_kdbgblock)
			return &_kdblock;

		auto block_size = 0;
		switch (OsIndex)
		{
		case OsIndex_7:
			block_size = 0x340;
			break;
		case OsIndex_8:
		case OsIndex_BLUE:
		case OsIndex_10_1511:
		case OsIndex_10_1507:
			block_size = 0x360;
			break;
		case OsIndex_10_1607:
		case OsIndex_10_1707:
			block_size = 0x368;
			break;
		default:
			return nullptr;
			break;
		}
		PVOID KdDebuggerDataBlock_ = nullptr;
		KiWaitNever_ = nullptr;
		KdpDataBlockEncoded_ = nullptr;
		KiWaitAlways_ = nullptr;
		UCHAR Win7_pattern[] = { 0x49,0x8b,0x00,0x41,0x8B,0xCA,0x49,0x33,0xC2,0x48,0xD3,0xC0 };
		UCHAR Win81_pattern[] = { 0x48,0xD3,0xC2,0x48,0x33,0xD0,0x48,0x0F,0xCA };
		UCHAR Win80_pattern[] = { 0x48,0x8B,0x02,0x41,0x8B,0xCA,0x49,0x33,0xC2,0x48,0xD3,0xC0 };
		UCHAR Win10_pattern[] = { 0x48,0xD3,0xC2,0x48,0x33,0xD0,0x48,0x0F,0xCA };
		PUCHAR p_pattern = nullptr;
		SIZE_T size_pattern = 0;
		LONG off_KdDDB = -19;
		LONG off_KiWN = 0;
		LONG off_KiWA = 0;
		LONG off_KdpDBE = 0;
		auto p_func = PVOID(nullptr);
		switch (*NtBuildNumber)
		{
		case 7600:
		case 7601:
			//Windows 7从KdChangeOption开始搜索
			p_func = ddk::util::get_proc_address("KdChangeOption");
			p_pattern = Win7_pattern;
			size_pattern = sizeof(Win7_pattern);
			off_KdDDB = -13;
			off_KiWN = -20;
			off_KiWA = 28;
			off_KdpDBE = 15;
			break;
		case 9200:
			p_func = ddk::util::get_proc_address("KdChangeOption");
			p_pattern = Win80_pattern;
			size_pattern = sizeof(Win80_pattern);
			off_KdDDB = -29;
			off_KiWN = -20;
			off_KiWA = -13;
			off_KdpDBE = 15;
			break;
		case 9600:
			p_func = ddk::util::get_proc_address("KdDeregisterPowerHandler");
			p_pattern = Win81_pattern;
			size_pattern = sizeof(Win81_pattern);
			off_KdDDB = -37;
			off_KiWN = -19;
			off_KiWA = 12;
			off_KdpDBE = -4;
			break;
		case 10586:
		case 14393:
		case 15063:
			p_func = ddk::util::get_proc_address("KdDeregisterPowerHandler");
			p_pattern = Win10_pattern;
			size_pattern = sizeof(Win10_pattern);
			off_KdDDB = -40;
			off_KiWN = -19;
			off_KiWA = 12;
			off_KdpDBE = -4;
			break;
		}
		if (!p_func || !p_pattern)
		{
			return nullptr;
		}
		auto ns = ddk::mem_util::MmGenericPointerSearch(
			(PUCHAR *)&KdDebuggerDataBlock_,
			((PUCHAR)p_func) - (1 * PAGE_SIZE),
			((PUCHAR)p_func) + (1 * PAGE_SIZE),
			p_pattern,
			size_pattern,
			off_KdDDB);
		if (!NT_SUCCESS(ns))
		{
			return nullptr;
		}
		ns = ddk::mem_util::MmGenericPointerSearch(
			(PUCHAR *)&KdpDataBlockEncoded_,
			((PUCHAR)p_func) - (1 * PAGE_SIZE),
			((PUCHAR)p_func) + (1 * PAGE_SIZE),
			p_pattern,
			size_pattern,
			off_KdpDBE);
		if (!NT_SUCCESS(ns))
		{
			return nullptr;
		}
		ns = ddk::mem_util::MmGenericPointerSearch(
			(PUCHAR *)&KiWaitAlways_,
			((PUCHAR)p_func) - (1 * PAGE_SIZE),
			((PUCHAR)p_func) + (1 * PAGE_SIZE),
			p_pattern,
			size_pattern,
			off_KiWA);
		if (!NT_SUCCESS(ns))
		{
			return nullptr;
		}
		ns = ddk::mem_util::MmGenericPointerSearch(
			(PUCHAR *)&KiWaitNever_,
			((PUCHAR)p_func) - (1 * PAGE_SIZE),
			((PUCHAR)p_func) + (1 * PAGE_SIZE),
			p_pattern,
			size_pattern,
			off_KiWN);
		if (!NT_SUCCESS(ns))
		{
			return nullptr;
		}
		RtlCopyMemory(&_kdblock, KdDebuggerDataBlock_, block_size);
		if (*KdpDataBlockEncoded_)
		{
			LOG_DEBUG("need decode\r\n");
			LOG_DEBUG("kwaitalways %p kwaitnever %p\r\n", KiWaitAlways_, KiWaitNever_);
			//需要解密
			for (int i = 0; i < block_size / 8; i++) {
				auto tmpEncodedData = ((DWORD64*)KdDebuggerDataBlock_)[i];
				((DWORD64*)&_kdblock)[i] = uncipherData(tmpEncodedData, *KiWaitNever_, *KiWaitAlways_, (DWORD64)KdpDataBlockEncoded_);
			}
		}
		init_kdbgblock = true;
		return &_kdblock;
	}
}
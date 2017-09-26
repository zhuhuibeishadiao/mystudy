#pragma once
#include "stdafx.h"

//win2k-win7 搜索方案 或者 时光倒流dispg1.0大法
//win8-win10 物理内存-->VA-->属于无模块的都PTE或者PDE的NX=1,pf大法
//特征码也很固定,如果cr2==rip，
//cmp dword ptr[cr2],48513148h
//cmp qword ptr[cr2],85131481131482Eh,
//如果是则改rip到 一个retn上去，pg就没了，就没了
//不是则恢复PTE或者PDE的NX
namespace ddk::patchguard
{
	typedef struct _POOL_TRACKER_BIG_PAGES // 4 elements, 0x18 bytes (sizeof) 
	{
		/*0x000*/     VOID*        Va;
		/*0x008*/     ULONG32      Key;
		/*0x00C*/     ULONG32      PoolType;
		/*0x010*/     UINT64       NumberOfBytes;
	}POOL_TRACKER_BIG_PAGES, *PPOOL_TRACKER_BIG_PAGES;

	void DisPg();
	void Disable_Early();
	void dispg_new();
}
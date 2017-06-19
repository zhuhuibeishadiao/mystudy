// MemoryAPI.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"


/***********堆的各项操作**********************/
void CreateReHeapDemo()
{
	SYSTEM_INFO si;
	GetSystemInfo(&si);
	printf("系统内存页大小：0x%x\n系统分配粒度：0x%x\n", si.dwPageSize, si.dwAllocationGranularity);
	//方式1：获得堆
	auto h_Heap1 = HeapCreate(HEAP_NO_SERIALIZE, si.dwPageSize, si.dwPageSize * 10);
	printf("创建了一个堆，初始化大小为1页，最大为10页\n");

	//方式2：获得堆
	auto h_Heap2 = GetProcessHeap();
	printf("获得当前进程的一个堆\n");

	auto h_Heap3 = HeapCreate(HEAP_NO_SERIALIZE, 0, 0);
	printf("创建增长无上限的堆\n");

	//方式3：获得进程中所有的堆
	HANDLE h_Heaps[10] = {};
	auto dwHeapCount = GetProcessHeaps(10, h_Heaps);
	printf("系统中一共有%d个堆\n", dwHeapCount);

	//从堆中分配内存
	auto lpMem = HeapAlloc(h_Heap1, HEAP_ZERO_MEMORY, si.dwPageSize * 3);
	auto lpMem1 = HeapAlloc(h_Heap1, HEAP_ZERO_MEMORY, si.dwPageSize);
	if (lpMem)
		printf("在堆上成功分配内存，起始位置是:0x%x\n", lpMem);
	memset(lpMem, 0x22, 3 * si.dwPageSize);
	memset(lpMem1, 0x33, si.dwPageSize);
	
	//获得某块大小
	SIZE_T size = HeapSize(h_Heap1, HEAP_NO_SERIALIZE, lpMem);
	printf("lpMem大小%x\n", size);
	size = HeapSize(h_Heap1, HEAP_NO_SERIALIZE, lpMem1);
	printf("lpMem1大小%x\n", size);

	//重新定义大小
	auto lpReAlloc = HeapReAlloc(h_Heap1, HEAP_ZERO_MEMORY, lpMem1, si.dwPageSize * 3);

	//释放堆，对象HeapAlloc, HeapReAlloc
	HeapFree(h_Heap1, HEAP_NO_SERIALIZE, lpMem);
	//HeapFree(h_Heap1, HEAP_NO_SERIALIZE, lpMem1);执行这句话出错，因为已经重新分配，并不需要释放了
	HeapFree(h_Heap1, HEAP_NO_SERIALIZE, lpReAlloc);
	//销毁堆
	HeapDestroy(h_Heap1), HeapDestroy(h_Heap3);

}
/************************************************************/

void TestHeapDemo()
{
	CreateReHeapDemo();
}


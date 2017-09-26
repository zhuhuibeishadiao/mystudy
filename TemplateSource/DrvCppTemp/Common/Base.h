#pragma once
#pragma warning(disable:4018)
#pragma warning(disable:4242)
//#pragma warning(disable:4505)
#define _CRT_ALLOCATION_DEFINED
#define NDIS61 1
#define NDIS_SUPPORT_NDIS61 1

#if defined(DBG)
#define LOG_BUILD 1//加上這個build速度很慢
#endif
#ifdef __cplusplus
#ifndef _HAS_EXCEPTIONS
#define _HAS_EXCEPTIONS 0
#endif

#define FLT_MGR_LEGACY_PUSH_LOCKS
#define NTSTRSAFE_LIB
#define NTSTRSAFE_NO_CB_FUNCTIONS

extern "C"
{
#pragma warning(push, 0)
#include <initguid.h>
#include <fltKernel.h>
#include <Wdmsec.h>
#include <ntdef.h>
#include <ntimage.h>
#include <stdarg.h>
#include <ntstrsafe.h>
#include <ntdddisk.h>
//#include <ntddstor.h>
#include <mountdev.h>
#include <ntddvol.h>
#include <intrin.h>
#include <Aux_klib.h>
#include <wdmguid.h>
#pragma warning(pop)
#include <ntifs.h>
#include <fwpmk.h>
#pragma warning(push)
#pragma warning(disable:4201)       // unnamed struct/union
#include <fwpsk.h>
#pragma warning(pop)
#include <ntddkbd.h>
#include <ntddscsi.h>
#include <srb.h>
#include <scsi.h>
#include <wsk.h>

#include <basetsd.h>
};
#else
//C的头
#include <ntifs.h>
#include <ntdef.h>
#include <stdarg.h>
#include <stdio.h>
#include <wchar.h>
#include <ntddscsi.h>
#include <srb.h>
#include <ntimage.h>
#include <aux_klib.h>
#include <ntstrsafe.h>
#include "ddk_stdint.h"
#endif

#ifdef __cplusplus
#include "../Common/stdcpp/stdcpp.h"
#include "../Common/stdcpp/kernel_stl.h"
#include "../Common/stdcpp/unique_resource.h"
#include "../Common/stdcpp/scope_exit.h"
#include "../Common/stdcpp/Singleton.h"
#include "../Common/intel_def.h"
//STL for ddk
#include <tuple>
#include <map>
#include <vector>
#include <string>
#include <algorithm>
#include <random>
#include <array>
#include <memory>
#include <list>
#include <deque>
#include <functional>
#include <regex>
#include <utility>
#include <memory>
#include <unordered_map>
#include <sstream>
#include <atomic>
#include <set>
#endif


#define INOUT 
#ifdef ALLOC_PRAGMA
#define ALLOC_TEXT(Section, Name) __pragma(alloc_text(Section, Name))
#else
#define ALLOC_TEXT(Section, Name)
#endif
// _countof. You do not want to type RTL_NUMBER_OF, do you?
#ifndef _countof
#define _countof(x)    RTL_NUMBER_OF(x)
#endif

// Returns true when it is running on the x64 system.
inline bool IsX64() {
#ifdef _AMD64_
	return true;
#else
	return false;
#endif
}
// Break point that works only when a debugger is attached.
#ifndef DBG_BREAK
#ifdef _ARM_
// Nullify it since an ARM device never allow us to attach a debugger.
#define DBG_BREAK()
#else
#define DBG_BREAK()               \
  if (KD_DEBUGGER_NOT_PRESENT) {  \
          } else {                        \
    __debugbreak();               \
          }                               \
  reinterpret_cast<void *>(0)
#endif
#endif

//0x80070000 
//#define STATUS_CUSTOM_STATUS(x) 0x80070000+x 不出错误框
//需要管理员处理才能工作 STATUS_DOWNGRADE_DETECTED
//恶意软件通报 STATUS_VIRUS_INFECTED

#if defined(DBG)
#define LOG_DEBUG(format, ...) \
	DbgPrintEx(DPFLTR_DEFAULT_ID, DPFLTR_ERROR_LEVEL, (format), __VA_ARGS__)
#define LOG_INFO(format, ...) \
	DbgPrintEx(DPFLTR_DEFAULT_ID, DPFLTR_ERROR_LEVEL, (format), __VA_ARGS__)
#define LOG_WARN(format, ...) \
	DbgPrintEx(DPFLTR_DEFAULT_ID, DPFLTR_ERROR_LEVEL, (format), __VA_ARGS__)
#define LOG_ERROR(format, ...) \
	DbgPrintEx(DPFLTR_DEFAULT_ID, DPFLTR_ERROR_LEVEL, (format), __VA_ARGS__)
#define LOG_DEBUG_SAFE(format, ...) \
	DbgPrintEx(DPFLTR_DEFAULT_ID, DPFLTR_ERROR_LEVEL, (format), __VA_ARGS__)
#define LOG_INFO_SAFE(format, ...) \
	DbgPrintEx(DPFLTR_DEFAULT_ID, DPFLTR_ERROR_LEVEL, (format), __VA_ARGS__)                                       
#define LOG_WARN_SAFE(format, ...) \
	DbgPrintEx(DPFLTR_DEFAULT_ID, DPFLTR_ERROR_LEVEL, (format), __VA_ARGS__)                                         
#define LOG_ERROR_SAFE(format, ...) \
	DbgPrintEx(DPFLTR_DEFAULT_ID, DPFLTR_ERROR_LEVEL, (format), __VA_ARGS__)
#else
#define LOG_DEBUG(format, ...)
#define LOG_DEBUG_SAFE(format, ...)   
#define LOG_INFO_SAFE(format, ...)                                         
#define LOG_WARN_SAFE(format, ...)                                         
#define LOG_ERROR_SAFE(format, ...)
#endif

template<typename T, typename U> size_t offsetOf(U T::*member)
{
	return (char*)&((T*)nullptr->*member) - (char*)nullptr;
}


typedef enum _OS_INDEX {
	OsIndex_UNK = 0,
	OsIndex_XP = 1,
	OsIndex_2K3 = 2,
	OsIndex_VISTA = 3,
	OsIndex_7 = 4,
	OsIndex_8 = 5,
	OsIndex_BLUE = 6,
	OsIndex_10_1507 = 7,
	OsIndex_10_1511 = 8,
	OsIndex_10_1607 = 9,
	OsIndex_10_1707 = 10,
	OsIndex_MAX = 11,
} OS_INDEX, *POS_INDEX;

extern OS_INDEX OsIndex;
extern PDRIVER_OBJECT g_pDriverObject;
extern PVOID g_pDrvImageBase;
extern SIZE_T g_DrvImageSize;
extern PLIST_ENTRY PsLoadedModuleList;
extern wchar_t g_DriverRegistryKey[MAX_PATH];

typedef const BYTE *LPCBYTE;



#ifdef __cplusplus
//Status
#include "../Common/nt_status.h"
//A2W W2A
#include "../Common/string/nt_string.h"
//ntos函數定義
#include "../Common/ntos/ntos_func_def.h"
//結構定義
#include "../Common/ntos/ntos.h"
//Lock
#include "../Common/Lock/nt_lock.h"
#include "../Common/Lock/nt_mutex.h"
#include "../Common/Lock/nt_rwlock.h"
#include "../Common/Lock/nt_spinlock.h"
#include "../Common/Lock/push_lock.h"
#include "../Common/Lock/nt_rundownlock.h"
//cpulock
#include "../Common/Lock/cpu_lock.h"
//util
#include "../Common/util/util.h"
//Thread
#include "../Common/thread/nt_thread.h"
//WorkItem
#include "../Common/workitem/nt_work_item.h"
//DPC IPI TIMER
#include "../Common/dpc/nt_dpc.h"
#include "../Common/dpc/nt_dpc_help.h"
#include "../Common/timer/nt_timer.h"
#include "../Common/ipi/nt_ipi.h"
//File
#include "../Common/file/nt_file.h"
//File Mapping
#include "../Common/section/nt_file_map.h"
//Event
#include "../Common/event/nt_event.h"
#include "../Common/event/nt_kevent.h"
//Register
#include "../Common/reg/nt_reg.h"
//Hive
#include "../Common/hive/nt_hive.h"
//Driver
#include "../Common/Driver/nt_driver.h"
//Device
#include "../Common/Device/Device.h"
//Callbacks
#include "../Common/callback/nt_callback.h"
//Socket
#include "../Common/socket/nt_socket.h"
//filter
#include "../Common/irpfilter/nt_irp_filter.h"
//Log
#include "../Common/Log/nt_log.h"
//内存文件
#include "../Common/memfile/nt_memfile.h"
//Cab解析
#include "../Common/cabfile/nt_cabfile.h"
//pdb解析
#include "../Common/pdbfile/nt_pdbfile.h"
#endif


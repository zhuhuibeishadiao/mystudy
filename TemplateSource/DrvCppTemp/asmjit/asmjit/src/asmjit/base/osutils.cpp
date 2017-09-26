#include "stdafx.h"
// Complete x86/x64 JIT and Remote Assembler for C++.
//
// [License]
// Zlib - See LICENSE.md file in the package.

// [Export]
#define ASMJIT_EXPORTS

// [Dependencies]
#include "../base/osutils.h"
#include "../base/utils.h"

#if ASMJIT_OS_POSIX
# include <sys/types.h>
# include <sys/mman.h>
# include <time.h>
# include <unistd.h>
#endif // ASMJIT_OS_POSIX

#if ASMJIT_OS_MAC
# include <mach/mach_time.h>
#endif // ASMJIT_OS_MAC

#if ASMJIT_OS_WINDOWS
# if defined(_MSC_VER) && _MSC_VER >= 1400
#  include <intrin.h>
# else
#  define _InterlockedCompareExchange InterlockedCompareExchange
# endif // _MSC_VER
#endif // ASMJIT_OS_WINDOWS

// [Api-Begin]
#include "../asmjit_apibegin.h"

namespace asmjit {

// ============================================================================
// [asmjit::OSUtils - Virtual Memory]
// ============================================================================
#ifdef _NTDDK_
static ASMJIT_NOINLINE const VMemInfo& OSUtils_GetVMemInfo() noexcept {
	static VMemInfo vmi;
	vmi.pageSize = PAGE_SIZE;
	vmi.pageGranularity = max(PAGE_SIZE, PAGE_SIZE * 16);
	return vmi;
};
VMemInfo OSUtils::getVirtualMemoryInfo() noexcept { return OSUtils_GetVMemInfo(); }

void* OSUtils::allocVirtualMemory(size_t size, size_t* allocated, uint32_t flags) noexcept {
	UNREFERENCED_PARAMETER(allocated);
	UNREFERENCED_PARAMETER(flags);
	return malloc(size);
}

Error OSUtils::releaseVirtualMemory(void* p, size_t size) noexcept {
	UNREFERENCED_PARAMETER(size);
	free(p);
	return kErrorOk;
}
#endif
// Posix specific implementation using `mmap()` and `munmap()`.
#if ASMJIT_OS_POSIX

// Mac uses MAP_ANON instead of MAP_ANONYMOUS.
#if !defined(MAP_ANONYMOUS)
# define MAP_ANONYMOUS MAP_ANON
#endif // MAP_ANONYMOUS

static const VMemInfo& OSUtils_GetVMemInfo() noexcept {
  static VMemInfo vmi;
  if (ASMJIT_UNLIKELY(!vmi.pageSize)) {
    size_t pageSize = ::getpagesize();
    vmi.pageSize = pageSize;
    vmi.pageGranularity = std::max<size_t>(pageSize, 65536);
  }
  return vmi;
};

VMemInfo OSUtils::getVirtualMemoryInfo() noexcept { return OSUtils_GetVMemInfo(); }

void* OSUtils::allocVirtualMemory(size_t size, size_t* allocated, uint32_t flags) noexcept {
  const VMemInfo& vmi = OSUtils_GetVMemInfo();

  size_t alignedSize = Utils::alignTo<size_t>(size, vmi.pageSize);
  int protection = PROT_READ;

  if (flags & kVMWritable  ) protection |= PROT_WRITE;
  if (flags & kVMExecutable) protection |= PROT_EXEC;

  void* mbase = ::mmap(nullptr, alignedSize, protection, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
  if (ASMJIT_UNLIKELY(mbase == MAP_FAILED)) return nullptr;

  if (allocated) *allocated = alignedSize;
  return mbase;
}

Error OSUtils::releaseVirtualMemory(void* p, size_t size) noexcept {
  if (ASMJIT_UNLIKELY(::munmap(p, size) != 0))
    return DebugUtils::errored(kErrorInvalidState);

  return kErrorOk;
}
#endif // ASMJIT_OS_POSIX

// ============================================================================
// [asmjit::OSUtils - GetTickCount]
// ============================================================================

//#error "[asmjit] OSUtils::getTickCount() is not implemented for your target OS."
uint32_t OSUtils::getTickCount() noexcept {
	LARGE_INTEGER tick_count = {};
	auto time_inc = KeQueryTimeIncrement();
	KeQueryTickCount(&tick_count);
	tick_count.QuadPart *= time_inc;
	tick_count.QuadPart /= 10000L;
	return tick_count.QuadPart;;
}



} // asmjit namespace

// [Api-End]
#include "../asmjit_apiend.h"

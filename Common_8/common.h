#pragma once
//////////////////////////////////////////////////////////////////////////
#pragma warning(disable:4996)
#pragma warning(disable:4101)
#pragma warning(disable:4005)
#pragma warning(disable:4091)
#pragma warning(disable:4800)
#pragma warning(disable:4312)
//////////////////////////////////////////////////////////////////////////
#include <tchar.h>
#include <Windows.h>
#include <assert.h>
#include <tlhelp32.h>
#include <Psapi.h>

#include <winternl.h>
#include <atomic>
#include <Shlwapi.h>
#include <winioctl.h>
#include <strsafe.h>
#include <intrin.h>
#include <intsafe.h>
//std库
#include <algorithm>
#include <functional>
#include <utility>
//stl库
#include <vector>
#include <map>
//系统库
#include <thread>
#include <chrono>
#include <memory>

namespace NTDLL
{
#include <ntstatus.h>
	extern"C"
	{
#include "ntos.h"
	}
};

#pragma warning(disable:4005)
extern "C"
{
	NTSTATUS
		NTAPI
		RtlAdjustPrivilege(
			ULONG Privilege,
			BOOLEAN Enable,
			BOOLEAN Client,
			PBOOLEAN WasEnabled
			);
};

#pragma comment(lib,"psapi.lib")
#pragma comment(lib,"shlwapi.lib")
#pragma comment(lib,"iphlpapi.lib")
#pragma comment(lib,"Wbemuuid.lib")
#pragma comment(lib,"Mpr.lib")
#pragma comment(lib,"ntdll.lib")

#include "scope_exit.h"
#include "unique_resource.h"



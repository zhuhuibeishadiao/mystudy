#pragma once
//定义头
#include <tchar.h>

//STL头
#include <string>
#include <vector>
#include <list>
#include <deque>
#include <set>
#include <map>
#include <bitset>
#include <stack>
#include <queue>
#include <array>
#include <tuple>

//STD头
#include <algorithm>
#include <functional>
#include <type_traits>
#include <utility>
#include <cmath>
#include <cstdint>
#include <atomic>

//系统头
#include <thread>
#include <chrono>
#include <mutex>
#include <random>
#include <memory>
#include <filesystem>

//Windows头
#include <windows.h>
#include <assert.h>
#include <tlhelp32.h>
#include <Psapi.h>
#include <Shlwapi.h>
#include <winioctl.h>
#include <strsafe.h>
#include <intrin.h>
#include <intsafe.h>
#include <Sfc.h>
#include <winsvc.h>



//WDK使用的头,namespace防和谐
namespace wdk::flt
{
#include <fltUser.h>
};

//lib库
#pragma comment(lib,"psapi.lib")
#pragma comment(lib,"shlwapi.lib")
#pragma comment(lib,"iphlpapi.lib")
#pragma comment(lib,"Wbemuuid.lib")
#pragma comment(lib,"Mpr.lib")
#pragma comment(lib,"ntdll.lib")
#pragma comment(lib,"FltLib.lib")
#pragma comment(lib,"Sfc.lib")

//N4189的头
#include "scope_exit.h"
#include "unique_resource.h"
//设计模式的头
#include "SingleTon.h"

//NTDLL的头
namespace ntdll
{
//NTAPI头
#include "NativeLib.h"
//Workstation的头
#include "WinStationLib.h"
};
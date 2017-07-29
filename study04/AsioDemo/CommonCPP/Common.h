#pragma once
//����ͷ
#define WIN32_LEAN_AND_MEAN 
#pragma warning(disable:4005)
#include <tchar.h>

//STLͷ
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

//STDͷ
#include <algorithm>
#include <functional>
#include <type_traits>
#include <utility>
#include <cmath>
#include <cstdint>
#include <atomic>
//IO
#include <iostream>

//ϵͳͷ
#include <thread>
#include <chrono>
#include <mutex>
#include <random>
#include <memory>
#include <filesystem>

//Windowsͷ
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
#include <winnt.h>




//WDKʹ�õ�ͷ,namespace����г
namespace wdk::flt
{
#include <fltUser.h>
};

//lib��
#pragma comment(lib, "psapi.lib")
#pragma comment(lib, "shlwapi.lib")
#pragma comment(lib, "iphlpapi.lib")
#pragma comment(lib, "Wbemuuid.lib")
#pragma comment(lib, "Mpr.lib")
#pragma comment(lib, "ntdll.lib")
#pragma comment(lib, "FltLib.lib")
#pragma comment(lib, "Sfc.lib")
#pragma comment(lib, "Ws2_32.lib")  
#pragma comment(lib, "Rpcrt4.lib")

//N4189��ͷ
#include "scope_exit.h"
#include "unique_resource.h"
//���ģʽ��ͷ
#include "SingleTon.h"
//string����ͷ
//#include "xor_string.h"
//NTDLL��ͷ
namespace ntdll
{
	//NTAPIͷ
#include "NativeLib.h"
//Workstation��ͷ
#include "WinStationLib.h"
};

#include<locale>
#include<codecvt>
using _tstring = std::basic_string<TCHAR>;

typedef _Return_type_success_(return >= 0) LONG NTSTATUS;
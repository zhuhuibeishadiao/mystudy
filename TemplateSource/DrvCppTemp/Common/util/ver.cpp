#include "stdafx.h"
OS_INDEX OsIndex = OsIndex_UNK;
namespace ddk::util
{
	RTL_OSVERSIONINFOEXW g_WindowsVersion = {};
	bool ver_ok = false;
	void init_version()
	{
		OsIndex = getWindowsIndex();
		g_WindowsVersion.dwOSVersionInfoSize = sizeof(g_WindowsVersion);
		auto ns = RtlGetVersion(reinterpret_cast<RTL_OSVERSIONINFOW*>(&g_WindowsVersion));
		if (NT_SUCCESS(ns))
		{
			ver_ok = true;
		}
	}
	bool is_window_10()
	{
		return OsIndex > OsIndex_8;
	}
	bool IsWindowsVistaOrGreater()
	{
		return OsIndex >= OsIndex_VISTA;
	}
	OS_INDEX getWindowsIndex()
	{
		if (*NtBuildNumber > 15063) // forever 10 =)
			return OsIndex_10_1707;

		switch (*NtBuildNumber)
		{
		case 2600:
			return OsIndex_XP;
			break;
		case 3790:
			return OsIndex_2K3;
			break;
		case 6000:
		case 6001:
		case 6002:
			return OsIndex_VISTA;
			break;
		case 7600:
		case 7601:
			return OsIndex_7;
			break;
		case 8102:
		case 8250:
		case 9200:
			return OsIndex_8;
		case 9431:
		case 9600:
			return OsIndex_BLUE;
			break;
		case 10240:
			return OsIndex_10_1507;
			break;
		case 10586:
			return OsIndex_10_1511;
			break;
		case 14393:
			return OsIndex_10_1607;
			break;
		case 15063:
			return OsIndex_10_1707;
			break;
		default:
			return OsIndex_UNK;
		}
	}
}
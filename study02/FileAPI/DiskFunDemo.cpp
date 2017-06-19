#include "stdafx.h"
using namespace std;


/************************************************************************/
/*               遍历系统中的驱动器以及其属性                           */
bool GetDriverInfo(LPWSTR szDrive);
//使用GetLogicalDriveStrings遍历系统中的卷和属性
void TraverseLogicDrv1()
{
	//一个数组包含所有的盘符
	WCHAR szStr[MAX_PATH] = {};
	WCHAR* pStr = szStr;
	DWORD dwRet = GetLogicalDriveStrings(MAX_PATH, szStr);
	if (dwRet <= MAX_PATH && dwRet != 0)
	{
		while (*pStr)
		{
			GetDriverInfo(pStr);
			pStr += wcslen(pStr) + 1;
		}
	}
}
//使用FindVolume遍历系统中的卷和属性
void TraverseLogicDrv2()
{
	//返回格式 "\\\\?\\Volume{GUID}\\"
	WCHAR strVolName[MAX_PATH] = {};
	HANDLE h_vol = NULL;
	h_vol = FindFirstVolume(strVolName, MAX_PATH);

	GetDriverInfo(strVolName);
	while (FindNextVolume(h_vol, strVolName, MAX_PATH))
	{
		GetDriverInfo(strVolName);
	}
	FindVolumeClose(h_vol);
}
bool GetDriverInfo(LPWSTR szDrive)
{
	//wcout << szDrive << endl;
	printf("%ws\n", szDrive);
	auto uDrvType = GetDriveType(szDrive);
	DWORD dwVolumeSerialNumber;
	DWORD dwMaximumComponentLength;
	DWORD dwFileSystemFlags;
	WCHAR szFileSystemNameBuffer[MAX_PATH];
	WCHAR szDriveName[MAX_PATH];

	switch (uDrvType)
	{
	case DRIVE_UNKNOWN:
		cout << DRIVE_UNKNOWN << ":the drive type cannot be determined" << endl;
		break;
	case DRIVE_NO_ROOT_DIR:
		cout << DRIVE_NO_ROOT_DIR << ":The root path is invalid; for example, there is no volume mounted at the specified path." << endl;
		break;
	case DRIVE_REMOVABLE:
		cout << DRIVE_REMOVABLE << ":The drive has removable media; for example, a floppy drive, thumb drive, or flash card reader." << endl;
		break;
	case DRIVE_FIXED:
		cout << DRIVE_FIXED << ":The drive has fixed media; for example, a hard disk drive or flash drive." << endl;
		break;
	case DRIVE_REMOTE:
		cout << DRIVE_REMOTE << ":The drive is a remote (network) drive." << endl;
		break;
	case DRIVE_CDROM:
		cout << DRIVE_CDROM << ":The drive is a CD-ROM drive." << endl;
		break;
	case DRIVE_RAMDISK:
		cout << DRIVE_RAMDISK << ":The drive is a RAM disk." << endl;
		break;
	default:
		break;
	}
	if (!GetVolumeInformation(szDrive, szDriveName, MAX_PATH,
		&dwVolumeSerialNumber, &dwMaximumComponentLength,
		&dwFileSystemFlags, szFileSystemNameBuffer, MAX_PATH))
		return false;
	if (0 != wcslen(szDriveName))
		wcout << "name is " << szDriveName << endl;
	cout << "Volume Serial Number is " << hex << dwVolumeSerialNumber << endl;//验证方法：在cmd中输入vol c:
	cout << "Maximum Component Length is " << dec << dwMaximumComponentLength << endl;
	wcout << "system type is " << szFileSystemNameBuffer << endl;

	if (!(dwFileSystemFlags & FILE_SUPPORTS_REPARSE_POINTS))
	{
		cout << "the file system does not support volume mount points." << endl;
	}
	if (dwFileSystemFlags & FILE_VOLUME_QUOTAS)
	{
		cout << "the file system support disk quotas" << endl;
	}
	//后面还有很多很多可以判断具体查看MSDN
	cout << endl;
	return true;
}
/************************************************************************/

/************************************************************************/
/*                     磁盘空间信息                                    */
void GetDiskSpaceInfo(LPCWSTR szPath)
{
	DWORD64 qwFreeBytesToCaller, qwTotalBytes, qeFreeBytes;
	BOOL bRet = GetDiskFreeSpaceEx(szPath, reinterpret_cast<PULARGE_INTEGER>(&qwFreeBytesToCaller),
		reinterpret_cast<PULARGE_INTEGER>(&qwTotalBytes), reinterpret_cast<PULARGE_INTEGER>(&qeFreeBytes));
	cout << qwFreeBytesToCaller << endl;
	cout << qwTotalBytes << endl;
	cout << qeFreeBytes << endl;

	DWORD dwSectPerClust, dwBytesPerSect, dwFreeCluster, dwTotalCluster;
	bRet = GetDiskFreeSpace(szPath, &dwSectPerClust, &dwBytesPerSect, &dwFreeCluster, &dwTotalCluster);
}


/************************************************************************/



void TestDiskFunDemo()
{
	//遍历驱动器的测试
	/*TraverseLogicDrv1();
	cout << "==============" << endl;
	TraverseLogicDrv2();*/

	GetDiskSpaceInfo(L"e:\\");
}
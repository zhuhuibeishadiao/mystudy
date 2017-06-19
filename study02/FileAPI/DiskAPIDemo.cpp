#include "stdafx.h"
using namespace std;

//GetLogicalDrive例子
void GetLogicalDrivesDemo()
{
	//例：输出为1c--->11100
	//从右边开始数A,B,没有！CDE有
	DWORD dwFlag = GetLogicalDrives();
	cout << hex << dwFlag << endl;
}

//GetLogicalDriveStrings例子
void GetLogicalDriveStringsDemo()
{
	//一个数组包含所有的盘符
	WCHAR szStr[MAX_PATH] = {};
	WCHAR* pStr = szStr;
	DWORD dwRet = GetLogicalDriveStrings(MAX_PATH, szStr);
	if (dwRet <= MAX_PATH && dwRet != 0)
	{
		while (*pStr)
		{
			wcout << pStr << endl;
			pStr += wcslen(pStr) + 1;
		}
	}
}

//FindFirstVolume和FindNextVolume的例子
void FindVolumeDemo()
{
	//返回格式 "\\\\?\\Volume{GUID}\\"
	WCHAR strVolName[MAX_PATH] = {};
	HANDLE h_vol = NULL;
	h_vol = FindFirstVolume(strVolName, MAX_PATH);
	wcout << strVolName << endl;

	while (FindNextVolume(h_vol, strVolName, MAX_PATH))
	{
		wcout << strVolName << endl;
	}
	FindVolumeClose(h_vol);
}

//GetDriveType例子
void GetDriveTypeDemo()
{
	//DRIVE_UNKNOWN(0)     不能确定
	//DRIVE_NO_ROOT_DIR(1) 可能路径错误
	//DRIVE_REMOVABLE(2)   设备已卸载
	//DRIVE_FIXED(3)	   固定设备，如硬盘驱动，闪存驱动(U盘，SD卡)
	//DRIVE_REMOTE(4)      网络驱动
	//DRIVE_CDROM(5)       CD - ROM驱动
	//DRIVE_RAMDISK(6)     RAM驱动
	UINT uFlag = GetDriveType(L"c:\\");
	cout << uFlag << endl;
	uFlag = GetDriveType(L"d:\\");
	cout << uFlag << endl;
	uFlag = GetDriveType(L"e:\\");
	cout << uFlag << endl;
}

//GetVolumeInformation例子
void GetVolumeInformationDemo()
{
	WCHAR szVolName[MAX_PATH] = {0};
	BOOL isSuc = GetVolumeInformation(L"c:\\", szVolName, MAX_PATH, NULL, NULL, NULL, NULL, NULL);
}

//FindVolumeMountPoint例子
void FindVolumeMountPointDemo()
{
	//找到c盘下有多少个挂载点
	WCHAR szVolName[MAX_PATH] = {0};
	HANDLE h_VolMntPoint = FindFirstVolumeMountPoint(L"c:\\",szVolName, MAX_PATH);
	wcout << "c:\\" << szVolName << endl;

	while (FindNextVolumeMountPoint(h_VolMntPoint, szVolName, MAX_PATH))
	{
		wcout << "c:\\" << szVolName << endl;
	}
	FindVolumeMountPointClose(h_VolMntPoint);
}

//GetVolumeNameForVolumeMountPoint例子
void GetVolumeNameForVolumeMountPointDemo()
{

	WCHAR szVolName[MAX_PATH] = {};
	//可以输入"c:\\","c\\mnt"挂载点也可以
	BOOL isSuc = GetVolumeNameForVolumeMountPoint(L"c:\\mnt\\", szVolName, MAX_PATH);
}

void SetVolumeMountPointDemo()
{
	WCHAR szStr[MAX_PATH] = {};
	GetVolumeNameForVolumeMountPoint(L"e:\\", szStr, MAX_PATH);
	SetVolumeMountPoint(L"c:\\mnt2\\", szStr);
}
void TestDiskApiDemo()
{
	//GetLogicalDrivesDemo();
	//GetLogicalDriveStringsDemo();
	//FindVolumeDemo();
	//GetDriveTypeDemo();
	//GetVolumeInformationDemo();
	//FindVolumeMountPointDemo();
	//GetVolumeNameForVolumeMountPointDemo();
	//SetVolumeMountPointDemo();
	
}
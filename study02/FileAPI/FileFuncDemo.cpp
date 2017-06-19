#include "stdafx.h"
#include <functional>
using namespace std;

//
/*********************文件删除，复制，重命名，移动文件 ********************/
#define FILE_PATH L"C:\\Users\\killvxk\\Desktop\\123.txt"
#define NEW_PATH  L"C:\\Users\\killvxk\\Desktop\\456.txt"
void Demo1()
{
	BOOL isSuc = FALSE;
	isSuc = CopyFile(FILE_PATH, NEW_PATH, TRUE);
	if (isSuc)
		cout << "copy success" << endl;

	isSuc = DeleteFile(FILE_PATH);
	if (isSuc)
		cout << "delete success!" << endl;

	isSuc = MoveFile(NEW_PATH, L"c:\\Users\\killvxk\\Desktop\\mov.txt");
	if (isSuc)
		cout << "move success" << endl;
}
/************************************************************************/

/*********************获取文件大小、读取文件内容*************************/
DWORD ReadFileContent(LPWSTR szFilePath)
{
	LARGE_INTEGER liFileSize;
	DWORD64 totalRead = 0;
	DWORD dwReadedSize = 0;
	BYTE lpBuf[32];

	auto h_file = CreateFile(szFilePath,
		GENERIC_READ,
		FILE_SHARE_READ,
		NULL, OPEN_ALWAYS,
		FILE_ATTRIBUTE_NORMAL,
		NULL);
	if (h_file == INVALID_HANDLE_VALUE)
	{
		cout << "CreateFile失败 :" << GetLastError() << endl;
		return 0;
	}

	if (!GetFileSizeEx(h_file, &liFileSize))//读取文件大小
	{

		CloseHandle(h_file);
		cout << "GetFileSizeEx失败:" << GetLastError() << endl;
		return 0;
	}
	else
	{
		cout << "文件大小为:" << liFileSize.QuadPart << endl;
	}

	while (true)
	{
		auto isSuc = ReadFile(h_file, lpBuf, 32, &dwReadedSize, NULL);//读取内容，每次读取都会移动文件指针
		if (!isSuc) break;

		for (DWORD i = 0; i < dwReadedSize; i++)
		{
			printf("%c", lpBuf[i]);
		}

		totalRead += dwReadedSize;

		if (totalRead == liFileSize.QuadPart)
		{
			printf("\n");
			cout << "读取结束" << endl;
			break;
		}
	}
	return 0;
}
/************************************************************************/

/****************** 设置文件指针，写数据到文件中**************************/
DWORD SaveDataToFile(LPWSTR szFilePath, LPVOID lpData, DWORD dwDataSize)
{
	HANDLE h_file = NULL;
	DWORD dwWriteSize = 0;
	h_file = CreateFile(szFilePath, GENERIC_ALL, 0, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

	if (h_file == INVALID_HANDLE_VALUE)
	{
		cout << "CreateFile失败 :" << GetLastError() << endl;
		return 0;
	}
	SetFilePointer(h_file, 0, 0, FILE_END);
	WriteFile(h_file, lpData, dwDataSize, &dwWriteSize, NULL);
	CloseHandle(h_file);
	return dwWriteSize;
}
/************************************************************************/

/*************************目录基本操作********************************/
void DirectoryDemo()
{
	WCHAR szCurDir[MAX_PATH];
	WCHAR szModuleName[MAX_PATH];
	DWORD dwCurlen = GetCurrentDirectory(MAX_PATH, szCurDir);	//获得当前目录
	if (dwCurlen == 0)
		return;
	wcout << szCurDir << "     长度：" << dwCurlen << endl;

	wcscpy_s(szCurDir, L"E:\\");
	SetCurrentDirectory(szCurDir);	//设置当前目录
	dwCurlen = GetCurrentDirectory(MAX_PATH, szCurDir);
	wcout << szCurDir << "     长度：" << dwCurlen << endl;
	CreateDirectory(L"hamage", NULL);

	GetModuleFileName(NULL, szModuleName, MAX_PATH);
	wcout << szModuleName << endl;
	GetModuleFileName(LoadLibraryA("kernel32.dll"), szModuleName, MAX_PATH);
	wcout << szModuleName << endl;

}
/************************************************************************/

/***********************遍历某目录中的文件和子目录**************************/
void EnumFileInDir(LPWSTR szPath)
{
	//传过来的要使用通配符
	WIN32_FIND_DATA fileData;
	wcscat_s(szPath, MAX_PATH, L"\\*");
	auto h_find = FindFirstFile(szPath, &fileData);
	if (INVALID_HANDLE_VALUE == h_find)
		return;

	do
	{
		//这里有很多属性，如隐藏等，你可以来筛选
		wcout << hex << fileData.dwFileAttributes << "\t";
		wcout << fileData.cFileName << endl;
	} while (FindNextFile(h_find, &fileData));
	FindClose(h_find);
}
//遍历某目录下所有的文件和子目录
void EnumAllInDir(LPCWSTR szPath)
{
	WCHAR szFilePath[MAX_PATH] = {};	
	WCHAR szRecurPath[MAX_PATH] = {};//如果有文件夹继续进去
	WIN32_FIND_DATA data = {};
	wcscpy_s(szFilePath, MAX_PATH, szPath);
	wcscat_s(szFilePath, MAX_PATH, L"\\*");//通配符，API要求，
	auto h_find = FindFirstFile(szFilePath, &data);
	if (h_find == INVALID_HANDLE_VALUE)
		return;
	do
	{
		if (wcscmp(L".", data.cFileName) == 0 || wcscmp(L"..", data.cFileName) == 0)
			continue;
		
		if (data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		{
			wsprintf(szRecurPath, L"%s\\%s", szPath, data.cFileName);
			EnumAllInDir(szRecurPath);
		}
		else
		{
			wcout << data.cFileName << endl;
		}
	} while (FindNextFile(h_find, &data));
}
/************************************************************************/

/*************************获取文件属性********************************/
void ShowFileAttribute(LPCWSTR szPath)
{
	auto showTime = [=](PFILETIME ptime) {
		FILETIME ftLocal;
		SYSTEMTIME st;
		FileTimeToLocalFileTime(ptime, &ftLocal);
		FileTimeToSystemTime(&ftLocal, &st);
		cout << st.wYear << "年";
		cout << st.wMonth << "月";
		cout << st.wDay << "日";
		cout << "星期" << st.wDayOfWeek << "  ";
		cout << st.wHour << ":";
		cout << st.wMinute << ":";
		cout << st.wSecond << endl;
	};
	WIN32_FIND_DATA data;
	if (!GetFileAttributesEx(szPath, GetFileExInfoStandard, &data))
		return;
	cout << "创建时间：", showTime(&data.ftCreationTime);
	cout << "最后访问时间", showTime(&data.ftLastWriteTime);
	cout << "最后修改时间", showTime(&data.ftLastWriteTime);
	
	LARGE_INTEGER fileSize = {};
	fileSize.HighPart = data.nFileSizeHigh;
	fileSize.LowPart = data.nFileSizeLow;
	cout << "文件大小" << fileSize.QuadPart << endl;;
	cout << "还有些表示在dwFileAttributes" << endl;
}
/************************************************************************/

/*********************设置文件属性*************************************/
void SetFileAttributesDemo(LPCWSTR szPath)
{
	DWORD dwFlag = GetFileAttributes(szPath);
	dwFlag |= FILE_ATTRIBUTE_READONLY;
	SetFileAttributes(szPath, dwFlag);
}
/************************************************************************/

/**************************文件映射,修改及刷新*******************************/
#define FILE_MAP_START 0x0
void FileMapDemo(LPCWSTR szPath)
{
	SYSTEM_INFO si = {};
	GetSystemInfo(&si);
	DWORD dwSysGran = si.dwAllocationGranularity;
	DWORD dwFileMapStart = (FILE_MAP_START / dwSysGran) *dwSysGran;
	DWORD dwMapViewSize = 0x1000;

	auto h_file = CreateFile(szPath, GENERIC_ALL, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	auto h_mapfile = CreateFileMapping(h_file, NULL, PAGE_EXECUTE_READWRITE, 0, dwMapViewSize, NULL);
	auto lpMapAddr = MapViewOfFile(h_mapfile, FILE_MAP_ALL_ACCESS, 0, dwFileMapStart, dwMapViewSize);

	PBYTE pToFileMap = static_cast<PBYTE>(lpMapAddr);
	for (UINT i = 1; i <= dwMapViewSize; i++)
	{
		printf("%02x ", *pToFileMap);
		pToFileMap++;
		if (i % 0x10 == 0)
			cout << endl;
	}
	//FillMemory(lpMapAddr, 1, 0xff);
	FillMemory(lpMapAddr, 1, 0x4d);
	FlushViewOfFile(lpMapAddr, dwMapViewSize);
	UnmapViewOfFile(lpMapAddr);
	CloseHandle(h_mapfile);
	CloseHandle(h_file);
}
/************************************************************************/

void TestFileFuncDemo()
{
	std::locale::global(std::locale(""));
	//Demo1();

	/*******************文件读写*******************************/
	//ReadFileContent(FILE_PATH);
	//CHAR szFoo[] = "\r\n我草娘们";
	//SaveDataToFile(FILE_PATH, szFoo, strlen(szFoo));
	//DirectoryDemo();
	
	/*******************目录遍历*******************************/
	//枚举目录
	//WCHAR path[MAX_PATH] = L"E:\\360Downloads\\software";
	//EnumFileInDir(path);
	//EnumAllInDir(L"C:\\360安全浏览器下载");

	/*******************文件属性******************************/
	//ShowFileAttribute(L"E:\\360Downloads\\software\\AdobeReader_yjzj.exe");
	//SetFileAttributesDemo(L"E:\\360Downloads\\software\\AdobeReader_yjzj.exe");

	/****************************************************/
	FileMapDemo(L"E:\\360Downloads\\software\\1111.exe");

}
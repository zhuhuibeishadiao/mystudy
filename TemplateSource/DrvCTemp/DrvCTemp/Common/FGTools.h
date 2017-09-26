#include <ntifs.h>
#include <ntstrsafe.h>
#include <windef.h>

#define MAX_PATH	260

//通过进程id获得全路径
NTSTATUS  FGGetProcessFullNameByPid(IN HANDLE nPid, OUT PUNICODE_STRING  FullPath);


/***************************************获得进程全路径*********************************************/

//输入：\\??\\c:  输出：\\device\\\harddiskvolume1
//LinkTarget.Buffer注意要释放
NTSTATUS FGGetSymbolicVolume(IN PUNICODE_STRING SymbolicLinkName, OUT PUNICODE_STRING LinkTarget);

//输入：\\Device\\harddiskvolume1    输出：C:
//DosName.Buffer的内存记得释放
NTSTATUS FGGetDosVolume(IN PUNICODE_STRING DeviceName, OUT PUNICODE_STRING DosName);

//输入：\\??\\c:\\windows\\hi.txt    输出\\device\\harddiskvolume1\\windows\\hi.txt
BOOLEAN FGGetSymbolicPath(IN WCHAR * filename, OUT WCHAR * ntname);

//输入\\device\\harddiskvolume1\\windows\\hi.txt    输出：c:\\windows\\hi.txt
BOOLEAN FGGeDosPath(IN WCHAR *wszNTName, OUT WCHAR *wszFileName);

//输入：：\\??\\c:\\windows\\hi.txt    输出：c:\\windows\\hi.txt  未完成

/***************************************获得进程全路径*********************************************/


/***************************************短路劲向长路劲转换*****************************************/
//size -->  WCHAR wszLongName[MAX_PATH] = {0}; sizeof(wszLongName)
BOOL FGConverShortToLongPath(OUT WCHAR *wszLongName, IN WCHAR *wszShortName, IN ULONG size);
/***************************************短路劲向长路劲转换*****************************************/




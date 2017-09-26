#pragma once
#pragma pack(1)
typedef struct _KSERVICE_TABLE_DESCRIPTOR {
#ifdef _X86_
	PULONG_PTR	Base;
#else
	LONG	*OffsetToService;
#endif
	PULONG Count;
	ULONG Limit;
	PUCHAR Number;
} KSERVICE_TABLE_DESCRIPTOR, *PKSERVICE_TABLE_DESCRIPTOR;
#pragma pack()
#ifndef SEC_IMAGE
#define SEC_IMAGE         0x1000000  
#endif // !
#pragma pack(1)
#define CV_SIGNATURE_NB10   '01BN'
#define CV_SIGNATURE_RSDS   'SDSR'
// CodeView header 
struct CV_HEADER
{
	DWORD CvSignature; // NBxx
	LONG  Offset;      // Always 0 for NB10
};

// CodeView NB10 debug information 
// (used when debug information is stored in a PDB 2.00 file) 
struct CV_INFO_PDB20
{
	CV_HEADER  Header;
	DWORD      Signature;       // seconds since 01.01.1970
	DWORD      Age;             // an always-incrementing value 
	BYTE       PdbFileName[1];  // zero terminated string with the name of the PDB file 
};

// CodeView RSDS debug information 
// (used when debug information is stored in a PDB 7.00 file) 
struct CV_INFO_PDB70
{
	DWORD      CvSignature;
	GUID       Signature;       // unique identifier 
	DWORD      Age;             // an always-incrementing value 
	BYTE       PdbFileName[1];  // zero terminated string with the name of the PDB file 
};
#pragma pack()
namespace ddk::util
{
	PVOID get_sysinfo(SYSTEM_INFORMATION_CLASS InfoClass);
	PVOID load_dll(std::wstring filename);
	VOID free_dll(HANDLE hMod);
	bool get_sys_module_list(std::vector<AUX_MODULE_EXTENDED_INFO> &syslist);
	PVOID get_module_base(std::string modulename, PULONG_PTR pImageSize = nullptr);
	bool getPdbInfo(PVOID ImageBase, std::string &pdbFileName, std::string &symSignature);
	ULONG_PTR get_proc_address(PVOID Image, std::string functionname);
	PVOID get_proc_address(std::string functionname);
	//////////////////////////////////////////////////////////////////////////
	NTSTATUS LoadFileToMem(wchar_t *strFileName, PVOID *lpVirtualAddress);
	NTSTATUS GetSystemImageInfo(char* __in pszImageName, RTL_PROCESS_MODULE_INFORMATION* __out lpsystemModule);
	//////////////////////////////////////////////////////////////////////////
	ULONG GetPreviousModeOffset();
	PKSERVICE_TABLE_DESCRIPTOR get_ssdt();
	PVOID get_ssdt_function_address(DWORD index);
	ULONG get_syscall_number(std::string functionname);
	//////////////////////////////////////////////////////////////////////////
	PFILE_OBJECT get_file_object_by_fullimagename(PUNICODE_STRING usFileName);
	bool get_file_object_full_path(PFILE_OBJECT fileobject, PUNICODE_STRING usFullPath);
	bool get_file_full_path(PUNICODE_STRING inPath, PUNICODE_STRING outPath);
	//////////////////////////////////////////////////////////////////////////
	BOOLEAN get_reg_fullname(
		PUNICODE_STRING pRegistryPath,
		PUNICODE_STRING pPartialRegistryPath,
		PVOID pRegistryObject);
	//////////////////////////////////////////////////////////////////////////
	bool get_driver_object(std::wstring drvname, PDRIVER_OBJECT &drv_object);
	//////////////////////////////////////////////////////////////////////////
	bool nt_mkdir(std::wstring _dirname);
	//////////////////////////////////////////////////////////////////////////
	SIZE_T AlignSize(SIZE_T nSize, UINT nAlign);
	//////////////////////////////////////////////////////////////////////////
	PVOID get_ntos_imagebase();
	//////////////////////////////////////////////////////////////////////////
	PVOID LoadResource(PVOID Module, PIMAGE_RESOURCE_DATA_ENTRY ResourceDataEntry, PULONG Size);
	PIMAGE_RESOURCE_DATA_ENTRY FindResource(PVOID Module, PCWSTR Name, PCWSTR Type);
};

namespace ddk::util
{
	void init_version();
	bool is_window_10();
	bool IsWindowsVistaOrGreater();
	OS_INDEX getWindowsIndex();
};

namespace ddk::util
{
	using _EachCallBackRoutine = std::function<NTSTATUS(PVOID)>;

	ULONG KeQueryActiveProcessorCountCompatible();

	NTSTATUS ForEachProcessors(
		__in _EachCallBackRoutine CallbackRoutine,
		__in_opt void* Context);
}

namespace ddk::util
{
	bool set_idt_handler(ULONG _vec, PVOID _pfn);
	PVOID get_idt_handler(ULONG _vec);
}
#include "../util/mem_util.h"
#include "../util/time.h"
#include "../util/syscall/nt_syscall.h"
#include "../util/ldr.h"


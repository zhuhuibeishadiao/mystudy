#include <ntddk.h>

/*创建一个KEY*/
NTSTATUS FGCreateKey(IN PUNICODE_STRING keyPath);

/*打开一个KEY，返回一个句柄，用完切记释放*/
NTSTATUS FGOpenKey(IN PUNICODE_STRING regPath, OUT PHANDLE phReg);

/*删除一个KEY*/
NTSTATUS FGDeleteKey(IN PUNICODE_STRING keyPath);

/*设置一个VALUEKEY,type:REG_BINARY, REG_DWORD,REG_SZ, REG_EXPAND_SZ, REG_MULTI_SZ*/
NTSTATUS FGSetValueKey(IN PUNICODE_STRING keyPath, IN ULONG type, IN PUNICODE_STRING valueName, IN PUNICODE_STRING value);

/*删除一个VALUEKEY*/
NTSTATUS FGdeleteValueKey(IN PUNICODE_STRING keyPath, PUNICODE_STRING valuename);

/*查询一个DWORD值*/
NTSTATUS FGQueryDword(IN PUNICODE_STRING keyPath, IN PUNICODE_STRING valuename, OUT PULONG value);

/*查询一个SZ,需要自己定义value的缓冲，因为注册表操作比较小，可以定义大致合适的*/
NTSTATUS FGQuerySZ(IN PUNICODE_STRING keyPath, IN PUNICODE_STRING valuename, OUT PUNICODE_STRING value);
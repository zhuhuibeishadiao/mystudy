#pragma once
#ifdef _cplusplus
extern "C"
{
#endif
#include <ntddk.h>
#ifdef _cplusplus
}
#endif

#define  PAGECODE code_seg("PAGE")
#define  LOCKEDCODE code_seg()
#define  INITCODE code_seg("INIT")

#pragma warning(disable:4245)
#pragma warning(disable:4100)
#pragma warning(disable:4456)
#pragma warning(disable:4458 4189 4706 4702 4838)


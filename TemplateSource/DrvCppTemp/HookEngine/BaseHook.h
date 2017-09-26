#pragma once

#ifndef _NTDDK_
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

#include <vector>

// Distorm v3.3 X86/X64
#include "../distorm/distorm.h"

#include "Intrinsic.h"
#include "AsmGen.h"
#include "Reassembler.h"
#include "Detours.h"
#include "DetoursInternal.h"

// TODO:  在此处引用程序需要的其他头文件

#ifdef _WIN64
namespace hook = Detours::X64;
#else
namespace hook = Detours::X86;
#endif

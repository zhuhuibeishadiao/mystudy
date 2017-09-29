#pragma once
//#include "stdafx.h"
#include "util.h"
#include "snapshot_process.h"
#include "Process.h"
#include "Memory.h"
#include "inject.h"
#include "Services.h"


//ÍøÂç
#define ASIO_STANDALONE
#include "asio.hpp"
#include "pkg_msg.h"
#include "client.h"
#include "server.h"

#include "lsp_helper.h"

#include <msgpack.hpp>

//https http
//vcpkg install cpr cpr:x64-windows
//#include <cpr\cpr.h>

//PEÎÄ¼þ¿â
//LIEF
//git clone https://github.com/lief-project/LIEF.git
//cmake -G"Visual Studio 14 2015"
//devenv LIEF.sln
#include <iso646.h>
#include <LIEF/LIEF.hpp>

//LDR
#include "ldr_tools.h"
#include "ldr_patch_loader.h"
#include "ldr_module.h"

//hijack
#include "dll_hijack.h"
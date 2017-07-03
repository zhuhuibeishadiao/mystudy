#pragma once
//#include "stdafx.h"
//服务操作
namespace usr::util
{
	//创建
	//删除
	//改变状态
	using SERVICES_TYPE = enum 
	{
		EXESVC,
		SYSSVC,
		FLTSVC,
		SVCHOST,
	};
	class SvcMgr:public Singleton<SvcMgr>
	{
	public:
		SvcMgr():_schandle(nullptr){
			get_all_privilege();
			_schandle = OpenSCManager(nullptr, nullptr, SC_MANAGER_ALL_ACCESS);
			if (!_schandle)
			{
				DebugBreak();
			}
		};
		~SvcMgr() {
			if (_schandle)
			{
				CloseServiceHandle(_schandle);
			}
		}
		//停止服务,_delete=true时则停止并删除！
		bool stop_service(_tstring svc_name,bool _delete=false)
		{
			const auto serviceHandle = std::experimental::make_unique_resource(
				OpenService(_schandle, svc_name.c_str(),
					SERVICE_ALL_ACCESS),
				&CloseServiceHandle);
			if (!serviceHandle.get()) {
				return false;
			}

			// Stop the service
			SERVICE_STATUS status = {};
			if (ControlService(serviceHandle.get(), SERVICE_CONTROL_STOP, &status)) {
				while (QueryServiceStatus(serviceHandle.get(), &status)) {
					if (status.dwCurrentState != SERVICE_START_PENDING) break;

					Sleep(500);
				}
			}

			if (_delete)
			{
				DeleteService(serviceHandle.get());
				TCHAR fsRegistry[MAX_PATH] = {};
				if (SUCCEEDED(StringCchPrintf(fsRegistry, _countof(fsRegistry),
					_T("SYSTEM\\CurrentControlSet\\Services\\%s\\"),
					svc_name.c_str()))) {
					SHDeleteKey(HKEY_LOCAL_MACHINE, fsRegistry);
				}
			}

			return (status.dwCurrentState == SERVICE_STOPPED);
		}
		//启动一个服务
		bool start_service(_tstring svc_name)
		{
			const auto serviceHandle = std::experimental::make_unique_resource(
				OpenService(_schandle, svc_name.c_str(),
					 SERVICE_ALL_ACCESS),
				&CloseServiceHandle);
			if (!serviceHandle.get()) {
				return false;
			}
			SERVICE_STATUS status = {};
			if (StartService(serviceHandle.get(), 0, nullptr)) {
				while (QueryServiceStatus(serviceHandle.get(), &status)) {
					if (status.dwCurrentState != SERVICE_START_PENDING) {
						break;
					}

					Sleep(500);
				}
			}
			else {
				OutputDebugString(_T("StartService failed"));
			}

			if (status.dwCurrentState != SERVICE_RUNNING) {
				//DeleteService(serviceHandle.get());
				return false;
			}
			return true;
		}

		bool is_svc_installed(_tstring svc_name)
		{
			return (FALSE != ::CloseServiceHandle(::OpenService(
				_schandle, svc_name.c_str(), GENERIC_READ)));
		}

		bool create_service(_tstring svc_name, _tstring file_path, SERVICES_TYPE svc_Type,bool _start=true)
		{
			using _loader = std::function<SC_HANDLE(_tstring, _tstring)>;
			if (is_svc_installed(svc_name))
			{
				return false;
			}
			auto get_loader = [&](SERVICES_TYPE svcType){
				_loader ret = nullptr;
				switch (svcType)
				{
				case SYSSVC:
					ret = std::bind(&SvcMgr::loader_SysSvc, this, std::placeholders::_1, std::placeholders::_2);
					break;
				case SVCHOST:
					ret = std::bind(&SvcMgr::loader_Svchost, this, std::placeholders::_1, std::placeholders::_2);
					break;
				case FLTSVC:
					ret = std::bind(&SvcMgr::loader_FltSvc, this, std::placeholders::_1, std::placeholders::_2);;
					break;
				case EXESVC:
					ret = std::bind(&SvcMgr::loader_ExeSvc, this, std::placeholders::_1, std::placeholders::_2);
					break;
				default:
					ret=nullptr;
					break;
				}
				return ret;
			};
			const auto loader = get_loader(svc_Type);
			if (!loader)
			{
				return false;
			}
			const auto serviceHandle = std::experimental::make_unique_resource(
				loader(svc_name, file_path), &CloseServiceHandle);
			if (!serviceHandle.get())
			{
				OutputDebugString(_T("what\r\n"));
				return false;
			}
			if (_start)
			{

				SERVICE_STATUS status = {};
				if (StartService(serviceHandle.get(), 0, nullptr)) {
					while (QueryServiceStatus(serviceHandle.get(), &status)) {
						if (status.dwCurrentState != SERVICE_START_PENDING) {
							break;
						}

						Sleep(500);
					}
				}
				else {
					OutputDebugString(_T("StartService failed"));
				}

				if (status.dwCurrentState != SERVICE_RUNNING) {
					return false;
				}
			}
			return true;
		}
	private:
		SC_HANDLE loader_SysSvc(_tstring svc_name, _tstring file_path)
		{
			auto ret = CreateService(_schandle, svc_name.c_str(), svc_name.c_str(),
				SERVICE_ALL_ACCESS, SERVICE_KERNEL_DRIVER,
				SERVICE_DEMAND_START, SERVICE_ERROR_NORMAL, 
				file_path.c_str(),
				nullptr, nullptr, nullptr, nullptr, nullptr);
			return ret;
		}
		SC_HANDLE loader_ExeSvc(_tstring svc_name, _tstring file_path)
		{
			auto ret = CreateService(_schandle, svc_name.c_str(), svc_name.c_str(),
				SERVICE_ALL_ACCESS, SERVICE_WIN32_SHARE_PROCESS |SERVICE_INTERACTIVE_PROCESS,
				SERVICE_DEMAND_START, SERVICE_ERROR_NORMAL,
				file_path.c_str(),
				nullptr, nullptr, nullptr, nullptr, nullptr);
			return ret;
		}
		SC_HANDLE loader_FltSvc(_tstring svc_name, _tstring file_path)
		{
			const TCHAR ALTITUDE[] = _T("370000");
			TCHAR fsRegistry[MAX_PATH] = {};
			if (!SUCCEEDED(StringCchPrintf(
				fsRegistry, _countof(fsRegistry),
				_T("SYSTEM\\CurrentControlSet\\Services\\%s\\Instances"),
				svc_name.c_str()))) {
				return nullptr;
			}

			HKEY key = nullptr;
			if (ERROR_SUCCESS != RegCreateKeyEx(HKEY_LOCAL_MACHINE, fsRegistry, 0,
				nullptr, 0, KEY_ALL_ACCESS, nullptr,
				&key, nullptr)) {
				return nullptr;
			}

			const auto valueSize = (_tcslen(svc_name.c_str()) + 1) * 2;
			const auto scopedRegCloseKey =
				std::experimental::make_scope_exit([key] { RegCloseKey(key); });
			if (ERROR_SUCCESS !=
				RegSetValueEx(key, _T("DefaultInstance"), 0, REG_SZ,
					reinterpret_cast<const BYTE*>(svc_name.c_str()), valueSize)) {
				return nullptr;
			}

			StringCchCat(fsRegistry, _countof(fsRegistry), _T("\\"));
			if (!SUCCEEDED(
				StringCchCat(fsRegistry, _countof(fsRegistry), svc_name.c_str()))) {
				return nullptr;
			}

			HKEY keySub = nullptr;
			if (ERROR_SUCCESS != RegCreateKeyEx(HKEY_LOCAL_MACHINE, fsRegistry, 0,
				nullptr, 0, KEY_ALL_ACCESS, nullptr,
				&keySub, nullptr)) {
				return nullptr;
			}
			const auto scopedRegCloseKey2 =
				std::experimental::make_scope_exit([keySub] { ::RegCloseKey(keySub); });

			if (ERROR_SUCCESS != RegSetValueEx(keySub, _T("Altitude"), 0, REG_SZ,
				reinterpret_cast<const BYTE*>(ALTITUDE),
				sizeof(ALTITUDE))) {
				return nullptr;
			}

			DWORD regValue = 0;
			if (ERROR_SUCCESS != RegSetValueEx(keySub, _T("Flags"), 0, REG_DWORD,
				reinterpret_cast<const BYTE*>(&regValue),
				sizeof(regValue))) {
				return nullptr;
			}

			return CreateService(_schandle, svc_name.c_str(), svc_name.c_str(),
				SERVICE_ALL_ACCESS, SERVICE_FILE_SYSTEM_DRIVER,
				SERVICE_DEMAND_START, SERVICE_ERROR_NORMAL, file_path.c_str(),
				_T("FSFilter Activity Monitor"), nullptr, _T("FltMgr"),
				nullptr, nullptr);
		}
		SC_HANDLE loader_Svchost(_tstring svc_name, _tstring file_path)
		{
			auto host_key = _tstring(_T("SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Svchost"));
			auto svchost = _tstring(_T("%SystemRoot%\\system32\\svchost.exe -k "))+svc_name;
			TCHAR svcRegister[MAX_PATH] = {};
			if (!SUCCEEDED(StringCchPrintf(
				svcRegister, _countof(svcRegister),
				_T("SYSTEM\\CurrentControlSet\\Services\\%s\\Parameters"),
				svc_name.c_str()))) {
				return nullptr;
			}
			HKEY key = nullptr;
			if (ERROR_SUCCESS != RegCreateKeyEx(HKEY_LOCAL_MACHINE, svcRegister, 0,
				nullptr, 0, KEY_ALL_ACCESS, nullptr,
				&key, nullptr)) {
				return nullptr;
			}
			const auto scopedRegCloseKey =
				std::experimental::make_scope_exit([key] { RegCloseKey(key); });


			//写ServiceDLL
			const auto valueSize = (_tcslen(file_path.c_str()) + 1) * 2;
			if (ERROR_SUCCESS !=
				RegSetValueEx(key, _T("ServiceDll"), 0, REG_EXPAND_SZ,
					reinterpret_cast<const BYTE*>(file_path.c_str()),valueSize)) {
				return nullptr;
			}

			HKEY hHostKey = nullptr;
			if (ERROR_SUCCESS != RegOpenKeyEx(HKEY_LOCAL_MACHINE, host_key.c_str()
				,0,KEY_ALL_ACCESS, &hHostKey))
			{
				return nullptr;
			}
			const auto scopedRegCloseKey2 =
				std::experimental::make_scope_exit([hHostKey] { RegCloseKey(hHostKey); });
			DWORD dwSize = 0;
			RegQueryValueEx(hHostKey, svc_name.c_str(), 0, 0, 0, &dwSize);
			if (dwSize)
			{
				auto Len = (_tcslen(svc_name.c_str()) + 1) * 2;
				auto pData = (char*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, dwSize + Len + 2);
				if (pData)
				{
					
					std::uninitialized_fill_n(pData, Len + dwSize + 2,0);
					RegQueryValueEx(hHostKey, svc_name.c_str(), 0, 0, (BYTE*)pData, &dwSize);
					wchar_t *pNames = nullptr;
					for (pNames = (wchar_t *)pData; *pNames; pNames = wcschr(pNames, 0) + 1)
					{
						if (!lstrcmpi(pNames, svc_name.c_str()))
							break;
					}
					if (*pNames==0)
					{
						memcpy(pData + dwSize - 1, svc_name.c_str(), Len);
						RegSetValueEx(hHostKey, svc_name.c_str(), 0, REG_MULTI_SZ, (BYTE*)pData, dwSize + Len + 2);
					}
					HeapFree(GetProcessHeap(), 0, pData);
				}
			}
			else
			{
				const auto valueSize = (_tcslen(svc_name.c_str()) + 1) * 2;
				if (ERROR_SUCCESS !=
					RegSetValueEx(hHostKey, svc_name.c_str(), 0, REG_MULTI_SZ,
						reinterpret_cast<const BYTE*>(svc_name.c_str()),valueSize)) {
					return nullptr;
				}
			}
			auto ret = CreateService(_schandle, svc_name.c_str(), svc_name.c_str(),
				SERVICE_ALL_ACCESS, SERVICE_WIN32_SHARE_PROCESS|SERVICE_INTERACTIVE_PROCESS,
				SERVICE_AUTO_START, SERVICE_ERROR_NORMAL,
				svchost.c_str(),
				nullptr, nullptr, nullptr, nullptr, nullptr);
			return ret;
		}
	private:
		SC_HANDLE _schandle;
	};
};
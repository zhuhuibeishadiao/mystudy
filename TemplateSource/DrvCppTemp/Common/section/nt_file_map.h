#pragma once
namespace ddk
{
	class nt_file_map
	{
	public:
		nt_file_map() {
			_hSection = nullptr;
			_MemView = nullptr;
			_Process = ZwCurrentProcess();
			_SizeToMap = 0;
		};
		~nt_file_map() {

			if (_MemView)
			{
				LOG_DEBUG("free mem\r\n");
				unmap();
			}
			if (_hSection)
			{
				LOG_DEBUG("free handle\r\n");
				ZwClose(_hSection);
			}
		}
		nt_file_map(std::wstring map_name)
		{
			_hSection = nullptr;
			_MemView = nullptr;
			_Process = ZwCurrentProcess();
			_SizeToMap = 0;
			if (!create(map_name))
			{
				LOG_DEBUG("map create failed\r\n");
			}
		}
		nt_file_map(HANDLE hFile)
		{
			_hSection = nullptr;
			_MemView = nullptr;
			_Process = ZwCurrentProcess();
			_SizeToMap = 0;
			if (!open(hFile))
			{
				LOG_DEBUG("map open handle failed\r\n");
			}
		}
	public:
		nt_file_map & operator = (nt_file_map &_filemap)
		{
			this->_MemView = _filemap._MemView;
			this->_hSection = _filemap._hSection;
			this->_Process = _filemap._Process;
			_filemap.set_rel();
			return (*this);
		}
		void set_rel()
		{
			_MemView = nullptr;
			_hSection = nullptr;
		}
	public:
		bool open(std::wstring map_name)
		{
			SECURITY_DESCRIPTOR Se;
			auto ns = RtlCreateSecurityDescriptor(&Se, SECURITY_DESCRIPTOR_REVISION);
			if (!NT_SUCCESS(ns))
			{
				ddk::status::LogStatus(ns);
				return false;
			}
			ns = RtlSetDaclSecurityDescriptor(&Se, TRUE, NULL, TRUE);
			if (!NT_SUCCESS(ns))
			{
				ddk::status::LogStatus(ns);
				return false;
			}

			OBJECT_ATTRIBUTES oa;
			UNICODE_STRING nsSecName;
			auto full_map_name = std::wstring(L"\\BaseNamedObjects\\") + map_name;
			RtlInitUnicodeString(&nsSecName, full_map_name.c_str());
			InitializeObjectAttributes(&oa, &nsSecName,
				OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE,
				NULL,
				&Se);
			ns = ZwOpenSection(&_hSection, SECTION_ALL_ACCESS, &oa);
			if (!NT_SUCCESS(ns))
			{
				LOG_DEBUG("open section name\r\n");
				ddk::status::LogStatus(ns);
				return false;
			}
			return true;
		}
		bool create(std::wstring map_name)
		{
			SECURITY_DESCRIPTOR Se;
			auto ns = RtlCreateSecurityDescriptor(&Se, SECURITY_DESCRIPTOR_REVISION);
			if (!NT_SUCCESS(ns))
			{
				ddk::status::LogStatus(ns);
				return false;
			}
			ns = RtlSetDaclSecurityDescriptor(&Se, TRUE, NULL, TRUE);
			if (!NT_SUCCESS(ns))
			{
				ddk::status::LogStatus(ns);
				return false;
			}

			OBJECT_ATTRIBUTES oa;
			UNICODE_STRING nsSecName;
			LARGE_INTEGER SectionSize = { 0 };
			SectionSize.QuadPart = PAGE_SIZE;
			auto full_map_name = std::wstring(L"\\BaseNamedObjects\\") + map_name;
			RtlInitUnicodeString(&nsSecName, full_map_name.c_str());
			InitializeObjectAttributes(&oa, &nsSecName, 
				OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE,
				NULL,
				&Se);

			ns = ZwCreateSection(
				&_hSection, 
				SECTION_ALL_ACCESS,
				&oa,
				&SectionSize,
				PAGE_READWRITE, 
				SEC_COMMIT,
				NULL);
			if (!NT_SUCCESS(ns))
			{
				LOG_DEBUG(__FUNCTION__);
				ddk::status::LogStatus(ns);
				return false;
			}
			return true;
		}
		bool open(HANDLE hFile)
		{
			OBJECT_ATTRIBUTES oa = {};
			InitializeObjectAttributes(&oa,
				nullptr,
				OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE,
				NULL,
				NULL);
			auto ns = ZwCreateSection(&_hSection,
				SECTION_ALL_ACCESS,
				&oa,
				nullptr,
				PAGE_READWRITE,
				SEC_COMMIT,
				hFile);
			if (NT_SUCCESS(ns))
			{
				return true;
			}
			LOG_DEBUG("map open handle\r\n");
			ddk::status::LogStatus(ns);
			return false;
		}
	public:
		bool unmap()
		{
			auto ns = ZwUnmapViewOfSection(_Process, _MemView);
			if (!NT_SUCCESS(ns))
			{
				if (ns== STATUS_INVALID_PAGE_PROTECTION)
				{
					//無法釋放的事情內存
					LOG_DEBUG("unmap %p STATUS_INVALID_PAGE_PROTECTION\r\n", _MemView);
				}
				ddk::status::LogStatus(ns);
				return false;
			}
			_MemView = nullptr;
			return true;
		}
	private:
		bool mapview(PVOID _baseAddress,SIZE_T _sizeMap) {
			_SizeToMap = _sizeMap;
			if (_MemView)
			{
				return true;
			}
			//PAGE_READWRITE
			LARGE_INTEGER SectionOffset = {0};
			SIZE_T ViewSize = _SizeToMap;
			LPVOID ViewBase = _baseAddress;
			ULONG Protect = PAGE_READWRITE;
			auto ns = ZwMapViewOfSection(_hSection,
				_Process,
				&ViewBase,
				0,
				0,
				&SectionOffset,
				&ViewSize,
				ViewShare,
				0,
				Protect);
			if (NT_SUCCESS(ns))
			{
				_MemView = ViewBase;
				_SizeToMap = ViewSize;
				LOG_DEBUG("mapped %p %d\r\n", _MemView, _SizeToMap);
				return true;
			}
			LOG_DEBUG("mapview %p\r\n", _hSection);
			ddk::status::LogStatus(ns);
			return false;
		}
	public:
		PVOID get_view(PVOID _base_addr=NULL,SIZE_T _sizeMap=0) {
			if (mapview(_base_addr,_sizeMap))
			{
				return _MemView;
			}
			return nullptr;
		}
		void flush()
		{
			if (_MemView)
			{
				auto BaseAddress = _MemView;
				SIZE_T FlushSize = _SizeToMap;
				IO_STATUS_BLOCK iocb = {};
				ZwFlushVirtualMemory(_Process, &BaseAddress, &FlushSize, &iocb);
			}
		}
	private:
		HANDLE _hSection;
		PVOID _MemView;
		HANDLE _Process;
		SIZE_T _SizeToMap;
	};
}
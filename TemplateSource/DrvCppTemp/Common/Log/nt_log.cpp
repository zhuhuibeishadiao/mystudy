#include "stdafx.h"

namespace ddk::log
{
	const auto LOG_ENTRY_MAX_LENGTH_IN_WCHARS = 520;
	const auto LOG_DIR = L"\\??\\Global\\C:\\$Extend\\$RmMetadata\\$NTLOG";
	const char utf_header[] = "\xFF\xFE";
	using LOG_DATA = struct
	{
		WCHAR szLogData[LOG_ENTRY_MAX_LENGTH_IN_WCHARS];
	};
	class nt_log :public ddk::Singleton<nt_log>
	{
	public:
		nt_log() {
			ddk::util::nt_mkdir(std::wstring(LOG_DIR));
			{
				TIME_FIELDS tf = {};
				LARGE_INTEGER time = {};
				LARGE_INTEGER ltime = {};
				wchar_t _fname[MAX_PATH] = {};
				KeQuerySystemTime(&time);
				ExSystemTimeToLocalTime(&time, &ltime);
				RtlTimeToTimeFields(&ltime, &tf);
				RtlStringCchPrintfW(_fname,
					MAX_PATH,
					L"%ws\\%d-%.2d-%.2d-%.2d-%.2d.log",
					LOG_DIR,
					tf.Year,
					tf.Month,
					tf.Day,
					tf.Hour,
					tf.Minute);
				if (!_logfile.open_if(std::wstring(_fname)))
				{
					LOG_DEBUG("create log failed\r\n");
				}
				size_t write_size = 0;
				_logfile.write((PVOID)utf_header, sizeof(utf_header) - 1, write_size);
				_logfile.writeline(std::wstring(L"LOG_FILE_BEGIN"));
			}
		}
		~nt_log()
		{
			_lock.wait_for_release();
			_logfile.writeline(std::wstring(L"LOG_FILE_END"));
			_logfile.close();
		}
	private:
		ddk::nt_file _logfile;
		ddk::nt_lock _lock;
	public:
		void logRoutine(LOG_DATA *pContext)
		{
			if (pContext)
			{
				log2file(pContext);
				free(pContext);
			}
			_lock.release();
		}
		void log2file(LOG_DATA *_data)
		{

			LARGE_INTEGER  SystemTime, LocalTime;
			TIME_FIELDS    TimeFields;
			WCHAR szString[LOG_ENTRY_MAX_LENGTH_IN_WCHARS * 2] = {};
			KeQuerySystemTime(&SystemTime);
			ExSystemTimeToLocalTime(&SystemTime, &LocalTime);
			RtlTimeToTimeFields(&LocalTime, &TimeFields);

			auto ns = RtlStringCchPrintfW(szString, LOG_ENTRY_MAX_LENGTH_IN_WCHARS * 2,
				L"%02u/%02u/%04u %02u:%02u:%02u.%03u, %ws\r\n",
				TimeFields.Day,
				TimeFields.Month,
				TimeFields.Year,
				TimeFields.Hour,
				TimeFields.Minute,
				TimeFields.Second,
				TimeFields.Milliseconds,
				_data->szLogData
			);

			if (NT_SUCCESS(ns))
			{
				SIZE_T Len = 0;
				SIZE_T write_size = 0;
				RtlStringCchLengthW(szString, LOG_ENTRY_MAX_LENGTH_IN_WCHARS * 2, &Len);
				_logfile.write(szString, sizeof(WCHAR)*Len, write_size);
			}
		}
	public:
		void pushLog(LOG_DATA _logData)
		{
			if (KeGetCurrentIrql() <= PASSIVE_LEVEL)
			{
				_lock.only_acquire();
				log2file(&_logData);
				_lock.release();
			}
			else
			{
				auto pCtx = malloc(sizeof(LOG_DATA));
				if (pCtx)
				{
					_lock.only_acquire();
					RtlCopyMemory(pCtx, &_logData, sizeof(LOG_DATA));
					auto work_item = ddk::work_item(DelayedWorkQueue,
						std::bind(
							&ddk::log::nt_log::logRoutine,
							this,
							(LOG_DATA *)pCtx));
					UNREFERENCED_PARAMETER(work_item);
				}
			}
		}
	};

}

namespace ddk
{
	void LogToFileA(LPCSTR lpszFormat, ...)
	{
		ddk::log::LOG_DATA _log = {};
		va_list args;
		va_start(args, lpszFormat);
		//	size_t wsize = 0;

		CHAR szAString[520] = {};
		auto stat = RtlStringCchVPrintfA(szAString, 520, lpszFormat, args);
		if (NT_SUCCESS(stat)) {
			auto forA = ddk::string::A2WEX(szAString);
			if (forA)
			{
				RtlStringCchCopyW(_log.szLogData, 520, forA);
				ddk::log::nt_log::getInstance().pushLog(_log);
				free(forA);
			}
		}
		va_end(args);
	}
	void LogToFileW(LPCWSTR lpszFormat, ...)
	{
		ddk::log::LOG_DATA _log = {};
		va_list args;
		va_start(args, lpszFormat);
		//size_t wsize = 0;
		auto stat = RtlStringCchVPrintfW(_log.szLogData, 520, lpszFormat, args);
		if (NT_SUCCESS(stat)) {
			ddk::log::nt_log::getInstance().pushLog(_log);
		}
		va_end(args);
	}
}
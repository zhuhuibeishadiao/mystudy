#pragma once
namespace ddk
{
	struct NtStatusInfo
	{
		const unsigned long m_code;
		const char *m_name;
		const char *m_description;
	};
	namespace status
	{
		NtStatusInfo *DecodeNtStatusInfo(NTSTATUS code);
		inline void LogStatus(NTSTATUS ns)
		{
			auto nsinfo = ddk::status::DecodeNtStatusInfo(ns);
			if (nsinfo)
				LOG_DEBUG("%s %s\r\n", nsinfo->m_name, nsinfo->m_description);

		}
	}
};
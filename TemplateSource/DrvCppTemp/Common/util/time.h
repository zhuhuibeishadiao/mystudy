#pragma once
namespace ddk::util
{
	void sleep(LONGLONG ltime);
	void get_now_time(std::wstring &wtime);
	void get_now_time(PTIME_FIELDS now_time);
	LONGLONG get_now_time();
	LONGLONG get_timestamp();
	LONGLONG get_tick_count();
	LONGLONG nano_seconds(LONG nanos);
	LONGLONG micro_seconds(LONG micros);
	LONGLONG milli_seconds(LONG millis);
	LONGLONG seconds(LONG sec);
	LONGLONG minutes(LONG minu);

};
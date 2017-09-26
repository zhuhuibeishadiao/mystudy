#include "stdafx.h"

namespace ddk::util
{
	void sleep(LONGLONG ltime)
	{
		LARGE_INTEGER sleep_time;
		sleep_time.QuadPart = -ltime;
		KeDelayExecutionThread(KernelMode,
			FALSE,
			&sleep_time);
	}
};
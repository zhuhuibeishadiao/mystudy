#include "stdafx.h"
#include "patchguard.h"

namespace ddk::patchguard
{
	void DisPg()
	{
		if (OsIndex < OsIndex_8)
		{
			Disable_Early();
		}
		else
		{
			dispg_new();
		}
		//全部采用此招术
		//dispg_new();
	}
}
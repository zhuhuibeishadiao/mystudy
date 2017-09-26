#pragma once
#include "stdafx.h"
#include "BaseHook.h"

//hook irp 
namespace ddk
{
	using IRP_SAVE = struct
	{
		PDRIVER_OBJECT pDrv;
		PDRIVER_DISPATCH old_function;
		USHORT IrpFunction;
	};
	class irp_hook :public Singleton<irp_hook>
	{
	public:
		irp_hook() {

		}
		~irp_hook() {
			_hooklock.wait_for_release();
			namespace DI = ::Detours::Internal;
			for (auto _hook : _save)
			{
				DI::AtomicCopy4X8(
					(uint8_t *)&(_hook.pDrv->MajorFunction[_hook.IrpFunction]),
					(uint8_t *)&_hook.old_function,
					sizeof(PVOID));
				ObDereferenceObject(_hook.pDrv);
			}
		}
	private:
		std::vector<IRP_SAVE> _save;
		ddk::nt_lock _hooklock;
	public:
		void acquire()
		{
			_hooklock.only_acquire();
		}
		void release()
		{
			_hooklock.release();
		}
	public:
		bool hook(
			LPCWSTR wscDriverName,
			USHORT IrpFunction,
			PDRIVER_DISPATCH NewFunction,
			PDRIVER_DISPATCH *OldFunction)
		{
			IRP_SAVE _s = {};
			_s.IrpFunction = IrpFunction;
			PDRIVER_OBJECT object = nullptr;
			if (!ddk::util::get_driver_object(wscDriverName, object))
			{
				return false;
			}
			if (!object)
			{
				return false;
			}
			auto old = object->MajorFunction[IrpFunction];
			_s.old_function = old;
			_s.pDrv = object;
			if (OldFunction)
			{
				*OldFunction = old;
			}
			auto bret = ::Detours::Internal::AtomicCopy4X8(
				(uint8_t *)&(object->MajorFunction[IrpFunction]),
				(uint8_t *)&NewFunction,
				sizeof(PVOID));
			if (bret)
			{
				_save.push_back(_s);
				return true;
			}
			return false;
		}
	public:

	};
};
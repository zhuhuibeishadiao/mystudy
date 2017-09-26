#pragma once
#include "stdafx.h"
#include "BaseHook.h"
namespace ddk
{
	class hook_engine:public Singleton<hook_engine>
	{
	public:
		hook_engine() {};
		~hook_engine() {
			_hooklock.wait_for_release();
			for (auto &_hook:hooks)
			{
				hook::DetourRemove(_hook.second);
			}
			hooks.clear();
		};
	private:
		std::map<PVOID,Detours::uint8_t*> hooks;
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
		bool inline_hook(PVOID Target, PVOID NewFunction, PVOID &OldFunction)
		{
			OldFunction = nullptr;
			auto _target = (Detours::uint8_t*)Target;
			auto _dest = (Detours::uint8_t*)NewFunction;
			auto _old = hook::DetourFunction(_target, _dest);
			if (!_old)
			{
				return false;
			}
			OldFunction = (PVOID)_old;
			hooks[Target] = _old;
			return true;
		}
		bool unhook_target(PVOID Target)
		{
			auto _hook = hooks.find(Target);
			if (_hook!=hooks.end())
			{
				auto bret= hook::DetourRemove(_hook->second);
				if (bret)
				{
					hooks.erase(Target);
				}
				return bret;
			}
			return false;
		}
		bool unhook_proc(PVOID OldFunction)
		{
			auto _old = (Detours::uint8_t*)OldFunction;
			for (auto &_hook:hooks)
			{
				if (_hook.second == _old)
				{
					auto b_ret = hook::DetourRemove(_old);
					if (b_ret)
					{
						hooks.erase(_hook.first);
					}
					return b_ret;
				}
			}
			return false;
		}
	};
}
#pragma once
namespace ddk::callback
{
	class nt_power_callback
	{
	public:
		nt_power_callback() {
			_setpower = _power.open(L"\\Callback\\PowerState");
		};
		~nt_power_callback() {};
	public:
		bool set_callback(ddk::callback::nt_excallback::nt_excallback_type _function)
		{
			return _power.set_callback(_function);
		}
	private:
		nt_excallback _power;
		bool _setpower;
	};
};
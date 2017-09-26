#pragma once

namespace ddk::callback {

	class nt_obj_process_callback
	{
	public:
		nt_obj_process_callback() { };
		~nt_obj_process_callback() {};
	private:
		nt_object_callback _process;
	public:
		bool init(ULONG Altitude)
		{
			return _process.init(PsProcessType, Altitude);
		}
	public:
		bool set_pre_callback(nt_object_callback::nt_object_pre_callback_type _preCall)
		{
			return _process.set_pre_callback(_preCall);
		}
		bool set_post_callback(nt_object_callback::nt_object_post_callback_type _postCall)
		{
			return _process.set_post_callback(_postCall);
		}
	};
};
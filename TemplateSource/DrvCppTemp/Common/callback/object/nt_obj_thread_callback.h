#pragma once
namespace ddk::callback {
	class nt_obj_thread_callback
	{
	public:
		nt_obj_thread_callback() { };
		~nt_obj_thread_callback() {};
	private:
		nt_object_callback _thread;
	public:
		bool init(ULONG Altitude)
		{
			return _thread.init(PsThreadType, Altitude);
		}
	public:
		bool set_pre_callback(nt_object_callback::nt_object_pre_callback_type _preCall)
		{
			return _thread.set_pre_callback(_preCall);
		}
		bool set_post_callback(nt_object_callback::nt_object_post_callback_type _postCall)
		{
			return _thread.set_post_callback(_postCall);
		}
	};
};
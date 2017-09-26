#pragma once
namespace ddk::callback
{
	class nt_obj_file_callback
	{
	public:
		nt_obj_file_callback(){}
		~nt_obj_file_callback(){}
	private:
		nt_object_callback _file;
	public:
		bool init(ULONG Altitude)
		{
			return _file.init(IoFileObjectType, Altitude);
		}
		bool set_pre_callback(nt_object_callback::nt_object_pre_callback_type _preCall)
		{
			return _file.set_pre_callback(_preCall);
		}
		bool set_post_callback(nt_object_callback::nt_object_post_callback_type _postCall)
		{
			return _file.set_post_callback(_postCall);
		}
	};

}
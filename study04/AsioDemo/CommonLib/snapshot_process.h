#pragma once
namespace usr::util::snapshot
{
	using process_item = struct 
	{
		DWORD_PTR ProcessId;
		_tstring process_name;
	};
	void get_snapshot_process(std::vector<process_item> &_process);
}
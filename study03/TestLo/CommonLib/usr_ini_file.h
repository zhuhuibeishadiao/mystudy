#pragma once
#include "stdafx.h"
#pragma warning(disable:4018)
#pragma warning(disable:4267)
namespace usr
{
	using ini_item = std::map<_tstring, _tstring>;
	class ini_file
	{
	public:
		ini_file():ini_file_name(),nSubNameCount(0),nItemCount(0){};
		ini_file(_tstring _inifilename) {
			ini_file_name = _inifilename;
			load_ini();
		}
		~ini_file() {};
	public:
		ini_file & operator =(const ini_file &_ini)
		{
			this->ini_file_name = _ini.ini_file_name;
			this->load_ini();
			return *this;
		}
	private:
		void load_ini()
		{
			nSubNameCount = nItemCount = 0;
			auto temp1 = GetPrivateProfileInt(_T("IniConfig"), _T("SubNameCount"), 0, ini_file_name.c_str());
			auto temp2 = GetPrivateProfileInt(_T("IniConfig"), _T("ItemCount"), 0, ini_file_name.c_str());
			for (auto i=0;i<temp1;i++)
			{
				TCHAR szNum[10] = {};
				_itot_s(i, szNum, 10);
				_tstring Sub = _tstring(_T("SubName")) + szNum;
				TCHAR szSubNameString[MAX_PATH] = {};
				GetPrivateProfileString(_T("SubSetting"), Sub.c_str(), _T("noName"), szSubNameString, sizeof(szSubNameString), ini_file_name.c_str());
				if (_tcsicmp(szSubNameString,_T("noname"))!=0)
				{
					add_sub(_tstring(szSubNameString));
				}
			}
			for (auto i=0;i<temp2;i++)
			{
				TCHAR szNum[10] = {};
				_itot_s(i, szNum, 10);
				_tstring ItemName = _tstring(_T("Item")) + szNum;
				ini_item item;
				for (auto subName:SubNameList)
				{
					TCHAR szItemString[MAX_PATH] = {};
					GetPrivateProfileString(ItemName.c_str(), subName.c_str(), _T("noData"), szItemString, sizeof(szItemString), ini_file_name.c_str());
					item[subName] = szItemString;
				}
				insert(item);
			}
			nSubNameCount = SubNameList.size();
			nItemCount = IniData.size();
		}
	private:
		void WriteInt(LPCTSTR Section, LPCTSTR ValueName, int Value)
		{
			TCHAR szValue[MAX_PATH] = {};
			_itot_s(Value, szValue, 10);
			WritePrivateProfileString(Section,ValueName,szValue,
				ini_file_name.c_str());
		}
		void WriteString(LPCTSTR Section, LPCTSTR ValueName, LPCTSTR ValueString)
		{
			WritePrivateProfileString(Section, ValueName, ValueString,
				ini_file_name.c_str());
		}
	public:
		void save_ini()
		{
			nSubNameCount = SubNameList.size();
			WriteInt(_T("IniConfig"), _T("SubNameCount"), nSubNameCount);
			nItemCount = IniData.size();
			WriteInt(_T("IniConfig"), _T("ItemCount"), nItemCount);

			for (size_t i=0;i<SubNameList.size();i++)
			{
				TCHAR szNum[10] = {};
				_itot_s(i, szNum, 10);
				_tstring Sub = _tstring(_T("SubName")) + szNum;
				WriteString(_T("SubSetting"), Sub.c_str(), SubNameList[i].c_str());
			}

			for (size_t i=0;i<IniData.size();i++)
			{
				TCHAR szNum[10] = {};
				_itot_s(i, szNum, 10);
				_tstring ItemName = _tstring(_T("Item")) + szNum;
				auto Itemx = IniData[i];
				for (auto sub:SubNameList)
				{
					WriteString(ItemName.c_str(), sub.c_str(), Itemx[sub].c_str());
				}
			}
		}
	public:
		void get_sub_list(std::vector<_tstring> &_list)
		{
			_list = SubNameList;
		}
		void set_sub_list(std::vector<_tstring> _list)
		{
			SubNameList = _list;
			nSubNameCount = SubNameList.size();
		}
		int size() {
			return IniData.size();
		}
		void add_sub(_tstring subname)
		{
			auto p = std::find(SubNameList.cbegin(), SubNameList.cend(), subname);
			if (p!=SubNameList.cend())
			{
				return;
			}
			SubNameList.push_back(subname);
			nSubNameCount++;
		}
		void insert(ini_item item)
		{
			IniData.push_back(item);
			nItemCount = IniData.size();
		}
		auto get()
		{
			return IniData;
		}
		void set(std::vector<ini_item> _data)
		{
			IniData = _data;
		}
	private:
		_tstring ini_file_name;
		int nSubNameCount;
		int nItemCount;
		std::vector<_tstring> SubNameList;
		std::vector<ini_item> IniData;
	};
}
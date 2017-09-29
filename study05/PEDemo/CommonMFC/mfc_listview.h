#pragma once
#include "stdafx.h"
#include "afxcmn.h"

namespace mfc
{
	using _listview_item = std::map<_tstring, _tstring>;
	using _listview_list = std::vector<_listview_item>;
	class listview
	{
	public:
		listview() :_listctrl(nullptr), _nColumnCount(0)
		{};
		~listview() {};
		listview(CListCtrl*_list) :_listctrl(_list) {
			_nColumnCount = 0;
			init_list();
		};
	public:
		//划分XX,自动化处理宽度根据文本大小*100
		void add_column(_tstring _columnName, UINT _width = 0)
		{
			_MapColumnToString[_nColumnCount] = _columnName;
			_MapStringToColumn[_columnName] = _nColumnCount;
			auto _min_width = _columnName.size() * 40;
			_listctrl->InsertColumn(_nColumnCount, _columnName.c_str(), LVCFMT_LEFT, max(_width, _min_width));
			_nColumnCount++;
		}
	public:
		static int CALLBACK list_compare(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
		{
			CListCtrl* pListCtrl = (CListCtrl*)lParamSort;
			CString strItem1 = pListCtrl->GetItemText(lParam1, 0);
			CString strItem2 = pListCtrl->GetItemText(lParam2, 0);


			LPCTSTR s1 = (LPCTSTR)strItem1;
			LPCTSTR s2 = (LPCTSTR)strItem2;
			
			auto i= _tcscmp(s2, s1);
			if (i>0)
			{
				return -1;
			}
			if (i<0)
			{
				return 1;
			}
			return 0;
		}
		//插入item
		template<typename ...ARGS>
		void insert(ARGS &&...args)
		{
			auto i = 0;
			auto _item_index = 0;
			_tstring xitem[] = { std::forward<ARGS>(args)... };
			for (auto _p : xitem)
			{
				if (i == 0)
				{
					_item_index = _listctrl->InsertItem(0, xitem[i].c_str());
				}
				else
				{
					_listctrl->SetItemText(_item_index, i, xitem[i].c_str());
				}
				i++;
			}
			sort();
		}
	private:
		void sort()
		{
			_listctrl->SortItemsEx(list_compare, (DWORD_PTR)_listctrl);
		}
		int get_subitemIndex(_tstring item_name)
		{
			return _MapStringToColumn[item_name];
		}
	public:
		//获取选中
		void getSelected(_listview_list &_selected)
		{
			for (int i = 0; i < _listctrl->GetItemCount(); i++)
			{
				if (_listctrl->GetItemState(i, LVIS_SELECTED) == LVIS_SELECTED
					|| _listctrl->GetCheck(i))
				{
					_listview_item _item = {};
					for (auto subItem = 0; subItem < _nColumnCount; subItem++)
					{
						TCHAR szSubItem[MAX_PATH] = {};
						_listctrl->GetItemText(i, subItem, szSubItem, MAX_PATH);
						_item[_MapColumnToString[subItem]] = _tstring(szSubItem);
					}
					_selected.push_back(_item);
				}
			}
		}
		//删除选中
		void delSelected()
		{
			auto bfind = false;
			do {
				bfind = false;
				for (int i = 0; i < _listctrl->GetItemCount(); i++)
				{
					if (_listctrl->GetItemState(i, LVIS_SELECTED) == LVIS_SELECTED
						|| _listctrl->GetCheck(i))
					{
						_listctrl->DeleteItem(i);
						bfind = true;
						break;
					}
				}
			} while (bfind);
		}
		//保存整个表到一个_listview_list
		void save(_listview_list &_saveList)
		{
			for (int i = 0; i < _listctrl->GetItemCount(); i++)
			{
				_listview_item _item = {};
				for (auto subItem = 0; subItem < _nColumnCount; subItem++)
				{
					TCHAR szSubItem[MAX_PATH] = {};
					_listctrl->GetItemText(i, subItem, szSubItem, MAX_PATH);
					_item[_MapColumnToString[subItem]] = _tstring(szSubItem);
				}
				_saveList.push_back(_item);
			}
		}
		//加载_listview_list
		void load(_listview_list _listdata)
		{
			for (auto _item : _listdata)
			{
				auto Index = _listctrl->InsertItem(0, _T(""));
				for (auto _xitem : _item)
				{
					auto subItemName = _xitem.first;
					auto subItemText = _xitem.second;
					auto subIndex = get_subitemIndex(subItemName);
					_listctrl->SetItemText(Index, subIndex, subItemText.c_str());
				}
			}
			sort();
		}
		void get_column(std::vector<_tstring> &_list)
		{
			_list.resize(_nColumnCount);
			for (auto i =0;i<_nColumnCount;i++)
			{
				_list[i] = _MapColumnToString[i];
			}
		}
	public:
		//防闪烁
		void supend_redraw()
		{
			_listctrl->SetRedraw(FALSE);
		}
		void resume_redraw()
		{
			_listctrl->SetRedraw(TRUE);
			_listctrl->Invalidate();
			_listctrl->UpdateWindow();
		}
	public:
		void clear()
		{
			//删除整个listview
			_listctrl->DeleteAllItems();
			while (_listctrl->DeleteColumn(0));
			_nColumnCount = 0;
			_MapColumnToString.clear();
			_MapStringToColumn.clear();
		}
	public:
		listview & operator =(const listview &_ll)
		{
			this->_listctrl = _ll._listctrl;
			this->_MapColumnToString = _ll._MapColumnToString;
			this->_MapStringToColumn = _ll._MapStringToColumn;
			this->_nColumnCount = _ll._nColumnCount;
			return *this;
		}
	private:
		CListCtrl *_listctrl;
		std::map<int, _tstring> _MapColumnToString;
		std::map<_tstring, int> _MapStringToColumn;
	private:
		void init_list()
		{
			if (_listctrl == nullptr)
			{
				RaiseException(0x80000003L, 0, 0, nullptr);
				return;
			}
			_nColumnCount = 0;
			_MapStringToColumn.clear();
			_MapColumnToString.clear();

			LONG lStyle;
			lStyle = GetWindowLong(_listctrl->m_hWnd, GWL_STYLE);//获取当前窗口style
			lStyle &= ~LVS_TYPEMASK; //清除显示方式位
			lStyle |= LVS_REPORT; //设置style
			lStyle |= LVS_SINGLESEL;//单选
			SetWindowLong(_listctrl->m_hWnd, GWL_STYLE, lStyle);//设置style
			
			DWORD dwStyle = _listctrl->GetExtendedStyle();
			dwStyle |= LVS_EX_FULLROWSELECT;//选中某行使整行高亮（只适用与report风格的listctrl）
			dwStyle |= LVS_EX_GRIDLINES;//网格线（只适用与report风格的listctrl）
			dwStyle |= LVS_EX_CHECKBOXES;//item前生成checkbox控件
			_listctrl->SetExtendedStyle(dwStyle); //设置扩展风格
		}
	private:
		int _nColumnCount;
	};
}

#pragma once
#include "stdafx.h"

namespace mfc
{

	class TabCtrl {
	private:
		CTabCtrl *m_tab;
		DWORD m_tabCnts = 0;
		std::vector<CDialogEx *> listDlgs;
	public:
		TabCtrl(){}
		TabCtrl(CTabCtrl *tabCtl):m_tab(tabCtl){}
		~TabCtrl(){}
		TabCtrl * operator =(const TabCtrl *tab)
		{
			this->m_tab = tab->m_tab;
			return this;
		}
	public:
		void insertPage(int resID, const WCHAR *title, CDialogEx *page)
		{
			m_tab->InsertItem(m_tabCnts++, title);
			page->Create(MAKEINTRESOURCE(resID), m_tab);

			CRect rc;
			m_tab->GetClientRect(&rc);
			rc.top += 25;
			//page->SetWindowPos(NULL, rc.left, rc.top, rc.right - 300, rc.bottom - 200, SWP_NOCOPYBITS);
			page->MoveWindow(rc);
			
			if (m_tabCnts == 1)
				page->ShowWindow(SW_SHOW);
			listDlgs.push_back(page);
			m_tabCnts++;
		}

		void tabChange()
		{
			auto index = m_tab->GetCurSel();
			for_each(listDlgs.begin(), listDlgs.end(), [&](CDialogEx *dlg) {
				dlg->ShowWindow(SW_HIDE);
			});
			listDlgs[index]->ShowWindow(SW_SHOW);
		}

	};
}

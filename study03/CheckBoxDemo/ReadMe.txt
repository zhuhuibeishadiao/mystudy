==========================================
    CheckBoxDemo 项目概述
==========================================
主要逻辑在BOOL CCheckBoxDemoDlg::OnEraseBkgnd(CDC* pDC)
1.CheckBox和Radio 实际上都是button（单选框无法添加到变量中，只能通过GetDlgItem）
2.GetCheck查看当前选中状态，SetCheck可以设置选中状态（可以在初始化中使用）
3.学了一个WM_ERASEBKGND

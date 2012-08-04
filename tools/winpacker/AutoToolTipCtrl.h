#ifndef __AUTOTOOLTIPCTRL_H__
#define __AUTOTOOLTIPCTRL_H__

class CAutoToolTipCtrl : public CToolTipCtrl
{
public:

	BOOL	AddWindowTool(CWnd* pWnd, DWORD nStrID)
	{
		return AddWindowTool(pWnd, MAKEINTRESOURCE(nStrID));
	}

	BOOL	AddWindowTool(CWnd* pWnd, LPCTSTR pszTooltip)
	{
		TOOLINFO ti;
		ti.cbSize	= sizeof(TOOLINFO);
		ti.uFlags	= TTF_SUBCLASS | TTF_IDISHWND;
		ti.hwnd		= pWnd->GetParent()->GetSafeHwnd();
		ti.uId		= (UINT)pWnd->GetSafeHwnd();
		ti.hinst	= AfxGetInstanceHandle();
		ti.lpszText	= (LPTSTR)pszTooltip;

		return (BOOL)SendMessage(TTM_ADDTOOL, 0, (LPARAM)&ti);
	}

private:

};

#endif

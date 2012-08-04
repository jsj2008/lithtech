// ----------------------------------------------------------------------- //
//
// MODULE  : BaseMenu.h
//
// PURPOSE : Base Class for In-game Menus
//
// CREATED : 3/20/00
//
// ----------------------------------------------------------------------- //


#ifndef __BASE_MENU_H__
#define __BASE_MENU_H__

class CLTMenuWnd;

class CBaseMenu
{
public:
	CBaseMenu();

	virtual void	Init(CLTMenuWnd *pMenuWnd) {m_pMenuWnd = pMenuWnd;}
    virtual void    Show(LTBOOL bShow);
    virtual void    Select(uint8 byItem) {}

protected:
	virtual void	Build() {}
	CLTMenuWnd *m_pMenuWnd;

};

#endif
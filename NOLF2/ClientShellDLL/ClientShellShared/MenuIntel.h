// ----------------------------------------------------------------------- //
//
// MODULE  : MenuIntel.h
//
// PURPOSE : In-game intel item menu
//
// (c) 2001 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //


#if !defined(_MENU_INTEL_H_)
#define _MENU_INTEL_H_

#include "BaseMenu.h"
#include "PopupText.h"

class CMenuIntel : public CBaseMenu
{
public:

	virtual LTBOOL	Init();
	virtual void	Term();

	virtual void OnFocus(LTBOOL bFocus);

	virtual LTBOOL  OnUp ( );
    virtual LTBOOL  OnDown ( );
    virtual LTBOOL  OnLeft ( );
    virtual LTBOOL  OnRight ( );
    virtual LTBOOL  OnEnter ( );
    virtual LTBOOL	OnEscape ();

    virtual LTBOOL  HandleKeyDown(int key, int rep);
    virtual LTBOOL  OnLButtonDown(int x, int y);
	
	// Render the control
	virtual void Render ();

	// Handle a command
    virtual uint32 OnCommand(uint32 nCommand, uint32 nParam1, uint32 nParam2);
private:
	void GetIntelName(uint32 nTextId, char *pBuf, int nBufSize);

	CPopupText		m_PopupText;
	void			ClosePopup();
};

#endif //!defined(_MENU_INTEL_H_)
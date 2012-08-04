
// ----------------------------------------------------------------------- //
//
// MODULE  : HUDInventoryList.h
//
// PURPOSE : Definition of an inventory list HUD element
//
// CREATED : 12/16/03
//
// (c) 1999-2003 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __HUDINVENTORYLIST_H__
#define __HUDINVENTORYLIST_H__


#include "HUDInventory.h"

class CHUDInventoryList : public CHUDItem
{
public:
	CHUDInventoryList();
	virtual ~CHUDInventoryList() {}
	
	virtual void	Term();

    virtual void	Render();
	virtual void	Update();
	virtual void	ScaleChanged();

	virtual void	UpdateFade();
	virtual void	ResetFade();
	virtual void	StartFlicker();
	virtual void	UpdateFlicker();
	virtual void	EndFlicker();

	virtual void	UpdateFlash();

	virtual void	UpdatePos();

protected:

	typedef std::vector<CHUDInventory*, LTAllocator<CHUDInventory*, LT_MEM_TYPE_CLIENTSHELL> > InventoryArray;
	InventoryArray	m_Items;

	LTVector2n		m_vItemOffset;

};

#endif  // __HUDINVENTORYLIST_H__

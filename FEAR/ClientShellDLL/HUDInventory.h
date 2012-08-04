// ----------------------------------------------------------------------- //
//
// MODULE  : HUDInventory.h
//
// PURPOSE : generic HUD display for an inventory item
//
// (c) 2003 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __HUDINVENTORY_H__
#define __HUDINVENTORY_H__

#include "HUDItem.h"
#include "idatabasemgr.h"

class CHUDInventory : public CHUDItem
{
public:
	CHUDInventory();
	virtual ~CHUDInventory() {}

    virtual void	Render();
	virtual void	SetBasePos(const LTVector2n& pos);

protected:
	//status
	bool			m_bDraw;
	uint32			m_nCount;
	uint32			m_nThreshold;


};


#endif
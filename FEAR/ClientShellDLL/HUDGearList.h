// ----------------------------------------------------------------------- //
//
// MODULE  : HUDGearList.h
//
// PURPOSE : HUD Element displaying a list of gear items
//
// CREATED : 12/16/03
//
// (c) 1999-2003 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __HUDGEARLIST_H__
#define __HUDGEARLIST_H__

#include "HUDInventoryList.h"
#include "HUDGear.h"

class CHUDGearList : public CHUDInventoryList
{
public:
	CHUDGearList();
	virtual ~CHUDGearList() {}

	static CHUDGearList& Instance();

	virtual bool Init();
	virtual void UpdateLayout();

protected:
	static CHUDGearList* g_pGearList;
};

#endif  // __HUDGEARLIST_H__

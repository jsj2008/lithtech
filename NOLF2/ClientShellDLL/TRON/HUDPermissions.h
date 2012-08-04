// ----------------------------------------------------------------------- //
//
// MODULE  : HUDPermissions.h
//
// PURPOSE : HUDItem to display permission sets
//
// (c) 2001 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __HUD_PERMISSIONS_H
#define __HUD_PERMISSIONS_H

#include "HUDItem.h"

//******************************************************************************************
//** HUD Ratings display
//******************************************************************************************
class CHUDPermissions : public CHUDItem
{
public:
	CHUDPermissions();

    LTBOOL      Init();
	void		Term();

    void        Render();
    void        Update();

    void        UpdateLayout();

private:
	float		m_fScale;
	uint8		m_nPermissions;
	uint8		m_nRequiredPermissions;

	LT_POLYFT4	m_Base[8];
	LT_POLYFT4	m_Have[8];
	LT_POLYFT4	m_Need[8];
	HTEXTURE	m_hBaseTex;
	HTEXTURE	m_hHaveTex[8];
	HTEXTURE	m_hNeedTex[8];
};

#endif
// ----------------------------------------------------------------------- //
//
// MODULE  : HUDDamageDir.h
//
// PURPOSE : HUDItem to display directional damage info
//
// (c) 2002 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __HUD_DAMAGE_DIR_H
#define __HUD_DAMAGE_DIR_H

#include "HUDItem.h"



//******************************************************************************************
//** HUD directional damage display
//******************************************************************************************
class CHUDDamageDir : public CHUDItem
{
public:
	CHUDDamageDir();

    LTBOOL      Init();
	void		Term();

    void        Render();
    void        Update();

    void        UpdateLayout();

	void		UpdateScale();

private:
	uint16		m_nSize;

	LTBOOL		m_bDraw;
	float		m_fScale;

	enum eConstants
	{
		kNumDamageSectors = 12,
	};


	LTPoly_GT4	m_Poly[kNumDamageSectors];

	HTEXTURE	m_hArrow;


};

#endif
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

	virtual bool	Init();
	virtual void	Term();

	virtual void	Render();
	virtual void	Update();
	virtual void	ScaleChanged();		

	virtual void	UpdateLayout();

private:
	uint32		m_nInnerRadius;
	uint32		m_nOuterRadius;
	float		m_fMinAlpha;
	float		m_fMaxAlpha;

	bool		m_bDraw;

	enum eConstants
	{
		kNumDamageSectors = 12,
	};


	LTPoly_GT4	m_Poly[kNumDamageSectors];

};

#endif
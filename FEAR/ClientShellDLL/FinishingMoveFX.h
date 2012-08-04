// ----------------------------------------------------------------------- //
//
// MODULE  : FinishingMoveFX.h
//
// PURPOSE : FinishingMoveFX - Definition
//
// CREATED : 03/30/05
//
// (c) 2005 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __FINISHINGMOVEFX_H__
#define __FINISHINGMOVEFX_H__

// ----------------------------------------------------------------------- //

#include "SpecialMoveFX.h"

// ----------------------------------------------------------------------- //

class CFinishingMoveFX : public CSpecialMoveFX
{
public:

	CFinishingMoveFX();
	virtual ~CFinishingMoveFX();

	virtual bool Update();
	virtual bool Init( HLOCALOBJ hServObj, ILTMessage_Read* pMsg );
	virtual uint32 GetSFXID() { return SFX_FINISHINGMOVE_ID; }
	virtual bool ShouldDisableWeapons() const { return true; }	//!!ARL: Add random chance according to player's current weapon.

protected:
	void UpdatePosition();
};

// ----------------------------------------------------------------------- //

#endif//__FINISHINGMOVEFX_H__

// ----------------------------------------------------------------------- //
//
// MODULE  : LadderFX.h
//
// PURPOSE : 
//
// CREATED : 06/21/04
//
// (c) 1999-2004 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __LADDERFX_H__
#define __LADDERFX_H__

#include "SpecialFX.h"


class CLadderFX : public CSpecialFX
{
	public :
		CLadderFX();
		virtual ~CLadderFX();

		virtual bool Init(HLOCALOBJ hServObj, ILTMessage_Read *pMsg);
		virtual uint32 GetSFXID() { return SFX_LADDER_ID; }

		const LTVector& GetTop() const { return m_vTop; }
		const LTVector& GetBottom() const { return m_vBottom; }
		const LTRotation& GetRotation() const { return m_rRot; }

		SurfaceType GetSurfaceOverride() const { return m_eSurfaceOverrideType; }




	private :
		LTRotation	m_rRot;
		LTVector	m_vTop;
		LTVector	m_vBottom;
		SurfaceType	m_eSurfaceOverrideType;

};

#endif // __LADDERFX_H__
// ----------------------------------------------------------------------- //
//
// MODULE  : JumpVolumeFX.h
//
// PURPOSE : JumpVolume special fx class - Definition
//
// CREATED : 1/24/02
//
// (c) 2002 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __JUMP_VOLUME_FX_H__
#define __JUMP_VOLUME_FX_H__

//
// Includes...
//

	#include "SpecialFX.h"


class CJumpVolumeFX : public CSpecialFX
{
	public : // Methods...

		CJumpVolumeFX();
		~CJumpVolumeFX();

		virtual LTBOOL Init( HLOCALOBJ hServObj, ILTMessage_Read *pMsg );
		virtual LTBOOL OnServerMessage( ILTMessage_Read *pMsg );

		virtual uint32 GetSFXID() { return SFX_JUMPVOLUME_ID; }

		inline LTVector GetVelocity() const { return m_vVelocity; }

	protected : // Members...

		LTVector	m_vVelocity;
};

#endif // __JUMP_VOLUME_FX_H__
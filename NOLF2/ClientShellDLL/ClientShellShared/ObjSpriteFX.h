// ----------------------------------------------------------------------- //
//
// MODULE  : ObjSpriteFX.h
//
// PURPOSE : ObjSprite special fx class - Definition
//
// CREATED : 9/12/00
//
// (c) 1999 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __OBJSPRITE_FX_H__
#define __OBJSPRITE_FX_H__

#include "SpecialFX.h"
#include "SmokeFX.h"
#include "SharedFXStructs.h"

class CObjSpriteFX : public CSpecialFX
{
	public :

		CObjSpriteFX();
		~CObjSpriteFX() {};

        virtual LTBOOL Update();
		virtual	LTBOOL Init(SFXCREATESTRUCT* psfxCreateStruct);
		virtual LTBOOL Init(HLOCALOBJ hServObj, ILTMessage_Read *pMsg);

		virtual uint32 GetSFXID() { return SFX_OBJSPRITE_ID; }

	protected :

		// Creation data...

		SFXCREATESTRUCT		m_cs;		// Holds all initialization data

};

#endif // __OBJSPRITE_FX_H__
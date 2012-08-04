 // ----------------------------------------------------------------------- //
//
// MODULE  : SpriteFX.h
//
// PURPOSE : Sprite special fx class - Definition
//
// CREATED : 5/22/98
//
// ----------------------------------------------------------------------- //

#ifndef __SPRITE_FX_H__
#define __SPRITE_FX_H__

#include "BaseScaleFX.h"

struct SPRITECREATESTRUCT : public BSCREATESTRUCT
{
};

class CSpriteFX : public CBaseScaleFX
{
	public :

		CSpriteFX() : CBaseScaleFX() 
		{
			m_nType = OT_SPRITE;
		}

		virtual LTBOOL Init(SFXCREATESTRUCT* psfxCreateStruct);
};

#endif // __SPRITE_FX_H__
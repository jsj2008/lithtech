// ----------------------------------------------------------------------- //
//
// MODULE  : SpriteFX.cpp
//
// PURPOSE : Sprite special FX - Implementation
//
// CREATED : 5/22/98
//
// ----------------------------------------------------------------------- //

#include "SpriteFX.h"
#include "clientheaders.h"

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CSpriteFX::Init
//
//	PURPOSE:	Create the sprite
//
// ----------------------------------------------------------------------- //

LTBOOL CSpriteFX::Init(SFXCREATESTRUCT* psfxCreateStruct)
{
	return CBaseScaleFX::Init(psfxCreateStruct);
}

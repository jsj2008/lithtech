// ----------------------------------------------------------------------- //
//
// MODULE  : ObjSpriteFX.cpp
//
// PURPOSE : ObjSprite special FX - Implementation
//
// CREATED : 10/19/99
//
// (c) 1999 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "ObjSpriteFX.h"
#include "VarTrack.h"
#include "GameClientShell.h"

extern CGameClientShell*	g_pGameClientShell;

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CObjSpriteFX::CObjSpriteFX
//
//	PURPOSE:	Construct
//
// ----------------------------------------------------------------------- //

CObjSpriteFX::CObjSpriteFX() : CSpecialFX()
{
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CObjSpriteFX::Init
//
//	PURPOSE:	Construct
//
// ----------------------------------------------------------------------- //

LTBOOL CObjSpriteFX::Init(SFXCREATESTRUCT* psfxCreateStruct)
{

	m_cs = *psfxCreateStruct;

    return LTTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CObjSpriteFX::Init
//
//	PURPOSE:	Init the object sprite fx
//
// ----------------------------------------------------------------------- //

LTBOOL CObjSpriteFX::Init(HLOCALOBJ hServObj, ILTMessage_Read *pMsg)
{
    if (!CSpecialFX::Init(hServObj, pMsg)) return LTFALSE;
    if (!pMsg) return LTFALSE;

	SFXCREATESTRUCT scs;

	scs.hServerObj = hServObj;
    scs.Read(pMsg);

	return Init(&scs);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CObjSpriteFX::Update
//
//	PURPOSE:	Update the particle system
//
// ----------------------------------------------------------------------- //

LTBOOL CObjSpriteFX::Update()
{
    if (m_bWantRemove) return LTFALSE;

	return CSpecialFX::Update();
}
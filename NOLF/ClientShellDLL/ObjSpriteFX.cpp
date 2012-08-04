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
#include "iltcustomdraw.h"

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


	OBJSPRITECREATESTRUCT* pCS = (OBJSPRITECREATESTRUCT*)psfxCreateStruct;

	m_cs = *pCS;

    return LTTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CObjSpriteFX::Init
//
//	PURPOSE:	Init the object sprite fx
//
// ----------------------------------------------------------------------- //

LTBOOL CObjSpriteFX::Init(HLOCALOBJ hServObj, HMESSAGEREAD hMessage)
{
    if (!CSpecialFX::Init(hServObj, hMessage)) return LTFALSE;
    if (!hMessage) return LTFALSE;

	OBJSPRITECREATESTRUCT scs;

	scs.hServerObj = hServObj;
    scs.Read(g_pLTClient, hMessage);

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


	if (g_pGameClientShell->GetGameType() == COOPERATIVE_ASSAULT && m_cs.m_nPlayerTeamFilter > 0)
	{
		uint32 dwFlags = g_pLTClient->GetObjectFlags(m_hServerObject);
		CLIENT_INFO* pLocalInfo = g_pInterfaceMgr->GetClientInfoMgr()->GetLocalClient();
		if (pLocalInfo)
		{
			if (pLocalInfo->team == m_cs.m_nPlayerTeamFilter)
			{
				dwFlags |= FLAG_VISIBLE;
			}
			else
			{
				dwFlags &= ~FLAG_VISIBLE;
			}

			g_pLTClient->SetObjectFlags(m_hServerObject,dwFlags);
		}
	}

	return CSpecialFX::Update();
}
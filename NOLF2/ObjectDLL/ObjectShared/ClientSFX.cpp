// ----------------------------------------------------------------------- //
//
// MODULE  : CClientSFX.cpp
//
// PURPOSE : CClientSFX - Base class for client-side sfx objects - Implementation
//
// CREATED : 10/21/97
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "ClientSFX.h"
#include "CommandMgr.h"

LINKFROM_MODULE( ClientSFX );

#pragma force_active on
BEGIN_CLASS(CClientSFX)
END_CLASS_DEFAULT_FLAGS(CClientSFX, GameBase, NULL, NULL, CF_HIDDEN)
#pragma force_active off

CMDMGR_BEGIN_REGISTER_CLASS( CClientSFX )
CMDMGR_END_REGISTER_CLASS( CClientSFX, GameBase )

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CClientSFX::CClientSFX
//
//	PURPOSE:	Initialize id data member
//
// ----------------------------------------------------------------------- //

CClientSFX::CClientSFX(uint8 nType) : GameBase(nType)
{
}
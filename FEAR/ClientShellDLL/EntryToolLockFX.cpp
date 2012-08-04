// ----------------------------------------------------------------------- //
//
// MODULE  : EntryToolLockFX.cpp
//
// PURPOSE : EntryToolLockFX - Implementation
//
// CREATED : 10/18/04
//
// (c) 1999-2005 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "EntryToolLockFX.h"

// ----------------------------------------------------------------------- //

void ENTRYTOOLLOCKCREATESTRUCT::Read( ILTMessage_Read* pMsg )
{
	m_rEntryTool = pMsg->ReadDatabaseRecord( g_pLTDatabase, g_pWeaponDB->GetWeaponsCategory() );
	m_bPosition = pMsg->Readbool();
}

// ----------------------------------------------------------------------- //

CEntryToolLockFX::CEntryToolLockFX() : CSpecialMoveFX()
{
}

// ----------------------------------------------------------------------- //

CEntryToolLockFX::~CEntryToolLockFX()
{
}

// ----------------------------------------------------------------------- //

bool CEntryToolLockFX::Init( HLOCALOBJ hServObj, ILTMessage_Read* pMsg )
{
	if( !CSpecialMoveFX::Init( hServObj, pMsg ) ) return false;

	m_cs.Read( pMsg );

	return true;
}


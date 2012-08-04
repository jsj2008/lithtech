// ----------------------------------------------------------------------- //
//
// MODULE  : CClientWeaponSFX.cpp
//
// PURPOSE : CClientWeaponSFX - Implementation
//
// CREATED : 1/17/98
//
// (c) 1998-2001 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "ClientWeaponSFX.h"
#include "SFXMsgIds.h"
#include "SurfaceFunctions.h"
#include "WeaponFXTypes.h"
#include "CommonUtilities.h"
#include "ServerUtilities.h"
#include "WorldModel.h"

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CreateClientWeaponFX
//
//	PURPOSE:	Send message to client with data
//
// ----------------------------------------------------------------------- //

void CreateClientWeaponFX( CLIENTWEAPONFX & theStruct )
{
	if( !g_pLTServer )
		return;

	// If this is a movable object, set the flags of fx to ignore
	// marks and smoke...
	if( IsMoveable( theStruct.hObj ))
	{
		theStruct.wIgnoreFX |= WFX_MARK;
	}

//No dings
//	// Do impact dings if applicable...
//	if (!(IsMultiplayerGameServer() && IsCharacter(theStruct.hObj)))
//	{
//		theStruct.wIgnoreFX |= WFX_IMPACTDING;
//	}


	// [KLS 2/28/02] - If the object hit is a character, re-evaluate the surface type.
	// We do this here because the process of applying damage to the character may have
	// changed the character's surface type (e.g., from Armor to Flesh).

	if (IsCharacter(theStruct.hObj))
	{
		theStruct.nSurfaceType = GetSurfaceType(theStruct.hObj);
	}


	// Tell all the clients who can see this fx about the fx...

	CAutoMessage cMsg;
	cMsg.Writeuint8(SFX_WEAPON_ID);

	// If the object that fired is a player then just send the clientID since it is
	// smaller that the object handle...
	bool bSendClientID = IsPlayer( theStruct.hFiredFrom );
	cMsg.Writebool( bSendClientID );
	if( bSendClientID )
	{
		cMsg.Writeuint8( theStruct.nShooterId );
	}
	else
	{
		cMsg.WriteObject( theStruct.hFiredFrom );
	}
	

	cMsg.Writebool( theStruct.bSendWeaponRecord );
	if( theStruct.bSendWeaponRecord )
		cMsg.WriteDatabaseRecord( g_pLTDatabase, theStruct.hWeapon );

	cMsg.Writebool( theStruct.bSendAmmoRecord );
	if( theStruct.bSendAmmoRecord )
	{
		cMsg.WriteDatabaseRecord( g_pLTDatabase, theStruct.hAmmo );
	}
	else
	{
		// Inform the clients if an AI fired the weapon so they can retrieve
		// the correct weapon data record...
		bool bIsAI = IsAI(theStruct.hFiredFrom);
		cMsg.Writebool( bIsAI );
	}

	cMsg.Writeuint16(theStruct.wIgnoreFX);

	// Don't write out the mainworld, since this is a known on both client and server.
	if( g_pLTBase->GetMainWorldModel() == theStruct.hObj )
	{
		cMsg.Writebool(true);
	}
	else
	{
		cMsg.Writebool(false);
		cMsg.WriteObject(theStruct.hObj);
	}
	cMsg.Writeuint8(theStruct.nSurfaceType);
	cMsg.WriteCompLTVector(theStruct.vFirePos);
	cMsg.Writebool(theStruct.bFXAtFlashSocket);
	cMsg.WriteCompLTVector(theStruct.vPos);
	cMsg.WriteCompLTPolarCoord(LTPolarCoord(theStruct.vSurfaceNormal));
	bool bNodeHit = (theStruct.hNodeHit != INVALID_MODEL_NODE);
	cMsg.Writebool(bNodeHit);
	if (bNodeHit)
	{
		LTASSERT(theStruct.hNodeHit <= 0xFF, "Node hit encountered above weapon structure limits");
		cMsg.Writeuint8(theStruct.hNodeHit);
	}
	g_pLTServer->SendSFXMessage(cMsg.Read(), 0);
}




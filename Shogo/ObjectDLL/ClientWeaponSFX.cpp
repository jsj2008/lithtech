// ----------------------------------------------------------------------- //
//
// MODULE  : CClientWeaponSFX.cpp
//
// PURPOSE : CClientWeaponSFX - Implementation
//
// CREATED : 1/17/98
//
// ----------------------------------------------------------------------- //

#include "ClientWeaponSFX.h"
#include "SFXMsgIds.h"
#include "SurfaceFunctions.h"

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CreateClientWeaponFX
//
//	PURPOSE:	Send message to client with data
//
// ----------------------------------------------------------------------- //

void CreateClientWeaponFX(CLIENTWEAPONFX & theStruct)
{
	if (!g_pServerDE) return;

	// If this is a moveable object, set the flags of fx to ignore
	// marks and smoke...

	if (IsMoveable(theStruct.hObj))
	{
		theStruct.nIgnoreFX |= WFX_MARK | WFX_SMOKE;	 
	}


	// Tell all the clients who can see this fx about the fx...

	HMESSAGEWRITE hMessage = g_pServerDE->StartInstantSpecialEffectMessage(&(theStruct.vPos));
	g_pServerDE->WriteToMessageByte(hMessage, SFX_WEAPON_ID);
	g_pServerDE->WriteToMessageByte(hMessage, theStruct.nWeaponId);
	g_pServerDE->WriteToMessageByte(hMessage, theStruct.nSurfaceType);
	g_pServerDE->WriteToMessageByte(hMessage, theStruct.nIgnoreFX);
	g_pServerDE->WriteToMessageByte(hMessage, theStruct.nShooterId);
	g_pServerDE->WriteToMessageCompPosition(hMessage, &(theStruct.vFirePos));
	g_pServerDE->WriteToMessageCompPosition(hMessage, &(theStruct.vPos));
	g_pServerDE->WriteToMessageRotation(hMessage, &(theStruct.rRot));
	g_pServerDE->EndMessage2(hMessage, MESSAGE_NAGGLEFAST);
}



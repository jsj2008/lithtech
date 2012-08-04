// ----------------------------------------------------------------------- //
//
// MODULE  : DebrisFuncs.cpp
//
// PURPOSE : Misc functions for creating debris
//
// CREATED : 6/29/98
//
// ----------------------------------------------------------------------- //

// Includes...

#include "serverobj_de.h"
#include "DebrisFuncs.h"
#include "SFXMsgIds.h"

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CreateDebris()
//
//	PURPOSE:	Create client-side debris...
//
// ----------------------------------------------------------------------- //

void CreateDebris(CLIENTDEBRIS & cd)
{
	if (!g_pServerDE) return;

	HMESSAGEWRITE hMessage = g_pServerDE->StartInstantSpecialEffectMessage(&cd.vPos);
	g_pServerDE->WriteToMessageByte(hMessage, SFX_DEBRIS_ID);
	g_pServerDE->WriteToMessageRotation(hMessage, &cd.rRot);
	g_pServerDE->WriteToMessageCompPosition(hMessage, &cd.vPos);
	g_pServerDE->WriteToMessageCompVector(hMessage, &cd.vMinVel);
	g_pServerDE->WriteToMessageCompVector(hMessage, &cd.vMaxVel);
	g_pServerDE->WriteToMessageFloat(hMessage, cd.fLifeTime);
	g_pServerDE->WriteToMessageFloat(hMessage, cd.fFadeTime);
	g_pServerDE->WriteToMessageFloat(hMessage, cd.fMinScale);
	g_pServerDE->WriteToMessageFloat(hMessage, cd.fMaxScale);
	g_pServerDE->WriteToMessageByte(hMessage, cd.nNumDebris);
	g_pServerDE->WriteToMessageByte(hMessage, cd.nDebrisFlags);
	g_pServerDE->WriteToMessageByte(hMessage, cd.nDebrisType);
	g_pServerDE->WriteToMessageByte(hMessage, (DBYTE)cd.bRotate);
	g_pServerDE->WriteToMessageByte(hMessage, (DBYTE)cd.bPlayBounceSound);
	g_pServerDE->WriteToMessageByte(hMessage, (DBYTE)cd.bPlayExplodeSound);
	g_pServerDE->EndMessage(hMessage);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CreatePropDebris()
//
//	PURPOSE:	Create client-side debris for props...
//
// ----------------------------------------------------------------------- //

void CreatePropDebris(DVector & vPos, DFLOAT fDimsMag, 
					  DVector & vDir, DBYTE nDebrisType, 
					  DBYTE nMinNum, DBYTE nMaxNum)
{
	if (!g_pServerDE || (nMinNum > nMaxNum || nMaxNum <= 0)) return;

	CLIENTDEBRIS cd;

	DVector vUp;
	VEC_SET(vUp, 0.0f, 1.0f, 0.0f);
	g_pServerDE->AlignRotation(&cd.rRot, &vDir, &vUp);

	VEC_COPY(cd.vPos, vPos);
	VEC_SET(cd.vMinVel, -200.0f, 250.0f, -200.0f); 
	VEC_SET(cd.vMaxVel, 200.0f, 400.0f, 200.0f);

	cd.fLifeTime		 = g_pServerDE->Random(10.0f, 15.0f);
	cd.fFadeTime		 = cd.fLifeTime * 0.75f;
	cd.fMinScale		 = fDimsMag * 0.25f;
	cd.fMaxScale		 = fDimsMag;
	cd.nNumDebris		 = (DBYTE)g_pServerDE->Random(nMinNum, nMaxNum);
	cd.nDebrisType		 = nDebrisType;
	cd.bRotate			 = DTRUE;
	cd.bPlayBounceSound  = DTRUE;
	cd.bPlayExplodeSound = DTRUE;

	::CreateDebris(cd);
}

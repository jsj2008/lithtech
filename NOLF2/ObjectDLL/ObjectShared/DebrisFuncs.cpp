// ----------------------------------------------------------------------- //
//
// MODULE  : DebrisFuncs.cpp
//
// PURPOSE : Misc functions for creating debris
//
// CREATED : 6/29/98
//
// (c) 1998-2000 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

// Includes...

#include "stdafx.h"
#include "ltserverobj.h"
#include "DebrisFuncs.h"
#include "SFXMsgIds.h"
#include "DebrisMgr.h"

#ifndef __PSX2
CDebrisMgrPlugin s_DebrisMgrPlugin;
#endif  // !__PSX2

#define DEBRIS_PROPERTY_NAME	"DebrisType"

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CreateDebris()
//
//	PURPOSE:	Create client-side debris...
//
// ----------------------------------------------------------------------- //

static void CreateDebris(CLIENTDEBRIS & cd)
{
    if (!g_pLTServer) return;

	CAutoMessage cMsg;
	cMsg.Writeuint8(SFX_DEBRIS_ID);
	cMsg.WriteLTRotation(cd.rRot);
	cMsg.WriteCompPos(cd.vPos);
	cMsg.Writeuint8(cd.nDebrisId);
	g_pLTServer->SendSFXMessage(cMsg.Read(), cd.vPos, 0);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CreatePropDebris()
//
//	PURPOSE:	Create client-side debris for props...
//
// ----------------------------------------------------------------------- //

void CreatePropDebris( const LTVector &vPos, const LTVector &vDir, uint8 nDebrisId )
{
	CLIENTDEBRIS cd;

	cd.rRot = LTRotation(vDir, LTVector(0.0f, 1.0f, 0.0f));

	cd.vPos			= vPos;
	cd.nDebrisId	= nDebrisId;

	::CreateDebris(cd);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	GetDebrisProperties()
//
//	PURPOSE:	Determine the debris properties (This should only be
//				called during an object's ReadProp function if the object
//				added the ADD_DEBRISTYPE_PROPERTY macro).
//
// ----------------------------------------------------------------------- //

void GetDebrisProperties(uint8 & nDebrisId)
{
    if (!g_pLTServer) return;

	GenericProp genProp;
    if (g_pLTServer->GetPropGeneric(DEBRIS_PROPERTY_NAME, &genProp) == LT_OK)
	{
		DEBRIS* pDebris = g_pDebrisMgr->GetDebris(genProp.m_String);
		if (pDebris)
		{
			nDebrisId = pDebris->nId;
		}
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDebrisPlugin::PreHook_EditStringList()
//
//	PURPOSE:	Fill in property string list
//
// ----------------------------------------------------------------------- //
#ifndef __PSX2
LTRESULT CDebrisPlugin::PreHook_EditStringList(const char* szRezPath, const char* szPropName, char** aszStrings, uint32* pcStrings, const uint32 cMaxStrings, const uint32 cMaxStringLength)
{
	// See if we can handle the property...

	if (_strcmpi(DEBRIS_PROPERTY_NAME, szPropName) == 0)
	{
		s_DebrisMgrPlugin.PreHook_EditStringList(szRezPath, szPropName,
			aszStrings, pcStrings, cMaxStrings, cMaxStringLength);

		if (!s_DebrisMgrPlugin.PopulateStringList(aszStrings, pcStrings,
			 cMaxStrings, cMaxStringLength)) return LT_UNSUPPORTED;

		return LT_OK;
	}

	return LT_UNSUPPORTED;
}
#endif  // !__PSX2
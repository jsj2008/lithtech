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

CDebrisMgrPlugin s_DebrisMgrPlugin;

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

    HMESSAGEWRITE hMessage = g_pLTServer->StartInstantSpecialEffectMessage(&cd.vPos);
    g_pLTServer->WriteToMessageByte(hMessage, SFX_DEBRIS_ID);
    g_pLTServer->WriteToMessageRotation(hMessage, &cd.rRot);
    g_pLTServer->WriteToMessageCompPosition(hMessage, &cd.vPos);
    g_pLTServer->WriteToMessageByte(hMessage, cd.nDebrisId);
    g_pLTServer->EndMessage(hMessage);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CreatePropDebris()
//
//	PURPOSE:	Create client-side debris for props...
//
// ----------------------------------------------------------------------- //

void CreatePropDebris(LTVector & vPos, LTVector & vDir, uint8 nDebrisId)
{
	CLIENTDEBRIS cd;

    LTVector vUp(0.0f, 1.0f, 0.0f);
    g_pLTServer->AlignRotation(&(cd.rRot), &vDir, &vUp);

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

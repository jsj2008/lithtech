// ----------------------------------------------------------------------- //
//
// MODULE  : SFXFuncs.cpp
//
// PURPOSE : Misc functions used with special fx
//
// CREATED : 6/9/99
//
// (c) 1999 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

// Includes...

#include "stdafx.h"
#include "SFXFuncs.h"
#include "SFXMsgIds.h"

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	GetLensFlareProperties()
//
//	PURPOSE:	Read in the lensflare properties (This should only be
//				called during an object's ReadProp function if the object
//				added the ADD_LENSFLARE_PROPERTIES macro).
//
// ----------------------------------------------------------------------- //

void GetLensFlareProperties(LENSFLARE & lensProps)
{
    if (!g_pLTServer) return;

	GenericProp genProp;

    if (g_pLTServer->GetPropGeneric("InSkyBox", &genProp ) == LT_OK)
	{
		lensProps.bInSkyBox = genProp.m_Bool;
	}

    if (g_pLTServer->GetPropGeneric("MinAngle", &genProp ) == LT_OK)
	{
		lensProps.fMinAngle = genProp.m_Float;
	}

    if (g_pLTServer->GetPropGeneric("CreateSprite", &genProp ) == LT_OK)
	{
		lensProps.bCreateSprite = genProp.m_Bool;
	}

    if (g_pLTServer->GetPropGeneric("SpriteOnly", &genProp ) == LT_OK)
	{
		lensProps.bSpriteOnly = genProp.m_Bool;
	}

    if (g_pLTServer->GetPropGeneric("UseObjectAngle", &genProp ) == LT_OK)
	{
		lensProps.bUseObjectAngle = genProp.m_Bool;
	}

    if (g_pLTServer->GetPropGeneric("SpriteOffset", &genProp ) == LT_OK)
	{
		lensProps.fSpriteOffset = genProp.m_Float;
	}

    if (g_pLTServer->GetPropGeneric("MinSpriteAlpha", &genProp ) == LT_OK)
	{
		lensProps.fMinSpriteAlpha = genProp.m_Float;
	}

    if (g_pLTServer->GetPropGeneric("MaxSpriteAlpha", &genProp ) == LT_OK)
	{
		lensProps.fMaxSpriteAlpha = genProp.m_Float;
	}

    if (g_pLTServer->GetPropGeneric("MinSpriteScale", &genProp ) == LT_OK)
	{
		lensProps.fMinSpriteScale = genProp.m_Float;
	}

    if (g_pLTServer->GetPropGeneric("MaxSpriteScale", &genProp ) == LT_OK)
	{
		lensProps.fMaxSpriteScale = genProp.m_Float;
	}

    if (g_pLTServer->GetPropGeneric("SpriteFile", &genProp ) == LT_OK)
	{
		lensProps.SetSpriteFile(genProp.m_String);
	}

    if (g_pLTServer->GetPropGeneric("BlindingFlare", &genProp ) == LT_OK)
	{
		lensProps.bBlindingFlare = genProp.m_Bool;
	}

    if (g_pLTServer->GetPropGeneric("BlindObjectAngle", &genProp ) == LT_OK)
	{
		lensProps.fBlindObjectAngle = genProp.m_Float;
	}

    if (g_pLTServer->GetPropGeneric("BlindCameraAngle", &genProp ) == LT_OK)
	{
		lensProps.fBlindCameraAngle = genProp.m_Float;
	}

    if (g_pLTServer->GetPropGeneric("MinBlindScale", &genProp ) == LT_OK)
	{
		lensProps.fMinBlindScale = genProp.m_Float;
	}

    if (g_pLTServer->GetPropGeneric("MaxBlindScale", &genProp ) == LT_OK)
	{
		lensProps.fMaxBlindScale = genProp.m_Float;
	}

    if (g_pLTServer->GetPropGeneric("SpriteAdditive", &genProp ) == LT_OK)
	{
		lensProps.bSpriteAdditive = genProp.m_Bool;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	BuildLensFlareSFXMessage()
//
//	PURPOSE:	Build the lens flare special fx message...
//
// ----------------------------------------------------------------------- //

void BuildLensFlareSFXMessage(LENSFLARE & lensProps, LPBASECLASS pClass)
{
	if (!pClass) return;

	CAutoMessage cMsg;
	cMsg.Writeuint8(SFX_LENSFLARE_ID);
	::AddLensFlareInfoToMessage(lensProps, cMsg);
	g_pLTServer->SetObjectSFXMessage(pClass->m_hObject, cMsg.Read());
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	BuildLensFlareSFXMessage()
//
//	PURPOSE:	Build the lens flare special fx message...
//
// ----------------------------------------------------------------------- //

void AddLensFlareInfoToMessage(LENSFLARE & lensProps, ILTMessage_Write *pMsg)
{
	if (!pMsg) return;

    pMsg->Writeuint8(lensProps.bInSkyBox);
    pMsg->Writeuint8(lensProps.bCreateSprite);
    pMsg->Writeuint8(lensProps.bSpriteOnly);
    pMsg->Writeuint8(lensProps.bUseObjectAngle);
    pMsg->Writeuint8(lensProps.bSpriteAdditive);
    pMsg->Writefloat(lensProps.fSpriteOffset);
    pMsg->Writefloat(lensProps.fMinAngle);
    pMsg->Writefloat(lensProps.fMinSpriteAlpha);
    pMsg->Writefloat(lensProps.fMaxSpriteAlpha);
    pMsg->Writefloat(lensProps.fMinSpriteScale);
    pMsg->Writefloat(lensProps.fMaxSpriteScale);
    pMsg->WriteHString(lensProps.hstrSpriteFile);
    pMsg->Writeuint8(lensProps.bBlindingFlare);
    pMsg->Writefloat(lensProps.fBlindObjectAngle);
    pMsg->Writefloat(lensProps.fBlindCameraAngle);
    pMsg->Writefloat(lensProps.fMinBlindScale);
    pMsg->Writefloat(lensProps.fMaxBlindScale);
}
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

    HMESSAGEWRITE hMessage = g_pLTServer->StartSpecialEffectMessage(pClass);
    g_pLTServer->WriteToMessageByte(hMessage, SFX_LENSFLARE_ID);
	::AddLensFlareInfoToMessage(lensProps, hMessage);
    g_pLTServer->EndMessage(hMessage);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	BuildLensFlareSFXMessage()
//
//	PURPOSE:	Build the lens flare special fx message...
//
// ----------------------------------------------------------------------- //

void AddLensFlareInfoToMessage(LENSFLARE & lensProps, HMESSAGEWRITE hMessage)
{
	if (!hMessage) return;

    g_pLTServer->WriteToMessageByte(hMessage, lensProps.bInSkyBox);
    g_pLTServer->WriteToMessageByte(hMessage, lensProps.bCreateSprite);
    g_pLTServer->WriteToMessageByte(hMessage, lensProps.bSpriteOnly);
    g_pLTServer->WriteToMessageByte(hMessage, lensProps.bUseObjectAngle);
    g_pLTServer->WriteToMessageByte(hMessage, lensProps.bSpriteAdditive);
    g_pLTServer->WriteToMessageFloat(hMessage, lensProps.fSpriteOffset);
    g_pLTServer->WriteToMessageFloat(hMessage, lensProps.fMinAngle);
    g_pLTServer->WriteToMessageFloat(hMessage, lensProps.fMinSpriteAlpha);
    g_pLTServer->WriteToMessageFloat(hMessage, lensProps.fMaxSpriteAlpha);
    g_pLTServer->WriteToMessageFloat(hMessage, lensProps.fMinSpriteScale);
    g_pLTServer->WriteToMessageFloat(hMessage, lensProps.fMaxSpriteScale);
    g_pLTServer->WriteToMessageHString(hMessage, lensProps.hstrSpriteFile);
    g_pLTServer->WriteToMessageByte(hMessage, lensProps.bBlindingFlare);
    g_pLTServer->WriteToMessageFloat(hMessage, lensProps.fBlindObjectAngle);
    g_pLTServer->WriteToMessageFloat(hMessage, lensProps.fBlindCameraAngle);
    g_pLTServer->WriteToMessageFloat(hMessage, lensProps.fMinBlindScale);
    g_pLTServer->WriteToMessageFloat(hMessage, lensProps.fMaxBlindScale);
}
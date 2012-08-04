// ----------------------------------------------------------------------- //
//
// MODULE  : SharedFXStructs.cpp
//
// PURPOSE : Shared Special FX structs - Implementation
//
// CREATED : 10/21/99
//
// (c) 1999 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "SharedFXStructs.h"

/////////////////////////////////////////////////////////////////////////////
//
// CHARCREATESTRUCT class Implementation
//
// (used by client-side CBodyFX and server-side Character classes)
//
/////////////////////////////////////////////////////////////////////////////

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CHARCREATESTRUCT::Write()
//
//	PURPOSE:	Write the data to the message
//
// ----------------------------------------------------------------------- //

void CHARCREATESTRUCT::Write(ILTCSBase *pInterface, HMESSAGEWRITE hWrite)
{
	if (!hWrite) return;

    SFXCREATESTRUCT::Write(pInterface, hWrite);

    pInterface->WriteToMessageByte(hWrite, bIsPlayer);
    pInterface->WriteToMessageByte(hWrite, byFXFlags);
    pInterface->WriteToMessageByte(hWrite, eModelId);
    pInterface->WriteToMessageByte(hWrite, eModelStyle);
    pInterface->WriteToMessageByte(hWrite, nTrackers);
    pInterface->WriteToMessageByte(hWrite, nDimsTracker);
    pInterface->WriteToMessageFloat(hWrite, fStealthPercent);
    pInterface->WriteToMessageByte(hWrite, nClientID);
    pInterface->WriteToMessageByte(hWrite, eCrosshairCharacterClass);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CHARCREATESTRUCT::Read()
//
//	PURPOSE:	Read the data from the message
//
// ----------------------------------------------------------------------- //

void CHARCREATESTRUCT::Read(ILTCSBase *pInterface, HMESSAGEREAD hRead)
{
	if (!hRead) return;

    SFXCREATESTRUCT::Read(pInterface, hRead);

    bIsPlayer       = pInterface->ReadFromMessageByte(hRead);
    byFXFlags       = pInterface->ReadFromMessageByte(hRead);
    eModelId        = (ModelId)     pInterface->ReadFromMessageByte(hRead);
    eModelStyle     = (ModelStyle)  pInterface->ReadFromMessageByte(hRead);
    nTrackers       = pInterface->ReadFromMessageByte(hRead);
    nDimsTracker    = pInterface->ReadFromMessageByte(hRead);
    fStealthPercent = pInterface->ReadFromMessageFloat(hRead);
    nClientID       = pInterface->ReadFromMessageByte(hRead);
    eCrosshairCharacterClass = (CharacterClass) pInterface->ReadFromMessageByte(hRead);

	// Set these based on the model id...

	if (eModelIdInvalid != eModelId)
	{
		eModelSkeleton	= g_pModelButeMgr->GetModelSkeleton(eModelId);
		eModelType		= g_pModelButeMgr->GetModelType(eModelId);
	}
	else
	{
		eModelSkeleton	= eModelSkeletonInvalid;
		eModelType		= eModelTypeInvalid;
	}
}


/////////////////////////////////////////////////////////////////////////////
//
// BODYCREATESTRUCT class Implementation
//
// (used by client-side CBodyFX and server-side Character classes)
//
/////////////////////////////////////////////////////////////////////////////

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	BODYCREATESTRUCT::Write()
//
//	PURPOSE:	Write the data to the message
//
// ----------------------------------------------------------------------- //

void BODYCREATESTRUCT::Write(ILTCSBase *pInterface, HMESSAGEWRITE hWrite)
{
	if (!hWrite) return;

    SFXCREATESTRUCT::Write(pInterface, hWrite);

    pInterface->WriteToMessageByte(hWrite, eBodyState);
    pInterface->WriteToMessageByte(hWrite, nClientId);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	BODYCREATESTRUCT::Read()
//
//	PURPOSE:	Read the data from the message
//
// ----------------------------------------------------------------------- //

void BODYCREATESTRUCT::Read(ILTCSBase *pInterface, HMESSAGEREAD hRead)
{
	if (!hRead) return;

    SFXCREATESTRUCT::Read(pInterface, hRead);

    eBodyState = (BodyState)pInterface->ReadFromMessageByte(hRead);
    nClientId = pInterface->ReadFromMessageByte(hRead);
}

/////////////////////////////////////////////////////////////////////////////
//
// STEAMCREATESTRUCT class Implementation
//
// (used by client-side CSteamFX and server-side Steam classes)
//
/////////////////////////////////////////////////////////////////////////////

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	STEAMCREATESTRUCT::ReadProps()
//
//	PURPOSE:	Read in the properties
//
// ----------------------------------------------------------------------- //

void STEAMCREATESTRUCT::ReadProps()
{
#ifndef _CLIENTBUILD
	GenericProp genProp;

    if (g_pLTServer->GetPropGeneric("Range", &genProp) == LT_OK)
	{
		fRange = genProp.m_Float;
	}

    if (g_pLTServer->GetPropGeneric("VolumeRadius", &genProp) == LT_OK)
	{
		fVolumeRadius = genProp.m_Float;
	}

    if (g_pLTServer->GetPropGeneric("ParticleVelocity", &genProp) == LT_OK)
	{
		fVel = genProp.m_Float;
	}

    if (g_pLTServer->GetPropGeneric("SoundRadius", &genProp) == LT_OK)
	{
		fSoundRadius = genProp.m_Float;
	}

    if (g_pLTServer->GetPropGeneric("ParticleRadius", &genProp) == LT_OK)
	{
		fParticleRadius = genProp.m_Float;
	}

    if (g_pLTServer->GetPropGeneric("NumParticles", &genProp) == LT_OK)
	{
        nNumParticles = (uint8) genProp.m_Long;
	}

    if (g_pLTServer->GetPropGeneric("CreateDelta", &genProp) == LT_OK)
	{
		fCreateDelta = genProp.m_Float;
	}

    if (g_pLTServer->GetPropGeneric("StartAlpha", &genProp) == LT_OK)
	{
		fStartAlpha = genProp.m_Float;
	}

    if (g_pLTServer->GetPropGeneric("EndAlpha", &genProp) == LT_OK)
	{
		fEndAlpha = genProp.m_Float;
	}

    if (g_pLTServer->GetPropGeneric("StartScale", &genProp) == LT_OK)
	{
		fStartScale= genProp.m_Float;
	}

    if (g_pLTServer->GetPropGeneric("EndScale", &genProp) == LT_OK)
	{
		fEndScale = genProp.m_Float;
	}

    if (g_pLTServer->GetPropGeneric("SoundName", &genProp) == LT_OK)
	{
		if (genProp.m_String[0])
		{
            hstrSoundName = g_pLTServer->CreateString(genProp.m_String);
		}
	}

    if (g_pLTServer->GetPropGeneric("Particle", &genProp) == LT_OK)
	{
		if (genProp.m_String[0])
		{
            hstrParticle = g_pLTServer->CreateString(genProp.m_String);
		}
	}

    if (g_pLTServer->GetPropGeneric("MinDriftVel", &genProp) == LT_OK)
	{
		vMinDriftVel = genProp.m_Vec;
	}

    if (g_pLTServer->GetPropGeneric("MaxDriftVel", &genProp) == LT_OK)
	{
		vMaxDriftVel = genProp.m_Vec;
	}

    if (g_pLTServer->GetPropGeneric("Color1", &genProp) == LT_OK)
	{
		vColor1 = genProp.m_Color;
	}

    if (g_pLTServer->GetPropGeneric("Color2", &genProp) == LT_OK)
	{
		vColor2 = genProp.m_Color;
	}
#endif
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	STEAMCREATESTRUCT::~STEAMCREATESTRUCT()
//
//	PURPOSE:	Deallocate the object
//
// ----------------------------------------------------------------------- //

STEAMCREATESTRUCT::~STEAMCREATESTRUCT()
{
// Deallocate the strings on the server (the client must do this manually)
#ifndef _CLIENTBUILD

	if (hstrSoundName)
	{
        g_pLTServer->FreeString(hstrSoundName);
	}

	if (hstrParticle)
	{
        g_pLTServer->FreeString(hstrParticle);
	}

#endif
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	STEAMCREATESTRUCT::Write()
//
//	PURPOSE:	Write the data to the message
//
// ----------------------------------------------------------------------- //

void STEAMCREATESTRUCT::Write(ILTCSBase *pInterface, HMESSAGEWRITE hWrite)
{
	if (!hWrite) return;

    SFXCREATESTRUCT::Write(pInterface, hWrite);

    pInterface->WriteToMessageFloat(hWrite, fRange);
    pInterface->WriteToMessageFloat(hWrite, fVel);
    pInterface->WriteToMessageFloat(hWrite, fSoundRadius);
    pInterface->WriteToMessageFloat(hWrite, fParticleRadius);
    pInterface->WriteToMessageFloat(hWrite, fStartAlpha);
    pInterface->WriteToMessageFloat(hWrite, fEndAlpha);
    pInterface->WriteToMessageFloat(hWrite, fStartScale);
    pInterface->WriteToMessageFloat(hWrite, fEndScale);
    pInterface->WriteToMessageFloat(hWrite, fCreateDelta);
    pInterface->WriteToMessageFloat(hWrite, fVolumeRadius);
    pInterface->WriteToMessageByte(hWrite, nNumParticles);
    pInterface->WriteToMessageHString(hWrite, hstrSoundName);
    pInterface->WriteToMessageHString(hWrite, hstrParticle);
    pInterface->WriteToMessageVector(hWrite, &vMinDriftVel);
    pInterface->WriteToMessageVector(hWrite, &vMaxDriftVel);
    pInterface->WriteToMessageVector(hWrite, &vColor1);
    pInterface->WriteToMessageVector(hWrite, &vColor2);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	STEAMCREATESTRUCT::Read()
//
//	PURPOSE:	Read the data from the message
//
// ----------------------------------------------------------------------- //

void STEAMCREATESTRUCT::Read(ILTCSBase *pInterface, HMESSAGEREAD hRead)
{
	if (!hRead) return;

    SFXCREATESTRUCT::Read(pInterface, hRead);

    fRange          = pInterface->ReadFromMessageFloat(hRead);
    fVel            = pInterface->ReadFromMessageFloat(hRead);
    fSoundRadius    = pInterface->ReadFromMessageFloat(hRead);
    fParticleRadius = pInterface->ReadFromMessageFloat(hRead);
    fStartAlpha     = pInterface->ReadFromMessageFloat(hRead);
    fEndAlpha       = pInterface->ReadFromMessageFloat(hRead);
    fStartScale     = pInterface->ReadFromMessageFloat(hRead);
    fEndScale       = pInterface->ReadFromMessageFloat(hRead);
    fCreateDelta    = pInterface->ReadFromMessageFloat(hRead);
    fVolumeRadius   = pInterface->ReadFromMessageFloat(hRead);
    nNumParticles   = pInterface->ReadFromMessageByte(hRead);
    hstrSoundName   = pInterface->ReadFromMessageHString(hRead);
    hstrParticle    = pInterface->ReadFromMessageHString(hRead);
    pInterface->ReadFromMessageVector(hRead, &vMinDriftVel);
    pInterface->ReadFromMessageVector(hRead, &vMaxDriftVel);
    pInterface->ReadFromMessageVector(hRead, &vColor1);
    pInterface->ReadFromMessageVector(hRead, &vColor2);
}



/////////////////////////////////////////////////////////////////////////////
//
// EXPLOSIONCREATESTRUCT class Implementation
//
// (used by client-side CExplosionFX and server-side Explosion classes)
//
/////////////////////////////////////////////////////////////////////////////

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	EXPLOSIONCREATESTRUCT::Write()
//
//	PURPOSE:	Write the data to the message
//
// ----------------------------------------------------------------------- //

void EXPLOSIONCREATESTRUCT::Write(ILTCSBase *pInterface, HMESSAGEWRITE hWrite)
{
	if (!hWrite) return;

    SFXCREATESTRUCT::Write(pInterface, hWrite);

    pInterface->WriteToMessageByte(hWrite, nImpactFX);
    pInterface->WriteToMessageFloat(hWrite, fDamageRadius);
    pInterface->WriteToMessageVector(hWrite, &vPos);
    pInterface->WriteToMessageRotation(hWrite, &rRot);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	EXPLOSIONCREATESTRUCT::Read()
//
//	PURPOSE:	Read the data from the message
//
// ----------------------------------------------------------------------- //

void EXPLOSIONCREATESTRUCT::Read(ILTCSBase *pInterface, HMESSAGEREAD hRead)
{
	if (!hRead) return;

    SFXCREATESTRUCT::Read(pInterface, hRead);

    nImpactFX       = pInterface->ReadFromMessageByte(hRead);
    fDamageRadius   = pInterface->ReadFromMessageFloat(hRead);
    pInterface->ReadFromMessageVector(hRead, &vPos);
    pInterface->ReadFromMessageRotation(hRead, &rRot);
}


/////////////////////////////////////////////////////////////////////////////
//
// SPRINKLETYPECREATESTRUCT class Implementation
//
// (used by client-side SprinklesFX and server-side Sprinkles classes)
//
/////////////////////////////////////////////////////////////////////////////

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	SPRINKLETYPECREATESTRUCT::~SPRINKLETYPECREATESTRUCT()
//
//	PURPOSE:	Deallocate the object
//
// ----------------------------------------------------------------------- //

SPRINKLETYPECREATESTRUCT::~SPRINKLETYPECREATESTRUCT()
{
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	SPRINKLETYPECREATESTRUCT::Write()
//
//	PURPOSE:	Write the data to the message
//
// ----------------------------------------------------------------------- //

void SPRINKLETYPECREATESTRUCT::Write(ILTCSBase *pInterface, HMESSAGEWRITE hWrite)
{
	if (!hWrite) return;

    pInterface->WriteToMessageHString(hWrite, m_hFilename);
    pInterface->WriteToMessageHString(hWrite, m_hSkinName);
    pInterface->WriteToMessageDWord(hWrite, m_Count);
    pInterface->WriteToMessageFloat(hWrite, m_Speed);
    pInterface->WriteToMessageFloat(hWrite, m_Size);
    pInterface->WriteToMessageFloat(hWrite, m_SpawnRadius);
    pInterface->WriteToMessageVector(hWrite, &m_AnglesVel);
    pInterface->WriteToMessageVector(hWrite, &m_ColorMin);
    pInterface->WriteToMessageVector(hWrite, &m_ColorMax);

}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	SPRINKLETYPECREATESTRUCT::Read()
//
//	PURPOSE:	Read the data from the message
//
// ----------------------------------------------------------------------- //

void SPRINKLETYPECREATESTRUCT::Read(ILTCSBase *pInterface, HMESSAGEREAD hRead)
{
	if (!hRead) return;

    m_hFilename     = pInterface->ReadFromMessageHString(hRead);
    m_hSkinName     = pInterface->ReadFromMessageHString(hRead);
    m_Count         = pInterface->ReadFromMessageDWord(hRead);
    m_Speed         = pInterface->ReadFromMessageFloat(hRead);
    m_Size          = pInterface->ReadFromMessageFloat(hRead);
    m_SpawnRadius   = pInterface->ReadFromMessageFloat(hRead);
    pInterface->ReadFromMessageVector(hRead, &m_AnglesVel);
    pInterface->ReadFromMessageVector(hRead, &m_ColorMin);
    pInterface->ReadFromMessageVector(hRead, &m_ColorMax);

}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	SPRINKLESCREATESTRUCT::Write()
//
//	PURPOSE:	Write the data to the message
//
// ----------------------------------------------------------------------- //

void SPRINKLESCREATESTRUCT::Write(ILTCSBase *pInterface, HMESSAGEWRITE hWrite)
{
	if (!hWrite) return;

    SFXCREATESTRUCT::Write(pInterface, hWrite);

    pInterface->WriteToMessageDWord(hWrite, m_nTypes);

    for (uint32 i = 0; i < m_nTypes && m_nTypes < MAX_SPRINKLE_TYPES; i++)
	{
        m_Types[i].Write(pInterface, hWrite);
	}

}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	SPRINKLESCREATESTRUCT::Read()
//
//	PURPOSE:	Read the data from the message
//
// ----------------------------------------------------------------------- //

void SPRINKLESCREATESTRUCT::Read(ILTCSBase *pInterface, HMESSAGEREAD hRead)
{
	if (!hRead) return;

    SFXCREATESTRUCT::Read(pInterface, hRead);

    m_nTypes = pInterface->ReadFromMessageDWord(hRead);

    for (uint32 i = 0; i < m_nTypes && m_nTypes < MAX_SPRINKLE_TYPES; i++)
	{
        m_Types[i].Read(pInterface, hRead);
	}
}


/////////////////////////////////////////////////////////////////////////////
//
// LTCREATESTRUCT class Implementation
//
// (used by client-side CLaserTriggerFX and server-side LaserTrigger classes)
//
/////////////////////////////////////////////////////////////////////////////

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	LTCREATESTRUCT::Write()
//
//	PURPOSE:	Write the data to the message
//
// ----------------------------------------------------------------------- //

void LTCREATESTRUCT::Write(ILTCSBase *pInterface, HMESSAGEWRITE hWrite)
{
	if (!hWrite) return;

    SFXCREATESTRUCT::Write(pInterface, hWrite);

    pInterface->WriteToMessageVector(hWrite, &vColor);
    pInterface->WriteToMessageVector(hWrite, &vDims);
    pInterface->WriteToMessageFloat(hWrite, fAlpha);
    pInterface->WriteToMessageFloat(hWrite, fSpriteScale);
    pInterface->WriteToMessageByte(hWrite, bCreateSprite);
    pInterface->WriteToMessageHString(hWrite, hstrSpriteFilename);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	LTCREATESTRUCT::Read()
//
//	PURPOSE:	Read the data from the message
//
// ----------------------------------------------------------------------- //

void LTCREATESTRUCT::Read(ILTCSBase *pInterface, HMESSAGEREAD hRead)
{
	if (!hRead) return;

    SFXCREATESTRUCT::Read(pInterface, hRead);

    pInterface->ReadFromMessageVector(hRead, &vColor);
    pInterface->ReadFromMessageVector(hRead, &vDims);
    fAlpha              = pInterface->ReadFromMessageFloat(hRead);
    fSpriteScale        = pInterface->ReadFromMessageFloat(hRead);
    bCreateSprite       = (LTBOOL) pInterface->ReadFromMessageByte(hRead);
    hstrSpriteFilename  = pInterface->ReadFromMessageHString(hRead);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	LTCREATESTRUCT::~LTCREATESTRUCT()
//
//	PURPOSE:	Deallocate the struct
//
// ----------------------------------------------------------------------- //

LTCREATESTRUCT::~LTCREATESTRUCT()
{
}




/////////////////////////////////////////////////////////////////////////////
//
// MINECREATESTRUCT class Definition
//
// (used by client-side CMineFX and server-side Mine classes)
//
/////////////////////////////////////////////////////////////////////////////

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	MINECREATESTRUCT::Write()
//
//	PURPOSE:	Write the data to the message
//
// ----------------------------------------------------------------------- //

void MINECREATESTRUCT::Write(ILTCSBase *pInterface, HMESSAGEWRITE hWrite)
{
	if (!hWrite) return;

    SFXCREATESTRUCT::Write(pInterface, hWrite);

    pInterface->WriteToMessageFloat(hWrite, fMinRadius);
    pInterface->WriteToMessageFloat(hWrite, fMaxRadius);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	MINECREATESTRUCT::Read()
//
//	PURPOSE:	Read the data from the message
//
// ----------------------------------------------------------------------- //

void MINECREATESTRUCT::Read(ILTCSBase *pInterface, HMESSAGEREAD hRead)
{
	if (!hRead) return;

    SFXCREATESTRUCT::Read(pInterface, hRead);

    fMinRadius = pInterface->ReadFromMessageFloat(hRead);
    fMaxRadius = pInterface->ReadFromMessageFloat(hRead);
}



/////////////////////////////////////////////////////////////////////////////
//
// PVCREATESTRUCT class Definition
//
// (used by client-side CPlayerVehicleFX and server-side PlayerVehicle
// classes)
//
/////////////////////////////////////////////////////////////////////////////

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	PVCREATESTRUCT::Write()
//
//	PURPOSE:	Write the data to the message
//
// ----------------------------------------------------------------------- //

void PVCREATESTRUCT::Write(ILTCSBase *pInterface, HMESSAGEWRITE hWrite)
{
	if (!hWrite) return;

    SFXCREATESTRUCT::Write(pInterface, hWrite);

    pInterface->WriteToMessageByte(hWrite, ePhysicsModel);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	PVCREATESTRUCT::Read()
//
//	PURPOSE:	Read the data from the message
//
// ----------------------------------------------------------------------- //

void PVCREATESTRUCT::Read(ILTCSBase *pInterface, HMESSAGEREAD hRead)
{
	if (!hRead) return;

    SFXCREATESTRUCT::Read(pInterface, hRead);

    ePhysicsModel = (PlayerPhysicsModel) pInterface->ReadFromMessageByte(hRead);
}




/////////////////////////////////////////////////////////////////////////////
//
// OBJSPRITECREATESTRUCT class Implementation
//
// (used by client-side CObjSpriteFX and server-side ObjSprite classes)
//
/////////////////////////////////////////////////////////////////////////////

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	OBJSPRITECREATESTRUCT::ReadProps()
//
//	PURPOSE:	Read in the properties
//
// ----------------------------------------------------------------------- //

void OBJSPRITECREATESTRUCT::ReadProps()
{
#ifndef _CLIENTBUILD
	GenericProp genProp;

	long nLongVal;
    g_pLTServer->GetPropLongInt("PlayerTeamFilter", &nLongVal);
	m_nPlayerTeamFilter = nLongVal;


#endif
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	OBJSPRITECREATESTRUCT::~OBJSPRITECREATESTRUCT()
//
//	PURPOSE:	Deallocate the object
//
// ----------------------------------------------------------------------- //

OBJSPRITECREATESTRUCT::~OBJSPRITECREATESTRUCT()
{
// Deallocate the strings on the server (the client must do this manually)
#ifndef _CLIENTBUILD

#endif
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	OBJSPRITECREATESTRUCT::Write()
//
//	PURPOSE:	Write the data to the message
//
// ----------------------------------------------------------------------- //

void OBJSPRITECREATESTRUCT::Write(ILTCSBase *pInterface, HMESSAGEWRITE hWrite)
{
	if (!hWrite) return;

    SFXCREATESTRUCT::Write(pInterface, hWrite);

    pInterface->WriteToMessageDWord(hWrite, m_nPlayerTeamFilter);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	OBJSPRITECREATESTRUCT::Read()
//
//	PURPOSE:	Read the data from the message
//
// ----------------------------------------------------------------------- //

void OBJSPRITECREATESTRUCT::Read(ILTCSBase *pInterface, HMESSAGEREAD hRead)
{
	if (!hRead) return;

    SFXCREATESTRUCT::Read(pInterface, hRead);

    m_nPlayerTeamFilter = pInterface->ReadFromMessageDWord(hRead);
}


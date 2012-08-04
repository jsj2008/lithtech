// ----------------------------------------------------------------------- //
//
// MODULE  : SharedFXStructs.cpp
//
// PURPOSE : Shared Special FX structs - Implementation
//
// CREATED : 10/21/99
//
// (c) 1999-2002 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "SharedFXStructs.h"
#include "VersionMgr.h"

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

void CHARCREATESTRUCT::Write(ILTMessage_Write *pMsg)
{
	if (!pMsg) return;

    SFXCREATESTRUCT::Write(pMsg);

	pMsg->Writebool(bIsCinematicAI);
	pMsg->Writebool(bIsPlayer != LTFALSE);
	pMsg->Writeuint8(byFXFlags);
    pMsg->Writeuint8(eModelId);
    pMsg->Writeuint8(nTrackers);
    pMsg->Writeuint8(nDimsTracker);
    pMsg->Writefloat(fStealthPercent);
    pMsg->Writeuint8(nClientID);
    pMsg->Writeuint8(eCrosshairCharacterClass);
	pMsg->Writeuint64(nDamageFlags);
	pMsg->Writefloat(fPitch);

	// Send the hitbox info
	pMsg->WriteCompLTVector( vHitBoxDims );
	pMsg->WriteCompLTVector( vHitBoxOffset );
	
	pMsg->Writebool( bCanCarry );
	pMsg->Writebool( bCanWake );
	pMsg->Writebool( bTracking );
	pMsg->Writebool( bRadarVisible );

	// type of thing player is carrying
	pMsg->Writeuint8( nCarrying);

}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CHARCREATESTRUCT::Read()
//
//	PURPOSE:	Read the data from the message
//
// ----------------------------------------------------------------------- //

void CHARCREATESTRUCT::Read(ILTMessage_Read *pMsg)
{
	if (!pMsg) return;

    SFXCREATESTRUCT::Read(pMsg);

 	bIsCinematicAI	= pMsg->Readbool();
    bIsPlayer       = pMsg->Readbool() ? LTTRUE : LTFALSE;
    byFXFlags       = pMsg->Readuint8();
    eModelId        = (ModelId)pMsg->Readuint8();
    nTrackers       = pMsg->Readuint8();
    nDimsTracker    = pMsg->Readuint8();
    fStealthPercent = pMsg->Readfloat();
    nClientID       = pMsg->Readuint8();
    eCrosshairCharacterClass = (CharacterClass)pMsg->Readuint8();
	nDamageFlags	= pMsg->Readuint64();
	fPitch			= pMsg->Readfloat();

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

	// Update the hitbox dims
	vHitBoxDims		= pMsg->ReadCompLTVector();
	vHitBoxOffset	= pMsg->ReadCompLTVector();

	bCanCarry		= pMsg->Readbool();
	bCanWake		= pMsg->Readbool();
	bTracking		= pMsg->Readbool();

#ifdef _CLIENTBUILD
	bRadarVisible	= pMsg->Readbool();
	nCarrying		= pMsg->Readuint8();
#else  // _CLIENTBUILD
	if( g_pVersionMgr->GetCurrentSaveVersion( ) > CVersionMgr::kSaveVersion__1_1 )
	{
		bRadarVisible	= pMsg->Readbool();
	}
	

	if( g_pVersionMgr->GetCurrentSaveVersion( ) > CVersionMgr::kSaveVersion__1_2 )
	{
		nCarrying		= pMsg->Readuint8();
	}
	

#endif //_CLIENTBUILD


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

void BODYCREATESTRUCT::Write(ILTMessage_Write *pMsg)
{
	if (!pMsg) return;

    SFXCREATESTRUCT::Write(pMsg);

    pMsg->Writeuint8(eModelId);
	pMsg->Writeuint8(eBodyState);
	pMsg->Writeuint8(nClientId);
	pMsg->Writebool(bCanBeCarried);
	pMsg->Writebool(bCanBeRevived);
	pMsg->Writeuint8(eDeathDamageType);
	pMsg->Writebool(bPermanentBody);
	pMsg->Writebool(bHitBoxUpdated);
	pMsg->Writebool(bCanBeSearched);
	
	// Send the hitbox info
	pMsg->WriteCompLTVector( vHitBoxDims );
	pMsg->WriteCompLTVector( vHitBoxOffset );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	BODYCREATESTRUCT::Read()
//
//	PURPOSE:	Read the data from the message
//
// ----------------------------------------------------------------------- //

void BODYCREATESTRUCT::Read(ILTMessage_Read *pMsg)
{
	if (!pMsg) return;

    SFXCREATESTRUCT::Read(pMsg);

    eModelId			= (ModelId)pMsg->Readuint8();
    eBodyState			= (BodyState)pMsg->Readuint8();
    nClientId			= pMsg->Readuint8();
	bCanBeCarried		= pMsg->Readbool();
	bCanBeRevived		= pMsg->Readbool();
	eDeathDamageType	= (DamageType)pMsg->Readuint8();
	bPermanentBody		= pMsg->Readbool();
	bHitBoxUpdated		= pMsg->Readbool();
	bCanBeSearched		= pMsg->Readbool();

		// Update the hitbox dims
	vHitBoxDims		= pMsg->ReadCompLTVector();
	vHitBoxOffset	= pMsg->ReadCompLTVector();
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

void STEAMCREATESTRUCT::Write(ILTMessage_Write *pMsg)
{
	if (!pMsg) return;

    SFXCREATESTRUCT::Write(pMsg);

	pMsg->Writefloat(fRange);
    pMsg->Writefloat(fVel);
    pMsg->Writefloat(fSoundRadius);
    pMsg->Writefloat(fParticleRadius);
    pMsg->Writefloat(fStartAlpha);
    pMsg->Writefloat(fEndAlpha);
    pMsg->Writefloat(fStartScale);
    pMsg->Writefloat(fEndScale);
    pMsg->Writefloat(fCreateDelta);
    pMsg->Writefloat(fVolumeRadius);
    pMsg->Writeuint8(nNumParticles);
    pMsg->WriteHString(hstrSoundName);
    pMsg->WriteHString(hstrParticle);
    pMsg->WriteLTVector(vMinDriftVel);
    pMsg->WriteLTVector(vMaxDriftVel);
    pMsg->WriteLTVector(vColor1);
    pMsg->WriteLTVector(vColor2);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	STEAMCREATESTRUCT::Read()
//
//	PURPOSE:	Read the data from the message
//
// ----------------------------------------------------------------------- //

void STEAMCREATESTRUCT::Read(ILTMessage_Read *pMsg)
{
	if (!pMsg) return;

    SFXCREATESTRUCT::Read(pMsg);

    fRange          = pMsg->Readfloat();
    fVel            = pMsg->Readfloat();
    fSoundRadius    = pMsg->Readfloat();
    fParticleRadius = pMsg->Readfloat();
    fStartAlpha     = pMsg->Readfloat();
    fEndAlpha       = pMsg->Readfloat();
    fStartScale     = pMsg->Readfloat();
    fEndScale       = pMsg->Readfloat();
    fCreateDelta    = pMsg->Readfloat();
    fVolumeRadius   = pMsg->Readfloat();
    nNumParticles   = pMsg->Readuint8();
    hstrSoundName   = pMsg->ReadHString();
    hstrParticle    = pMsg->ReadHString();
    vMinDriftVel	= pMsg->ReadLTVector();
    vMaxDriftVel	= pMsg->ReadLTVector();
    vColor1			= pMsg->ReadLTVector();
    vColor2			= pMsg->ReadLTVector();
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

void EXPLOSIONCREATESTRUCT::Write(ILTMessage_Write *pMsg)
{
	if (!pMsg) return;

    SFXCREATESTRUCT::Write(pMsg);

	pMsg->Writeuint8(nImpactFX);
	pMsg->Writefloat(fDamageRadius);
	pMsg->WriteLTVector(vPos);
	pMsg->WriteLTRotation(rRot);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	EXPLOSIONCREATESTRUCT::Read()
//
//	PURPOSE:	Read the data from the message
//
// ----------------------------------------------------------------------- //

void EXPLOSIONCREATESTRUCT::Read(ILTMessage_Read *pMsg)
{
	if (!pMsg) return;

    SFXCREATESTRUCT::Read(pMsg);

    nImpactFX       = pMsg->Readuint8();
    fDamageRadius   = pMsg->Readfloat();
	vPos			= pMsg->ReadLTVector();
	rRot			= pMsg->ReadLTRotation();
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

void SPRINKLETYPECREATESTRUCT::Write(ILTMessage_Write *pMsg)
{
	if (!pMsg) return;

	pMsg->WriteHString(m_hFilename);
	pMsg->WriteHString(m_hSkinName);
	pMsg->Writeuint32(m_Count);
	pMsg->Writefloat(m_Speed);
	pMsg->Writefloat(m_Size);
	pMsg->Writefloat(m_SpawnRadius);
	pMsg->WriteLTVector(m_AnglesVel);
    pMsg->WriteLTVector(m_ColorMin);
    pMsg->WriteLTVector(m_ColorMax);

}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	SPRINKLETYPECREATESTRUCT::Read()
//
//	PURPOSE:	Read the data from the message
//
// ----------------------------------------------------------------------- //

void SPRINKLETYPECREATESTRUCT::Read(ILTMessage_Read *pMsg)
{
	if (!pMsg) return;

    m_hFilename     = pMsg->ReadHString();
    m_hSkinName     = pMsg->ReadHString();
    m_Count         = pMsg->Readuint32();
    m_Speed         = pMsg->Readfloat();
    m_Size          = pMsg->Readfloat();
    m_SpawnRadius   = pMsg->Readfloat();
    m_AnglesVel		= pMsg->ReadLTVector();
	m_ColorMin		= pMsg->ReadLTVector();
	m_ColorMax		= pMsg->ReadLTVector();

}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	SPRINKLESCREATESTRUCT::Write()
//
//	PURPOSE:	Write the data to the message
//
// ----------------------------------------------------------------------- //

void SPRINKLESCREATESTRUCT::Write(ILTMessage_Write *pMsg)
{
	if (!pMsg) return;

    SFXCREATESTRUCT::Write(pMsg);

	pMsg->Writeuint32(m_nTypes);

    for (uint32 i = 0; i < m_nTypes && m_nTypes < MAX_SPRINKLE_TYPES; i++)
	{
        m_Types[i].Write(pMsg);
	}

}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	SPRINKLESCREATESTRUCT::Read()
//
//	PURPOSE:	Read the data from the message
//
// ----------------------------------------------------------------------- //

void SPRINKLESCREATESTRUCT::Read(ILTMessage_Read *pMsg)
{
	if (!pMsg) return;

    SFXCREATESTRUCT::Read(pMsg);

    m_nTypes = pMsg->Readuint32();

    for (uint32 i = 0; i < m_nTypes && m_nTypes < MAX_SPRINKLE_TYPES; i++)
	{
        m_Types[i].Read(pMsg);
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

void LTCREATESTRUCT::Write(ILTMessage_Write *pMsg)
{
	if (!pMsg) return;

    SFXCREATESTRUCT::Write(pMsg);

	pMsg->WriteLTVector(vColor);
	pMsg->WriteLTVector(vDims);
	pMsg->Writefloat(fAlpha);
	pMsg->Writefloat(fSpriteScale);
	pMsg->Writebool(bCreateSprite != LTFALSE);
	pMsg->WriteHString(hstrSpriteFilename);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	LTCREATESTRUCT::Read()
//
//	PURPOSE:	Read the data from the message
//
// ----------------------------------------------------------------------- //

void LTCREATESTRUCT::Read(ILTMessage_Read *pMsg)
{
	if (!pMsg) return;

    SFXCREATESTRUCT::Read(pMsg);

	vColor = pMsg->ReadLTVector();
	vDims = pMsg->ReadLTVector();
    fAlpha              = pMsg->Readfloat();
    fSpriteScale        = pMsg->Readfloat();
    bCreateSprite       = pMsg->Readbool() ? LTTRUE : LTFALSE;
    hstrSpriteFilename  = pMsg->ReadHString();
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

void MINECREATESTRUCT::Write(ILTMessage_Write *pMsg)
{
	if (!pMsg) return;

    SFXCREATESTRUCT::Write(pMsg);

	pMsg->Writefloat(fMinRadius);
	pMsg->Writefloat(fMaxRadius);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	MINECREATESTRUCT::Read()
//
//	PURPOSE:	Read the data from the message
//
// ----------------------------------------------------------------------- //

void MINECREATESTRUCT::Read(ILTMessage_Read *pMsg)
{
	if (!pMsg) return;

    SFXCREATESTRUCT::Read(pMsg);

    fMinRadius = pMsg->Readfloat();
    fMaxRadius = pMsg->Readfloat();
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

void PVCREATESTRUCT::Write(ILTMessage_Write *pMsg)
{
	if (!pMsg) return;

    SFXCREATESTRUCT::Write(pMsg);

	pMsg->Writeuint8(ePhysicsModel);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	PVCREATESTRUCT::Read()
//
//	PURPOSE:	Read the data from the message
//
// ----------------------------------------------------------------------- //

void PVCREATESTRUCT::Read(ILTMessage_Read *pMsg)
{
	if (!pMsg) return;

    SFXCREATESTRUCT::Read(pMsg);

    ePhysicsModel = (PlayerPhysicsModel) pMsg->Readuint8();
}


/////////////////////////////////////////////////////////////////////////////
//
// TRIGGERCREATESTRUCT class Definition
//
// (used by client-side CTriggerFX and server-side Trigger classes)
//
/////////////////////////////////////////////////////////////////////////////

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	TRIGGERCREATESTRUCT::Write()
//
//	PURPOSE:	Write the data to the message
//
// ----------------------------------------------------------------------- //

void TRIGGERCREATESTRUCT::Write( ILTMessage_Write *pMsg )
{
	if( !pMsg ) return;

	SFXCREATESTRUCT::Write( pMsg );

	pMsg->Writefloat( fHUDLookAtDist );
	pMsg->Writefloat( fHUDAlwaysOnDist );
	pMsg->Writebool( bLocked );
	pMsg->WriteCompLTVector( vDims );
	pMsg->Writeuint8( nTriggerTypeId );
	pMsg->Writeuint32( nPlayerInsideID );
	pMsg->Writeuint32( nPlayerOutsideID );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	TRIGGERCREATESTRUCT::Read()
//
//	PURPOSE:	Read the data from the message
//
// ----------------------------------------------------------------------- //

void TRIGGERCREATESTRUCT::Read( ILTMessage_Read *pMsg )
{
	if( !pMsg ) return;

	SFXCREATESTRUCT::Read( pMsg );

	fHUDLookAtDist		= pMsg->Readfloat();
	fHUDAlwaysOnDist	= pMsg->Readfloat();
	bLocked				= pMsg->Readbool();
	vDims				= pMsg->ReadCompLTVector();
	nTriggerTypeId		= pMsg->Readuint8();
	nPlayerInsideID		= pMsg->Readuint32();
	nPlayerOutsideID	= pMsg->Readuint32();
}


/////////////////////////////////////////////////////////////////////////////
//
// RADAROBJCREATESTRUCT class Definition
//
// (used by client-side CRadarObjectFX and server-side RadarObject classes)
//
/////////////////////////////////////////////////////////////////////////////

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	RADAROBJCREATESTRUCT::Write()
//
//	PURPOSE:	Write the data to the message
//
// ----------------------------------------------------------------------- //

void RADAROBJCREATESTRUCT::Write( ILTMessage_Write *pMsg )
{
	if( !pMsg ) return;

	SFXCREATESTRUCT::Write( pMsg );
	
	pMsg->Writeuint8( nRadarTypeId );
	pMsg->Writebool( bOn );
	pMsg->Writeuint8( nTeamId );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	RADAROBJCREATESTRUCT::Read()
//
//	PURPOSE:	Read the data from the message
//
// ----------------------------------------------------------------------- //

void RADAROBJCREATESTRUCT::Read( ILTMessage_Read *pMsg )
{
	if( !pMsg ) return;

	SFXCREATESTRUCT::Read( pMsg );
	
	nRadarTypeId	= pMsg->Readuint8();
	bOn				= pMsg->Readbool();
	nTeamId			= pMsg->Readuint8( );
	if( nTeamId > MAX_TEAMS )
		nTeamId = INVALID_TEAM;
}
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

#include "Stdafx.h"
#include "SharedFXStructs.h"
#include "VersionMgr.h"
#include "TriggerTypeDB.h"
#include "SoundFilterDB.h"
#include "NavMarkerTypeDB.h"


#ifdef _SERVERBUILD

#include "../ObjectDLL/PlayerObj.h"
#include "EngineTimer.h"
#include "../ObjectDLL/CharacterHitBox.h"

#endif // _SERVERBUILD

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

	pMsg->Writebool(bIsPlayer);
	if( !bIsPlayer )
	{
		pMsg->Writebool(bIsCinematicAI);
	}
	
	// Determine what data needs to be sent to the client by comparing it against default values.
	bool bSendFlags = byFXFlags != 0;
	bool bSendStealth = fStealthPercent != 1.0f;
	bool bSendCrosshair = eCrosshairPlayerStance != kCharStance_Like;
	bool bSendDamageFlags = nDamageFlags != 0;
	bool bUseDefaultHitboxDims = false;
	SERVER_CODE
	(
		CCharacter* pChar = CCharacter::DynamicCast( hServerObj );
		if( pChar )
		{
			CCharacterHitBox* pHitBox = CCharacterHitBox::DynamicCast( pChar->GetHitBox( ));
			if( pHitBox )
			{
				LTVector vDims;
				g_pPhysicsLT->GetObjectDims( pChar->m_hObject, &vDims );
				pHitBox->EnlargeDims( vDims );
				if( vHitBoxDims == vDims )
					bUseDefaultHitboxDims = true;
			}
		}
	)
	bool bSendHitBoxDims = !bUseDefaultHitboxDims;
	bool bSendHitBoxOffset = vHitBoxOffset != LTVector::GetIdentity();
	bool bSendPhysicsModel = ePlayerPhysicsModel != PPM_NORMAL;
	bool bSendSpectating = bIsSpectating;
	bool bSendTimers = false;
	SERVER_CODE
	(

		// Write out our timescale if we have one.  If we don't
		// have an object specific timer, then we can assume simulation time.
		nTimeScaleNumerator = 1;
		nTimeScaleDenominator = 1;
		EngineTimer engineTimer = hServerObj;
		if( engineTimer.IsValid( ))
		{
			engineTimer.GetTimerTimeScale( nTimeScaleNumerator, nTimeScaleDenominator );
			bSendTimers = ( nTimeScaleNumerator != nTimeScaleDenominator );
		}
	)
	bool bSendDead = bIsDead;
	
	// If all the data checked can use default values, we can send a single bit to indicate use defaults.
	// Otherwise, we send all the individual data values.
	if( !bSendFlags && !bSendStealth && !bSendCrosshair && !bSendDamageFlags && !bSendHitBoxDims && !bSendHitBoxOffset && 
		!bSendPhysicsModel && !bSendSpectating && !bSendTimers && !bSendDead && !bPermanentBody )
	{
		// Use all defaults.
		pMsg->Writebool( false );
	}
	else
	{
		// Can't use all defaults, need to send data.
		pMsg->Writebool( true );

		pMsg->WriteBits(byFXFlags, FNumBitsExclusive<eFlagLast>::k_nValue + 1);
/*
Hacked out stealth.

		if( fStealthPercent == 1.0f )
		{
			pMsg->Writebool( false );
		}
		else
		{
			pMsg->Writebool( true );
			pMsg->Writeuint8(( uint8 )( fStealthPercent * 255 + 0.5f ));
		}
*/
		pMsg->WriteBits(eCrosshairPlayerStance, FNumBitsExclusive<kCharStance_Count>::k_nValue);

		pMsg->Writebool( bSendDamageFlags );
		if( bSendDamageFlags )
		{
			pMsg->WriteType(nDamageFlags);
		}

		// Send the hitbox info
		pMsg->Writebool( bSendHitBoxDims );
		if( bSendHitBoxDims )
		{
			pMsg->WriteCompLTVector( vHitBoxDims );
		}

		pMsg->Writebool( bSendHitBoxOffset );
		if( bSendHitBoxOffset )
		{
			pMsg->WriteCompLTVector( vHitBoxOffset );
		}
		
		pMsg->Writebool( bSendPhysicsModel );
		if( bSendPhysicsModel )
		{
			pMsg->WriteBits(ePlayerPhysicsModel, FNumBitsExclusive<PPM_NUM_MODELS>::k_nValue );
		}

		pMsg->Writebool( bIsSpectating );
		pMsg->Writebool( bPermanentBody );

		SERVER_CODE
		(
			// Write out our timescale if we have one.  If we don't
			// have an object specific timer, then we can assume simulation time.
			pMsg->Writebool( bSendTimers );
			if( bSendTimers )
			{
				LTASSERT( nTimeScaleNumerator == ( uint16 )nTimeScaleNumerator, "Invalid timer scale." );
				LTASSERT( nTimeScaleDenominator == ( uint16 )nTimeScaleDenominator, "Invalid timer scale." );
				pMsg->Writeuint16(( uint16 )nTimeScaleNumerator );
				pMsg->Writeuint16(( uint16 )nTimeScaleDenominator );
			}
		)

		pMsg->Writebool(bSendDead);
		if( bSendDead )
		{
			pMsg->WriteBits(eDeathDamageType, FNumBitsExclusive<kNumDamageTypes>::k_nValue );
			pMsg->WriteCompLTPolarCoord( LTPolarCoord( vDeathDir ));
			pMsg->Writefloat( fDeathImpulseForce );
			pMsg->WriteDatabaseRecord( g_pLTDatabase, hModelNodeLastHit	);
			pMsg->Writefloat( fDeathNodeImpulseForceScale );
			pMsg->WriteDatabaseRecord( g_pLTDatabase, hDeathAmmo );
		}
	}


	SERVER_CODE
	(
		if( IsMultiplayerGameServer( ) && bIsPlayer)
		{
			pMsg->Writeuint8( nMPModelIndex );
		}
		else
		{
			pMsg->WriteDatabaseRecord( g_pLTDatabase, hModel);
		}
	)

	if( bIsPlayer )
	{
		pMsg->WriteBits(nClientID, FNumBitsExclusive<MAX_MULTI_PLAYERS*2>::k_nValue);
	}

	pMsg->WriteDatabaseRecord( g_pLTDatabase, hCurWeaponRecord );
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

	bIsPlayer					= pMsg->Readbool();
	if( !bIsPlayer )
	{
		bIsCinematicAI			= pMsg->Readbool();
	}

	bool bUseAllDefaults = !pMsg->Readbool();
	if( bUseAllDefaults )
	{
		byFXFlags = 0;
//		fStealthPercent = 1.0f;
		eCrosshairPlayerStance = kCharStance_Like;
		nDamageFlags = 0;
		bUseDefaultHitboxDims = true;
		vHitBoxOffset = LTVector::GetIdentity();
		ePlayerPhysicsModel = PPM_NORMAL;
		bIsSpectating = false;
		nTimeScaleNumerator = 1;
		nTimeScaleDenominator = 1;
		bIsDead = false;
		bPermanentBody = false;
	}
	else
	{
		byFXFlags					= pMsg->ReadBits( FNumBitsExclusive<eFlagLast>::k_nValue + 1);

/*
Hacked out stealth.
		if( !pMsg->Readbool())
		{
			fStealthPercent			= 1.0f;
		}
		else
		{
			fStealthPercent			= pMsg->Readuint8() / 255.0f;
		}
*/
		eCrosshairPlayerStance		= (EnumCharacterStance)pMsg->ReadBits( FNumBitsExclusive<kCharStance_Count>::k_nValue );
	
		if( !pMsg->Readbool( ))
		{
			nDamageFlags = 0;
		}
		else
		{
			pMsg->ReadType( &nDamageFlags );
		}

		// Update the hitbox dims.  If can't use defaults, characterfx will fix it up in its init.
		bUseDefaultHitboxDims = !pMsg->Readbool();
		if( !bUseDefaultHitboxDims )
		{
			vHitBoxDims		= pMsg->ReadCompLTVector();
		}

		if( !pMsg->Readbool())
		{
			vHitBoxOffset = LTVector::GetIdentity();
		}
		else
		{
			vHitBoxOffset	= pMsg->ReadCompLTVector();
		}

		if( !pMsg->Readbool())
		{
			ePlayerPhysicsModel = PPM_NORMAL;
		}
		else
		{
			ePlayerPhysicsModel = (PlayerPhysicsModel)pMsg->ReadBits( FNumBitsExclusive<PPM_NUM_MODELS>::k_nValue );
		}

		bIsSpectating = pMsg->Readbool( );
		bPermanentBody = pMsg->Readbool( );

		CLIENT_CODE
		(
			// Read the time scale sent from the server.
			nTimeScaleNumerator = 1;
			nTimeScaleDenominator = 1;
			if( pMsg->Readbool( ))
			{
				nTimeScaleNumerator = pMsg->Readuint16();
				nTimeScaleDenominator = pMsg->Readuint16();
			}
		)

// Note : Code for skipping the simulation timer must be present
// only when _CLIENTBUILD is not defined.
		SERVER_CODE
		(
			// Even though the data won't be used when 
			// read on the server, we need to read it anyway
			// so we don't mess up the message.
			if( pMsg->Readbool( ))
			{
				pMsg->Readuint16();
				pMsg->Readuint16();
			}
		)

		bIsDead						= pMsg->Readbool();
		if( bIsDead )
		{
			eDeathDamageType			= static_cast<DamageType>(pMsg->ReadBits( FNumBitsExclusive<kNumDamageTypes>::k_nValue ));
			vDeathDir					= pMsg->ReadCompLTPolarCoord();
			fDeathImpulseForce			= pMsg->Readfloat();
			hModelNodeLastHit			= (ModelsDB::HNODE)pMsg->ReadDatabaseRecord( g_pLTDatabase, g_pModelsDB->GetNodesCategory( ));
			fDeathNodeImpulseForceScale = pMsg->Readfloat();
			hDeathAmmo					= (HAMMO)pMsg->ReadDatabaseRecord( g_pLTDatabase, g_pWeaponDB->GetAmmoCategory( ));
		}
	}

	CLIENT_CODE
	(
		if( IsMultiplayerGameClient( ) && bIsPlayer)
		{
			nMPModelIndex = pMsg->Readuint8( );
		}
		else
		{
			hModel = ( ModelsDB::HMODEL )pMsg->ReadDatabaseRecord( g_pLTDatabase, g_pModelsDB->GetModelsCategory());
		}
	)
	SERVER_CODE
	(
		if( IsMultiplayerGameServer( ))
		{
			nMPModelIndex = pMsg->Readuint8( );
		}
		else
		{
			hModel = ( ModelsDB::HMODEL )pMsg->ReadDatabaseRecord( g_pLTDatabase, g_pModelsDB->GetModelsCategory());
		}
	)

	if( bIsPlayer )
	{
		nClientID = pMsg->ReadBits( FNumBitsExclusive<MAX_MULTI_PLAYERS*2>::k_nValue );
		if( nClientID == ( 1 << FNumBitsExclusive<MAX_MULTI_PLAYERS*2>::k_nValue ) - 1 )
			nClientID = INVALID_CLIENT;
	}
	else
	{
		nClientID = INVALID_CLIENT;
	}

	hCurWeaponRecord = pMsg->ReadDatabaseRecord( g_pLTDatabase, g_pWeaponDB->GetWeaponsCategory( ));

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
	pMsg->WriteLTVector(vPos);
	pMsg->WriteLTRotation(rRot);

	// If an ammo record is specified that's all that needs to be sent...
	if( hAmmo )
	{
		pMsg->Writebool( true );
		pMsg->WriteDatabaseRecord( g_pLTDatabase, hAmmo );
	}
	else
	{
		pMsg->Writebool( false );
		pMsg->Writefloat( fRadius );
		pMsg->Writefloat( fMinDamageRadius );
		pMsg->Writefloat( fImpulse );
	}
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

    nImpactFX       	= pMsg->Readuint8();
	vPos				= pMsg->ReadLTVector();
	rRot				= pMsg->ReadLTRotation();

	// Determine if only the ammo record is needed...
	bool bReadAmmo		= pMsg->Readbool( ); 
	if( bReadAmmo )
	{
		hAmmo			= pMsg->ReadDatabaseRecord( g_pLTDatabase, g_pWeaponDB->GetAmmoCategory( ));
	}
	else
	{
		fRadius			= pMsg->Readfloat( );
		fMinDamageRadius= pMsg->Readfloat( );
		fImpulse		= pMsg->Readfloat( );
	}
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
	pMsg->WriteDatabaseRecord(g_pLTDatabase, hTriggerTypeRecord );
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
	hTriggerTypeRecord	= pMsg->ReadDatabaseRecord(g_pLTDatabase, TriggerTypeDB::Instance().GetCategory());
	nPlayerInsideID		= pMsg->Readuint32();
	nPlayerOutsideID	= pMsg->Readuint32();
}

/////////////////////////////////////////////////////////////////////////////
//
// SOUNDNONPOINTCREATESTRUCT class Definition
//
// (used by client-side CSoundNonPointFX and server-side SoundNonPoint classes)
//
/////////////////////////////////////////////////////////////////////////////

// this is a helper struct, so the read and write aren't complete...
SOUNDNONPOINTZONECREATESTRUCT::SOUNDNONPOINTZONECREATESTRUCT()
:	m_vPos (0.0f, 0.0f, 0.0f),
	m_vHalfDims (0.0f, 0.0f, 0.0f),
	m_rRotation (0.0f, 0.0f, 0.0f)

{
}

void SOUNDNONPOINTZONECREATESTRUCT::Write(ILTMessage_Write *pMsg)
{
	pMsg->WriteCompLTVector(m_vPos);
	pMsg->WriteCompLTVector(m_vHalfDims);
	pMsg->WriteCompLTRotation(m_rRotation);
}
void SOUNDNONPOINTZONECREATESTRUCT::Read(ILTMessage_Read *pMsg)
{
	m_vPos = pMsg->ReadCompLTVector();
	m_vHalfDims = pMsg->ReadCompLTVector();
	m_rRotation = pMsg->ReadCompLTRotation();
}

// this is the main class.
SOUNDNONPOINTCREATESTRUCT::SOUNDNONPOINTCREATESTRUCT()
:	m_bStartOn (true),
m_fOuterRadius    (10.0f),
m_fInnerRadius	(5.0f),
m_nVolume (100),
m_hFilterRecord (NULL),
m_fPitchShift (1.0f),
m_nPriority (100),
m_nMixChannel (PLAYSOUND_MIX_DEFAULT),
m_fDopplerFactor (1.0f),
m_bUseOcclusion (true),
m_bOcclusionNoInnerRadius (false),
m_nNumZones (0)
{
}

void SOUNDNONPOINTCREATESTRUCT::Write(ILTMessage_Write *pMsg)
{
	if( !pMsg ) return;

	SFXCREATESTRUCT::Write( pMsg );

	int32 i;

	pMsg->WriteString(m_sSound.c_str());
	pMsg->Writebool(m_bStartOn);
	pMsg->Writebool(m_bSoundOn);
	pMsg->Writefloat(m_fOuterRadius);
	pMsg->Writefloat(m_fInnerRadius);
	pMsg->Writeint8(m_nVolume);
	pMsg->WriteDatabaseRecord(g_pLTDatabase,m_hFilterRecord);
	pMsg->Writefloat(m_fPitchShift);
	pMsg->Writeint8(m_nPriority);
	pMsg->Writeint16(m_nMixChannel);
	pMsg->Writefloat(m_fDopplerFactor);
	pMsg->Writebool(m_bUseOcclusion);
	pMsg->Writebool(m_bOcclusionNoInnerRadius);
	pMsg->Writeint16(m_nNumZones);

	for (i=0; i < m_nNumZones; i++)
	{
		m_SoundZone[i].Write(pMsg);
	}
}
void SOUNDNONPOINTCREATESTRUCT::Read(ILTMessage_Read *pMsg)
{
	if( !pMsg ) return;

	SFXCREATESTRUCT::Read( pMsg );

	int32 i;
	char readstringbuf[80];

	pMsg->ReadString(readstringbuf, 80);
	m_sSound = readstringbuf;
	m_bStartOn = pMsg->Readbool();
	m_bSoundOn = pMsg->Readbool();
	m_fOuterRadius = pMsg->Readfloat();
	m_fInnerRadius = pMsg->Readfloat();
	m_nVolume = pMsg->Readint8();
	m_hFilterRecord = pMsg->ReadDatabaseRecord(g_pLTDatabase, SoundFilterDB::Instance().GetSoundFilterCategory());
	m_fPitchShift = pMsg->Readfloat();
	m_nPriority = pMsg->Readint8();
	m_nMixChannel = pMsg->Readint16();
	m_fDopplerFactor = pMsg->Readfloat();
	m_bUseOcclusion = pMsg->Readbool();
	m_bOcclusionNoInnerRadius = pMsg->Readbool();
	m_nNumZones = pMsg->Readint16();

	for (i=0; i < m_nNumZones; i++)
	{
		m_SoundZone[i].Read(pMsg);
	}
}

VOLUMETRICLIGHTCREATESTRUCT::VOLUMETRICLIGHTCREATESTRUCT() :
	m_eLOD(eEngineLOD_Never),
	m_fNoiseIntensity(0.5f),
	m_fNoiseScale(1.0f),
	m_vColor(1.0f, 1.0f, 1.0f),
	m_sTexture(""),
	m_fAttenuation(1.0f),
	m_bAdditive(true),
	m_fDepth(100.0f),
	m_bShadow(true),
	m_fFarZ(300.0f)
{
}

void VOLUMETRICLIGHTCREATESTRUCT::Read(ILTMessage_Read *pMsg)
{
	if( !pMsg ) return;

	SFXCREATESTRUCT::Read( pMsg );

	m_eLOD = (EEngineLOD)pMsg->ReadBits(3);
	LTASSERT(eEngineLOD_NumLODTypes <= (1 << 3), "Engine LOD values changed.  Message update required.");
	m_fNoiseIntensity = pMsg->Readfloat();
	m_fNoiseScale = pMsg->Readfloat();
	m_vColor = pMsg->ReadLTVector();
	std::vector<char> aTempBuff;
	aTempBuff.resize(pMsg->PeekString(NULL, 0) + 1);
	pMsg->ReadString(&aTempBuff[0], aTempBuff.size());
	m_sTexture.assign(&aTempBuff[0], aTempBuff.size());
	m_fAttenuation = pMsg->Readfloat();
	m_bAdditive = pMsg->Readbool();
	m_fDepth = pMsg->Readfloat();
	m_bShadow = pMsg->Readbool();
	m_fFarZ = pMsg->Readfloat();
}

void VOLUMETRICLIGHTCREATESTRUCT::Write(ILTMessage_Write *pMsg)
{
	if( !pMsg ) return;

	SFXCREATESTRUCT::Write( pMsg );

	pMsg->WriteBits(m_eLOD, 3);
	LTASSERT(eEngineLOD_NumLODTypes <= (1 << 3), "Engine LOD values changed.  Message update required.");
	pMsg->Writefloat(m_fNoiseIntensity);
	pMsg->Writefloat(m_fNoiseScale);
	pMsg->WriteLTVector(m_vColor);
	pMsg->WriteString(m_sTexture.c_str());
	pMsg->Writefloat(m_fAttenuation);
	pMsg->Writebool(m_bAdditive);
	pMsg->Writefloat(m_fDepth);
	pMsg->Writebool(m_bShadow);
	pMsg->Writefloat(m_fFarZ);
}

void NAVMARKERCREATESTRUCT::Read(ILTMessage_Read *pMsg)
{
	m_bInstant = pMsg->Readbool();
	if( !m_bInstant )
	{
		m_bIsActive = pMsg->Readbool();
		m_hTarget = pMsg->ReadObject();
	}
	else
	{
		m_bIsActive = true;
		m_hTarget = NULL;
	}

	m_nClientID = pMsg->Readuint8();
	if( pMsg->Readbool())
	{
		m_nTeamId = pMsg->ReadBits( FNumBitsExclusive<MAX_TEAMS>::k_nValue );
	}
	else
	{
		m_nTeamId = INVALID_TEAM;
	}
	m_hType = pMsg->ReadDatabaseRecord(g_pLTDatabase,g_pNavMarkerTypeDB->GetCategory());
	if( pMsg->Readbool( ))
	{
		m_nStringId = pMsg->Readuint32();
		m_wsString.clear();
	}
	else
	{
		m_nStringId = -1;
		if( pMsg->Readbool())
		{
			wchar_t wszString[256] = L"";
			pMsg->ReadWString( wszString, LTARRAYSIZE( wszString ));
			m_wsString = wszString;
		}
		else
		{
			m_wsString.clear();
		}
	}
	m_bBroadcast = pMsg->Readbool();
	if( m_bInstant )
	{
		m_vPos = pMsg->ReadCompPos();
	}
}

void NAVMARKERCREATESTRUCT::Write(ILTMessage_Write *pMsg)
{
	if( !pMsg ) return;

	SFXCREATESTRUCT::Write( pMsg );

	pMsg->Writebool( m_bInstant );
	if( !m_bInstant )
	{
		pMsg->Writebool(m_bIsActive);
		pMsg->WriteObject(m_hTarget);
	}
	pMsg->Writeuint8( m_nClientID );
	bool bSendTeam = ( m_nTeamId != INVALID_TEAM );
	pMsg->Writebool( bSendTeam );
	if( bSendTeam )
	{
		pMsg->WriteBits(m_nTeamId, FNumBitsExclusive<MAX_TEAMS>::k_nValue);
	}
	pMsg->WriteDatabaseRecord( g_pLTDatabase, m_hType );
	bool bSendStringId = ( m_nStringId != -1 );
	pMsg->Writebool( bSendStringId );
	if( bSendStringId )
	{
		pMsg->Writeuint32(m_nStringId);
	}
	else 
	{
		bool bSendSecondaryString = ( !m_wsString.empty( ));
		pMsg->Writebool( bSendSecondaryString );
		if( bSendSecondaryString )
		{
			pMsg->WriteWString( m_wsString.c_str( ));
		}
	}
	pMsg->Writebool(m_bBroadcast);
	if( m_bInstant )
	{
		pMsg->WriteCompPos( m_vPos );
	}
}

// Read the physics constraint data based on the constraint type...
void PHYSICSCONSTRAINTCREATESTRUCT::Read( ILTMessage_Read *pMsg )
{
	if( !pMsg )
		return;

	m_eConstraintType	= static_cast<EConstraintType>(pMsg->ReadBits( FNumBitsExclusive<kConstraintType_NumTypes>::k_nValue ));
	
	m_bHasObject1		= pMsg->Readbool( );
	if( m_bHasObject1 )
		m_hObject1			= pMsg->ReadObject( );

	m_bHasObject2		= pMsg->Readbool( );
	if( m_bHasObject2 )
		m_hObject2			= pMsg->ReadObject( );

	switch( m_eConstraintType )
	{
		case kConstraintType_Hinge:
		{
			m_vPivotPt1		= pMsg->ReadLTVector( );
			m_vPivotPt2		= pMsg->ReadLTVector( );
			m_vPivotDir1	= pMsg->ReadLTVector( );
			m_vPivotDir2	= pMsg->ReadLTVector( );
		}
		break;

		case kConstraintType_LimitedHinge:
		{
			m_vPivotPt1		= pMsg->ReadLTVector( );
			m_vPivotPt2		= pMsg->ReadLTVector( );
			m_vPivotDir1	= pMsg->ReadLTVector( );
			m_vPivotDir2	= pMsg->ReadLTVector( );
			m_vPivotPerp1	= pMsg->ReadLTVector( );
			m_vPivotPerp2	= pMsg->ReadLTVector( );
			m_fAngleMin		= pMsg->Readfloat( );
			m_fAngleMax		= pMsg->Readfloat( );
			m_fFriction		= pMsg->Readfloat( );
		}
		break;

		case kConstraintType_Point:
		{
			m_vPivotPt1		= pMsg->ReadLTVector( );
			m_vPivotPt2		= pMsg->ReadLTVector( );
		}
		break;

		case kConstraintType_Prismatic:
		{
			m_vPivotPt1		= pMsg->ReadLTVector( );
			m_vPivotPt2		= pMsg->ReadLTVector( );
			m_vAxis1		= pMsg->ReadLTVector( );
			m_vAxis2		= pMsg->ReadLTVector( );
			m_vAxisPerp1	= pMsg->ReadLTVector( );
			m_vAxisPerp2	= pMsg->ReadLTVector( );
			m_fMovementMin	= pMsg->Readfloat( );
			m_fMovementMax	= pMsg->Readfloat( );
			m_fFriction		= pMsg->Readfloat( );
		}
		break;

		case kConstraintType_Ragdoll:
		{
			m_vPivotPt1		= pMsg->ReadLTVector( );
			m_vPivotPt2		= pMsg->ReadLTVector( );
			m_vTwist1		= pMsg->ReadLTVector( );
			m_vTwist2		= pMsg->ReadLTVector( );
			m_vPlane1		= pMsg->ReadLTVector( );
			m_vPlane2		= pMsg->ReadLTVector( );
			m_fConeAngle	= pMsg->Readfloat( );
			m_fPosCone		= pMsg->Readfloat( );
			m_fNegCone		= pMsg->Readfloat( );
			m_fTwistMin		= pMsg->Readfloat( );
			m_fTwistMax		= pMsg->Readfloat( );
			m_fFriction		= pMsg->Readfloat( );
		}
		break;

		case kConstraintType_StiffSpring:
		{
			m_vPivotPt1		= pMsg->ReadLTVector( );
			m_vPivotPt2		= pMsg->ReadLTVector( );
			m_fDistance		= pMsg->Readfloat( );
		}
		break;

		case kConstraintType_Wheel:
		{
			m_vPivotPt1				= pMsg->ReadLTVector( );
			m_vPivotPt2				= pMsg->ReadLTVector( );
			m_vRotation1			= pMsg->ReadLTVector( );
			m_vRotation2			= pMsg->ReadLTVector( );
			m_vSuspension2			= pMsg->ReadLTVector( );
			m_fSuspensionMin		= pMsg->Readfloat( );
			m_fSuspensionMax		= pMsg->Readfloat( );
			m_fSuspensionStrength	= pMsg->Readfloat( );
			m_fSuspensionDamping	= pMsg->Readfloat( );
		}
		break;		

		default:
		break;
	}
}

// Write the physics constraint data based on the constraint type...
void PHYSICSCONSTRAINTCREATESTRUCT::Write( ILTMessage_Write *pMsg )
{
	if( !pMsg )
		return;

	// Every constraint writes the type and objects with rigidbodies...
	pMsg->WriteBits( m_eConstraintType, FNumBitsExclusive<kConstraintType_NumTypes>::k_nValue );

	pMsg->Writebool( m_bHasObject1 );
	if( m_bHasObject1 )
		pMsg->WriteObject( m_hObject1 );

	pMsg->Writebool( m_bHasObject2 );
	if( m_bHasObject2 )
        pMsg->WriteObject( m_hObject2 );

	switch( m_eConstraintType )
	{
		case kConstraintType_Hinge:
		{
			pMsg->WriteLTVector( m_vPivotPt1 );
			pMsg->WriteLTVector( m_vPivotPt2 );
			pMsg->WriteLTVector( m_vPivotDir1 );
			pMsg->WriteLTVector( m_vPivotDir2 );
		}
		break;
			
		case kConstraintType_LimitedHinge:
		{
			pMsg->WriteLTVector( m_vPivotPt1 );
			pMsg->WriteLTVector( m_vPivotPt2 );
			pMsg->WriteLTVector( m_vPivotDir1 );
			pMsg->WriteLTVector( m_vPivotDir2 );
			pMsg->WriteLTVector( m_vPivotPerp1 );
			pMsg->WriteLTVector( m_vPivotPerp2 );
			pMsg->Writefloat( m_fAngleMin );
			pMsg->Writefloat( m_fAngleMax );
			pMsg->Writefloat( m_fFriction );
		}
		break;

		case kConstraintType_Point:
		{
			pMsg->WriteLTVector( m_vPivotPt1 );
			pMsg->WriteLTVector( m_vPivotPt2 );
		}
		break;

		case kConstraintType_Prismatic:
		{
			pMsg->WriteLTVector( m_vPivotPt1 );
			pMsg->WriteLTVector( m_vPivotPt2 );
			pMsg->WriteLTVector( m_vAxis1 );
			pMsg->WriteLTVector( m_vAxis2 );
			pMsg->WriteLTVector( m_vAxisPerp1 );
			pMsg->WriteLTVector( m_vAxisPerp2 );
			pMsg->Writefloat( m_fMovementMin );
			pMsg->Writefloat( m_fMovementMax );
			pMsg->Writefloat( m_fFriction );
		}
		break;

		case kConstraintType_Ragdoll:
		{
			pMsg->WriteLTVector( m_vPivotPt1 );
			pMsg->WriteLTVector( m_vPivotPt2 );
			pMsg->WriteLTVector( m_vTwist1 );
			pMsg->WriteLTVector( m_vTwist2 );
			pMsg->WriteLTVector( m_vPlane1 );
			pMsg->WriteLTVector( m_vPlane2 );
			pMsg->Writefloat( m_fConeAngle );
			pMsg->Writefloat( m_fPosCone );
			pMsg->Writefloat( m_fNegCone );
			pMsg->Writefloat( m_fTwistMin );
			pMsg->Writefloat( m_fTwistMax );
			pMsg->Writefloat( m_fFriction );
		}
		break;

		case kConstraintType_StiffSpring:
		{
			pMsg->WriteLTVector( m_vPivotPt1 );
			pMsg->WriteLTVector( m_vPivotPt2 );
			pMsg->Writefloat( m_fDistance );
		}
		break;

		case kConstraintType_Wheel:
		{
			pMsg->WriteLTVector( m_vPivotPt1 );
			pMsg->WriteLTVector( m_vPivotPt2 );
			pMsg->WriteLTVector( m_vRotation1 );
			pMsg->WriteLTVector( m_vRotation2 );
			pMsg->WriteLTVector( m_vSuspension2 );
			pMsg->Writefloat( m_fSuspensionMin );
			pMsg->Writefloat( m_fSuspensionMax );
			pMsg->Writefloat( m_fSuspensionStrength );
			pMsg->Writefloat( m_fSuspensionDamping );
		}
		break;		

		default:
		break;
	}
}

/////////////////////////////////////////////////////////////////////////////
//
// PICKUPITEMCREATESTRUCT class Definition
//
/////////////////////////////////////////////////////////////////////////////

void PICKUPITEMCREATESTRUCT::Read( ILTMessage_Read *pMsg )
{
	if( !pMsg )
		return;
	
	bool bReadClientFX = pMsg->Readbool( );
	if( bReadClientFX )
		Read_StdString( pMsg, m_sClientFX );

	bool bReadDroppedClientFX = pMsg->Readbool( );
	if( bReadDroppedClientFX )
		Read_StdString( pMsg, m_sDroppedClientFX );

	bool bReadTemaId = pMsg->Readbool( );
	if( bReadTemaId )
	{
		m_nTeamId = static_cast< uint8 >( pMsg->ReadBits( FNumBitsExclusive< MAX_TEAMS >::k_nValue ) );
	}
	else
	{
		m_nTeamId = INVALID_TEAM;
	}
	
	m_bLocked = pMsg->Readbool( );

	// Read data specific to the type of pickup.
	m_ePickupItemType = static_cast< PickupItemType >( pMsg->ReadBits( FNumBitsExclusive< kPickupItemType_NumPickupTypes >::k_nValue ) );
	switch( m_ePickupItemType )
	{
		case kPickupItemType_Weapon:
		{
			m_hRecord = pMsg->ReadDatabaseRecord( g_pLTDatabase, g_pWeaponDB->GetWeaponsCategory( ) );
		}
		break;
		
		case kPickupItemType_Gear:
		{	
			m_hRecord = pMsg->ReadDatabaseRecord( g_pLTDatabase, g_pWeaponDB->GetGearCategory( ) );
			m_hNavMarker = pMsg->ReadObject( );
		}
		break;
		
		default:
		break;
	}
}

void PICKUPITEMCREATESTRUCT::Write( ILTMessage_Write *pMsg )
{
	if( !pMsg )
		return;

	bool bSendClientFx = !m_sClientFX.empty( );
	pMsg->Writebool( bSendClientFx );
	if( bSendClientFx )
		pMsg->WriteString( m_sClientFX.c_str() );

	bool bSendDroppedFx = !m_sDroppedClientFX.empty( );
	pMsg->Writebool( bSendDroppedFx );
	if( bSendDroppedFx )
		pMsg->WriteString( m_sDroppedClientFX.c_str() );

	bool bSendTeam = ( m_nTeamId != INVALID_TEAM );
	pMsg->Writebool( bSendTeam );
	if( bSendTeam )
	{
		pMsg->WriteBits( m_nTeamId, FNumBitsExclusive< MAX_TEAMS >::k_nValue );
	}

	pMsg->Writebool( m_bLocked );

	pMsg->WriteBits( m_ePickupItemType, FNumBitsExclusive< kPickupItemType_NumPickupTypes >::k_nValue );
	switch( m_ePickupItemType )
	{
		case kPickupItemType_Weapon:
		{
			pMsg->WriteDatabaseRecord( g_pLTDatabase, m_hRecord );
		}
		break;

		case kPickupItemType_Gear:
		{	
			pMsg->WriteDatabaseRecord( g_pLTDatabase, m_hRecord );
			pMsg->WriteObject( m_hNavMarker );
		}
		break;

		default:
		break;
	}
}

void PICKUPITEMCREATESTRUCT::Load( ILTMessage_Read *pMsg )
{
	if( !pMsg )
		return;

	bool bReadClientFX = pMsg->Readbool( );
	if( bReadClientFX )
		Read_StdString( pMsg, m_sClientFX );

	bool bReadDroppedClientFX = pMsg->Readbool( );
	if( bReadDroppedClientFX )
		Read_StdString( pMsg, m_sDroppedClientFX );

	bool bReadTemaId = pMsg->Readbool( );
	if( bReadTemaId )
	{
		m_nTeamId = static_cast< uint8 >( pMsg->ReadBits( FNumBitsExclusive< MAX_TEAMS >::k_nValue ) );
	}
	else
	{
		m_nTeamId = INVALID_TEAM;
	}

	m_bLocked = pMsg->Readbool( );

	// Read data specific to the type of pickup.
	m_ePickupItemType = static_cast< PickupItemType >( pMsg->ReadBits( FNumBitsExclusive< kPickupItemType_NumPickupTypes >::k_nValue ) );
	switch( m_ePickupItemType )
	{
	case kPickupItemType_Weapon:
		{
			m_hRecord = pMsg->ReadDatabaseRecord( g_pLTDatabase, g_pWeaponDB->GetWeaponsCategory( ) );
		}
		break;

	case kPickupItemType_Gear:
		{	
			m_hRecord = pMsg->ReadDatabaseRecord( g_pLTDatabase, g_pWeaponDB->GetGearCategory( ) );
			//HACK to fix save/load bug caused by adding a gear item between versions
			if (g_pVersionMgr->GetCurrentSaveVersion( ) <= CVersionMgr::kSaveVersion__1_04)
			{
				uint8 nGearIndex = g_pWeaponDB->GetRecordIndex(m_hRecord);
				//a new gear item was added at index 11, so if we load one that is at 11 or higher, it needs to be bumped
				if (nGearIndex >= 11)
				{
					m_hRecord = g_pWeaponDB->GetGearRecord(nGearIndex+1);
				}
			}
			
			m_hNavMarker = pMsg->ReadObject( );
		}
		break;

	default:
		break;
	}
}



/////////////////////////////////////////////////////////////////////////////
//
// PROJECTILECREATESTRUCT class Definition
//
/////////////////////////////////////////////////////////////////////////////

void PROJECTILECREATESTRUCT::Read( ILTMessage_Read *pMsg )
{
	if( !pMsg )
		return;

	m_hWeapon = pMsg->ReadDatabaseRecord( g_pLTDatabase, g_pWeaponDB->GetWeaponsCategory( ) );
	m_hAmmo = pMsg->ReadDatabaseRecord( g_pLTDatabase, g_pWeaponDB->GetAmmoCategory( ) );
	m_bFiredFromValid = pMsg->Readbool();
	if( m_bFiredFromValid )
		m_hFiredFrom = pMsg->ReadObject( );
	else
		m_hFiredFrom = NULL;
	m_nTeamId = pMsg->Readuint8( );
	
	Read_StdString( pMsg, m_sOverrideFX );
	Read_StdString( pMsg, m_sSameTeamFX );
	Read_StdString( pMsg, m_sOtherTeamFX );

}

void PROJECTILECREATESTRUCT::Write( ILTMessage_Write *pMsg )
{
	if( !pMsg )
		return;

	pMsg->WriteDatabaseRecord( g_pLTDatabase, m_hWeapon );
	pMsg->WriteDatabaseRecord( g_pLTDatabase, m_hAmmo );
	bool bFiredFromValid = ( m_hFiredFrom != NULL );
	pMsg->Writebool( bFiredFromValid );
	if( bFiredFromValid )
		pMsg->WriteObject( m_hFiredFrom );
	pMsg->Writeuint8( m_nTeamId );
	pMsg->WriteString( m_sOverrideFX.c_str( ) );
	pMsg->WriteString( m_sSameTeamFX.c_str( ) );
	pMsg->WriteString( m_sOtherTeamFX.c_str( ) );
}

// EOF

// ----------------------------------------------------------------------- //
//
// MODULE  : AISensorCloak.cpp
//
// PURPOSE : AISensorCloak class implementation
//
// CREATED : 04/22/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "AISensorCloak.h"
#include "AI.h"
#include "AIDB.h"
#include "AIWorkingMemory.h"
#include "AIBlackBoard.h"
#include "ServerSoundMgr.h"
#include "FxDefs.h"

DEFINE_AI_FACTORY_CLASS_SPECIFIC( Sensor, CAISensorCloak, kSensor_Cloak );

#define FORCE	true
#define ASSASSIN_CLOAK_SOUND		"AssassinCloakSound"
#define ASSASSIN_UNCLOAK_SOUND		"AssassinUncloakSound"
#define ASSASSIN_UNCLOAK_THRESHOLD	"AssassinUncloakThreshold"
#define ASSASSIN_UNCLOAK_DURATION	"AssassinUncloakDuration"
#define ASSASSIN_CLOAK_FX			"Test_Ass_Cloak"

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISensorCloak::Con/destructor
//
//	PURPOSE:	Factory Con/destructor
//
// ----------------------------------------------------------------------- //

CAISensorCloak::CAISensorCloak()
{
	m_bCloakingInitialized = false;
	m_bCloaked = true;
	m_bCloakDamaged = false;
	m_fLastCloakTime = 0.f;
	m_fUncloakDuration = 0.f;
	m_fMinCloakDamageStim = 0.f;
	m_fDamageThreshold = 0.f;
	m_hLoopSound = NULL;
}

CAISensorCloak::~CAISensorCloak()
{
	// Uncloak the AI.

	Cloak( false, m_pAI, FORCE );

	// Spark.

	SpawnCloakFX( m_pAI->m_hObject );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISensorCloak::Save
//
//	PURPOSE:	Save the sensor
//
// ----------------------------------------------------------------------- //

void CAISensorCloak::Save(ILTMessage_Write *pMsg)
{
	super::Save( pMsg );

	SAVE_bool( m_bCloakingInitialized );
	SAVE_bool( m_bCloaked );
	SAVE_bool( m_bCloakDamaged );
	SAVE_FLOAT( m_fMinCloakDamageStim );
	SAVE_FLOAT( m_fDamageThreshold );
	SAVE_FLOAT( m_fUncloakDuration );
	SAVE_TIME( m_fLastCloakTime );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISensorCloak::Load
//
//	PURPOSE:	Load the sensor
//
// ----------------------------------------------------------------------- //

void CAISensorCloak::Load(ILTMessage_Read *pMsg)
{
	super::Load( pMsg );

	LOAD_bool( m_bCloakingInitialized );
	LOAD_bool( m_bCloaked );
	LOAD_bool( m_bCloakDamaged );
	LOAD_FLOAT( m_fMinCloakDamageStim );
	LOAD_FLOAT( m_fDamageThreshold );
	LOAD_FLOAT( m_fUncloakDuration );
	LOAD_TIME( m_fLastCloakTime );

	Cloak( m_bCloaked, m_pAI, FORCE );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISensorCloak::UpdateSensor
//
//	PURPOSE:	Return true if this sensor updated, and the SensorMgr
//              should wait to update others.
//
// ----------------------------------------------------------------------- //

bool CAISensorCloak::UpdateSensor()
{
	if( !super::UpdateSensor() )
	{
		return false;
	}

	// Initialize cloaking.

	if( !m_bCloakingInitialized )
	{
		InitializeCloaking();
	}

	// Damage fact indicates current stimulation due to damage.

	float fDamageStim = 0.f;
	CAIWMFact factDamageQuery;
	factDamageQuery.SetFactType(kFact_Damage);
	CAIWMFact* pFactDamage = m_pAI->GetAIWorkingMemory()->FindWMFact( factDamageQuery );
	if( pFactDamage )
	{
		fDamageStim = pFactDamage->GetConfidence( CAIWMFact::kFactMask_Stimulus );
	}

	m_fMinCloakDamageStim = LTMIN( m_fMinCloakDamageStim, fDamageStim );

	// Flee when damaged enough while retreating.

	if( pFactDamage 
		&& DidDamage( m_pAI, pFactDamage )
		&& fDamageStim >= m_fMinCloakDamageStim + m_fDamageThreshold )
	{
		CAIWMFact factQuery;
		factQuery.SetFactType(kFact_Desire);
		factQuery.SetDesireType(kDesire_Retreat);
		CAIWMFact* pFact = m_pAI->GetAIWorkingMemory()->FindWMFact( factQuery );
		if( pFact )
		{
			// Find an existing memory for the desire to flee, or create one.

			factQuery.SetDesireType(kDesire_Flee);
			pFact = m_pAI->GetAIWorkingMemory()->FindWMFact( factQuery );
			if( !pFact )
			{
				pFact = m_pAI->GetAIWorkingMemory()->CreateWMFact( kFact_Desire );
				pFact->SetDesireType( kDesire_Flee, 1.f );

				// Immediately re-evaluate goals if cloaking fails.

				m_pAI->GetAIBlackBoard()->SetBBSelectAction( true );
			}
		}
	}


	// Uncomment the following lines to prevent AI from ever cloaking.
	//Cloak( false, m_pAI, !FORCE );
	//return true;

	//
	// Uncloak if AI has the desire to uncloak.
	//

	// Find an existing memory for the desire to uncloak.

	CAIWMFact factCloakQuery;
	factCloakQuery.SetFactType(kFact_Desire);
	factCloakQuery.SetDesireType(kDesire_Uncloak);
	CAIWMFact* pFact = m_pAI->GetAIWorkingMemory()->FindWMFact( factCloakQuery );
	if( !pFact )
	{
		factCloakQuery.SetDesireType(kDesire_NeverCloak);
		pFact = m_pAI->GetAIWorkingMemory()->FindWMFact( factCloakQuery );
	}
	if( pFact )
	{
		// Uncloak if necessary.

		if( m_bCloaked )
		{
			Cloak( false, m_pAI, !FORCE );
		}

		return true;
	}


	//
	// Uncloak when taking enough damage.
	//

	if( m_bCloaked )
	{
		if( pFactDamage &&
			( DidDamage( m_pAI, pFactDamage ) ) &&
			( fDamageStim >= m_fMinCloakDamageStim + m_fDamageThreshold ) &&
			( pFactDamage->GetUpdateTime() > m_fLastCloakTime ) )
		{
			m_bCloakDamaged = true;
			Cloak( false, m_pAI, !FORCE );
			SpawnCloakFX( m_pAI->m_hObject );
		}

		return true;
	}


	//
	// We NOT are currently cloaked.
	//

	// Cloak if damage was not recent.

	if( ( !pFactDamage ) ||
		( pFactDamage->GetUpdateTime() < g_pLTServer->GetTime() - m_fUncloakDuration ) )
	{
		Cloak( true, m_pAI, !FORCE );

		if( m_bCloakDamaged )
		{
			m_bCloakDamaged = false;
			SpawnCloakFX( m_pAI->m_hObject );
			m_fMinCloakDamageStim = fDamageStim;
		}
	}

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISensorCloak::InitializeCloaking
//
//	PURPOSE:	Initialize cloaking.
//
// ----------------------------------------------------------------------- //

void CAISensorCloak::InitializeCloaking()
{
	m_bCloakingInitialized = true;

	// Data-driven uncloak threshold and duration.

	m_fDamageThreshold = g_pAIDB->GetMiscFloat( ASSASSIN_UNCLOAK_THRESHOLD );
	m_fUncloakDuration = g_pAIDB->GetMiscFloat( ASSASSIN_UNCLOAK_DURATION );

	// Use the armored impact FX while cloaked.

	m_pAI->GetAIBlackBoard()->SetBBSurfaceOverride( g_pModelsDB->GetArmorSurfaceType( m_pAI->GetModel() ) );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISensorCloak::Cloak
//
//	PURPOSE:	Cloak the AI.
//
// ----------------------------------------------------------------------- //

void CAISensorCloak::Cloak( bool bCloak, ILTBaseClass* pObj, bool bForce )
{
	// Nothing has changed.

	if( ( !bForce ) && 
		( bCloak == m_bCloaked ) )
	{
		return;
	}

	// Record new state.

	m_bCloaked = bCloak;

	// Send a command to hide or show the correct pieces of the model.
	// NOTE: Eventually swap materials instead.

	if( m_bCloaked )
	{
		m_fLastCloakTime = g_pLTServer->GetTime();

		g_pCmdMgr->QueueMessage( pObj, pObj, "HIDEPIECE body 1" );
		g_pCmdMgr->QueueMessage( pObj, pObj, "HIDEPIECE hands 1" );
		g_pCmdMgr->QueueMessage( pObj, pObj, "HIDEPIECE body0 0" );
		g_pCmdMgr->QueueMessage( pObj, pObj, "HIDEPIECE hands0 0" );
		g_pCmdMgr->QueueMessage( pObj, pObj, "HIDE_ATTACHFX" );

		// Use the armored impact FX while cloaked.

		m_pAI->GetAIBlackBoard()->SetBBSurfaceOverride( g_pModelsDB->GetArmorSurfaceType( m_pAI->GetModel() ) );

		// Loop the cloak sound. 

		HRECORD hRecord = g_pAIDB->GetMiscRecordLink( ASSASSIN_CLOAK_SOUND );
		const char* pszSoundFile = g_pLTDatabase->GetRecordName( hRecord );
		HRECORD hSR = g_pSoundDB->GetSoundDBRecord( pszSoundFile );

		m_hLoopSound = g_pServerSoundMgr->PlayDBSoundFromObject( pObj->m_hObject, hSR,
			-1.0f, SOUNDPRIORITY_MISC_LOW, PLAYSOUND_LOOP | PLAYSOUND_GETHANDLE,
			SMGR_DEFAULT_VOLUME, 1.0f, -1.0f,
			DEFAULT_SOUND_CLASS, PLAYSOUND_MIX_OBJECTS);
	}
	else {
		g_pCmdMgr->QueueMessage( pObj, pObj, "HIDEPIECE body 0" );
		g_pCmdMgr->QueueMessage( pObj, pObj, "HIDEPIECE hands 0" );
		g_pCmdMgr->QueueMessage( pObj, pObj, "HIDEPIECE body0 1" );
		g_pCmdMgr->QueueMessage( pObj, pObj, "HIDEPIECE hands0 1" );
		g_pCmdMgr->QueueMessage( pObj, pObj, "SHOW_ATTACHFX" );

		// Bleed while uncloaked.

		m_pAI->GetAIBlackBoard()->SetBBSurfaceOverride( ST_UNKNOWN );

		// Kill the looping cloak sound.

		if( m_hLoopSound )
		{
			g_pLTServer->SoundMgr()->KillSound( m_hLoopSound );
			m_hLoopSound = NULL;
		}

		// Play the non-looping uncloak sound.

		HRECORD hRecord = g_pAIDB->GetMiscRecordLink( ASSASSIN_UNCLOAK_SOUND );
		const char* pszSoundFile = g_pLTDatabase->GetRecordName( hRecord );
		HRECORD hSR = g_pSoundDB->GetSoundDBRecord( pszSoundFile );

		g_pServerSoundMgr->PlayDBSoundFromObject( pObj->m_hObject, hSR,
			-1.0f, SOUNDPRIORITY_MISC_LOW, 0,
			SMGR_DEFAULT_VOLUME, 1.0f, -1.0f,
			DEFAULT_SOUND_CLASS, PLAYSOUND_MIX_OBJECTS);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISensorCloak::SpawnCloakFX
//
//	PURPOSE:	Spawn cloak FX.
//
// ----------------------------------------------------------------------- //

void CAISensorCloak::SpawnCloakFX( HOBJECT hObject )
{
	LTRigidTransform tTransform;
	g_pLTServer->GetObjectTransform( hObject, &tTransform );

	CAutoMessage cMsg;
	cMsg.Writeuint8( SFX_CLIENTFXGROUPINSTANT );
	cMsg.WriteString( ASSASSIN_CLOAK_FX );
	cMsg.Writebool( false ); // FXFLAG_LOOP
	cMsg.Writebool( false ); // FXFLAG_NOSMOOTHSHUTDOWN
	cMsg.Writebool( false ); // Parent
	cMsg.WriteLTVector( tTransform.m_vPos );
	cMsg.WriteCompLTRotation( tTransform.m_rRot );
	cMsg.Writebool( false );

	g_pLTServer->SendSFXMessage( cMsg.Read(), 0 );
}

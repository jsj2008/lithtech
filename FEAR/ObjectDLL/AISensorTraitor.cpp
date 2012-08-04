// ----------------------------------------------------------------------- //
//
// MODULE  : AISensorTraitor.cpp
//
// PURPOSE : 
//
// CREATED : 3/15/05
//
// (c) 2005 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "AISensorTraitor.h"
#include "AIStimulusMgr.h"
#include "CharacterDB.h"

DEFINE_AI_FACTORY_CLASS_SPECIFIC( Sensor, CAISensorTraitor, kSensor_Traitor );

// Helper function for accessing the database; returns the percent damage an 
// AI must sustain from an ally before the AI decides the ally is a traitor.
static float GetTraitorDamageAmount( CAIDB* pAIDB, CAI* pAI )
{
	HRECORD hMisc = pAIDB->GetMiscRecord();
	
	if ( NULL == hMisc )
	{
		return 0;
	}

	// Look for an override specialized for this model.

	HATTRIBUTE hAtt = pAIDB->GetAttribute( hMisc, "TraitorDamageAmountOverride" );
	for ( uint32 iEachOverride = 0; ; ++iEachOverride )
	{
		HATTRIBUTE hModelTypeAtt = pAIDB->GetStructAttribute( hAtt, iEachOverride, "Model" );

		// End of the list; no match found.

		if ( NULL == hModelTypeAtt )
		{
			break;
		}

		// Not a match.

		if ( pAI->GetModel() != pAIDB->GetRecordLink( hModelTypeAtt ) )
		{
			continue;
		}

		// Found a match!  Return the kick count.

		HATTRIBUTE hKickCountAtt = pAIDB->GetStructAttribute( hAtt, iEachOverride, "TraitorDamageAmount" );
		return pAIDB->GetFloat( hKickCountAtt );
	}

	// Get the default.

	return pAIDB->GetFloat( hMisc, "TraitorDamageAmountDefault" );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISensorTraitor::Con/destructor
//
//	PURPOSE:	Factory Con/destructor
//
// ----------------------------------------------------------------------- //

CAISensorTraitor::CAISensorTraitor() : 
	m_flDamageThreshold( 1.0f )
{
}

CAISensorTraitor::~CAISensorTraitor()
{
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAISensorTraitor::Save/Load
//              
//	PURPOSE:	Handle saving and restoring the CAISensorTraitor
//              
//----------------------------------------------------------------------------

void CAISensorTraitor::Load(ILTMessage_Read *pMsg)
{
	super::Load(pMsg);

	LOAD_FLOAT( m_flDamageThreshold );
	for ( int i = 0; i < LTARRAYSIZE(m_aFriendlyFireHistory); ++i )
	{
		LOAD_FLOAT( m_aFriendlyFireHistory[i].m_flLastDamageTime );
		LOAD_FLOAT( m_aFriendlyFireHistory[i].m_flTotalDamage );
		LOAD_HOBJECT( m_aFriendlyFireHistory[i].m_hDamager );
	}
	LOAD_HOBJECT( m_hTraitorCharacter );
}

void CAISensorTraitor::Save(ILTMessage_Write *pMsg)
{
	super::Save(pMsg);

	SAVE_FLOAT( m_flDamageThreshold );
	for ( int i = 0; i < LTARRAYSIZE(m_aFriendlyFireHistory); ++i )
	{
		SAVE_DOUBLE( m_aFriendlyFireHistory[i].m_flLastDamageTime );
		SAVE_FLOAT( m_aFriendlyFireHistory[i].m_flTotalDamage );
		SAVE_HOBJECT( m_aFriendlyFireHistory[i].m_hDamager );
	}
	SAVE_HOBJECT( m_hTraitorCharacter );
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAISensorTraitor::InitSensor
//              
//	PURPOSE:	
//              
//----------------------------------------------------------------------------

void CAISensorTraitor::InitSensor( EnumAISensorType eSensorType, CAI* pAI )
{
	super::InitSensor( eSensorType, pAI );

	// Read the damage threshold out of the database.

	m_flDamageThreshold = GetTraitorDamageAmount( g_pAIDB, pAI );
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAISensorTraitor::StimulateSensor
//              
//	PURPOSE:	
//              
//----------------------------------------------------------------------------

bool CAISensorTraitor::StimulateSensor( CAIStimulusRecord* pStimulusRecord )
{
	// Ignore records other than damage.

	if ( NULL == pStimulusRecord 
		|| ( 0 == ( pStimulusRecord->m_eStimulusType & kStim_DamageMelee ) ) 
		)
	{
		return false;
	}

	// Extract useful fields from the Stimulus record for clarity.

	HOBJECT hDamager		= pStimulusRecord->m_hStimulusTarget;
	float flDamageAmount	= pStimulusRecord->m_fDamageAmount;

	// Ignore the damager if they are not a ai (we don't want to treat the 
	// player as a traitor here).

	CAI* pDamager = CAI::DynamicCast( hDamager );
	if ( !pDamager )
	{
		return false;
	}

	// Ignore the record if the stance towards the damager is not LIKE

	EnumCharacterStance eStance = g_pCharacterDB->GetStance( m_pAI->GetAlignment(), pDamager->GetAlignment() );
	if ( kCharStance_Like != eStance )
	{
		return false;
	}

	// Record the damage.

	int iRecord = GetFireRecordIndex( hDamager );
	if ( m_aFriendlyFireHistory[iRecord].m_hDamager != hDamager )
	{
		m_aFriendlyFireHistory[iRecord].Clear();
		m_aFriendlyFireHistory[iRecord].m_hDamager = hDamager;
	}
	m_aFriendlyFireHistory[iRecord].m_flTotalDamage += flDamageAmount;
	m_aFriendlyFireHistory[iRecord].m_flLastDamageTime = g_pLTServer->GetTime();

	// Update the target if it is warranted and the AI is in a good state 
	// to attack this enemy.

	if ( (HOBJECT)NULL == m_hTraitorCharacter )
	{
		float flTotalPercentDamaged = m_aFriendlyFireHistory[iRecord].m_flTotalDamage / m_pAI->GetDestructible()->GetMaxHitPoints();
		if ( flTotalPercentDamaged > m_flDamageThreshold )
		{
			m_hTraitorCharacter = m_aFriendlyFireHistory[iRecord].m_hDamager;
		}
	}

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISensorTraitor::UpdateSensor
//
//	PURPOSE:	Return true if this sensor updated, and the SensorMgr
//              should wait to update others.
//
//	TODO:		This function is a big hack.  This feature is still 
//				loosely defined, so 
//
// ----------------------------------------------------------------------- //

bool CAISensorTraitor::UpdateSensor()
{
	bool bRet = super::UpdateSensor();

	if ( !IsDeadCharacter( m_hTraitorCharacter ) )
	{
		CCharacter* pTraitor = CCharacter::DynamicCast( m_hTraitorCharacter );
		if ( pTraitor )
		{
			CAIStimulusRecord* pStimulusRecord = g_pAIStimulusMgr->GetStimulusRecord( pTraitor->GetEnemyVisibleStimulusID() );
			if ( pStimulusRecord )
			{
				CAISensorAbstract* pSensor = m_pAI->GetAISensorMgr()->FindSensor(  kSensor_SeeEnemy );
				if ( pSensor )
				{
					pSensor->StimulateSensor( pStimulusRecord );

					// Insure the fact, if it exists, has the special 'this is 
					// an ally' flag set to prevent deletion due to incorrect 
					// stance

					CAIWMFact queryFact;
					queryFact.SetFactType( kFact_Character );
					queryFact.SetTargetObject( m_hTraitorCharacter );
					CAIWMFact* pFact = m_pAI->GetAIWorkingMemory()->FindWMFact( queryFact );
					if ( pFact )
					{
						pFact->SetFactFlags( pFact->GetFactFlags() | kFactFlag_CharacterIsTraitor );
					}

					return true;
				}
			}
		}
	}

	return false;
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAISensorTraitor::GetFireRecordIndex
//              
//	PURPOSE:	Returns the index into m_aFriendlyFireHistory to use for 
//				storing information about friendly fire from the passed 
//				in damager.
//              
//----------------------------------------------------------------------------

int CAISensorTraitor::GetFireRecordIndex( HOBJECT hDamager )
{
	// Look for:
	// 0) Look for a matching damager.
	// 1) An unused friendly fire record, if one exists.
	// 2) Otherwise, the oldest record.

	for ( int i = 0; i < LTARRAYSIZE( m_aFriendlyFireHistory ); ++i )
	{
		if ( m_aFriendlyFireHistory[i].m_hDamager == hDamager )
			return i;
	}

	for ( int i = 0; i < LTARRAYSIZE( m_aFriendlyFireHistory ); ++i )
	{
		if ( (HOBJECT)NULL == m_aFriendlyFireHistory[i].m_hDamager )
			return i;
	}

	int iOldestRecord = 0;
	double flOldestRecordTime = DBL_MAX;
	for ( int i = 0; i < LTARRAYSIZE( m_aFriendlyFireHistory ); ++i )
	{
		if ( m_aFriendlyFireHistory[i].m_flLastDamageTime < flOldestRecordTime )
		{
			iOldestRecord		= i;
			flOldestRecordTime	= m_aFriendlyFireHistory[i].m_flLastDamageTime;
		}
	}

	return iOldestRecord;
}

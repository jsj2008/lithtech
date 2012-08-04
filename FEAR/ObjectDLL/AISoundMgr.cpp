// ----------------------------------------------------------------------- //
//
// MODULE  : AISoundMgr.cpp
//
// PURPOSE : AISoundMgr implementation
//
// CREATED : 6/24/03
//
// (c) 2003 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "AISoundMgr.h"
#include "AI.h"
#include "AIDB.h"
#include "AIBlackBoard.h"
#include "AIUtils.h"

DEFINE_AI_FACTORY_CLASS(CAISoundRecord);


// Globals / Statics

CAISoundMgr* g_pAISoundMgr = NULL;

//
// CAISoundRecord functions.
//

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISoundRecord::Con/destructor
//
//	PURPOSE:	Factory Con/destructor
//
// ----------------------------------------------------------------------- //

CAISoundRecord::CAISoundRecord()
{
	m_hAI = NULL;
	m_hSoundTarget = NULL;
	m_eSoundType = kAIS_InvalidType;
	m_eSoundCategory = kAISndCat_Always;
	m_fSoundRequestTime = 0.f;	
	m_fSoundDelayTime = 0.f;	
	m_fSoundCompletionTime = 0.f;	
	m_bSoundDeleted = false;
}

CAISoundRecord::~CAISoundRecord()
{
}


//
// CAISoundMgr functions.
//

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISoundMgr::CAISoundMgr()
//
//	PURPOSE:	Con/destructor
//
// ----------------------------------------------------------------------- //

CAISoundMgr::CAISoundMgr()
{
	ASSERT(g_pAISoundMgr == NULL);
	g_pAISoundMgr = this;

	InitAISoundMgr();
}

CAISoundMgr::~CAISoundMgr()
{
	ASSERT(g_pAISoundMgr != NULL);

	TermAISoundMgr();

	g_pAISoundMgr = NULL;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISoundMgr::InitAISoundMgr()
//
//	PURPOSE:	Init object
//
// ----------------------------------------------------------------------- //

void CAISoundMgr::InitAISoundMgr()
{
	ASSERT(g_pAISoundMgr != NULL);

	m_LastPlayedSoundRecord.m_fSoundRequestTime = -DBL_MAX;

	for( uint32 iEvent=0; iEvent < kAIS_Count; ++iEvent )
	{
		m_afLastEventTime[iEvent] = -DBL_MAX;
	}

	m_fLastLimitedWarnAllyTime = -DBL_MAX;
	m_fLastCheckInTime = -DBL_MAX;
	m_fLastDisturbanceTime = -DBL_MAX;
	m_fLastManDownTime = -DBL_MAX;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISoundMgr::TermAISoundMgr()
//
//	PURPOSE:	Destroy object
//
// ----------------------------------------------------------------------- //

void CAISoundMgr::TermAISoundMgr()
{
	ASSERT(g_pAISoundMgr != NULL);

	CAISoundRecord* pSoundRecord;

	// Delete records from lists.

	AISOUND_LIST::iterator itSound;
	for( itSound = m_lstRequestedSounds.begin(); itSound != m_lstRequestedSounds.end(); ++itSound )
	{
		pSoundRecord = *itSound;
		if( pSoundRecord )
		{
			AI_FACTORY_DELETE( pSoundRecord );
		}
	}
	m_lstRequestedSounds.resize( 0 );

	for( itSound = m_lstSequencedSounds.begin(); itSound != m_lstSequencedSounds.end(); ++itSound )
	{
		pSoundRecord = *itSound;
		if( pSoundRecord )
		{
			AI_FACTORY_DELETE( pSoundRecord );
		}
	}
	m_lstSequencedSounds.resize( 0 );

	for( itSound = m_lstActiveSounds.begin(); itSound != m_lstActiveSounds.end(); ++itSound )
	{
		pSoundRecord = *itSound;
		if( pSoundRecord )
		{
			AI_FACTORY_DELETE( pSoundRecord );
		}
	}
	m_lstActiveSounds.resize( 0 );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISoundMgr::RequestAISound()
//
//	PURPOSE:	Request that an AI plays a sound.
//
// ----------------------------------------------------------------------- //

void CAISoundMgr::RequestAISound( HOBJECT hAI, EnumAISoundType eSoundType, EnumAISoundCategory eSoundCategory, HOBJECT hTarget, float fDelay )
{
	// Ignore invalid sounds as early as is possible.

	if ( kAIS_InvalidType == eSoundType )
	{
		return;
	}

	// Insure the sound is not a duplicate (AI emitting more than 1 damage
	// sound a frame for instance when hit by shotgun)
	
	for (AISOUND_LIST::iterator it = m_lstRequestedSounds.begin(); it != m_lstRequestedSounds.end(); ++it)
	{
		CAISoundRecord* pRecord = *it;

		if (pRecord 
			&& pRecord->m_fSoundRequestTime == g_pLTServer->GetTime()
			&& pRecord->m_eSoundType == eSoundType
			&& pRecord->m_hAI == hAI)
		{
			return;
		}
	}

	AITRACE( AIShowSounds, ( hAI, "Requesting AISound: %s", s_aszAISoundTypes[eSoundType] ) );

	// Create a new record for the request.

	CAISoundRecord* pSoundRecord = AI_FACTORY_NEW( CAISoundRecord );
	if( !pSoundRecord )
	{
		return;
	}

	pSoundRecord->m_hAI = hAI;
	pSoundRecord->m_eSoundType = eSoundType;
	pSoundRecord->m_eSoundCategory = eSoundCategory;
	pSoundRecord->m_hSoundTarget = hTarget;
	pSoundRecord->m_fSoundRequestTime = g_pLTServer->GetTime();
	pSoundRecord->m_fSoundCompletionTime = 0.f;

	// Sounds may be delayed.

	if( fDelay > 0.f )
	{
		pSoundRecord->m_fSoundDelayTime = g_pLTServer->GetTime() + fDelay;
	}
	else {
		pSoundRecord->m_fSoundDelayTime = 0.f;
	}

	// No sequence requirements.

	pSoundRecord->m_hSoundSequenceAIPrior = NULL;

	// Add record for requested sound to the list of requests.

	m_lstRequestedSounds.push_back( pSoundRecord );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISoundMgr::ClearPendingAISounds()
//
//	PURPOSE:	Clear pending sound requests for some AI.
//
// ----------------------------------------------------------------------- //

void CAISoundMgr::ClearPendingAISounds( HOBJECT hAI )
{
	CAISoundRecord* pSoundRecord;
	AISOUND_LIST::iterator itSound;
	for( itSound = m_lstActiveSounds.begin(); itSound != m_lstActiveSounds.end(); ++itSound )
	{
		pSoundRecord = *itSound;
		if( pSoundRecord && pSoundRecord->m_hAI == hAI )
		{
			pSoundRecord->m_bSoundDeleted = true;
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISoundMgr::RequestAISoundSequence()
//
//	PURPOSE:	Request that an AI plays a sound.
//
// ----------------------------------------------------------------------- //

void CAISoundMgr::RequestAISoundSequence( HOBJECT hAI, EnumAISoundType eSoundType, HOBJECT hAIPrior, EnumAISoundType eSoundTypePrior, EnumAISoundType eSoundTypeFirst, EnumAISoundCategory eSoundCategory, HOBJECT hTarget, float fDelay )
{
	AITRACE( AIShowSounds, ( hAI, "Requesting sequence AISound: %s", s_aszAISoundTypes[eSoundType] ) );

	// Create a new record for the request.

	CAISoundRecord* pSoundRecord = AI_FACTORY_NEW( CAISoundRecord );
	if( !pSoundRecord )
	{
		return;
	}

	pSoundRecord->m_hAI = hAI;
	pSoundRecord->m_eSoundType = eSoundType;
	pSoundRecord->m_eSoundCategory = eSoundCategory;
	pSoundRecord->m_hSoundTarget = hTarget;
	pSoundRecord->m_fSoundRequestTime = g_pLTServer->GetTime();
	pSoundRecord->m_fSoundCompletionTime = 0.f;

	// Sounds may be delayed.

	if( fDelay > 0.f )
	{
		pSoundRecord->m_fSoundDelayTime = fDelay;
	}
	else {
		pSoundRecord->m_fSoundDelayTime = 0.f;
	}

	// Sequence requirements.

	pSoundRecord->m_hSoundSequenceAIPrior = hAIPrior;
	pSoundRecord->m_eSoundSequenceTypePrior = eSoundTypePrior;
	pSoundRecord->m_eSoundSequenceTypeFirst = eSoundTypeFirst;

	// Find a NULL sequenced sound slot.

	CAISoundRecord* pSequencedSoundRecord;
	AISOUND_LIST::iterator itSound;
	for( itSound = m_lstSequencedSounds.begin(); itSound != m_lstSequencedSounds.end(); ++itSound )
	{
		pSequencedSoundRecord = *itSound;
		if( !pSequencedSoundRecord )
		{
			*itSound = pSoundRecord;
			return;
		}
	}

	// Add record for requested sound to the list of requests.

	m_lstSequencedSounds.push_back( pSoundRecord );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISoundMgr::UpdateAISoundMgr()
//
//	PURPOSE:	Play requested sounds.
//
// ----------------------------------------------------------------------- //

void CAISoundMgr::UpdateAISoundMgr()
{
	// Iterate over requests.

	CAI* pAI;
	CAISoundRecord* pSoundRecord;
	AISOUND_LIST::iterator itSound;
	for( itSound = m_lstRequestedSounds.begin(); itSound != m_lstRequestedSounds.end(); ++itSound )
	{
		pSoundRecord = *itSound;
		if( !pSoundRecord )
		{
			continue;
		}

		// Clear any waiting sequences.

		ClearAISoundSequences( pSoundRecord );

		// Play sound if possible.

		pAI = (CAI*)g_pLTServer->HandleToObject( pSoundRecord->m_hAI );
		if( pAI && CanPlayAISound( pAI, pSoundRecord ) )
		{
			// Actually play the sound.

			if( pSoundRecord->m_fSoundDelayTime == 0.f )
			{
				PlayActiveAISound( pSoundRecord );
			}

			// Active sound records are retained for later comparisons.

			RecordActiveAISound( pSoundRecord );
		}
		
		// Delete the unplayed record.

		else {
			AI_FACTORY_DELETE( pSoundRecord );
		}
	}

	// Clear the request list.

	m_lstRequestedSounds.resize( 0 );

	// Play delayed sounds, and clear expired sounds.

	UpdateActiveAISounds();

	// Play next sound in sequence.

	UpdateSequencedAISounds();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISoundMgr::PlayActiveAISound()
//
//	PURPOSE:	Play an active sound.
//
// ----------------------------------------------------------------------- //

void CAISoundMgr::PlayActiveAISound( CAISoundRecord* pSoundRecord )
{
	if( !pSoundRecord )
	{
		return;
	}

	CAI* pAI = (CAI*)g_pLTServer->HandleToObject( pSoundRecord->m_hAI );
	if( !pAI )
	{
		return;
	}

	char* pSound = GetSound( pAI, pSoundRecord->m_eSoundType );
	CharacterSoundType eCharacterSoundType = GetCharacterSoundType( pSoundRecord->m_eSoundType );

	// If this sound is an AISOUND (meaning general AI, with no special case
	// handling) and if the category is MODELKEY, remap the character sound
	// type so that this sound does not count against the total number of
	// AISounds played at once. This allows sounds to be filtered into the
	// correct category (handling violence settings) if they do are explicitly
	// handled by GetCharacterSoundType.

	if ( CST_AI_SOUND == eCharacterSoundType 
		&& kAISndCat_ModelKey == pSoundRecord->m_eSoundCategory )
	{
		eCharacterSoundType = CST_AI_SOUND_MODELKEY;
	}
	
	pAI->PlayDialogSound( pSound, eCharacterSoundType );
	pSoundRecord->m_fSoundDelayTime = 0.f;

	AITRACE( AIShowSounds, ( pAI->m_hObject, "Playing AISound: %s '%s.wav'", s_aszAISoundTypes[pSoundRecord->m_eSoundType], pSound ) );

	// Record time last sound played.

	EnumAISoundCategory eSoundCategory = pSoundRecord->m_eSoundCategory;
	if( eSoundCategory != kAISndCat_Interrupt )
	{
		m_LastPlayedSoundRecord = *pSoundRecord;

		if( ( eSoundCategory == kAISndCat_Event ) ||
			( eSoundCategory == kAISndCat_Location ) ||
			( eSoundCategory == kAISndCat_InterruptMelee ) )
		{
			m_afLastEventTime[m_LastPlayedSoundRecord.m_eSoundType] = g_pLTServer->GetTime();
		}

		else if( eSoundCategory == kAISndCat_LimitedWarnAlly )
		{
			m_fLastLimitedWarnAllyTime = g_pLTServer->GetTime();
		}

		else if( eSoundCategory == kAISndCat_CheckIn )
		{
			m_fLastCheckInTime = g_pLTServer->GetTime();
		}

		else if( eSoundCategory == kAISndCat_ManDown )
		{
			m_fLastManDownTime = g_pLTServer->GetTime();
		}

		else if( ( eSoundCategory == kAISndCat_DisturbanceHeard ) ||
				 ( eSoundCategory == kAISndCat_DisturbanceSeen ) )
		{
			m_fLastDisturbanceTime = g_pLTServer->GetTime();
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISoundMgr::UpdateActiveAISounds()
//
//	PURPOSE:	Play delayed sounds, and clear expired sounds.
//
// ----------------------------------------------------------------------- //

void CAISoundMgr::UpdateActiveAISounds()
{
	CAI* pAI;
	CAISoundRecord* pSoundRecord;
	AISOUND_LIST::iterator itSound;
	for( itSound = m_lstActiveSounds.begin(); itSound != m_lstActiveSounds.end(); ++itSound )
	{
		pSoundRecord = *itSound;
		if( !pSoundRecord )
		{
			continue;
		}
	
		// Delete active sounds associated with a dead AI.

		if( IsDeadAI( pSoundRecord->m_hAI ) )
		{
			AI_FACTORY_DELETE( pSoundRecord );
			*itSound = NULL;
			continue;
		}

		// Play delayed sound.

		if( pSoundRecord->m_fSoundDelayTime > 0.f )
		{
			if( pSoundRecord->m_fSoundDelayTime <= g_pLTServer->GetTime() )
			{
				pAI = (CAI*)g_pLTServer->HandleToObject( pSoundRecord->m_hAI );
				if( pAI && CanPlayAISound( pAI, pSoundRecord ) )
				{
					PlayActiveAISound( pSoundRecord );
				}
				else {
					AI_FACTORY_DELETE( pSoundRecord );
					*itSound = NULL;
				}
			}
			continue;
		}

		// Record completion times.

		if( pSoundRecord->m_fSoundCompletionTime == 0.f )
		{
			pAI = (CAI*)g_pLTServer->HandleToObject( pSoundRecord->m_hAI );
			if( pAI && !pAI->IsPlayingDialogSound() )
			{
				pSoundRecord->m_fSoundCompletionTime = g_pLTServer->GetTime();
				continue;
			}
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISoundMgr::UpdateSequencedAISounds()
//
//	PURPOSE:	Play delayed sounds, and clear expired sounds.
//
// ----------------------------------------------------------------------- //

void CAISoundMgr::UpdateSequencedAISounds()
{
	CAISoundRecord* pSoundRecord;
	AISOUND_LIST::iterator itSound;
	for( itSound = m_lstSequencedSounds.begin(); itSound != m_lstSequencedSounds.end(); ++itSound )
	{
		pSoundRecord = *itSound;
		if( !pSoundRecord )
		{
			continue;
		}

		// Delete sequenced sounds associated with a dead AI.

		if( IsDeadAI( pSoundRecord->m_hAI ) ||
			IsDeadAI( pSoundRecord->m_hSoundSequenceAIPrior ) )
		{
			AI_FACTORY_DELETE( pSoundRecord );
			*itSound = NULL;
			continue;
		}

		CAISoundRecord* pPriorSoundRecord = FindActiveSoundForTime( pSoundRecord->m_fSoundRequestTime );
		if( !pPriorSoundRecord )
		{
			AI_FACTORY_DELETE( pSoundRecord );
			*itSound = NULL;
			continue;
		}

		// Keep waiting if AI is saying the wrong type of sound.

		if( pPriorSoundRecord->m_eSoundType != pSoundRecord->m_eSoundSequenceTypePrior )
		{
			continue;
		}

		// Keep waiting if AI is still speaking.

		if( pPriorSoundRecord->m_fSoundCompletionTime == 0.f )
		{
			continue;
		}

		// Play the next sound in the sequence!
		// Delete the record for the sequenced sound.

		if( pSoundRecord->m_fSoundDelayTime == 0.f )
		{
			PlayActiveAISound( pSoundRecord );

			*pPriorSoundRecord = *pSoundRecord;
			AI_FACTORY_DELETE( pSoundRecord );
			*itSound = NULL;
		}
		else {
			*pPriorSoundRecord = *pSoundRecord;
			pPriorSoundRecord->m_fSoundDelayTime += g_pLTServer->GetTime();
			AI_FACTORY_DELETE( pSoundRecord );
			*itSound = NULL;
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISoundMgr::ClearAISoundSequences()
//
//	PURPOSE:	Play delayed sounds, and clear expired sounds.
//
// ----------------------------------------------------------------------- //

void CAISoundMgr::ClearAISoundSequences( CAISoundRecord* pRequestSoundRecord )
{
	CAISoundRecord* pSoundRecord;
	AISOUND_LIST::iterator itSound;
	for( itSound = m_lstSequencedSounds.begin(); itSound != m_lstSequencedSounds.end(); ++itSound )
	{
		pSoundRecord = *itSound;
		if( !pSoundRecord )
		{
			continue;
		}

		// Don't clear sequences that should always play.

		if( pSoundRecord->m_eSoundCategory == kAISndCat_Always)
		{
			continue;
		}

		// Clear pending sounds that require a visible target.

		if( pSoundRecord->m_eSoundCategory == kAISndCat_TargetVisible )
		{
			CAI* pAI = (CAI*)g_pLTServer->HandleToObject( pSoundRecord->m_hAI );
			if( pAI && pAI->GetAIBlackBoard()->GetBBTargetVisibleFromWeapon() )
			{
				continue;
			}
		}

		if( pSoundRecord->m_eSoundType == pRequestSoundRecord->m_eSoundType )
		{
			continue;
		}

		if( pSoundRecord->m_eSoundSequenceTypePrior == pRequestSoundRecord->m_eSoundType )
		{
			continue;
		}

		if( pSoundRecord->m_eSoundSequenceTypeFirst == pRequestSoundRecord->m_eSoundType )
		{
			continue;
		}

		AI_FACTORY_DELETE( pSoundRecord );
		*itSound = NULL;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISoundMgr::SkipAISound()
//
//	PURPOSE:	Play a blank, to ensure this sound does not play again for some time.
//
// ----------------------------------------------------------------------- //

void CAISoundMgr::SkipAISound( HOBJECT hAI, EnumAISoundType eSoundType )
{
	AIASSERT( eSoundType != kAIS_InvalidType, hAI, "CAISoundMgr::SkipAISound: Invalid sound type." );

	double fCurTime = g_pLTServer->GetTime();
	float fFreq = g_pAIDB->GetAIConstantsRecord()->fAISoundFrequencyEvent;
	if( fCurTime - m_afLastEventTime[eSoundType] > fFreq )
	{
		m_afLastEventTime[eSoundType] = fCurTime;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISoundMgr::CanPlayAISound()
//
//	PURPOSE:	Return true if sound is allowed to play.
//
// ----------------------------------------------------------------------- //

bool CAISoundMgr::CanPlayAISound( CAI* pAI, CAISoundRecord* pSoundRecord )
{
	if( !( pAI && pSoundRecord ) )
	{
		return false;
	}

	// Sound has been pre-emptively deleted.

	if( pSoundRecord->m_bSoundDeleted )
	{
		return false;
	}

	// Incapacitated AI cannot speak.

	if( pAI->GetAIBlackBoard()->GetBBIncapacitated() )
	{
		return false;
	}

	EnumAISoundCategory eSoundCategory = pSoundRecord->m_eSoundCategory;

	// Always play interrupt sounds. (e.g. Pain)

	if( eSoundCategory == kAISndCat_Interrupt )
	{
		return true;
	}

	float fFreq;
	if( eSoundCategory == kAISndCat_InterruptMelee )
	{
  		double fCurTime = g_pLTServer->GetTime();
		fFreq = g_pAIDB->GetAIConstantsRecord()->fAISoundFrequencyMelee;
		if( (float)(fCurTime - m_afLastEventTime[pSoundRecord->m_eSoundType]) < fFreq )
		{
			return false;
		}
		else return true;
	}

	// [KLS 7/2/02] - If too many AI sounds are playing, don't play the sound.
	// Note: we do this here so that GetSound() isn't called (since GetSound()
	// keeps track of which sound was played last)...

//	if( CCharacter::GetAISoundCount() >= 2 )
	if( CCharacter::GetAISoundCount() >= 1 
		&& kAISndCat_ModelKey != eSoundCategory )
	{
		return false;
	}

	// Do not interrupt currently playing AISound.

	if( pAI->IsPlayingDialogSound() )
	{
		return false;
	}

	// Limit the frequency of some categories of sounds.

	double fCurTime = g_pLTServer->GetTime();
	HOBJECT hTarget = pSoundRecord->m_hSoundTarget;

	// Always allow the next sound in the sequence to play.

	if( ( pSoundRecord->m_hSoundSequenceAIPrior == m_LastPlayedSoundRecord.m_hAI ) &&
		( pSoundRecord->m_eSoundSequenceTypePrior == m_LastPlayedSoundRecord.m_eSoundType ) &&
		( pSoundRecord->m_fSoundRequestTime == m_LastPlayedSoundRecord.m_fSoundRequestTime ) )
	{
		return true;
	}

	switch( eSoundCategory )
	{
		case kAISndCat_ModelKey:
		case kAISndCat_Always:
			return true;

		case kAISndCat_TargetVisible:
			{
				if( !pAI->GetAIBlackBoard()->GetBBTargetVisibleFromWeapon() )
				{
					return false;
				}
			}
			break;

		case kAISndCat_Combat:
			fFreq = g_pAIDB->GetAIConstantsRecord()->fAISoundFrequencyChatter;
			if( (float)(fCurTime - m_LastPlayedSoundRecord.m_fSoundRequestTime) < fFreq )
			{
				return false;
			}
			break;

		case kAISndCat_Event:
			fFreq = g_pAIDB->GetAIConstantsRecord()->fAISoundFrequencyEvent;
			if( (float)(fCurTime - m_afLastEventTime[pSoundRecord->m_eSoundType]) < fFreq )
			{
				return false;
			}
			break;

		case kAISndCat_Location:
			fFreq = g_pAIDB->GetAIConstantsRecord()->fAISoundFrequencyLocation;
			if( (float)(fCurTime - m_afLastEventTime[pSoundRecord->m_eSoundType]) < fFreq )
			{
				return false;
			}
			break;
	
		case kAISndCat_LimitedWarnAlly:
			fFreq = g_pAIDB->GetAIConstantsRecord()->fAISoundFrequencyEvent;
			if( (float)(fCurTime - m_fLastLimitedWarnAllyTime) < fFreq )
			{
				return false;
			}
			break;

		case kAISndCat_DisturbanceHeard:
		case kAISndCat_DisturbanceSeen:
			fFreq = g_pAIDB->GetAIConstantsRecord()->fAISoundFrequencyChatter;
			if( (float)(fCurTime - m_fLastDisturbanceTime) < fFreq )
			{
				return false;
			}
			break;

		case kAISndCat_CheckIn:
			fFreq = g_pAIDB->GetAIConstantsRecord()->fAISoundFrequencyChatter;
			if( (float)(fCurTime - m_fLastCheckInTime) < fFreq )
			{
				return false;
			}
			break;

		case kAISndCat_ManDown:
			fFreq = g_pAIDB->GetAIConstantsRecord()->fAISoundFrequencyEvent;
			if( (float)(fCurTime - m_fLastManDownTime) < fFreq )
			{
				return false;
			}
			break;

		default:
			AIASSERT( 0, NULL, "Unrecognized sound category." );
			break;
	}


	// Play the sound.

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISoundMgr::RecordActiveAISound()
//
//	PURPOSE:	Keep track of active sounds.
//
// ----------------------------------------------------------------------- //

void CAISoundMgr::RecordActiveAISound( CAISoundRecord* pSoundRecord )
{
	// Find an existing active sound for the same AI.

	CAISoundRecord* pActiveSoundRecord;
	AISOUND_LIST::iterator itSound;
	for( itSound = m_lstActiveSounds.begin(); itSound != m_lstActiveSounds.end(); ++itSound )
	{
		pActiveSoundRecord = *itSound;
		if( !pActiveSoundRecord )
		{
			continue;
		}

		if( pActiveSoundRecord->m_hAI == pSoundRecord->m_hAI )
		{
			*pActiveSoundRecord = *pSoundRecord;
			AI_FACTORY_DELETE( pSoundRecord );
			return;
		}
	}

	// Find a NULL active sound slot.

	for( itSound = m_lstActiveSounds.begin(); itSound != m_lstActiveSounds.end(); ++itSound )
	{
		pActiveSoundRecord = *itSound;
		if( !pActiveSoundRecord )
		{
			*itSound = pSoundRecord;
			return;
		}
	}

	// Add a new active sound.

	m_lstActiveSounds.push_back( pSoundRecord );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISoundMgr::FindActiveSound()
//
//	PURPOSE:	Return an active sound with the specified sound type and target.
//
// ----------------------------------------------------------------------- //

CAISoundRecord* CAISoundMgr::FindActiveSound( EnumAISoundCategory eSoundCategory, HOBJECT hTarget )
{
	CAISoundRecord* pSoundRecord;
	AISOUND_LIST::iterator itSound;
	for( itSound = m_lstActiveSounds.begin(); itSound != m_lstActiveSounds.end(); ++itSound )
	{
		pSoundRecord = *itSound;
		if( !pSoundRecord )
		{
			continue;
		}

		if( ( pSoundRecord->m_eSoundCategory == eSoundCategory ) &&
			( pSoundRecord->m_hSoundTarget == hTarget ) )
		{
			return pSoundRecord;
		}
	}

	return NULL;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISoundMgr::FindLastActiveSoundForTarget()
//
//	PURPOSE:	Return the most recent active sound with the specified target.
//
// ----------------------------------------------------------------------- //

CAISoundRecord* CAISoundMgr::FindLastActiveSoundForTarget( HOBJECT hTarget )
{
	if( !hTarget )
	{
////		return NULL;
	}

	double fTime = 0.f;

	CAISoundRecord* pLastSoundRecord = NULL;
	CAISoundRecord* pSoundRecord;
	AISOUND_LIST::iterator itSound;
	for( itSound = m_lstActiveSounds.begin(); itSound != m_lstActiveSounds.end(); ++itSound )
	{
		pSoundRecord = *itSound;
		if( !pSoundRecord )
		{
			continue;
		}

		if( pSoundRecord->m_hSoundTarget != hTarget )
		{
////			continue;
		}

		// Ignore delayed sounds.

		if( pSoundRecord->m_fSoundDelayTime > 0.f )
		{
			continue;
		}

		if( fTime < pSoundRecord->m_fSoundCompletionTime )
		{
			fTime = pSoundRecord->m_fSoundCompletionTime;
			pLastSoundRecord = pSoundRecord;
		}

		else if( fTime < pSoundRecord->m_fSoundRequestTime )
		{
			fTime = pSoundRecord->m_fSoundRequestTime;
			pLastSoundRecord = pSoundRecord;
		}
	}

	return pLastSoundRecord;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISoundMgr::FindActiveSoundForSpeaker()
//
//	PURPOSE:	Return an active sound with the specified speaker.
//
// ----------------------------------------------------------------------- //

CAISoundRecord* CAISoundMgr::FindActiveSoundForSpeaker( HOBJECT hSpeaker )
{
	CAISoundRecord* pSoundRecord;
	AISOUND_LIST::iterator itSound;
	for( itSound = m_lstActiveSounds.begin(); itSound != m_lstActiveSounds.end(); ++itSound )
	{
		pSoundRecord = *itSound;
		if( !pSoundRecord )
		{
			continue;
		}

		if( pSoundRecord->m_hAI == hSpeaker )
		{
			return pSoundRecord;
		}
	}

	return NULL;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISoundMgr::FindActiveSoundForTime()
//
//	PURPOSE:	Return an active sound with the specified request time.
//
// ----------------------------------------------------------------------- //

CAISoundRecord* CAISoundMgr::FindActiveSoundForTime( double fTime )
{
	CAISoundRecord* pSoundRecord;
	AISOUND_LIST::iterator itSound;
	for( itSound = m_lstActiveSounds.begin(); itSound != m_lstActiveSounds.end(); ++itSound )
	{
		pSoundRecord = *itSound;
		if( !pSoundRecord )
		{
			continue;
		}

		if( pSoundRecord->m_fSoundRequestTime == fTime )
		{
			return pSoundRecord;
		}
	}

	return NULL;
}


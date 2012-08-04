// ----------------------------------------------------------------------- //
//
// MODULE  : AISoundMgr.h
//
// PURPOSE : AISoundMgr class definition
//
// CREATED : 6/24/03
//
// (c) 2003 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef __AISOUND_MGR_H__
#define __AISOUND_MGR_H__

#include "AIClassFactory.h"
#include "AISounds.h"


// Forward declarations.

class	CAISoundMgr;
class	CAI;

// Sound categories.

enum EnumAISoundCategory
{
	kAISndCat_Always,
	kAISndCat_CheckIn,
	kAISndCat_Combat,
	kAISndCat_DisturbanceHeard,
	kAISndCat_DisturbanceSeen,
	kAISndCat_Event,
	kAISndCat_Interrupt,
	kAISndCat_InterruptMelee,
	kAISndCat_LimitedWarnAlly,
	kAISndCat_Location,
	kAISndCat_ManDown,
	kAISndCat_ModelKey,
	kAISndCat_TargetVisible,
};


extern CAISoundMgr *g_pAISoundMgr;

//
// CLASS: Record of a sound.
//
class CAISoundRecord : public CAIClassAbstract
{
	public :
		DECLARE_AI_FACTORY_CLASS(CAISoundRecord);

		CAISoundRecord();
		~CAISoundRecord();

	public:

		LTObjRef				m_hAI;
		EnumAISoundType			m_eSoundType;
		EnumAISoundCategory		m_eSoundCategory;
		LTObjRef				m_hSoundTarget;
		double					m_fSoundRequestTime;
		double					m_fSoundDelayTime;
		double					m_fSoundCompletionTime;

		LTObjRef				m_hSoundSequenceAIPrior;
		EnumAISoundType			m_eSoundSequenceTypePrior;
		EnumAISoundType			m_eSoundSequenceTypeFirst;

		bool					m_bSoundDeleted;
};


//
// VECTOR: List of sounds.
//
typedef std::vector<CAISoundRecord*, LTAllocator<CAISoundRecord*, LT_MEM_TYPE_OBJECTSHELL> > AISOUND_LIST;


//
// CLASS: Global manager of active and requested sounds.
//
class CAISoundMgr
{
	public : // Public methods

		 CAISoundMgr();
		~CAISoundMgr();

		void InitAISoundMgr();
		void TermAISoundMgr();

		// Requests.

		void	RequestAISound( HOBJECT hAI, EnumAISoundType eSoundType, EnumAISoundCategory eSoundCategory, HOBJECT hTarget, float fDelay );
		void	ClearPendingAISounds( HOBJECT hAI );

		// Sequences.

		void	RequestAISoundSequence( HOBJECT hAI, EnumAISoundType eSoundType, HOBJECT hAIPrior, EnumAISoundType eSoundTypePrior, EnumAISoundType eSoundTypeFirst, EnumAISoundCategory eSoundCategory, HOBJECT hTarget, float fDelay );
		void	ClearAISoundSequences( CAISoundRecord* pRequestSoundRecord );

		// Skip AI sound.

		void	SkipAISound( HOBJECT hAI, EnumAISoundType eSoundType );

		// Update.

		void	UpdateAISoundMgr();

	protected:

		void			UpdateActiveAISounds();
		void			UpdateSequencedAISounds();

		bool			CanPlayAISound( CAI* pAI, CAISoundRecord* pSoundRecord );
		void			RecordActiveAISound( CAISoundRecord* pSoundRecord );

		void			PlayActiveAISound( CAISoundRecord* pSoundRecord );

		CAISoundRecord* FindActiveSound( EnumAISoundCategory eSoundCategory, HOBJECT hTarget );
		CAISoundRecord* FindLastActiveSoundForTarget( HOBJECT hTarget );
		CAISoundRecord* FindActiveSoundForSpeaker( HOBJECT hSpeaker );
		CAISoundRecord* FindActiveSoundForTime( double fTime );

	protected:

		AISOUND_LIST		m_lstRequestedSounds;
		AISOUND_LIST		m_lstSequencedSounds;
		AISOUND_LIST		m_lstActiveSounds;

		CAISoundRecord		m_LastPlayedSoundRecord;
		double				m_afLastEventTime[kAIS_Count];
		double				m_fLastLimitedWarnAllyTime;
		double				m_fLastCheckInTime;
		double				m_fLastDisturbanceTime;
		double				m_fLastManDownTime;
};

#endif

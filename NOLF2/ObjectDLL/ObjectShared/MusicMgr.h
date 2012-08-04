// (c) 1997-2000 Monolith Productions, Inc.  All Rights Reserved

#ifndef MUSICMGR_H
#define MUSICMGR_H

#include "ButeMgr.h"
extern class CMusicMgr* g_pMusicMgr;
class CMusicButeMgr;

extern const char* s_aszMusicMoods[];

class CMusicMgr
{
	public :

		enum Event
		{
			eEventInvalid		= -1,
			eEventPlayerDie		= 0, 
			eEventAIDie			= 1,
			eEventAIDodge		= 2,

			kNumEvents			= 3,
			kMaxMotifsPerEvent	= 10,
		};

		enum Mood
		{
			eMoodInvalid		= -1,
			eMoodNone			= 0,
			eMoodRoutine		= 1,
			eMoodInvestigate	= 2,
			eMoodAggressive		= 3,

			kNumMoods			= 4,
			kMaxLevelsPerMood	= 10,
		};

	public :

		// Ctors/Dtors/etc

		CMusicMgr();
		virtual ~CMusicMgr();

		void Init(const char* szMusicControlFile);
		void Term();

		void Save(ILTMessage_Write *pMsg);
		void Load(ILTMessage_Read *pMsg);

		void Invalidate() { /*m_eLastMood = eMoodInvalid;*/ }

		// Update

		void Update();

		// Lock

		void LockMood() { m_bLockedMood = LTTRUE; }
		void UnlockMood() { m_bLockedMood = LTFALSE; Invalidate(); }
		LTBOOL IsMoodLocked() { return m_bLockedMood; }

		void LockEvent() { m_bLockedEvent = LTTRUE; }
		void UnlockEvent() { m_bLockedEvent = LTFALSE; }

		// Moods

		void DoMood(Mood eMood);

		// Sets to specific mood and sends message immediately to client.
		bool SetMood( Mood eMood );

		// Events

		void DoEvent(Event eEvent);

		// Intensity

		void SetRestoreMusicIntensity(uint32 iRestoreMusicIntensity) { m_iRestoreMusicIntensity = iRestoreMusicIntensity; }
		void RestoreMusicIntensity() { m_bRestoreMusicIntensity = LTTRUE; }

		// Enable/Disable

		void Enable() { m_bEnabled = LTTRUE; }
		void Disable() { m_bEnabled = LTFALSE; }
		
	protected :

		CMusicButeMgr*	m_pMusicButeMgr;

		LTBOOL		m_bInitialized;

		char		m_szTheme[128];

		LTBOOL		m_bLockedMood;
		LTBOOL		m_bLockedEvent;

		uint32		m_iRestoreMusicIntensity;
		LTBOOL		m_bRestoreMusicIntensity;

		LTFLOAT		m_afMoods[kNumMoods];

		Mood		m_eLastMood;

		uint32		m_acMoods[kNumMoods];
		uint32		m_aanMoods[kNumMoods][kMaxLevelsPerMood];

		uint32		m_acEvents[kNumEvents];
		char		m_aaszEvents[kNumEvents][kMaxMotifsPerEvent][128];

		LTFLOAT		m_afEventChances[kNumEvents];

		LTBOOL		m_bEnabled;
};

#endif // MUSICMGR_H
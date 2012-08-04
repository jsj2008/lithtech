// (c) 1997-2000 Monolith Productions, Inc.  All Rights Reserved

#include "StdAfx.h"
#include "MusicMgr.h"
#include "GameButeMgr.h"
#include "MsgIds.h"

// ----------------------------------------------------------------------- //

class CMusicButeMgr : public CGameButeMgr
{
	public : // Public member variables

        LTBOOL	Init(ILTCSBase *pInterface, const char* szAttributeFile = "Attributes\\Music.txt")
		{
			return Parse(pInterface, szAttributeFile);
		}

		CButeMgr* GetButeMgr() { return &m_buteMgr; }
};

static CMusicButeMgr s_MusicButeMgr;
static CButeMgr* g_pMusicButeMgr = s_MusicButeMgr.GetButeMgr();

// ----------------------------------------------------------------------- //

CMusicMgr* g_pMusicMgr = LTNULL;
static CMusicMgr s_MusicMgr;

// ----------------------------------------------------------------------- //

static const char* s_aszMoods[] = { "None", "Routine", "Investigate", "Aggressive" };
static const char* s_aszEvents[] = { "PlayerDie", "AIDie", "AIDodge" };

static const LTFLOAT s_fMax = 10.0f;

// ----------------------------------------------------------------------- //

CMusicMgr::CMusicMgr()
{
	m_bEnabled = LTFALSE;
	g_pMusicMgr = this;

	for ( uint32 iMood = 0 ; iMood < kNumMoods ; ++iMood )
	{
		m_afMoods[iMood] = 0.0f;
	}

	for ( uint32 iEvent = 0 ; iEvent < kNumEvents ; ++iEvent )
	{
		m_afEventChances[iEvent] = 1.0f;
	}

	m_iRestoreMusicIntensity = -1;

	m_eLastMood = eMoodInvalid;

	m_bLockedMood = LTFALSE;
	m_bLockedEvent = LTFALSE;

	m_bInitialized = LTFALSE;
}

// ----------------------------------------------------------------------- //

CMusicMgr::~CMusicMgr()
{
	Term();

	g_pMusicMgr = LTNULL;
}

// ----------------------------------------------------------------------- //

void CMusicMgr::Init(const char* szMusicControlFile)
{
	if ( m_bInitialized  ) return;

	if ( !s_MusicButeMgr.Init(g_pLTServer) )
	{
		return;
	}

	strcpy(m_szTheme, szMusicControlFile);
	char* pchDot;
	if ( pchDot = strchr(m_szTheme, '.') )
	{
		*pchDot = 0;
	}

    _strlwr(m_szTheme);

	for ( uint32 iMood = 0 ; iMood < kNumMoods ; ++iMood )
	{
		m_acMoods[iMood] = 0;

		for ( uint32 iLevel = 0 ; iLevel < kMaxLevelsPerMood ; iLevel++ )
		{
			char szMood[128];
			sprintf(szMood, "%s%d", s_aszMoods[iMood], iLevel);
			if ( g_pMusicButeMgr->Exist(m_szTheme, szMood) )
			{
				m_aanMoods[iMood][m_acMoods[iMood]] = g_pMusicButeMgr->GetInt(m_szTheme, szMood);
				m_acMoods[iMood]++;
			}
			else
			{
				break;
			}
		}
	}

	for ( uint32 iEvent = 0 ; iEvent < kNumEvents ; ++iEvent )
	{
		m_acEvents[iEvent] = 0;

		for ( uint32 iMotif = 0 ; iMotif < kMaxMotifsPerEvent ; iMotif++ )
		{
			char szEvent[128];
			sprintf(szEvent, "%s%d", s_aszEvents[iEvent], iMotif);
			if ( g_pMusicButeMgr->Exist(m_szTheme, szEvent) )
			{
				strcpy(m_aaszEvents[iEvent][m_acEvents[iEvent]], g_pMusicButeMgr->GetString(m_szTheme, szEvent));
				m_acEvents[iEvent]++;
			}
			else
			{
				break;
			}
		}
	}

	m_afEventChances[eEventPlayerDie]	= (LTFLOAT)g_pMusicButeMgr->GetDouble("settings", "DieChance");
	m_afEventChances[eEventAIDie]		= (LTFLOAT)g_pMusicButeMgr->GetDouble("settings", "DieChance");
	m_afEventChances[eEventAIDodge]		= (LTFLOAT)g_pMusicButeMgr->GetDouble("settings", "DodgeChance");

	m_bInitialized = LTTRUE;
}

// ----------------------------------------------------------------------- //

void CMusicMgr::Term()
{
	if ( !m_bInitialized ) return;

	m_bLockedMood = LTFALSE;
	m_bLockedEvent = LTFALSE;
	m_bRestoreMusicIntensity = LTFALSE;
	m_eLastMood = eMoodInvalid;
	m_bEnabled = LTFALSE;

	for ( uint32 iMood = 0 ; iMood < kNumMoods ; ++iMood )
	{
		m_afMoods[iMood] = 0.0f;
	}

	m_bInitialized = LTFALSE;
}

// ----------------------------------------------------------------------- //

void CMusicMgr::Save(HMESSAGEWRITE hWrite)
{
	for ( uint32 iMood = 0 ; iMood < kNumMoods ; iMood++ )
	{
		SAVE_FLOAT(m_afMoods[iMood]);
	}

	SAVE_BOOL(m_bLockedMood);
	SAVE_BOOL(m_bLockedEvent);
	SAVE_INT(m_iRestoreMusicIntensity);
}

// ----------------------------------------------------------------------- //

void CMusicMgr::Load(HMESSAGEREAD hRead)
{
	for ( uint32 iMood = 0 ; iMood < kNumMoods ; iMood++ )
	{
		LOAD_FLOAT(m_afMoods[iMood]);
	}

	LOAD_BOOL(m_bLockedMood);
	LOAD_BOOL(m_bLockedEvent);
	LOAD_INT(m_iRestoreMusicIntensity);
}

// ----------------------------------------------------------------------- //

void CMusicMgr::Update()
{
	if ( !m_bEnabled ) return;
	if ( g_pGameServerShell->GetGameType() != SINGLE ) return;

//	g_pLTServer->CPrint("MusicMgr: Events are%s locked", m_bLockedEvent ? "" : " not");
//	g_pLTServer->CPrint("MusicMgr: Moods are%s locked", m_bLockedMood ? "" : " not");

	if ( m_bLockedMood )
	{
		if ( m_bRestoreMusicIntensity )
		{
			char szMusic[128];
			sprintf(szMusic, "MUSIC I %d measure", m_iRestoreMusicIntensity);

			HSTRING hMusic = g_pLTServer->CreateString(szMusic);
			HMESSAGEWRITE hMessage = g_pLTServer->StartMessage(NULL, MID_MUSIC);
			g_pLTServer->WriteToMessageHString(hMessage, hMusic);
			g_pLTServer->EndMessage2(hMessage, MESSAGE_GUARANTEED | MESSAGE_NAGGLE);
			FREE_HSTRING(hMusic);

			m_eLastMood = eMoodInvalid;
			m_bRestoreMusicIntensity = LTFALSE;
		}

		return;
	}

	LTBOOL bChoseMood = LTFALSE;

	for ( int32 iMood = kNumMoods-1 ; iMood >= 0 ; --iMood )
	{
		if ( !bChoseMood && (m_afMoods[iMood] != 0.0f || (iMood == eMoodNone)) )
		{
			if ( m_eLastMood == iMood )
			{
				bChoseMood = LTTRUE;
			}
			else
			{
#ifndef _FINAL
				g_pLTServer->CPrint("Choosing Mood \"%s\"", s_aszMoods[iMood]);
#endif
				char szMusic[128];
				uint32 iLevel = GetRandom(0, m_acMoods[iMood]-1);
				sprintf(szMusic, "MUSIC I %d measure", m_aanMoods[iMood][iLevel]);
//				g_pLTServer->CPrint(szMusic);

				HSTRING hMusic = g_pLTServer->CreateString(szMusic);
				HMESSAGEWRITE hMessage = g_pLTServer->StartMessage(NULL, MID_MUSIC);
				g_pLTServer->WriteToMessageHString(hMessage, hMusic);
				g_pLTServer->EndMessage2(hMessage, MESSAGE_GUARANTEED | MESSAGE_NAGGLE);
				FREE_HSTRING(hMusic);
/*
				if ( m_eLastMood == eMoodInvalid )
				{
					HSTRING hMusic = g_pLTServer->CreateString("play play");
					HMESSAGEWRITE hMessage = g_pLTServer->StartMessage(NULL, MID_MUSIC);
					g_pLTServer->WriteToMessageHString(hMessage, hMusic);
					g_pLTServer->EndMessage2(hMessage, MESSAGE_GUARANTEED | MESSAGE_NAGGLE);
					FREE_HSTRING(hMusic);
				}
*/
				m_eLastMood = (Mood)iMood;
				bChoseMood = LTTRUE;
			}
		}

		m_afMoods[iMood] = Max<LTFLOAT>(m_afMoods[iMood] - g_pLTServer->GetFrameTime(), 0.0f);

//		g_pLTServer->CPrint("%s Mood = %f", s_aszMoods[iMood], m_afMoods[iMood]);
	}
}

// ----------------------------------------------------------------------- //

void CMusicMgr::DoMood(Mood eMood)
{
	m_afMoods[eMood] = Min<LTFLOAT>(m_afMoods[eMood] + g_pLTServer->GetFrameTime(), s_fMax);
}

// ----------------------------------------------------------------------- //

void CMusicMgr::DoEvent(Event eEvent)
{
	if ( !m_bEnabled ) return;
	if ( g_pGameServerShell->GetGameType() != SINGLE ) return;

	if ( m_bLockedEvent ) return;

	if ( m_acEvents[eEvent] != 0 && (m_afEventChances[eEvent] > GetRandom(0.0, 1.0f)) )
	{
		char szMusic[128];
		uint32 iEvent = GetRandom(0, m_acEvents[eEvent]-1);
		sprintf(szMusic, "MUSIC PM %s %s Beat", m_szTheme, m_aaszEvents[eEvent][iEvent]);
//		g_pLTServer->CPrint(szMusic);

		HSTRING hMusic = g_pLTServer->CreateString(szMusic);
		HMESSAGEWRITE hMessage = g_pLTServer->StartMessage(NULL, MID_MUSIC);
		g_pLTServer->WriteToMessageHString(hMessage, hMusic);
		g_pLTServer->EndMessage2(hMessage, MESSAGE_GUARANTEED | MESSAGE_NAGGLE);
		FREE_HSTRING(hMusic);
	}
}

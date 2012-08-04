// (c) 1997-2000 Monolith Productions, Inc.  All Rights Reserved

#include "StdAfx.h"
#include "MusicMgr.h"
#include "GameButeMgr.h"
#include "MsgIds.h"
#include "GameServerShell.h"

extern CGameServerShell* g_pGameServerShell;

// ----------------------------------------------------------------------- //

class CMusicButeMgr : public CGameButeMgr
{
	public : // Public member variables

        LTBOOL	Init(const char* szAttributeFile = "Attributes\\Music.txt")
		{
			return Parse(szAttributeFile);
		}

		CButeMgr* GetButeMgr() { return &m_buteMgr; }
};

// ----------------------------------------------------------------------- //

CMusicMgr* g_pMusicMgr = LTNULL;
static CMusicMgr s_MusicMgr;

// ----------------------------------------------------------------------- //

const char* s_aszMusicMoods[] = { "None", "Routine", "Investigate", "Aggressive" };
static const char* s_aszEvents[] = { "PlayerDie", "AIDie", "AIDodge" };

static const LTFLOAT s_fMax = 10.0f;

// ----------------------------------------------------------------------- //
static CVarTrack g_ShowMusicTrack;

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
	
	m_pMusicButeMgr = NULL;

	m_bInitialized = LTFALSE;
}

// ----------------------------------------------------------------------- //

CMusicMgr::~CMusicMgr()
{
	Term();

	// Only destroy musicbutemgr on destruction, not in term.  We only
	// need it initialized once.
	if( m_pMusicButeMgr )
	{
		delete m_pMusicButeMgr;
		m_pMusicButeMgr = NULL;
	}

	g_pMusicMgr = LTNULL;
}

// ----------------------------------------------------------------------- //

void CMusicMgr::Init(const char* szMusicControlFile)
{
	// Start fresh.
	Term( );

	if( !m_pMusicButeMgr )
	{
		m_pMusicButeMgr = new CMusicButeMgr( );
		if( !m_pMusicButeMgr )
			return;

		if( !m_pMusicButeMgr->Init())
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
			sprintf(szMood, "%s%d", s_aszMusicMoods[iMood], iLevel);
			m_aanMoods[iMood][m_acMoods[iMood]] = m_pMusicButeMgr->GetButeMgr( )->GetInt(m_szTheme, szMood, m_aanMoods[iMood][m_acMoods[iMood]]);
			if ( m_pMusicButeMgr->GetButeMgr( )->Success( ))
			{
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
			m_pMusicButeMgr->GetButeMgr( )->GetString(m_szTheme, szEvent, "", m_aaszEvents[iEvent][m_acEvents[iEvent]], sizeof(m_aaszEvents[iEvent][m_acEvents[iEvent]]));
			if ( m_pMusicButeMgr->GetButeMgr( )->Success( ))
			{
				m_acEvents[iEvent]++;
			}
			else
			{
				break;
			}
		}
	}

	m_afEventChances[eEventPlayerDie]	= (LTFLOAT)m_pMusicButeMgr->GetButeMgr( )->GetDouble("settings", "DieChance");
	m_afEventChances[eEventAIDie]		= (LTFLOAT)m_pMusicButeMgr->GetButeMgr( )->GetDouble("settings", "DieChance");
	m_afEventChances[eEventAIDodge]		= (LTFLOAT)m_pMusicButeMgr->GetButeMgr( )->GetDouble("settings", "DodgeChance");

	m_bInitialized = LTTRUE;

#ifndef _FINAL
	if ( !g_ShowMusicTrack.IsInitted() )
	{
		g_ShowMusicTrack.Init(g_pLTServer, "ShowMusicChange", LTNULL, 0.0f);
	}
#endif

}

// ----------------------------------------------------------------------- //

void CMusicMgr::Term()
{
	if ( !m_bInitialized ) return;

	m_bLockedMood = LTFALSE;
	m_bLockedEvent = LTFALSE;
	m_bRestoreMusicIntensity = LTFALSE;
	m_eLastMood = eMoodInvalid;

	for ( uint32 iMood = 0 ; iMood < kNumMoods ; ++iMood )
	{
		m_afMoods[iMood] = 0.0f;
	}

	m_bInitialized = LTFALSE;
}

// ----------------------------------------------------------------------- //

void CMusicMgr::Save(ILTMessage_Write *pMsg)
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

void CMusicMgr::Load(ILTMessage_Read *pMsg)
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

	if ( m_bLockedMood )
	{
		if ( m_bRestoreMusicIntensity )
		{
			char szMusic[128];
			sprintf(szMusic, "MUSIC I %d measure", m_iRestoreMusicIntensity);

#ifndef _FINAL
			if ( g_ShowMusicTrack.GetFloat() > 0 )
			{
				g_pLTServer->CPrint("Server sending client Music Message: (%s)", szMusic);
			}
#endif

			HSTRING hMusic = g_pLTServer->CreateString(szMusic);
			CAutoMessage cMsg;
			cMsg.Writeuint8(MID_MUSIC);
			cMsg.WriteHString(hMusic);
			g_pLTServer->SendToClient(cMsg.Read(), LTNULL, MESSAGE_GUARANTEED);
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
				SetMood(( Mood )iMood );
				bChoseMood = LTTRUE;
			}
		}

		m_afMoods[iMood] = Max<LTFLOAT>(m_afMoods[iMood] - g_pLTServer->GetFrameTime(), 0.0f);
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

	if ( m_bLockedEvent ) return;

	if ( m_acEvents[eEvent] != 0 && (m_afEventChances[eEvent] > GetRandom(0.0, 1.0f)) )
	{
		char szMusic[128];
		uint32 iEvent = GetRandom(0, m_acEvents[eEvent]-1);
		sprintf(szMusic, "MUSIC PM %s %s Beat", m_szTheme, m_aaszEvents[eEvent][iEvent]);

#ifndef _FINAL
		if ( g_ShowMusicTrack.GetFloat() > 0 )
		{
			g_pLTServer->CPrint("Server sending client Music Message: (%s)", szMusic);
		}
#endif

		HSTRING hMusic = g_pLTServer->CreateString(szMusic);
		CAutoMessage cMsg;
		cMsg.Writeuint8(MID_MUSIC);
		cMsg.WriteHString(hMusic);
		g_pLTServer->SendToClient(cMsg.Read(), LTNULL, MESSAGE_GUARANTEED);
		FREE_HSTRING(hMusic);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CMusicMgr::SetMood
//
//	PURPOSE:	Sets the mood.  Sends client message immediately.
//
// ----------------------------------------------------------------------- //

bool CMusicMgr::SetMood( Mood eMood )
{
	char szMusic[128];
	uint32 iLevel = GetRandom(0, m_acMoods[eMood]-1);
	sprintf(szMusic, "MUSIC I %d measure", m_aanMoods[eMood][iLevel]);

#ifndef _FINAL
	if ( g_ShowMusicTrack.GetFloat() > 0 )
	{
		g_pLTServer->CPrint("Server sending client Music Message: (%s)", szMusic);
	}
#endif

	HSTRING hMusic = g_pLTServer->CreateString(szMusic);
	CAutoMessage cMsg;
	cMsg.Writeuint8(MID_MUSIC);
	cMsg.WriteHString(hMusic);
	g_pLTServer->SendToClient(cMsg.Read(), LTNULL, MESSAGE_GUARANTEED);
	FREE_HSTRING(hMusic);

	m_eLastMood = eMood;
	return true;
}

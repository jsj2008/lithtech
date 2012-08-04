// ----------------------------------------------------------------------- //
//
// MODULE  : ServerSoundMgr.cpp
//
// PURPOSE : ServerSoundMgr implementation - Controls sound on the server
//
// CREATED : 7/10/00
//
// (c) 2000-2002 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "ServerSoundMgr.h"
#include "CommonUtilities.h"

// Global pointer to server sound mgr...

CServerSoundMgr*  g_pServerSoundMgr = LTNULL;

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CServerSoundMgr::CServerSoundMgr
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

CServerSoundMgr::CServerSoundMgr()
{
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CServerSoundMgr::~CServerSoundMgr
//
//	PURPOSE:	Destructor
//
// ----------------------------------------------------------------------- //

CServerSoundMgr::~CServerSoundMgr()
{
	Term();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CServerSoundMgr::Init()
//
//	PURPOSE:	Init mgr
//
// ----------------------------------------------------------------------- //

LTBOOL CServerSoundMgr::Init(const char* szAttributeFile)
{
	g_pServerSoundMgr = this;

    return CGameSoundMgr::Init(szAttributeFile);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CServerSoundMgr::Term()
//
//	PURPOSE:	Clean up.
//
// ----------------------------------------------------------------------- //

void CServerSoundMgr::Term()
{
    g_pServerSoundMgr = LTNULL;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CServerSoundMgr::GetParentSoundTemplate()
//
//	PURPOSE:	Get the name of the parent sound template
//
// ----------------------------------------------------------------------- //

void CServerSoundMgr::GetParentSoundTemplate(char* szParentTemplate, int nStrSize, 
											 const char* szSoundTemplate)
{
	m_buteMgr.GetString(szSoundTemplate, "Parent", "", szParentTemplate, nStrSize);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CServerSoundMgr::PlaySoundDirect()
//
//	PURPOSE:	Allows the game to *basically* bypass the sound mgr and
//				call directly into the engine sound code.  This is mainly
//				for backwards compatibility.
//
// ----------------------------------------------------------------------- //

HLTSOUND CServerSoundMgr::PlaySoundDirect(PlaySoundInfo & psi)
{
	return PlaySound(psi);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CServerSoundMgr::PlaySound()
//
//	PURPOSE:	Play the sound associated with the sound info...
//
// ----------------------------------------------------------------------- //

HLTSOUND CServerSoundMgr::PlaySound(PlaySoundInfo & psi)
{
	// Play the sound...

	HLTSOUND hSnd = LTNULL;
	LTRESULT hResult = g_pLTServer->SoundMgr()->PlaySound(&psi, hSnd);

	if (hResult != LT_OK)
	{
		// [RP] We are missing so many resources that this is just annoying.  When we start getting 
		// more sounds, put the assert back so we know whats missing.
		//_ASSERT(LTFALSE);
		g_pLTServer->CPrint("ERROR in CServerSoundMgr::PlaySound() - Couldn't play sound '%s'", psi.m_szSoundName);
		return LTNULL;
	}

	// [RP] The sound handle that gets passed into PlaySound(), hSnd, will *always* get set.  Since the
	// SoundTracks get recycled if we return hSnd we may be setting a handle to a SoundTrack that will
	// get removed by the engine and put back on the free list.  Any future calls to KillSound() using that
	// handle will be killing the wrong sound or *worse*, a sound that doesn't exist.  Returning the handle 
	// of the PlaySoundInfo struct will ensure we only return a valid handle if explicitly told to (ie. PLAYSOUND_GETHANDLE);

	return psi.m_hSound;
}



// [KLS 6/24/02] - Added this function as a way to format the attribute
// file correctly.  This is ONLY used during development to assist the
// content creators in formatting the file correctly.  It takes advantage
// of a bunch of game-specific knowledge and really only needs to be called
// once to set up the attribute file.
#ifdef _DEBUG
// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CServerSoundMgr::FormatAttributeFile()
//
//	PURPOSE:	Format the AI section of the attribute file
//
// ----------------------------------------------------------------------- //

#include "AISounds.h"
void CServerSoundMgr::FormatAttributeFile()
{

#ifdef _FORMAT_ATTRIBUTE_FILE

	uint32 cModels = g_pModelButeMgr->GetNumModels();

	// Loop over every AI sound template.  If it doesn't exist in the attribute file
	// add it.

	for ( uint32 iModel = 0 ; iModel < cModels ; iModel++ )
	{
		char* szSoundTemplate = (char*) g_pModelButeMgr->GetModelSoundTemplate((ModelId)iModel);

		if ( !m_buteMgr.Exist(szSoundTemplate) )
		{
			m_buteMgr.AddTag(szSoundTemplate);
		}
	}

	// For every AI sound template make sure at least one attribute for every possible
	// sound type exists.
	
	for ( iModel = 0 ; iModel < cModels ; iModel++ )
	{
		char* szSoundTemplate = (char*) g_pModelButeMgr->GetModelSoundTemplate((ModelId)iModel);

		if ( szSoundTemplate )
		{
			for ( uint32 iTag = 0 ; iTag < kAIS_Count ; iTag++ )
			{
				char aAttName[100];
				sprintf(aAttName, "%s0", s_aszAISoundTypes[iTag]);

				if ( !m_buteMgr.Exist(szSoundTemplate, aAttName) )
				{
					m_buteMgr.SetString(szSoundTemplate, aAttName, "");
				}
			}
		}
	}

	// Make sure we write this bad-boy out. ;)

	m_buteMgr.Save();

#endif // _FORMAT_ATTRIBUTE_FILE

}

#endif // _DEBUG
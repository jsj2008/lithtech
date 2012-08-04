#include "bdefs.h"

#include "iltsoundmgr.h"
#include "serverevent.h"
#include "sounddata.h"
#include "servermgr.h"
#include "soundtrack.h"
#include "s_net.h"
#include "packetdefs.h"


//------------------------------------------------------------------
//------------------------------------------------------------------
// Holders and their headers.
//------------------------------------------------------------------
//------------------------------------------------------------------

//IWorldServerBSP holder
#include "world_server_bsp.h"
static IWorldServerBSP *world_bsp_server;
define_holder(IWorldServerBSP, world_bsp_server);

//server file mgr.
#include "server_filemgr.h"
static IServerFileMgr *server_filemgr;
define_holder(IServerFileMgr, server_filemgr);




//
//Our server side implementation of ILTSoundMgr.
//

class CLTSoundMgrServer : public ILTSoundMgr {
public:
    declare_interface(CLTSoundMgrServer);

    //the ILTSoundMgr functions.
    LTRESULT PlaySound(PlaySoundInfo *pPlaySoundInfo, HLTSOUND &hResult);
    LTRESULT GetSoundDuration(HLTSOUND hSound, LTFLOAT &fDuration);
    LTRESULT IsSoundDone(HLTSOUND hSound, bool &bDone);
    LTRESULT KillSound(HLTSOUND hSound);
    LTRESULT KillSoundLoop(HLTSOUND hSound);
};

//instantiate our implementation class
instantiate_interface(CLTSoundMgrServer, ILTSoundMgr, Server);


LTRESULT CLTSoundMgrServer::PlaySound(PlaySoundInfo *pPlaySoundInfo, HLTSOUND &hResult) {
    CServerEvent *pEvent;
    CSoundData *pSoundData = LTNULL;
    CSoundTrack *pSoundTrack = LTNULL;
    UsedFile *pFile;
	bool bTrackTime;

    ASSERT(pPlaySoundInfo);
    if (!pPlaySoundInfo) {
        return LT_INVALIDPARAMS;
    }
 
    // Need a world to compress positions...
    if (!world_bsp_server->IsLoaded()) 
	{
		RETURN_ERROR(1, (CLTSoundMgrServer::PlaySound:  No world.), LT_ERROR);	
    }

    // Check if sound attached to object and someone forgot the handle to the object...
    if (pPlaySoundInfo->m_dwFlags & PLAYSOUND_ATTACHED && !pPlaySoundInfo->m_hObject) 
	{
		RETURN_ERROR(1, (CLTSoundMgrServer::PlaySound:  Parent object not specified in attachment.), LT_ERROR);	
    }

    // Make sure they have gethandle when the do anything that makes the sound stick around.
    if( !( pPlaySoundInfo->m_dwFlags & PLAYSOUND_GETHANDLE ) &&
		( pPlaySoundInfo->m_dwFlags & ( PLAYSOUND_TIME | PLAYSOUND_TIMESYNC )))
	{
		RETURN_ERROR(1, (CLTSoundMgrServer::PlaySound:  PLAYSOUND_GETHANDLE needed with specified flags.), LT_ERROR);
    }

    // Get pointer to file...
	char pszFinalFilename[_MAX_PATH + 1];
	CHelpers::FormatFilename(pPlaySoundInfo->m_szSoundName, pszFinalFilename, _MAX_PATH + 1);

    if (server_filemgr->AddUsedFile(pszFinalFilename, 0, &pFile) == 0) 
	{
        if (g_DebugLevel >= 2) {
            dsi_PrintToConsole("Missing sound file %s", pszFinalFilename);
        }
        return LT_MISSINGFILE;
    }

    // Limit the radii...
    if (pPlaySoundInfo->m_dwFlags & (PLAYSOUND_AMBIENT | PLAYSOUND_3D)) {
        pPlaySoundInfo->m_fOuterRadius = LTCLAMP(pPlaySoundInfo->m_fOuterRadius, MIN_SOUND_RADIUS, MAX_SOUND_RADIUS);
        pPlaySoundInfo->m_fInnerRadius = LTCLAMP(pPlaySoundInfo->m_fInnerRadius, MIN_SOUND_RADIUS, MAX_SOUND_RADIUS);
    }

    // Remove flags that don't change anything.
    if ((pPlaySoundInfo->m_dwFlags & PLAYSOUND_CTRL_VOL) && pPlaySoundInfo->m_nVolume == 100) {
        pPlaySoundInfo->m_dwFlags &= ~PLAYSOUND_CTRL_VOL;
    }
    if ((pPlaySoundInfo->m_dwFlags & PLAYSOUND_CTRL_PITCH) && pPlaySoundInfo->m_fPitchShift == 1.0f) {
        pPlaySoundInfo->m_dwFlags &= ~PLAYSOUND_CTRL_PITCH;
    }

    // Check if server needs to keep a reference to the sound...
    if (pPlaySoundInfo->m_dwFlags & (PLAYSOUND_GETHANDLE | PLAYSOUND_LOOP | PLAYSOUND_TIME | PLAYSOUND_TIMESYNC | PLAYSOUND_ATTACHED)) {
        // Removed an invalid combination...
        if (pPlaySoundInfo->m_dwFlags & PLAYSOUND_ATTACHED) {
            pPlaySoundInfo->m_dwFlags &= ~PLAYSOUND_CLIENTLOCAL;
        }

        // Check if server needs to track the time...
        if (pPlaySoundInfo->m_dwFlags & (PLAYSOUND_TIME | PLAYSOUND_TIMESYNC | PLAYSOUND_ATTACHED) &&
            !(pPlaySoundInfo->m_dwFlags & PLAYSOUND_LOOP))
        {
            // Check if file data already read...
            pSoundData = g_pServerMgr->GetSoundData(pFile);
            if (!pSoundData) {
                return LT_ERROR;
            }
 			bTrackTime = TRUE;
        }
        else
        {
        	bTrackTime = FALSE;
        }

        // Create the instance of the sound on the server...
        LT_MEM_TRACK_ALLOC(pSoundTrack = (CSoundTrack *)sb_Allocate(&g_pServerMgr->m_SoundTrackBank), LT_MEM_TYPE_SOUND);
        ASSERT(pSoundTrack);
        if (!pSoundTrack) {
            return LT_ERROR;
        }

        // Set handle for caller...
        if (pPlaySoundInfo->m_dwFlags & PLAYSOUND_GETHANDLE) {
            pPlaySoundInfo->m_hSound = (HLTSOUND)pSoundTrack;
        }
        else {
            pPlaySoundInfo->m_hSound = LTNULL;
        }

        // Initialize the soundtrack...
        if (!pSoundTrack->Init(pPlaySoundInfo, g_pServerMgr->m_GameTime, pFile, pSoundData, bTrackTime )) {
            // Undo all of it...
            pPlaySoundInfo->m_hSound = LTNULL;
            sb_Free(&g_pServerMgr->m_SoundTrackBank, pSoundTrack);
            return LT_ERROR;
        }

        // Put in sound list...
        dl_AddHead(&g_pServerMgr->m_SoundTrackList, &pSoundTrack->m_Link, pSoundTrack);
    }
    // Handle sound that the server doesn't need to care about...
    else {
        // Setup a sound event.
        pEvent = CreateServerEvent(EVENT_PLAYSOUND);
		if (pEvent)
		{
			PLAYSOUNDINFO_COPY(pEvent->m_PlaySoundInfo, *pPlaySoundInfo);
			pEvent->m_pUsedFile = pFile;
		}
    }

    hResult = (HLTSOUND)pSoundTrack;
    return LT_OK;
}

LTRESULT CLTSoundMgrServer::GetSoundDuration(HLTSOUND hSound, LTFLOAT &fDuration) {
    CSoundTrack *pSoundTrack;

    pSoundTrack = (CSoundTrack *)hSound;
    if (!pSoundTrack || !pSoundTrack->IsTrackTime()) {
        RETURN_ERROR(1, CLTSoundMgrServer::GetSoundDuration, LT_INVALIDPARAMS);
    }

    fDuration = pSoundTrack->GetDuration();

    return LT_OK;
}

LTRESULT CLTSoundMgrServer::IsSoundDone(HLTSOUND hSound, bool &bDone) {
    CSoundTrack *pSoundTrack;

    pSoundTrack = (CSoundTrack *)hSound;
    if (!pSoundTrack) {
        RETURN_ERROR(1, CLTSoundMgrServer::IsSoundDone, LT_INVALIDPARAMS);
    }

    bDone = (pSoundTrack->IsDone() != 0);

    return LT_OK;
}

LTRESULT CLTSoundMgrServer::KillSound(HLTSOUND hSound) {
    CSoundTrack *pSoundTrack;

    pSoundTrack = (CSoundTrack *)hSound;
    if (!pSoundTrack) {
        RETURN_ERROR(1, CLTSoundMgrServer::KillSound, LT_INVALIDPARAMS);
    }

    if (!(pSoundTrack->m_dwFlags & PLAYSOUND_GETHANDLE)) {
        RETURN_ERROR(1, CLTSoundMgrServer::KillSound, LT_ERROR);
    }

    pSoundTrack->m_fTimeLeft = 0.0f;
    pSoundTrack->SetRemove(LTTRUE);

    return LT_OK;
}

LTRESULT CLTSoundMgrServer::KillSoundLoop(HLTSOUND hSound) {
    CSoundTrack *pSoundTrack;

    pSoundTrack = (CSoundTrack *)hSound;
    if (!pSoundTrack) {
        RETURN_ERROR(1, CLTSoundMgrServer::KillSoundLoop, LT_INVALIDPARAMS);
    }

    if ((pSoundTrack->m_dwFlags & (PLAYSOUND_GETHANDLE | PLAYSOUND_LOOP)) != (PLAYSOUND_GETHANDLE | PLAYSOUND_LOOP))
        RETURN_ERROR(1, CLTSoundMgrServer::KillSoundLoop, LT_ERROR);
    {
        SetSoundTrackChangeFlags(pSoundTrack, CF_SOUNDINFO);
        pSoundTrack->m_fTimeLeft = 0.0f;
        pSoundTrack->SetRemove(LTTRUE);
    }

    return LT_OK;
}


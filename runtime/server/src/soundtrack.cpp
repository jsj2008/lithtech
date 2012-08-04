//------------------------------------------------------------------
//
//  FILE      : SOUNDTRACK.CPP
//
//  PURPOSE   : Defines CSoundTrack class
//
//  CREATED   : 11/5/98
//
//  COPYRIGHT : Microsoft 1998 All Rights Reserved
//
//------------------------------------------------------------------

#include "bdefs.h"

#include "soundtrack.h"
#include "serverobj.h"
#include "interlink.h"
#include "servermgr.h"
#include "sounddata.h"
#include "s_object.h"
#include "packetdefs.h"
#include "s_client.h"



LTBOOL CSoundTrack::Init(PlaySoundInfo *pPlaySoundInfo, float fStartTime, 
				UsedFile *pFile, CSoundData *pSoundData, bool bTrackTime)
{
    LTObject *pObj;
    LTVector vForward, vUp, vRight;
    float fExtraTime;

    ASSERT(pPlaySoundInfo);
    if (!pPlaySoundInfo)
        return LTFALSE;

    m_wChangeFlags = 0;
    m_nClientRefs = 0;
    m_pFile = pFile;

 	if (pSoundData == NULL) m_bTrackTime = FALSE;
 	else m_bTrackTime = bTrackTime;

    // Copy the playinfo information...
    m_dwFlags = pPlaySoundInfo->m_dwFlags;
    
    // Create link to object...
    if (m_dwFlags & (PLAYSOUND_ATTACHED | PLAYSOUND_CLIENTLOCAL))
    {
        pObj = HandleToServerObj(pPlaySoundInfo->m_hObject);
        ASSERT(pObj);
        if (!pObj)
        {
            m_fTimeLeft = 0.0f;
            SetRemove(LTTRUE);
            return LTFALSE;
        }

        // If it's an attachment, then we have to create an interlink, cuz object may go
        // away later...
        if (m_dwFlags & PLAYSOUND_ATTACHED)
            CreateInterLink(pObj, this, LINKTYPE_SOUND);
        // The clientlocal sounds only need the object pointer this frame...
        else
            m_pClientLocalObject = pObj;
    }
    else
    {
        m_pInterLink = LTNULL;
        pObj = LTNULL;
    }

 
	m_nPriority = pPlaySoundInfo->m_nPriority;
	m_fOuterRadius = pPlaySoundInfo->m_fOuterRadius;
	m_fInnerRadius = pPlaySoundInfo->m_fInnerRadius;
	m_nVolume = pPlaySoundInfo->m_nVolume;
	m_fPitchShift = pPlaySoundInfo->m_fPitchShift;
	
    // Need position from playsoundinfo if not attached...
    if (!(m_dwFlags & PLAYSOUND_ATTACHED))
    {
        if (m_dwFlags & (PLAYSOUND_3D | PLAYSOUND_AMBIENT))
        {
            m_vPosition = pPlaySoundInfo->m_vPosition;

        }
    }
    // Need position from object if attached...
    else
    {
        if (m_dwFlags & (PLAYSOUND_3D | PLAYSOUND_AMBIENT))
        {
            m_vPosition = pObj->GetPos();
        }
    }

    // Remember the user data
    m_UserData = pPlaySoundInfo->m_UserData;

    m_bRemove = LTFALSE;

    // Check if server needs to keep track of time...
    if (pSoundData)
    {
        m_pSoundData = pSoundData;
        m_fDuration = m_pSoundData->GetDuration();

		// MAG : this is a hack to try and give enough time for
		// the client to decompress file and finish first
		// so dialog isn't cut off
		if ( m_fDuration < 1.0f )
			fExtraTime = m_fDuration * 0.5f;
		else
			fExtraTime = 0.5f + m_fDuration * 0.03f;
        m_fTimeLeft = m_fDuration + fExtraTime;
    }
    else
    {
        m_pSoundData = LTNULL;
        m_fTimeLeft = 0.0f;
        m_fDuration = 0.0f;
    }

    m_fStartTime = fStartTime;

    // Get an id...
    if (sm_AllocateID(&m_pIDLink, INVALID_OBJECTID) != LT_OK)
    {
        return LTFALSE;
    }

    // Assign the id...
    g_pServerMgr->m_ObjectMap[GetLinkID(m_pIDLink)].m_nRecordType = RECORDTYPE_SOUND;
    g_pServerMgr->m_ObjectMap[GetLinkID(m_pIDLink)].m_pRecordData = this;

    SetSoundTrackChangeFlags(this, sm_GetNewSoundTrackChangeFlags(this));

    return LTTRUE;
}


void CSoundTrack::Update(float fDeltaTime)
{
    LTObject *pObj;
    uint16 wChangeFlags;
    LTVector vForward, vUp, vRight;

    // Update the timer...
    if (m_fTimeLeft > fDeltaTime)
	{
        m_fTimeLeft -= fDeltaTime;
	}
    else
	{
        m_fTimeLeft = 0.0f;
	}

    if (GetRemove())
        return;

    // Check to see if we need to remove the track for some reason.  Only do 
    // this if no handle.  If handle, then user always deletes...
    if (!(m_dwFlags & PLAYSOUND_GETHANDLE))
    {
        // Remove the track if timed out...
        if (!(m_dwFlags & PLAYSOUND_LOOP) && GetTimeLeft() <= 0.0f)
        {
            // Can only really remove when all the clients that know about have told us they are done with it...
            if (m_nClientRefs == 0)
            {
                SetRemove(LTTRUE);
                return;
            }
        }
    }

    if ((m_dwFlags & PLAYSOUND_ATTACHED) && (m_dwFlags & (PLAYSOUND_3D | PLAYSOUND_AMBIENT)))
    {
        pObj = GetObject();
        if (!pObj)
        {
            // If no handle, dump the sound...
            if (!(m_dwFlags & PLAYSOUND_GETHANDLE))
            {
                m_fTimeLeft = 0.0f;
                SetRemove(LTTRUE);
            }

            // Someone left a sound handle without an object!
            return;
        }

        wChangeFlags = 0;
        if (pObj->GetPos().x != m_vPosition.x || 
            pObj->GetPos().y != m_vPosition.y || 
            pObj->GetPos().z != m_vPosition.z)
        {
            wChangeFlags |= CF_POSITION;
            m_vPosition = pObj->GetPos();
        }

        if (wChangeFlags)
            SetSoundTrackChangeFlags(this, wChangeFlags);
    }
}


void CSoundTrack::Term()
{
    if (m_pIDLink)
	{
        sm_FreeID(m_pIDLink);
    }

    m_pSoundData = LTNULL;
    m_pIDLink = LTNULL;
}

float CSoundTrack::GetTimeLeft()
{
    return m_fTimeLeft;
}

LTBOOL CSoundTrack::IsTrackTime()
{
	return m_bTrackTime;
}

LTObject *CSoundTrack::GetObject()
{
    if (m_dwFlags & PLAYSOUND_ATTACHED)
    {
        if (m_pInterLink)
            return m_pInterLink->m_pOwner;
    }
    else if (m_dwFlags & PLAYSOUND_CLIENTLOCAL)
    {
        return m_pClientLocalObject;
    }

    return LTNULL;
}

// Adds the object to the list of changed objects.
void AddSoundTrackToChangeList(CSoundTrack *pSoundTrack)
{
    pSoundTrack->m_pChangedNext = g_pServerMgr->m_ChangedSoundTrackHead;
    g_pServerMgr->m_ChangedSoundTrackHead = pSoundTrack;
}

// ORs the object's flags with the flags you specify and adds
// the object to the 'changed object' list.
LTRESULT SetSoundTrackChangeFlags(CSoundTrack *pSoundTrack, uint32 flags)
{
    if (g_pServerMgr->m_ObjectMap[GetLinkID(pSoundTrack->m_pIDLink)].m_nRecordType != RECORDTYPE_SOUND || 
        !g_pServerMgr->m_ObjectMap[GetLinkID(pSoundTrack->m_pIDLink)].m_pRecordData)
    {
        RETURN_ERROR(1, SetSoundTrackChangeFlags, LT_ERROR);
    }

    // Make sure not to re-add it and screw it up.
    if (pSoundTrack->m_wChangeFlags == 0)
    {
        AddSoundTrackToChangeList(pSoundTrack);
    }

    pSoundTrack->m_wChangeFlags |= flags;
    ASSERT(pSoundTrack->m_wChangeFlags);

    return LT_OK;
}


float CSoundTrack::GetDuration()
{
    return m_fDuration;
}

LTBOOL CSoundTrack::IsDone()
{
    if (!IsTrackTime())
        return LTFALSE;

    // Not done until timer says we're done and no
	// clients are still playing the sound.
    if( m_fTimeLeft > 0.0f || m_nClientRefs != 0 )
        return LTFALSE;

    return LTTRUE;
}

void CSoundTrack::AddRef()
{
    // Some sounds the client has to tell us when it's done...
    if (m_dwFlags & (PLAYSOUND_TIME | PLAYSOUND_TIMESYNC | PLAYSOUND_ATTACHED) && 
        !(m_dwFlags & PLAYSOUND_LOOP))
        // Another client told about sound...
        m_nClientRefs++;
}

void CSoundTrack::Release(uint8 *pnClientSoundFlags)
{
    // Some sounds the client has to tell us when it's done...
    if (m_dwFlags & (PLAYSOUND_TIME | PLAYSOUND_TIMESYNC | PLAYSOUND_ATTACHED) && 
        !(m_dwFlags & PLAYSOUND_LOOP))
    {
        *pnClientSoundFlags |= OBJINFOSOUNDF_CLIENTDONE;

        // Remove client reference count to sound.
        if (m_nClientRefs > 0)
        {
            m_nClientRefs--;
        }

        // Time it out if no references left...
        if (m_nClientRefs == 0)
            m_fTimeLeft = 0.0f;
    }
}

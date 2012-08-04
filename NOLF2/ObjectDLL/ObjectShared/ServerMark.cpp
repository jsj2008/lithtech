// ----------------------------------------------------------------------- //
//
// MODULE  : CServerMark.cpp
//
// PURPOSE : CServerMark implementation
//
// CREATED : 1/15/99
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "ServerMark.h"
#include "WeaponMgr.h"
#include "ilttransform.h"
#include "CVarTrack.h"

CVarTrack	g_cvarMarkWMShow;
CVarTrack	g_cvarMarkWMFadeTime;
CVarTrack	g_cvarMarkWMSolidTime;

LINKFROM_MODULE( ServerMark );

#pragma force_active on
BEGIN_CLASS(CServerMark)
END_CLASS_DEFAULT_FLAGS(CServerMark, BaseClass, NULL, NULL, CF_HIDDEN)
#pragma force_active off

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CServerMark::CServerMark()
//
//	PURPOSE:	Initialize object
//
// ----------------------------------------------------------------------- //

CServerMark::CServerMark() : 
	BaseClass(OT_SPRITE),
	m_fElapsedTime(0.0f)
{
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CServerMark::~CServerMark()
//
//	PURPOSE:	Destructor
//
// ----------------------------------------------------------------------- //

CServerMark::~CServerMark()
{
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CServerMark::EngineMessageFn
//
//	PURPOSE:	Handle engine messages
//
// ----------------------------------------------------------------------- //

uint32 CServerMark::EngineMessageFn(uint32 messageID, void *pData, LTFLOAT fData)
{
	switch(messageID)
	{
		case MID_PARENTATTACHMENTREMOVED :
		{
            g_pLTServer->RemoveObject(m_hObject);
		}
		break;

		case MID_INITIALUPDATE:
		{
			//make sure that our console variables are setup
			if (!g_cvarMarkWMShow.IsInitted())
			{
				g_cvarMarkWMShow.Init(g_pLTServer, "MarkWMShow", NULL, 1.0f);
			}

			if (!g_cvarMarkWMFadeTime.IsInitted())
			{
				g_cvarMarkWMFadeTime.Init(g_pLTServer, "MarkWMFadeTime", NULL, 3.0f);
			}

			if (!g_cvarMarkWMSolidTime.IsInitted())
			{
				g_cvarMarkWMSolidTime.Init(g_pLTServer, "MarkWMSolidTime", NULL, 3.0f);
			}

			SetNextUpdate(m_hObject, UPDATE_NEXT_FRAME);
		}
		break;

		case MID_UPDATE:
		{
			//update the time based upon the server frame time
			m_fElapsedTime += g_pLTServer->GetFrameTime();

			//figure out our lifespan times
			float fSolidTime = g_cvarMarkWMSolidTime.GetFloat();
			float fFadeTime = g_cvarMarkWMFadeTime.GetFloat();

			//however, if we are disabled, set our time to 0
			if(g_cvarMarkWMShow.GetFloat() < 0.99f)
			{
				fSolidTime = fFadeTime = 0.0f;
			}

			//see if we are solid
			if(m_fElapsedTime < fSolidTime)
			{
				//we are solid, don't do anything, just don't update until we will start fading
				SetNextUpdate(m_hObject, UPDATE_NEXT_FRAME);
			}
			else if(m_fElapsedTime < (fSolidTime + fFadeTime))
			{
				//we are fading out, if we are in single player update our alpha
				//we don't in multiplayer because these marks could otherwise consume
				//a huge amount of bandwidth
				if(g_pGameServerShell->GetGameType() == eGameTypeSingle)
				{
					//determine what our color currently is
					float fRed, fGreen, fBlue, fAlpha;
					g_pLTServer->GetObjectColor(m_hObject, &fRed, &fGreen, &fBlue, &fAlpha);
					
					//set our alpha so it will ramp down to 0
					fAlpha = 1.0f - ((m_fElapsedTime - fSolidTime) / fFadeTime);
					g_pLTServer->SetObjectColor(m_hObject, fRed, fGreen, fBlue, fAlpha);
				}

				SetNextUpdate(m_hObject, UPDATE_NEXT_FRAME);
			}
			else
			{
				//we are out of time, remove this object
				g_pLTServer->RemoveObject(m_hObject);
			}
		}
		break;

		case MID_SAVEOBJECT:
		{
			OnSave( (ILTMessage_Write*)pData, (uint32)fData );
		}
		break;

		case MID_LOADOBJECT:
		{
			OnLoad( (ILTMessage_Read*)pData, (uint32)fData );
		}
		break;


		default : break;
	}

	return BaseClass::EngineMessageFn(messageID, pData, fData);
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	WorldModel::OnSave
//
//  PURPOSE:	Save the object
//
// ----------------------------------------------------------------------- //

void CServerMark::OnSave( ILTMessage_Write *pMsg, uint32 dwSaveFlags )
{
	pMsg->Writefloat(m_fElapsedTime);
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	WorldModel::OnLoad
//
//  PURPOSE:	Load the object
//
// ----------------------------------------------------------------------- //

void CServerMark::OnLoad( ILTMessage_Read *pMsg, uint32 dwSaveFlags )
{
	m_fElapsedTime = pMsg->Readfloat();
}
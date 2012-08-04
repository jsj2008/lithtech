 // ----------------------------------------------------------------------- //
//
// MODULE  : CameraObj.cpp
//
// PURPOSE : CameraObj object will be used to place cameras for 
//           scripted scenes.
//
// CREATED : 02/17/98
//
// ----------------------------------------------------------------------- //

#include "CameraObj.h"
#include "Generic_msg_de.h"
#include "ClientCastLineSFX.h"
#include "ClientServerShared.h"
#include "SfxMsgIDs.h"
#include "SharedDefs.h"
#include <mbstring.h>

DLink CameraObj::m_CameraHead;
DDWORD CameraObj::m_dwNumCameras = 0;



BEGIN_CLASS(CameraObj)
	ADD_LONGINTPROP(Type, CAMTYPE_FULLSCREEN)
	ADD_REALPROP(ActiveTime, -1)
	ADD_BOOLPROP(AllowPlayerMovement, DFALSE)
	ADD_BOOLPROP(StartActive, DFALSE)
	ADD_BOOLPROP(IsListener, DTRUE)
	ADD_BOOLPROP(HidePlayer, DFALSE)
	ADD_BOOLPROP_FLAG(TypeActionMode, DFALSE, PF_GROUP6)
END_CLASS_DEFAULT(CameraObj, B2BaseClass, NULL, NULL)


// Used by the player object to set the view if a camera is active
HOBJECT g_hActiveCamera;

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CameraObj::CameraObj()
//
//	PURPOSE:	Initialize object
//
// ----------------------------------------------------------------------- //

CameraObj::CameraObj() : B2BaseClass(OT_NORMAL)
{
    m_bActive		  = DFALSE;
	m_bStartActive	  = DFALSE;
	m_bIsListener	  = DTRUE;
	m_fActiveTime	  = -1;
	m_fDeactivateTime = 0;
	m_bPlayerMovement = DFALSE;
	m_nType			  = CAMTYPE_FULLSCREEN;
	m_hRay			  = DNULL;
    m_hLinkObject	  = DNULL;
	m_Link.m_pData	  = DNULL;
	m_bHidePlayer     = DFALSE;
	if( m_dwNumCameras == 0 )
	{
		dl_TieOff( &m_CameraHead );
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CameraObj::~CameraObj()
//
//	PURPOSE:	Deallocate object
//
// ----------------------------------------------------------------------- //

CameraObj::~CameraObj()
{
	SetActive(DFALSE);

	if( m_Link.m_pData && m_dwNumCameras > 0 )
	{
		dl_Remove( &m_Link );
		m_dwNumCameras--;
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CameraObj::EngineMessageFn
//
//	PURPOSE:	Handle engine messages
//
// ----------------------------------------------------------------------- //

DDWORD CameraObj::EngineMessageFn(DDWORD messageID, void *pData, DFLOAT fData)
{
	if (!g_pServerDE) return 0;

	switch(messageID)
	{
		case MID_UPDATE:
		{
			if (!Update())
            {
            	g_pServerDE->RemoveObject(m_hObject);		
            }
		}
		break;

		case MID_LINKBROKEN:
		{
			HOBJECT hObj = (HOBJECT)pData;
            if (m_hLinkObject == hObj)
            {
	    		m_hLinkObject = DNULL;
		    }
		}
		break;

		case MID_PRECREATE:
		{
			DDWORD dwRet = B2BaseClass::EngineMessageFn(messageID, pData, fData);

			if (fData == PRECREATE_WORLDFILE || fData == PRECREATE_STRINGPROP)
				ReadProp((ObjectCreateStruct*)pData);

			PostPropRead((ObjectCreateStruct*)pData);

			return dwRet;
		}
        break;

		case MID_INITIALUPDATE:
		{
			if (fData != INITIALUPDATE_SAVEGAME)
				InitialUpdate();

			// insert it into the list
			if (!m_Link.m_pData)
			{
				dl_Insert( &m_CameraHead, &m_Link );
				m_Link.m_pData = ( void * )this;
				m_dwNumCameras++;
			}
		}
		break;

		case MID_SAVEOBJECT:
		{
			Save((HMESSAGEWRITE)pData, (DDWORD)fData);
		}
		break;

		case MID_LOADOBJECT:
		{
			Load((HMESSAGEREAD)pData, (DDWORD)fData);
		}
		break;

		default : break;
	}


	return B2BaseClass::EngineMessageFn(messageID, pData, fData);
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CameraObj::ObjectMessageFn()
//
//	PURPOSE:	Handler for server object messages.
//
// --------------------------------------------------------------------------- //

DDWORD CameraObj::ObjectMessageFn(HOBJECT hSender, DDWORD messageID, HMESSAGEREAD hRead)
{
	CServerDE* pServerDE = GetServerDE();
	switch (messageID)
	{
		case MID_TRIGGER:
		{
			HSTRING hMsg = pServerDE->ReadFromMessageHString(hRead);
			if (hMsg)
			{
				HandleTrigger(hSender, hMsg);
				pServerDE->FreeString(hMsg);
			}
		}
		break;
	}
	
	return B2BaseClass::ObjectMessageFn(hSender, messageID, hRead);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	PostPropRead()
//
//	PURPOSE:	Update properties
//
// ----------------------------------------------------------------------- //

void CameraObj::PostPropRead(ObjectCreateStruct *pStruct)
{
	if (!pStruct || !g_pServerDE) return;
    
    pStruct->m_Flags = FLAG_FORCECLIENTUPDATE | FLAG_FULLPOSITIONRES | FLAG_GOTHRUWORLD;

// See if debug is on
    if ( g_pServerDE->GetVarValueFloat(g_pServerDE->GetGameConVar("DebugCutScene")) == 1.0f )
    {
		pStruct->m_ObjectType = OT_MODEL;
        pStruct->m_Flags |= FLAG_VISIBLE;

		char* pFilename = "Models\\CameraObject.abc";
		char* pSkin = "Skins\\CameraObject.dtx";
		_mbscpy((unsigned char*)pStruct->m_Filename, (const unsigned char*)pFilename);
		_mbscpy((unsigned char*)pStruct->m_SkinName, (const unsigned char*)pSkin);
    }

	// Set the Update!
	pStruct->m_NextUpdate = 0.001f;

//	m_nType = (DBYTE)pStruct->m_UserData;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CameraObj::ReadProp
//
//	PURPOSE:	Set property value
//
// ----------------------------------------------------------------------- //

DBOOL CameraObj::ReadProp(ObjectCreateStruct *pStruct)
{
	if (!g_pServerDE || !pStruct) return DFALSE;

	GenericProp genProp;

	if (g_pServerDE->GetPropGeneric("AllowPlayerMovement", &genProp) == DE_OK)
	{
		m_bPlayerMovement = genProp.m_Bool;
	}

	if (g_pServerDE->GetPropGeneric("StartActive", &genProp) == DE_OK)
	{
		m_bStartActive = genProp.m_Bool;
	}

	if (g_pServerDE->GetPropGeneric("IsListener", &genProp) == DE_OK)
	{
		m_bIsListener = genProp.m_Bool;
	}

	if (g_pServerDE->GetPropGeneric("HidePlayer", &genProp) == DE_OK)
	{
		m_bHidePlayer = genProp.m_Bool;
	}

	if (g_pServerDE->GetPropGeneric("ActiveTime", &genProp) == DE_OK)
	{
		m_fActiveTime = genProp.m_Float;
	}

	if (g_pServerDE->GetPropGeneric("Type", &genProp) == DE_OK)
	{
		m_nType = (DBYTE)genProp.m_Long;
	}

	return DTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CameraObj::InitialUpdate
//
//	PURPOSE:	Initial update
//
// ----------------------------------------------------------------------- //

extern char g_szVarWorldName[];

DBOOL CameraObj::InitialUpdate()
{
	DBYTE nFlags;
	char szWorldName[_MAX_PATH];

	if (!g_pServerDE) return DFALSE;

	SetActive(m_bStartActive);

	// Tell clients about the camera...
	HMESSAGEWRITE hMessage = g_pServerDE->StartSpecialEffectMessage(this);
	g_pServerDE->WriteToMessageByte(hMessage, SFX_CAMERA_ID);
	g_pServerDE->WriteToMessageByte(hMessage, m_nType);
	nFlags = 0;
	if( m_bPlayerMovement )
		nFlags |= CAMERASFXFLAG_ALLOWPLAYERMOVEMENT;
	if( m_bHidePlayer )
		nFlags |= CAMERASFXFLAG_HIDEPLAYER;
	if( m_bIsListener )
	{
		nFlags |= CAMERASFXFLAG_ISLISTENER;

		// The b2 1.0 worlds were designed around a bug with the camera that did not pass the islistener
		// flag down.  So this hack prevents the islistener flag for those levels.
		if( g_szVarWorldName )
		{
			HCONVAR hTempVar = g_pServerDE->GetGameConVar(g_szVarWorldName);
			if (hTempVar)
			{
				char* sTempString = g_pServerDE->GetVarValueString(hTempVar);
				if (sTempString)
				{
					SAFE_STRCPY( szWorldName, sTempString);
					_strlwr( szWorldName );
					if( strstr( szWorldName, "worlds\\" ))
						nFlags &= ~CAMERASFXFLAG_ISLISTENER;
				}
			}
		}
	}
	g_pServerDE->WriteToMessageByte( hMessage, nFlags );
	g_pServerDE->EndMessage(hMessage);

	DDWORD dwUsrFlags = g_pServerDE->GetObjectUserFlags(m_hObject);
	g_pServerDE->SetObjectUserFlags(m_hObject, dwUsrFlags | USRFLG_SAVEABLE);

	return DTRUE;
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CameraObj::Setup
//
//	PURPOSE:	Handles setup
//
// ----------------------------------------------------------------------- //

DBOOL CameraObj::Setup(DBYTE nType)
{
	m_nType = nType;

	return DTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CameraObj::DisplayRay
//
//	PURPOSE:	Display a Ray from the Camera
//
// ----------------------------------------------------------------------- //
DBOOL CameraObj::DisplayRay()
{
	if (!g_pServerDE) return DFALSE;
    
    // Cast a Ray Forward From the camera (DebugOnly)
    if (m_hRay) g_pServerDE->RemoveObject(m_hRay);
    
	ObjectCreateStruct theStruct;
	INIT_OBJECTCREATESTRUCT(theStruct);

	DVector vPos;
	DRotation rRot;
    
	g_pServerDE->GetObjectPos(m_hObject, &vPos);
	g_pServerDE->GetObjectRotation(m_hObject, &rRot);
    
	VEC_COPY(theStruct.m_Pos, vPos);
	ROT_COPY(theStruct.m_Rotation,rRot);

	HCLASS hClass = g_pServerDE->GetClass("CClientCastLineSFX");
	CClientCastLineSFX* pLine = DNULL;

	if (hClass)
	{
		pLine = (CClientCastLineSFX*)g_pServerDE->CreateObject(hClass, &theStruct);
		if (!pLine) return DFALSE;
	}

	DVector vColor;
	VEC_SET(vColor, 0.0f, 1.0f, 0.0f);                      // Green Line
	pLine->Setup(vColor, vColor, 1.0f, 1.0f);
    
	m_hRay = pLine->m_hObject;
    
    return DTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CameraObj::Update
//
//	PURPOSE:	Handle Update
//
// ----------------------------------------------------------------------- //

DBOOL CameraObj::Update()
{
	if (!g_pServerDE) return DFALSE;

   	g_pServerDE->SetNextUpdate(m_hObject, 0.01f);

// Need to check if Camera has Moved...

// If so, we need to change the Line...
// and Send new camera position
//
// Might have to send a pan up, pan down, pan direction, because of client/server slow down.
// That way all the updates are done on the client.

//    if (g_pServerDE->GetVarValueFloat(g_pServerDE->GetGameConVar("DebugCutScene")) == 1.0f)
//    {
//        DisplayRay();
//    }

    // If we are linked then check to make sure the Link has not moved...
    if (m_hLinkObject)
    {
        DVector vPos, vMyPos;
	    VEC_INIT(vPos);
	    VEC_INIT(vMyPos);
        
        // From this Point
    	g_pServerDE->GetObjectPos(m_hObject, &vMyPos);
        
        // To this Point
    	g_pServerDE->GetObjectPos(m_hLinkObject, &vPos);

		DVector vF, vU;

		VEC_SUB(vF, vPos, vMyPos);
		VEC_SET(vU, 0, 1, 0);

    	DRotation rMyNewRot;
    	
		g_pServerDE->AlignRotation(&rMyNewRot, &vF, &vU);

		g_pServerDE->SetObjectRotation(m_hObject, &rMyNewRot);
	}

	// Deactivate if the active time has expired.
	if (m_fActiveTime > 0 && g_pServerDE->GetTime() > m_fDeactivateTime)
		SetActive(DFALSE);

	return DTRUE;
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CameraObj::SetActive
//
//	PURPOSE:	Activates or deactivates the camera
//
// ----------------------------------------------------------------------- //

void CameraObj::SetActive(DBOOL bActive)
{
	if (!g_pServerDE) return;

	m_bActive = bActive;

	DDWORD dwUsrFlags = g_pServerDE->GetObjectUserFlags(m_hObject);
	if (bActive)
	{
		g_pServerDE->SetObjectUserFlags(m_hObject, dwUsrFlags | USRFLG_VISIBLE);
		g_pServerDE->SetNextUpdate(m_hObject, 0.01f);
		if (m_fActiveTime > 0)
			m_fDeactivateTime = g_pServerDE->GetTime() + m_fActiveTime;
	}
	else
	{
		g_pServerDE->SetObjectUserFlags(m_hObject, dwUsrFlags & ~USRFLG_VISIBLE);
		g_pServerDE->SetNextUpdate(m_hObject, 0.0f);
		m_fDeactivateTime = 0;
	}

	if (m_bIsListener && bActive)
	{
		g_hActiveCamera = m_hObject;
	}
	else	// Look for another active camera
	{
		g_hActiveCamera = DNULL;

		DLink* pLink = CameraObj::m_CameraHead.m_pNext;
		while(pLink != &CameraObj::m_CameraHead)
		{
			CameraObj* pCam = (CameraObj*)pLink->m_pData;
			if (pCam->IsActive() && pCam->IsListener())
			{
				g_hActiveCamera = pCam->m_hObject;
				break;
			}
			pLink = pLink->m_pNext;
		}
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CameraObj::SetLinkObject
//
//	PURPOSE:	Sets an object for the camera to link to.
//
// ----------------------------------------------------------------------- //

DBOOL CameraObj::SetLinkObject(HOBJECT hLinkObject)
{
	if (!g_pServerDE) return DFALSE;

	if (hLinkObject)
	{
		g_pServerDE->CreateInterObjectLink(m_hObject, hLinkObject);

		// If active, start doing updates to track the object
		if (m_bActive)
			g_pServerDE->SetNextUpdate(m_hObject, 0.01f);
	}
	else
	{
		if (m_hLinkObject)	// Break link?
			g_pServerDE->BreakInterObjectLink(m_hObject, m_hLinkObject);

		g_pServerDE->SetNextUpdate(m_hObject, 0.0f);
	}

	m_hLinkObject = hLinkObject;

	return DTRUE;
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	VolumeBrush::TriggerMsg()
//
//	PURPOSE:	Handler for volume brush trigger messages.
//
// --------------------------------------------------------------------------- //

void CameraObj::HandleTrigger(HOBJECT hSender, HSTRING hMsg)
{
	if (!g_pServerDE) return;

	char* pMsg = g_pServerDE->GetStringData(hMsg);
	if (!pMsg) return;

	// Turn the camera on
	if (_mbsicmp((const unsigned char*)pMsg, (const unsigned char*)"ON") == 0)
	{
		SetActive(DTRUE);
	}
	
	// Turn the camera off
	else if (_mbsicmp((const unsigned char*)pMsg, (const unsigned char*)"OFF") == 0)
	{
		SetActive(DFALSE);
	}

	// Link to a named object
	else if (strnicmp(pMsg, "LINK", 4) == 0)
	{ 
		if (_mbstrlen(pMsg) >= 6)
		{
			// Skip ahead to the name of the object
			pMsg += 5;

			ObjectList*	pList = g_pServerDE->FindNamedObjects(pMsg);
    		if (pList)
			{
				ObjectLink* pLink = pList->m_pFirstLink;
				if (pLink)
    			{
					SetLinkObject(pLink->m_hObject);
	    		}

				g_pServerDE->RelinquishList(pList);
			}
		}
		else
		{
			SetLinkObject(DNULL);
		}
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CameraObj::Save
//
//	PURPOSE:	Save the object
//
// ----------------------------------------------------------------------- //

void CameraObj::Save(HMESSAGEWRITE hWrite, DDWORD dwSaveFlags)
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE || !hWrite) return;

	// Only need to save the data that changes (all the data in the
	// special fx message is saved/loaded for us)...

	DFLOAT fTime = pServerDE->GetTime();

	pServerDE->WriteToMessageByte(hWrite, m_bActive);
	pServerDE->WriteToMessageByte(hWrite, m_bStartActive);
	pServerDE->WriteToMessageByte(hWrite, m_bIsListener);
	pServerDE->WriteToMessageByte(hWrite, m_bPlayerMovement);
	pServerDE->WriteToMessageByte(hWrite, m_nType);
	pServerDE->WriteToMessageFloat(hWrite, m_fActiveTime);
	pServerDE->WriteToMessageFloat(hWrite, m_fDeactivateTime - fTime);
	pServerDE->WriteToMessageByte(hWrite, m_bHidePlayer);
	pServerDE->WriteToLoadSaveMessageObject(hWrite, m_hLinkObject);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CameraObj::Load
//
//	PURPOSE:	Load the object
//
// ----------------------------------------------------------------------- //

void CameraObj::Load(HMESSAGEREAD hRead, DDWORD dwLoadFlags)
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE || !hRead) return;

	DFLOAT fTime = pServerDE->GetTime();

	m_bActive		  = pServerDE->ReadFromMessageByte(hRead);
	m_bStartActive	  = pServerDE->ReadFromMessageByte(hRead);
	m_bIsListener	  = pServerDE->ReadFromMessageByte(hRead);
	m_bPlayerMovement = pServerDE->ReadFromMessageByte(hRead);
	m_nType			  = pServerDE->ReadFromMessageByte(hRead);
	m_fActiveTime	  = pServerDE->ReadFromMessageFloat(hRead);
	m_fDeactivateTime = pServerDE->ReadFromMessageFloat(hRead) + fTime;
	m_bHidePlayer     = pServerDE->ReadFromMessageByte(hRead);
	pServerDE->ReadFromLoadSaveMessageObject(hRead, &m_hLinkObject);

	SetActive(m_bActive);	// (new) [blg] 02/17/99
}



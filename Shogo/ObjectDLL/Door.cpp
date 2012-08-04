//----------------------------------------------------------
//
// MODULE  : DOOR.CPP
//
// PURPOSE : a Door object
//
// CREATED : 8/5/97 5:07:00 PM
//
//----------------------------------------------------------

// Includes...

#include "generic_msg_de.h"
#include "door.h"
#include "Trigger.h"
#include "DebrisTypes.h"
#include "ClientServerShared.h"


// Defines....

#define DOORSTATE_CLOSED		1
#define DOORSTATE_CLOSING		2
#define DOORSTATE_OPEN			3
#define DOORSTATE_OPENING		4

#define DOOR_UPDATE_DELTA			0.01f

#define	DOOR_DEFAULT_BLOCKING_PRIORITY	255
#define	DOOR_DEFAULT_DEACTIVATION_TIME	30.0f


// Static global variables..
static char *g_szTrigger = "TRIGGER"; 
static char *g_szTriggerClose = "TRIGGERCLOSE";


BEGIN_CLASS(Door)
	ADD_VISIBLE_FLAG(1, 0)
	ADD_SOLID_FLAG(1, 0)
	ADD_BOOLPROP(BoxPhysics, DTRUE)
	ADD_LONGINTPROP(StateFlags, 0)		//  flags
	ADD_REALPROP(Speed, 0.0f)			//  movement speed
	ADD_REALPROP(MoveDelay, 0.0f)		//  Time to wait before moving
	ADD_REALPROP(MoveDist, 0.0f)		//  distance to open
	ADD_VECTORPROP(MoveDir)				//  direction to open
	ADD_VECTORPROP_VAL_FLAG(SoundPos, 0.0f, 0.0f, 0.0f, 0) //  position to place sound
	ADD_STRINGPROP(PortalName, "")		//  Portal to open/close
	ADD_STRINGPROP(OpenStartSound, "")	//  sound to play when opening starts
	ADD_STRINGPROP(OpenBusySound, "")	//  sound to play while opening
	ADD_STRINGPROP(OpenStopSound, "")	//  sound to play when done opening
	ADD_STRINGPROP(CloseStartSound, "")	//  sound to play when closing starts
	ADD_STRINGPROP(CloseBusySound, "")	//  sound to play while closing
	ADD_STRINGPROP(CloseStopSound, "")	//  sound to play when done closing
	ADD_REALPROP_FLAG(SoundRadius, 1000.0f, PF_RADIUS) //  radius of all sounds
	ADD_REALPROP(OpenWaitTime, 0.0f)	//  length of time to stay open
	ADD_REALPROP(CloseWaitTime, 0.0f)	//  length of time to stay closed
	ADD_VECTORPROP_FLAG(TriggerDims, PF_DIMS)	//  Dimensions for trigger box
	ADD_REALPROP(ClosingSpeed, 0.0f)	//  movement speed while closing
	ADD_BOOLPROP(AITriggerable, 1)		// AI's can open doors
	ADD_LONGINTPROP(Waveform, DOORWAVE_LINEAR)
	ADD_LONGINTPROP(DebrisType, DBT_METAL_BIG)
	ADD_REALPROP(Mass, 30.0f)			//  Set above 2000/10000 to crush player
END_CLASS_DEFAULT(Door, BaseClass, NULL, NULL)

DFLOAT GetDoorWaveValue( DFLOAT fSpeed, DFLOAT fPercent, DDWORD nWaveType )
{
	if (nWaveType == DOORWAVE_LINEAR)
		return fSpeed;

	DFLOAT fNewSpeed;
	DFLOAT f10Percent = fSpeed * 0.1f;

	switch ( nWaveType )
	{
		case DOORWAVE_SINE:
			fNewSpeed = fSpeed * ((DFLOAT)sin(fPercent * MATH_PI)) + f10Percent;
			break;

		case DOORWAVE_SLOWOFF:
			fNewSpeed = fSpeed * ((DFLOAT)cos(fPercent * MATH_PI/2)) + f10Percent;
			break;

		case DOORWAVE_SLOWON:
			fNewSpeed = fSpeed * ((DFLOAT)sin(fPercent * MATH_PI/2)) + f10Percent;
			break;
	}

	return fNewSpeed;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Door::Door()
//
//	PURPOSE:	Initialize object
//
// ----------------------------------------------------------------------- //

Door::Door() : BaseClass(OT_WORLDMODEL)
{
	m_hstrOpenStartSound	= DNULL;
	m_hstrOpenBusySound		= DNULL;
	m_hstrOpenStopSound		= DNULL;
	m_hstrCloseStartSound	= DNULL;
	m_hstrCloseBusySound	= DNULL;
	m_hstrCloseStopSound	= DNULL;
	m_sndLastSound			= DNULL;
	m_fSoundRadius			= 1000.0f;
	m_fMoveDelay			= 0.0f;
	m_dwStateFlags			= 0;
	m_fMoveStartTime		= 0.0f;
	m_bPlayedBusySound		= DFALSE;
	m_fDoorStopTime			= 0.0f;
	m_bAITriggerable		= DTRUE;
	m_hstrPortalName		= DNULL;
	m_bBoxPhysics			= DTRUE;

	m_hTriggerObj			= DNULL;

	VEC_INIT(m_vTriggerDims);
	VEC_INIT(m_vMoveDir);
	VEC_INIT(m_vSoundPos);

	m_dwWaveform			= DOORWAVE_LINEAR;
	m_eSurfaceType			= ST_METAL;

	m_fMass					= 30.0f;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Door::~Door()
//
//	PURPOSE:	Deallocate object
//
// ----------------------------------------------------------------------- //

Door::~Door()
{
	CServerDE* pServerDE = GetServerDE();

	if (m_hstrOpenStartSound)
	{
		pServerDE->FreeString(m_hstrOpenStartSound);
		m_hstrOpenStartSound = NULL;
	}
	if (m_hstrOpenBusySound)
	{
		pServerDE->FreeString(m_hstrOpenBusySound);
		m_hstrOpenBusySound = NULL;
	}
	if (m_hstrOpenStopSound)
	{
		pServerDE->FreeString(m_hstrOpenStopSound);
		m_hstrOpenStopSound = NULL;
	}
	if (m_hstrCloseStartSound)
	{
		pServerDE->FreeString(m_hstrCloseStartSound);
		m_hstrCloseStartSound = NULL;
	}
	if (m_hstrCloseBusySound)
	{
		pServerDE->FreeString(m_hstrCloseBusySound);
		m_hstrCloseBusySound = NULL;
	}
	if (m_hstrCloseStopSound)
	{
		pServerDE->FreeString(m_hstrCloseStopSound);
		m_hstrCloseStopSound = NULL;
	}
	if (m_hstrPortalName)
	{
		pServerDE->FreeString(m_hstrPortalName);
		m_hstrPortalName = NULL;
	}
	if (m_sndLastSound)
	{
		pServerDE->KillSound(m_sndLastSound);
	}
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	Door::ReadProp()
//
//	PURPOSE:	Reads door properties
//
// --------------------------------------------------------------------------- //

DBOOL Door::ReadProp(ObjectCreateStruct *)
{
	CServerDE* pServerDE = GetServerDE();

	long nLongVal;
	char buf[MAX_CS_FILENAME_LEN];

	pServerDE->GetPropLongInt("StateFlags", &nLongVal);
	m_dwStateFlags |= nLongVal;

	pServerDE->GetPropBool("BoxPhysics", &m_bBoxPhysics);
	pServerDE->GetPropReal("Speed", &m_fSpeed);
	pServerDE->GetPropReal("MoveDelay", &m_fMoveDelay);
	pServerDE->GetPropReal("MoveDist", &m_fMoveDist);
	pServerDE->GetPropVector("MoveDir", &m_vMoveDir);

	pServerDE->GetPropLongInt("DebrisType", &nLongVal);
	m_eSurfaceType = GetDebrisSurfaceType((DebrisType)nLongVal);

	buf[0] = '\0';
	pServerDE->GetPropString("PortalName", buf, MAX_CS_FILENAME_LEN);
	if (buf[0]) m_hstrPortalName = pServerDE->CreateString(buf);

	buf[0] = '\0';
	pServerDE->GetPropString("OpenStartSound", buf, MAX_CS_FILENAME_LEN);
	if (buf[0]) m_hstrOpenStartSound = pServerDE->CreateString(buf);

	buf[0] = '\0';
	pServerDE->GetPropString("CloseStartSound", buf, MAX_CS_FILENAME_LEN);
	if (buf[0]) m_hstrCloseStartSound = pServerDE->CreateString(buf);

	buf[0] = '\0';
	pServerDE->GetPropString("OpenBusySound", buf, MAX_CS_FILENAME_LEN);
	if (buf[0]) m_hstrOpenBusySound = pServerDE->CreateString(buf);

	buf[0] = '\0';
	pServerDE->GetPropString("OpenStopSound", buf, MAX_CS_FILENAME_LEN);
	if (buf[0]) m_hstrOpenStopSound = pServerDE->CreateString(buf);

	buf[0] = '\0';
	pServerDE->GetPropString("CloseBusySound", buf, MAX_CS_FILENAME_LEN);
	if (buf[0]) m_hstrCloseBusySound = pServerDE->CreateString(buf);

	buf[0] = '\0';
	pServerDE->GetPropString("CloseStopSound", buf, MAX_CS_FILENAME_LEN);
	if (buf[0]) m_hstrCloseStopSound = pServerDE->CreateString(buf);

	pServerDE->GetPropReal("OpenWaitTime", &m_fOpenWaitTime);
	pServerDE->GetPropReal("CloseWaitTime", &m_fCloseWaitTime);
	pServerDE->GetPropVector("TriggerDims", &m_vTriggerDims);
	pServerDE->GetPropReal("ClosingSpeed", &m_fClosingSpeed);
	pServerDE->GetPropReal("SoundRadius", &m_fSoundRadius);

	pServerDE->GetPropVector("SoundPos", &m_vSoundPos);
	pServerDE->GetPropBool("AITriggerable", &m_bAITriggerable);

	nLongVal = DOORWAVE_LINEAR;
	pServerDE->GetPropLongInt("Waveform", &nLongVal);
	m_dwWaveform = (DDWORD) nLongVal;

	pServerDE->GetPropReal("Mass", &m_fMass);

	return DTRUE;
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	Door::PostPropRead()
//
//	PURPOSE:	Initializes door data
//
// --------------------------------------------------------------------------- //

void Door::PostPropRead(ObjectCreateStruct *pStruct)
{
	SAFE_STRCPY(pStruct->m_Filename, pStruct->m_Name);
	pStruct->m_SkinName[0] = '\0';
	pStruct->m_Flags |= FLAG_FULLPOSITIONRES | FLAG_GOTHRUWORLD | FLAG_DONTFOLLOWSTANDING | (m_bBoxPhysics ? FLAG_BOXPHYSICS : 0);
	pStruct->m_fDeactivationTime = DOOR_DEFAULT_DEACTIVATION_TIME;

	m_fClosingSpeed = m_fClosingSpeed ? m_fClosingSpeed : m_fSpeed;
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	Door::TriggerHandler()
//
//	PURPOSE:	Trigger function to open and close a door
//
// --------------------------------------------------------------------------- //

void Door::TriggerHandler()
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE) return;

	if (m_dwDoorState == DOORSTATE_CLOSED || 
	    ((m_dwDoorState == DOORSTATE_CLOSING) && 
	    !(m_dwStateFlags & DF_FORCECLOSE)) )
	{
		if (pServerDE->GetTime() > m_fDoorStopTime + m_fCloseWaitTime)
		{
			SetOpening();
		}
	}
	else if (m_dwDoorState == DOORSTATE_OPEN)
	{
		if (m_dwStateFlags & DF_TRIGGERCLOSE)
		{
			if (pServerDE->GetTime() > m_fDoorStopTime + m_fOpenWaitTime)
			{
				SetClosing();
			}
		}
		else
		{
			SetOpen(DFALSE);	// Call SetOpen again to reset the door times
		}
	}
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	Door::TriggerClose()
//
//	PURPOSE:	Trigger function to only close a door (useful for stay-open-
//              forever types.
//
// --------------------------------------------------------------------------- //

void Door::TriggerClose()
{
	CServerDE* pServerDE = GetServerDE();

	if (m_dwDoorState == DOORSTATE_OPEN || m_dwDoorState == DOORSTATE_OPENING)
	{
		if (pServerDE->GetTime() > m_fDoorStopTime + m_fOpenWaitTime)
		{
			SetClosing();
		}
	}
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	Door::InitialUpdate()
//
//	PURPOSE:	Initializes door data.
//
// --------------------------------------------------------------------------- //

DBOOL Door::InitialUpdate(int nInfo)
{
	CServerDE* pServerDE = GetServerDE();

	if (nInfo == INITIALUPDATE_SAVEGAME) return DTRUE;


	DDWORD dwUsrFlgs = pServerDE->GetObjectUserFlags(m_hObject);
	pServerDE->SetObjectUserFlags(m_hObject, dwUsrFlgs | USRFLG_MOVEABLE | SurfaceToUserFlag(m_eSurfaceType));

	
	// Calculate open & close positions

	DVector vt, pos;
	
	VEC_NORM(m_vMoveDir)

	// Current position is the closed position

	pServerDE->GetObjectPos(m_hObject, &pos);
	VEC_COPY(m_vClosedPos, pos);

	// Determine the open position

	VEC_MULSCALAR(vt, m_vMoveDir, m_fMoveDist);
	VEC_ADD(m_vOpenPos, m_vClosedPos, vt);

	pServerDE->SetNextUpdate(m_hObject, DOOR_UPDATE_DELTA);

	if (m_dwStateFlags & DF_STARTOPEN)
	{
		DVector vTemp;
		VEC_COPY(vTemp, m_vOpenPos);
		VEC_COPY(m_vOpenPos, m_vClosedPos);
		VEC_COPY(m_vClosedPos, vTemp);

		VEC_MULSCALAR(m_vMoveDir, m_vMoveDir, -1.0f);

		pServerDE->MoveObject(m_hObject, &m_vClosedPos);
	}

	SetClosed(DFALSE);

	if (m_dwStateFlags & DF_SELFTRIGGER)
	{
		SpawnTrigger();
	}

	pServerDE->SetBlockingPriority(m_hObject, DOOR_DEFAULT_BLOCKING_PRIORITY);
	pServerDE->SetObjectMass(m_hObject, m_fMass);

	return DTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Door::SpawnTrigger()
//
//	PURPOSE:	Spawn a trigger object
//
// ----------------------------------------------------------------------- //

void Door::SpawnTrigger()
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE || m_hTriggerObj) return;

	DVector vPos;
	pServerDE->GetObjectPos(m_hObject, &vPos);

	ObjectCreateStruct theStruct;
	INIT_OBJECTCREATESTRUCT(theStruct);

	VEC_COPY(theStruct.m_Pos, vPos);

	HCLASS hClass = pServerDE->GetClass("Trigger");
	Trigger* pTriggerObj = (Trigger*)pServerDE->CreateObject(hClass, &theStruct);

	if (pTriggerObj)
	{
		m_hTriggerObj = pTriggerObj->m_hObject;

		DVector vDims;
		if (m_vTriggerDims.x == 0.0f && m_vTriggerDims.y == 0.0f && m_vTriggerDims.z == 0.0f)
		{
			pServerDE->GetObjectDims(m_hObject, &vDims);
			vDims.x += 1.0f;
			vDims.y += 1.0f;
			vDims.z += 1.0f;
		}
		else
		{
			VEC_COPY(vDims, m_vTriggerDims);
		}

		pTriggerObj->SetActive(DTRUE);
		pTriggerObj->SetTargetName1(pServerDE->GetObjectName(m_hObject));
		pTriggerObj->SetMessageName1(g_szTrigger);
		pTriggerObj->SetTriggerDelay(0.001f);
		pTriggerObj->SetAITriggerable(m_bAITriggerable);
		pServerDE->SetObjectDims(m_hTriggerObj, &vDims);

		// Attach trigger to door...

		DVector vOffset;
		VEC_INIT(vOffset);

		DRotation rOffset;
		ROT_INIT(rOffset);

		HATTACHMENT hAttachment;
		pServerDE->CreateAttachment(m_hObject, m_hTriggerObj, DNULL, 
									&vOffset, &rOffset, &hAttachment);

	}
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	Door::Update()
//
//	PURPOSE:	Door update function, updates the current state.
//
// --------------------------------------------------------------------------- //

DBOOL Door::Update()
{
	CServerDE* pServerDE = GetServerDE();

	pServerDE->SetNextUpdate(m_hObject, DOOR_UPDATE_DELTA);

	switch (m_dwDoorState)
	{
		case DOORSTATE_OPEN: Open(); break;
		case DOORSTATE_OPENING: Opening(); break;
		case DOORSTATE_CLOSED: Closed(); break;
		case DOORSTATE_CLOSING: Closing(); break;
	}
	return DTRUE;
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	Door::SetOpen()
//
//	PURPOSE:	Sets the door open state
//
// --------------------------------------------------------------------------- //

void Door::SetOpen(DBOOL bPlaySound)
{
	CServerDE* pServerDE = GetServerDE();

	if (bPlaySound)
    {
		StartSound(m_hstrOpenStopSound, DFALSE);
    }    

	m_dwDoorState = DOORSTATE_OPEN;

	m_fDoorStopTime = pServerDE->GetTime();

	if (m_dwStateFlags & DF_TRIGGERCLOSE)		// Trigger to close
	{
		pServerDE->SetNextUpdate(m_hObject, 0.0f);
	}
	else if (m_dwStateFlags & DF_REMAINSOPEN)
	{
		pServerDE->SetNextUpdate(m_hObject, 0.0f);
	}
	else
	{
		pServerDE->SetNextUpdate(m_hObject, m_fOpenWaitTime + 0.001f);
	}
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	Door::Open()
//
//	PURPOSE:	Handles the door open state
//
// --------------------------------------------------------------------------- //

void Door::Open()
{
	SetClosing();
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	Door::SetOpening()
//
//	PURPOSE:	Sets the door opening state
//
// --------------------------------------------------------------------------- //

void Door::SetOpening()
{
	CServerDE* pServerDE = GetServerDE();

	// Can't self-trigger door while it is opening...

	if (m_dwStateFlags & DF_SELFTRIGGER)
	{
		if (m_hTriggerObj)
		{
			Trigger* pTriggerObj = (Trigger*)pServerDE->HandleToObject(m_hTriggerObj);
			if (pTriggerObj)
			{
				pTriggerObj->SetLocked(DTRUE);
			}
		}
	}

	m_fMoveStartTime = pServerDE->GetTime();

	pServerDE->SetNextUpdate(m_hObject, UPDATE_DELTA);

	StartSound(m_hstrOpenStartSound, DFALSE);

	m_bPlayedBusySound = DFALSE;

	m_dwDoorState = DOORSTATE_OPENING;

	// Open the portal if it exists...

	if (m_hstrPortalName)
	{
		char* pName = pServerDE->GetStringData(m_hstrPortalName);
		if (pName)
		{	
			DDWORD dwFlags = 0;
			pServerDE->GetPortalFlags(pName, &dwFlags);

			dwFlags |= PORTAL_OPEN;
			pServerDE->SetPortalFlags(pName, dwFlags);
		}
	}

}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	Door::Opening()
//
//	PURPOSE:	Handles the door opening state
//
// --------------------------------------------------------------------------- //

void Door::Opening()
{
	CServerDE* pServerDE = GetServerDE();

	if (pServerDE->GetTime() < m_fMoveStartTime + m_fMoveDelay)
	{
		return;
	}
	else if (!m_bPlayedBusySound && m_hstrOpenBusySound)
	{
		m_bPlayedBusySound = DTRUE;
		StartSound(m_hstrOpenBusySound, DTRUE);
	}

	DFLOAT distTo, moveDist;
	DVector vDir, pos;

	pServerDE->GetObjectPos(m_hObject, &pos);
	VEC_SUB(vDir, m_vOpenPos, pos)
	distTo = (DFLOAT)VEC_MAG(vDir);
	VEC_NORM(vDir)

	DFLOAT fPercent = 1 - distTo / m_fMoveDist;
	DFLOAT fSpeed = GetDoorWaveValue(m_fSpeed, fPercent, m_dwWaveform);

	moveDist = fSpeed * pServerDE->GetFrameTime();

	if (moveDist > distTo)
	{
		pServerDE->MoveObject(m_hObject, &m_vOpenPos);
		SetOpen();
	}
	else
	{
		VEC_MULSCALAR(vDir, vDir, moveDist)
		VEC_ADD(pos, pos, vDir)
		pServerDE->MoveObject(m_hObject, &pos);
	}
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	Door::SetClosed()
//
//	PURPOSE:	Sets the door closed state
//
// --------------------------------------------------------------------------- //

void Door::SetClosed(DBOOL bPlaySound)
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE) return;

	if (bPlaySound)
    {
		HSTRING hstrSound = m_hstrCloseStopSound ? m_hstrCloseStopSound : m_hstrOpenStopSound;
		StartSound(hstrSound, DFALSE);
    }            

	m_fDoorStopTime = pServerDE->GetTime();

	pServerDE->SetNextUpdate(m_hObject, 0.0f);

	m_dwDoorState = DOORSTATE_CLOSED;
	
	// Okay to self-trigger door now...

	if (m_dwStateFlags & DF_SELFTRIGGER)
	{
		if (m_hTriggerObj)
		{
			Trigger* pTriggerObj = (Trigger*)pServerDE->HandleToObject(m_hTriggerObj);
			if (pTriggerObj)
			{
				pTriggerObj->SetLocked(DFALSE);
			}
		}
	}

	// Close the portal if it exists...

	if (m_hstrPortalName)
	{
		char* pName = pServerDE->GetStringData(m_hstrPortalName);
		if (pName)
		{	
			DDWORD dwFlags = 0;
			pServerDE->GetPortalFlags(pName, &dwFlags);

			dwFlags &= ~PORTAL_OPEN;
			pServerDE->SetPortalFlags(pName, dwFlags);
		}
	}
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	Door::Closed()
//
//	PURPOSE:	Sets the door closed state
//
// --------------------------------------------------------------------------- //

void Door::Closed()
{
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	Door::SetClosing()
//
//	PURPOSE:	Sets the door closing state
//
// --------------------------------------------------------------------------- //
 
void Door::SetClosing()
{
	CServerDE* pServerDE = GetServerDE();

	// If we aren't forced to close, allow self-triggerable doors to be
	// triggered...

	if (m_dwStateFlags & DF_SELFTRIGGER)
	{
		if (m_hTriggerObj && !(m_dwStateFlags & DF_FORCECLOSE))
		{
			Trigger* pTriggerObj = (Trigger*)pServerDE->HandleToObject(m_hTriggerObj);
			if (pTriggerObj)
			{
				pTriggerObj->SetLocked(DFALSE);
			}
		}
	}

	m_fMoveStartTime = pServerDE->GetTime();

	pServerDE->SetNextUpdate(m_hObject, UPDATE_DELTA);

	m_dwDoorState = DOORSTATE_CLOSING;

	HSTRING hstrSound = m_hstrCloseStartSound ? m_hstrCloseStartSound : m_hstrOpenStartSound;
	StartSound(hstrSound, DFALSE);

	m_bPlayedBusySound = DFALSE;
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	Door::Closing()
//
//	PURPOSE:	Handles the door closing state
//
// --------------------------------------------------------------------------- //

void Door::Closing()
{
	CServerDE* pServerDE = GetServerDE();

	if (pServerDE->GetTime() < m_fMoveStartTime + m_fMoveDelay)
	{
		return;
	}
	else if (!m_bPlayedBusySound)
	{
		m_bPlayedBusySound = DTRUE;
		HSTRING hstrSound = m_hstrCloseBusySound ? m_hstrCloseBusySound : m_hstrOpenBusySound;
	
		if (hstrSound)
		{
			StartSound(hstrSound, DTRUE);
		}
	}
	
	DFLOAT distTo, moveDist;
	DVector vDir, pos;

	pServerDE->GetObjectPos(m_hObject, &pos);
	VEC_SUB(vDir, m_vClosedPos, pos)
	distTo = (DFLOAT)VEC_MAG(vDir);
	VEC_NORM(vDir)

	DFLOAT fPercent = 1 - distTo / m_fMoveDist;
	DFLOAT fSpeed = GetDoorWaveValue(m_fClosingSpeed, fPercent, m_dwWaveform);

	moveDist = fSpeed * pServerDE->GetFrameTime();

	if (moveDist > distTo)
	{
		pServerDE->MoveObject(m_hObject, &m_vClosedPos);
		SetClosed();
	}
	else
	{
		VEC_MULSCALAR(vDir, vDir, moveDist)
		VEC_ADD(pos, pos, vDir)
		pServerDE->MoveObject(m_hObject, &pos);
	}
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	Door::EngineMessageFn()
//
//	PURPOSE:	Handler for engine messages
//
// --------------------------------------------------------------------------- //

DDWORD Door::EngineMessageFn(DDWORD messageID, void *pData, float fData)
{
	switch (messageID)
	{
		case MID_UPDATE:
			Update();
		break;

		case MID_PRECREATE:
		{
			// Need to call base class to have the object name read in before
			// we call PostPropRead()

			DDWORD dwRet = BaseClass::EngineMessageFn(messageID, pData, fData);

			if ((int)fData == PRECREATE_WORLDFILE)
			{
				ReadProp((ObjectCreateStruct*)pData);
			}

			if ((int)fData != PRECREATE_SAVEGAME)
			{
				PostPropRead((ObjectCreateStruct*)pData);
			}

			return dwRet;
		}
		break;

		case MID_INITIALUPDATE:
		{
			InitialUpdate((int)fData);
			CacheFiles();
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
	}

	return BaseClass::EngineMessageFn(messageID, pData, fData);
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	Door::TriggerMsg()
//
//	PURPOSE:	Handler for door trigger messages.
//
// --------------------------------------------------------------------------- //

void Door::TriggerMsg(HOBJECT hSender, HSTRING hMsg)
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE) return;

	char* pMsg = pServerDE->GetStringData(hMsg);
	if (!pMsg) return;


	// Make sure the door is active...

	pServerDE->SetDeactivationTime(m_hObject, DOOR_DEFAULT_DEACTIVATION_TIME);

	
	if (_stricmp(pMsg, g_szTrigger) == 0)
	{
		TriggerHandler();
	}
	else if (_stricmp(pMsg, g_szTriggerClose) == 0)
	{
		TriggerClose();
	}
}
	
// --------------------------------------------------------------------------- //
//
//	ROUTINE:	Door::ObjectMessageFn()
//
//	PURPOSE:	Handler for server object messages.
//
// --------------------------------------------------------------------------- //

DDWORD Door::ObjectMessageFn(HOBJECT hSender, DDWORD messageID, HMESSAGEREAD hRead)
{
	CServerDE* pServerDE = GetServerDE();
	switch (messageID)
	{
		case MID_TRIGGER:
		{
			HSTRING hMsg = pServerDE->ReadFromMessageHString(hRead);
			TriggerMsg(hSender, hMsg);
			pServerDE->FreeString(hMsg);
		}
		break;
	}
	
	return BaseClass::ObjectMessageFn(hSender, messageID, hRead);
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	Door::StartSound()
//
//	PURPOSE:	Start the specified sound
//
// --------------------------------------------------------------------------- //

void Door::StartSound(HSTRING hstrSoundName, DBOOL bLoop)
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE) return;

	// Stop the last sound if there is one...

	if (m_sndLastSound)
	{
		pServerDE->KillSound(m_sndLastSound);
		m_sndLastSound = DNULL;
	}

	if (!hstrSoundName) return;

	char *pSoundName = pServerDE->GetStringData(hstrSoundName);
	if (!pSoundName) return;


	PlaySoundInfo playSoundInfo;
	PLAYSOUNDINFO_INIT(playSoundInfo);

	playSoundInfo.m_dwFlags = PLAYSOUND_3D | PLAYSOUND_REVERB;

	
	// Determine if we should use the sound position or not...

	if (m_vSoundPos.x == 0.0f && m_vSoundPos.y == 0.0f && m_vSoundPos.z == 0.0f)
	{
		playSoundInfo.m_dwFlags |= PLAYSOUND_ATTACHED;
	}
	else
	{
		VEC_COPY(playSoundInfo.m_vPosition, m_vSoundPos);
	}


	if (bLoop)
	{
		playSoundInfo.m_dwFlags |= PLAYSOUND_LOOP | PLAYSOUND_GETHANDLE;
	}

	strncpy(playSoundInfo.m_szSoundName, pSoundName, _MAX_PATH);
	playSoundInfo.m_hObject = m_hObject;
	playSoundInfo.m_fOuterRadius = m_fSoundRadius;
	playSoundInfo.m_fInnerRadius = m_fSoundRadius * 0.5f;

	pServerDE->PlaySound(&playSoundInfo);


	// Save the handle of the sound...

	if (bLoop)
	{
		m_sndLastSound = playSoundInfo.m_hSound;
	}
	else
	{
		m_sndLastSound = DNULL;
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Door::Save
//
//	PURPOSE:	Save the object
//
// ----------------------------------------------------------------------- //

void Door::Save(HMESSAGEWRITE hWrite, DDWORD dwSaveFlags)
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE || !hWrite) return;
	
	pServerDE->WriteToLoadSaveMessageObject(hWrite, m_hTriggerObj);

	pServerDE->WriteToMessageVector(hWrite, &m_vMoveDir);
	pServerDE->WriteToMessageVector(hWrite, &m_vSoundPos);
	pServerDE->WriteToMessageVector(hWrite, &m_vTriggerDims);
	pServerDE->WriteToMessageVector(hWrite, &m_vOpenPos);
	pServerDE->WriteToMessageVector(hWrite, &m_vClosedPos);
	pServerDE->WriteToMessageFloat(hWrite, m_fSpeed);
	pServerDE->WriteToMessageFloat(hWrite, m_fMoveDist);
	pServerDE->WriteToMessageFloat(hWrite, m_fOpenWaitTime);
	pServerDE->WriteToMessageFloat(hWrite, m_fCloseWaitTime);
	pServerDE->WriteToMessageFloat(hWrite, m_fClosingSpeed);
	pServerDE->WriteToMessageFloat(hWrite, m_fMoveStartTime);
	pServerDE->WriteToMessageFloat(hWrite, m_fMoveDelay);
	pServerDE->WriteToMessageFloat(hWrite, m_fDoorStopTime);
	pServerDE->WriteToMessageFloat(hWrite, m_fSoundRadius);
	pServerDE->WriteToMessageByte(hWrite, m_bBoxPhysics);
	pServerDE->WriteToMessageByte(hWrite, m_bAITriggerable);
	pServerDE->WriteToMessageByte(hWrite, m_bPlayedBusySound);
	pServerDE->WriteToMessageDWord(hWrite, m_dwStateFlags);
	pServerDE->WriteToMessageDWord(hWrite, m_dwDoorState);
	pServerDE->WriteToMessageDWord(hWrite, m_dwWaveform);
	pServerDE->WriteToMessageHString(hWrite, m_hstrOpenStartSound);
	pServerDE->WriteToMessageHString(hWrite, m_hstrCloseStartSound);
	pServerDE->WriteToMessageHString(hWrite, m_hstrOpenBusySound);
	pServerDE->WriteToMessageHString(hWrite, m_hstrOpenStopSound);
	pServerDE->WriteToMessageHString(hWrite, m_hstrCloseBusySound);
	pServerDE->WriteToMessageHString(hWrite, m_hstrCloseStopSound);
	pServerDE->WriteToMessageHString(hWrite, m_hstrPortalName);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Door::Load
//
//	PURPOSE:	Load the object
//
// ----------------------------------------------------------------------- //

void Door::Load(HMESSAGEREAD hRead, DDWORD dwLoadFlags)
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE || !hRead) return;

	pServerDE->ReadFromLoadSaveMessageObject(hRead, &m_hTriggerObj);

	pServerDE->ReadFromMessageVector(hRead, &m_vMoveDir);
	pServerDE->ReadFromMessageVector(hRead, &m_vSoundPos);
	pServerDE->ReadFromMessageVector(hRead, &m_vTriggerDims);
	pServerDE->ReadFromMessageVector(hRead, &m_vOpenPos);
	pServerDE->ReadFromMessageVector(hRead, &m_vClosedPos);
	m_fSpeed				= pServerDE->ReadFromMessageFloat(hRead);
	m_fMoveDist				= pServerDE->ReadFromMessageFloat(hRead);
	m_fOpenWaitTime			= pServerDE->ReadFromMessageFloat(hRead);
	m_fCloseWaitTime		= pServerDE->ReadFromMessageFloat(hRead);
	m_fClosingSpeed			= pServerDE->ReadFromMessageFloat(hRead);
	m_fMoveStartTime		= pServerDE->ReadFromMessageFloat(hRead);
	m_fMoveDelay			= pServerDE->ReadFromMessageFloat(hRead);
	m_fDoorStopTime			= pServerDE->ReadFromMessageFloat(hRead);
	m_fSoundRadius			= pServerDE->ReadFromMessageFloat(hRead);
	m_bBoxPhysics			= (DBOOL) pServerDE->ReadFromMessageByte(hRead);
	m_bAITriggerable		= (DBOOL) pServerDE->ReadFromMessageByte(hRead);
	m_bPlayedBusySound		= (DBOOL) pServerDE->ReadFromMessageByte(hRead);
	m_dwStateFlags			= pServerDE->ReadFromMessageDWord(hRead);
	m_dwDoorState			= pServerDE->ReadFromMessageDWord(hRead);
	m_dwWaveform			= pServerDE->ReadFromMessageDWord(hRead);
	m_hstrOpenStartSound	= pServerDE->ReadFromMessageHString(hRead);
	m_hstrCloseStartSound	= pServerDE->ReadFromMessageHString(hRead);
	m_hstrOpenBusySound		= pServerDE->ReadFromMessageHString(hRead);
	m_hstrOpenStopSound		= pServerDE->ReadFromMessageHString(hRead);
	m_hstrCloseBusySound	= pServerDE->ReadFromMessageHString(hRead);
	m_hstrCloseStopSound	= pServerDE->ReadFromMessageHString(hRead);
	m_hstrPortalName		= pServerDE->ReadFromMessageHString(hRead);


	// If opening or closing sounds were being played, make sure we play them...

	if (m_dwDoorState == DOORSTATE_OPENING)
	{
		if (m_bPlayedBusySound && m_hstrOpenBusySound)
		{
			StartSound(m_hstrOpenBusySound, DTRUE);
		}
	}
	else if (m_dwDoorState == DOORSTATE_CLOSING)
	{
		if (m_bPlayedBusySound)
		{
			HSTRING hstrSound = m_hstrCloseBusySound ? m_hstrCloseBusySound : m_hstrOpenBusySound;
	
			if (hstrSound)
			{
				StartSound(hstrSound, DTRUE);
			}
		}
	}

}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Door::CacheFiles
//
//	PURPOSE:	Cache resources used by this object
//
// ----------------------------------------------------------------------- //

void Door::CacheFiles()
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE) return;

	char* pFile = DNULL;
	if (m_hstrOpenStartSound)
	{
		pFile = pServerDE->GetStringData(m_hstrOpenStartSound);
		if (pFile)
		{
			pServerDE->CacheFile(FT_SOUND, pFile);
		}
	}

	if (m_hstrCloseStartSound)
	{
		pFile = pServerDE->GetStringData(m_hstrCloseStartSound);
		if (pFile)
		{
			pServerDE->CacheFile(FT_SOUND, pFile);
		}
	}

	if (m_hstrOpenBusySound)
	{
		pFile = pServerDE->GetStringData(m_hstrOpenBusySound);
		if (pFile)
		{
			pServerDE->CacheFile(FT_SOUND, pFile);
		}
	}

	if (m_hstrOpenStopSound)
	{
		pFile = pServerDE->GetStringData(m_hstrOpenStopSound);
		if (pFile)
		{
			pServerDE->CacheFile(FT_SOUND, pFile);
		}
	}

	if (m_hstrCloseBusySound)
	{
		pFile = pServerDE->GetStringData(m_hstrCloseBusySound);
		if (pFile)
		{
			pServerDE->CacheFile(FT_SOUND, pFile);
		}
	}

	if (m_hstrCloseStopSound)
	{
		pFile = pServerDE->GetStringData(m_hstrCloseStopSound);
		if (pFile)
		{
			pServerDE->CacheFile(FT_SOUND, pFile);
		}
	}
}


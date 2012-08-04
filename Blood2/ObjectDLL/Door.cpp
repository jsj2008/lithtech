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
#include "ClientServerShared.h"
#include "ObjectUtilities.h"
#include "SharedDefs.h"
#include <mbstring.h>
#include "SoundTypes.h"



#define	DOOR_DEFAULT_BLOCKING_PRIORITY	100
#define	DOOR_DEFAULT_DEACTIVATION_TIME	30.0f


// Static global variables..
static char *g_szTrigger		= "TRIGGER"; 
static char *g_szTriggerClose	= "TRIGGERCLOSE";
static char *g_szLock			= "LOCK"; 
static char *g_szUnLock			= "UNLOCK";


BEGIN_CLASS(Door)
	ADD_VISIBLE_FLAG(1, 0)
	ADD_SOLID_FLAG(1, 0)
	ADD_REALPROP(Speed, 0.0f)			//  movement speed
	ADD_REALPROP(MoveDist, 0.0f)		//  distance to open
	ADD_VECTORPROP(MoveDir)				//  direction to open
	ADD_STRINGPROP(OpenBusySound, "")	//  sound to play while opening
	ADD_STRINGPROP(OpenStopSound, "")	//  sound to play when done opening
	ADD_STRINGPROP(CloseBusySound, "")	//  sound to play while closing
	ADD_STRINGPROP(CloseStopSound, "")	//  sound to play when done closing
	ADD_REALPROP_FLAG(SoundRadius, 1000.0f, PF_RADIUS) //  radius of all sounds
	ADD_REALPROP(WaitTime, 0.0f)		//  length of time to stay open
	ADD_REALPROP(OpenDelay, 0.0f)		// Length of time to wait before opening
	ADD_VECTORPROP_FLAG(TriggerDims, PF_DIMS)	//  Dimensions for trigger box
	ADD_REALPROP(ClosingSpeed, 0.0f)	//  movement speed while closing
	ADD_STRINGPROP(PortalName, "")		// A Portal brush name
	ADD_LONGINTPROP(Waveform, DOORWAVE_LINEAR)
	ADD_BOOLPROP(BoxPhysics, DTRUE)		// Door uses "box physics"
	ADD_BOOLPROP(TriggerClosed, DFALSE)	// Door must be triggered to close
	ADD_BOOLPROP(StartOpen, DFALSE)		// Door starts in open position
	ADD_BOOLPROP(RemainsOpen, DFALSE)	// One-time door, remains open after opening

	ADD_BOOLPROP(SelfTrigger, DFALSE)	// Door creates it's own trigger
	ADD_BOOLPROP(FireThrough, DFALSE)	// Can shoot through the door.
	PROP_DEFINEGROUP(SelfTriggerOpts, PF_GROUP1) 
		ADD_BOOLPROP_FLAG(TouchActivate, DFALSE, PF_GROUP1)
		ADD_BOOLPROP_FLAG(PlayerActivate, DTRUE, PF_GROUP1)
		ADD_BOOLPROP_FLAG(AIActivate, DTRUE, PF_GROUP1)
		ADD_BOOLPROP_FLAG(ObjectActivate, DFALSE, PF_GROUP1)
		ADD_BOOLPROP_FLAG(TriggerRelayActivate, DTRUE, PF_GROUP1)
		ADD_BOOLPROP_FLAG(NamedObjectActivate, DFALSE, PF_GROUP1)
		ADD_STRINGPROP_FLAG(ActivationObjectName, "", PF_GROUP1)
		ADD_BOOLPROP_FLAG(Locked, DFALSE, PF_GROUP1)
		ADD_STRINGPROP_FLAG(LockedSound, "", PF_GROUP1)
		ADD_STRINGPROP_FLAG(UnlockedSound, "", PF_GROUP1)
		ADD_STRINGPROP_FLAG(UnlockKeyName, "Key", PF_GROUP1)

END_CLASS_DEFAULT(Door, B2BaseClass, NULL, NULL)


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Door::Door()
//
//	PURPOSE:	Initialize object
//
// ----------------------------------------------------------------------- //

Door::Door() : B2BaseClass(OT_WORLDMODEL)
{
	m_hstrOpenBusySound  = DNULL;
	m_hstrOpenStopSound  = DNULL;
	m_hstrCloseBusySound = DNULL;
	m_hstrCloseStopSound = DNULL;
	m_hstrPortal		 = DNULL;
	m_sndLastSound		 = DNULL;
	m_fSoundRadius		 = 1000.0f;

	m_dwWaveform		 = DOORWAVE_LINEAR;
	m_dwTriggerFlags	 = 0;
	m_bBoxPhysics		 = 0;

	m_hTriggerObj		 = DNULL;
	m_fOpenDelay		 = 0.0f;

	m_bTrigTouchActivate	= DFALSE;		// Trigger can be activated with TouchNotify
	m_bTrigPlayerActivate	= DTRUE;		// Can be triggered by player
	m_bTrigAIActivate		= DTRUE;		// Can be triggered by AI
	m_bTrigObjectActivate	= DFALSE;		// Can be triggered by another object
	m_bTrigTriggerRelayActivate	= DTRUE;	// Can be triggered by another trigger
	m_bTrigNamedObjectActivate	= DFALSE;	// Can it only be triggered by a specific object?
	m_hstrTrigActivationObjectName = DNULL;
	m_bTrigLocked				= DFALSE;		// Trigger is locked.
	m_hstrTrigLockedSound		= DNULL;		// Message to display when trigger is locked.
	m_hstrTrigUnlockedSound		= DNULL;		// Message to display when trigger is locked.
	m_hstrTrigKeyName			= DNULL;		// Name of key item needed to open the trigger.
	m_bFireThrough			= DFALSE;

	m_bDoorBlocked			= DFALSE;
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
	if (m_hstrPortal)
	{
		pServerDE->FreeString(m_hstrPortal);
		m_hstrPortal = NULL;
	}
	if (m_hstrTrigActivationObjectName)
	{
		pServerDE->FreeString(m_hstrTrigActivationObjectName);
		m_hstrTrigActivationObjectName = NULL;
	}
	if (m_hstrTrigLockedSound)
	{
		pServerDE->FreeString(m_hstrTrigLockedSound);
		m_hstrTrigLockedSound = NULL;
	}
	if (m_hstrTrigUnlockedSound)
	{
		pServerDE->FreeString(m_hstrTrigUnlockedSound);
		m_hstrTrigUnlockedSound = NULL;
	}
	if (m_hstrTrigKeyName)
	{
		pServerDE->FreeString(m_hstrTrigKeyName);
		m_hstrTrigKeyName = NULL;
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

	GenericProp genProp;

	if (pServerDE->GetPropGeneric("Speed", &genProp) == DE_OK)
		m_fSpeed = genProp.m_Float;

	if (pServerDE->GetPropGeneric("ClosingSpeed", &genProp) == DE_OK)
		m_fClosingSpeed = genProp.m_Float;

	if (pServerDE->GetPropGeneric("MoveDist", &genProp) == DE_OK)
		m_fMoveDist = genProp.m_Float;

	if (pServerDE->GetPropGeneric("MoveDir", &genProp) == DE_OK)
		VEC_COPY(m_vMoveDir, genProp.m_Vec);

	if (pServerDE->GetPropGeneric("OpenBusySound", &genProp) == DE_OK)
	{
		if (genProp.m_String[0])
			m_hstrOpenBusySound = pServerDE->CreateString(genProp.m_String);
	}

	if (pServerDE->GetPropGeneric("OpenStopSound", &genProp) == DE_OK)
	{
		if (genProp.m_String[0])
			m_hstrOpenStopSound = pServerDE->CreateString(genProp.m_String);
	}

	if (pServerDE->GetPropGeneric("CloseBusySound", &genProp) == DE_OK)
	{
		if (genProp.m_String[0])
			m_hstrCloseBusySound = pServerDE->CreateString(genProp.m_String);
	}

	if (pServerDE->GetPropGeneric("CloseStopSound", &genProp) == DE_OK)
	{
		if (genProp.m_String[0])
			m_hstrCloseStopSound = pServerDE->CreateString(genProp.m_String);
	}

	if (pServerDE->GetPropGeneric("PortalName", &genProp) == DE_OK)
	{
		if (genProp.m_String[0])
			m_hstrPortal = pServerDE->CreateString(genProp.m_String);
	}

	if (pServerDE->GetPropGeneric("WaitTime", &genProp) == DE_OK)
		m_fWaitTime = genProp.m_Float;

	if (pServerDE->GetPropGeneric("OpenDelay", &genProp) == DE_OK)
	{
		m_fOpenDelay = genProp.m_Float;
	}

	if (pServerDE->GetPropGeneric("SoundRadius", &genProp) == DE_OK)
		m_fSoundRadius = genProp.m_Float;

	if (pServerDE->GetPropGeneric("TriggerDims", &genProp) == DE_OK)
		VEC_COPY(m_vTriggerDims, genProp.m_Vec);

	if (pServerDE->GetPropGeneric("Waveform", &genProp) == DE_OK)
		m_dwWaveform = genProp.m_Long;

	if (pServerDE->GetPropGeneric("BoxPhysics", &genProp) == DE_OK)
	{
		m_bBoxPhysics = genProp.m_Bool;
	}

	if (pServerDE->GetPropGeneric("SelfTrigger", &genProp) == DE_OK)
	{
		m_dwTriggerFlags |= genProp.m_Bool ? DTF_SELFTRIGGER : 0;
	}

	if (pServerDE->GetPropGeneric("TriggerClosed", &genProp) == DE_OK)
	{
		m_dwTriggerFlags |= genProp.m_Bool ? DTF_TRIGGERCLOSED : 0;
	}

	if (pServerDE->GetPropGeneric("StartOpen", &genProp) == DE_OK)
	{
		m_dwTriggerFlags |= genProp.m_Bool ? DTF_STARTOPEN : 0;
	}

	if (pServerDE->GetPropGeneric("RemainsOpen", &genProp) == DE_OK)
	{
		m_dwTriggerFlags |= genProp.m_Bool ? DTF_REMAINSOPEN : 0;
	}

	// Self-create trigger options
	if (pServerDE->GetPropGeneric("TouchActivate", &genProp) == DE_OK)
	{
		m_bTrigTouchActivate = genProp.m_Bool;
	}

	if (pServerDE->GetPropGeneric("PlayerActivate", &genProp) == DE_OK)
	{
		m_bTrigPlayerActivate = genProp.m_Bool;
	}

	if (pServerDE->GetPropGeneric("AIActivate", &genProp) == DE_OK)
	{
		m_bTrigAIActivate = genProp.m_Bool;
	}

	if (pServerDE->GetPropGeneric("TriggerRelayActivate", &genProp) == DE_OK)
	{
		m_bTrigTriggerRelayActivate = genProp.m_Bool;
	}

	if (pServerDE->GetPropGeneric("ObjectActivate", &genProp) == DE_OK)
	{
		m_bTrigObjectActivate = genProp.m_Bool;
	}

	if (pServerDE->GetPropGeneric("NamedObjectActivate", &genProp) == DE_OK)
	{
		m_bTrigNamedObjectActivate = genProp.m_Bool;
	}

	if (g_pServerDE->GetPropGeneric("ActivationObjectName", &genProp) == DE_OK)
	{
		if (genProp.m_String[0])
			 m_hstrTrigActivationObjectName = g_pServerDE->CreateString(genProp.m_String);
	}

	if (g_pServerDE->GetPropGeneric("Locked", &genProp) == DE_OK)
		m_bTrigLocked = genProp.m_Bool;

	if (g_pServerDE->GetPropGeneric("LockedSound", &genProp) == DE_OK)
	{
		if (genProp.m_String[0])
			 m_hstrTrigLockedSound = g_pServerDE->CreateString(genProp.m_String);
	}

	if (g_pServerDE->GetPropGeneric("UnlockedSound", &genProp) == DE_OK)
	{
		if (genProp.m_String[0])
			 m_hstrTrigUnlockedSound = g_pServerDE->CreateString(genProp.m_String);
	}

	if (g_pServerDE->GetPropGeneric("UnlockKeyName", &genProp) == DE_OK)
	{
		if (genProp.m_String[0])
			 m_hstrTrigKeyName = g_pServerDE->CreateString(genProp.m_String);
	}

	if (pServerDE->GetPropGeneric("FireThrough", &genProp) == DE_OK)
	{
		m_bFireThrough = genProp.m_Bool;
	}

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
	_mbscpy((unsigned char*)pStruct->m_Filename, (const unsigned char*)pStruct->m_Name);
	pStruct->m_SkinName[0] = '\0';
	pStruct->m_Flags |= FLAG_GOTHRUWORLD | FLAG_FULLPOSITIONRES | FLAG_MODELGOURAUDSHADE;
	pStruct->m_Flags |= FLAG_FULLPOSITIONRES | FLAG_GOTHRUWORLD | FLAG_DONTFOLLOWSTANDING | (m_bBoxPhysics ? FLAG_BOXPHYSICS : 0);
	pStruct->m_fDeactivationTime = DOOR_DEFAULT_DEACTIVATION_TIME;

	if (m_fClosingSpeed == 0.0f)
		m_fClosingSpeed = m_fSpeed;

	// Set the move delay to a minimal value.
	if (m_fOpenDelay == 0.0f)
		m_fOpenDelay = 0.01f;
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
	if (m_dwDoorState == DOORSTATE_CLOSED || m_dwDoorState == DOORSTATE_CLOSING)
	{
		SetOpening();
	}
	else if (m_dwDoorState == DOORSTATE_OPEN)
	{
		if (m_dwTriggerFlags & DTF_TRIGGERCLOSED)
			SetClosing();
		else
			SetOpen(DFALSE);		// Call SetOpen again to reset the door times
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
	if (m_dwDoorState == DOORSTATE_OPEN || m_dwDoorState == DOORSTATE_OPENING)
	{
		SetClosing();
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

	if (nInfo == INITIALUPDATE_SAVEGAME)
		return DTRUE;

	if (m_dwTriggerFlags & DTF_SELFTRIGGER)
	{
		SpawnTrigger();
	}

	SetClosed(DFALSE);

	// Calculate open & close positions
	DVector vt, pos;
	
	VEC_NORM(m_vMoveDir)

	// Current position is the closed position
	pServerDE->GetObjectPos(m_hObject, &pos);
	VEC_COPY(m_vClosedPos, pos);

	// Determine the open position
	VEC_MULSCALAR(vt, m_vMoveDir, m_fMoveDist);
	VEC_ADD(m_vOpenPos, m_vClosedPos, vt);

	// If start open, simply swap open & closed positions
	if (m_dwTriggerFlags & DTF_STARTOPEN)
	{
		VEC_COPY(vt, m_vOpenPos);
		VEC_COPY(m_vOpenPos, m_vClosedPos);
		VEC_COPY(m_vClosedPos, vt);

		VEC_MULSCALAR(m_vMoveDir, m_vMoveDir, -1.0f);

		pServerDE->MoveObject(m_hObject, &m_vClosedPos);
	}

	pServerDE->SetBlockingPriority(m_hObject, DOOR_DEFAULT_BLOCKING_PRIORITY);

	// Initially closed
	SetClosed(DFALSE);
	// Set the portal state if there is one
//	SetPortalState(DFALSE);
	pServerDE->SetNextUpdate(m_hObject, (DFLOAT).01);

	// Mark this object as savable
	DDWORD dwFlags = pServerDE->GetObjectUserFlags(m_hObject);
	dwFlags |= USRFLG_SAVEABLE;
	pServerDE->SetObjectUserFlags(m_hObject, dwFlags);

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

//		pTriggerObj->SetActive(DTRUE);
//		pTriggerObj->SetTargetName(0, pServerDE->GetObjectName(m_hObject));
//		pTriggerObj->SetMessageName(0, g_szTrigger);
		pServerDE->SetObjectDims(pTriggerObj->m_hObject, &vDims);
		m_hTriggerObj = pTriggerObj->m_hObject;

		// New:  Set up trigger properties
		pTriggerObj->Setup(DTRUE, 
						   pServerDE->GetObjectName(m_hObject),
						   g_szTrigger,
						   m_bTrigTouchActivate,
						   m_bTrigPlayerActivate,
						   m_bTrigAIActivate,
						   m_bTrigObjectActivate,
						   m_bTrigTriggerRelayActivate,
						   m_bTrigNamedObjectActivate,
						   m_hstrTrigActivationObjectName,
						   m_bTrigLocked,
						   DNULL,
						   m_hstrTrigLockedSound,
						   DNULL,
						   m_hstrTrigUnlockedSound,
						   m_hstrTrigKeyName);
	}
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	Door::Update()
//
//	PURPOSE:	Door update function, updates the current state.
//
// --------------------------------------------------------------------------- //

DBOOL Door::Update(DVector *pMovement)
{
	CServerDE* pServerDE = GetServerDE();

	pServerDE->SetNextUpdate(m_hObject, (DFLOAT).01);

	switch (m_dwDoorState)
	{
		case DOORSTATE_OPEN: Open(); break;
		case DOORSTATE_OPENING: Opening(); break;
		case DOORSTATE_CLOSED: Closed(); break;
		case DOORSTATE_CLOSING: Closing(); break;
	}

	// Reset blocked state
	m_bDoorBlocked = DFALSE;

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
	if (m_dwTriggerFlags & DTF_TRIGGERCLOSED)
		pServerDE->SetNextUpdate(m_hObject, (DFLOAT)0);
	else if (m_dwTriggerFlags & DTF_REMAINSOPEN)
		pServerDE->SetNextUpdate(m_hObject, (DFLOAT)0);
	else
		m_fDoorOpenTime = m_fWaitTime;
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
	CServerDE* pServerDE = GetServerDE();

	// Changed to count down instead of relying on GetTime GK 4/17/98
	DFLOAT fTime = pServerDE->GetFrameTime();
	m_fDoorOpenTime -= fTime;

	if (m_fDoorOpenTime <= 0)
	{
		m_fDoorOpenTime = 0;
		SetClosing();
	}
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

	if (m_hTriggerObj)
	{
		Trigger* pTriggerObj = (Trigger*)pServerDE->HandleToObject(m_hTriggerObj);
		if (pTriggerObj)
		{
			pTriggerObj->SetActive(DFALSE);
		}
	}

	pServerDE->SetNextUpdate(m_hObject, m_fOpenDelay);

	m_bStartedMoving = DFALSE;

	m_dwDoorState = DOORSTATE_OPENING;
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

	DFLOAT distTo, moveDist;
	DVector vDir, pos;

	if (!m_bStartedMoving)
	{
		m_bStartedMoving = DTRUE;
		StartSound(m_hstrOpenBusySound, DTRUE);
		SetPortalState(DTRUE);
	}

	pServerDE->GetObjectPos(m_hObject, &pos);
	VEC_SUB(vDir, m_vOpenPos, pos)
	distTo = (DFLOAT)VEC_MAG(vDir);
	VEC_NORM(vDir)

	DFLOAT fPercent = 1 - distTo / m_fMoveDist;
	DFLOAT fSpeed = GetWaveValue(m_fSpeed, fPercent, m_dwWaveform);

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
		StartSound(m_hstrCloseStopSound, DFALSE);
    }            

	pServerDE->SetNextUpdate(m_hObject, (DFLOAT)0);
	m_dwDoorState = DOORSTATE_CLOSED;
	if (m_dwTriggerFlags & DTF_SELFTRIGGER)
	{
		Trigger* pTriggerObj = (Trigger*)pServerDE->HandleToObject(m_hTriggerObj);
		if (pTriggerObj)
		{
			pTriggerObj->SetActive(DTRUE);
		}
	}

	SetPortalState(DFALSE);
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

	pServerDE->SetNextUpdate(m_hObject, (DFLOAT).01);
	m_dwDoorState = DOORSTATE_CLOSING;

	StartSound(m_hstrCloseBusySound, DTRUE);
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
	DFLOAT distTo, moveDist;
	DVector vDir, pos;

	pServerDE->GetObjectPos(m_hObject, &pos);
	VEC_SUB(vDir, m_vClosedPos, pos)
	distTo = (DFLOAT)VEC_MAG(vDir);
	VEC_NORM(vDir)

	DFLOAT fPercent = 1 - distTo / m_fMoveDist;
	DFLOAT fSpeed = GetWaveValue(m_fClosingSpeed, fPercent, m_dwWaveform);

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
			Update((DVector*)pData);
			break;

		case MID_PRECREATE:
		{
			// Need to call base class to have the object name read in before
			// we call PostPropRead()

			DDWORD dwRet = B2BaseClass::EngineMessageFn(messageID, pData, fData);

			if (fData == PRECREATE_WORLDFILE)
			{
				ReadProp((ObjectCreateStruct*)pData);
			}

			PostPropRead((ObjectCreateStruct*)pData);

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

	return B2BaseClass::EngineMessageFn(messageID, pData, fData);
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
	char* pMsg = pServerDE->GetStringData(hMsg);
	if (!pMsg) return;

	// Make sure the door is active...

	pServerDE->SetDeactivationTime(m_hObject, DOOR_DEFAULT_DEACTIVATION_TIME);

	if (_mbsicmp((const unsigned char*)pMsg, (const unsigned char*)g_szTrigger) == 0)
	{
		TriggerHandler();
	}
	else if (_mbsicmp((const unsigned char*)pMsg, (const unsigned char*)g_szTriggerClose) == 0)
	{
		TriggerClose();
	}
	// If it's a lock or unlock, relay it to our trigger object
	else if (m_hTriggerObj && ((_mbsicmp((const unsigned char*)pMsg, (const unsigned char*)g_szLock) == 0) || (_mbsicmp((const unsigned char*)pMsg, (const unsigned char*)g_szUnLock) == 0)))
	{
		HMESSAGEWRITE hMessage = pServerDE->StartMessageToObject(g_pServerDE->HandleToObject(hSender), m_hTriggerObj, MID_TRIGGER);
		pServerDE->WriteToMessageHString(hMessage, hMsg);
		pServerDE->EndMessage(hMessage);
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

		case MID_DOORBLOCK:
		{
			m_bDoorBlocked = DTRUE;
		}
		break;
	}
	
	return B2BaseClass::ObjectMessageFn(hSender, messageID, hRead);
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

	playSoundInfo.m_dwFlags = PLAYSOUND_3D | PLAYSOUND_ATTACHED | PLAYSOUND_REVERB;
	if (bLoop)
	{
		playSoundInfo.m_dwFlags |= PLAYSOUND_LOOP | PLAYSOUND_GETHANDLE;
	}

	_mbsncpy((unsigned char*)playSoundInfo.m_szSoundName, (const unsigned char*)pSoundName, _MAX_PATH );
	playSoundInfo.m_hObject = m_hObject;
	playSoundInfo.m_fOuterRadius = 1500;
	playSoundInfo.m_fInnerRadius = 500;
	playSoundInfo.m_nVolume = 100;
	playSoundInfo.m_nPriority = SOUNDPRIORITY_MISC_MEDIUM;

	pServerDE->PlaySound(&playSoundInfo);

	// Save the handle of the sound...

	if (bLoop)
		m_sndLastSound = playSoundInfo.m_hSound;
}



// --------------------------------------------------------------------------- //
//
//	ROUTINE:	Door::SetPortalState()
//
//	PURPOSE:	Sets the state of the portal, if any
//
// --------------------------------------------------------------------------- //

void Door::SetPortalState(DBOOL bOpen)
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE) return;

	if (m_hstrPortal)
	{
		DDWORD dwPortalFlags = 0;
		DRESULT res;
		if ((res = pServerDE->GetPortalFlags(pServerDE->GetStringData(m_hstrPortal), &dwPortalFlags)) == LT_OK)
		{
			if (bOpen)
				dwPortalFlags |= PORTAL_OPEN;
			else
				dwPortalFlags &= ~PORTAL_OPEN;

			pServerDE->SetPortalFlags(pServerDE->GetStringData(m_hstrPortal), dwPortalFlags);
		}
	}
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Door::GetWaveValue()
//
//	PURPOSE:	Given a speed value and a percentag
//
// ----------------------------------------------------------------------- //

DFLOAT Door::GetWaveValue( DFLOAT fSpeed, DFLOAT fPercentOpen, DDWORD dwWaveType )
{
	if (dwWaveType == DOORWAVE_LINEAR)
		return fSpeed;

	DFLOAT fNewSpeed;
	DFLOAT f10Percent = fSpeed * 0.1f;

	switch ( dwWaveType )
	{
		case DOORWAVE_SINE:
			fNewSpeed = fSpeed * ((DFLOAT)sin(fPercentOpen * PI)) + f10Percent;
			break;

		case DOORWAVE_SLOWOFF:
			fNewSpeed = fSpeed * ((DFLOAT)cos(fPercentOpen * PI/2)) + f10Percent;
			break;

		case DOORWAVE_SLOWON:
			fNewSpeed = fSpeed * ((DFLOAT)sin(fPercentOpen * PI/2)) + f10Percent;
			break;
	}

	return fNewSpeed;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Door::CacheFiles
//
//	PURPOSE:	Cache resources used by the object
//
// ----------------------------------------------------------------------- //

void Door::CacheFiles()
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE) return;

	// {MD 9/23/98}
	if(!(pServerDE->GetServerFlags() & SS_CACHING))
		return;

	char* pFile = DNULL;

	if (m_hstrOpenBusySound)
	{
		pFile = g_pServerDE->GetStringData(m_hstrOpenBusySound);
		if (pFile)
		{
			 g_pServerDE->CacheFile(FT_SOUND ,pFile);
		}
	}

	if (m_hstrOpenStopSound)
	{
		pFile = g_pServerDE->GetStringData(m_hstrOpenStopSound);
		if (pFile)
		{
			 g_pServerDE->CacheFile(FT_SOUND ,pFile);
		}
	}

	if (m_hstrCloseBusySound)
	{
		pFile = g_pServerDE->GetStringData(m_hstrCloseBusySound);
		if (pFile)
		{
			 g_pServerDE->CacheFile(FT_SOUND ,pFile);
		}
	}

	if (m_hstrCloseStopSound)
	{
		pFile = g_pServerDE->GetStringData(m_hstrCloseStopSound);
		if (pFile)
		{
			 g_pServerDE->CacheFile(FT_SOUND ,pFile);
		}
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
	pServerDE->WriteToMessageDWord(hWrite, m_dwTriggerFlags);
	pServerDE->WriteToMessageFloat(hWrite, m_fSpeed);
	pServerDE->WriteToMessageFloat(hWrite, m_fMoveDist);
	pServerDE->WriteToMessageVector(hWrite, &m_vMoveDir);
	pServerDE->WriteToMessageHString(hWrite, m_hstrOpenBusySound);
	pServerDE->WriteToMessageHString(hWrite, m_hstrOpenStopSound);
	pServerDE->WriteToMessageHString(hWrite, m_hstrCloseBusySound);
	pServerDE->WriteToMessageHString(hWrite, m_hstrCloseStopSound);
	pServerDE->WriteToMessageFloat(hWrite, m_fWaitTime);
	pServerDE->WriteToMessageVector(hWrite, &m_vTriggerDims);
	pServerDE->WriteToMessageFloat(hWrite, m_fClosingSpeed);

	pServerDE->WriteToMessageByte(hWrite, (DBYTE)m_bLocked);
	pServerDE->WriteToMessageByte(hWrite, (DBYTE)m_bBoxPhysics);
	pServerDE->WriteToMessageVector(hWrite, &m_vOpenPos);
	pServerDE->WriteToMessageVector(hWrite, &m_vClosedPos);
	pServerDE->WriteToMessageDWord(hWrite, m_dwDoorState);
	pServerDE->WriteToMessageFloat(hWrite, m_fDoorOpenTime);

	pServerDE->WriteToMessageFloat(hWrite, m_fSoundRadius);

	pServerDE->WriteToMessageHString(hWrite, m_hstrPortal);
	pServerDE->WriteToMessageDWord(hWrite, m_dwFlags);
	pServerDE->WriteToMessageDWord(hWrite, m_dwWaveform);

	pServerDE->WriteToMessageByte(hWrite, (DBYTE)m_bStartedMoving);
	pServerDE->WriteToMessageFloat(hWrite, m_fOpenDelay);
	pServerDE->WriteToMessageByte(hWrite, (DBYTE)m_bFireThrough);
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
	m_dwTriggerFlags		= pServerDE->ReadFromMessageDWord(hRead); 
	m_fSpeed				= pServerDE->ReadFromMessageFloat(hRead); 
	m_fMoveDist				= pServerDE->ReadFromMessageFloat(hRead); 
	pServerDE->ReadFromMessageVector(hRead, &m_vMoveDir);
	m_hstrOpenBusySound		= pServerDE->ReadFromMessageHString(hRead); 
	m_hstrOpenStopSound		= pServerDE->ReadFromMessageHString(hRead); 
	m_hstrCloseBusySound	= pServerDE->ReadFromMessageHString(hRead); 
	m_hstrCloseStopSound	= pServerDE->ReadFromMessageHString(hRead); 
	m_fWaitTime				= pServerDE->ReadFromMessageFloat(hRead); 
	pServerDE->ReadFromMessageVector(hRead, &m_vTriggerDims);
	m_fClosingSpeed			= pServerDE->ReadFromMessageFloat(hRead); 

	m_bLocked				= (DBOOL)pServerDE->ReadFromMessageByte(hRead); 
	m_bBoxPhysics			= (DBOOL)pServerDE->ReadFromMessageByte(hRead); 
	pServerDE->ReadFromMessageVector(hRead, &m_vOpenPos);
	pServerDE->ReadFromMessageVector(hRead, &m_vClosedPos);
	m_dwDoorState			= pServerDE->ReadFromMessageDWord(hRead); 
	m_fDoorOpenTime			= pServerDE->ReadFromMessageFloat(hRead); 

	m_fSoundRadius			= pServerDE->ReadFromMessageFloat(hRead); 

	m_hstrPortal			= pServerDE->ReadFromMessageHString(hRead);
	m_dwFlags				= pServerDE->ReadFromMessageDWord(hRead); 
	m_dwWaveform			= pServerDE->ReadFromMessageDWord(hRead); 

	m_bStartedMoving		= (DBOOL)pServerDE->ReadFromMessageByte(hRead);
	m_fOpenDelay			= pServerDE->ReadFromMessageFloat(hRead);
	m_bFireThrough			= (DBOOL)pServerDE->ReadFromMessageByte(hRead); 
}

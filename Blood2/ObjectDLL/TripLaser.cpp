// ----------------------------------------------------------------------- //
//
// MODULE  : TripLaser.cpp
//
// PURPOSE : TripLaser - Implementation
//
// CREATED : 10/23/97
//
// ----------------------------------------------------------------------- //

#include "TripLaser.h"
#include "Generic_msg_de.h"
#include "ClientServerShared.h"
#include "Trigger.h"
#include "ObjectUtilities.h"
#include <mbstring.h>
#include "SoundTypes.h"


// Static global variables..
static char *g_szTriggerOn = "TRIGGER"; 
static char *g_szTriggerOff = "TRIGGEROFF";
static char *g_szTriggerKill = "KILL";

#define DAMAGE_UPDATE_DELTA		0.1f

void CreateEndObject(DVector &vPos);


BEGIN_CLASS(TripLaser)
	ADD_STRINGPROP_FLAG(Filename, "Models\\default.abc", PF_HIDDEN)
	ADD_STRINGPROP_FLAG(Skin, "Skins\\ammo\\beamred.dtx", PF_HIDDEN)
	ADD_VECTORPROP_VAL_FLAG(Dims, 1.0f, 20.0f, 1.0f, PF_DIMS | PF_LOCALDIMS)
	ADD_COLORPROP(Color, 255.0f, 0.0f, 0.0f)
	ADD_REALPROP(Alpha, 0.5f)
	ADD_BOOLPROP(StartOn, DFALSE)
	ADD_STRINGPROP(TouchTriggerTarget, "") 
	ADD_STRINGPROP(TouchTriggerMessage, "")
	ADD_STRINGPROP(Sound, "") 
	ADD_STRINGPROP(TouchSound, "")
	ADD_REALPROP(Damage, 10.0f)
END_CLASS_DEFAULT(TripLaser, B2BaseClass, NULL, NULL)


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	TripLaser::TripLaser
//
//	PURPOSE:	Initialize
//
// ----------------------------------------------------------------------- //

TripLaser::TripLaser() : B2BaseClass(OT_MODEL)
{ 
	VEC_INIT(m_vDims);
	VEC_SET(m_vColor, 255.0f, 0.0f, 0.0f);
	m_fAlpha = 0.8f;
	VEC_INIT(m_vEndPoints[0]);
	VEC_INIT(m_vEndPoints[1]);

	m_dwFlags = 0;

	m_hstrTriggerTarget = DNULL;
	m_hstrTriggerMessage = DNULL;

	m_hstrSound = DNULL;
	m_hstrTouchSound = DNULL;
	m_hSound = DNULL;

	m_bTriggered = DFALSE;

	m_fDamage = 10.0f;
	m_fDamageTime = 0.0f;
	m_bRemove = DFALSE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	TripLaser::~TripLaser()
//
//	PURPOSE:	Deallocate object
//
// ----------------------------------------------------------------------- //

TripLaser::~TripLaser()
{
	CServerDE* pServerDE = GetServerDE();

	if (m_hstrTriggerTarget)
	{
		pServerDE->FreeString(m_hstrTriggerTarget);
		m_hstrTriggerTarget = DNULL;
	}

	if (m_hstrTriggerMessage)
	{
		pServerDE->FreeString(m_hstrTriggerMessage);
		m_hstrTriggerMessage = DNULL;
	}

	if (m_hstrSound)
	{
		pServerDE->FreeString(m_hstrSound);
		m_hstrSound = DNULL;
	}

	if (m_hstrTouchSound)
	{
		pServerDE->FreeString(m_hstrTouchSound);
		m_hstrTouchSound = DNULL;
	}

	if (m_hSound)	
	{
		pServerDE->KillSound(m_hSound);
	}
}

	
// ----------------------------------------------------------------------- //
//
//	ROUTINE:	TripLaser::EngineMessageFn
//
//	PURPOSE:	Handle engine messages
//
// ----------------------------------------------------------------------- //

DDWORD TripLaser::EngineMessageFn(DDWORD messageID, void *pData, DFLOAT fData)
{
	switch(messageID)
	{
		case MID_UPDATE:
		{
			Update();
			break;
		}

		case MID_PRECREATE:
		{
			DDWORD dwRet = B2BaseClass::EngineMessageFn(messageID, pData, fData);

			ObjectCreateStruct *pStruct = (ObjectCreateStruct*)pData;
			if (fData == PRECREATE_WORLDFILE || fData == PRECREATE_STRINGPROP)
				ReadProp(pStruct);

			_mbscpy((unsigned char*)pStruct->m_Filename, (const unsigned char*)"Models\\default.abc");
			_mbscpy((unsigned char*)pStruct->m_SkinName, (const unsigned char*)"Skins\\ammo\\beamred.dtx");

			return dwRet;
			break;
		}

		case MID_INITIALUPDATE:
		{
			if (fData != INITIALUPDATE_SAVEGAME)
				InitialUpdate();
			break;
		}

		case MID_TOUCHNOTIFY:
		{
			HandleTouch((HOBJECT)pData);
			break;
		}

		case MID_SAVEOBJECT:
			Save((HMESSAGEWRITE)pData, (DDWORD)fData);
			break;

		case MID_LOADOBJECT:
			Load((HMESSAGEREAD)pData, (DDWORD)fData);
			break;

		default : break;
	}

	return B2BaseClass::EngineMessageFn(messageID, pData, fData);
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	TripLaser::ObjectMessageFn()
//
//	PURPOSE:	Handler for server object messages.
//
// --------------------------------------------------------------------------- //

DDWORD TripLaser::ObjectMessageFn(HOBJECT hSender, DDWORD messageID, HMESSAGEREAD hRead)
{
	CServerDE* pServerDE = GetServerDE();
	switch (messageID)
	{
		case MID_TRIGGER:
			HSTRING hMsg = pServerDE->ReadFromMessageHString(hRead);
			TriggerMsg(hSender, hMsg);
			pServerDE->FreeString(hMsg);
			break;
	}
	
	return B2BaseClass::ObjectMessageFn(hSender, messageID, hRead);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	TripLaser::ReadProp
//
//	PURPOSE:	Set property value
//
// ----------------------------------------------------------------------- //

DBOOL TripLaser::ReadProp(ObjectCreateStruct *pStruct)
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE || !pStruct) return DFALSE;

	pServerDE->GetPropVector("Dims", &m_vDims);
	pServerDE->GetPropVector("Color", &m_vColor);
	pServerDE->GetPropReal("Alpha", &m_fAlpha);
	pServerDE->GetPropBool("StartOn", &m_bTriggered);
	pServerDE->GetPropReal("Damage", &m_fDamage);

	char buf[MAX_CS_FILENAME_LEN];
	buf[0] = '\0';
	pServerDE->GetPropString("TouchTriggerTarget", buf, MAX_CS_FILENAME_LEN);
	if (buf[0]) m_hstrTriggerTarget = pServerDE->CreateString(buf);

	buf[0] = '\0';
	pServerDE->GetPropString("TouchTriggerMessage", buf, MAX_CS_FILENAME_LEN);
	if (buf[0]) m_hstrTriggerMessage = pServerDE->CreateString(buf);

	buf[0] = '\0';
	pServerDE->GetPropString("Sound", buf, MAX_CS_FILENAME_LEN);
	if (buf[0]) m_hstrSound = pServerDE->CreateString(buf);

	buf[0] = '\0';
	pServerDE->GetPropString("TouchSound", buf, MAX_CS_FILENAME_LEN);
	if (buf[0]) m_hstrTouchSound = pServerDE->CreateString(buf);

	return DTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	TripLaser::InitialUpdate
//
//	PURPOSE:	Update - If this is called, the laser is switching off.
//
// ----------------------------------------------------------------------- //

void TripLaser::Update()
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE) return;

	// Flicker the light back on for a short time..
	if (!m_bTriggered)
	{
		DDWORD dwFlags = pServerDE->GetObjectFlags(m_hObject);

		// Already visible - hide it
		if (dwFlags & FLAG_VISIBLE)
		{
			pServerDE->SetObjectFlags(m_hObject, m_dwFlags);
			if (m_bRemove)
				pServerDE->RemoveObject(m_hObject);
			else
				pServerDE->SetNextUpdate(m_hObject, 0.0f);
		}
		else
		{
			pServerDE->SetObjectFlags(m_hObject, m_dwFlags | FLAG_VISIBLE);
			DFLOAT fNextUpdate = pServerDE->Random(0.2f, 0.3f);
			pServerDE->SetNextUpdate(m_hObject, fNextUpdate);
		}

	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	TripLaser::InitialUpdate
//
//	PURPOSE:	Handle initial Update
//
// ----------------------------------------------------------------------- //

void TripLaser::InitialUpdate()
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE) return;

	// Mark this object as savable
	DDWORD dwUsrFlags = pServerDE->GetObjectUserFlags(m_hObject);
	dwUsrFlags |= USRFLG_SAVEABLE;
	pServerDE->SetObjectUserFlags(m_hObject, dwUsrFlags);

	// Scale color values from 0-255 to 0-1;
	VEC_DIVSCALAR(m_vColor, m_vColor, 255.0f);

	// Create the model
	VEC_MULSCALAR(m_vDims, m_vDims, 2.0f);
	pServerDE->ScaleObject(m_hObject, &m_vDims);

	// Set the color
	pServerDE->SetObjectColor(m_hObject, m_vColor.x, m_vColor.y, m_vColor.z, m_fAlpha);

	// Compute the endpoints of the laser
	DVector vPos, vF, vR, vU, vTmp;
	DRotation rRot;

	// Use the longest dimension of m_vDims..
	pServerDE->GetObjectPos(m_hObject, &vPos);
	pServerDE->GetObjectRotation(m_hObject, &rRot);

	pServerDE->GetRotationVectors(&rRot, &vU, &vR, &vF);
	if (m_vDims.x >= m_vDims.y && m_vDims.x >= m_vDims.z)
	{
		VEC_MULSCALAR(vTmp, vR, m_vDims.x);
	}
	else if  (m_vDims.y >= m_vDims.x && m_vDims.y >= m_vDims.z)
	{
		VEC_MULSCALAR(vTmp, vU, m_vDims.y);
	}
	else if  (m_vDims.z >= m_vDims.x && m_vDims.z >= m_vDims.y)
	{
		VEC_MULSCALAR(vTmp, vF, m_vDims.z);
	}
	VEC_MULSCALAR(vTmp, vTmp, 0.5f);

	VEC_SUB(m_vEndPoints[0], vPos, vTmp);
	VEC_ADD(m_vEndPoints[1], vPos, vTmp);

	// Make sure the dims have at least some thickness..
	vTmp.x = (DFLOAT)fabs(vTmp.x);
	vTmp.y = (DFLOAT)fabs(vTmp.y);
	vTmp.z = (DFLOAT)fabs(vTmp.z);
	if (vTmp.x <= 1.0f) vTmp.x = 1.0f;
	if (vTmp.y <= 1.0f) vTmp.y = 1.0f;
	if (vTmp.z <= 1.0f) vTmp.z = 1.0f;
	pServerDE->SetObjectDims(m_hObject, &vTmp);

	// Tell the clients about the TripLaser
	if (m_bTriggered)
	{
		m_dwFlags |= FLAG_VISIBLE | FLAG_TOUCH_NOTIFY | FLAG_NOLIGHT;

		// Play the sound, if any
		if (m_hstrSound)
			m_hSound = PlaySoundFromObject(m_hObject, 
											pServerDE->GetStringData(m_hstrSound), 
											400, SOUNDPRIORITY_MISC_LOW,
											DTRUE, DTRUE, DTRUE);
	}

	pServerDE->SetObjectFlags(m_hObject, m_dwFlags);
	pServerDE->SetNextUpdate(m_hObject, 0.0f);
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	TripLaser::TriggerMsg()
//
//	PURPOSE:	Handler for TripLaser trigger messages.
//
// --------------------------------------------------------------------------- //

void TripLaser::TriggerMsg(HOBJECT hSender, HSTRING hMsg)
{
	CServerDE* pServerDE = GetServerDE();
	HSTRING hstr;

	hstr = pServerDE->CreateString(g_szTriggerOn);
	if (pServerDE->CompareStringsUpper(hMsg, hstr))
	{
		m_bTriggered = DTRUE;
		m_dwFlags = FLAG_TOUCH_NOTIFY | FLAG_VISIBLE | FLAG_NOLIGHT;
		if (m_hstrSound && !m_hSound)
			m_hSound = PlaySoundFromObject(m_hObject, 
											pServerDE->GetStringData(m_hstrSound), 
											400, SOUNDPRIORITY_MISC_LOW,
											DTRUE, DTRUE, DTRUE);
	}
	pServerDE->FreeString(hstr);

	hstr = pServerDE->CreateString(g_szTriggerOff);
	if (pServerDE->CompareStringsUpper(hMsg, hstr))
	{
		m_bTriggered = DFALSE;
		m_dwFlags = 0;
		if (m_hSound)
		{
			pServerDE->KillSound(m_hSound);
			m_hSound = DNULL;
		}
		// Set update so it'll flash a little
		DFLOAT fNextUpdate = pServerDE->Random(0.25f, 0.5f);
		pServerDE->SetNextUpdate(m_hObject, fNextUpdate);
	}
	pServerDE->FreeString(hstr);

	// Kill trigger message
	hstr = pServerDE->CreateString(g_szTriggerKill);
	if (pServerDE->CompareStringsUpper(hMsg, hstr))
	{
		m_bTriggered = DFALSE;
		m_dwFlags = 0;
		if (m_hSound)
		{
			pServerDE->KillSound(m_hSound);
			m_hSound = DNULL;
		}
		// Set update so it'll flash a little
		DFLOAT fNextUpdate = pServerDE->Random(0.25f, 0.5f);
		pServerDE->SetNextUpdate(m_hObject, fNextUpdate);
		m_bRemove = DTRUE;
	}
	pServerDE->FreeString(hstr);

	pServerDE->SetObjectFlags(m_hObject, m_dwFlags);
}

	

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	TripLaser::HandleTouch()
//
//	PURPOSE:	Handle touch notify message
//
// ----------------------------------------------------------------------- //

void TripLaser::HandleTouch(HOBJECT hObj)
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE) return;

	// Apply damage 10 times a second.
	DFLOAT fTime = pServerDE->GetTime();
	if (fTime <= m_fDamageTime)
		return;

	m_fDamageTime = fTime + DAMAGE_UPDATE_DELTA;
	// Something's touching the bounding box, so cast a line 
	// between the endpoints and see if the laser hits it.
	IntersectQuery iq;
	IntersectInfo  ii;

	VEC_COPY(iq.m_From, m_vEndPoints[0]);	// Get start point at the last known position.
	VEC_COPY(iq.m_To, m_vEndPoints[1]);		// Get start point at the last known position.
	iq.m_Flags = INTERSECT_OBJECTS | IGNORE_NONSOLID | INTERSECT_HPOLY;
	iq.m_FilterFn = NULL;
	iq.m_pUserData = NULL;	

	// Hit something!
	if (pServerDE->IntersectSegment(&iq, &ii))
	{
		if (ii.m_hObject == hObj)	// hit it!
		{
			DVector vDir;
			VEC_SUB(vDir, m_vEndPoints[1], m_vEndPoints[0]);
			// Send damage message to object...
			DamageObject(m_hObject, this, hObj, m_fDamage*DAMAGE_UPDATE_DELTA, vDir, ii.m_Point, DAMAGE_TYPE_NORMAL); 

			if (m_hstrTouchSound)
				PlaySoundFromObject(m_hObject, 
									pServerDE->GetStringData(m_hstrTouchSound), 
									800, SOUNDPRIORITY_MISC_HIGH);
		}
	}
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	TripLaser::Save
//
//	PURPOSE:	Save the object
//
// ----------------------------------------------------------------------- //

void TripLaser::Save(HMESSAGEWRITE hWrite, DDWORD dwSaveFlags)
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE || !hWrite) return;

	DFLOAT fTime = pServerDE->GetTime();

	pServerDE->WriteToMessageVector(hWrite, &m_vDims);
	pServerDE->WriteToMessageVector(hWrite, &m_vColor);
	pServerDE->WriteToMessageFloat(hWrite, m_fAlpha);
	pServerDE->WriteToMessageFloat(hWrite, m_fDamage);
	pServerDE->WriteToMessageFloat(hWrite, m_fDamageTime - fTime);

	pServerDE->WriteToMessageHString(hWrite, m_hstrTriggerTarget);
	pServerDE->WriteToMessageHString(hWrite, m_hstrTriggerMessage);

	pServerDE->WriteToMessageHString(hWrite, m_hstrSound);
	pServerDE->WriteToMessageHString(hWrite, m_hstrTouchSound);

	pServerDE->WriteToMessageVector(hWrite, &m_vEndPoints[0]);
	pServerDE->WriteToMessageVector(hWrite, &m_vEndPoints[1]);
	pServerDE->WriteToMessageDWord(hWrite, m_dwFlags);
	pServerDE->WriteToMessageByte(hWrite, m_bTriggered);
	pServerDE->WriteToMessageByte(hWrite, m_bRemove);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	TripLaser::Load
//
//	PURPOSE:	Load the object
//
// ----------------------------------------------------------------------- //

void TripLaser::Load(HMESSAGEREAD hRead, DDWORD dwLoadFlags)
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE || !hRead) return;

	DFLOAT fTime = pServerDE->GetTime();

	pServerDE->ReadFromMessageVector(hRead, &m_vDims);
	pServerDE->ReadFromMessageVector(hRead, &m_vColor);
	m_fAlpha			= pServerDE->ReadFromMessageFloat(hRead);
	m_fDamage			= pServerDE->ReadFromMessageFloat(hRead);
	m_fDamageTime		= pServerDE->ReadFromMessageFloat(hRead) + fTime;

	m_hstrTriggerTarget	= pServerDE->ReadFromMessageHString(hRead);
	m_hstrTriggerMessage= pServerDE->ReadFromMessageHString(hRead);

	m_hstrSound			= pServerDE->ReadFromMessageHString(hRead);
	m_hstrTouchSound	= pServerDE->ReadFromMessageHString(hRead);

	pServerDE->ReadFromMessageVector(hRead, &m_vEndPoints[0]);
	pServerDE->ReadFromMessageVector(hRead, &m_vEndPoints[1]);
	m_dwFlags			= pServerDE->ReadFromMessageDWord(hRead);
	m_bTriggered		= pServerDE->ReadFromMessageByte(hRead);
	m_bRemove			= pServerDE->ReadFromMessageByte(hRead);

	// Tell the clients about the TripLaser
	if (m_bTriggered)
	{
		if( m_hSound )
		{
			pServerDE->KillSound( m_hSound );
			m_hSound = DNULL;
		}

		// Play the sound, if any
		if( m_hstrSound && m_hObject )
			m_hSound = PlaySoundFromObject(m_hObject, 
											pServerDE->GetStringData(m_hstrSound), 
											400, SOUNDPRIORITY_MISC_LOW,
											DTRUE, DTRUE, DTRUE);
	}
}




// ----------------------------------------------------------------------- //
//
// MODULE  : Rotating.cpp
//
// PURPOSE : Rotating implementation
//
// CREATED : 4/29/98
//
// ----------------------------------------------------------------------- //

#include "Rotating.h"
#include "cpp_server_de.h"
#include "ObjectUtilities.h"
#include "generic_msg_de.h"
#include <mbstring.h>


#define RWM_UPDATE_DELTA	0.001f
#define TRIGGER_MSG_ON		"ON"
#define TRIGGER_MSG_OFF		"OFF"
#define TRIGGER_MSG_TOGGLE	"TOGGLE"

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Rotating::Rotating()
//
//	PURPOSE:	Initialize object
//
// ----------------------------------------------------------------------- //

Rotating::Rotating() : Aggregate()
{
	m_hObject			= DNULL;
	m_hstrSpinUpSound	= DNULL;
	m_hstrBusySound		= DNULL;
	m_hstrSpinDownSound	= DNULL;
	m_fSoundRadius		= 1000.0f;
	m_sndLastSound		= DNULL;

	VEC_INIT(m_vVelocity);
	VEC_INIT(m_vSaveVelocity);
	VEC_INIT(m_vSpinUpTime);
	VEC_INIT(m_vSpinDownTime);
	VEC_INIT(m_vSpinTimeLeft);
	VEC_SET(m_vSign, 1.0f, 1.0f, 1.0f);
	
	m_fLastTime		= 0.0f;
	m_fStartTime	= 0.0f;

	m_eState		= RWM_OFF;

	m_fPitch		= 0.0f;
	m_fYaw			= 0.0f;
	m_fRoll			= 0.0f;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Rotating::~Rotating()
//
//	PURPOSE:	Destroy object
//
// ----------------------------------------------------------------------- //

Rotating::~Rotating()
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE) return;

	if (m_hstrSpinUpSound)
	{
		pServerDE->FreeString(m_hstrSpinUpSound);
	}
	if (m_hstrSpinDownSound)
	{
		pServerDE->FreeString(m_hstrSpinDownSound);
	}
	if (m_hstrBusySound)
	{
		pServerDE->FreeString(m_hstrBusySound);
	}
	if( m_sndLastSound )
		g_pServerDE->KillSound( m_sndLastSound );
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Rotating::EngineMessageFn
//
//	PURPOSE:	Handle engine messages
//
// ----------------------------------------------------------------------- //

DDWORD Rotating::EngineMessageFn(LPBASECLASS pObject, DDWORD messageID, void *pData, DFLOAT fData)
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
			ObjectCreateStruct* pStruct = (ObjectCreateStruct*)pData;
			
			if (pStruct)
			{
				if (fData == PRECREATE_WORLDFILE)
				{
					ReadProp(pStruct);
				}
			}
			break;
		}

		case MID_INITIALUPDATE:
		{
			if (fData != INITIALUPDATE_SAVEGAME)
				InitialUpdate();
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


	return Aggregate::EngineMessageFn(pObject, messageID, pData, fData);
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	Rotating::ObjectMessageFn()
//
//	PURPOSE:	Handler for server object messages.
//
// --------------------------------------------------------------------------- //

DDWORD Rotating::ObjectMessageFn(LPBASECLASS pObject, HOBJECT hSender, DDWORD messageID, HMESSAGEREAD hRead)
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE) return 0;

	switch (messageID)
	{
		case MID_TRIGGER:
		{
			HSTRING hMsg = pServerDE->ReadFromMessageHString(hRead);
			HandleTrigger(hSender, hMsg);
			pServerDE->FreeString(hMsg);
		}
		break;
	}
	
	return Aggregate::ObjectMessageFn(pObject, hSender, messageID, hRead);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Rotating::ReadProp
//
//	PURPOSE:	Set property value
//
// ----------------------------------------------------------------------- //

DBOOL Rotating::ReadProp(ObjectCreateStruct *)
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE) return DFALSE;

	DFLOAT fRealVal;
	pServerDE->GetPropReal("XAxisRevTime", &fRealVal);
	if (fRealVal > 0.0f) m_vVelocity.x = MATH_CIRCLE / fRealVal;

	pServerDE->GetPropReal("YAxisRevTime", &fRealVal);
	if (fRealVal > 0.0f) m_vVelocity.y = MATH_CIRCLE / fRealVal;

	pServerDE->GetPropReal("ZAxisRevTime", &fRealVal);
	if (fRealVal > 0.0f) m_vVelocity.z = MATH_CIRCLE / fRealVal;

	DBOOL bVal = DFALSE;
	pServerDE->GetPropBool("XRotateForward", &bVal);
	m_vSign.x = (bVal == DTRUE) ? 1.0f : -1.0f;

	bVal = DFALSE;
	pServerDE->GetPropBool("YRotateForward", &bVal);
	m_vSign.y = (bVal == DTRUE) ? 1.0f : -1.0f;

	bVal = DFALSE;
	pServerDE->GetPropBool("ZRotateForward", &bVal);
	m_vSign.z = (bVal == DTRUE) ? 1.0f : -1.0f;

	bVal = DFALSE;
	pServerDE->GetPropBool("StartOn", &bVal);
	if (bVal) m_eState = RWM_NORMAL;
	
	pServerDE->GetPropReal("SoundRadius", &m_fSoundRadius);

	pServerDE->GetPropReal("XAxisSpinUpTime", &fRealVal);
	m_vSpinUpTime.x = fRealVal;

	pServerDE->GetPropReal("XAxisSpinDownTime", &fRealVal);
	m_vSpinDownTime.x = fRealVal;

	pServerDE->GetPropReal("YAxisSpinUpTime", &fRealVal);
	m_vSpinUpTime.y = fRealVal;

	pServerDE->GetPropReal("YAxisSpinDownTime", &fRealVal);
	m_vSpinDownTime.y = fRealVal;

	pServerDE->GetPropReal("ZAxisSpinUpTime", &fRealVal);
	m_vSpinUpTime.z = fRealVal;

	pServerDE->GetPropReal("ZAxisSpinDownTime", &fRealVal);
	m_vSpinDownTime.z = fRealVal;

	char buf[MAX_CS_FILENAME_LEN];
	if (pServerDE->GetPropString("SpinUpSound", buf, MAX_CS_FILENAME_LEN) == DE_OK)
	{
		m_hstrSpinUpSound = pServerDE->CreateString(buf);
	}

	if (pServerDE->GetPropString("BusySound", buf, MAX_CS_FILENAME_LEN) == DE_OK)
	{
		m_hstrBusySound = pServerDE->CreateString(buf);
	}

	if (pServerDE->GetPropString("SpinDownSound", buf, MAX_CS_FILENAME_LEN) == DE_OK)
	{
		m_hstrSpinDownSound = pServerDE->CreateString(buf);
	}

	return DTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Rotating::InitialUpdate()
//
//	PURPOSE:	First update
//
// ----------------------------------------------------------------------- //

void Rotating::InitialUpdate()
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE) return;

	if (m_eState != RWM_OFF)
	{
//		pServerDE->SetNextUpdate(m_hObject, RWM_UPDATE_DELTA);
		SetNormalRotation();
	}

	VEC_COPY(m_vSaveVelocity, m_vVelocity);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Rotating::Init
//
//	PURPOSE:	Initialize object
//
// ----------------------------------------------------------------------- //

DBOOL Rotating::Init(HOBJECT hObject)
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!hObject || !pServerDE) return DFALSE;

	m_hObject = hObject;

	return DTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Rotating::SetNormalRotation()
//
//	PURPOSE:	Set the model to normal rotation state
//
// ----------------------------------------------------------------------- //

void Rotating::SetNormalRotation()
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE) return;

	m_eState = RWM_NORMAL;

	m_fLastTime = pServerDE->GetTime();

	StartSound(m_hstrBusySound, DTRUE);

	pServerDE->SetNextUpdate(m_hObject, RWM_UPDATE_DELTA);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Rotating::SetOff()
//
//	PURPOSE:	Set the model to off state
//
// ----------------------------------------------------------------------- //

void Rotating::SetOff()
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE) return;

	m_eState = RWM_OFF;

	if (m_sndLastSound)
	{
		pServerDE->KillSound(m_sndLastSound);
		m_sndLastSound = DNULL;
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Rotating::SetSpinUp()
//
//	PURPOSE:	Set the model to the spin up state
//
// ----------------------------------------------------------------------- //

void Rotating::SetSpinUp()
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE) return;

	m_eState = RWM_SPINUP;

	m_fStartTime = pServerDE->GetTime();

	VEC_COPY(m_vSpinTimeLeft, m_vSpinUpTime);

	StartSound(m_hstrSpinUpSound, DFALSE);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Rotating::SetSpinDown()
//
//	PURPOSE:	Set the model to the spin up state
//
// ----------------------------------------------------------------------- //

void Rotating::SetSpinDown()
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE) return;

	m_eState = RWM_SPINDOWN;

	m_fStartTime = pServerDE->GetTime();

	VEC_COPY(m_vSpinTimeLeft, m_vSpinDownTime);

	StartSound(m_hstrSpinDownSound, DFALSE);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Rotating::Update()
//
//	PURPOSE:	Update the model
//
// ----------------------------------------------------------------------- //

void Rotating::Update()
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE) return;

	DFLOAT fUpdateDelta = RWM_UPDATE_DELTA;

	switch (m_eState)
	{
		case RWM_SPINUP:
			UpdateSpinUp();
		break;

		case RWM_SPINDOWN:
			UpdateSpinDown();
		break;

		case RWM_OFF:
			fUpdateDelta = 0.0f;
		break;

		case RWM_NORMAL:
		default:
			UpdateNormalRotation();
		break;
	}

	if (fUpdateDelta)
		pServerDE->SetNextUpdate(m_hObject, fUpdateDelta);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Rotating::UpdateNormalRotation()
//
//	PURPOSE:	Update normal rotation
//
// ----------------------------------------------------------------------- //

void Rotating::UpdateNormalRotation()
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE) return;

	DRotation rRot;
	pServerDE->GetObjectRotation(m_hObject, &rRot);

	DFLOAT fTime = pServerDE->GetTime();
	DFLOAT fDeltaTime = fTime - m_fLastTime;

	if (m_vVelocity.x > 0.0f)
	{
		m_fPitch += (m_vSign.x * (m_vVelocity.x * fDeltaTime));
	}

	if (m_vVelocity.y > 0.0f)
	{
		m_fYaw += (m_vSign.y * (m_vVelocity.y * fDeltaTime));
	}

	if (m_vVelocity.z > 0.0f)
	{
		m_fRoll += (m_vSign.z * (m_vVelocity.z * fDeltaTime));
	}

	pServerDE->SetupEuler(&rRot, m_fPitch, m_fYaw, m_fRoll);
	pServerDE->RotateObject(m_hObject, &rRot);	

	m_fLastTime = fTime;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Rotating::UpdateSpinUp()
//
//	PURPOSE:	Update spin up 
//
// ----------------------------------------------------------------------- //

void Rotating::UpdateSpinUp()
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE) return;

	DBOOL bXDone = DFALSE, bYDone = DFALSE, bZDone = DFALSE;
	DFLOAT fDeltaTime = pServerDE->GetFrameTime();

	// Calculate current velocity...

	m_vSpinTimeLeft.x -= fDeltaTime;
	if (m_vSaveVelocity.x > 0.0f && m_vSpinTimeLeft.x >= 0.0f)
	{
		m_vVelocity.x = m_vSaveVelocity.x * (m_vSpinUpTime.x - m_vSpinTimeLeft.x) / m_vSpinUpTime.x;
	}
	else
	{
		m_vVelocity.x = m_vSaveVelocity.x;
		bXDone = DTRUE;
	}

	m_vSpinTimeLeft.y -= fDeltaTime;
	if (m_vSaveVelocity.y > 0.0f && m_vSpinTimeLeft.y >= 0.0f)
	{
		m_vVelocity.y = m_vSaveVelocity.y * (m_vSpinUpTime.y - m_vSpinTimeLeft.y) / m_vSpinUpTime.y;
	}
	else
	{
		m_vVelocity.y = m_vSaveVelocity.y;
		bYDone = DTRUE;
	}

	m_vSpinTimeLeft.z -= fDeltaTime;
	if (m_vSaveVelocity.z > 0.0f && m_vSpinTimeLeft.z >= 0.0f)
	{
		m_vVelocity.z = m_vSaveVelocity.z * (m_vSpinUpTime.z - m_vSpinTimeLeft.z) / m_vSpinUpTime.z;
	}
	else
	{
		m_vVelocity.z = m_vSaveVelocity.z;
		bZDone = DTRUE;
	}

	// Call normal update to do the work...

	UpdateNormalRotation();

	if (bXDone && bYDone && bZDone)
	{
		SetNormalRotation();
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Rotating::UpdateSpinDown()
//
//	PURPOSE:	Update spin down
//
// ----------------------------------------------------------------------- //

void Rotating::UpdateSpinDown()
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE) return;

	DBOOL bXDone = DFALSE, bYDone = DFALSE, bZDone = DFALSE;
	DFLOAT fDeltaTime = pServerDE->GetFrameTime();

	// Calculate current velocity...

	m_vSpinTimeLeft.x -= fDeltaTime;
	if (m_vSaveVelocity.x > 0.0f && m_vSpinTimeLeft.x >= 0.0f)
	{
		m_vVelocity.x = m_vSaveVelocity.x - (m_vSaveVelocity.x * (m_vSpinDownTime.x - m_vSpinTimeLeft.x) / m_vSpinDownTime.x);
	}
	else
	{
		m_vVelocity.x = 0.0f;
		bXDone = DTRUE;
	}

	m_vSpinTimeLeft.y -= fDeltaTime;
	if (m_vSaveVelocity.y > 0.0f && m_vSpinTimeLeft.y >= 0.0f)
	{
		m_vVelocity.y = m_vSaveVelocity.y - (m_vSaveVelocity.y * (m_vSpinDownTime.y - m_vSpinTimeLeft.y) / m_vSpinDownTime.y);
	}
	else
	{
		m_vVelocity.y = 0.0f;
		bYDone = DTRUE;
	}

	m_vSpinTimeLeft.z -= fDeltaTime;
	if (m_vSaveVelocity.z > 0.0f && m_vSpinTimeLeft.z >= 0.0f)
	{
		m_vVelocity.z = m_vSaveVelocity.z - (m_vSaveVelocity.z * (m_vSpinDownTime.z - m_vSpinTimeLeft.z) / m_vSpinDownTime.z);
	}
	else
	{
		m_vVelocity.z = 0.0f;
		bZDone = DTRUE;
	}

	// Call normal update to do the work...

	UpdateNormalRotation();

	if (bXDone && bYDone && bZDone)
	{
		SetOff();
	}
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	Rotating::TriggerMsg()
//
//	PURPOSE:	Handler trigger messages
//
// --------------------------------------------------------------------------- //

void Rotating::HandleTrigger(HOBJECT hSender, HSTRING hMsg)
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE) return;

	char* pMsg = pServerDE->GetStringData(hMsg);
	if (!pMsg || !pMsg[0]) return;

	if (m_eState == RWM_OFF && _mbsicmp((const unsigned char*)pMsg, (const unsigned char*)TRIGGER_MSG_ON) == 0)
	{
		SetSpinUp();
		pServerDE->SetNextUpdate(m_hObject, RWM_UPDATE_DELTA);
	}
	else if (m_eState != RWM_OFF && _mbsicmp((const unsigned char*)pMsg, (const unsigned char*)TRIGGER_MSG_OFF) == 0)
	{
		SetSpinDown();
		pServerDE->SetNextUpdate(m_hObject, RWM_UPDATE_DELTA);
	}
	else if (_mbsicmp((const unsigned char*)pMsg, (const unsigned char*)TRIGGER_MSG_TOGGLE) == 0)
	{
		if (m_eState == RWM_OFF)
			SetSpinUp();
		else
			SetSpinDown();
		pServerDE->SetNextUpdate(m_hObject, RWM_UPDATE_DELTA);
	}
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	Rotating::StartSound()
//
//	PURPOSE:	Start the specified sound
//
// --------------------------------------------------------------------------- //

void Rotating::StartSound(HSTRING hstrSoundName, DBOOL bLoop)
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
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
	playSoundInfo.m_dwFlags |= PLAYSOUND_ATTACHED;

	if (bLoop)
	{
		playSoundInfo.m_dwFlags |= PLAYSOUND_LOOP | PLAYSOUND_GETHANDLE;
	}

	_mbsncpy((unsigned char*)playSoundInfo.m_szSoundName, (const unsigned char*)pSoundName, _MAX_PATH);
	playSoundInfo.m_hObject = m_hObject;
	playSoundInfo.m_fOuterRadius = m_fSoundRadius;
	playSoundInfo.m_fInnerRadius = 200;

	pServerDE->PlaySound(&playSoundInfo);

	// Save the handle of the sound...

	m_sndLastSound = playSoundInfo.m_hSound;
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Rotating::Save
//
//	PURPOSE:	Save the object
//
// ----------------------------------------------------------------------- //

void Rotating::Save(HMESSAGEWRITE hWrite, DDWORD dwSaveFlags)
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE || !hWrite) return;

	DFLOAT fTime = pServerDE->GetTime();

	pServerDE->WriteToLoadSaveMessageObject(hWrite, m_hObject);
	pServerDE->WriteToMessageVector(hWrite, &m_vVelocity);
	pServerDE->WriteToMessageVector(hWrite, &m_vSaveVelocity);
	pServerDE->WriteToMessageVector(hWrite, &m_vSign);
	pServerDE->WriteToMessageVector(hWrite, &m_vSpinUpTime);
	pServerDE->WriteToMessageVector(hWrite, &m_vSpinDownTime);
	pServerDE->WriteToMessageVector(hWrite, &m_vSpinTimeLeft);

	pServerDE->WriteToMessageFloat(hWrite, m_fLastTime - fTime);
	pServerDE->WriteToMessageFloat(hWrite, m_fStartTime - fTime);
	pServerDE->WriteToMessageFloat(hWrite, m_fPitch);
	pServerDE->WriteToMessageFloat(hWrite, m_fYaw);
	pServerDE->WriteToMessageFloat(hWrite, m_fRoll);

	pServerDE->WriteToMessageDWord(hWrite, (DDWORD)m_eState);

	pServerDE->WriteToMessageHString(hWrite, m_hstrBusySound);
	pServerDE->WriteToMessageHString(hWrite, m_hstrSpinUpSound);
	pServerDE->WriteToMessageHString(hWrite, m_hstrSpinDownSound);
	pServerDE->WriteToMessageFloat(hWrite, m_fSoundRadius);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Rotating::Load
//
//	PURPOSE:	Load the object
//
// ----------------------------------------------------------------------- //

void Rotating::Load(HMESSAGEREAD hRead, DDWORD dwLoadFlags)
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE || !hRead) return;

	DFLOAT fTime = pServerDE->GetTime();

	pServerDE->ReadFromLoadSaveMessageObject(hRead, &m_hObject);
	pServerDE->ReadFromMessageVector(hRead, &m_vVelocity);
	pServerDE->ReadFromMessageVector(hRead, &m_vSaveVelocity);
	pServerDE->ReadFromMessageVector(hRead, &m_vSign);
	pServerDE->ReadFromMessageVector(hRead, &m_vSpinUpTime);
	pServerDE->ReadFromMessageVector(hRead, &m_vSpinDownTime);
	pServerDE->ReadFromMessageVector(hRead, &m_vSpinTimeLeft);

	m_fLastTime			= pServerDE->ReadFromMessageFloat(hRead) + fTime;
	m_fStartTime		= pServerDE->ReadFromMessageFloat(hRead) + fTime;
	m_fPitch			= pServerDE->ReadFromMessageFloat(hRead);
	m_fYaw				= pServerDE->ReadFromMessageFloat(hRead);
	m_fRoll				= pServerDE->ReadFromMessageFloat(hRead);

	m_eState			= (RWMState)pServerDE->ReadFromMessageDWord(hRead);

	m_hstrBusySound		= pServerDE->ReadFromMessageHString(hRead);
	m_hstrSpinUpSound	= pServerDE->ReadFromMessageHString(hRead);
	m_hstrSpinDownSound	= pServerDE->ReadFromMessageHString(hRead);
	m_fSoundRadius		= pServerDE->ReadFromMessageFloat(hRead);
}

// ----------------------------------------------------------------------- //
//
// MODULE  : RotatingWorldModel.cpp
//
// PURPOSE : RotatingWorldModel implementation
//
// CREATED : 10/27/97
//
// ----------------------------------------------------------------------- //

#include "RotatingWorldModel.h"
#include "cpp_server_de.h"
#include "RiotObjectUtilities.h"
#include "generic_msg_de.h"
#include "ClientServerShared.h"

#define INFINITE_MASS			100000.0f

BEGIN_CLASS(RotatingWorldModel)
	ADD_VISIBLE_FLAG(1, 0)
	ADD_SOLID_FLAG(1, 0)
	ADD_GRAVITY_FLAG(1, 0)
	ADD_BOOLPROP(BoxPhysics, DTRUE)
	ADD_DESTRUCTABLE_WORLD_MODEL_AGGREGATE()
	ADD_BOOLPROP(StartOn, DTRUE)
	ADD_STRINGPROP(SpinUpSound, "")
	ADD_STRINGPROP(BusySound, "")
	ADD_STRINGPROP(SpinDownSound, "")
	ADD_REALPROP_FLAG(SoundRadius, 1000.0f, PF_RADIUS)
	ADD_REALPROP(XAxisRevTime, 0.0f)
	ADD_REALPROP(XAxisSpinUpTime,   0.0f)
	ADD_REALPROP(XAxisSpinDownTime, 0.0f)
	ADD_BOOLPROP(XRotateForward, DTRUE)
	ADD_REALPROP(YAxisRevTime, 0.0f)
	ADD_REALPROP(YAxisSpinUpTime,   0.0f)
	ADD_REALPROP(YAxisSpinDownTime, 0.0f)
	ADD_BOOLPROP(YRotateForward, DTRUE)
	ADD_REALPROP(ZAxisRevTime, 0.0f)
	ADD_REALPROP(ZAxisSpinUpTime,   0.0f)
	ADD_REALPROP(ZAxisSpinDownTime, 0.0f)
	ADD_BOOLPROP(ZRotateForward, DTRUE)
END_CLASS_DEFAULT(RotatingWorldModel, BaseClass, NULL, NULL)

#define RWM_UPDATE_DELTA	0.01f
#define TRIGGER_MSG_ON		"ON"
#define TRIGGER_MSG_OFF		"OFF"

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	RotatingWorldModel::RotatingWorldModel()
//
//	PURPOSE:	Initialize object
//
// ----------------------------------------------------------------------- //

RotatingWorldModel::RotatingWorldModel() : BaseClass(OT_WORLDMODEL)
{
	AddAggregate(&m_damage);

	m_hstrSpinUpSound	= DNULL;
	m_hstrBusySound		= DNULL;
	m_hstrSpinDownSound	= DNULL;
	m_fSoundRadius		= 1000.0f;
	m_sndLastSound		= DNULL;
	m_bBoxPhysics		= DTRUE;

	VEC_INIT(m_vVelocity);
	VEC_INIT(m_vSaveVelocity);
	VEC_INIT(m_vSpinUpTime);
	VEC_INIT(m_vSpinDownTime);
	VEC_INIT(m_vSpinTimeLeft);
	VEC_SET(m_vSign, 1.0f, 1.0f, 1.0f);
	
	m_fLastTime		= 0.0f;
	m_fStartTime	= 0.0f;

	m_eState		= RWM_NORMAL;

	m_fPitch		= 0.0f;
	m_fYaw			= 0.0f;
	m_fRoll			= 0.0f;

	m_eSurfaceType	= ST_METAL;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	RotatingWorldModel::~RotatingWorldModel()
//
//	PURPOSE:	Destroy object
//
// ----------------------------------------------------------------------- //

RotatingWorldModel::~RotatingWorldModel()
{
	CServerDE* pServerDE = GetServerDE();
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

	if (m_sndLastSound)
	{
		pServerDE->KillSound(m_sndLastSound);
	}

}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	RotatingWorldModel::EngineMessageFn
//
//	PURPOSE:	Handle engine messages
//
// ----------------------------------------------------------------------- //

DDWORD RotatingWorldModel::EngineMessageFn(DDWORD messageID, void *pData, DFLOAT fData)
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
			DDWORD dwRet = BaseClass::EngineMessageFn(messageID, pData, fData);

			ObjectCreateStruct* pStruct = (ObjectCreateStruct*)pData;
			
			if (pStruct)
			{
				if (fData == PRECREATE_WORLDFILE)
				{
					ReadProp(pStruct);
				}

				pStruct->m_ObjectType = OT_WORLDMODEL;
				SAFE_STRCPY(pStruct->m_Filename, pStruct->m_Name);
				pStruct->m_SkinName[0] = '\0';

				pStruct->m_Flags |= FLAG_FULLPOSITIONRES | FLAG_GOTHRUWORLD | FLAG_DONTFOLLOWSTANDING | (m_bBoxPhysics ? FLAG_BOXPHYSICS : 0);

				// Don't go through world if gravity is set...

				if (pStruct->m_Flags & FLAG_GRAVITY)
				{
					pStruct->m_Flags &= ~FLAG_GOTHRUWORLD;
				}
			}

			return dwRet;
		}

		case MID_INITIALUPDATE:
		{
			if (fData != INITIALUPDATE_SAVEGAME)
			{
				InitialUpdate();
			}

			CacheFiles();
			break;
		}

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


	return BaseClass::EngineMessageFn(messageID, pData, fData);
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	RotatingWorldModel::ObjectMessageFn()
//
//	PURPOSE:	Handler for server object messages.
//
// --------------------------------------------------------------------------- //

DDWORD RotatingWorldModel::ObjectMessageFn(HOBJECT hSender, DDWORD messageID, HMESSAGEREAD hRead)
{
	CServerDE* pServerDE = GetServerDE();
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
	
	return BaseClass::ObjectMessageFn(hSender, messageID, hRead);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	RotatingWorldModel::ReadProp
//
//	PURPOSE:	Set property value
//
// ----------------------------------------------------------------------- //

DBOOL RotatingWorldModel::ReadProp(ObjectCreateStruct *)
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE) return DFALSE;

	pServerDE->GetPropBool("BoxPhysics", &m_bBoxPhysics);

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
	if (!bVal) m_eState = RWM_OFF;
	
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

	long nLongVal;
	pServerDE->GetPropLongInt("DebrisType", &nLongVal);
	m_eSurfaceType = GetDebrisSurfaceType((DebrisType)nLongVal);

	return DTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	RotatingWorldModel::InitialUpdate()
//
//	PURPOSE:	First update
//
// ----------------------------------------------------------------------- //

void RotatingWorldModel::InitialUpdate()
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE) return;

	DDWORD dwUsrFlgs = pServerDE->GetObjectUserFlags(m_hObject);
	pServerDE->SetObjectUserFlags(m_hObject, dwUsrFlgs | USRFLG_MOVEABLE | SurfaceToUserFlag(m_eSurfaceType));

	if (m_eState != RWM_OFF)
	{
		pServerDE->SetNextUpdate(m_hObject, RWM_UPDATE_DELTA);
		SetNormalRotation();
	}

	VEC_COPY(m_vSaveVelocity, m_vVelocity);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	RotatingWorldModel::SetNormalRotation()
//
//	PURPOSE:	Set the model to normal rotation state
//
// ----------------------------------------------------------------------- //

void RotatingWorldModel::SetNormalRotation()
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE) return;

	m_eState = RWM_NORMAL;

	m_fLastTime = pServerDE->GetTime();

	StartSound(m_hstrBusySound, DTRUE);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	RotatingWorldModel::SetOff()
//
//	PURPOSE:	Set the model to off state
//
// ----------------------------------------------------------------------- //

void RotatingWorldModel::SetOff()
{
	CServerDE* pServerDE = GetServerDE();
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
//	ROUTINE:	RotatingWorldModel::SetSpinUp()
//
//	PURPOSE:	Set the model to the spin up state
//
// ----------------------------------------------------------------------- //

void RotatingWorldModel::SetSpinUp()
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE) return;

	m_eState = RWM_SPINUP;

	m_fStartTime = pServerDE->GetTime();

	VEC_COPY(m_vSpinTimeLeft, m_vSpinUpTime);

	StartSound(m_hstrSpinUpSound, DFALSE);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	RotatingWorldModel::SetSpinDown()
//
//	PURPOSE:	Set the model to the spin up state
//
// ----------------------------------------------------------------------- //

void RotatingWorldModel::SetSpinDown()
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE) return;

	m_eState = RWM_SPINDOWN;

	m_fStartTime = pServerDE->GetTime();

	VEC_COPY(m_vSpinTimeLeft, m_vSpinDownTime);

	StartSound(m_hstrSpinDownSound, DFALSE);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	RotatingWorldModel::Update()
//
//	PURPOSE:	Update the model
//
// ----------------------------------------------------------------------- //

void RotatingWorldModel::Update()
{
	CServerDE* pServerDE = GetServerDE();
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

	pServerDE->SetNextUpdate(m_hObject, fUpdateDelta);
	pServerDE->SetDeactivationTime(m_hObject, fUpdateDelta + 1.0f);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	RotatingWorldModel::UpdateNormalRotation()
//
//	PURPOSE:	Update normal rotation
//
// ----------------------------------------------------------------------- //

void RotatingWorldModel::UpdateNormalRotation()
{
	CServerDE* pServerDE = GetServerDE();
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
	pServerDE->SetObjectRotation(m_hObject, &rRot);	

	m_fLastTime = fTime;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	RotatingWorldModel::UpdateSpinUp()
//
//	PURPOSE:	Update spin up 
//
// ----------------------------------------------------------------------- //

void RotatingWorldModel::UpdateSpinUp()
{
	CServerDE* pServerDE = GetServerDE();
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
//	ROUTINE:	RotatingWorldModel::UpdateSpinDown()
//
//	PURPOSE:	Update spin down
//
// ----------------------------------------------------------------------- //

void RotatingWorldModel::UpdateSpinDown()
{
	CServerDE* pServerDE = GetServerDE();
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
//	ROUTINE:	RotatingWorldModel::TriggerMsg()
//
//	PURPOSE:	Handler trigger messages
//
// --------------------------------------------------------------------------- //

void RotatingWorldModel::HandleTrigger(HOBJECT hSender, HSTRING hMsg)
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE) return;

	char* pMsg = pServerDE->GetStringData(hMsg);
	if (!pMsg || !pMsg[0]) return;

	if (m_eState == RWM_OFF && stricmp(pMsg, TRIGGER_MSG_ON) == 0)
	{
		SetSpinUp();
		pServerDE->SetNextUpdate(m_hObject, RWM_UPDATE_DELTA);
	}
	else if (m_eState == RWM_NORMAL && stricmp(pMsg, TRIGGER_MSG_OFF) == 0)
	{
		SetSpinDown();
		pServerDE->SetNextUpdate(m_hObject, RWM_UPDATE_DELTA);
	}
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	RotatingWorldModel::StartSound()
//
//	PURPOSE:	Start the specified sound
//
// --------------------------------------------------------------------------- //

void RotatingWorldModel::StartSound(HSTRING hstrSoundName, DBOOL bLoop)
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
	playSoundInfo.m_dwFlags |= PLAYSOUND_ATTACHED;

	if (bLoop)
	{
		playSoundInfo.m_dwFlags |= PLAYSOUND_LOOP | PLAYSOUND_GETHANDLE;
	}

	strncpy(playSoundInfo.m_szSoundName, pSoundName, _MAX_PATH);
	playSoundInfo.m_hObject = m_hObject;
	playSoundInfo.m_fOuterRadius = m_fSoundRadius;
	playSoundInfo.m_fInnerRadius = 200;

	pServerDE->PlaySound(&playSoundInfo);

	// Save the handle of the sound...

	m_sndLastSound = playSoundInfo.m_hSound;
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	RotatingWorldModel::Save
//
//	PURPOSE:	Save the object
//
// ----------------------------------------------------------------------- //

void RotatingWorldModel::Save(HMESSAGEWRITE hWrite, DDWORD dwSaveFlags)
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE || !hWrite) return;

	pServerDE->WriteToMessageVector(hWrite, &m_vVelocity);	
	pServerDE->WriteToMessageVector(hWrite, &m_vSaveVelocity);	
	pServerDE->WriteToMessageVector(hWrite, &m_vSign);	
	pServerDE->WriteToMessageVector(hWrite, &m_vSpinUpTime);	
	pServerDE->WriteToMessageVector(hWrite, &m_vSpinDownTime);	
	pServerDE->WriteToMessageVector(hWrite, &m_vSpinTimeLeft);	
	pServerDE->WriteToMessageFloat(hWrite, m_fLastTime);	
	pServerDE->WriteToMessageFloat(hWrite, m_fStartTime);	
	pServerDE->WriteToMessageFloat(hWrite, m_fPitch);	
	pServerDE->WriteToMessageFloat(hWrite, m_fYaw);	
	pServerDE->WriteToMessageFloat(hWrite, m_fRoll);	
	pServerDE->WriteToMessageFloat(hWrite, m_fSoundRadius);	
	pServerDE->WriteToMessageByte(hWrite, m_eState);	
	pServerDE->WriteToMessageByte(hWrite, m_bBoxPhysics);	
	pServerDE->WriteToMessageHString(hWrite, m_hstrBusySound);	
	pServerDE->WriteToMessageHString(hWrite, m_hstrSpinUpSound);	
	pServerDE->WriteToMessageHString(hWrite, m_hstrSpinDownSound);	
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	RotatingWorldModel::Load
//
//	PURPOSE:	Load the object
//
// ----------------------------------------------------------------------- //

void RotatingWorldModel::Load(HMESSAGEREAD hRead, DDWORD dwLoadFlags)
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE || !hRead) return;

	pServerDE->ReadFromMessageVector(hRead, &m_vVelocity);	
	pServerDE->ReadFromMessageVector(hRead, &m_vSaveVelocity);	
	pServerDE->ReadFromMessageVector(hRead, &m_vSign);	
	pServerDE->ReadFromMessageVector(hRead, &m_vSpinUpTime);	
	pServerDE->ReadFromMessageVector(hRead, &m_vSpinDownTime);	
	pServerDE->ReadFromMessageVector(hRead, &m_vSpinTimeLeft);	
	m_fLastTime			= pServerDE->ReadFromMessageFloat(hRead);	
	m_fStartTime		= pServerDE->ReadFromMessageFloat(hRead);	
	m_fPitch			= pServerDE->ReadFromMessageFloat(hRead);	
	m_fYaw				= pServerDE->ReadFromMessageFloat(hRead);	
	m_fRoll				= pServerDE->ReadFromMessageFloat(hRead);	
	m_fSoundRadius		= pServerDE->ReadFromMessageFloat(hRead);	
	m_eState			= (RWMState) pServerDE->ReadFromMessageByte(hRead);	
	m_bBoxPhysics		= (DBOOL) pServerDE->ReadFromMessageByte(hRead);	
	m_hstrBusySound		= pServerDE->ReadFromMessageHString(hRead);	
	m_hstrSpinUpSound	= pServerDE->ReadFromMessageHString(hRead);	
	m_hstrSpinDownSound = pServerDE->ReadFromMessageHString(hRead);	
	

	if (m_eState == RWM_NORMAL)
	{
		StartSound(m_hstrBusySound, DTRUE);
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	RotatingWorldModel::CacheFiles
//
//	PURPOSE:	Cache resources associated with this object
//
// ----------------------------------------------------------------------- //

void RotatingWorldModel::CacheFiles()
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE) return;

	char* pFile = DNULL;
	if (m_hstrBusySound)
	{
		pFile = pServerDE->GetStringData(m_hstrBusySound);
		if (pFile)
		{
			pServerDE->CacheFile(FT_SOUND, pFile);
		}
	}

	if (m_hstrSpinUpSound)
	{
		pFile = pServerDE->GetStringData(m_hstrSpinUpSound);
		if (pFile)
		{
			pServerDE->CacheFile(FT_SOUND, pFile);
		}
	}

	if (m_hstrSpinDownSound)
	{
		pFile = pServerDE->GetStringData(m_hstrSpinDownSound);
		if (pFile)
		{
			pServerDE->CacheFile(FT_SOUND, pFile);
		}
	}

}

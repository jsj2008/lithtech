//----------------------------------------------------------
//
// MODULE  : Bouncer.cpp
//
// PURPOSE : Bouncer aggregate
//
// CREATED : 3/18/98
//
//----------------------------------------------------------

#include "Bouncer.h"
#include "cpp_server_de.h"
#include "RiotObjectUtilities.h"


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CBouncer::CBouncer()
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

CBouncer::CBouncer() : Aggregate()
{
	// Set up angular velocities...

	m_fPitchVel			= m_fYawVel = 0.0f;
	m_fPitch			= m_fYaw = 0.0f;
	m_nBounceCount		= 1;
	m_hstrBounceSound	= DNULL; 
	m_hstrBounceSound2	= DNULL; 
	m_bDoneBouncing		= DFALSE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CBouncer::CBouncer()
//
//	PURPOSE:	Destructor
//
// ----------------------------------------------------------------------- //

CBouncer::~CBouncer()
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE) return;

	if (m_hstrBounceSound)
	{
		pServerDE->FreeString(m_hstrBounceSound);
	}

	if (m_hstrBounceSound2)
	{
		pServerDE->FreeString(m_hstrBounceSound2);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CBouncer::EngineMessageFn()
//
//	PURPOSE:	Handle engine messages
//
// ----------------------------------------------------------------------- //

DDWORD CBouncer::EngineMessageFn(LPBASECLASS pObject, DDWORD messageID, void *pData, DFLOAT fData)
{
	switch(messageID)
	{
		case MID_TOUCHNOTIFY:
		{
			HandleTouch(pObject, (HOBJECT)pData);
		}
		break;

		case MID_UPDATE:
		{
			Update(pObject);
		}
		break;

		case MID_INITIALUPDATE:
		{
			int nInfo = (int)fData;
			if (nInfo != INITIALUPDATE_SAVEGAME)
			{
				InitialUpdate(pObject);
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

	return Aggregate::EngineMessageFn(pObject, messageID, pData, fData);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CBouncer::InitialUpdate()
//
//	PURPOSE:	Initialize object
//
// ----------------------------------------------------------------------- //
	
void CBouncer::InitialUpdate(LPBASECLASS pObject)
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE || !pObject) return;

	//m_fPitchVel = pServerDE->Random(-MATH_CIRCLE/2.0f, MATH_CIRCLE/2.0f);
	m_fYawVel	= pServerDE->Random(-MATH_CIRCLE/2.0f, MATH_CIRCLE/2.0f);

	if (!m_hstrBounceSound)
	{
		SetBounceSound("Sounds\\Weapons\\bounce.wav");
	}

	if (!m_hstrBounceSound2)
	{
		SetBounceSound2("Sounds\\Weapons\\bounce2.wav");
	}

	pServerDE->SetForceIgnoreLimit(pObject->m_hObject, 0.0f);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CBouncer::Update()
//
//	PURPOSE:	Handle updating
//
// ----------------------------------------------------------------------- //
	
void CBouncer::Update(LPBASECLASS pObject)
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE || !pObject || m_bDoneBouncing) return;

	// Save position for bounce calculations...

	pServerDE->GetObjectPos(pObject->m_hObject, &m_vLastPos);

	DVector vVel;
	DRotation rRot;

	pServerDE->GetVelocity(pObject->m_hObject , &vVel);
	pServerDE->GetObjectRotation(pObject->m_hObject, &rRot);

	// pServerDE->BPrint("Bouncer Vel: %.2f, %.2f, %.2f", vVel.x, vVel.y, vVel.z);

	// Only spin the object if the velocity is high enough...

	if (VEC_MAG(vVel) > 5.0f)
	{
		if (m_fPitchVel != 0 || m_fYawVel != 0)
		{
			DFLOAT fDeltaTime = pServerDE->GetFrameTime();

			m_fPitch += m_fPitchVel * fDeltaTime;
			m_fYaw += m_fYawVel * fDeltaTime;

			pServerDE->SetupEuler(&rRot, m_fPitch, m_fYaw, 0.0f);
			pServerDE->SetObjectRotation(pObject->m_hObject, &rRot);	
		}
	}
	else
	{
		m_fPitch = 0.0f;
		pServerDE->SetupEuler(&rRot, m_fPitch, m_fYaw, 0.0f);
		pServerDE->SetObjectRotation(pObject->m_hObject, &rRot);
		m_bDoneBouncing = DTRUE;
		m_nBounceCount  = 0;
	}

}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CBouncer::HandleTouch()
//
//	PURPOSE:	Handle object touches
//
// ----------------------------------------------------------------------- //
	
void CBouncer::HandleTouch(LPBASECLASS pObject, HOBJECT hObj)
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE || !pObject || !hObj || (m_nBounceCount <= 0)) return;


	// Return if it hit a non solid object...

	HOBJECT hWorld = pServerDE->GetWorldObject();
	DDWORD dwFlags = pServerDE->GetObjectFlags(hObj);

	if (hObj != hWorld && !(dwFlags & FLAG_SOLID)) return;

	DVector vVel;
	pServerDE->GetVelocity(pObject->m_hObject, &vVel);

	CollisionInfo info;
	pServerDE->GetLastCollision(&info);

	VEC_MULSCALAR(info.m_vStopVel, info.m_vStopVel, -1.6f);
 	VEC_ADD(vVel, vVel, info.m_vStopVel);
	pServerDE->SetVelocity(pObject->m_hObject, &vVel);

// 	pServerDE->BPrint("Bounce Vel: %.2f, %.2f, %.2f", vVel.x, vVel.y, vVel.z);

	// Adjust the bouncing...

	// m_fPitchVel = pServerDE->Random(-MATH_CIRCLE/2.0f, MATH_CIRCLE/2.0f);
	m_fYawVel	= pServerDE->Random(-MATH_CIRCLE/2.0f, MATH_CIRCLE/2.0f);

	// Play a bounce sound...
			
	if (m_hstrBounceSound)
	{
		char* pSound = pServerDE->GetStringData(m_hstrBounceSound);
		if (m_hstrBounceSound2)
		{ 
			if (GetRandom(0,1) == 0) pSound = pServerDE->GetStringData(m_hstrBounceSound2);
		}

		DVector vPos;
		pServerDE->GetObjectPos( pObject->m_hObject, &vPos );
		PlaySoundFromPos( &vPos, pSound, 1500.0f, SOUNDPRIORITY_MISC_MEDIUM );
	}

	m_nBounceCount--;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CBouncer::SetBounceSound()
//
//	PURPOSE:	Set the bounce sound
//
// ----------------------------------------------------------------------- //
	
void CBouncer::SetBounceSound(char* pSound)
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE || !pSound) return;

	if (m_hstrBounceSound)
	{
		pServerDE->FreeString(m_hstrBounceSound);
	}

	m_hstrBounceSound = pServerDE->CreateString(pSound);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CBouncer::SetBounceSound2()
//
//	PURPOSE:	Set the 2nd bounce sound
//
// ----------------------------------------------------------------------- //
	
void CBouncer::SetBounceSound2(char* pSound)
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE || !pSound) return;

	if (m_hstrBounceSound2)
	{
		pServerDE->FreeString(m_hstrBounceSound2);
	}

	m_hstrBounceSound2 = pServerDE->CreateString(pSound);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CBouncer::Save
//
//	PURPOSE:	Save the object
//
// ----------------------------------------------------------------------- //

void CBouncer::Save(HMESSAGEWRITE hWrite, DDWORD dwSaveFlags)
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE || !hWrite) return;

	pServerDE->WriteToMessageFloat(hWrite, m_fPitchVel);
	pServerDE->WriteToMessageFloat(hWrite, m_fYawVel);
	pServerDE->WriteToMessageFloat(hWrite, m_fPitch);
	pServerDE->WriteToMessageFloat(hWrite, m_fYaw);
	pServerDE->WriteToMessageDWord(hWrite, m_nBounceCount);
	pServerDE->WriteToMessageByte(hWrite, m_bDoneBouncing);
	pServerDE->WriteToMessageHString(hWrite, m_hstrBounceSound);
	pServerDE->WriteToMessageHString(hWrite, m_hstrBounceSound2);
	pServerDE->WriteToMessageVector(hWrite, &m_vLastPos);
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CBouncer::Load
//
//	PURPOSE:	Load the object
//
// ----------------------------------------------------------------------- //

void CBouncer::Load(HMESSAGEREAD hRead, DDWORD dwLoadFlags)
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE || !hRead) return;

	m_fPitchVel			= pServerDE->ReadFromMessageFloat(hRead);
	m_fYawVel			= pServerDE->ReadFromMessageFloat(hRead);
	m_fPitch			= pServerDE->ReadFromMessageFloat(hRead);
	m_fYaw				= pServerDE->ReadFromMessageFloat(hRead);
	m_nBounceCount		= pServerDE->ReadFromMessageDWord(hRead);
	m_bDoneBouncing		= pServerDE->ReadFromMessageByte(hRead);
	m_hstrBounceSound	= pServerDE->ReadFromMessageHString(hRead);
	m_hstrBounceSound2	= pServerDE->ReadFromMessageHString(hRead);
	pServerDE->ReadFromMessageVector(hRead, &m_vLastPos);
}
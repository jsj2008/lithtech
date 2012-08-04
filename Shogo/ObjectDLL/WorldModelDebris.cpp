// ----------------------------------------------------------------------- //
//
// MODULE  : WorldModelDebris.cpp
//
// PURPOSE : A WorldModelDebris object
//
// CREATED : 2/27/98
//
// ----------------------------------------------------------------------- //

// Includes...
#include "WorldModelDebris.h"
#include "RiotObjectUtilities.h"
#include <stdio.h>

BEGIN_CLASS(WorldModelDebris)
END_CLASS_DEFAULT(WorldModelDebris, DestructableDoor, NULL, NULL)


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	WorldModelDebris::WorldModelDebris()
//
//	PURPOSE:	Initialize object
//
// ----------------------------------------------------------------------- //

WorldModelDebris::WorldModelDebris() : DestructableDoor()
{
	m_bRotate	= DTRUE;
	m_fXRotVel	= 0.0f;
	m_fYRotVel	= 0.0f;
	m_fZRotVel	= 0.0f;
	m_fLastTime = 0.0f;
	m_fPitch    = 0.0f;
	m_fYaw      = 0.0f;
	m_fRoll		= 0.0f;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	WorldModelDebris::Setup
//
//	PURPOSE:	Set up a WorldModelDebris with the information needed
//
// ----------------------------------------------------------------------- //

void WorldModelDebris::Start(DVector *pvRotationPeriods, DVector* pvVel)
{
	if (!pvRotationPeriods || !pvVel) return;

	DFLOAT fMag;
	fMag = VEC_MAGSQR(*pvRotationPeriods);
	if (fMag > 0.001f)
	{
		m_bRotate = DTRUE;
		
		if ( pvRotationPeriods->x < -0.001 || 0.001f < pvRotationPeriods->x )
			m_fXRotVel = MATH_CIRCLE / pvRotationPeriods->x;
		if ( pvRotationPeriods->y < -0.001 || 0.001f < pvRotationPeriods->y )
			m_fXRotVel = MATH_CIRCLE / pvRotationPeriods->y;
		if ( pvRotationPeriods->z < -0.001 || 0.001f < pvRotationPeriods->z )
			m_fXRotVel = MATH_CIRCLE / pvRotationPeriods->z;
	}

	DDWORD dwFlags = FLAG_VISIBLE | FLAG_GRAVITY | FLAG_RAYHIT;
	g_pServerDE->SetObjectFlags(m_hObject, dwFlags);
	g_pServerDE->SetNextUpdate(m_hObject, 0.01f);
	g_pServerDE->SetVelocity(m_hObject, pvVel);
	g_pServerDE->SetBlockingPriority(m_hObject, 100);
	g_pServerDE->SetForceIgnoreLimit(m_hObject, 0.0f);
	g_pServerDE->SetFrictionCoefficient(m_hObject, GetRandom(10.0f, 20.0f));

	m_fLastTime = g_pServerDE->GetTime();
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	WorldModelDebris::EngineMessageFn()
//
//	PURPOSE:	Handler for engine messages
//
// --------------------------------------------------------------------------- //

DDWORD WorldModelDebris::EngineMessageFn(DDWORD messageID, void *pData, float fData)
{
	switch (messageID)
	{
		case MID_UPDATE:
		{
			Update();
			break;
		}

		case MID_PRECREATE:
		{
			if ( fData == PRECREATE_WORLDFILE )
				ReadProp(( ObjectCreateStruct * )pData);
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

	return DestructableDoor::EngineMessageFn(messageID, pData, fData);
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	WorldModelDebris::ReadProp()
//
//	PURPOSE:	Reads WorldModelDebris properties
//
// --------------------------------------------------------------------------- //

void WorldModelDebris::ReadProp(ObjectCreateStruct *pStruct)
{
	DVector vAngles;

	if( g_pServerDE->GetPropRotationEuler( "Rotation", &vAngles ) == DE_OK )
	{

		// Set initial pitch, yaw, roll...
		m_fPitch = vAngles.x;
		m_fYaw   = vAngles.y;
		m_fRoll  = vAngles.z;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	WorldModelDebris::Update
//
//	PURPOSE:	Update the WorldModelDebris
//
// ----------------------------------------------------------------------- //

void WorldModelDebris::Update()
{
	CollisionInfo standingInfo;

	if (m_bRotate) 
	{
		g_pServerDE->SetNextUpdate(m_hObject, 0.01f);
		g_pServerDE->GetStandingOn(m_hObject, &standingInfo);
			
		if (standingInfo.m_hObject) 
		{
			m_bRotate = DFALSE;
		}
		else 
		{
			UpdateRotation();
		}
	}
	else
	{
		g_pServerDE->SetNextUpdate(m_hObject, 0.0f);
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	WorldModelDebris::UpdateRotation
//
//	PURPOSE:	Update Rotation
//
// ----------------------------------------------------------------------- //

void WorldModelDebris::UpdateRotation()
{
	DFLOAT fTime	  = g_pServerDE->GetTime();
	DFLOAT fDeltaTime = fTime - m_fLastTime;

	DRotation rRot;
	g_pServerDE->GetObjectRotation(m_hObject, &rRot);

	if ( m_fXRotVel < 0.0f || 0.0f < m_fXRotVel )
		m_fPitch += m_fXRotVel * fDeltaTime;
	if ( m_fYRotVel < 0.0f || 0.0f < m_fYRotVel )
		m_fYaw += m_fYRotVel * fDeltaTime;
	if ( m_fZRotVel < 0.0f || 0.0f < m_fZRotVel )
		m_fRoll += m_fZRotVel * fDeltaTime;

	g_pServerDE->SetupEuler(&rRot, m_fPitch, m_fYaw, m_fRoll);
	g_pServerDE->SetObjectRotation(m_hObject, &rRot);	

	m_fLastTime = fTime;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	WorldModelDebris::Save
//
//	PURPOSE:	Save the object
//
// ----------------------------------------------------------------------- //

void WorldModelDebris::Save(HMESSAGEWRITE hWrite, DDWORD dwSaveFlags)
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE || !hWrite) return;

	pServerDE->WriteToMessageByte(hWrite, m_bRotate);
	pServerDE->WriteToMessageFloat(hWrite, m_fXRotVel);
	pServerDE->WriteToMessageFloat(hWrite, m_fYRotVel);
	pServerDE->WriteToMessageFloat(hWrite, m_fZRotVel);
	pServerDE->WriteToMessageFloat(hWrite, m_fPitch);
	pServerDE->WriteToMessageFloat(hWrite, m_fYaw);
	pServerDE->WriteToMessageFloat(hWrite, m_fRoll);
	pServerDE->WriteToMessageFloat(hWrite, m_fLastTime);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	WorldModelDebris::Load
//
//	PURPOSE:	Load the object
//
// ----------------------------------------------------------------------- //

void WorldModelDebris::Load(HMESSAGEREAD hRead, DDWORD dwLoadFlags)
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE || !hRead) return;

	m_bRotate	= pServerDE->ReadFromMessageByte(hRead);
	m_fXRotVel	= pServerDE->ReadFromMessageFloat(hRead);
	m_fYRotVel	= pServerDE->ReadFromMessageFloat(hRead);
	m_fZRotVel	= pServerDE->ReadFromMessageFloat(hRead);
	m_fPitch	= pServerDE->ReadFromMessageFloat(hRead);
	m_fYaw		= pServerDE->ReadFromMessageFloat(hRead);
	m_fRoll		= pServerDE->ReadFromMessageFloat(hRead);
	m_fLastTime = pServerDE->ReadFromMessageFloat(hRead);
}
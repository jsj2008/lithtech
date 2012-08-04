// ----------------------------------------------------------------------- //
//
// MODULE  : WorldModelDebris.cpp
//
// PURPOSE : A WorldModelDebris object
//
// CREATED : 2/27/98
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "WorldModelDebris.h"
#include "ServerUtilities.h"
#include <stdio.h>

BEGIN_CLASS(WorldModelDebris)
END_CLASS_DEFAULT(WorldModelDebris, Door, NULL, NULL)


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	WorldModelDebris::WorldModelDebris()
//
//	PURPOSE:	Initialize object
//
// ----------------------------------------------------------------------- //

WorldModelDebris::WorldModelDebris() : Door()
{
    m_bRotate   = LTTRUE;
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

void WorldModelDebris::Start(LTVector *pvRotationPeriods, LTVector* pvVel)
{
	if (!pvRotationPeriods || !pvVel) return;

    LTFLOAT fMag = VEC_MAGSQR(*pvRotationPeriods);
	if (fMag > 0.001f)
	{
        m_bRotate = LTTRUE;

		if (pvRotationPeriods->x < -0.001 || 0.001f < pvRotationPeriods->x)
		{
			m_fXRotVel = MATH_CIRCLE / pvRotationPeriods->x;
		}
		if (pvRotationPeriods->y < -0.001 || 0.001f < pvRotationPeriods->y)
		{
			m_fXRotVel = MATH_CIRCLE / pvRotationPeriods->y;
		}
		if (pvRotationPeriods->z < -0.001 || 0.001f < pvRotationPeriods->z)
		{
			m_fXRotVel = MATH_CIRCLE / pvRotationPeriods->z;
		}
	}

    uint32 dwFlags = FLAG_VISIBLE | FLAG_GRAVITY | FLAG_RAYHIT;
    g_pLTServer->SetObjectFlags(m_hObject, dwFlags);
    g_pLTServer->SetNextUpdate(m_hObject, 0.01f);
    g_pLTServer->SetVelocity(m_hObject, pvVel);
    g_pLTServer->SetBlockingPriority(m_hObject, 100);
    g_pLTServer->SetForceIgnoreLimit(m_hObject, 0.0f);
    g_pLTServer->SetFrictionCoefficient(m_hObject, GetRandom(10.0f, 20.0f));

    m_fLastTime = g_pLTServer->GetTime();
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	WorldModelDebris::EngineMessageFn()
//
//	PURPOSE:	Handler for engine messages
//
// --------------------------------------------------------------------------- //

uint32 WorldModelDebris::EngineMessageFn(uint32 messageID, void *pData, float fData)
{
	switch (messageID)
	{
		case MID_UPDATE:
		{
			Update();
		}
		break;

		case MID_PRECREATE:
		{
			if (fData == PRECREATE_WORLDFILE)
			{
				ReadProp((ObjectCreateStruct*)pData);
			}
		}
		break;

		case MID_SAVEOBJECT:
		{
            Save((HMESSAGEWRITE)pData, (uint32)fData);
		}
		break;

		case MID_LOADOBJECT:
		{
            Load((HMESSAGEREAD)pData, (uint32)fData);
		}
		break;

		default : break;
	}

	return Door::EngineMessageFn(messageID, pData, fData);
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
    LTVector vAngles;

    if (g_pLTServer->GetPropRotationEuler("Rotation", &vAngles) == LT_OK)
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
        g_pLTServer->SetNextUpdate(m_hObject, 0.01f);
        g_pLTServer->GetStandingOn(m_hObject, &standingInfo);

		if (standingInfo.m_hObject)
		{
            m_bRotate = LTFALSE;
		}
		else
		{
			UpdateRotation();
		}
	}
	else
	{
        g_pLTServer->SetNextUpdate(m_hObject, 0.0f);
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
    LTFLOAT fTime      = g_pLTServer->GetTime();
    LTFLOAT fDeltaTime = fTime - m_fLastTime;

    LTRotation rRot;
    g_pLTServer->GetObjectRotation(m_hObject, &rRot);

	if (m_fXRotVel < 0.0f || 0.0f < m_fXRotVel)
	{
		m_fPitch += m_fXRotVel * fDeltaTime;
	}
	if (m_fYRotVel < 0.0f || 0.0f < m_fYRotVel)
	{
		m_fYaw += m_fYRotVel * fDeltaTime;
	}
	if (m_fZRotVel < 0.0f || 0.0f < m_fZRotVel)
	{
		m_fRoll += m_fZRotVel * fDeltaTime;
	}

    g_pLTServer->SetupEuler(&rRot, m_fPitch, m_fYaw, m_fRoll);
    g_pLTServer->SetObjectRotation(m_hObject, &rRot);

	m_fLastTime = fTime;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	WorldModelDebris::Save
//
//	PURPOSE:	Save the object
//
// ----------------------------------------------------------------------- //

void WorldModelDebris::Save(HMESSAGEWRITE hWrite, uint32 dwSaveFlags)
{
    ILTServer* pServerDE = GetServerDE();
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

void WorldModelDebris::Load(HMESSAGEREAD hRead, uint32 dwLoadFlags)
{
    ILTServer* pServerDE = GetServerDE();
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
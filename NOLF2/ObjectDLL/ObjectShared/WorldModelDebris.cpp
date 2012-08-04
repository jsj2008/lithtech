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

LINKFROM_MODULE( WorldModelDebris );

#pragma force_active on
BEGIN_CLASS(WorldModelDebris)
END_CLASS_DEFAULT_FLAGS(WorldModelDebris, Door, NULL, NULL, CF_WORLDMODEL)
#pragma force_active off


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
	g_pCommonLT->SetObjectFlags(m_hObject, OFT_Flags, dwFlags, FLAGMASK_ALL);
    SetNextUpdate(UPDATE_NEXT_FRAME);
	g_pPhysicsLT->SetVelocity(m_hObject, (LTVector*)pvVel);
    g_pLTServer->SetBlockingPriority(m_hObject, 100);
    g_pPhysicsLT->SetForceIgnoreLimit(m_hObject, 0.0f);
    g_pPhysicsLT->SetFrictionCoefficient(m_hObject, GetRandom(10.0f, 20.0f));

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
            Save((ILTMessage_Write*)pData, (uint32)fData);
		}
		break;

		case MID_LOADOBJECT:
		{
            Load((ILTMessage_Read*)pData, (uint32)fData);
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
        SetNextUpdate(UPDATE_NEXT_FRAME);
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
        SetNextUpdate(UPDATE_NEVER);
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

    rRot = LTRotation(m_fPitch, m_fYaw, m_fRoll);
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

void WorldModelDebris::Save(ILTMessage_Write *pMsg, uint32 dwSaveFlags)
{
	if (!pMsg) return;

	SAVE_BOOL(m_bRotate);
	SAVE_FLOAT(m_fXRotVel);
	SAVE_FLOAT(m_fYRotVel);
	SAVE_FLOAT(m_fZRotVel);
	SAVE_FLOAT(m_fPitch);
	SAVE_FLOAT(m_fYaw);
	SAVE_FLOAT(m_fRoll);
	SAVE_TIME(m_fLastTime);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	WorldModelDebris::Load
//
//	PURPOSE:	Load the object
//
// ----------------------------------------------------------------------- //

void WorldModelDebris::Load(ILTMessage_Read *pMsg, uint32 dwLoadFlags)
{
	if (!pMsg) return;

	LOAD_BOOL(m_bRotate);
	LOAD_FLOAT(m_fXRotVel);
	LOAD_FLOAT(m_fYRotVel);
	LOAD_FLOAT(m_fZRotVel);
	LOAD_FLOAT(m_fPitch);
	LOAD_FLOAT(m_fYaw);
	LOAD_FLOAT(m_fRoll);
	LOAD_TIME(m_fLastTime);
}
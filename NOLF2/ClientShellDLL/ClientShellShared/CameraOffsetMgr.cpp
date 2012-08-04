// ----------------------------------------------------------------------- //
//
// MODULE  : CameraOffsetMgr.cpp
//
// PURPOSE : Camera offset mgr - Implementation
//
// CREATED : 8/23/99
//
// (c) 1999-2000 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "CameraOffsetMgr.h"
#include "VarTrack.h"
#include "GameClientShell.h"

VarTrack	g_vtCamMaxPitchOffset;
VarTrack	g_vtCamMaxYawOffset;
VarTrack	g_vtCamMaxRollOffset;
VarTrack	g_vtCamMaxPosXOffset;
VarTrack	g_vtCamMaxPosYOffset;
VarTrack	g_vtCamMaxPosZOffset;
VarTrack	g_vtCamInfo;

// Testing var trackers...

VarTrack	g_vtCamWeaponImpact;

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CCameraOffsetMgr::CCameraOffsetMgr
//
//	PURPOSE:	Constructor
//
// --------------------------------------------------------------------------- //

CCameraOffsetMgr::CCameraOffsetMgr()
{
	m_vPitchYawRollDelta.Init();
	m_vPosDelta.Init();
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CCameraOffsetMgr::Init
//
//	PURPOSE:	Init
//
// --------------------------------------------------------------------------- //

LTBOOL CCameraOffsetMgr::Init()
{
	m_vPitchYawRollDelta.Init();
	m_vPosDelta.Init();

    g_vtCamMaxPitchOffset.Init(g_pLTClient, "CamMaxPitchOffset", NULL, 15.0);
    g_vtCamMaxYawOffset.Init(g_pLTClient, "CamMaxYawOffset", NULL, 15.0);
    g_vtCamMaxRollOffset.Init(g_pLTClient, "CamMaxRollOffset", NULL, 15.0);
    g_vtCamMaxPosXOffset.Init(g_pLTClient, "CamMaxPosXOffset", NULL, 200.0);
    g_vtCamMaxPosYOffset.Init(g_pLTClient, "CamMaxPosYOffset", NULL, 200.0);
    g_vtCamMaxPosZOffset.Init(g_pLTClient, "CamMaxPosZOffset", NULL, 200.0);
	g_vtCamInfo.Init(g_pLTClient, "CamInfo", NULL, 0.0f);

    g_vtCamWeaponImpact.Init(g_pLTClient, "CamWeaponImpact", NULL, 0.0);

    return LTTRUE;
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CCameraOffsetMgr::Update
//
//	PURPOSE:	Update all our deltas
//
// --------------------------------------------------------------------------- //

void CCameraOffsetMgr::Update()
{
	// Testing only...
	ProcessTestingVars();
	// End Testing only...


	// Reset offsets...

	m_vPitchYawRollDelta.Init();
	m_vPosDelta.Init();

    float fTimeDelta = g_pGameClientShell->GetFrameTime();

    int i;
    for (i=0; i < MAX_CAMERA_DELTAS; i++)
	{
		if (m_CameraDeltas[i].GetTotalDelta() > 0.0f)
		{
			m_CameraDeltas[i].Pitch.UpdateVar(fTimeDelta);
			m_vPitchYawRollDelta.x += m_CameraDeltas[i].Pitch.GetValue();

			m_CameraDeltas[i].Yaw.UpdateVar(fTimeDelta);
			m_vPitchYawRollDelta.y += m_CameraDeltas[i].Yaw.GetValue();

			m_CameraDeltas[i].Roll.UpdateVar(fTimeDelta);
			m_vPitchYawRollDelta.z += m_CameraDeltas[i].Roll.GetValue();

			m_CameraDeltas[i].PosX.UpdateVar(fTimeDelta);
			m_vPosDelta.x += m_CameraDeltas[i].PosX.GetValue();

			m_CameraDeltas[i].PosY.UpdateVar(fTimeDelta);
			m_vPosDelta.y += m_CameraDeltas[i].PosY.GetValue();

			m_CameraDeltas[i].PosZ.UpdateVar(fTimeDelta);
			m_vPosDelta.z += m_CameraDeltas[i].PosZ.GetValue();
		}
	}

	for (i=0; i < MAX_STATIC_CAMERA_DELTAS; i++)
	{
		if (m_StaticCameraDeltas[i].GetTotalDelta() > 0.0f)
		{
			m_StaticCameraDeltas[i].Pitch.UpdateVar(fTimeDelta);
			m_vPitchYawRollDelta.x += m_StaticCameraDeltas[i].Pitch.GetValue();

			m_StaticCameraDeltas[i].Yaw.UpdateVar(fTimeDelta);
			m_vPitchYawRollDelta.y += m_StaticCameraDeltas[i].Yaw.GetValue();

			m_StaticCameraDeltas[i].Roll.UpdateVar(fTimeDelta);
			m_vPitchYawRollDelta.z += m_StaticCameraDeltas[i].Roll.GetValue();

			m_StaticCameraDeltas[i].PosX.UpdateVar(fTimeDelta);
			m_vPosDelta.x += m_StaticCameraDeltas[i].PosX.GetValue();

			m_StaticCameraDeltas[i].PosY.UpdateVar(fTimeDelta);
			m_vPosDelta.y += m_StaticCameraDeltas[i].PosY.GetValue();

			m_StaticCameraDeltas[i].PosZ.UpdateVar(fTimeDelta);
			m_vPosDelta.z += m_StaticCameraDeltas[i].PosZ.GetValue();
		}
	}

	ValidateDeltas();


	// Print out our current values...

	if (g_vtCamInfo.GetFloat())
	{
		if (m_vPitchYawRollDelta.x != 0.0f)
			g_pLTClient->CPrint("COM Pitch = %.4f (in Deg = %.2f)", m_vPitchYawRollDelta.x, RAD2DEG(m_vPitchYawRollDelta.x));
		if (m_vPitchYawRollDelta.y != 0.0f)
			g_pLTClient->CPrint("COM Yaw   = %.4f (in Deg = %.2f)", m_vPitchYawRollDelta.y, RAD2DEG(m_vPitchYawRollDelta.y));
		if (m_vPitchYawRollDelta.z != 0.0f)
			g_pLTClient->CPrint("COM Roll  = %.4f (in Deg = %.2f)", m_vPitchYawRollDelta.z, RAD2DEG(m_vPitchYawRollDelta.z));

		if (m_vPosDelta.x != 0.0f)
			g_pLTClient->CPrint("COM Offset X = %.2f", m_vPosDelta.x);
		if (m_vPosDelta.y != 0.0f)
			g_pLTClient->CPrint("COM Offset Y = %.2f", m_vPosDelta.y);
		if (m_vPosDelta.z != 0.0f)
			g_pLTClient->CPrint("COM Offset Z = %.2f", m_vPosDelta.z);
	}
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CCameraOffsetMgr::ValidateDeltas
//
//	PURPOSE:	Make sure all the deltas are valid
//
// --------------------------------------------------------------------------- //

void CCameraOffsetMgr::ValidateDeltas()
{
	float fMaxVal = g_vtCamMaxPitchOffset.GetFloat();
	if (m_vPitchYawRollDelta.x > fMaxVal)
	{
		m_vPitchYawRollDelta.x = fMaxVal;
	}
	else if (m_vPitchYawRollDelta.x < -fMaxVal)
	{
		m_vPitchYawRollDelta.x = -fMaxVal;
	}

	fMaxVal = g_vtCamMaxYawOffset.GetFloat();
	if (m_vPitchYawRollDelta.y > fMaxVal)
	{
		m_vPitchYawRollDelta.y = fMaxVal;
	}
	else if (m_vPitchYawRollDelta.y < -fMaxVal)
	{
		m_vPitchYawRollDelta.y = -fMaxVal;
	}

	fMaxVal = g_vtCamMaxRollOffset.GetFloat();
	if (m_vPitchYawRollDelta.z > fMaxVal)
	{
		m_vPitchYawRollDelta.z = fMaxVal;
	}
	else if (m_vPitchYawRollDelta.z < -fMaxVal)
	{
		m_vPitchYawRollDelta.z = -fMaxVal;
	}

	fMaxVal = g_vtCamMaxPosXOffset.GetFloat();
	if (m_vPosDelta.x > fMaxVal)
	{
		m_vPosDelta.x = fMaxVal;
	}
	else if (m_vPosDelta.x < -fMaxVal)
	{
		m_vPosDelta.x = -fMaxVal;
	}

	fMaxVal = g_vtCamMaxPosYOffset.GetFloat();
	if (m_vPosDelta.y > fMaxVal)
	{
		m_vPosDelta.y = fMaxVal;
	}
	else if (m_vPosDelta.y < -fMaxVal)
	{
		m_vPosDelta.y = -fMaxVal;
	}

	fMaxVal = g_vtCamMaxPosZOffset.GetFloat();
	if (m_vPosDelta.z > fMaxVal)
	{
		m_vPosDelta.z = fMaxVal;
	}
	else if (m_vPosDelta.z < -fMaxVal)
	{
		m_vPosDelta.z = -fMaxVal;
	}
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CCameraOffsetMgr::AddDelta
//
//	PURPOSE:	Add a new delta
//
// --------------------------------------------------------------------------- //

void CCameraOffsetMgr::AddDelta(CameraDelta & delta)
{
	// Find an open slot for the new delta...

	float fMinTotalDelta = 100000.0f;
	int nSlot = 0;
	for (int i=0; i < MAX_CAMERA_DELTAS; i++)
	{
		float fTotalDelta = m_CameraDeltas[i].GetTotalDelta();

		if (fTotalDelta == 0.0f)
		{
			m_CameraDeltas[i] = delta;
			return;
		}
		else
		{
			if (fTotalDelta < fMinTotalDelta)
			{
				fMinTotalDelta = fTotalDelta;
				nSlot = i;
			}
		}
	}

	// Override the slot with the least amount of change...

	if (delta.GetTotalDelta() > fMinTotalDelta)
	{
		m_CameraDeltas[nSlot] = delta;
	}
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CCameraOffsetMgr::SetStaticDelta
//
//	PURPOSE:	Set a static delta
//
// --------------------------------------------------------------------------- //

void CCameraOffsetMgr::SetStaticDelta(CameraDelta & delta, int nIndex)
{
	if (nIndex < 0 || nIndex >= MAX_STATIC_CAMERA_DELTAS) return;

	m_StaticCameraDeltas[nIndex] = delta;
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CCameraOffsetMgr::SetStaticDelta
//
//	PURPOSE:	Set a static delta
//
// --------------------------------------------------------------------------- //

CameraDelta* CCameraOffsetMgr::GetStaticDelta(int nIndex)
{
	if (nIndex < 0 || nIndex >= MAX_STATIC_CAMERA_DELTAS) return LTNULL;

	return &(m_StaticCameraDeltas[nIndex]);
}



// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CameraAdjustVar::UpdateVar
//
//	PURPOSE:	Update the variable value
//
// --------------------------------------------------------------------------- //

void CameraAdjustVar::UpdateVar(float fTimeDelta)
{
	if (fVar == 0.0f) return;

	// Figure out the direction we are going...

	if (m_fCurTime <= 0.0f)
	{
		if (fTime1 > 0.0f)
		{
			m_nDir = 1;
			m_fLastRealValue = 0.0f;
		}
		else if (fTime2 > 0.0f)
		{
			m_nDir = -1;
			m_fLastRealValue = fVar;
		}
		else
		{
			return;
		}
	}


	// Determine percent of time gone by...

	float fPercent = 0.0f;
	WaveType eType = eWave1;

	if (m_nDir == 1)
	{
		eType = eWave1;
		fPercent = m_fCurTime / fTime1;
		m_fRealValue = fVar * GetWaveFn(eType)(fPercent);
	}
	else
	{
		eType = eWave2;
		fPercent = m_fCurTime / fTime2;
		m_fRealValue = fVar - (fVar * GetWaveFn(eType)(fPercent));
	}


	// Set our variable value as an increment from the last value...

	if (bIncrement)
	{
		m_fValue = m_fRealValue - m_fLastRealValue;
		m_fLastRealValue = m_fRealValue;
	}
	else // Normal calculation...
	{
		m_fValue = m_fRealValue;
	}

    // g_pLTClient->CPrint("m_fValue = %.2f (in Deg = %.2f)", m_fValue, RAD2DEG(m_fValue));

	// Calculate new value...

	m_fCurTime += fTimeDelta;

	if (m_nDir == 1 && m_fCurTime > fTime1)
	{
		m_fCurTime = 0.0f;
		fTime1 = 0.0f;
	}
	else if (m_nDir == -1 && m_fCurTime > fTime2)
	{
		// We're done, so clear everything...

		Init();
	}
}




// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CCameraOffsetMgr::ProcessTestingVars
//
//	PURPOSE:	Process testing console vars...
//
// --------------------------------------------------------------------------- //

void CCameraOffsetMgr::ProcessTestingVars()
{
	CameraDelta delta;

	// See if any testing vars were set...

	if (g_vtCamWeaponImpact.GetFloat() > 0.0f)
	{
		g_vtCamWeaponImpact.SetFloat(0.0f);

		switch(GetRandom(1, 3))
		{
			case 1:
			{
				delta.Pitch.fVar	= GetRandom(1, 2) == 1 ? -DEG2RAD(5.0f) : DEG2RAD(5.0f);
				delta.Pitch.fTime1	= 0.1f;
				delta.Pitch.fTime2	= 0.25f;
				delta.Pitch.eWave1	= Wave_SlowOff;
				delta.Pitch.eWave2	= Wave_SlowOff;

				g_pLTClient->CPrint("Test Impact Pitch = %.4f (in Deg = %.2f)", delta.Pitch.fVar, RAD2DEG(delta.Pitch.fVar));
			}
			break;

			case 2 :
			{
				delta.Yaw.fVar	= GetRandom(1, 2) == 1 ? -DEG2RAD(5.0f) : DEG2RAD(5.0f);
				delta.Yaw.fTime1	= 0.1f;
				delta.Yaw.fTime2	= 0.25f;
				delta.Yaw.eWave1	= Wave_SlowOff;
				delta.Yaw.eWave2	= Wave_SlowOff;

				g_pLTClient->CPrint("Test Impact Yaw = %.4f (in Deg = %.2f)", delta.Yaw.fVar, RAD2DEG(delta.Yaw.fVar));
			}
			break;

			default :
			case 3 :
			{
				delta.Roll.fVar	= GetRandom(1, 2) == 1 ? -DEG2RAD(5.0f) : DEG2RAD(5.0f);
				delta.Roll.fTime1	= 0.1f;
				delta.Roll.fTime2	= 0.25f;
				delta.Roll.eWave1	= Wave_SlowOff;
				delta.Roll.eWave2	= Wave_SlowOff;

				g_pLTClient->CPrint("Test Impact Roll = %.4f (in Deg = %.2f)", delta.Roll.fVar, RAD2DEG(delta.Roll.fVar));
			}
			break;
		}

		AddDelta(delta);
	}
}
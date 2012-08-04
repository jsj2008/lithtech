// ----------------------------------------------------------------------- //
//
// MODULE  : PlayerCamera.cpp
//
// PURPOSE : PlayerCamera - Implementation
//
// CREATED : 10/5/97
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "PlayerCamera.h"
#include "iltclient.h"
#include <stdio.h>
#include "GameClientShell.h"
#include "VarTrack.h"
#include "VehicleMgr.h"

VarTrack		g_vtCameraMoveUpTime;
VarTrack		g_vtCameraMoveUpMinVel;
VarTrack		g_vtCameraMaxYDifference;
VarTrack		g_vtCameraClipDistance;

// Camera's distance back from the player's position
#define DEFAULT_CAMERA_DIST_BACK	110
// Camera's distance up from the player's position
#define DEFAULT_CAMERA_DIST_UP		40
// Camera's distance back from the player's position but at a 45 degree angle
// (for example, if the camera is looking at the player from the southeast position)
#define DEFAULT_CAMERA_DIST_DIAG	106

// Camera's X offset from the player when it is in the MLOOK state
#define DEFAULT_CAMERA_DIST_MLOOK_X	0
// Camera's Y offset from the player when it is in the MLOOK state
#define DEFAULT_CAMERA_DIST_MLOOK_Y 10
// Camera's Z offset from the player when it is in the MLOOK state
#define DEFAULT_CAMERA_DIST_MLOOK_Z 29

extern CGameClientShell*	g_pGameClientShell;

LTBOOL Equal(LTVector & v1, LTVector & v2)
{
    LTVector v;
	VEC_SUB(v, v1, v2);
    return LTBOOL(VEC_MAG(v) < 1.0f);
}

LTBOOL Equal(LTRotation & r1, LTRotation & r2)
{
    LTBOOL bRet = LTTRUE;

	if (r1.m_Quat[0] != r2.m_Quat[0] ||
		r1.m_Quat[1] != r2.m_Quat[1] ||
		r1.m_Quat[2] != r2.m_Quat[2] ||
		r1.m_Quat[3] != r2.m_Quat[3])
	{
        bRet = LTFALSE;
	}

	return bRet;
}

void CalcNonClipPos(LTVector & vPos, LTRotation & rRot)
{
	LTVector vTemp, vU, vR, vF;
	g_pLTClient->GetRotationVectors(&rRot, &vU, &vR, &vF);

	// Make sure we aren't clipping into walls...

    LTFLOAT fClipDistance = g_vtCameraClipDistance.GetFloat();


	// Check for walls to the right...

	vTemp = (vR * fClipDistance);
	vTemp += vPos;

	ClientIntersectQuery iQuery;
	ClientIntersectInfo  iInfo;

	iQuery.m_From = vPos;
	iQuery.m_To   = vTemp;

	if (g_pLTClient->IntersectSegment(&iQuery, &iInfo))
	{
		vTemp = iInfo.m_Point - vPos;
        LTFLOAT fDist = (fClipDistance - vTemp.Mag());

		vTemp = (vR * -fDist);
		vPos += vTemp;
	}
	else
	{
	 	// If we didn't adjust for a wall to the right, check walls to the left...

		vTemp = (vR * -fClipDistance);
		vTemp += vPos;

		iQuery.m_From = vPos;
		iQuery.m_To = vTemp;

		if (g_pLTClient->IntersectSegment(&iQuery, &iInfo))
		{
			vTemp = iInfo.m_Point - vPos;
            LTFLOAT fDist = (fClipDistance - vTemp.Mag());

			vTemp = (vR * fDist);
			vPos += vTemp;
		}
	}


	// Check for ceilings...

	vTemp = vU * fClipDistance;
	vTemp += vPos;

	iQuery.m_From = vPos;
	iQuery.m_To = vTemp;

	if (g_pLTClient->IntersectSegment(&iQuery, &iInfo))
	{
		vTemp = iInfo.m_Point - vPos;
        LTFLOAT fDist = (fClipDistance - vTemp.Mag());

		vTemp = vU * -fDist;
		vPos += vTemp;
	}
	else
	{
 		// If we didn't hit any ceilings, check for floors...

		vTemp = vU * -fClipDistance;
		vTemp += vPos;

		iQuery.m_From = vPos;
		iQuery.m_To = vTemp;

		if (g_pLTClient->IntersectSegment(&iQuery, &iInfo))
		{
			vTemp = iInfo.m_Point - vPos;
            LTFLOAT fDist = (fClipDistance - vTemp.Mag());

			vTemp = vU * fDist;
			vPos += vTemp;
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerCamera::CPlayerCamera()
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

CPlayerCamera::CPlayerCamera()
{
    m_hTarget           = LTNULL;
    m_pClientDE         = LTNULL;

    m_rRotation.Init();
    m_rLastTargetRot.Init();
	m_vPos.Init();
	m_vLastTargetPos.Init();
	m_vLastOptPos.Init();

	m_eOrientation		= SOUTH;
	m_eSaveOrientation	= SOUTH;

	m_OptX				= 0.0f;
	m_OptY				= 0.0f;
	m_OptZ				= 0.0f;
	m_CircleStartTime	= 0.0f;
	m_SaveAnglesY		= 0.0f;

    m_bSlide            = LTTRUE;

	m_CameraDistBack	= DEFAULT_CAMERA_DIST_BACK;
	m_CameraDistUp		= DEFAULT_CAMERA_DIST_UP;
	m_CameraDistDiag	= DEFAULT_CAMERA_DIST_DIAG;

	m_ePointType			= AT_POSITION;
    m_bStartCircle          = LTFALSE;
	m_CircleHeightOffset	= 0.0f;
	m_CircleRadius			= 75.0f;
	m_CircleTime			= 3.0f;
    m_bRestoreFirstPerson   = LTFALSE;

	m_GoingFirstPersonStart	= 0.0f;
	m_GoingFirstPersonTransition = 1.5f;

	m_eCameraMode			= CHASE;
	m_eSaveCameraMode		= CHASE;

	m_TargetFirstPersonOffset.Init();
	m_TargetChaseOffset.Init();
	m_TargetPointAtOffset.Init();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerCamera::Init()
//
//	PURPOSE:	Init the camera
//
// ----------------------------------------------------------------------- //

LTBOOL CPlayerCamera::Init(ILTClient* pClientDE)
{
    if (!pClientDE) return LTFALSE;
	m_pClientDE = pClientDE;

	g_vtCameraMoveUpTime.Init(pClientDE, "CameraMoveUpTime", NULL, 0.1f);
	g_vtCameraMoveUpMinVel.Init(pClientDE, "CameraMoveUpMinVel", NULL, 140.0f);
	g_vtCameraMaxYDifference.Init(pClientDE, "CameraMoveMaxYDiff", NULL, 30.0f);
	g_vtCameraClipDistance.Init(pClientDE, "CameraClipDist", NULL, 30.0f);

    return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerCamera::CameraUpdate()
//
//	PURPOSE:	Update the position & orientation of the camera based
//				on the target
//
// ----------------------------------------------------------------------- //

void CPlayerCamera::CameraUpdate(LTFLOAT deltaTime)
{
	if (!m_pClientDE || !m_hTarget) return;

    LTRotation rRot;
    LTVector vOpt, vPos;

	m_pClientDE->GetObjectPos(m_hTarget, &vPos);
	m_pClientDE->GetObjectRotation(m_hTarget, &rRot);

    LTVector vU, vR, vF;
	m_pClientDE->GetRotationVectors(&rRot, &vU, &vR, &vF);

	if (m_hTarget)
	{
		if (m_eOrientation == HOLD)
			return;

		switch(m_eCameraMode)
		{
			case CHASE:
			break;

			case GOINGFIRSTPERSON:
			{
				vF.y = 0.0f;
				vPos += (vF * m_TargetFirstPersonOffset.x);
				vPos.y += m_TargetFirstPersonOffset.y;

				vOpt = vPos;
			}
			break;

			case CIRCLING:
			{
				CircleAroundTarget();
				return;
			}
			break;

			case FIRSTPERSON:
			{
				UpdateFirstPerson(vF, vPos, deltaTime);
				PointInDirection(rRot);
				return;
			}
			break;

			case SCRIPT:
				return;
			break;

			default:
				return;
		}

		// Find the optimal position for the camera
		if (m_eCameraMode == GOINGFIRSTPERSON)
		{
			MoveCameraToPosition(vOpt, deltaTime);
			PointAtPosition(vOpt, rRot, m_vPos);
			return;
		}
		else
		{
			vOpt = FindOptimalCameraPosition();
		}

		// Move the camera to the optimal position
		// (it will slide or not depending on the m_bSlide param)
		MoveCameraToPosition(vOpt, deltaTime);

		// Either point the camera at the player or MLOOK
		if(m_eOrientation == MLOOK)
		{
            m_rRotation = rRot;
		}
		else
		{
			switch(m_ePointType)
			{
				case IN_DIRECTION:
					PointInDirection(rRot);
				break;

				case AT_POSITION:
				default:
				{
                    LTVector vTemp;
					vTemp = vPos + m_TargetPointAtOffset;
					PointAtPosition(vTemp, rRot, vOpt);
				}
				break;
			}
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerCamera::UpdateFirstPerson()
//
//	PURPOSE:	Update the position of the camera when in first person
//
// ----------------------------------------------------------------------- //

void CPlayerCamera::UpdateFirstPerson(LTVector vF, LTVector vPos, LTFLOAT fDeltaTime)
{
	vF.y = 0.0f;
	vPos += (vF * m_TargetFirstPersonOffset.x);
	vPos.y += m_TargetFirstPersonOffset.y;

	// Make sure we don't move the camera up too fast (going up stairs)...

	if (vPos.y > (m_vPos.y + 1.0f))
	{
        LTFLOAT fNewY = m_vPos.y;
		CMoveMgr* pMoveMgr = g_pGameClientShell->GetMoveMgr();
		if (!pMoveMgr) return;

        LTBOOL bFreeMovement =
			(IsFreeMovement(g_pGameClientShell->GetCurContainerCode()) ||
			pMoveMgr->IsBodyOnLadder() ||
			pMoveMgr->GetVehicleMgr()->IsVehiclePhysics() ||
			pMoveMgr->Jumped() || g_pGameClientShell->IsSpectatorMode() ||
			pMoveMgr->IsOnLift());

		if (!bFreeMovement)
		{
            LTFLOAT fMaxYDiff = g_vtCameraMaxYDifference.GetFloat();
            LTFLOAT fYDiff = (vPos.y - fNewY);

			// Force the camera to never be more than fMaxYDiff away from
			// its "real" position...

			if (fYDiff > fMaxYDiff)
			{
				fNewY  += (fYDiff - fMaxYDiff);
				fYDiff = fMaxYDiff;
			}

            LTFLOAT fVel = fYDiff / g_vtCameraMoveUpTime.GetFloat();
			fVel = fVel < g_vtCameraMoveUpMinVel.GetFloat() ? g_vtCameraMoveUpMinVel.GetFloat() : fVel;

			fNewY += (fDeltaTime * fVel);

			if (fabs(fNewY - vPos.y) > 1.0f)
			{
				vPos.y = fNewY > vPos.y ? vPos.y : fNewY;
			}
		}
	}

	m_vPos = vPos;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerCamera::Apply()
//
//	PURPOSE:	Matrix apply function
//
// ----------------------------------------------------------------------- //

LTVector CPlayerCamera::Apply(LTVector right, LTVector up, LTVector forward, LTVector copy)
{
    LTVector target;

	target.x = copy.x*right.x + copy.y*right.y + copy.z*right.z;
	target.y = copy.x*up.x + copy.y*up.y + copy.z*up.z;
	target.z = copy.x*forward.x + copy.y*forward.y + copy.z*forward.z;

	return target;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerCamera::PointAtPosition()
//
//	PURPOSE:	Point the camera at a position from a position
//
// ----------------------------------------------------------------------- //

void CPlayerCamera::PointAtPosition(LTVector pos, LTRotation rRot, LTVector pointFrom)
{
	if (!m_pClientDE) return;

    LTVector vAngles;
	vAngles.Init();

	m_rRotation = rRot;

    LTFLOAT diffX = pos.x - m_vPos.x;
    LTFLOAT diffY = pos.z - m_vPos.z;
    vAngles.y = (LTFLOAT)atan2(diffX, diffY);

    LTVector     target, copy;
    LTVector up, right, forward;

	m_pClientDE->GetRotationVectors(&m_rRotation, &up, &right, &forward);

	VEC_SUB(copy, pos, pointFrom);

	target = Apply(right, up, forward, copy);

	diffX = target.z;
	diffY = target.y;


    vAngles.x = (LTFLOAT)-atan2(diffY, diffX);

    LTRotation rOldRot;
    rOldRot = m_rRotation;

	m_pClientDE->SetupEuler(&m_rRotation, vAngles.x, vAngles.y, vAngles.z);

	// Make sure rotation is valid...

	m_pClientDE->GetRotationVectors(&m_rRotation, &up, &right, &forward);
    if (up.y < 0) m_rRotation = rOldRot;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerCamera::SetCameraState()
//
//	PURPOSE:	Set the state/orientation of the camera
//
// ----------------------------------------------------------------------- //

void CPlayerCamera::SetCameraState(CameraState eOrientation)
{
	switch(eOrientation)
	{
		case SOUTHEAST:
			m_OptX = m_CameraDistDiag;
			m_OptY = m_CameraDistUp;
			m_OptZ = -m_CameraDistDiag;
		break;

		case EAST:
			m_OptX = m_CameraDistBack;
			m_OptY = m_CameraDistUp;
			m_OptZ = 0;
		break;

		case NORTHEAST:
			m_OptX = m_CameraDistDiag;
			m_OptY = m_CameraDistUp;
			m_OptZ = m_CameraDistDiag;
		break;

		case NORTH:
			m_OptX = 0;
			m_OptY = m_CameraDistUp;
			m_OptZ = m_CameraDistBack;
		break;

		case NORTHWEST:
			m_OptX = -m_CameraDistDiag;
			m_OptY = m_CameraDistUp;
			m_OptZ = m_CameraDistDiag;
		break;

		case WEST:
			m_OptX = -m_CameraDistBack;
			m_OptY = m_CameraDistUp;
			m_OptZ = 0;
		break;

		case SOUTHWEST:
			m_OptX = -m_CameraDistDiag;
			m_OptY = m_CameraDistUp;
			m_OptZ = -m_CameraDistDiag;
		break;

		case SOUTH:
			m_OptX = 0;
			m_OptY = m_CameraDistUp;
			m_OptZ = -m_CameraDistBack;
		break;

		case MLOOK:
			m_OptX = DEFAULT_CAMERA_DIST_MLOOK_X;
			m_OptY = DEFAULT_CAMERA_DIST_MLOOK_Y;
			m_OptZ = DEFAULT_CAMERA_DIST_MLOOK_Z;
		break;

		default:
		break;
	}

	m_eOrientation = eOrientation;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerCamera::RotateCameraState()
//
//	PURPOSE:	Rotate the camera clockwise or counterclockwise around
//				the target
//
// ----------------------------------------------------------------------- //

void CPlayerCamera::RotateCameraState(LTBOOL bClockwise)
{
	switch(m_eOrientation)
	{
		case SOUTHEAST:
			if(bClockwise)
				SetCameraState(SOUTH);
			else
				SetCameraState(EAST);
		break;

		case EAST:
			if(bClockwise)
				SetCameraState(SOUTHEAST);
			else
				SetCameraState(NORTHEAST);
		break;

		case NORTHEAST:
			if(bClockwise)
				SetCameraState(EAST);
			else
				SetCameraState(NORTH);
		break;

		case NORTH:
			if(bClockwise)
				SetCameraState(NORTHEAST);
			else
				SetCameraState(NORTHWEST);
		break;

		case NORTHWEST:
			if(bClockwise)
				SetCameraState(NORTH);
			else
				SetCameraState(WEST);
		break;

		case WEST:
			if(bClockwise)
				SetCameraState(NORTHWEST);
			else
				SetCameraState(SOUTHWEST);
		break;

		case SOUTHWEST:
			if(bClockwise)
				SetCameraState(WEST);
			else
				SetCameraState(SOUTH);
		break;

		case SOUTH:
			if(bClockwise)
				SetCameraState(SOUTHWEST);
			else
				SetCameraState(SOUTHEAST);
		break;

		default:
		break;

	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerCamera::MoveCameraToPosition()
//
//	PURPOSE:	Move the camera to a position over a time period
//
// ----------------------------------------------------------------------- //

void CPlayerCamera::MoveCameraToPosition(LTVector pos, LTFLOAT deltaTime)
{
	if (!m_pClientDE || !m_hTarget) return;

	if (m_eCameraMode == GOINGFIRSTPERSON)
	{
		m_eCameraMode = FIRSTPERSON;

        LTRotation rRot;
		m_pClientDE->GetObjectRotation(m_hTarget, &rRot);

        LTVector vU, vR, vF;
		m_pClientDE->GetRotationVectors(&rRot, &vU, &vR, &vF);

        LTVector vPos;
		m_pClientDE->GetObjectPos(m_hTarget, &vPos);

		vF.y = 0.0f;
		vPos += (vF * m_TargetFirstPersonOffset.x);
		vPos.y += m_TargetFirstPersonOffset.y;

		m_vPos = vPos;

		PointInDirection(rRot);
		return;
	}

    LTVector dir;
	dir = (pos - m_vPos);

    LTFLOAT multiplier = 1.0f; // 0.5f;

    LTVector toMove;
	toMove = (dir * multiplier);

    LTFLOAT moveMag;

	if (m_bSlide)
	{
		moveMag = toMove.Mag();

		if (moveMag > dir.Mag())
		{
			moveMag = dir.Mag();
		}

		if (toMove.x != 0.0f || toMove.y != 0.0f || toMove.z != 0.0f)
		{
			toMove.Norm();
		}

		m_vPos += (toMove * moveMag);
	}
	else
	{
		m_vPos = pos;
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerCamera::FindOptimalCameraPosition()
//
//	PURPOSE:	Find the optimal camera position
//
// ----------------------------------------------------------------------- //

LTVector CPlayerCamera::FindOptimalCameraPosition()
{
    LTVector pos;
	VEC_INIT(pos);

	if (!m_pClientDE || !m_hTarget) return pos;

    LTVector     up, right, forward, dir;
    LTFLOAT      distToOptimal;
    LTVector     TargetPlusOffset;

    LTVector vTargetPos;
	m_pClientDE->GetObjectPos(m_hTarget, &vTargetPos);

    LTRotation rRot;
	m_pClientDE->GetObjectRotation(m_hTarget, &rRot);

	if (Equal(vTargetPos, m_vLastTargetPos) && Equal(rRot, m_rLastTargetRot))
	{
		return m_vLastOptPos;
	}
	else
	{
		VEC_COPY(m_vLastTargetPos, vTargetPos);
        m_rLastTargetRot = rRot;
	}

    LTVector vTemp;
	VEC_ADD(vTemp, vTargetPos, m_TargetChaseOffset);
	VEC_COPY(TargetPlusOffset, vTemp);

	m_pClientDE->GetRotationVectors(&rRot, &up, &right, &forward);

	//	pos = TargetPlusOffset + right*m_OptX + up*m_OptY + forward*m_OptZ;

    LTVector vTemp1, vTemp2;
	VEC_MULSCALAR(vTemp, right, m_OptX);
	VEC_MULSCALAR(vTemp1, up, m_OptY)
	VEC_MULSCALAR(vTemp2, forward, m_OptZ);

	ClientIntersectQuery iQuery;
	ClientIntersectInfo  iInfo;

	VEC_ADD(vTemp, vTemp, vTemp1);
	VEC_ADD(vTemp, vTemp, vTemp2);
	VEC_ADD(pos, TargetPlusOffset, vTemp);

	VEC_SUB(vTemp, TargetPlusOffset, pos);
	distToOptimal = VEC_MAG(vTemp);

	VEC_SUB(dir, pos, TargetPlusOffset);
	VEC_NORM(dir);

	VEC_COPY(iQuery.m_From, TargetPlusOffset);
	VEC_COPY(iQuery.m_To, pos);

	if (m_pClientDE->IntersectSegment(&iQuery, &iInfo))
	{
		VEC_SUB(vTemp, iInfo.m_Point, TargetPlusOffset);

		// If there was something in the way, move in front of that thing.
		if (VEC_MAG(vTemp) < distToOptimal)
		{
			VEC_ADD(pos, iInfo.m_Point, iInfo.m_Plane.m_Normal);
		}
	}


	CalcNonClipPos(pos, rRot);

	m_vLastOptPos = pos;

	return pos;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerCamera::PrintVector()
//
//	PURPOSE:	Print it!
//
// ----------------------------------------------------------------------- //

void CPlayerCamera::PrintVector(LTVector v)
{
	if (!m_pClientDE) return;

	char buf[50];
	sprintf(buf, "x = %f, y = %f, z = %f", v.x, v.y, v.z);
	m_pClientDE->CPrint(buf);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerCamera::StartCircle()
//
//	PURPOSE:	Start the circle...jerk!
//
// ----------------------------------------------------------------------- //

void CPlayerCamera::StartCircle(LTFLOAT HeightOffset, LTFLOAT Radius, LTFLOAT PointAtOffset, LTFLOAT Time)
{
	if (!m_pClientDE || !m_hTarget) return;

	if(m_eCameraMode != CIRCLING)
	{
		m_CircleHeightOffset = HeightOffset;
		m_CircleRadius		 = Radius;
		m_CircleHeightOffset = PointAtOffset;
		m_CircleTime		 = Time;
        m_bStartCircle       = LTFALSE;

        LTVector vTargetPos, up, right, forward;
        LTRotation rRot;

		m_pClientDE->GetObjectRotation(m_hTarget, &rRot);
		m_pClientDE->GetRotationVectors(&rRot, &up, &right, &forward);
		m_pClientDE->GetObjectPos(m_hTarget, &vTargetPos);

		SaveState();

		SaveCameraMode();
		//GetClientShell().CPrint("Going circle");
		m_eCameraMode = CIRCLING;

		m_CircleStartTime = m_pClientDE->GetTime();


		//	m_Pos.Copy(m_Target.m_Pos + right*0 + up*m_CircleHeightOffset - forward*m_CircleRadius);
        LTVector vTemp, vTemp1;
		VEC_MULSCALAR(vTemp, forward, m_CircleRadius);
		VEC_MULSCALAR(vTemp1, up, m_CircleHeightOffset);
		VEC_SUB(vTemp, vTemp1, vTemp);
		VEC_ADD(m_vPos, vTargetPos, vTemp);

		// PointAtPosition(m_Target.m_Pos+CreateVector(0,m_CircleHeightOffset,0), m_Target.m_Angles, m_Pos);
		VEC_SET(vTemp, 0.0f, m_CircleHeightOffset, 0.0f);
		VEC_ADD(vTemp, vTargetPos, vTemp);

		PointAtPosition(vTemp, rRot, m_vPos);

		// m_SaveAnglesY = m_Angles.y;
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerCamera::CircleAroundTarget()
//
//	PURPOSE:	Circle the target
//
// ----------------------------------------------------------------------- //

void CPlayerCamera::CircleAroundTarget()
{
	if (!m_pClientDE || !m_hTarget) return;

    LTFLOAT diff = (m_pClientDE->GetTime() - m_CircleStartTime) / m_CircleTime;

	if(diff >= 1.0f)
	{
		RestoreState();
		RestoreCameraMode();
		//GetClientShell().CPrint("Done with circle");
		return;
	}

	//m_Angles.y = m_SaveAnglesY + (MATH_CIRCLE*diff);
	m_pClientDE->EulerRotateY(&m_rRotation, (MATH_CIRCLE*diff));

    LTVector up, right, forward;
	m_pClientDE->GetRotationVectors(&m_rRotation, &up, &right, &forward);

    LTVector vTargetPos;
    LTRotation rRot;
	m_pClientDE->GetObjectPos(m_hTarget, &vTargetPos);
	m_pClientDE->GetObjectRotation(m_hTarget, &rRot);

	// m_Pos.Copy(m_Target.m_Pos + right*0 + up*m_CircleHeightOffset - forward*m_CircleRadius);
    LTVector vTemp, vTemp1;
	VEC_MULSCALAR(vTemp, forward, m_CircleRadius);
	VEC_MULSCALAR(vTemp1, up, m_CircleHeightOffset);
	VEC_SUB(vTemp, vTemp1, vTemp);
	VEC_ADD(m_vPos, vTargetPos, vTemp);

	// PointAtPosition(m_Target.m_Pos+CreateVector(0,m_CircleHeightOffset,0), m_Target.m_Angles, m_Pos);
	VEC_SET(vTemp, 0.0f, m_CircleHeightOffset, 0.0f);
	VEC_ADD(vTemp, vTargetPos, vTemp);

	PointAtPosition(vTemp, rRot, m_vPos);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerCamera::VCompare()
//
//	PURPOSE:	Compare two vectors
//
// ----------------------------------------------------------------------- //

LTBOOL CPlayerCamera::VCompare(LTVector a, LTVector b)
{
	if((fabs(a.x - b.x) > 5.0f) || (fabs(a.y - b.y) > 5.0f) || (fabs(a.z - b.z) > 5.0f))
        return LTFALSE;
	else
        return LTTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerCamera::AttachToObject()
//
//	PURPOSE:	Attach camera to an object
//
// ----------------------------------------------------------------------- //

void CPlayerCamera::AttachToObject(HLOCALOBJ hObj)
{
	m_hTarget = hObj;

	// Initialize our position to that of the object...

	if (m_hTarget && m_pClientDE)
	{
		m_pClientDE->GetObjectPos(m_hTarget, &m_vPos);

		// Assume for now first person...

        LTRotation rRot;
		m_pClientDE->GetObjectRotation(m_hTarget, &rRot);

        LTVector vU, vR, vF;
		m_pClientDE->GetRotationVectors(&rRot, &vU, &vR, &vF);
		vF.y = 0.0f;

		m_vPos += (vF * m_TargetFirstPersonOffset.x);
		m_vPos.y += m_TargetFirstPersonOffset.y;
	}
}

void CPlayerCamera::GoFirstPerson()
{
	if((m_eCameraMode != GOINGFIRSTPERSON) && (m_eCameraMode != FIRSTPERSON))
	{
        m_GoingFirstPersonStart = g_pLTClient->GetTime();
		m_eCameraMode = GOINGFIRSTPERSON;
        CameraUpdate(g_pGameClientShell->GetFrameTime());
	}
}

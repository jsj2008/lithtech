// ----------------------------------------------------------------------- //
//
// MODULE  : PlayerCamera.cpp
//
// PURPOSE : PlayerCamera - Implementation
//
// CREATED : 10/5/97
//
// ----------------------------------------------------------------------- //

#include "PlayerCamera.h"
#include "clientheaders.h"
#include <stdio.h>
#include "iltcommon.h"

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


LTBOOL Equal(LTVector & v1, LTVector & v2)
{
	LTVector v;
	VEC_SUB(v, v1, v2);
	return LTBOOL(VEC_MAG(v) < 1.0f);
}

LTBOOL Equal(LTRotation & r1, LTRotation & r2)
{
	LTBOOL bRet = LTTRUE;

	if (r1.m_Quat[0] != r2.m_Quat[0] || r1.m_Quat[1] != r2.m_Quat[1] ||
		r1.m_Quat[2] != r2.m_Quat[2] || r1.m_Quat[3] != r2.m_Quat[3])
	{
		bRet = LTFALSE;
	}

	return bRet;
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
	m_hTarget			= LTNULL;
	m_pClientDE			= LTNULL;

	m_rRotation.Init();
	VEC_INIT(m_vPos);
	VEC_INIT(m_vLastTargetPos);
	VEC_INIT(m_vLastOptPos);
	m_rLastTargetRot.Init();

	m_eOrientation		= SOUTH;
	m_eSaveOrientation	= SOUTH;
	
	m_OptX				= 0.0f;
	m_OptY				= 0.0f;
	m_OptZ				= 0.0f;
	m_CircleStartTime	= 0.0f;
	m_SaveAnglesY		= 0.0f;

	m_bSlide			= LTTRUE;
	
	m_CameraDistBack	= DEFAULT_CAMERA_DIST_BACK;
	m_CameraDistUp		= DEFAULT_CAMERA_DIST_UP;
	m_CameraDistDiag	= DEFAULT_CAMERA_DIST_DIAG;

	m_ePointType			= AT_POSITION;
	m_bStartCircle			= LTFALSE;
	m_CircleHeightOffset	= 0.0f;
	m_CircleRadius			= 75.0f;
	m_CircleTime			= 3.0f;
	m_bRestoreFirstPerson	= LTFALSE;

	m_GoingFirstPersonStart	= 0.0f;
	m_GoingFirstPersonTransition = 1.5f;

	m_eCameraMode			= CHASE;
	m_eSaveCameraMode		= CHASE;

	VEC_INIT(m_TargetFirstPersonOffset);
	VEC_INIT(m_TargetChaseOffset);
	VEC_INIT(m_TargetPointAtOffset);
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
	m_pClientDE->Common()->GetRotationVectors(rRot, vU, vR, vF);

	if(m_hTarget)
	{
		if(m_eOrientation == HOLD)
			return;

		switch(m_eCameraMode)
		{
			case CHASE:
			break;
			
			case GOINGFIRSTPERSON:
			{
				LTVector vTemp;
				VEC_MULSCALAR(vTemp, vF, m_TargetFirstPersonOffset.x);
				VEC_ADD(vPos, vPos, vTemp);
				vPos.y += m_TargetFirstPersonOffset.y;

				VEC_COPY(vOpt, vPos);
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
				LTVector vTemp;
				VEC_MULSCALAR(vTemp, vF, m_TargetFirstPersonOffset.x);
				VEC_ADD(vPos, vPos, vTemp);
				vPos.y += m_TargetFirstPersonOffset.y;

				VEC_COPY(m_vPos, vPos);

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
		if(m_eCameraMode == GOINGFIRSTPERSON)
		{
			MoveCameraToPosition(vOpt, deltaTime);
			//PointInDirection(m_Target.m_Angles);

			PointAtPosition(vOpt, rRot, m_vPos);
			return;
		}
		else
		{
			VEC_COPY(vOpt, FindOptimalCameraPosition());
		}

		// Move the camera to the optimal position
		// (it will slide or not depending on the m_bSlide param)
		MoveCameraToPosition(vOpt, deltaTime);

		// Either point the camera at the player or MLOOK
		if(m_eOrientation == MLOOK)
		{
			//if(((m_Pos.x - opt.x) < 1) && ((m_Pos.y - opt.y) < 1) && ((m_Pos.z - opt.z) < 1))
			//{
				m_rRotation = rRot;
			//}
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
					VEC_ADD(vTemp, vPos, m_TargetPointAtOffset);
					PointAtPosition(vTemp, rRot, vOpt);
				}	
				break;
			}
		}
	}
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

	//m_Angles.Copy(m_Target.m_Angles);
	
	//m_Angles.z = angles.z;
	//m_Angles.x = angles.x;
	LTVector vAngles;
	VEC_INIT(vAngles);

	m_rRotation = rRot;


	LTFLOAT diffX = pos.x - m_vPos.x;
	LTFLOAT diffY = pos.z - m_vPos.z;
	vAngles.y = (LTFLOAT)atan2(diffX, diffY);

	LTVector		target, copy;
	LTVector	up, right, forward;

	m_pClientDE->Common()->GetRotationVectors(m_rRotation, up, right, forward);

	VEC_SUB(copy, pos, pointFrom);

	target = Apply(right, up, forward, copy);

	diffX = target.z;
	diffY = target.y;

	//LTFLOAT temp = -ATan2(diffY,diffX);
	//if(Abs(temp - m_Angles.x) < .5)

	vAngles.x = (LTFLOAT)-atan2(diffY, diffX);

	//PrintVector(pos);
	//GetClientShell().CPrint("X = " + RealStr(m_Angles.x));

	LTRotation rOldRot;
	rOldRot = m_rRotation;

	m_pClientDE->Common()->SetupEuler(m_rRotation, vAngles.x, vAngles.y, vAngles.z);

	// Make sure rotation is valid...

	m_pClientDE->Common()->GetRotationVectors(m_rRotation, up, right, forward);
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

	LTFLOAT nCurrentTime = m_pClientDE->GetTime();
	if (m_eCameraMode == GOINGFIRSTPERSON)
	{
#ifdef OLD_WAY
		if (nCurrentTime > m_GoingFirstPersonStart + m_GoingFirstPersonTransition)
		{
			m_eCameraMode = FIRSTPERSON;
			return;
		}
		
		LTFLOAT percentage = (nCurrentTime - m_GoingFirstPersonStart) / (m_GoingFirstPersonTransition);

		LTVector vMagnitude;
		VEC_SUB (vMagnitude, pos, m_vPos);
		VEC_MULSCALAR (vMagnitude, vMagnitude, percentage);
		VEC_ADD (m_vPos, m_vPos, vMagnitude);
#else

		m_eCameraMode = FIRSTPERSON;

		LTRotation rRot;
		m_pClientDE->GetObjectRotation(m_hTarget, &rRot);

		LTVector vU, vR, vF;
		m_pClientDE->Common()->GetRotationVectors(rRot, vU, vR, vF);

		LTVector vPos;
		m_pClientDE->GetObjectPos(m_hTarget, &vPos);

		LTVector vTemp;
		VEC_MULSCALAR(vTemp, vF, m_TargetFirstPersonOffset.x);
		VEC_ADD(vPos, vPos, vTemp);
		vPos.y += m_TargetFirstPersonOffset.y;

		VEC_COPY(m_vPos, vPos);

		PointInDirection(rRot);
		return;

#endif
		return;
	}
/*
	if(VCompare(m_vPos, pos))
	{
		if(m_eCameraMode == GOINGFIRSTPERSON)
		{
			//GetClientShell().CPrint("We're first person");
			m_eCameraMode = FIRSTPERSON;
			return;
		}
	}
*/

	LTVector	dir;
	VEC_SUB(dir, pos, m_vPos);

	LTFLOAT multiplier = 1.0f; // 0.5f;
	
	LTVector	toMove;
	VEC_MULSCALAR(toMove, dir, multiplier);
	
	LTFLOAT moveMag;

	if(m_bSlide)
	{
		moveMag = VEC_MAG(toMove);
		if(moveMag > VEC_MAG(dir))
			moveMag = VEC_MAG(dir);

		if (toMove.x != 0.0f || toMove.y != 0.0f || toMove.z != 0.0f)
		{
			VEC_NORM(toMove);
		}
		VEC_MULSCALAR(toMove, toMove, moveMag);

		VEC_ADD(m_vPos, m_vPos, toMove);
	}
	else
	{
		VEC_COPY(m_vPos, pos);
	}

//#define CAST_RAY_THANG
#ifdef CAST_RAY_THANG
	////////////////////////////////////////////////////////////////////////////////
	// THIS NEXT SECTION IS TEMPORARY UNTIL THE CASTRAY BUG IS FIXED...
	// It is only in the engine because there is a bug in the CastRay function that
	// sometimes returns the reverse normal of the intersected polygon...
	// In order to keep the camera out of the walls, this check is made...
	////////////////////////////////////////////////////////////////////////////////

	LTVector vTargetPos;
	m_pClientDE->GetObjectPos(m_hTarget, &vTargetPos);
	
	VEC_SUB(dir, pos, vTargetPos);
	VEC_NORM(dir);

	ClientIntersectQuery iQuery;
	ClientIntersectInfo  iInfo;

	VEC_COPY(iQuery.m_From, vTargetPos);
	VEC_COPY(iQuery.m_Direction, dir);

	if (m_pClientDE->CastRay(&iQuery, &iInfo))
	{		
		// If there was something in the way, move in front of that thing.
		LTVector vTemp1, vTemp2;
		VEC_SUB(vTemp1, iInfo.m_Point, vTargetPos);
		VEC_SUB(vTemp2, pos, vTargetPos);

		if(VEC_MAG(vTemp1) < VEC_MAG(vTemp2))
		{
			LTVector vTemp;
			VEC_SUB(vTemp, iInfo.m_Point, iInfo.m_Plane.m_Normal);

			VEC_COPY(m_vPos, vTemp);

			//PrintVector(iInfo.m_Plane.m_Normal);
		}
	}
	////////////////////////////////////////////////////////////////////////////////
	// END OF TEMPORARY SECTION
	////////////////////////////////////////////////////////////////////////////////
#endif
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerCamera::FindOptimalCameraPosition()
//
//	PURPOSE:	Find the optimal camera position
//
// ----------------------------------------------------------------------- //

LTVector	CPlayerCamera::FindOptimalCameraPosition()
{
	LTVector pos;
	VEC_INIT(pos);

	if (!m_pClientDE || !m_hTarget) return pos;

	LTVector		up, right, forward, dir;
	LTFLOAT		distToOptimal;
	LTVector		TargetPlusOffset;
	
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

	m_pClientDE->Common()->GetRotationVectors(rRot, up, right, forward);

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


#define DOING_EXTRA_CHECKS
#ifdef DOING_EXTRA_CHECKS

	// Make sure we aren't clipping into walls...
	
	LTFLOAT fClipDistance	= 15.0f;
	LTBOOL bClipRightIssues	= LTTRUE;
	LTBOOL bClipUpIssues		= LTTRUE;


	// Check for walls to the right...

	VEC_MULSCALAR(vTemp, right, fClipDistance);
	VEC_ADD(vTemp, pos, vTemp);

	VEC_COPY(iQuery.m_From, pos);
	VEC_COPY(iQuery.m_To, vTemp);

	if (m_pClientDE->IntersectSegment(&iQuery, &iInfo))
	{
		VEC_SUB(vTemp, iInfo.m_Point, pos);
		LTFLOAT fDist = (fClipDistance - VEC_MAG(vTemp));

		VEC_MULSCALAR(vTemp, right, -fDist)
		VEC_ADD(pos, pos, vTemp);
	}
	else
	{
		bClipRightIssues = LTFALSE;
	}


	// If we didn't adjust for a wall to the right, check walls to the left...

	if (!bClipRightIssues)
	{
		VEC_MULSCALAR(vTemp, right, -fClipDistance);
		VEC_ADD(vTemp, pos, vTemp);

		VEC_COPY(iQuery.m_From, pos);
		VEC_COPY(iQuery.m_To, vTemp);

		if (m_pClientDE->IntersectSegment(&iQuery, &iInfo))
		{
			VEC_SUB(vTemp, iInfo.m_Point, pos);
			LTFLOAT fDist = (fClipDistance - VEC_MAG(vTemp));

			VEC_MULSCALAR(vTemp, right, fDist)
			VEC_ADD(pos, pos, vTemp);
		}
	}
			

	// Check for ceilings...

	VEC_MULSCALAR(vTemp, up, fClipDistance);
	VEC_ADD(vTemp, pos, vTemp);

	VEC_COPY(iQuery.m_From, pos);
	VEC_COPY(iQuery.m_To, vTemp);

	if (m_pClientDE->IntersectSegment(&iQuery, &iInfo))
	{
		VEC_SUB(vTemp, iInfo.m_Point, pos);
		LTFLOAT fDist = (fClipDistance - VEC_MAG(vTemp));

		VEC_MULSCALAR(vTemp, up, -fDist)
		VEC_ADD(pos, pos, vTemp);
	}
	else
	{
		bClipUpIssues = LTFALSE;
	}


	// If we didn't hit any ceilings, check for floors...

	if (!bClipUpIssues)
	{
		VEC_MULSCALAR(vTemp, up, -fClipDistance);
		VEC_ADD(vTemp, pos, vTemp);

		VEC_COPY(iQuery.m_From, pos);
		VEC_COPY(iQuery.m_To, vTemp);

		if (m_pClientDE->IntersectSegment(&iQuery, &iInfo))
		{
			VEC_SUB(vTemp, iInfo.m_Point, pos);
			LTFLOAT fDist = (fClipDistance - VEC_MAG(vTemp));

			VEC_MULSCALAR(vTemp, up, fDist)
			VEC_ADD(pos, pos, vTemp);
		}
	}
#endif  // DOING_EXTRA_CHECKS 

	VEC_COPY(m_vLastOptPos, pos);
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
		m_bStartCircle		 = LTFALSE;

		LTVector	vTargetPos, up, right, forward;
		LTRotation rRot;

		m_pClientDE->GetObjectRotation(m_hTarget, &rRot);
		m_pClientDE->Common()->GetRotationVectors(rRot, up, right, forward);
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
	m_pClientDE->Math()->EulerRotateY(m_rRotation, (MATH_CIRCLE*diff));

	LTVector	up, right, forward;
	m_pClientDE->Common()->GetRotationVectors(m_rRotation, up, right, forward);

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

	if (hObj && m_pClientDE)
	{
		m_pClientDE->GetObjectPos(hObj, &m_vPos);
	}
}

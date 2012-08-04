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
#include "cpp_client_de.h"
#include "SharedDefs.h"
#include <stdio.h>
#include <math.h>

// Camera's distance back from the player's position
#define DEFAULT_CAMERA_DIST_BACK	90
// Camera's distance up from the player's position
#define DEFAULT_CAMERA_DIST_UP		35
// Camera's distance back from the player's position but at a 45 degree angle
// (for example, if the camera is looking at the player from the southeast position)
#define DEFAULT_CAMERA_DIST_DIAG	90

// Camera's X offset from the player when it is in the MLOOK state
#define DEFAULT_CAMERA_DIST_MLOOK_X	0
// Camera's Y offset from the player when it is in the MLOOK state
#define DEFAULT_CAMERA_DIST_MLOOK_Y 10
// Camera's Z offset from the player when it is in the MLOOK state
#define DEFAULT_CAMERA_DIST_MLOOK_Z 29

#define DEFAULT_ROTATE_SPEED		0.8f	// angular velocity


DBOOL Equal(DVector & v1, DVector & v2)
{
	DVector v;
	VEC_SUB(v, v1, v2);
	return DBOOL(VEC_MAG(v) < 1.0f);
}

DBOOL Equal(DRotation & r1, DRotation & r2)
{
	DBOOL bRet = DTRUE;

	if (r1.m_Vec.x != r2.m_Vec.x || r1.m_Vec.y != r2.m_Vec.y ||
		r1.m_Vec.z != r2.m_Vec.z || r1.m_Spin != r2.m_Spin)
	{
		bRet = DFALSE;
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
	m_hTarget			= DNULL;
	m_pClientDE			= DNULL;

	ROT_INIT(m_rRotation);
	VEC_INIT(m_vPos);
	VEC_INIT(m_vLastTargetPos);
	VEC_INIT(m_vLastOptPos);
	ROT_INIT(m_rLastTargetRot);

//	m_eOrientation		= SOUTH;
//	m_eSaveOrientation	= SOUTH;
	
	m_CameraDistBack	= DEFAULT_CAMERA_DIST_BACK;
	m_CameraDistUp		= DEFAULT_CAMERA_DIST_UP;
	m_CameraDistDiag	= DEFAULT_CAMERA_DIST_DIAG;

	m_OptX				= 0.0f;
//	m_OptY				= 0.0f;
//	m_OptZ				= 0.0f;
	m_OptY				= m_CameraDistUp;
	m_OptZ				= -m_CameraDistBack;
	m_CircleStartTime	= 0.0f;
	m_SaveAnglesY		= 0.0f;

	m_bSlide			= DTRUE;
	
	m_bStartCircle			= DFALSE;
	m_CircleHeightOffset	= 0.0f;
	m_CircleRadius			= 75.0f;
	m_CircleTime			= 3.0f;
	m_bRestoreFirstPerson	= DFALSE;

	m_fTransitionStart	= 0.0f;
	m_fTransitionTime = 1.0f;

	m_eCameraMode			= FIRSTPERSON;
	m_eSaveCameraMode		= FIRSTPERSON;

	VEC_INIT(m_TargetFirstPersonOffset);
	VEC_INIT(m_TargetChaseOffset);
	VEC_INIT(m_TargetPointAtOffset);
	VEC_INIT(m_TargetDeathOffset);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerCamera::CameraUpdate()
//
//	PURPOSE:	Update the position & orientation of the camera based 
//				on the target
//
// ----------------------------------------------------------------------- //

void CPlayerCamera::CameraUpdate(DFLOAT deltaTime)
{
	if (!m_pClientDE || !m_hTarget) return;


	DRotation rRot;
	DVector vOpt, vPos;

	m_pClientDE->GetObjectPos(m_hTarget, &vPos);
	m_pClientDE->GetObjectRotation(m_hTarget, &rRot);
	
	DVector vU, vR, vF;
	m_pClientDE->GetRotationVectors(&rRot, &vU, &vR, &vF);

	if(m_hTarget)
	{
//		if(m_eOrientation == HOLD)
//			return;

		switch(m_eCameraMode)
		{
			case CHASE:
			break;
			
			case GOINGFIRSTPERSON:
			{
//				DVector vTemp;
//				VEC_MULSCALAR(vTemp, vF, m_TargetFirstPersonOffset.x);
//				VEC_ADD(vPos, vPos, vTemp);
//				vPos.y += m_TargetFirstPersonOffset.y;

				VEC_COPY(vOpt, FindFirstPersonCameraPosition(vPos, vF));
	
				MoveCameraToPosition(vOpt, FindOptimalCameraPosition(), deltaTime);
				PointAtPosition(vOpt, rRot, m_vPos);
				//PointInDirection(m_Target.m_Angles);
				return;
			}	
			break;

			case GOINGCHASE:
			{
				VEC_COPY(vOpt, FindFirstPersonCameraPosition(vPos, vF));
	
				MoveCameraToPosition(FindOptimalCameraPosition(), vOpt, deltaTime);
				PointAtPosition(vOpt, rRot, m_vPos);
				//PointInDirection(m_Target.m_Angles);
				return;
			}	
			break;

			case DEATH:
			{
				CircleAroundTarget();

				VEC_COPY(vOpt, FindFirstPersonCameraPosition(vPos, vF));
	
				MoveCameraToPosition(FindOptimalCameraPosition(), vOpt, deltaTime);
				PointAtPosition(vOpt, rRot, m_vPos);
				return;
			}
			break;
/*
			case CIRCLING:
			{
				CircleAroundTarget();
				return;
			}
			break;
*/
			case FIRSTPERSON:
			{
				DVector vTemp;
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

		VEC_COPY(vOpt, FindOptimalCameraPosition());

		// Move the camera to the optimal position
		// (it will slide or not depending on the m_bSlide param)
		MoveCameraToPosition(vOpt, vOpt, deltaTime);

		// Either point the camera at the player or MLOOK
//		if(m_eOrientation == MLOOK)
//		{
//			ROT_COPY(m_rRotation, rRot);
//		}
//		else
		{
			DVector vTemp;
			VEC_ADD(vTemp, vPos, m_TargetPointAtOffset);
			PointAtPosition(vTemp, rRot, vOpt);
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

DVector CPlayerCamera::Apply(DVector right, DVector up, DVector forward, DVector copy)
{
	DVector target;
	
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

void CPlayerCamera::PointAtPosition(DVector pos, DRotation rRot, DVector pointFrom)
{
	if (!m_pClientDE) return;

//	m_pServerDE->GetObjectPos(m_hObject, &vPos);
//	vTargetPos.y = vPos.y; // Don't look up/down.

	DVector vDir;
	VEC_SUB(vDir, pos, pointFrom);
	VEC_NORM(vDir);

	m_pClientDE->AlignRotation(&m_rRotation, &vDir, NULL);

	//m_Angles.Copy(m_Target.m_Angles);
	
	//m_Angles.z = angles.z;
	//m_Angles.x = angles.x;
/*	DVector vAngles;
	VEC_INIT(vAngles);

	m_rRotation = rRot;


	DFLOAT diffX = pos.x - m_vPos.x;
	DFLOAT diffY = pos.z - m_vPos.z;
	vAngles.y = (DFLOAT)atan2(diffX, diffY);

	DVector		target, copy;
	DVector	up, right, forward;

	m_pClientDE->GetRotationVectors(&m_rRotation, &up, &right, &forward);

	VEC_SUB(copy, pos, pointFrom);

	target = Apply(right, up, forward, copy);

	diffX = target.z;
	diffY = target.y;

	//DFLOAT temp = -ATan2(diffY,diffX);
	//if(Abs(temp - m_Angles.x) < .5)

	vAngles.x = (DFLOAT)-atan2(diffY, diffX);

	//PrintVector(pos);
	//GetClientShell().CPrint("X = " + RealStr(m_Angles.x));

	DRotation rOldRot;
	ROT_COPY(rOldRot, m_rRotation);

	m_pClientDE->SetupEuler(&m_rRotation, vAngles.x, vAngles.y, vAngles.z);

	// Make sure rotation is valid...

	m_pClientDE->GetRotationVectors(&m_rRotation, &up, &right, &forward);
	if (up.y < 0) ROT_COPY(m_rRotation, rOldRot);
*/
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerCamera::SetCameraState()
//
//	PURPOSE:	Set the state/orientation of the camera
//
// ----------------------------------------------------------------------- //
/*
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
*/

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerCamera::RotateCameraState()
//
//	PURPOSE:	Rotate the camera clockwise or counterclockwise around 
//				the target
//
// ----------------------------------------------------------------------- //
/*
void CPlayerCamera::RotateCameraState(DBOOL bClockwise)
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
*/

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerCamera::MoveCameraToPosition()
//
//	PURPOSE:	Move the camera to a position over a time period
//
// ----------------------------------------------------------------------- //

void CPlayerCamera::MoveCameraToPosition(DVector pos, DVector vStartPos, DFLOAT deltaTime)
{
	if (!m_pClientDE || !m_hTarget) return;

	DFLOAT nCurrentTime = m_pClientDE->GetTime();

	// Handle transition
	if (m_eCameraMode == GOINGFIRSTPERSON || m_eCameraMode == GOINGCHASE || m_eCameraMode == DEATH)
	{

		if (nCurrentTime > m_fTransitionStart + m_fTransitionTime)
		{
			switch (m_eCameraMode)
			{
				case GOINGFIRSTPERSON:
					m_eCameraMode = FIRSTPERSON;
					break;
				case GOINGCHASE:
					m_eCameraMode = CHASE;
					break;
			}
		}
		else
		{
			// vStartPos is starting pos, pos is the position we want
			DFLOAT percentage = (nCurrentTime - m_fTransitionStart) / (m_fTransitionTime);

			DVector vMagnitude;
			VEC_SUB (vMagnitude, pos, vStartPos);
			VEC_MULSCALAR (vMagnitude, vMagnitude, percentage);
			VEC_ADD (pos, vStartPos, vMagnitude);
		}
	}

	DVector	dir;
	VEC_SUB(dir, pos, m_vPos);

	DFLOAT multiplier = 1.0f; // 0.5f;
	
	DVector	toMove;
	VEC_MULSCALAR(toMove, dir, multiplier);
	
	DFLOAT moveMag;

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
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerCamera::FindOptimalCameraPosition()
//
//	PURPOSE:	Find the optimal camera position
//
// ----------------------------------------------------------------------- //

DVector	CPlayerCamera::FindFirstPersonCameraPosition(DVector vPos, DVector vF)
{
	DVector vTemp;
	VEC_MULSCALAR(vTemp, vF, m_TargetFirstPersonOffset.x);
	VEC_ADD(vPos, vPos, vTemp);
	vPos.y += m_TargetFirstPersonOffset.y;

	return vPos;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerCamera::FindOptimalCameraPosition()
//
//	PURPOSE:	Find the optimal camera position
//
// ----------------------------------------------------------------------- //

DVector	CPlayerCamera::FindOptimalCameraPosition()
{
	DVector pos;
	VEC_INIT(pos);

	if (!m_pClientDE || !m_hTarget) return pos;

	DVector		up, right, forward, dir;
	DFLOAT		distToOptimal;
	DVector		TargetPlusOffset;
	
	DVector vTargetPos;
	m_pClientDE->GetObjectPos(m_hTarget, &vTargetPos);

	DRotation rRot;
	m_pClientDE->GetObjectRotation(m_hTarget, &rRot);

	if (Equal(vTargetPos, m_vLastTargetPos) && Equal(rRot, m_rLastTargetRot) && m_eCameraMode != DEATH)
	{
		return m_vLastOptPos;
	}
	else
	{
		VEC_COPY(m_vLastTargetPos, vTargetPos);
		ROT_COPY(m_rLastTargetRot, rRot);
	}
		
	DVector vTemp;

	if (m_eCameraMode == DEATH)
	{
		VEC_COPY(vTemp, m_TargetDeathOffset);
	}
	else
	{
		VEC_COPY(vTemp, m_TargetChaseOffset);
	}
	VEC_ADD(vTemp, vTargetPos, vTemp);
	VEC_COPY(TargetPlusOffset, vTemp);

	m_pClientDE->GetRotationVectors(&rRot, &up, &right, &forward);

	//	pos = TargetPlusOffset + right*m_OptX + up*m_OptY + forward*m_OptZ;
	
	DVector vTemp1, vTemp2;
	if (m_eCameraMode == DEATH)
	{
		VEC_MULSCALAR(vTemp, right, m_DeathOptX);
		VEC_MULSCALAR(vTemp2, forward, m_DeathOptZ);
	}
	else
	{
		VEC_MULSCALAR(vTemp, right, m_OptX);
		VEC_MULSCALAR(vTemp2, forward, m_OptZ);
	}
	VEC_MULSCALAR(vTemp1, up, m_OptY);

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

#ifdef DOING_EXTRA_CHECKS

	// Make sure we aren't clipping into walls...
	
	DFLOAT fClipDistance	= 100.0f; // 15.0f;
	DBOOL bClipRightIssues	= DTRUE;
	DBOOL bClipUpIssues		= DTRUE;


	// Check for walls to the right...

	VEC_MULSCALAR(vTemp, right, fClipDistance);
	VEC_ADD(vTemp, pos, vTemp);

	VEC_COPY(iQuery.m_From, pos);
	VEC_COPY(iQuery.m_To, vTemp);

	if (m_pClientDE->IntersectSegment(&iQuery, &iInfo))
	{
		VEC_SUB(vTemp, iInfo.m_Point, pos);
		DFLOAT fDist = (fClipDistance - VEC_MAG(vTemp));

		VEC_MULSCALAR(vTemp, right, -fDist)
		VEC_ADD(pos, pos, vTemp);
	}
	else
	{
		bClipRightIssues = DFALSE;
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
			DFLOAT fDist = (fClipDistance - VEC_MAG(vTemp));

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
		DFLOAT fDist = (fClipDistance - VEC_MAG(vTemp));

		VEC_MULSCALAR(vTemp, up, -fDist)
		VEC_ADD(pos, pos, vTemp);
	}
	else
	{
		bClipUpIssues = DFALSE;
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
			DFLOAT fDist = (fClipDistance - VEC_MAG(vTemp));

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

void CPlayerCamera::PrintVector(DVector v)
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

void CPlayerCamera::StartCircle()
{
	if (!m_pClientDE || !m_hTarget) return;
/*
	if(m_eCameraMode == DEATH)
	{
		m_CircleHeightOffset = HeightOffset;
		m_CircleRadius		 = Radius;
		m_CircleHeightOffset = PointAtOffset;
		m_CircleTime		 = Time;
		m_bStartCircle		 = DFALSE;

		DVector	vTargetPos, up, right, forward;
		DRotation rRot;

		m_pClientDE->GetObjectRotation(m_hTarget, &rRot);
		m_pClientDE->GetRotationVectors(&rRot, &up, &right, &forward);
		m_pClientDE->GetObjectPos(m_hTarget, &vTargetPos);
*/
//		SaveState();

//		SaveCameraMode();
		//GetClientShell().CPrint("Going circle");
//		m_eCameraMode = CIRCLING;

		m_CircleStartTime = m_pClientDE->GetTime();
		
/*		
		//	m_Pos.Copy(m_Target.m_Pos + right*0 + up*m_CircleHeightOffset - forward*m_CircleRadius);
		DVector vTemp, vTemp1;
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
*/
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

	DVector vTargetPos;
	DRotation rRot;
	DVector vU, vR, vF;

	DFLOAT fTimeDelta = m_pClientDE->GetTime() - m_CircleStartTime;
	DFLOAT fAngle = fTimeDelta * DEFAULT_ROTATE_SPEED; // * MATH_PI * 2;
	fAngle = (DFLOAT)fmod(fAngle, MATH_PI*2);

	m_pClientDE->GetObjectRotation(m_hTarget, &rRot);
	m_pClientDE->EulerRotateY(&rRot, fAngle);
	m_pClientDE->GetRotationVectors(&rRot, &vU, &vR, &vF);

	m_pClientDE->GetObjectPos(m_hTarget, &vTargetPos);

	VEC_MULSCALAR(vF, vF, -m_CameraDistBack*1.2f)
//	VEC_ADD(vTargetPos, vTargetPos, vF);

	m_DeathOptX = vF.x;
	m_DeathOptZ = vF.z;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerCamera::VCompare()
//
//	PURPOSE:	Compare two vectors
//
// ----------------------------------------------------------------------- //

DBOOL CPlayerCamera::VCompare(DVector a, DVector b)
{
	if((fabs(a.x - b.x) > 5.0f) || (fabs(a.y - b.y) > 5.0f) || (fabs(a.z - b.z) > 5.0f))
		return DFALSE;
	else
		return DTRUE;
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

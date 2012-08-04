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
#include "CMoveMgr.h"

VarTrack		g_vtCameraMoveUpTime;
VarTrack		g_vtCameraMoveUpMinVel;
VarTrack		g_vtCameraMaxYDifference;
VarTrack		g_vtCameraClipDistance;
VarTrack		g_vtCameraCollisionObjScale;
VarTrack		g_vtCameraCollisionUseObject;

#define CAMERA_COLLISION_MODEL			"models\\1x1_square.ltb"
#define DEFAULT_COLLISION_MODEL_SCALE	12.0f

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

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerCamera::CPlayerCamera()
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

CPlayerCamera::CPlayerCamera()
{
    m_hTarget           = NULL;
    m_pClientDE         = NULL;

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

	m_hCollisionObject	= LTNULL;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerCamera::~CPlayerCamera()
//
//	PURPOSE:	Destructor
//
// ----------------------------------------------------------------------- //

CPlayerCamera::~CPlayerCamera()
{
	if( m_hCollisionObject )
	{
		g_pLTClient->RemoveObject( m_hCollisionObject );
	}
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
	g_vtCameraCollisionObjScale.Init(pClientDE, "CameraCollisionObjScale", NULL, DEFAULT_COLLISION_MODEL_SCALE );
	g_vtCameraCollisionUseObject.Init(pClientDE, "CameraCollisionUseObject", NULL, 1.0f );

	CreateCameraCollisionObject();

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

void CPlayerCamera::CameraUpdate(float deltaTime)
{
	if (!m_pClientDE || !m_hTarget) return;

    LTRotation rRot;
    LTVector vOpt, vPos;

	g_pLTClient->GetObjectPos(m_hTarget, &vPos);
	g_pLTClient->GetObjectRotation(m_hTarget, &rRot);

    LTVector vF = rRot.Forward();

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
					// g_pLTClient->CPrint("Camera orientation : %0.4f, %0.4f, %0.4f", VEC_EXPAND(m_rRotation.Forward()));
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

void CPlayerCamera::UpdateFirstPerson(LTVector vF, LTVector vPos, float fDeltaTime)
{
	vF.y = 0.0f;
	vPos += (vF * m_TargetFirstPersonOffset.x);
	vPos.y += m_TargetFirstPersonOffset.y;
	
	// Modify the position so it's always based on the standing height...

	vPos.y += g_pMoveMgr->GetCurrentHeightDifference();

	// Make sure we don't move the camera up too fast (going up stairs)...

	if (vPos.y > (m_vPos.y + 1.0f))
	{
        float fNewY = m_vPos.y;
		CMoveMgr* pMoveMgr = g_pPlayerMgr->GetMoveMgr();
		if (!pMoveMgr) return;

        LTBOOL bFreeMovement = pMoveMgr->IsFreeMovement();

		if (!bFreeMovement)
		{
            float fMaxYDiff = g_vtCameraMaxYDifference.GetFloat();
            float fYDiff = (vPos.y - fNewY);

			// Force the camera to never be more than fMaxYDiff away from
			// its "real" position...

			if (fYDiff > fMaxYDiff)
			{
				fNewY  += (fYDiff - fMaxYDiff);
				fYDiff = fMaxYDiff;
			}

            float fVel = fYDiff / g_vtCameraMoveUpTime.GetFloat();
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

LTVector CPlayerCamera::Apply(const LTVector &right, const LTVector &up, const LTVector &forward, const LTVector &copy)
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

void CPlayerCamera::PointAtPosition(const LTVector &pos, const LTRotation &rRot, const LTVector &pointFrom)
{
	if (!m_pClientDE) return;

    LTVector vAngles;
	vAngles.Init();

	m_rRotation = rRot;

    float diffX = pos.x - m_vPos.x;
    float diffY = pos.z - m_vPos.z;
    vAngles.y = (float)atan2(diffX, diffY);

    LTVector     target, copy;

	copy = pos - pointFrom;

	target = Apply(m_rRotation.Right(), m_rRotation.Up(), m_rRotation.Forward(), copy);

	diffX = target.z;
	diffY = target.y;


    vAngles.x = (float)-atan2(diffY, diffX);

    LTRotation rOldRot;
    rOldRot = m_rRotation;

	m_rRotation = LTRotation(VEC_EXPAND(vAngles));

	// Make sure rotation is valid...

    if (m_rRotation.Up().y < 0) m_rRotation = rOldRot;
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

void CPlayerCamera::MoveCameraToPosition(LTVector pos, float deltaTime)
{
	if (!m_pClientDE || !m_hTarget) return;

	if (m_eCameraMode == GOINGFIRSTPERSON)
	{
		m_eCameraMode = FIRSTPERSON;

        LTRotation rRot;
		g_pLTClient->GetObjectRotation(m_hTarget, &rRot);

        LTVector vF = rRot.Forward();

        LTVector vPos;
		g_pLTClient->GetObjectPos(m_hTarget, &vPos);

		vF.y = 0.0f;
		vPos += (vF * m_TargetFirstPersonOffset.x);
		vPos.y += m_TargetFirstPersonOffset.y;

		m_vPos = vPos;

		PointInDirection(rRot);
		return;
	}

    LTVector dir;
	dir = (pos - m_vPos);

    float multiplier = 1.0f; // 0.5f;

    LTVector toMove;
	toMove = (dir * multiplier);

    float moveMag;

	if (m_bSlide)
	{
		moveMag = toMove.Length();

		if (moveMag > dir.Length())
		{
			moveMag = dir.Length();
		}

		if (toMove.x != 0.0f || toMove.y != 0.0f || toMove.z != 0.0f)
		{
			toMove.Normalize();
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

    LTVector     dir;
    float      distToOptimal;
    LTVector     TargetPlusOffset;

    LTVector vTargetPos;
	g_pLTClient->GetObjectPos(m_hTarget, &vTargetPos);

    LTRotation rRot;
	g_pLTClient->GetObjectRotation(m_hTarget, &rRot);

	if (vTargetPos.NearlyEquals(m_vLastTargetPos, 1.0f) && (rRot == m_rLastTargetRot))
	{
		return m_vLastOptPos;
	}
	else
	{
		m_vLastTargetPos = vTargetPos;
        m_rLastTargetRot = rRot;
	}

    LTVector vTemp;
	vTemp = vTargetPos + m_TargetChaseOffset;
	TargetPlusOffset = vTemp;

	//	pos = TargetPlusOffset + right*m_OptX + up*m_OptY + forward*m_OptZ;

	ClientIntersectQuery iQuery;
	ClientIntersectInfo  iInfo;

	pos = TargetPlusOffset + 
		(rRot.Right() * m_OptX) + 
		(rRot.Up() * m_OptY) + 
		(rRot.Forward() * m_OptZ);

	vTemp = TargetPlusOffset - pos;
	distToOptimal = vTemp.Length();

	dir = pos - TargetPlusOffset;
	dir.Normalize();

	iQuery.m_From = TargetPlusOffset;
	iQuery.m_To = pos;

	if (m_pClientDE->IntersectSegment(&iQuery, &iInfo))
	{
		vTemp = iInfo.m_Point - TargetPlusOffset;

		// If there was something in the way, move in front of that thing.
		if (vTemp.Length() < distToOptimal)
		{
			pos = iInfo.m_Point + iInfo.m_Plane.m_Normal;
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

void CPlayerCamera::StartCircle(float HeightOffset, float Radius, float PointAtOffset, float Time)
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

		g_pLTClient->GetObjectRotation(m_hTarget, &rRot);
		g_pLTClient->GetObjectPos(m_hTarget, &vTargetPos);

		SaveState();

		SaveCameraMode();
		//GetClientShell().CPrint("Going circle");
		m_eCameraMode = CIRCLING;

		m_CircleStartTime = m_pClientDE->GetTime();


		m_vPos = vTargetPos + 
			((rRot.Up() * m_CircleHeightOffset) - 
			 (rRot.Forward() * m_CircleRadius));

		// PointAtPosition(m_Target.m_Pos+CreateVector(0,m_CircleHeightOffset,0), m_Target.m_Angles, m_Pos);
		LTVector vTemp = vTargetPos + LTVector(0.0f, m_CircleHeightOffset, 0.0f);

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

    float diff = (m_pClientDE->GetTime() - m_CircleStartTime) / m_CircleTime;

	if(diff >= 1.0f)
	{
		RestoreState();
		RestoreCameraMode();
		//GetClientShell().CPrint("Done with circle");
		return;
	}

	//m_Angles.y = m_SaveAnglesY + (MATH_CIRCLE*diff);
	m_rRotation.Rotate(m_rRotation.Up(), MATH_CIRCLE*diff);

    LTVector vTargetPos;
    LTRotation rRot;
	g_pLTClient->GetObjectPos(m_hTarget, &vTargetPos);
	g_pLTClient->GetObjectRotation(m_hTarget, &rRot);

	m_vPos = vTargetPos + 
		(rRot.Up() * m_CircleHeightOffset) - 
		(rRot.Forward() * m_CircleRadius);

	LTVector vTemp = vTargetPos + LTVector(0.0f, m_CircleHeightOffset, 0.0f);

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
		g_pLTClient->GetObjectPos(m_hTarget, &m_vPos);

		// Assume for now first person...

        LTRotation rRot;
		g_pLTClient->GetObjectRotation(m_hTarget, &rRot);

        LTVector vF = rRot.Forward();
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

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerCamera::CreateCameraCollisionObject()
//
//	PURPOSE:	Create an object to use for collision detection when calculating a no-clip pos...
//
// ----------------------------------------------------------------------- //

bool CPlayerCamera::CreateCameraCollisionObject()
{
	if( !g_pLTClient )
		return false;

	ObjectCreateStruct ocs;

	SAFE_STRCPY( ocs.m_Filename, CAMERA_COLLISION_MODEL );
	ocs.m_ObjectType	= OT_MODEL;
	ocs.m_Flags			=0;
	ocs.m_Flags2		= FLAG2_PLAYERCOLLIDE;
	ocs.m_Pos.Init();
	
	float fScale = g_vtCameraCollisionObjScale.GetFloat();
	ocs.m_Scale.Init( fScale, fScale, fScale );

	m_hCollisionObject = g_pLTClient->CreateObject( &ocs );
	if( !m_hCollisionObject )
		return false;

	g_pPhysicsLT->SetObjectDims( m_hCollisionObject, &LTVector( fScale, fScale, fScale ), 0 );

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerCamera::CalcNonClipPos()
//
//	PURPOSE:	Get a position that won't clip into the world or other objects...
//				Doesn't actullay move the camera to that position!
//
// ----------------------------------------------------------------------- //

void CPlayerCamera::CalcNonClipPos( LTVector & vPos, LTRotation & rRot )
{
	LTVector vTemp, vU, vR, vF;
	vU = rRot.Up();
	vR = rRot.Right();
	vF = rRot.Forward();

	if( !m_hCollisionObject || (g_vtCameraCollisionUseObject.GetFloat() < 1.0f) )
	{
		CalcNoClipPos_WithoutObject( vPos, rRot );
		return;
	}

	LTVector vCamPos;
	g_pLTClient->GetObjectPos( g_pPlayerMgr->GetCamera(), &vCamPos );

	g_pLTClient->SetObjectPos( m_hCollisionObject, &vCamPos );
	g_pLTClient->SetObjectRotation( m_hCollisionObject, &rRot );

	if( g_vtCameraCollisionObjScale.GetFloat() != DEFAULT_COLLISION_MODEL_SCALE )
	{
		float fScale = g_vtCameraCollisionObjScale.GetFloat();
		LTVector vScale( fScale, fScale, fScale );

		g_pPhysicsLT->SetObjectDims( m_hCollisionObject, &vScale, 0 );
	}

	uint32 dwMoveObjFlags;
	HOBJECT hMoveObj = g_pMoveMgr->GetObject();

	g_pCommonLT->GetObjectFlags( hMoveObj, OFT_Flags, dwMoveObjFlags );
	
	// Set objects the camera is inside of to be non-solid...

	g_pCommonLT->SetObjectFlags( hMoveObj, OFT_Flags, 0, FLAG_SOLID );
	
	// Our collision object needs to be solid while moving...

	g_pCommonLT->SetObjectFlags( m_hCollisionObject, OFT_Flags,	FLAG_SOLID, FLAG_SOLID );
	
	// Do the collison checking and get the final pos...

	g_pPhysicsLT->MoveObject( m_hCollisionObject, &vPos, 0 );
	g_pLTClient->GetObjectPos( m_hCollisionObject, &vPos );
	
	// Set the objects to their previous solidity...

	g_pCommonLT->SetObjectFlags( hMoveObj, OFT_Flags, dwMoveObjFlags, FLAG_SOLID );
	g_pCommonLT->SetObjectFlags( m_hCollisionObject, OFT_Flags, 0, FLAG_SOLID );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerCamera::CalcNoClipPos_WithoutObject()
//
//	PURPOSE:	Just use ray casts to test for collision...
//
// ----------------------------------------------------------------------- //

void CPlayerCamera::CalcNoClipPos_WithoutObject( LTVector & vPos, LTRotation & rRot )
{
	LTVector vTemp, vU, vR, vF;
	vU = rRot.Up();
	vR = rRot.Right();
	vF = rRot.Forward();

	
	uint32 dwFlags = INTERSECT_OBJECTS | IGNORE_NONSOLID | INTERSECT_HPOLY;

	// Make sure we aren't clipping into walls...

    float fClipDistance = g_vtCameraClipDistance.GetFloat();


	// Check for walls to the right...

	vTemp = (vR * fClipDistance);
	vTemp += vPos;

	ClientIntersectQuery iQuery;
	ClientIntersectInfo  iInfo;

	iQuery.m_Flags = dwFlags;
	iQuery.m_From = vPos;
	iQuery.m_To   = vTemp;

	if (g_pLTClient->IntersectSegment(&iQuery, &iInfo))
	{
		vTemp = iInfo.m_Point - vPos;
        float fDist = (fClipDistance - vTemp.Length());

		vTemp = (vR * -fDist);
		vPos += vTemp;
	}
	else
	{
	 	// If we didn't adjust for a wall to the right, check walls to the left...

		vTemp = (vR * -fClipDistance);
		vTemp += vPos;

		iQuery.m_Flags = dwFlags;
		iQuery.m_From = vPos;
		iQuery.m_To = vTemp;

		if (g_pLTClient->IntersectSegment(&iQuery, &iInfo))
		{
			vTemp = iInfo.m_Point - vPos;
            float fDist = (fClipDistance - vTemp.Length());

			vTemp = (vR * fDist);
			vPos += vTemp;
		}
	}

	// Check for ceilings...

	vTemp = vU * fClipDistance;
	vTemp += vPos;

	iQuery.m_Flags = dwFlags;
	iQuery.m_From = vPos;
	iQuery.m_To = vTemp;

	if (g_pLTClient->IntersectSegment(&iQuery, &iInfo))
	{
		vTemp = iInfo.m_Point - vPos;
        float fDist = (fClipDistance - vTemp.Length());

		vTemp = vU * -fDist;
		vPos += vTemp;
	}
	else
	{
 		// If we didn't hit any ceilings, check for floors...

		vTemp = vU * -fClipDistance;
		vTemp += vPos;

		iQuery.m_Flags = dwFlags;
		iQuery.m_From = vPos;
		iQuery.m_To = vTemp;

		if (g_pLTClient->IntersectSegment(&iQuery, &iInfo))
		{
			vTemp = iInfo.m_Point - vPos;
            float fDist = (fClipDistance - vTemp.Length());

			vTemp = vU * fDist;
			vPos += vTemp;
		}
	}

	// Check infront of us...

	vTemp = vF * fClipDistance;
	vTemp += vPos;

	iQuery.m_Flags = dwFlags;
	iQuery.m_From = vPos;
	iQuery.m_To = vTemp;

	if( g_pLTClient->IntersectSegment( &iQuery, &iInfo ))
	{
		vTemp = iInfo.m_Point - vPos;
		float fDist = (fClipDistance - vTemp.Length());

		vTemp = vF * -fDist;
		vPos += vTemp;
	}
	else
	{
		vTemp = vF * -fClipDistance;
		vTemp += vPos;

		iQuery.m_Flags = dwFlags;
		iQuery.m_From = vPos;
		iQuery.m_To = vTemp;

		if( g_pLTClient->IntersectSegment( &iQuery, &iInfo ))
		{
			vTemp = iInfo.m_Point - vPos;
			float fDist = (fClipDistance - vTemp.Length());

			vTemp = vU * -fDist;
			vPos += vTemp;
		}
	}
}
// ----------------------------------------------------------------------- //
//
// MODULE  : AutoTargetMgr.cpp
//
// PURPOSE : AutoTargetMgr - handle auto targeting for vehicles and easy play
//
// CREATED : 2/28/03
//
// (c) 2003 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "AutoTargetMgr.h"
#include "CharacterFX.h"
#include "AimMagnetFX.h"
#include "SurfaceFunctions.h"
#include "CMoveMgr.h"
#include "VehicleMgr.h"
#include "PlayerMgr.h"
#include "ClientWeaponMgr.h"
#include <algorithm>
#include "PlayerCamera.h"
#include "SurfaceDB.h"
#include "GameModeMgr.h"
#include "ClientDB.h"

//these members are static so they can be referenced by the static sort function
LTVector CAutoTargetMgr::m_vFirePos;
LTVector CAutoTargetMgr::m_vForward;

static float kfMinRangeSqr = (40.0f * 40.0f);
extern VarTrack g_vtFOVYNormal;
extern VarTrack g_vtFOVYWide;

static bool DoVectorPolyFilterFn(HPOLY hPoly, void *pUserData, const LTVector& vIntersectPoint)
{
	// Make sure we hit a surface type we care about...

	SurfaceType eSurfType = GetSurfaceType(hPoly);

	if (eSurfType == ST_INVISIBLE)
	{
        return false;
	}

	HRECORD hSurf = g_pSurfaceDB->GetSurface(eSurfType);
	if (hSurf && g_pSurfaceDB->GetBool(hSurf,SrfDB_Srf_bCanShootThrough) && g_pSurfaceDB->GetBool(hSurf,SrfDB_Srf_bCanSeeThrough)) 
		return false;


    return true;
}

static bool AutoTargetFilterFn(HOBJECT hTest, void *pUserData)
{
	// Ignore objects that are invisible
	uint32 dwFlags;
	g_pCommonLT->GetObjectFlags(hTest, OFT_Flags, dwFlags);

	if (!(dwFlags & FLAG_VISIBLE))
	{
		return false;
	}
	
	if(!(dwFlags & FLAG_RAYHIT))
	{
		return false;
	}

	HOBJECT hClientHitBox = NULL;
	CCharacterFX* pCharacter = g_pGameClientShell->GetSFXMgr()->GetCharacterFX(g_pLTClient->GetClientObject());
	if (pCharacter) 
	{
		hClientHitBox = pCharacter->GetHitBox();
	}

	
	// Okay, do normal tests...
	HOBJECT hFilterList[] = 
	{
		g_pLTClient->GetClientObject(),
		g_pPlayerMgr->GetMoveMgr()->GetObject(),
		hClientHitBox,
		NULL
	};

	if (!ObjListFilterFn(hTest, (void*) hFilterList))
	{
		return false;
	}



    return true;
}


// ----------------------------------------------------------------------- //
//
// CAutoTargetMgr - class to handle auto targeting for vehicles and easy play
//
// ----------------------------------------------------------------------- //

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAutoTargetMgr::Instance()
//
//	PURPOSE:	Instatiator of singleton
//
//  This function is the only way to instatiate this object.  It
//  ensures that there is only one object, the singleton.
//
// ----------------------------------------------------------------------- //

CAutoTargetMgr& CAutoTargetMgr::Instance()
{
	static CAutoTargetMgr sAutoTargetMgr;
	return sAutoTargetMgr;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAutoTargetMgr::CAutoTargetMgr
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

CAutoTargetMgr::CAutoTargetMgr()
{
	m_Targets.reserve(MAX_AUTOTARGET_CHARACTERS);

}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAutoTargetMgr::CAutoTargetMgr
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

CAutoTargetMgr::~CAutoTargetMgr()
{
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAutoTargetMgr::Update
//
//	PURPOSE:	Update auto-target
//
// ----------------------------------------------------------------------- //

void CAutoTargetMgr::Update()
{
	//remember whether we were locked on last frame
	bool bWasLocked = m_bLockOn;
	m_bLockOn = false;

	//no auto-aim while zoomed in 
	if (g_pPlayerMgr->GetPlayerCamera()->IsZoomed())
		return;

	// Get our weapon.
	CClientWeaponMgr *pClientWeaponMgr = g_pPlayerMgr->GetClientWeaponMgr();
	CClientWeapon* pClientWeapon = pClientWeaponMgr->GetCurrentClientWeapon();
	if (!pClientWeapon) return;

	HWEAPONDATA hWpnData = g_pWeaponDB->GetWeaponData(pClientWeapon->GetWeaponRecord(), !USE_AI_DATA);

	AutoTargetType eUseAutoTarget = (AutoTargetType)g_pWeaponDB->GetInt32( hWpnData, WDB_WEAPON_nUseAutoTarget );
	if (eUseAutoTarget == AT_NEVER || (eUseAutoTarget == AT_EASY && g_pGameClientShell->GetDifficulty() != GD_EASY) )
		return;

	// Check if they want to manually aim.
	if( g_pPlayerMgr->IsManualAim( ))
		return;

	m_fAngle = g_pWeaponDB->GetFloat( hWpnData, WDB_WEAPON_fAutoTargetAngle );
	m_fRangeSqr = g_pWeaponDB->GetFloat( hWpnData, WDB_WEAPON_fAutoTargetRange );
	m_fRangeSqr *= m_fRangeSqr;
	
	// Get the camera position and set up the vectors
	m_vFirePos = g_pPlayerMgr->GetPlayerCamera()->GetCameraPos( );
	LTRotation const& rRot = g_pPlayerMgr->GetPlayerCamera()->GetCameraRotation( );

	m_vForward = rRot.Forward();

	// if we weren't locked last frame, reset our current aim
	if (!bWasLocked)
		m_vCurTarget = m_vForward;

	CSFXMgr* psfxMgr = g_pGameClientShell->GetSFXMgr();
	if(psfxMgr)
	{

		//Generate array of chars sorted by distance
		GenerateCharArray();

		//using this list of chars, generate an array of node positions
		GenerateNodeArray();

		//add any aim magnets to the array
		AddMagnets();

		// sort array proximity to the center of the screen
		qsort(m_NodeArray, m_nNodeCount, sizeof(AutoTargetNode), CompareTargetNodes);

		//find the visible node closest to the center
		m_vTarget = m_vForward;
		m_bLockOn = FindNode();

		if (m_bLockOn)
			InterpolateAim();
			
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAutoTargetMgr::GenerateCharArray()
//
//	PURPOSE:	Fill array with list of chars sorted by distance
//
// ----------------------------------------------------------------------- //
void CAutoTargetMgr::GenerateCharArray()
{
	//clear our target array
	m_Targets.resize(0);

	CSFXMgr* psfxMgr = g_pGameClientShell->GetSFXMgr();

	//step through the chars
	CSpecialFXList* const pCharList = psfxMgr->GetFXList(SFX_CHARACTER_ID);	
	int nNumSFX  = pCharList->GetSize();
	
	for (int nChar=0; nChar < nNumSFX; nChar++)
	{
		CCharacterFX* pChar = (CCharacterFX*)(*pCharList)[nChar];
		if (pChar)
		{
			if (pChar->m_cs.bIsPlayer)
			{
				//filter out local player
			    HLOCALOBJ hPlayerObj = g_pLTClient->GetClientObject();
			    if (hPlayerObj == pChar->GetServerObj())
					continue;
				if(pChar->IsPlayerDead())
					continue;

				//if this is a team game filter out our teammates
				if (GameModeMgr::Instance( ).m_grbUseTeams )
				{
					// Get the client information of the body and us.
					uint32 nId = pChar->m_cs.nClientID;
					CClientInfoMgr* pCIMgr = g_pInterfaceMgr->GetClientInfoMgr();
					CLIENT_INFO* pCI = pCIMgr->GetClientByID(nId);
					CLIENT_INFO *pLocalCI = g_pInterfaceMgr->GetClientInfoMgr()->GetLocalClient();

					// Only allow us to auto-target people on the other team.
					if( pCI && pLocalCI )
					{
						if (pCI->nTeamID == pLocalCI->nTeamID)
							continue;
					}

				}
			}
			else
			{
				// Check alignment of non-players
				if(pChar->m_cs.eCrosshairPlayerStance != kCharStance_Hate)
					continue;
			}

			//filter out anyone outside the cone
			LTVector vTargetPos;
			g_pLTClient->GetObjectPos(pChar->GetServerObj(), &vTargetPos);
			LTVector vOffset(0.0f,32.0f,0.0f);

			// we check both upper and lower parts of the body and if either is in the cone, we're good
			if (IsPointInCone( vTargetPos - vOffset) || IsPointInCone( vTargetPos + vOffset) )
			{
				// we only care about the n closest characters, so...
				// if the new one farther away than the n-th one, drop it, 
				//	otherwise drop the n-th one and insert the new one
			
				//step through the chars we already know about...
				CharFXArray::iterator iter = m_Targets.begin();
				bool bInserted = false;
				while (iter != m_Targets.end() && !bInserted)
				{
					//figure out how far away this one is
					CCharacterFX* pTestChar = (CCharacterFX*)(*iter);
					LTVector vTestPos;
					g_pLTClient->GetObjectPos(pTestChar->GetServerObj(), &vTestPos);
					float fTestDistSqr = m_vFirePos.DistSqr(vTestPos);

					//if this char is farther away than the one we're inserting
					if (fTestDistSqr > m_fRangeSqr)
					{
						//if our list is full, pop off the last one...
						if (m_Targets.size() >= MAX_AUTOTARGET_CHARACTERS)
							m_Targets.pop_back();

						m_Targets.insert(iter,pChar);
						bInserted = true;
					}

					iter++;
				}

				//if we haven't inseted it yet, and we have room, add it to the back
				if (!bInserted && m_Targets.size() < MAX_AUTOTARGET_CHARACTERS)
					m_Targets.push_back(pChar);
			}
		}

	}

}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAutoTargetMgr::GenerateNodeArray()
//
//	PURPOSE:	Find the nodes closest to the center of view
//
// ----------------------------------------------------------------------- //
void CAutoTargetMgr::GenerateNodeArray()
{
	int cNodes;

	ILTModel *pModelLT = g_pLTClient->GetModelLT();

	//clear our node array
	m_nNodeCount = 0;


	//step through the chars we know about
	CharFXArray::iterator iter = m_Targets.begin();
	while (iter != m_Targets.end())
	{
		CCharacterFX* pChar = (CCharacterFX*)(*iter);
		ModelsDB::HSKELETON hModelSkeleton = pChar->GetModelSkeleton();
		cNodes = g_pModelsDB->GetSkeletonNumNodes(hModelSkeleton);
	
		// Enumerate through the nodez
		for(int iNode = 0; iNode < cNodes && m_nNodeCount < MAX_AUTOTARGET_NODES; iNode++)
		{
			ModelsDB::HNODE hCurNode = g_pModelsDB->GetSkeletonNode( hModelSkeleton, iNode );
			if( g_pModelsDB->GetNodeAutoTarget( hCurNode ))
			{
				// get the nodes position
				LTTransform lTrans;
				HMODELNODE hNode;

				char const* szNodeName = g_pModelsDB->GetNodeName( hCurNode );
				if( LT_OK == pModelLT->GetNode( pChar->GetServerObj(), szNodeName, hNode ) )
				{
					if( LT_OK == pModelLT->GetNodeTransform( pChar->GetServerObj(), hNode, lTrans, true ) )
					{	
						m_NodeArray[m_nNodeCount].vPos = lTrans.m_vPos;
						m_NodeArray[m_nNodeCount].hChar = pChar->GetServerObj();
						m_nNodeCount++;
					}
				}
			}
		}
		iter++;
	}

}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAutoTargetMgr::CompareTargetNodes()
//
//	PURPOSE:	Compare two nodes to find which is closest to the center of view
//
// ----------------------------------------------------------------------- //

int CAutoTargetMgr::CompareTargetNodes(const void* lhs, const void* rhs)
{
	AutoTargetNode* pL = (AutoTargetNode*)lhs;
	AutoTargetNode* pR = (AutoTargetNode*)rhs;
	// See how close each is to our line of fire
	LTVector vecDiff1 = pL->vPos - m_vFirePos;
	vecDiff1.Normalize( );

	LTVector vecDiff2 = pR->vPos - m_vFirePos;
	vecDiff2.Normalize( );

	
	float fDiff = (m_vForward.Dot(vecDiff2) - m_vForward.Dot(vecDiff1));

	if (fDiff < 0.0f)
		return -1;
	if (fDiff > 0.0f)
		return 1;

	return 0;

	
		
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAutoTargetMgr::FindNode()
//
//	PURPOSE:	Find the node closest to the center of view
//
// ----------------------------------------------------------------------- //

bool CAutoTargetMgr::FindNode()
{

	IntersectQuery IQuery;
	IntersectInfo IInfo;
	IQuery.m_From = m_vFirePos;						
	IQuery.m_Flags = INTERSECT_HPOLY | INTERSECT_OBJECTS | IGNORE_NONSOLID;
	IQuery.m_FilterActualIntersectFn	= AutoTargetFilterFn;
	IQuery.m_pActualIntersectUserData	= (void*)&IQuery;
	IQuery.m_PolyFilterFn				= DoVectorPolyFilterFn;


	uint8 nNode = 0;
	while (nNode < m_nNodeCount)
	{
		IQuery.m_To = m_NodeArray[nNode].vPos;					
			
		if(g_pLTClient->IntersectSegment(IQuery, &IInfo))
		{
			if(IInfo.m_hObject == m_NodeArray[nNode].hChar)
			{
				m_vTarget = m_NodeArray[nNode].vPos - m_vFirePos;
				m_vTarget.Normalize();
				return true;
			}
			else
			{
//				g_pLTClient->CPrint("no target visible");

			}
		}
		else
		{
			//we didn't hit anything so our view of the target point is unobstructed
			m_vTarget = m_NodeArray[nNode].vPos - m_vFirePos;
			m_vTarget.Normalize();
			return true;
		}
		nNode++;
		
	}
	return false;
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAutoTargetMgr::IsPointInCone()
//
//	PURPOSE:	Test a point to see if he is within the target cone
//
// ----------------------------------------------------------------------- //

bool CAutoTargetMgr::IsPointInCone(const LTVector &vTargetPos)
{
	// convert angle to radians
	float radAngle = (float)(m_fAngle * MATH_PI / 180); //angle;
	
	// divide by 2 because we're taking the half angle left or right of the forward vector
	float cosOfAngle = (float)cos(radAngle/2); //angle / 2.0f;
	
	float fOffset = GetConsoleFloat("AutoTargetOffset",300.0f);

	//check range
	float fDist = m_vFirePos.DistSqr(vTargetPos);
	if  (fDist > m_fRangeSqr || fDist < kfMinRangeSqr)
		return false;


	//make sure it's on screen too
	LTVector vecD = vTargetPos - m_vFirePos;
	vecD.Normalize();
	float MaxScreenAngle = (float)cos( DEG2RAD(g_vtFOVYNormal.GetFloat()) / 2.0);
	if (g_pInterfaceResMgr->IsWidescreen())
	{
		MaxScreenAngle = (float)cos( DEG2RAD(g_vtFOVYWide.GetFloat()) / 2.0);
	}

	if (m_vForward.Dot(vecD) < MaxScreenAngle)
	{
		return false;
	}



	LTVector vNewOrigin = m_vFirePos + (m_vForward * -fOffset);

	LTVector vecDiff = vTargetPos - vNewOrigin;
	vecDiff.Normalize();

	//check the angle
	if (m_vForward.Dot(vecDiff) >= cosOfAngle)
	{
		return true;
	}

	return false;
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAutoTargetMgr::AddMagnets()
//
//	PURPOSE:	Add any aim magnets in our cone to the array
//
// ----------------------------------------------------------------------- //
void CAutoTargetMgr::AddMagnets()
{
	CSFXMgr* psfxMgr = g_pGameClientShell->GetSFXMgr();

	//step through the chars
	CSpecialFXList* const pMagnetList = psfxMgr->GetFXList(SFX_AIMMAGNET_ID);	
	int nNumSFX  = pMagnetList->GetSize();
	
	for (int nMag=0; nMag < nNumSFX; nMag++)
	{
		CAimMagnetFX* pMag = (CAimMagnetFX*)(*pMagnetList)[nMag];
		if (pMag)
		{
			uint32 dwFlags;
			g_pCommonLT->GetObjectFlags(pMag->GetServerObj(), OFT_Flags, dwFlags);

			if (!(dwFlags & FLAG_VISIBLE))
				continue;

			//filter out anyone outside the cone
			LTVector vTargetPos;
			g_pLTClient->GetObjectPos(pMag->GetTarget(), &vTargetPos);

			if (IsPointInCone( vTargetPos ) && m_nNodeCount < MAX_AUTOTARGET_NODES)
			{
				m_NodeArray[m_nNodeCount].vPos = vTargetPos;
				m_NodeArray[m_nNodeCount].hChar = pMag->GetTarget();
				m_nNodeCount++;
			}
		}
	}

}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAutoTargetMgr::GetCrosshairPos()
//
//	PURPOSE:	Generate a point in screen space corresponding the the target pos
//
// ----------------------------------------------------------------------- //
LTVector CAutoTargetMgr::GetCrosshairPos() const
{
	LTVector vTemp;
	if (IsLockedOn())
        vTemp = m_vFirePos + (m_vCurTarget * 100.0f);
	else
		vTemp = m_vFirePos + (m_vForward * 100.0f);

	bool bOnScreen = false;
	LTVector pos = g_pInterfaceMgr->GetScreenFromWorldPos(vTemp, g_pPlayerMgr->GetPlayerCamera()->GetCamera(), bOnScreen);
	return pos;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAutoTargetMgr::InterpolateAim()
//
//	PURPOSE:	Rotate our current aim vector to match our target vector
//
// ----------------------------------------------------------------------- //
void CAutoTargetMgr::InterpolateAim()
{
	//we are currently aimed at m_vCurTarget
	//we want to aim at m_vTarget
	float fActualToTargetAng = m_vTarget.Dot(m_vCurTarget);

	//get the maximum angle we can move
	float fMaxAngVel = ObjectContextTimer( g_pMoveMgr->GetServerObject( )).GetTimerElapsedS( );
	if (IsMultiplayerGameClient())
	{
		fMaxAngVel *= ClientDB::Instance( ).GetMPAutoTargetSpeed();
	}
	else
	{
		fMaxAngVel *= ClientDB::Instance( ).GetAutoTargetSpeed();
	}
	float fCosMaxAngVel = (float)cos(fMaxAngVel);

	//if we are at 180 degrees difference, much can go wrong, so ensure that we aren't,
	//but if we are, we just want to keep looking in the direction that we currently
	//are
	if(fActualToTargetAng >= fCosMaxAngVel)
	{
		//the look target is within our reach, so just go there
		m_vCurTarget = m_vTarget;
	}
	else
	{
		//form a right vector that passes through the arc that we are interpolating
		//upon 
		LTVector vRight = m_vCurTarget - (m_vTarget - m_vCurTarget) / (fActualToTargetAng - 1.0f);
		vRight.Normalize();

		//now we can get our values based upon that space
		m_vCurTarget =  fCosMaxAngVel * m_vCurTarget + (float)sin(fMaxAngVel) * vRight;
		m_vCurTarget.Normalize();
		
	}	

}

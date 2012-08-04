#include "bdefs.h"

#include "geometry.h"
#include "moveobject.h"
#include "collision.h"
#include "syscounter.h"
#include "geomroutines.h"
#include "objectmgr.h"
#include "packetdefs.h"
#include "de_world.h"
#include "ltserverobj.h"
#include "de_mainworld.h"
#include "ltsysoptim.h"
#include "moveplayer.h"
#include "fullintersectline.h"

extern int32 g_CV_NewPlayerPhysics;	// Use the new player physics

void RetransformWorldModel(WorldModelInstance *pWorldModel)
{
	obj_SetupWorldModelTransform(pWorldModel);
	w_TransformWorldModel(pWorldModel, &pWorldModel->m_Transform, !!(pWorldModel->m_Flags & FLAG_BOXPHYSICS));
}


uint32 g_Ticks_MoveObject;
uint32 g_nMoveObjectCalls;


//*
#define MAX_INTERSECTING_OBJECTS	128
#define MAX_CARRIED_OBJECTS			32


class IntersectingObjectArray
{
public:
	MoveState	*m_pState;
	LTObject	**m_pObjects;
	int32		m_nObjects;
	LTObject	*m_DefaultArray[MAX_INTERSECTING_OBJECTS];

	IntersectingObjectArray()
	{
		m_pObjects = m_DefaultArray;
		m_nObjects = 0;
	}
};


struct StartPosInfo
{
	LTObject	*m_pObj;
	LTVector		m_vRelPos;
};


static LTVector g_PushPlanes[6] = 
{
	LTVector(1.0f, 0.0f, 0.0f),
	LTVector(-1.0f, 0.0f, 0.0f),
	LTVector(0.0f, 1.0f, 0.0f),
	LTVector(0.0f, -1.0f, 0.0f),
	LTVector(0.0f, 0.0f, 1.0f),
	LTVector(0.0f, 0.0f, -1.0f)
};

// [kls 9/2/99] Is this a solid piece of the world?
static LTBOOL IsSolidWorld(LTObject *pObj)
{
	if(pObj->IsMainWorldModel() || ((pObj->m_Flags & FLAG_SOLID) && pObj->HasWorldModel()))
	{
		return LTTRUE;
	}

	return LTFALSE;
}

// [RP] - 1/25/02 Is the object solid? Dependent on client/server
LTBOOL IsSolid( uint32 dwFlags, LTBOOL bServer )
{
	if( bServer )
	{
		return !!(dwFlags & FLAG_SOLID);
	}
	
	return !!((dwFlags & FLAG_SOLID) && !(dwFlags & FLAG_CLIENTNONSOLID));
	
}

// Detaches the object from whatever it was standing on.
void DetachObjectStanding(LTObject *pObj)
{

	if(pObj->m_pStandingOn)
	{
		dl_Remove(&pObj->m_StandingOnLink);
		pObj->m_pStandingOn = LTNULL;
	}

	pObj->m_pNodeStandingOn = LTNULL;
}

void DetachObjectsStandingOn(LTObject *pObj)
{
	LTLink *pCur, *pNext;
	LTObject *pStandingObj;

	pCur = pObj->m_ObjectsStandingOn.m_pNext;
	while(pCur != &pObj->m_ObjectsStandingOn)
	{
		pNext = pCur->m_pNext;
		
		pStandingObj = (LTObject*)pCur->m_pData;
		DetachObjectStanding(pStandingObj);
		pStandingObj->m_InternalFlags |= IFLAG_APPLYPHYSICS;
		
		pCur = pNext;
	}
}

// Attaches the object to another one (standing on it).
void SetObjectStanding(LTObject *pObj, LTObject *pStandingOn, const Node *pNode)
{
//	ASSERT(!!pObj->sd == !!pStandingOn->sd);

	DetachObjectStanding(pObj);

	if(pStandingOn)
	{
		dl_Insert(&pStandingOn->m_ObjectsStandingOn, &pObj->m_StandingOnLink);
	}

	pObj->m_pStandingOn = pStandingOn;
	pObj->m_pNodeStandingOn = pNode;
}


inline void SetObjectBoundingBox(LTObject *pObj, LTBOOL bTransformWorldModel)
{
	if(bTransformWorldModel && HasWorldModel(pObj))
	{
		RetransformWorldModel(ToWorldModel(pObj));
	}
}


//collides the object against the BSP.
//returns LTTRUE if its path was diverted.
static bool CollideAgainstWorld
(
	MoveState*		pState,
	const WorldBsp*	pWorldBsp,
	LTObject*		pWorldBspObj,
	LTObject*		pObj,	//the LTObject to collide against the world
	LTVector&		P0,		//the LTObject's initial position
	LTVector&		P1,		//the LTObject's final position
	LTBOOL			bSlide
)
{
	CollideRequest request;
	CollideInfo info;
	float forceMagSqr;
	LTBOOL bSolid;


	if(pObj->m_Flags & FLAG_GOTHRUWORLD)
	{
		return false;
	}
	else
	{
		request.m_pAbstract		= pState->m_pAbstract;
		request.m_pCollisionInfo = pState->m_pAbstract->GetCollisionInfo();
		request.m_pWorld		= pWorldBsp;
		request.m_pWorldObj		= pWorldBspObj;
		request.m_OriginalPos	= P0;
		request.m_NewPos		= P1;
		request.m_Dims			= pObj->GetDims();
		request.m_pObject		= pObj;
		request.m_bSlide		= bSlide;
		request.m_nRestart		= pState->m_nRestart;

		CollideWithWorld( request, &info );

		// Copy the final position to the object's position.
		P1 = info.m_FinalPos;

		// [RP]
		// See if it's standing on anything.
		//bSolid = IsSolidWorld(pWorldBspObj) || (pObj->m_Flags & FLAG_SOLID);
		bSolid = IsSolidWorld(pWorldBspObj) || IsSolid(pObj->m_Flags, pState->m_bServer);

		if( bSolid && info.m_pStandingOn )
		{
			SetObjectStanding(pObj, pWorldBspObj, info.m_pStandingOn);
		}

		// If it was stopped by anything, update for its new position.
		if(info.m_nHits > 0)
		{
			SetObjectBoundingBox(pObj, true);

			//get the pointer to the object that was just hit
			LTObject *pObj2 = (LTObject*)pState->m_pAbstract->GetCollisionInfo()->m_hObject;

			forceMagSqr = info.m_vForce.MagSqr();
			if(forceMagSqr >= pObj->m_ForceIgnoreLimitSqr)
			{
				//if the object has the flag indicating that it wants touch notifications
				//send on the notification
				if(pObj->m_Flags & FLAG_TOUCH_NOTIFY)
				{
					pState->m_pAbstract->DoTouchNotify(pObj,pObj2, info.m_VelOffset, ltsqrtf(forceMagSqr));
				}
			}

			//the touch notification should be recipricated to the object that it collided
			//with if it wishes to receive touch notifications
			if(pObj2!=NULL && forceMagSqr >= pObj2->m_ForceIgnoreLimitSqr)
			{
				if(pObj2->m_Flags & FLAG_TOUCH_NOTIFY)
				{
					pState->m_pAbstract->DoTouchNotify(pObj2,pObj, info.m_VelOffset, ltsqrtf(forceMagSqr));
				}
			}
		
			// Apply the velocity offset.
			if(bSolid)
			{
				pObj->m_Velocity += info.m_VelOffset;
			}
		}

		return info.m_nHits > 0;
	}
}


//---------------------------------------------------------------------------//
static void GetSmallestPushaway
(
	LTVector &moverMin,
	LTVector &moverMax, 
	LTVector &blockerMin,
	LTVector &blockerMax,
	LTVector &pushAmount,
	int32 &pushPlane
)
{
	float		minPush;
	int32			minPushDim = 0;
	
	float		testPush;
	int32			i, curPushPlane, minPushPlane;

	
	minPush = (float)MAX_CREAL;
	curPushPlane = 0;
	minPushPlane = 0;
	
	for(i=0; i < 3; i++)
	{
		testPush = blockerMax[i] - moverMin[i];
		if(fabs(testPush) < fabs(minPush))
		{
			minPush = testPush;
			minPushDim = i;
			minPushPlane = curPushPlane;
		}
		++curPushPlane;
		
		testPush = blockerMin[i] - moverMax[i];
		if(fabs(testPush) < fabs(minPush))
		{
			minPush = testPush;
			minPushDim = i;
			minPushPlane = curPushPlane;
		}
		++curPushPlane;
	}

	pushAmount.Init(0.0f, 0.0f, 0.0f);
	pushAmount[minPushDim] = minPush;
	pushPlane = minPushPlane;
}

// Determines if either object is a world model...
inline LTBOOL IsWorldModel(LTObject *pObj1, LTObject *pObj2 )
{
	// If either of them is a WorldModel, do a more extensive test.
	if(HasWorldModel(pObj1) && !(pObj1->m_Flags & FLAG_BOXPHYSICS))
	{
		return LTTRUE;
	}
	else if(HasWorldModel(pObj2) && !(pObj2->m_Flags & FLAG_BOXPHYSICS))
	{
		return LTTRUE;
	}
	
	return LTFALSE;
}


//---------------------------------------------------------------------------//
//is there a world model involved?
inline bool IsEitherObjectAWorldModel
(
	LTObject *pObj1,
	LTObject *pObj2
)
{
	//NOTE:  why do we care if they have box physics?
	if( HasWorldModel(pObj1) && !(pObj1->m_Flags & FLAG_BOXPHYSICS) )
	{
		return true;
	}
	else if( HasWorldModel(pObj2) && !(pObj2->m_Flags & FLAG_BOXPHYSICS) )
	{
		return true;
	}

	return false;
}

//---------------------------------------------------------------------------//
// Do these boxes intersect to within the provided tolerance?
static inline bool DoBoxesIntersect( LTVector &min1, LTVector &max1, LTVector &min2, LTVector &max2, LTFLOAT fTolerance )
{
	if(	min1.x - max2.x >= -fTolerance || max1.x - min2.x <= fTolerance ||
		min1.y - max2.y >= -fTolerance || max1.y - min2.y <= fTolerance ||
		min1.z - max2.z >= -fTolerance || max1.z - min2.z <= fTolerance )
	{
		return false;
	}
	else
	{
		return true;
	}
}


// Determines if the two objects intersect.  Sets bWorldModel to LTTRUE if one of them is a WorldModel.
static bool DoObjectsIntersect
(
	LTObject *pObj1,
	LTObject *pObj2,
	LTVector *pObj1MinBox,
	LTVector *pObj1MaxBox,
	LTVector *pObj2MinBox,
	LTVector *pObj2MaxBox,
	const LTVector &vObjMoveStart,
	const LTVector &vObjMoveEnd,
	float boxTolerance,
	LTBOOL *bWorldModel
)
{
	WorldModelInstance *pWorldModel;
	LTObject *pOther;
	LTVector *pOtherMinBox, *pOtherMaxBox;

	if (!DoBoxesIntersect(*pObj1MinBox,  *pObj1MaxBox, *pObj2MinBox, *pObj2MaxBox, boxTolerance))
		return false;

	// If either of them is a WorldModel, do a more extensive test.
	if(HasWorldModel(pObj1) && !(pObj1->m_Flags & FLAG_BOXPHYSICS))
	{
		pWorldModel = ToWorldModel(pObj1);
		pOther = pObj2;
		pOtherMinBox = pObj2MinBox;
		pOtherMaxBox = pObj2MaxBox;
	}
	else if( HasWorldModel(pObj2) && !(pObj2->m_Flags & FLAG_BOXPHYSICS) )
	{
		pWorldModel = ToWorldModel(pObj2);
		pOther = pObj1;
		pOtherMinBox = pObj1MinBox;
		pOtherMaxBox = pObj1MaxBox;
	}
	else
	{
		pWorldModel = LTNULL;
		pOtherMinBox = LTNULL;
		pOtherMaxBox = LTNULL;
	}

	// Ok, one of them was a WorldModel, figure out if they really touch.
	if( pWorldModel )
	{
		*bWorldModel = true;

		if( DoesBoxIntersectBSP(pWorldModel->m_pValidBsp->GetRootNode(),
								*pOtherMinBox,
								*pOtherMaxBox,
								pWorldModel->m_ObjectType == OT_CONTAINER) )
		{
			return true;
		}
		else
		{
			return false;
		}
	}
	else
	{
		*bWorldModel = false;

		// Handle point collide physics...
		if (vObjMoveStart != vObjMoveEnd)
		{
			LTObject *pPointCollideObj = LTNULL;

			if ((pObj1->m_Flags & FLAG_POINTCOLLIDE) != 0)
			{
				pPointCollideObj = pObj1;
				pOther = pObj2;
			}
			else if ((pObj2->m_Flags & FLAG_POINTCOLLIDE) != 0)
			{
				pPointCollideObj = pObj2;
				pOther = pObj1;
			}

			if (pPointCollideObj)
			{
				LTVector testPt;
				LTPlane testPlane;
                if (!i_BoundingBoxTest(vObjMoveStart, vObjMoveEnd, pOther, &testPt, &testPlane)) 
				{
					return false;
				}
			}
		}

		return true;
	}
}


// Gives each object a touch notification and creates container links for them.
void DoNonsolidCollision(MoveAbstract *pAbstract, LTObject *pObj1, LTObject *pObj2)
{
	CollisionInfo *pInfo;
	LTVector zeroVec;


	pInfo = pAbstract->GetCollisionInfo();

	// Setup the collision info...
	pInfo->m_Plane.m_Normal.Init();
	pInfo->m_Plane.m_Dist = 0.0f;
	pInfo->m_hPoly = INVALID_HPOLY;
	
	zeroVec.Init();

	if(pObj1->m_Flags & FLAG_TOUCH_NOTIFY)
	{
		pAbstract->DoTouchNotify(pObj1, pObj2, zeroVec, 0.0f);
	}

	if(pObj2->m_Flags & FLAG_TOUCH_NOTIFY)
	{
		pAbstract->DoTouchNotify(pObj2, pObj1, zeroVec, 0.0f);
	}

	// Update area object touching.
	if(pObj1->m_Flags & FLAG_CONTAINER)
	{
		pAbstract->PutObjectInContainer(pObj2, pObj1);
	}
	
	if(pObj2->m_Flags & FLAG_CONTAINER)
	{
		pAbstract->PutObjectInContainer(pObj1, pObj2);
	}
}


// Collides the two solid objects using WorldModel physics for the one that is a WorldModel.
static LTBOOL DoSolidWMCollision
(
	MoveState*	pState,
	LTObject*	pTestObj,
	LTVector&	startPos,
	LTVector&	destPos,
	LTBOOL&		bCollision
)
{
	LTVector pos1, pos2, vecTo, vDir;
	LTBOOL bWorldModel;
	MoveState moveState;
	LTVector minBox, maxBox;

	bCollision = false;

	// Worldmodel moving into an object...
	if( HasWorldModel(pState->m_pObj) )
	{
		// pInputObj is pushing pTestObj.
		pos2 = pTestObj->GetPos();
		
		vecTo = destPos - startPos;
		pos1 = pos2 + vecTo;
		
		vDir = vecTo;
		vDir.Norm();
		pos1 += vDir * 0.5f; // Move a little further back.

		// See if the blocker collides with the mover at any point during the move...
		if( CollideAgainstWorld(pState,
								pState->m_pObj->ToWorldModel()->m_pValidBsp, 
								pState->m_pObj,
								pTestObj,
								pos1,
								pos2, 
								!(pState->m_pObj->m_Flags & FLAG_NOSLIDING) ) )
		{
			bCollision = true;

			if(pState->m_pAbstract->ShouldPushObject(pState, pState->m_pObj, pTestObj))
			{
				// There is a collision, so move the blocker to its new position...
				moveState.Inherit(pState, pTestObj);
				MoveObject(&moveState, pos2, MO_DETACHSTANDING|MO_SETCHANGEFLAG|MO_MOVESTANDINGONS);

				// If they're still intersecting, go to the below area where the pusher is stopped
				// by the blocker.  Only consider it crushing if there is significant penetration, otherwise
				// it's just date-crushing.
				minBox = pTestObj->m_MinBox;
				minBox.x += 0.1f;
				minBox.y += 0.1f;
				minBox.z += 0.1f;
				
				maxBox = pTestObj->m_MaxBox;
				maxBox.x -= 0.1f;
				maxBox.y -= 0.1f;
				maxBox.z -= 0.1f;

				if( DoObjectsIntersect( pState->m_pObj, pTestObj, 
					&pState->m_pObj->m_MinBox, &pState->m_pObj->m_MaxBox, 
					&minBox, &maxBox, 
					pState->m_pObj->GetPos(), pState->m_pObj->GetPos(), 
					0.1f, &bWorldModel) )
				{
					// Still intersecting.. send a crush message to pTestObj.
					pState->m_pAbstract->DoCrush(pTestObj, pState->m_pObj);
				}
				else
				{
					// Mover wasn't blocked...
					return false;
				}
			}

			// mover had a lower blocking priority or the blocker couldn't be pushed out of the way...

			// Get the amount the worldobject has to move back by...
			vecTo = pTestObj->GetPos() - pos2;

			// Move the worldobject back a little...
			destPos += vecTo;

			// Mover was blocked...
			return true;
		}
	}
	// Object moving into a worldmodel...
	else
	{
		ASSERT(HasWorldModel(pTestObj));

		pos2 = destPos;

		// Move the object into the worldobject to see how far it can get...
		if( CollideAgainstWorld(pState, 
								pTestObj->ToWorldModel()->m_pValidBsp, 
								pTestObj,
								pState->m_pObj,
								startPos, pos2, 
								!(pState->m_pObj->m_Flags & FLAG_NOSLIDING) ) )
		{
			// There was a hit...
			bCollision = true;

			if(pState->m_pAbstract->ShouldPushObject(pState, pState->m_pObj, pTestObj))
			{
				vecTo = destPos - pos2;
				pos1 = pTestObj->GetPos() + vecTo;

				// There is a collision, so move the blocker to its new position...
				moveState.Inherit(pState, pTestObj);
				MoveObject( &moveState, pos1, MO_DETACHSTANDING | MO_SETCHANGEFLAG | MO_MOVESTANDINGONS );

				minBox = pTestObj->m_MinBox;
				minBox.x += 0.1f;
				minBox.y += 0.1f;
				minBox.z += 0.1f;
				
				maxBox = pTestObj->m_MaxBox;
				maxBox.x -= 0.1f;
				maxBox.y -= 0.1f;
				maxBox.z -= 0.1f;

				// If they're still intersecting, go to the below area where the pusher is stopped
				// by the blocker.  Only consider it crushing if there is significant penetration, otherwise
				// it's just date-crushing.
				if(	DoObjectsIntersect( pState->m_pObj,
										pTestObj,
										&pState->m_pObj->m_MinBox,
										&pState->m_pObj->m_MaxBox, 
										&minBox,
										&maxBox,
										pState->m_pObj->GetPos(),
										pState->m_pObj->GetPos(),
										0.1f,
										&bWorldModel ) )
				{
					// Still intersecting.. send a crush message to pTestObj.
					pState->m_pAbstract->DoCrush( pTestObj, pState->m_pObj );

					// Get the amount the mover has to move back by...
					vecTo = pTestObj->GetPos() - pos1;

					// Move the mover back a little...
					destPos += vecTo;

					// Mover blocked...
					return true;
				}
			}
			else
			{
				// If the blocking priority is lower, then collideAgainstWorld finds our final position...
				destPos = pos2;

				// Mover blocked...
				return true;
			}

			// Mover not blocked...
			return false;
		}
	}

	// Mover not blocked...
	return false;
}


inline LTBOOL DoSolidBBoxCollision(MoveState *pState, LTObject *pTestObj,
	LTVector &startPos, LTVector &destPos)
{
	LTVector pushAmount, newPos;
	int32 pushPlane;
	float fPlaneDist;
	LTVector vDeltaPos, vNormal;
	float fDot1, fDot2;
	MoveState moveState;


	// If either of the objects don't slide, then we'll need the movement delta...
	if(( pTestObj->m_Flags & FLAG_NOSLIDING ) || ( pState->m_pObj->m_Flags & FLAG_NOSLIDING ))
	{
		// Get the movement delta...
		vDeltaPos = destPos - startPos;
	}

	// Special case if the pusher's blocking priority is greater than the object
	// it hit.  Then it physically moves the blocker.
	if(pState->m_pAbstract->ShouldPushObject(pState, pState->m_pObj, pTestObj))
	{
		GetSmallestPushaway(pTestObj->m_MinBox, pTestObj->m_MaxBox, 
			pState->m_pObj->m_MinBox, pState->m_pObj->m_MaxBox, pushAmount, pushPlane);

		// Is the pushplane on the minimum side of the input object...
		if( pushPlane & 0x01 )
		{
			// Get plane distance...
			fPlaneDist = -pState->m_pObj->m_MinBox[( pushPlane - 1 ) >> 1 ];
		}
		else
		{
			// Get plane distance...
			fPlaneDist = pTestObj->m_MaxBox[( pushPlane ) >> 1 ];
		}

		// Was it pushing the guy up?
		if(pushPlane == 2 || pushPlane == 3)
		{
			// Set the highest guy to be standing on the lowest one.
			if(pState->m_pObj->GetPos().y > pTestObj->GetPos().y)
			{
				// Only stand on if the input object is moving down.
				if(destPos.y < startPos.y)
				{
					SetObjectStanding(pState->m_pObj, pTestObj, LTNULL);
				}
			}
			else
			{
				// Only set pTestObj to be standing on the moving object if it moved up.
				if(destPos.y > startPos.y)
				{
					SetObjectStanding(pTestObj, pState->m_pObj, LTNULL);
				}
			}
		}

		if( !( pTestObj->m_Flags & FLAG_NOSLIDING ))
		{
			newPos = pTestObj->GetPos() + pushAmount;
		}
		else
		{
			// The push amount is the distance along the normal behind the plane.  If we had used
			// a dot product, this number would be negative...
			fDot2 = -pushAmount.Mag();
			vNormal = pushAmount / -fDot2;
			
			fDot1 = fDot2 + vNormal.Dot(vDeltaPos);
			newPos = vDeltaPos * (1.0f - ( -fDot1 / ( fDot2 - fDot1 )));
			newPos += pTestObj->GetPos();
		}

		moveState.Inherit(pState, pTestObj);
		MoveObject(&moveState, newPos, MO_DETACHSTANDING|MO_SETCHANGEFLAG|MO_MOVESTANDINGONS);

		DoInterObjectCollisionResponse(pState->m_pAbstract, pTestObj, 
			pState->m_pObj, &g_PushPlanes[pushPlane], fPlaneDist, INVALID_HPOLY);
						
		// If they're still intersecting, go to the below area where the pusher is stopped
		// by the blocker.  Only consider it crushing if there is significant penetration, otherwise
		// it's just date-crushing.
		if(DoBoxesIntersect( pState->m_pObj->m_MinBox, pState->m_pObj->m_MaxBox, 
			pTestObj->m_MinBox, pTestObj->m_MaxBox, 0.1f ))
		{
			// Still intersecting.. send a crush message to pTestObj.
			pState->m_pAbstract->DoCrush(pTestObj, pState->m_pObj);
		}
		else
		{
			return LTFALSE;
		}
	}
				
	// Find the smallest dimension we can move pInputObj back on.
	GetSmallestPushaway(pState->m_pObj->m_MinBox, pState->m_pObj->m_MaxBox, 
		pTestObj->m_MinBox, pTestObj->m_MaxBox, pushAmount, pushPlane);


	// Is the pushplane on the maximum side of the input object...
	if( pushPlane & 0x01 )
	{
		// Get plane distance...
		fPlaneDist = -pTestObj->m_MinBox[( pushPlane - 1 ) >> 1 ];
	}
	else
	{
		// Get plane distance...
		fPlaneDist = pTestObj->m_MaxBox[( pushPlane ) >> 1 ];
	}

	// Was it pushing the guy up?
	if(pushPlane == 2 || pushPlane == 3)
	{
		// Set the highest guy to be standing on the lowest one.
		if(pState->m_pObj->GetPos().y > pTestObj->GetPos().y)
		{
			if(destPos.y < startPos.y)
			{
				SetObjectStanding(pState->m_pObj, pTestObj, LTNULL);
			}
		}
		else
		{
			if(destPos.y > startPos.y)
			{
				SetObjectStanding(pTestObj, pState->m_pObj, LTNULL);
			}
		}
	}

	if( !( pState->m_pObj->m_Flags & FLAG_NOSLIDING ))
	{
		// Move it back, and find anything it intersects with, again.
		destPos += pushAmount;
	}
	else
	{
		// The push amount is the distance along the normal behind the plane.  If we had used
		// a dot product, this number would be negative...
		fDot2 = -pushAmount.Mag();
		vNormal = pushAmount / -fDot2;
		
		fDot1 = fDot2 - vNormal.Dot(vDeltaPos);
		destPos = vDeltaPos * ( -fDot1 / ( fDot2 - fDot1 ));
		destPos += startPos;
	}

	// Do a collision response.
	DoInterObjectCollisionResponse(pState->m_pAbstract, pState->m_pObj, 
		pTestObj, &g_PushPlanes[pushPlane], fPlaneDist, INVALID_HPOLY);
	
	return LTTRUE;
}

inline void GetMovementBox( LTVector *pvMoveMin, LTVector *pvMoveMax, LTVector *pvDeltaPos, LTVector *pMinBox, LTVector *pMaxBox )
{
	*pvMoveMin = *pMinBox;
	*pvMoveMax = *pMaxBox;

	if( pvDeltaPos->x > 0.0f )
	{
		pvMoveMin->x -= pvDeltaPos->x;
	}
	else if( pvDeltaPos->x < 0.0f )
	{
		pvMoveMax->x -= pvDeltaPos->x;
	}
	if( pvDeltaPos->y > 0.0f )
	{
		pvMoveMin->y -= pvDeltaPos->y;
	}
	else if( pvDeltaPos->y < 0.0f )
	{
		pvMoveMax->y -= pvDeltaPos->y;
	}
	if( pvDeltaPos->z > 0.0f )
	{
		pvMoveMin->z -= pvDeltaPos->z;
	}
	else if( pvDeltaPos->z < 0.0f )
	{
		pvMoveMax->z -= pvDeltaPos->z;
	}
}


inline LTBOOL CheckIntersectOnMovement(MoveState *pState, LTObject *pTestObj)
{
	float t1, t2, tClosest, tFarthest;
	int32 i, nIterations;
	LTBOOL bCollide, bMoved, bStopped;
	LTBOOL bPushAway, bWorldModel, bClosestIsT1;
	LTVector tempDestPos;
	LTVector vTempDestPos, vNewPos;

	 // [RP]
	//bPushAway = (pTestObj->m_Flags & FLAG_SOLID) && (pState->m_pObj->m_Flags & FLAG_SOLID);
	bPushAway = IsSolid(pTestObj->m_Flags, pState->m_bServer) && 
				IsSolid(pState->m_pObj->m_Flags, pState->m_bServer);

	nIterations = 0;
	bWorldModel = IsWorldModel( pState->m_pObj, pTestObj );

	do
	{
		// Loop over all the planes of the blocker.  Put the position of the mover on the plane and see if the
		// objects collide...
		bMoved = bCollide = bStopped = LTFALSE;
		for( i = 0; i < 3 && !bCollide; i++ )
		{
			// t == 0.0f is the start position.  There should've been no collision at the start position, so
			// start this loop assuming no collisions...
			t1 = t2 = 0.0f;

			// Find parameterized value of movement...
			if( pState->m_vDeltaPos[i] < 0.0f || pState->m_vDeltaPos[i] > 0.0f )
			{
				t1 = ( pTestObj->m_MinBox[i] - pState->m_vStartPos[i] ) / pState->m_vDeltaPos[i];
				t1 = CLAMP( t1, 0.0f, 1.0f );
				t2 = ( pTestObj->m_MaxBox[i] - pState->m_vStartPos[i] ) / pState->m_vDeltaPos[i];
				t2 = CLAMP( t2, 0.0f, 1.0f );
			}

			if( t1 < t2 )
			{
				bClosestIsT1 = LTTRUE;
				tClosest = t1;
				tFarthest = t2;
			}
			else
			{
				bClosestIsT1 = LTFALSE;
				tClosest = t2;
				tFarthest = t1;
			}

			// Check if the parameterized value is within range...
			if( 0.0f < tClosest || bWorldModel )
			{
				// Check for collisions at this plane if non-solid or if we are moving toward the test object's
				// minimum plane...
				if( !bPushAway || ( bClosestIsT1 && pState->m_vDeltaPos[i] > 0.0f ) || ( !bClosestIsT1 && pState->m_vDeltaPos[i] < 0.0f ))
				{
					bMoved = LTTRUE;
					VEC_LERP( vNewPos, pState->m_vStartPos, pState->m_vDestPos, tClosest );
					pState->m_pObj->SetPos(vNewPos);
					SetObjectBoundingBox( pState->m_pObj, LTTRUE );

					if( DoBoxesIntersect(pState->m_pObj->m_MinBox, pState->m_pObj->m_MaxBox, 
						pTestObj->m_MinBox, pTestObj->m_MaxBox, 0.01f))
					{
						bCollide = LTTRUE;
					}
				}
			}
			// Check if the parameterized value is within range...
			// Check this position if it's closer to the starting position...
			if( !bCollide && ( 0.0f < tFarthest || bWorldModel ))
			{
				// Check for collisions at this plane if non-solid or if we are moving toward the test object's
				// maximum plane...
				if( !bPushAway || ( bClosestIsT1 && pState->m_vDeltaPos[i] < 0.0f ) || ( !bClosestIsT1 && pState->m_vDeltaPos[i] > 0.0f ))
				{
					bMoved = LTTRUE;
					VEC_LERP( vNewPos, pState->m_vStartPos, pState->m_vDestPos, tFarthest );
					pState->m_pObj->SetPos(vNewPos);
					SetObjectBoundingBox( pState->m_pObj, LTTRUE );

					if( DoBoxesIntersect(pState->m_pObj->m_MinBox, pState->m_pObj->m_MaxBox, 
						pTestObj->m_MinBox, pTestObj->m_MaxBox, 0.01f))
					{
						bCollide = LTTRUE;
					}
				}
			}

			if( bMoved && !bCollide )
			{
				bMoved = LTFALSE;

				// Reset the position to the end of the movement...
				pState->m_pObj->SetPos(pState->m_vStartPos + pState->m_vDeltaPos);
				SetObjectBoundingBox( pState->m_pObj, LTTRUE);
			}
		}

		if( bCollide )
		{
			// Possibly push them away from each other.		
			if( bPushAway )
			{
				// World model physics does complete movement volume collisions, so put the input object at the
				// destination.  No tunneling is possible...
				if( bWorldModel )
				{
					pState->m_pObj->SetPos(pState->m_vDestPos);
					SetObjectBoundingBox(pState->m_pObj, LTTRUE);

					bStopped = DoSolidWMCollision(	pState,
													pTestObj,
													pState->m_vStartPos,
													pState->m_vDestPos,
													bCollide );

					if( bStopped )
					{
						pState->m_pObj->SetPos(pState->m_vDestPos);
						SetObjectBoundingBox( pState->m_pObj, LTTRUE );
					}

				}
				else
				{
					LTVector vDestPos = pState->m_pObj->GetPos();
					bStopped = DoSolidBBoxCollision(pState, pTestObj, pState->m_vStartPos, vDestPos);
					pState->m_pObj->SetPos(vDestPos);
				}
			}
			else
			{
				// If we've got a world model, make sure we really hit it.
				bStopped = LTTRUE;
				if(bWorldModel)
				{
					// We have to copy destpos into a temp variable, cuz CollideAgainstWorld will
					// change it...
					vTempDestPos = pState->m_vDestPos;

					if(HasWorldModel(pTestObj) && !(pTestObj->m_Flags & FLAG_BOXPHYSICS))
					{
						bStopped = CollideAgainstWorld(	pState, 
														pTestObj->ToWorldModel()->m_pValidBsp, 
														pTestObj,
														pState->m_pObj, pState->m_vStartPos, vTempDestPos,
														LTFALSE );
					}
					else if(HasWorldModel(pState->m_pObj) && !(pState->m_pObj->m_Flags & FLAG_BOXPHYSICS))
					{
						bStopped = CollideAgainstWorld(	pState, 
														pState->m_pObj->ToWorldModel()->m_pValidBsp, 
														pState->m_pObj,
														pTestObj, pState->m_vStartPos, vTempDestPos,
														LTFALSE );
					}
					// Adjust the delta position based on the collision
					if (bStopped)
						pState->m_vDeltaPos = vTempDestPos - pState->m_vStartPos;
				}
				else
				{
					// Ok, it has box physics, do a simple collision.
					DoNonsolidCollision( pState->m_pAbstract, pTestObj, pState->m_pObj);
				}

				bStopped = LTFALSE;
			}
		}

		// If we messed with the position of the mover and we weren't stopped by anything, then move us to the end...
		if( bMoved && !bStopped )
		{
			// Reset the position to the end of the movement...
			pState->m_pObj->SetPos(pState->m_vStartPos + pState->m_vDeltaPos);
			SetObjectBoundingBox(pState->m_pObj, LTTRUE);
		}

	}
	// Recheck the path for object's that are pushing around other objects...
	while( bPushAway && bCollide && !bStopped && nIterations++ < 10 );

	return bStopped;
}


inline LTBOOL MaybeCollide(MoveState *pState, LTObject *pTestObj)
{
	LTBOOL bPushAway, bStopped, bCollide, bWorldModel;

	// Don't move objects that are already moving.
	if(pTestObj->m_InternalFlags & IFLAG_MOVING)
		return LTFALSE;

	// Honor FLAG_GOTHRUWORLD.
	// [kls 9/2/99] if(pState->m_pObj->m_Flags & FLAG_GOTHRUWORLD && pTestObj->IsMainWorldModel())
	if(pState->m_pObj->m_Flags & FLAG_GOTHRUWORLD && IsSolidWorld(pTestObj))
	{
		return LTFALSE;
	}

	// Honor FLAG2_SPECIALNONSOLID
	if((pState->m_pObj->m_Flags2 & FLAG2_SPECIALNONSOLID) && (pTestObj->m_Flags2 & FLAG2_SPECIALNONSOLID))
	{
		return LTFALSE;
	}

	// [RP]
	// Possibly push them away from eachother.		
//	bPushAway = (pTestObj->m_Flags & FLAG_SOLID) && 
//		(pState->m_pObj->m_Flags & FLAG_SOLID);
	bPushAway = IsSolid(pTestObj->m_Flags, pState->m_bServer) &&
				IsSolid( pState->m_pObj->m_Flags, pState->m_bServer);

	// [kef 02/21/99] Player collision (i.e. cylinder) physics can avoid tunneling on their own..
	if(bPushAway && ((pState->m_pObj->m_Flags2 & FLAG2_PLAYERCOLLIDE) != 0) )
	{
		return CheckIntersectOnMovement(pState, pTestObj);
	}
	else
	{
		// Make sure the objects actually intersect before we do any collision and movement...

		if( DoObjectsIntersect( pState->m_pObj, pTestObj, &pState->m_vMoveMin, 
			&pState->m_vMoveMax, &pTestObj->m_MinBox, &pTestObj->m_MaxBox, 
			pState->m_vStartPos, pState->m_vDestPos,
			0.0f, &bWorldModel))
		{
			//Special case for main world... objects collide with it unless they have
			//FLAG_GOTHRUWORLD set.
			//NOTE: it checks for FLAG_GOTHRUWORLD
			//above so we don't need to check for it here.
			if(!bPushAway)
			{
				// [kls 9/2/99] if(pTestObj->IsMainWorldModel())
				if(IsSolidWorld(pTestObj))
					bPushAway = LTTRUE;
			}
			
			if( bPushAway )
			{
				if( bWorldModel )
				{
					bStopped = DoSolidWMCollision(	pState,
													pTestObj,
													pState->m_vStartPos,
													pState->m_vDestPos,
													bCollide );
					if(bStopped)
						pState->m_pObj->SetPos(pState->m_vDestPos);

					return bStopped;
				}
				else
				{
					 bStopped = DoSolidBBoxCollision( pState,
													  pTestObj,
													  pState->m_vStartPos,
													  pState->m_vDestPos );
					 if(bStopped)
						pState->m_pObj->SetPos(pState->m_vDestPos);

					return bStopped;
				}
			}
			else
			{
				// Notify the objects.
				DoNonsolidCollision(pState->m_pAbstract, pTestObj, pState->m_pObj);
				return LTFALSE; // Nothing moved.
			}
		}
	}

	return LTFALSE;
}


// Called by WorldTree::FindObjectsInBox.
static void FindObjectsCB(WorldTreeObj *pTreeObj, void *pCBUser)
{
	LTObject *pObject;
	IntersectingObjectArray *pArray;


	if(pTreeObj->GetObjType() != WTObj_DObject)
		return;

	pObject = (LTObject*)pTreeObj;
	pArray = (IntersectingObjectArray*)pCBUser;
	
	if(pArray->m_nObjects < MAX_INTERSECTING_OBJECTS)
	{
		if(pObject == pArray->m_pState->m_pObj)
			return;

		// Check for no possible collisions between these objects...
		if(!IsPhysical(pObject->m_Flags, pArray->m_pState->m_bServer) && !( pObject->m_Flags & FLAG_TOUCHABLE ))
			return;

		// [RP]
//		if(!pArray->m_pState->m_bServer)
//		{
//			if(pObject->m_Flags & FLAG_CLIENTNONSOLID || !(pObject->m_Flags & FLAG_SOLID))
//				return;
//		}

		pArray->m_pObjects[pArray->m_nObjects] = pObject;
		pArray->m_nObjects++;
	}
	else
	{
		dsi_ConsolePrint("FindObjectsCB: Overflowed (more than %d intersecting objects)!",
			MAX_INTERSECTING_OBJECTS);
	}
}


static void DetectAndProcessCollisions
(
	MoveState *pState,
	const LTVector& startPos,
	const LTVector& destPos
)
{
	int32 i, nRestarts;
	LTObject *pTestObj;
	LTObject *pHitObjects[2];
	IntersectingObjectArray objectArray;
	
	pState->m_vStartPos = startPos;
	pState->m_vDestPos = destPos;
	pState->m_vDeltaPos = destPos - startPos;
	pState->m_vMoveCenter = startPos + pState->m_vDeltaPos*0.5f;
	pState->m_fMoveRadius = (0.5f * pState->m_vDeltaPos.Mag()) + pState->m_pObj->GetDims().Mag();

	GetMovementBox(&pState->m_vMoveMin, &pState->m_vMoveMax, &pState->m_vDeltaPos, 
		&pState->m_pObj->m_MinBox, &pState->m_pObj->m_MaxBox);


	objectArray.m_pState = pState;

	if(pState->m_CustomTestObjects)
	{
		objectArray.m_pObjects = pState->m_CustomTestObjects;
		objectArray.m_nObjects = pState->m_nCustomTestObjects;
	}
	else
	{
		pState->m_pWorldTree->FindObjectsInBox(&pState->m_vMoveMin, &pState->m_vMoveMax,
			FindObjectsCB, &objectArray);
	}

	pHitObjects[0] = pHitObjects[1] = LTNULL;

	for( i = objectArray.m_nObjects - 1, nRestarts = 0; i >= 0 && nRestarts < 10; i-- )
	{
		pTestObj = objectArray.m_pObjects[i];

		// Keep track of whether or not this is a re-iteration hit of the same object for stairstepping support
		pState->m_nRestart = ((pTestObj == pHitObjects[0]) || (pTestObj == pHitObjects[1])) ? nRestarts : 0;

		if( MaybeCollide( pState, pTestObj ) )
		{
			// If we hit this guy last time or the time before, then stop the madness...
			if( (nRestarts >= 2) && ((pTestObj == pHitObjects[0]) || (pTestObj == pHitObjects[1])) )
			{
				pState->m_vDestPos = pState->m_vStartPos;
				pState->m_pObj->SetPos(pState->m_vStartPos);
				break;
			}

			// Record the hit...
			pHitObjects[ nRestarts & 1 ] = pTestObj;

			SetObjectBoundingBox(pState->m_pObj, LTTRUE);

			i = objectArray.m_nObjects;
			nRestarts++;

			// Reset some info...
			pState->m_vDestPos = pState->m_pObj->GetPos();
			pState->m_vDeltaPos = pState->m_vDestPos - startPos;
		}
	}
	// If we didn't find someplace to go, then go back to the beginning
	if (nRestarts >= 10)
	{
		pState->m_vDestPos = pState->m_vStartPos;
		pState->m_pObj->SetPos(pState->m_vStartPos);
	}
}


static void GrowDim(MoveState *pState, int32 nDim, float &newDim)
{
	MoveState moveState;

	LTObject *pObj = pState->m_pObj;

	// Find the amount to grow...
	float fOldDim = pObj->GetDims()[nDim];
	float fDiff = newDim - fOldDim;

	// Get original position...
	LTVector vNewPos = pObj->GetPos();
	float fOldPos = vNewPos[nDim];

	// Move the object in negative dir...
	vNewPos[nDim] -= fDiff;
	moveState.Setup(pState->m_pWorldTree, pState->m_pAbstract, pObj, pObj->m_BPriority);
	MoveObject(&moveState, vNewPos, MO_NOSLIDING);

	// Remember where we ended up
	float fLow = pObj->GetPos()[nDim];

	// Move the object in positive dir...
	vNewPos[nDim] = fLow + 2.0f * fDiff;
	MoveObject(&moveState, vNewPos, MO_NOSLIDING);
	
	// Remember where we ended up
	float fHigh = pObj->GetPos()[nDim];

	// Figure out how much space we have
	float fHalf = ( fHigh - fLow ) / 2.0f + fOldDim;
	// Center object...
	vNewPos[nDim] = (fHigh + fLow) / 2.0f;
	MoveObject(&moveState, vNewPos, MO_MOVESTANDINGONS|MO_NOSLIDING|MO_TELEPORT);

	// Set the dim...
	LTVector vNewDims = pObj->GetDims();
	vNewDims[nDim] = fHalf;
	pObj->SetDims(vNewDims);
}


static void MaybeCollideWorldModel(MoveState *pState, LTObject *pTestObj)
{
	LTVector pos1, pos2;
	LTVector dir, min, max, vTemp;
	LTBOOL bRet;
	MoveState moveState;
	WorldModelInstance *pInst;
	int32 betterNotUp;

	// Don't worry about the main world(s).
	if(pTestObj->IsMainWorldModel())
		return;

	pInst = pState->m_pObj->ToWorldModel();

	// Don't worry about it if they don't even intersect.
	if(!DoBoxesIntersect(pState->m_pObj->m_MinBox, pState->m_pObj->m_MaxBox, 
		pTestObj->m_MinBox, pTestObj->m_MaxBox, 0.01f))
		return;

	if( !DoesBoxIntersectBSP(ToWorldModel(pState->m_pObj)->m_pValidBsp->GetRootNode(),
		pTestObj->m_MinBox, pTestObj->m_MaxBox,
		pState->m_pObj->m_ObjectType == OT_CONTAINER) )
		return;

	// [RP]
	// Do a solid collision?
	//if(pState->m_pObj->m_Flags & FLAG_SOLID && pTestObj->m_Flags & FLAG_SOLID)
	if( IsSolid( pState->m_pObj->m_Flags, pState->m_bServer) &&
		IsSolid( pTestObj->m_Flags, pState->m_bServer))
	{
		// pInputObj is pushing pTestObj.
		pos2 = pTestObj->GetPos();
		MatVMul_H(&pos1, pState->m_pWMObjectTransform, &pos2);

		dir = pos1 - pos2;
		if(dir.MagSqr() < 0.001f)
			return;
		
		// This helps avoid error from small rotations.. make sure the starting position does not intersect
		// the WorldModel.
		dir = pTestObj->GetPos() - pState->m_pObj->GetPos();
		dir.Norm();

		betterNotUp = 200;
		while(betterNotUp-- > 0)
		{
			min = pos1 - pTestObj->GetDims();
			max = pos1 + pTestObj->GetDims();

			if(!DoesBoxIntersectBSP(pInst->m_pValidBsp->GetRootNode(), min, max, pInst->m_ObjectType == OT_CONTAINER))
				break;
			
			pos1 += dir;
		}
			
		// Setup fake velocity for the collision response.
		vTemp = pTestObj->m_Velocity;
		pTestObj->m_Velocity = pos2 - pos1;
		bRet = CollideAgainstWorld(	pState, 
									pInst->m_pValidBsp, 
									pState->m_pObj,
									pTestObj,
									pos1, pos2, 
									!( pTestObj->m_Flags & FLAG_NOSLIDING ) );

		pTestObj->m_Velocity = vTemp;

		if(bRet)
		{
			moveState.Setup(pState->m_pWorldTree, pState->m_pAbstract, pTestObj, pTestObj->m_BPriority);
			MoveObject(&moveState, pos2, MO_DETACHSTANDING|MO_SETCHANGEFLAG|MO_MOVESTANDINGONS);

			// If they're still intersecting, this means the world model pushed the object into the world
			// so send the object a crush message.
			if(DoesBoxIntersectBSP(pInst->m_pValidBsp->GetRootNode(),
				pTestObj->m_MinBox, pTestObj->m_MaxBox,
				pInst->m_ObjectType == OT_CONTAINER))
			{
				pState->m_pAbstract->DoCrush(pTestObj, pState->m_pObj);
			}
		}
	}
	else
	{
		DoNonsolidCollision(pState->m_pAbstract, pState->m_pObj, pTestObj);
	}
}


void SetupWorldModelDims(WorldModelInstance *pInstance, LTMatrix *pTransform)
{
	LTVector pts[8], minPt, maxPt;
	const LTVector &vPos = pInstance->GetPos();
	LTVector vDims;
	int32 i;
	WorldBsp *pWorldBsp;

	pWorldBsp = (WorldBsp*)pInstance->m_pOriginalBsp;

	// Set its intial dims to the unrotated WorldModel dims.
	vDims = pWorldBsp->m_MaxBox - pWorldBsp->m_MinBox;
	vDims *= 0.5f;

	pts[0].Init(vPos.x+vDims.x, vPos.y+vDims.y, vPos.z+vDims.z);
	pts[1].Init(vPos.x+vDims.x, vPos.y-vDims.y, vPos.z+vDims.z);
	pts[2].Init(vPos.x+vDims.x, vPos.y+vDims.y, vPos.z-vDims.z);
	pts[3].Init(vPos.x+vDims.x, vPos.y-vDims.y, vPos.z-vDims.z);

	pts[4].Init(vPos.x-vDims.x, vPos.y+vDims.y, vPos.z+vDims.z);
	pts[5].Init(vPos.x-vDims.x, vPos.y-vDims.y, vPos.z+vDims.z);
	pts[6].Init(vPos.x-vDims.x, vPos.y+vDims.y, vPos.z-vDims.z);
	pts[7].Init(vPos.x-vDims.x, vPos.y-vDims.y, vPos.z-vDims.z);

	minPt.Init((float)MAX_CREAL, (float)MAX_CREAL, (float)MAX_CREAL);
	maxPt.Init((float)-MAX_CREAL, (float)-MAX_CREAL, (float)-MAX_CREAL);

	for(i=0; i < 8; i++)
	{
		MatVMul_InPlace_H(pTransform, &pts[i]);
		VEC_MIN(minPt, minPt, pts[i]);
		VEC_MAX(maxPt, maxPt, pts[i]);
	}

	vDims = (maxPt - minPt) * 0.5f;
	pInstance->SetDims(vDims);	
}


void InitialWorldModelRotate(WorldModelInstance *pInstance)
{
	obj_SetupWorldModelTransform(pInstance);
	SetupWorldModelDims(pInstance, &pInstance->m_Transform);
	SetObjectBoundingBox((LTObject*)pInstance, LTTRUE);
}

// This will move MAX_CARRIED_OBJECTS objects standing on the moving object. If there are more than MAX_CARRIED_OBJECTS, 
// then they won't be moved
void MoveObject
(
	MoveState *pState,
	LTVector P1,		//passed by value because it may be clamped
	uint32 flags
)
{
	LTObject *pWasStandingOnObj;
	LTObject *pObjOn;
	LTVector startPos, newPos;//, offset;
	LTLink *pCur, *pNext;
	StartPosInfo startPosInfo[MAX_CARRIED_OBJECTS];
	int32 nNumObjectsStanding;
	LTBOOL bWorldModel;
	MoveState moveState;
	CollisionInfo collisionInfo, *pOldCollisionInfo;

	pState->SetupCall();

	// Don't move again if we're already moving.  Make sure we're moveable.
	if( (( pState->m_pObj->m_InternalFlags & IFLAG_MOVING ) || !pState->m_pObj->IsMoveable()) )
	{
		return;
    }

	// Break its links to containers / contained objects.
	pState->m_pAbstract->BreakContainerLinks(pState->m_pObj);

	// Save some work if we can.
	if( pState->m_pAbstract->CanOptimizeObject( pState->m_pObj ) )
	{
		if(flags & MO_SETCHANGEFLAG)
		{
			pState->m_pAbstract->SetObjectChangeFlags(pState->m_pObj, CF_POSITION);
		}

		pState->m_pObj->SetPos( P1 );
		return;
	}

	++g_nMoveObjectCalls;
	CountAdder cnt(&g_Ticks_MoveObject);

	pState->m_pAbstract->CheckMaxPos(pState, &P1);

	// Stack the collision info so this function is re-entrant
	pOldCollisionInfo = pState->m_pAbstract->GetCollisionInfo();
	pState->m_pAbstract->GetCollisionInfo() = &collisionInfo;
	memset( &collisionInfo, 0, sizeof( CollisionInfo ));

	// Set the moving flag so we can't be moved by things we push. 
	// Also set the apply physics flag so we do physics calcs next time around.
	pState->m_pObj->m_InternalFlags |= IFLAG_MOVING | IFLAG_APPLYPHYSICS;

	// If object is teleporting, then it doesn't really need to travel from somewhere...
	if(flags & MO_TELEPORT)
	{
		startPos = P1;
	}
	else
	{
		startPos = pState->m_pObj->GetPos();
	}

	// Remember who I'm standing on...
	if( pState->m_pObj->m_pStandingOn && !pState->m_pObj->m_pNodeStandingOn && !(flags & MO_TELEPORT) )
	{
		pWasStandingOnObj = pState->m_pObj->m_pStandingOn;
	}
	else
	{
		pWasStandingOnObj = LTNULL;
	}

	nNumObjectsStanding = -1;
	if(flags & MO_MOVESTANDINGONS)
	{
		// Transfer the objects that are on object now, so we can move them later...
		pCur = pState->m_pObj->m_ObjectsStandingOn.m_pNext;

		while(pCur != &pState->m_pObj->m_ObjectsStandingOn )
		{
			pNext = pCur->m_pNext;
			pObjOn = (LTObject*)pCur->m_pData;
			pCur = pNext;

			if( pObjOn->m_Flags & FLAG_DONTFOLLOWSTANDING )
			{
				DetachObjectStanding( pObjOn );
			}
			else
			{
				if( nNumObjectsStanding < MAX_CARRIED_OBJECTS )
				{
					nNumObjectsStanding++;
					startPosInfo[nNumObjectsStanding].m_pObj = pObjOn;
					startPosInfo[nNumObjectsStanding].m_vRelPos = pObjOn->GetPos() - pState->m_pObj->GetPos();
				}
				else
				{
					// Too many...
					DetachObjectStanding( pObjOn );
				}
			}
		}
	}
	else
	{
		DetachObjectsStandingOn( pState->m_pObj );
	}

	// Check collisions with other objects.  We still do this even if teleporting, cuz we need
	// to be put into containers...
	if (IsPhysical(pState->m_pObj->m_Flags, pState->m_bServer))
	{
		// Shortcut to the player collision physics
		if (g_CV_NewPlayerPhysics && 
			CPlayerMover::ShouldMoveObject(pState->m_pObj, pState->m_bServer != LTFALSE))
		{
			// Move the player
			CPlayerMover cMover(*pState, flags);
			LTVector vDest;
			cMover.MoveTo(P1, &vDest);

			// Remember where we ended up
			pState->m_pObj->SetPos(vDest);
		}
		else
		{
			if (flags & MO_DETACHSTANDING)
				DetachObjectStanding( pState->m_pObj );

			// Set curPos to the destination .. it'll be modified below.
			pState->m_pObj->SetPos(P1);

			// Reset its bounding box coordinates to reflect the test position.
			SetObjectBoundingBox(pState->m_pObj, LTTRUE);

			// Collide it with other objects.
			DetectAndProcessCollisions(pState, startPos, pState->m_pObj->GetPos());
		}
	}
	else
	{
		pState->m_pObj->SetPos(P1);
	}

	// Set their final new bounding box coordinates..
	SetObjectBoundingBox(pState->m_pObj, LTTRUE);

	// Done moving it around...
	pState->m_pWorldTree->InsertObject(pState->m_pObj);

	if(flags & MO_MOVESTANDINGONS)
	{
		// Offset everyone that was standing on us by the same amount.
		while( nNumObjectsStanding >= 0 )
		{
			pObjOn = startPosInfo[nNumObjectsStanding].m_pObj;
			newPos = pState->m_pObj->GetPos() + startPosInfo[nNumObjectsStanding].m_vRelPos;
			// Push the carried object into whatever it's standing on so it resets the standing on flag properly
			newPos -= LTVector(0.0f, 0.1f, 0.0f);
			
			moveState.Inherit(pState, pObjOn);
			MoveObject(&moveState, newPos, 
				MO_DETACHSTANDING|MO_SETCHANGEFLAG|MO_MOVESTANDINGONS|(flags & MO_TELEPORT));

			// Only reset the object to be standing on us if it's still near...
			// This may have a problem.  If the object was pushed off us, but still intersects
			// us, then we will falsely consider it standing on us.  If this error turns up
			// then we should do a getsmallestpushaway and make sure it's still above us...
			if(DoBoxesIntersect( pObjOn->m_MinBox, pObjOn->m_MaxBox, 
				pState->m_pObj->m_MinBox, pState->m_pObj->m_MaxBox, -0.01f))
			{
				SetObjectStanding(pObjOn, pState->m_pObj, LTNULL); // It's still standing on me...
			}
			
			nNumObjectsStanding--;
		}
	}

	if(pWasStandingOnObj && !HasWorldModel(pWasStandingOnObj))
	{
		if( DoObjectsIntersect(pWasStandingOnObj, pState->m_pObj, &pWasStandingOnObj->m_MinBox, 
			&pWasStandingOnObj->m_MaxBox, &pState->m_pObj->m_MinBox, &pState->m_pObj->m_MaxBox, 
			pState->m_pObj->GetPos(), pState->m_pObj->GetPos(),
			-0.01f, &bWorldModel))
		{
			SetObjectStanding( pState->m_pObj, pWasStandingOnObj, LTNULL );
		}
	}

	// Set its change flag.
	if(flags & MO_SETCHANGEFLAG)
	{
		pState->m_pAbstract->SetObjectChangeFlags(pState->m_pObj, CF_POSITION);

		if(flags & MO_TELEPORT)
			pState->m_pAbstract->SetObjectChangeFlags(pState->m_pObj, CF_TELEPORT);
	}
	
	pState->m_pAbstract->MoveAttachments(pState);

	// We're done moving.
	pState->m_pObj->m_InternalFlags &= ~IFLAG_MOVING;

	pState->m_pAbstract->GetCollisionInfo( ) = pOldCollisionInfo;
}


LTBOOL ChangeObjectDimensions(MoveState *pState, LTVector &newDims, LTBOOL bCollide, LTBOOL bTrivialReject)
{
	LTVector vDims, vDiff;
	LTBOOL bRet;
	LTObject *pObj;
	float distSqr;

	pState->SetupCall();
	pObj = pState->m_pObj;

	// See if the dims didn't change...
	distSqr = (pObj->GetDims() - newDims).MagSqr();
	if(bTrivialReject && distSqr < 0.0001f)
		return LTTRUE;

	// Save the standing-on state
	LTObject *pObj_WasStandingOn = pObj->m_pStandingOn;
	const Node *pObj_WasStandingOnNode = pObj->m_pNodeStandingOn;
	
	if( bCollide && IsPhysical(pObj->m_Flags, pState->m_bServer) )
	{
		vDims = newDims;
		LTVector vOldPos = pObj->GetPos();
		LTVector vOldDims = pObj->GetDims();

		// Note : X/Z dim changing is incompatible with player collision physics
		uint32 nOldPlayerFlag = pObj->m_Flags2 & FLAG2_PLAYERCOLLIDE;
		pObj->m_Flags2 &= ~FLAG2_PLAYERCOLLIDE;
	
		if( pObj->GetDims().x >= newDims.x )
			pObj->SetDims(LTVector(newDims.x, pObj->GetDims().y, pObj->GetDims().z));
		else
			GrowDim( pState, 0, newDims.x );

		if( pObj->GetDims().z >= newDims.z )
			pObj->SetDims(LTVector(pObj->GetDims().x, pObj->GetDims().y, newDims.z));
		else
			GrowDim( pState, 2, newDims.z );

		// Y dim changing is just fine under player collision physics
		pObj->m_Flags2 |= nOldPlayerFlag;

		if( pObj->GetDims().y >= newDims.y )
			pObj->SetDims(LTVector(pObj->GetDims().x, newDims.y, pObj->GetDims().z));
		else
			GrowDim( pState, 1, newDims.y );

		vDiff = vDims - pObj->GetDims();
		bRet = ( vDiff.MagSqr() < 0.001f );
		newDims = pObj->GetDims();
		if (!bRet)
		{
			pObj->SetDims(vOldDims);
			MoveObject(pState, vOldPos, MO_SETCHANGEFLAG|MO_MOVESTANDINGONS|MO_NOSLIDING|MO_TELEPORT);
		}
	}
	else
	{
		pObj->SetDims(newDims);
		bRet = LTTRUE;
	}

	// Restore the standing-on state
	if( pObj_WasStandingOn )
		SetObjectStanding( pObj, pObj_WasStandingOn, pObj_WasStandingOnNode );

	SetObjectBoundingBox(pObj, LTTRUE);

	pState->m_pWorldTree->InsertObject(pObj);
	return bRet;
}


static void CollideWorldModelCB(WorldTreeObj *pObj, void *pUser)
{
	MoveState *pState;
	LTObject *pObject;


	pState = (MoveState*)pUser;

	if(pObj->GetObjType() != WTObj_DObject)
		return;

	pObject = (LTObject*)pObj;
	ASSERT(pObject->GetCSType() == (ClientServerType)pState->m_bServer);

	if(pObject == pState->m_pObj)
		return;

	MaybeCollideWorldModel(pState, pObject);
}


void RotateWorldModel(MoveState *pState, 
	const LTRotation &cRotation, 
	LTBOOL bDoCollisions)
{
	LTMatrix fullTransform;
	LTMatrix temp, backTrans, backRot, forwardRot, forwardTrans;
	LTVector newPos;
	MoveState moveState;
	WorldModelInstance *pObj;
	LTLink *pCur, *pNext;
	LTObject *pObjOn;
	int32 nNumObjectsStanding;
	StartPosInfo startPosInfo[MAX_CARRIED_OBJECTS];
	uint32 i;
	CollisionInfo collisionInfo, *pOldCollisionInfo;


	++g_nMoveObjectCalls;
	CountAdder cnt(&g_Ticks_MoveObject);

	pState->SetupCall();


	pObj = ToWorldModel(pState->m_pObj);

	// Build the rotation transformation for objects.

	// back to origin, rotate (original WM rotation) transposed, rotate (new WM rotation), back to WM pos
	LTVector vPos = pObj->GetPos();
	backTrans.Init(
		1.0f, 0.0f, 0.0f, -vPos.x,
		0.0f, 1.0f, 0.0f, -vPos.y,
		0.0f, 0.0f, 1.0f, -vPos.z,
		0.0f, 0.0f, 0.0f, 1.0f);

	forwardTrans.Init(
		1.0f, 0.0f, 0.0f, vPos.x,
		0.0f, 1.0f, 0.0f, vPos.y,
		0.0f, 0.0f, 1.0f, vPos.z,
		0.0f, 0.0f, 0.0f, 1.0f);

	quat_ConvertToMatrix((float*)&pObj->m_Rotation, backRot.m);
	MatTranspose3x3(&backRot);

	cRotation.ConvertToMatrix(forwardRot);

	// Transform it into its new rotation and setup its new dims.
	pObj->m_Rotation = cRotation;
	obj_SetupWorldModelTransform(pObj);
	SetupWorldModelDims(pObj, &pObj->m_Transform);
	SetObjectBoundingBox(pObj, LTTRUE);

	// Relocate it in the BSP.
	pState->m_pWorldTree->InsertObject(pState->m_pObj);

	if(!bDoCollisions)
	{
		return;
	}
	
	// If it's only using box physics, don't test for object intersections.
	if(pObj->m_Flags & FLAG_BOXPHYSICS)
		return;

	MatMul(&fullTransform, &forwardTrans, &forwardRot);
	MatMul(&temp, &fullTransform, &backRot);
	MatMul(&fullTransform, &temp, &backTrans);

	// Stack the collision info so this function is re-entrant
	pOldCollisionInfo = pState->m_pAbstract->GetCollisionInfo( );
	pState->m_pAbstract->GetCollisionInfo( ) = &collisionInfo;
	memset( &collisionInfo, 0, sizeof( CollisionInfo ));

	// Transfer the objects that are on object now, so we can move them later...
	pCur = pObj->m_ObjectsStandingOn.m_pNext;
	nNumObjectsStanding = -1;
	while(pCur != &pObj->m_ObjectsStandingOn )
	{
		pNext = pCur->m_pNext;
		pObjOn = (LTObject*)pCur->m_pData;
		pCur = pNext;

		if( pObjOn->m_Flags & FLAG_DONTFOLLOWSTANDING )
		{
			DetachObjectStanding( pObjOn );
		}
		else
		{
			if( nNumObjectsStanding < MAX_CARRIED_OBJECTS )
			{
				nNumObjectsStanding++;
				startPosInfo[nNumObjectsStanding].m_pObj = pObjOn;
				startPosInfo[nNumObjectsStanding].m_vRelPos = pObjOn->GetPos();
			}
			// Too many...
			else
				DetachObjectStanding( pObjOn );
		}
	}
	
	// Find any intersecting objects and push them away.
	pState->m_pWMObjectTransform = &fullTransform;

	// Flag us as moving so no one can push us.
	pState->m_pObj->m_InternalFlags |= IFLAG_MOVING;
	
	if(pState->m_CustomTestObjects)
	{
		for(i=0; i < pState->m_nCustomTestObjects; i++)
		{
			MaybeCollideWorldModel(pState, pState->m_CustomTestObjects[i]);
		}
	}
	else
	{
		// Find objects and test them out..
		pState->m_pWorldTree->FindObjectsInBox(
			&pState->m_pObj->m_MinBox, &pState->m_pObj->m_MaxBox,
			CollideWorldModelCB, pState);
	}

	// Offset everyone that was standing on us by the same amount.
	while( nNumObjectsStanding >= 0 )
	{
		pObjOn = startPosInfo[nNumObjectsStanding].m_pObj;

		// Rotate its position.
		MatVMul_H(&newPos, &fullTransform, &startPosInfo[nNumObjectsStanding].m_vRelPos);
		moveState.Setup(pState->m_pWorldTree, pState->m_pAbstract, pObjOn, pObjOn->m_BPriority);
		// Move down a little so it will hit the object again...
		newPos.y -= 0.5f;
		MoveObject(&moveState, newPos, MO_DETACHSTANDING|MO_SETCHANGEFLAG|MO_MOVESTANDINGONS);

		nNumObjectsStanding--;
	}

	// We're done moving.
	pState->m_pObj->m_InternalFlags &= ~IFLAG_MOVING;

	pState->m_pAbstract->GetCollisionInfo( ) = pOldCollisionInfo;
}

//EOF


// For some reason we can't use certain optimizations in this module or 
// else the raycasting is all screwed up.  This module also can't use
// precompiled headers or else these pragmas don't take effect.
#pragma optimize("", off)
#pragma optimize("as", on)

#include "bdefs.h"
#include "de_world.h"
#include "objectmgr.h"
#include "geometry.h"
#include "syscounter.h"
#include "intersect_line.h"



uint32 g_Ticks_Intersect, g_nIntersectCalls;


static void (*g_FindIntersectionsFn)(const WorldBsp *pWorldBsp, const Node **pNodeIntersectionPtr, 
    LTVector *pIntersectionPosPtr, float *pDistSqrPtr, HPOLY *hWorldPoly,
    LTVector *pPoint1, LTVector *pPoint2, uint8 bWorldModel);


// The current query.
static IntersectQuery *g_pCurQuery;
static uint8 g_bProcessNonSolid;
static uint8 g_bProcessObjects;
static uint8 g_bCheckIfFromPointIsInsideObject;
static uint8 g_bProcessModelObbs;


// The current best intersection (LTNULL if none).
static const Node *g_pWorldIntersection; // The node we intersected if we hit a BSP.
static LTObject *g_pIntersection;
static float g_IntersectionBestDistSqr; // Distance to intersection point squared.
static LTPlane g_IntersectionPlane;
static LTVector g_IntersectionPos;
static HPOLY g_hWorldPoly;  // The WorldModel poly we're touching.
static HMODELNODE g_hModelNode; // The node we hit.

static LTVector g_V, g_VTimesInvVV, g_VOrigin, g_VDir;
static float g_VPTimesInvVV, g_LineLen;




// Get the intersection point and see if it's inside the other dimensions.
#define DO_PLANE_TEST_X(planeCoord, coord0, coord1, coord2, normalDirection) \
    t = (planeCoord - Point1.coord0) / (Point2.coord0 - Point1.coord0);\
    testCoords[0] = Point1.coord1 + ((Point2.coord1 - Point1.coord1) * t);\
    if (testCoords[0] > min.coord1 && testCoords[0] < max.coord1)\
    {\
        testCoords[1] = Point1.coord2 + ((Point2.coord2 - Point1.coord2) * t);\
        if (testCoords[1] > min.coord2 && testCoords[1] < max.coord2)\
        {\
            pIntersectPt->coord0 = planeCoord;\
            pIntersectPt->coord1 = testCoords[0];\
            pIntersectPt->coord2 = testCoords[1];\
            pIntersectPlane->m_Normal.x = normalDirection;\
            pIntersectPlane->m_Normal.y = 0.0f;\
            pIntersectPlane->m_Normal.z = 0.0f;\
            pIntersectPlane->m_Dist = min.x * normalDirection;\
            return true;\
        }\
    }

#define DO_PLANE_TEST_Y(planeCoord, coord0, coord1, coord2, normalDirection) \
    t = (planeCoord - Point1.coord0) / (Point2.coord0 - Point1.coord0);\
    testCoords[0] = Point1.coord1 + ((Point2.coord1 - Point1.coord1) * t);\
    if (testCoords[0] > min.coord1 && testCoords[0] < max.coord1)\
    {\
        testCoords[1] = Point1.coord2 + ((Point2.coord2 - Point1.coord2) * t);\
        if (testCoords[1] > min.coord2 && testCoords[1] < max.coord2)\
        {\
            pIntersectPt->coord0 = planeCoord;\
            pIntersectPt->coord1 = testCoords[0];\
            pIntersectPt->coord2 = testCoords[1];\
            pIntersectPlane->m_Normal.x = 0.0f;\
            pIntersectPlane->m_Normal.y = normalDirection;\
            pIntersectPlane->m_Normal.z = 0.0f;\
            pIntersectPlane->m_Dist = min.y * normalDirection;\
            return true;\
        }\
    }

#define DO_PLANE_TEST_Z(planeCoord, coord0, coord1, coord2, normalDirection) \
    t = (planeCoord - Point1.coord0) / (Point2.coord0 - Point1.coord0);\
    testCoords[0] = Point1.coord1 + ((Point2.coord1 - Point1.coord1) * t);\
    if (testCoords[0] > min.coord1 && testCoords[0] < max.coord1)\
    {\
        testCoords[1] = Point1.coord2 + ((Point2.coord2 - Point1.coord2) * t);\
        if (testCoords[1] > min.coord2 && testCoords[1] < max.coord2)\
        {\
            pIntersectPt->coord0 = planeCoord;\
            pIntersectPt->coord1 = testCoords[0];\
            pIntersectPt->coord2 = testCoords[1];\
            pIntersectPlane->m_Normal.x = 0.0f;\
            pIntersectPlane->m_Normal.y = 0.0f;\
            pIntersectPlane->m_Normal.z = normalDirection;\
            pIntersectPlane->m_Dist = min.z * normalDirection;\
            return true;\
        }\
    }



// Just sets up the current 'closest object'.
#define USE_THIS_OBJECT(pServerObj, distSqr, plane, intersectionPt, hPoly, hNode) \
    g_IntersectionBestDistSqr = distSqr;\
    g_pIntersection = pServerObj;\
    g_IntersectionPlane = plane;\
    g_IntersectionPos = intersectionPt;\
    g_hWorldPoly = hPoly;\
	g_hModelNode = hNode;


// Replaces macro above (left in for reference), so filtering can be done...
inline bool UseThisObject(LTObject* pServerObj, 
						  const float fDistSqr, 
						  const LTPlane & plane, 
						  const LTVector & intersectionPt, 
						  const HPOLY & hPoly,
						  const HMODELNODE hNode)
{
	if (g_pCurQuery->m_FilterActualIntersectFn && 
		!g_pCurQuery->m_FilterActualIntersectFn((HOBJECT)pServerObj, g_pCurQuery->m_pActualIntersectUserData))
	{
		// They said to ignore it..
		return false;
	}
	
	USE_THIS_OBJECT(pServerObj, fDistSqr, plane, intersectionPt, hPoly, hNode);
	return true;
}

inline bool i_BoundingBoxTest(const LTVector& Point1, const LTVector& Point2, const LTObject *pServerObj, 
    LTVector *pIntersectPt, LTPlane *pIntersectPlane)
{
    float t;
    float testCoords[2];
	const LTVector& min = pServerObj->GetBBoxMin();
	const LTVector& max = pServerObj->GetBBoxMax();

    // Left/Right.
    if (Point1.x < min.x) 
	{
        if (Point2.x < min.x) 
		{
            return false;
        }
    
        DO_PLANE_TEST_X(min.x, x, y, z, -1.0f);
    }
    else if (Point1.x > max.x) 
	{
        if (Point2.x > max.x) 
		{
            return false;
        }

        DO_PLANE_TEST_X(max.x, x, y, z, 1.0f);
    }

    // Top/Bottom.
    if (Point1.y < min.y) 
	{
        if (Point2.y < min.y) 
		{
            return false;
        }
    
        DO_PLANE_TEST_Y(min.y, y, x, z, -1.0f);
    }
    else if (Point1.y > max.y) 
	{
        if (Point2.y > max.y) 
		{
            return false;
        }

        DO_PLANE_TEST_Y(max.y, y, x, z, 1.0f);
    }

    // Front/Back.
    if (Point1.z < min.z) 
	{
        if (Point2.z < min.z) 
		{
            return false;
        }
    
        DO_PLANE_TEST_Z(min.z, z, x, y, -1.0f);
    }
    else if (Point1.z > max.z) 
	{
        if (Point2.z > max.z) 
		{
            return false;
        }

        DO_PLANE_TEST_Z(max.z, z, x, y, 1.0f);
    }


	// If we get here and our hackish backwards compatibility flag is set, we need to check
	// to see if Point1 is completely inside the dims.  The above checks don't catch this case...

	if (g_bCheckIfFromPointIsInsideObject)
	{
		if ( (min.x <= Point1.x && Point1.x <= max.x) &&
			 (min.y <= Point1.y && Point1.y <= max.y) &&
			 (min.z <= Point1.z && Point1.z <= max.z) )
		{
			LTVector vNormal = (Point1 - Point2);
			vNormal.Normalize();

			LTPlane pPlane;
			pPlane.Init(vNormal, Point1);

			*pIntersectPt = Point1;
			*pIntersectPlane = pPlane;

			return true;
		}
	}

    return false;
}


inline bool i_QuickSphereTest(const LTObject *pServerObj) 
{
    // Find the closest point to the line.
    // Here's the equation for t:
    // P = point1
    // V = point2 - point1 (ie: direction vector)
    // S = point you're testing
    // t = parametric (P + Vt)
    // t = -(-VS + VP) / VV

    //dirVec = *pPoint2 - *pPoint1;
    //t = pServerObj->m_Pos.Dot(dirVec) - dirVec.Dot(*pPoint1);
    //t /= dirVec.Dot(dirVec);

    float t = g_VTimesInvVV.Dot(pServerObj->GetPos()) - g_VPTimesInvVV;

	//cache this radius since it is a virtual function call and can be somewhat expensive on some
	//object types
	float fRadius = pServerObj->GetRadius();

    if (t < -fRadius || t > (g_LineLen + fRadius)) 
	{
        return false;
    }
    
    // Now see if it's within range.
    LTVector vecTo = g_pCurQuery->m_From + g_V * t - pServerObj->GetPos();
    return vecTo.MagSqr() < pServerObj->GetRadiusSquared();
}

inline bool i_QuickSphereTest2(const LTVector &pVector, const float fRadius) 
{
    // Find the closest point to the line.
    // Here's the equation for t:
    // P = point1
    // V = point2 - point1 (ie: direction vector)
    // S = point you're testing
    // t = parametric (P + Vt)
    // t = -(-VS + VP) / VV

    //dirVec = *pPoint2 - *pPoint1;
    //t = pServerObj->m_Pos.Dot(dirVec) - dirVec.Dot(*pPoint1);
    //t /= dirVec.Dot(dirVec);

    float t = g_VTimesInvVV.Dot(pVector) - g_VPTimesInvVV;

    if (t < -fRadius || t > (g_LineLen + fRadius)) 
	{
        return false;
    }
    
    // Now see if it's within range.
    LTVector vecTo = g_pCurQuery->m_From + g_V * t - pVector;
    return vecTo.MagSqr() < (fRadius * fRadius);
}


// returns true if intersection happened, plus fills hit dist param with 
// the parametric position of the intersection on the ray dir. 
// the obb must be within ray-origin + ray-dir. So ray-dir isn't normalized.
// source : Real-time rendering moller&haines
inline bool i_OrientedBoundingBoxTest(const ModelOBB &mobb, 
									  const LTransform &tf, 
									  const LTVector &origin, 
									  const LTVector &dir, 
									  float &t)
{
	float tmin = -999999999999.0f;
	float tmax = 999999999999.0f;

	LTVector vObbPos = mobb.m_Pos;

	// Setup our OBB's translated position
	LTMatrix obb_mat;
	obb_mat.Identity();
	obb_mat.SetBasisVectors( &tf.m_Rot.Right(), &tf.m_Rot.Up(), &tf.m_Rot.Forward() );
	obb_mat.SetTranslation( tf.m_Pos );
	obb_mat.Apply(vObbPos);  

	LTVector p = vObbPos - origin;
	float e ;
	float f;
	float hi;
	float t1,t2;
	
	// Setup OBB rotation tranforms
	LTMatrix mObbMat;
	mObbMat.SetBasisVectors(&mobb.m_Basis[0], &mobb.m_Basis[1], &mobb.m_Basis[2]);
	
	// Get matrix of our Node's rotation
	LTMatrix mTrMat;
	tf.m_Rot.ConvertToMatrix(mTrMat);
	
	// Apply our node rotation to our obb rotation
	mTrMat.Apply(mObbMat);
	
	// Get our translated basis vectors
	LTVector axis[3];
	mObbMat.GetBasisVectors(&axis[0], &axis[1], &axis[2]);

	LTVector vSize =  mobb.m_Size * 0.5f ; // we want the 1/2 size of the box.

	for( int i = 0 ; i < 3 ; i++ )
	{
		e = axis[i].Dot(p);  
		f = axis[i].Dot(dir);		
		hi= vSize[i]  ; // 

		if( fabs(f) > 0.00015 )
		{
			t1 = ( e + hi ) / f ;
			t2 = ( e - hi ) / f ;
			if( t1 > t2 ) {float v = t2 ; t2 = t1 ; t1 = v ; }
			if(t1 > tmin ) {tmin = t1 ;}
			if(t2 < tmax ) {tmax = t2 ;}
			if(tmin > tmax ) { 
				 return false ; }
			if(tmax < 0  ) { 
				 return false ; }
		}
		else if( ((-e - hi) > 0 ) || ( (-e + hi) < 0 )) { t = 0 ; return false ; }
	}

	if( tmin > 0 ) { t = tmin ;  return true ; }
	else { t = tmax ;  return true ;}
}


// Returns true if the segment hits the world model.
static bool i_TestWorldModel(WorldModelInstance *pObj) 
{
    const Node *pNodeIntersection;
    LTVector intersectionPt;
    LTFLOAT distToIntersectionSqr;
    HPOLY hWorldPoly;
    LTVector points[2], planePt;
    LTPlane tempPlane;


    // Pre-rotate the endpoints for the worldmodel.
    MatVMul_H(&points[0], &pObj->m_BackTransform, (LTVector*)&g_pCurQuery->m_From);
    MatVMul_H(&points[1], &pObj->m_BackTransform, (LTVector*)&g_pCurQuery->m_To);

    g_FindIntersectionsFn(pObj->m_pOriginalBsp, 
        &pNodeIntersection, &intersectionPt, &distToIntersectionSqr, &hWorldPoly,
        &points[0], &points[1], true);
    
    if (!pNodeIntersection) 
	{
        return false;
    }

    MatVMul_InPlace_H(&pObj->m_Transform, &intersectionPt);
    distToIntersectionSqr = g_pCurQuery->m_From.DistSqr(intersectionPt);

    if (distToIntersectionSqr < g_IntersectionBestDistSqr) 
	{
        planePt = pNodeIntersection->GetPlane()->m_Normal * pNodeIntersection->GetPlane()->m_Dist;
        MatVMul_3x3(&tempPlane.m_Normal, &pObj->m_Transform, &pNodeIntersection->GetPlane()->m_Normal);
        tempPlane.m_Dist = tempPlane.m_Normal.Dot(planePt);

        g_pWorldIntersection = pNodeIntersection;

        //USE_THIS_OBJECT(pObj, distToIntersectionSqr, tempPlane, intersectionPt, hWorldPoly);
 		//return true;
		return UseThisObject(pObj, distToIntersectionSqr, tempPlane, intersectionPt, hWorldPoly, INVALID_MODEL_NODE);
	}

    return false;
}

// Returns true if the segment hits a ModelOBB.
static bool i_TestModelOBBS(ModelInstance *pObj) 
{
    LTVector vIntersectPoint(0.0f, 0.0f, 0.0f);
    float currentBestIntersection = 99999999999.0f;
	uint32 currentBestNode = 0;
    bool bHit = false;
    LTransform tf;
    float parametric_dist = 0.0f;

	uint32 nNumObbs = pObj->NumCollisionObjects();
    for(uint32 i = 0; i < nNumObbs; ++i)
    {
        const ModelOBB *obb = pObj->GetCollisionObject(i);

        // Determine OBB Pos in 3D space.
        if (!pObj->GetNodeTransform(obb->m_iNode, tf, true))
        {
            continue;
        }

		// setup the coordinate frame of the obb.
        LTMatrix obb_mat;
        LTVector vTPos(obb->m_Pos);
        obb_mat.Identity();
        obb_mat.SetBasisVectors( &tf.m_Rot.Right(), &tf.m_Rot.Up(), &tf.m_Rot.Forward() );
        obb_mat.SetTranslation( tf.m_Pos );
        obb_mat.Apply(vTPos);

        // Test the sphere
        if(i_QuickSphereTest2(vTPos, obb->m_Radius))
        {           
            if(i_OrientedBoundingBoxTest(*obb, tf, g_VOrigin, g_VDir, parametric_dist))
            {
              
                // If this intersection is closer than the current best, 
                if (parametric_dist < currentBestIntersection)
                {
					// Set our current closest distance
					currentBestIntersection = parametric_dist;

					// Set our current best node ID
                    currentBestNode = obb->m_iNode;

					// Setup our intersection position in world space.
					vIntersectPoint = g_VOrigin + (g_VDir * parametric_dist);
                }
				// We were hit at least once!
                bHit = true;
            }            
        }
    }
    
    if(bHit)
    {
        // Since we aren't filling the return plane. 
		// Make sure they are atleast Initialized to sane values.
	    LTPlane ltPlane;
		ltPlane.m_Normal.Init();
		ltPlane.m_Dist = 0;

        USE_THIS_OBJECT(pObj, 
						tf.m_Pos.DistSqr(g_VOrigin),
						ltPlane,
						vIntersectPoint, 
						INVALID_HPOLY,
						currentBestNode);
        return true;
    }

    return false;
}

// Tries everything it can think of to reject this object intersection.
// If it does intersect and is closer than the current best world intersection
// then it replaces the current one.
inline bool i_HandlePossibleIntersection(const LTVector& Point1, const LTVector& Point2, LTObject *pServerObj)
{
    // Quick sphere test.
    if (i_QuickSphereTest(pServerObj)) 
	{
        // Ok, filter if necessary.
        if (g_pCurQuery->m_FilterFn && 
            !g_pCurQuery->m_FilterFn((HOBJECT)pServerObj, g_pCurQuery->m_pUserData))
        {
            // They said to ignore it..
        }
        else 
		{
            // If it's a WorldModel, add it to the list to be tested later.
            // You can't treat it like a solid box here because a ray could go
            // right through all its geometry.

            // [kls 5/16/00 Always do full intersection tests on world models
            // so we get the correct hpoly information
            // if (HasWorldModel(pServerObj) && !(pServerObj->m_Flags & FLAG_BOXPHYSICS))
            
            if (HasWorldModel(pServerObj)) 
			{
				return i_TestWorldModel(pServerObj->ToWorldModel());
            }
            else 
			{
                // Bounding box test...
				LTVector testPt;
				LTPlane testPlane;
                if (i_BoundingBoxTest(Point1, Point2, pServerObj, &testPt, &testPlane)) 
				{
                    // Is this intersection closer than the current best?
                    float distToIntersectionSqr = testPt.DistSqr(g_pCurQuery->m_From);
                    
                    if (g_pIntersection) 
					{
                        if (distToIntersectionSqr < g_IntersectionBestDistSqr) 
						{
                            // Do we care about model OBBs?
							if(g_bProcessModelObbs)
							{
                                // Is this object a model?
                                if(IsModel(pServerObj))
                                {
                                    ModelInstance *pModel =	pServerObj->ToModel();
                                    
                                    // Does this object want to expose it's OBBs?                                    
                                    if(pServerObj->m_Flags2 & FLAG2_USEMODELOBBS)
                                    {                                        
                                        // Does this model have OBBS? 
                                        if(pModel->IsCollisionObjectsEnabled())                                      
                                        {              
                                            // Then test the OBBs
                                            return i_TestModelOBBS(pModel);
                                        }else
                                        {
                                            // If this object has specified OBBs enabled but is 
                                            // not equipt for model obbs, then return no collision                                        
                                            return false;
                                        }
                                    }                                                                        
                                }
							}
							//else
                            return UseThisObject(pServerObj, distToIntersectionSqr, testPlane, testPt, INVALID_HPOLY, INVALID_MODEL_NODE);
                        }
                    }
                    else 
					{
                        //USE_THIS_OBJECT(pServerObj, distToIntersectionSqr, testPlane, testPt, INVALID_HPOLY);
                        //return true;
                        return UseThisObject(pServerObj, distToIntersectionSqr, testPlane, testPt, INVALID_HPOLY, INVALID_MODEL_NODE);
                    }
                }
            }
        }
    }

    return false;
}


// Finds intersections a slower way, but fills in hWorldPoly.
static void i_FindIntersectionsHPoly(const WorldBsp *pWorldBsp, const Node **pNodeIntersectionPtr, 
    LTVector *pIntersectionPosPtr, float *pDistSqrPtr, HPOLY *hWorldPoly, 
    LTVector *pPoint1, LTVector *pPoint2, uint8 bWorldModel)
{
    IntersectRequest req;

    req.m_pPoints[0] = pPoint1;
    req.m_pPoints[1] = pPoint2;
    req.m_pIPos      = pIntersectionPosPtr;
    req.m_pQuery     = g_pCurQuery;
    req.m_pWorldBsp  = pWorldBsp;
    
    if (IntersectLineNode(pWorldBsp->GetRootNode(), &req)) 
	{
        ASSERT(req.m_pNodeHit);

        *hWorldPoly = pWorldBsp->MakeHPoly(req.m_pNodeHit);
        *pNodeIntersectionPtr = req.m_pNodeHit;
        *pDistSqrPtr = g_pCurQuery->m_From.DistSqr(*req.m_pIPos);
    }
    else 
	{
        *hWorldPoly = INVALID_HPOLY;
        *pNodeIntersectionPtr = LTNULL;
    }
}


static void i_FindIntersections(const WorldBsp *pWorldBsp, const Node **pNodeIntersectionPtr, 
    LTVector *pIntersectionPosPtr, float *pDistSqrPtr, HPOLY *hWorldPoly, 
    LTVector *pPoint1, LTVector *pPoint2, uint8 bWorldModel)
{
    LTPlane iPlane;


    *hWorldPoly = INVALID_HPOLY;
    *pNodeIntersectionPtr = IntersectLine(pWorldBsp->GetRootNode(), pPoint1, pPoint2, pIntersectionPosPtr, &iPlane);
    if (*pNodeIntersectionPtr) 
	{
        *pDistSqrPtr = pPoint1->DistSqr(*pIntersectionPosPtr);
    }
}


// Called by the WorldTree in IntersectSegment.
static bool i_ISCallback(WorldTreeObj *pObj, void *pCBUser) 
{    
    ASSERT(pObj->GetObjType() == WTObj_DObject);
    LTObject *pObject = (LTObject*)pObj;
    
    if (pObject->m_Flags & (FLAG_RAYHIT|FLAG_SOLID) || g_bProcessNonSolid) 
	{
        // Honor the INTERSECT_OBJECTS flag (or lack thereof...)
        if (!g_bProcessObjects && !pObject->IsMainWorldModel()) 
		{
            return false;
        }

        // Hit the object if it has the ray hit flag, is solid, or if non-solid and it
        // query doesn't have ignore non-solid...
        // This is basically everything except for objects with only touch-notify...
        // Do further tests..
        return i_HandlePossibleIntersection(g_pCurQuery->m_From, g_pCurQuery->m_To, pObject);
    }

    return false;
}       


bool i_IntersectSegment(IntersectQuery *pQuery, IntersectInfo *pInfo, WorldTree *pWorldTree)
{
    float InvVV, VP, testMag;

    ++g_nIntersectCalls;
	CountAdder cTicks_Intersect(&g_Ticks_Intersect);
        
    // Init..
    g_pCurQuery = pQuery;
    g_pIntersection = LTNULL;
    g_pWorldIntersection = LTNULL;
    g_hWorldPoly = INVALID_HPOLY;
	g_hModelNode = INVALID_MODEL_NODE;
    g_bProcessNonSolid = !(pQuery->m_Flags & IGNORE_NONSOLID);
    g_bProcessObjects = !!(pQuery->m_Flags & INTERSECT_OBJECTS);
	g_bCheckIfFromPointIsInsideObject = !!(pQuery->m_Flags & CHECK_FROM_POINT_INSIDE_OBJECTS);
	g_bProcessModelObbs = !!(pQuery->m_Flags & INTERSECT_MODELOBBS);
    g_IntersectionBestDistSqr = (pQuery->m_From - pQuery->m_To).MagSqr() + 1.0f;

    // Precalculate stuff to totally accelerate i_QuickSphereTest.
    g_VOrigin = pQuery->m_From;
    g_V = pQuery->m_To - pQuery->m_From;

    // Calc Direction
    g_VDir = g_V.Unit();  

    g_LineLen = g_V.Mag();
    g_V /= g_LineLen;
    
    // Was it too short?
    testMag = g_V.MagSqr();
    if (testMag < 0.5f || testMag > 2.0f) 
	{
        return false;
    }
    
    VP = g_V.Dot(pQuery->m_From);
    InvVV = 1.0f / g_V.MagSqr();
    g_VTimesInvVV = g_V * InvVV;
    g_VPTimesInvVV = VP * InvVV;

    if (pQuery->m_Flags & INTERSECT_HPOLY) 
	{
        g_FindIntersectionsFn = i_FindIntersectionsHPoly;
    }
    else 
	{
        g_FindIntersectionsFn = i_FindIntersections;
    }

    // Start at the world tree.
    pWorldTree->IntersectSegment((LTVector*)&pQuery->m_From, (LTVector*)&pQuery->m_To, i_ISCallback, LTNULL);

    // If an object was hit, use it!
    if (g_pIntersection) 
	{
        pInfo->m_Point = g_IntersectionPos;
        pInfo->m_Plane = g_IntersectionPlane;
        pInfo->m_hObject = (HOBJECT)g_pIntersection;
        pInfo->m_hPoly = g_hWorldPoly;
		pInfo->m_hNode = g_hModelNode;
        
        if (g_pWorldIntersection) 
		{
            pInfo->m_SurfaceFlags = g_pWorldIntersection->m_pPoly->GetSurface()->m_TextureFlags;
        }
        else 
		{
            pInfo->m_SurfaceFlags = 0;
        }

        return true;
    }
    else 
	{
        return false;
    }
}


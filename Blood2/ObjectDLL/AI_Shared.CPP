// ----------------------------------------------------------------------- //
//
// MODULE  : AI_Shared.cpp
//
// PURPOSE : AI shared functions 
//
// CREATED : 
//
// ----------------------------------------------------------------------- //

#include <stdio.h>
#include "cpp_server_de.h"
//#include "Destructable.h"
#include "BaseCharacter.h"
#include "generic_msg_de.h"
#include "AI_Shared.h"
#include "ObjectUtilities.h"
#include "SoundTypes.h"

#include "PathPoint.h"
#include "gib.h"
#include <mbstring.h>

#define MAX_DISTANCE	100000.0f

void BPrint(char*);



// *************************************************************************
// *************************************************************************
// *************************************************************************
//
// Shared Functions
// All these functions are generic... they could be used by any game system
//
// *************************************************************************
// *************************************************************************
// *************************************************************************



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AI_Shared::AI_Shared()
//
//	PURPOSE:	Constructor - Initialize
//
// ----------------------------------------------------------------------- //
AI_Shared::AI_Shared()
{
    m_bTurnR = DFALSE;
    m_bTurnL = DFALSE;
}




// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AI_Shared::GetEyeLevel()
//
//	PURPOSE:	Compute our 'eye level'
//
// ----------------------------------------------------------------------- //
DVector AI_Shared::GetEyeLevel(HOBJECT m_hObject)
{
	DVector vPos, vDims;
	VEC_INIT(vPos);

	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE || !m_hObject) return vPos;

	pServerDE->GetObjectDims(m_hObject, &vDims);
	pServerDE->GetObjectPos(m_hObject, &vPos);

	vPos.y += vDims.y / 2.0f;
//	vPos.y -= m_fEyeLevelOffset;
	vPos.y -= 1;
	
	return vPos;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AI_Shared::DistToWorld()
//
//	PURPOSE:	How far away is the closes object
//
// ----------------------------------------------------------------------- //
DFLOAT AI_Shared::DistToWorld(HOBJECT m_hObject, DVector vDir)
{
	DVector vPos = GetEyeLevel(m_hObject);

	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE || !m_hObject) return MAX_DISTANCE;

	IntersectQuery IQuery;
	IntersectInfo IInfo;

	VEC_COPY(IQuery.m_From, vPos);
	VEC_COPY(IQuery.m_Direction, vDir);
	IQuery.m_Flags	  = INTERSECT_OBJECTS;
	IQuery.m_FilterFn = DNULL;

	if (pServerDE->CastRay(&IQuery, &IInfo))
	{
		return VEC_DIST(vPos, IInfo.m_Point);
	}

	return MAX_DISTANCE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AI_Shared::TurnToClear()
//
//	PURPOSE:	To to clearest direction for traveling
//
// ----------------------------------------------------------------------- //

DBOOL AI_Shared::TurnToClear(HOBJECT hObject)
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE || !hObject) return DFALSE;

	DRotation rRot;
	DVector m_vUp, m_vRight, m_vForward, vPos;

	pServerDE->GetObjectRotation(hObject, &rRot);
	pServerDE->GetRotationVectors(&rRot, &m_vUp, &m_vRight, &m_vForward);
	pServerDE->GetObjectPos(hObject,&vPos);

	IntersectQuery IQuery;
	IntersectInfo IInfo;

	IQuery.m_Flags	  = INTERSECT_OBJECTS;
	IQuery.m_FilterFn = DNULL;
	VEC_COPY(IQuery.m_From, vPos);

	//FORWARD
	VEC_COPY(IQuery.m_Direction, m_vForward);

	pServerDE->CastRay(&IQuery, &IInfo);
	DFLOAT fForward = VEC_DIST(vPos, IInfo.m_Point);

	//RIGHT
	VEC_COPY(IQuery.m_Direction, m_vRight);

	pServerDE->CastRay(&IQuery, &IInfo);
	DFLOAT fRight = VEC_DIST(vPos, IInfo.m_Point);

	//LEFT
	VEC_MULSCALAR(m_vRight,m_vRight,-1);
	VEC_COPY(IQuery.m_Direction, m_vRight);

	pServerDE->CastRay(&IQuery, &IInfo);
	DFLOAT fLeft = VEC_DIST(vPos, IInfo.m_Point);

	//Check which way we need to go
	if(fForward > fRight && fForward > fLeft)
		return DFALSE;
	else if(fRight > fForward && fRight > fLeft)
	{
	    pServerDE->EulerRotateY(&rRot, MATH_PI/10 * -1);
		pServerDE->SetObjectRotation(hObject, &rRot);	
	}
	else if(fLeft > fForward && fLeft > fRight)
	{
	    pServerDE->EulerRotateY(&rRot, MATH_PI/10);
		pServerDE->SetObjectRotation(hObject, &rRot);	
	}

	return DTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AI_Shared::IsLedge()
//
//	PURPOSE:	Check a ledge in front of you
//
// ----------------------------------------------------------------------- //
DBOOL AI_Shared::IsLedge(HOBJECT m_hObject, DFLOAT m_fDist)
{
	DVector vPos, vTemp, vDir, vDims;

	VEC_INIT(vPos);
	VEC_INIT(vDir);
	VEC_INIT(vTemp);
    
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE || !m_hObject) return DFALSE;

	DRotation rRot;
    DVector m_vUp, m_vRight, m_vForward;
    
    VEC_INIT(m_vUp);
	VEC_INIT(m_vRight);
	VEC_INIT(m_vForward);

	pServerDE->GetObjectRotation(m_hObject, &rRot);
	pServerDE->GetRotationVectors(&rRot, &m_vUp, &m_vRight, &m_vForward);    

//  m_fDist = 50;
    DFLOAT m_fStepHeight = 25;
    
	pServerDE->GetObjectDims(m_hObject, &vDims);
	pServerDE->GetObjectPos(m_hObject, &vPos);

	VEC_MULSCALAR(vTemp, m_vForward, m_fDist);
	VEC_ADD(vPos, vPos, vTemp);
    
    // vPos is a point in front of you.
    // vDir is a point in front of you, but down
	VEC_COPY(vDir, vPos);
	vDir.y = (vDir.y - vDims.y) - m_fStepHeight;

	IntersectQuery IQuery;
	IntersectInfo IInfo;

	VEC_COPY(IQuery.m_From, vPos);
	VEC_COPY(IQuery.m_To, vDir);
    
	IQuery.m_Flags	  = INTERSECT_OBJECTS | IGNORE_NONSOLID;
	IQuery.m_FilterFn = NULL;
	IQuery.m_pUserData = NULL;	

	if (pServerDE->IntersectSegment(&IQuery, &IInfo))
	{
    	return DFALSE;
	}
    
  	return DTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AI_Shared::DistToWorldForward()
//
//	PURPOSE:	How far away is the closes object straight ahead
//
// ----------------------------------------------------------------------- //
DFLOAT AI_Shared::DistToWorldForward(HOBJECT m_hObject)
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE || !m_hObject) return DFALSE;

	DRotation rRot;
    DVector m_vUp, m_vRight, m_vForward;
    
    VEC_INIT(m_vUp);
	VEC_INIT(m_vRight);
	VEC_INIT(m_vForward);

	pServerDE->GetObjectRotation(m_hObject, &rRot);
	pServerDE->GetRotationVectors(&rRot, &m_vUp, &m_vRight, &m_vForward);

	return DistToWorld(m_hObject, m_vForward);
}

// ----------------------------------------------------------------------- //
// ROUTINE		: AI_Shared::DistToWorldBackward
// DESCRIPTION	: 
// RETURN TYPE	: DFLOAT 
// PARAMS		: HOBJECT m_hObject
// ----------------------------------------------------------------------- //

DFLOAT AI_Shared::DistToWorldBackward(HOBJECT m_hObject)
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE || !m_hObject) return DFALSE;

	DRotation rRot;
    DVector m_vUp, m_vRight, m_vForward;
    
	VEC_INIT(m_vUp);
	VEC_INIT(m_vRight);
	VEC_INIT(m_vForward);

	pServerDE->GetObjectRotation(m_hObject, &rRot);
	pServerDE->GetRotationVectors(&rRot, &m_vUp, &m_vRight, &m_vForward);

	DVector vDir;
	VEC_COPY(vDir, m_vForward);
	VEC_MULSCALAR(vDir, vDir, -1.0f); // point left
	
	return DistToWorld(m_hObject, vDir);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AI_Shared::DistToWorldRight()
//
//	PURPOSE:	How far away is the closest object to our right
//
// ----------------------------------------------------------------------- //
DFLOAT AI_Shared::DistToWorldRight(HOBJECT m_hObject)
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE || !m_hObject) return DFALSE;

	DRotation rRot;
    DVector m_vUp, m_vRight, m_vForward;
    
	VEC_INIT(m_vUp);
	VEC_INIT(m_vRight);
	VEC_INIT(m_vForward);

	pServerDE->GetObjectRotation(m_hObject, &rRot);
	pServerDE->GetRotationVectors(&rRot, &m_vUp, &m_vRight, &m_vForward);
    
	return DistToWorld(m_hObject, m_vRight);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AI_Shared::DistToWorldLeft()
//
//	PURPOSE:	How far away is the closest object to our left
//
// ----------------------------------------------------------------------- //
DFLOAT AI_Shared::DistToWorldLeft(HOBJECT m_hObject)
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE || !m_hObject) return DFALSE;

	DRotation rRot;
    DVector m_vUp, m_vRight, m_vForward;
    
	VEC_INIT(m_vUp);
	VEC_INIT(m_vRight);
	VEC_INIT(m_vForward);

	pServerDE->GetObjectRotation(m_hObject, &rRot);
	pServerDE->GetRotationVectors(&rRot, &m_vUp, &m_vRight, &m_vForward);

	DVector vDir;
	VEC_COPY(vDir, m_vRight);
	VEC_MULSCALAR(vDir, vDir, -1.0f); // point left
	
	return DistToWorld(m_hObject, vDir);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AI_Shared::IsObjectVisible()
//
//	PURPOSE:	Is the test object visible
//
// ----------------------------------------------------------------------- //
DBOOL AI_Shared::IsObjectVisible(HOBJECT m_hObject, HOBJECT hTestObj)
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE || !m_hObject || !hTestObj) return DFALSE;

	DVector vOurPos, vTheirPos;
	DVector vPos = GetEyeLevel(m_hObject);

	pServerDE->GetObjectPos(m_hObject, &vOurPos);
	pServerDE->GetObjectPos(hTestObj, &vTheirPos);

	DVector vDir;
	VEC_SUB(vDir, vTheirPos, vOurPos);
	
	IntersectQuery IQuery;
	IntersectInfo IInfo;

	VEC_COPY(IQuery.m_From, vPos);
	VEC_COPY(IQuery.m_Direction, vDir);
	IQuery.m_Flags	  = INTERSECT_OBJECTS;
	IQuery.m_FilterFn = DNULL;

	if (pServerDE->CastRay(&IQuery, &IInfo))
	{
		// BUGBUG: If the object is partially occluded it might fail this test!
		if (IInfo.m_hObject == hTestObj)
		{
			return DTRUE;
		}
	}

	return DFALSE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AI_Shared::IsObjectVisibleToAI()
//
//	PURPOSE:	Is the test object visible to us (forward + FoV)?
//
// ----------------------------------------------------------------------- //
DBOOL AI_Shared::IsObjectVisibleToAI(HOBJECT m_hObject, HOBJECT hTestObj, DFLOAT m_fSeeingDist)
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE || !m_hObject || !hTestObj) return DFALSE;

	DVector vOurPos, vTheirPos;

    // Get the Rotation
	DRotation rRot;
    DVector m_vUp, m_vRight, m_vForward;
    
	VEC_INIT(m_vUp);
	VEC_INIT(m_vRight);
	VEC_INIT(m_vForward);

	pServerDE->GetObjectRotation(m_hObject, &rRot);
	pServerDE->GetRotationVectors(&rRot, &m_vUp, &m_vRight, &m_vForward);


	if(IsObjectVisible(m_hObject, hTestObj))
	{
		pServerDE->GetObjectPos(m_hObject, &vOurPos);
		pServerDE->GetObjectPos(hTestObj, &vTheirPos);

		DVector vDir;
		VEC_SUB(vDir, vTheirPos, vOurPos);
	
		// Now test to see if AI is facing test object...

		DFLOAT fDp;
		fDp = (vDir.x * m_vForward.x) + (vDir.y * m_vForward.y) + (vDir.z * m_vForward.z);
		
		if (fDp > 0)
		{
			// AI is facing test object so it can 'see' it
			if (VEC_MAG(vDir) < m_fSeeingDist) return DTRUE;
		}
	}
		
	return DFALSE;
}			





// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AI_Shared::ListenForEnemy()
//
//	PURPOSE:	Look for sound objects... (will be created by Player shooting, open doors)
//              sound object will remove itself after a few secs...
//
//              Message from sound...
//
// ----------------------------------------------------------------------- //
//HOBJECTL AI_Shared::ListenForEnemy(HOBJECT m_hObject, DFLOAT fRange)
//{
//// Sound Messages... SOUND_NOTIFY???
//    return DFALSE;
//}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AI_Shared::SenseForEnemy()
//
//	PURPOSE:	Look for Player objects (radius)
//
// ----------------------------------------------------------------------- //
HOBJECT AI_Shared::SenseForEnemy(HOBJECT m_hObject, DFLOAT fRange)
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE || !m_hObject ) return DFALSE;
    
	HCLASS hPlayerTest = pServerDE->GetClass("CPlayerObj");

    HOBJECT hFoundObj = FindObjectInRadius(m_hObject, hPlayerTest, fRange, DFALSE, "", DTRUE);
    
    if (hFoundObj != DNULL)
    {
        CBaseCharacter* pB = (CBaseCharacter*)pServerDE->HandleToObject(hFoundObj);
        if( pB->IsDead() == DFALSE )
        {
    	    return hFoundObj;
        }            
    }
    
    return DNULL;
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AI_Shared::SmellForEnemy()
//
//	PURPOSE:	Look for Player objects (radius)
//
// ----------------------------------------------------------------------- //
HOBJECT AI_Shared::SmellForEnemy(HOBJECT m_hObject, DFLOAT fRange)
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE || !m_hObject ) return DFALSE;

	HCLASS hPlayerTest = pServerDE->GetClass("CPlayerObj");

    HOBJECT hFoundObj = FindObjectInRadius(m_hObject, hPlayerTest, fRange, DTRUE, "", DTRUE);
    
    if (hFoundObj != DNULL)
    {
        CBaseCharacter* pB = (CBaseCharacter*)pServerDE->HandleToObject(hFoundObj);
        if( pB->IsDead() == DFALSE )
        {
    	    return hFoundObj;
        }            
    }
    
    return DNULL;
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AI_Shared::LookForEnemy()
//
//	PURPOSE:	Look for Player objects (radius)
//
// ----------------------------------------------------------------------- //
HOBJECT AI_Shared::LookForEnemy(HOBJECT m_hObject, DFLOAT fRange)
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE || !m_hObject ) return DFALSE;

    // Get the Rotation
	DRotation rRot;
    DVector m_vUp, m_vRight, m_vForward;
    
	VEC_INIT(m_vUp);
	VEC_INIT(m_vRight);
	VEC_INIT(m_vForward);

	pServerDE->GetObjectRotation(m_hObject, &rRot);
	pServerDE->GetRotationVectors(&rRot, &m_vUp, &m_vRight, &m_vForward);

	HCLASS hPlayerTest = pServerDE->GetClass("CPlayerObj");
    HOBJECT hFoundObj = FindObjectInRadius(m_hObject, hPlayerTest, fRange, DTRUE, "", DTRUE);
    
    if (hFoundObj != DNULL)
    {
    	DVector vOurPos, vTheirPos;

		pServerDE->GetObjectPos(m_hObject, &vOurPos);
		pServerDE->GetObjectPos(hFoundObj, &vTheirPos);

		DVector vDir;
		VEC_SUB(vDir, vTheirPos, vOurPos);
	
		// Now test to see if AI is facing test object...
		DFLOAT fDp;
		fDp = (vDir.x * m_vForward.x) + (vDir.y * m_vForward.y) + (vDir.z * m_vForward.z);
		
		if (fDp > 0)
		{
			// AI is facing test object so it can 'see' it
            CBaseCharacter* pB = (CBaseCharacter*)pServerDE->HandleToObject(hFoundObj);
            if( pB->IsDead() == DFALSE )
            {
    	        return hFoundObj;
            }            

            // Already checked for range in the FindNearObjectInRadius...
			//if (VEC_MAG(vDir) < fRange) return DTRUE;
		}
	}

	return DNULL;
}			






// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AI_Shared::FindNearObjectInRadius()
//
//  RETURN:     return the Ideal Object we are looking for, DNULL of nothing found
//
//	PURPOSE:	Look for nearest object within radius (visible or Not)
//
// ----------------------------------------------------------------------- //
HOBJECT AI_Shared::FindObjectInRadius(HOBJECT m_hObject, HCLASS hObjectTest, DFLOAT fRange, DBOOL bCheckVisible, char* sObjName, DBOOL bNear)
{

	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE || !m_hObject ) return DNULL;

    DFLOAT fObjDist, fSaveDist = 0;
    DFLOAT fObjTime, fSaveTime = 0;

    HOBJECT hIdealObj = DNULL;
    DBOOL bVisible = DFALSE;
    
    // Get AI's current Position
	DVector vPos;
//	pServerDE->GetObjectPos(m_hObject, &vPos);
	DRotation rRot;
	pServerDE->GetModelNodeTransform(m_hObject,szNodes[NODE_NECK-1],&vPos,&rRot);

    // Create a List of objects within the Radius
	ObjectList* pTargets = pServerDE->FindObjectsTouchingSphere(&vPos, fRange);
	if (!pTargets || pTargets->m_nInList <= 0) return DNULL;

    // Found all the Objects
	ObjectLink *pLink = pTargets->m_pFirstLink;
	while(pLink && pLink->m_hObject)
	{
		HCLASS hType = pServerDE->GetObjectClass(pLink->m_hObject);

        char sTestObjectName[128];
        
        if (_mbstrlen(sObjName) == 0)
        {
            _mbscpy((unsigned char*)sTestObjectName, (const unsigned char*)pServerDE->GetObjectName(pLink->m_hObject));
        }
        else
        {
            _mbscpy((unsigned char*)sTestObjectName, (const unsigned char*)sObjName);
        }        
        
        // Did we find one?
		if( (pServerDE->IsKindOf(hType, hObjectTest)) && 
            (_mbsncmp((const unsigned char*)pServerDE->GetObjectName(pLink->m_hObject), (const unsigned char*)sTestObjectName, _mbstrlen(sTestObjectName)) == 0 ) )
		{
            // Check for nearest Object!
        	DVector vTheirPos;
    		pServerDE->GetObjectPos(pLink->m_hObject, &vTheirPos);

            bVisible = DFALSE;
            
            if (bCheckVisible)
            {
            
                // Cast a ray for each sholder (left and right side of dims)
                
            	IntersectQuery IQuery;
            	IntersectInfo IInfo;

            	VEC_COPY(IQuery.m_From, vPos);
            	VEC_COPY(IQuery.m_To, vTheirPos);
    
            	IQuery.m_Flags	  = INTERSECT_OBJECTS | IGNORE_NONSOLID;
//            	IQuery.m_Flags	  = INTERSECT_OBJECTS;
            	IQuery.m_FilterFn = NULL;
            	IQuery.m_pUserData = NULL;	

            	if (pServerDE->IntersectSegment(&IQuery, &IInfo))
            	{
            		// BUGBUG: If the object is partially occluded it might fail this test!
            		if (IInfo.m_hObject == pLink->m_hObject)
        	    	{
                        bVisible = DTRUE;        
            		}
            	}
                else
                {
                    // Line of sight to the object is clear...
                    bVisible = DTRUE;
                }
            
                // If only looking for Visible objects and We found one NOT visible Break to next one.
                if (bVisible == DFALSE)
                {
                    break;
                }
                
            }
            
            
    		fObjDist = VEC_DIST(vPos, vTheirPos);

            // Smell Path Points
            if ( _mbsncmp((const unsigned char*)pServerDE->GetObjectName(pLink->m_hObject), (const unsigned char*)"SMELL", _mbstrlen("SMELL") ) == 0 ) 
            {
                PathPoint *pSmell = (PathPoint*)pServerDE->HandleToObject(pLink->m_hObject);
                fObjTime = pSmell->m_fStartTime;

                if (fSaveTime == 0)
                {
                    fSaveTime = fObjTime;
                    hIdealObj = pLink->m_hObject;
                }
                else
                {
                    if (fObjTime > fSaveTime)
                    {
                        fSaveTime = fObjTime;
                        hIdealObj = pLink->m_hObject;
                    }
                }
            
            }
            else
            {
            
                // NON Smell Objects
                if (fSaveDist == 0)
                {
                    fSaveDist = fObjDist;
                    hIdealObj = pLink->m_hObject;
                }
                else
                {
                    // check to see if we are the nearest or NOT
                    if (bNear)
                    {
                        if (fObjDist < fSaveDist)
                        {
                            fSaveDist = fObjDist;
                            hIdealObj = pLink->m_hObject;
                        }
                    }
                    else
                    {
                    // check for far
                        if (fObjDist > fSaveDist)
                        {
                            fSaveDist = fObjDist;
                            hIdealObj = pLink->m_hObject;
                        }
                    
                    }    
                }
                
            }        
            
            
		}

        // Next object
		pLink = pLink->m_pNext;
	}

    // clean up	
	pServerDE->RelinquishList(pTargets);

    // return the closest object found (DNULL of nothing found)
    return hIdealObj;

}




// ----------------------------------------------------------------------- //
//
//	ROUTINE:  AI_Shared::
//
//	PURPOSE:  
//
// ----------------------------------------------------------------------- //
DBOOL AI_Shared::CheckForProjectile(HOBJECT m_hObject)
{

	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE || !m_hObject ) return DNULL;

    HOBJECT hFoundObj = DNULL;
    
    // Get AI's current Position
	DVector vPos;
	pServerDE->GetObjectPos(m_hObject, &vPos);

    // Create a List of objects within the Radius
	ObjectList* pTargets = pServerDE->FindObjectsTouchingSphere(&vPos, 250);
	if (!pTargets || pTargets->m_nInList <= 0) return DNULL;

    // Found all the Objects
	ObjectLink *pLink = pTargets->m_pFirstLink;
	while(pLink && pLink->m_hObject)
	{
		HCLASS hType = pServerDE->GetObjectClass(pLink->m_hObject);
        
		if(pServerDE->IsKindOf(hType, pServerDE->GetClass("CGrenade")))
		{
            hFoundObj = pLink->m_hObject;        
            break;
        }        
//        else if(pServerDE->IsKindOf(hType, pServerDE->GetClass("CMissileProjectile")))
//		{
//            hFoundObj = pLink->m_hObject;        
//            break;
//        }        
//        else if(pServerDE->IsKindOf(hType, pServerDE->GetClass("CPulseProjectile")))
//		{
//            hFoundObj = pLink->m_hObject;        
//            break;
//        }        
//        else if(pServerDE->IsKindOf(hType, pServerDE->GetClass("CEnergyProjectile")))
//		{
//            hFoundObj = pLink->m_hObject;        
//            break;
//        }        
//        else if(pServerDE->IsKindOf(hType, pServerDE->GetClass("CFlareProjectile")))
//		{
//            hFoundObj = pLink->m_hObject;        
//            break;
//        }        
//        else if(pServerDE->IsKindOf(hType, pServerDE->GetClass("CDeadlyDiscProj")))
//		{
//            hFoundObj = pLink->m_hObject;        
//            break;
//        }        
        
        
        // Next object
		pLink = pLink->m_pNext;
	}

    // clean up	
	pServerDE->RelinquishList(pTargets);


    if (hFoundObj)
    {
        return DTRUE;
    }

    return DFALSE;

}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:  AI_Shared::
//
//	PURPOSE:  
//
// ----------------------------------------------------------------------- //
void AI_Shared::FacePosition(HOBJECT m_hObject, DVector vTargetPos)
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE || !m_hObject) return;

	DVector vDir, vPos;
    
    // Our current position	
	pServerDE->GetObjectPos(m_hObject, &vPos);

	VEC_SUB(vDir, vTargetPos, vPos);
	VEC_NORM(vDir);

	DRotation rRot;
	ROT_INIT(rRot);
    
	pServerDE->GetObjectRotation(m_hObject, &rRot);

	// m_Angles.y = ATan2(vDir.x, vDir.z);
	DFLOAT fAmount = (DFLOAT) atan2(vDir.x, vDir.z);
    
	DVector vU, vR, vF;
	pServerDE->GetRotationVectors(&rRot, &vU, &vR, &vF);

	DFLOAT fAmount2 = (DFLOAT) atan2(vF.x, vF.z);
    
	pServerDE->EulerRotateY(&rRot, fAmount2 - fAmount);
	pServerDE->SetObjectRotation(m_hObject, &rRot);
}





// ----------------------------------------------------------------------- //
//
//	ROUTINE:  AI_Shared::
//
//	PURPOSE:  
// Aim Up/down
//
// ----------------------------------------------------------------------- //
DFLOAT AI_Shared::PitchToObject(HOBJECT m_hObject, HOBJECT hPTarget)
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE || !m_hObject || !hPTarget) return 0;

	DVector vTargetPos, vPos;
    // Our current position	
	pServerDE->GetObjectPos(m_hObject, &vPos);
	pServerDE->GetObjectPos(hPTarget, &vTargetPos);
    
    // Change to the PTargets Eye level
    DVector vDims;
	pServerDE->GetObjectDims(hPTarget, &vDims);
	vTargetPos.y += vDims.y / 2.0f;
//	vTargetPos.y -= m_fEyeLevelOffset;
	vTargetPos.y -= 1;

    // Get the distance from the target
    DFLOAT ydist = vTargetPos.y - vPos.y;
    DFLOAT zdist = vTargetPos.z - vPos.z;
    
    // always want positive distance.
    if (zdist < 0)  zdist = zdist * -1;
    
    DFLOAT fPitch   = (DFLOAT) atan2(ydist,zdist);
    
//    char buf[128];
//    sprintf(buf, "vPos %f  %f %f ", vPos.x, vPos.y, vPos.z);
//    BPrint(buf);
//    sprintf(buf, "vTarget %f  %f %f ", vTargetPos.x, vTargetPos.y, vTargetPos.z);
//    BPrint(buf);
//    sprintf(buf, "Pitch %f  %f %f ", fPitch, ydist, zdist);
//    BPrint(buf);
    
    return fPitch;
}




// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AI_Shared::
//
//	PURPOSE:	
//
// ----------------------------------------------------------------------- //
void AI_Shared::Strafe(HOBJECT m_hObject, DFLOAT m_strafeSpeed)
{
    
    DFLOAT m_strafe_dist;
	DVector vAccel;
   	DVector vPos;

	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE || !m_hObject ) return;
    
	// Zero our acceleration
	VEC_INIT(vAccel);
    
	DRotation rRot;
    DVector m_vUp, m_vRight, m_vForward;
    
	VEC_INIT(m_vUp);
	VEC_INIT(m_vRight);
	VEC_INIT(m_vForward);

	pServerDE->GetObjectRotation(m_hObject, &rRot);
	pServerDE->GetRotationVectors(&rRot, &m_vUp, &m_vRight, &m_vForward);
    
    // check for jump completed
    if (m_bStrafing == DTRUE)
    {
    
// Check if we have reached the new location yet...
// Also check for ledge and walls...    

	    pServerDE->GetObjectPos(m_hObject, &vPos);
		m_strafe_dist = VEC_DIST(vPos, m_vLastStrafePos);
        
        if ( m_strafe_dist > 50.0f )
        {
    		VEC_ADD(vAccel, vAccel, m_vForward);
		    VEC_MULSCALAR(m_vRight, m_vRight, 0)
	    	VEC_ADD(vAccel, vAccel, m_vRight)

    		pServerDE->SetAcceleration(m_hObject, &vAccel);
//            NewSubState(m_newstate);
            m_bStrafing = DFALSE;
            
            // Cycle to the next metacommand
//        	m_bRecompute = DTRUE;
            
        }
        
    }
    else
    {
        // Save position 
        // check new position and distance from saved position
        // if => distance then stop strafing
	    pServerDE->GetObjectPos(m_hObject, &m_vLastStrafePos);
        
		VEC_ADD(vAccel, vAccel, m_vForward);
		VEC_MULSCALAR(m_vRight, m_vRight, m_strafeSpeed)
		VEC_ADD(vAccel, vAccel, m_vRight)

    //	pServerDE->PlaySound(m_hObject,"Sounds\\jump.wav", 400, 0,0,0 );
		pServerDE->SetAcceleration(m_hObject, &vAccel);
        m_bStrafing = DTRUE;
    }
        
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AI_Shared::
//
//	PURPOSE:	
//
// ----------------------------------------------------------------------- //
DBOOL AI_Shared::Jump(HOBJECT m_hObject, DFLOAT m_YjumpSpeed, DFLOAT m_ZjumpSpeed)
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE || !m_hObject ) return DFALSE;

	DVector vPos;
	DRotation rRot;
    DVector m_vUp, m_vRight, m_vForward;
    
	VEC_INIT(m_vUp);
	VEC_INIT(m_vRight);
	VEC_INIT(m_vForward);

	pServerDE->GetObjectRotation(m_hObject, &rRot);
	pServerDE->GetRotationVectors(&rRot, &m_vUp, &m_vRight, &m_vForward);
    
    // check for jump completed
    if (m_bJumping == DTRUE)
    {
    
	    CollisionInfo collisionInfo;
    	DBOOL bIsStandingOn;

	    pServerDE->GetStandingOn(m_hObject, &collisionInfo);
    	bIsStandingOn = (collisionInfo.m_hObject == pServerDE->GetWorldObject());
    
        if ( bIsStandingOn == DTRUE )
        {
            // Do not change until I hit the ground...
            // return to this substate
    		m_bJumping = DFALSE;
        }
        
    }
    else
    {
    
//	    DVector vTemp;
    	pServerDE->GetObjectPos(m_hObject, &vPos);

//    	VEC_MULSCALAR(vTemp, m_vForward, 1.0f);
//	    VEC_ADD(vPos, vPos, vTemp);
        
        // change the y velocity!
        vPos.y = m_vForward.y + m_YjumpSpeed;
        vPos.x = m_vForward.x;
        vPos.z = m_vForward.z;
        
    //	pServerDE->MoveObject(m_hObject, &vPos);
		PlaySoundFromObject(m_hObject, "Sounds\\jump.wav", 400, SOUNDPRIORITY_AI_MEDIUM);
	    pServerDE->SetVelocity (m_hObject, &vPos);
    
//    	DVector vGVec,vGAcc;
//	    pServerDE->GetVelocity (m_hObject, &vGVec);
//        char buf[128];
//        sprintf(buf, "GetVec %f %f %f", vGVec.x, vGVec.y, vGVec.z );
//        BPrint(buf);
        
		m_bJumping = DTRUE;
    }
    
    return m_bJumping;
        
}



// MoveForward

// MoveBackward
int AI_Shared::MoveBackward(HOBJECT m_hObject, DFLOAT m_fSpeed, int m_theAnima)
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE || !m_hObject ) return 0;

// get rotation
	DRotation rRot;
    DVector m_vUp, m_vRight, m_vForward;
    
	VEC_INIT(m_vUp);
	VEC_INIT(m_vRight);
	VEC_INIT(m_vForward);

	pServerDE->GetObjectRotation(m_hObject, &rRot);
	pServerDE->GetRotationVectors(&rRot, &m_vUp, &m_vRight, &m_vForward);

	DVector vPos, vTemp, vLastPos, vDir;
    
	VEC_COPY(vDir, m_vForward);
	VEC_MULSCALAR(vDir, vDir, -1.0f); // Backward
    
	pServerDE->GetObjectPos(m_hObject, &vPos);

    // Save the current position as the Last Position...
	VEC_COPY(vLastPos, vPos);

	VEC_MULSCALAR(vTemp, vDir, m_fSpeed);
    
	VEC_ADD(vPos, vPos, vTemp);
	pServerDE->MoveObject(m_hObject, &vPos);

    m_bTurnR = DFALSE;
    m_bTurnL = DFALSE;
    return 0;
}





// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AI_Shared::WalkThisWay
//
//	PURPOSE:	
//
// ----------------------------------------------------------------------- //
// return 0 if now walls, etc...
// return 1 if found a wall
// return 2 if found a ledge
// return 3 if stuck on something
// 
int AI_Shared::WalkThisWay(HOBJECT m_hObject, DFLOAT m_fSpeed, int m_theAnima)
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE || !m_hObject ) return 0;

// get rotation
	DRotation rRot;
    DVector m_vUp, m_vRight, m_vForward, vDims, m_vLeft;
    
	VEC_INIT(m_vUp);
	VEC_INIT(m_vRight);
	VEC_INIT(m_vForward);

	pServerDE->GetObjectRotation(m_hObject, &rRot);
	pServerDE->GetRotationVectors(&rRot, &m_vUp, &m_vRight, &m_vForward);


//    // Only set the looping animation if it has not been set Yet!
//    if (m_nCurAnimation != m_theAnima)
//    {
//    	SetAnimation(m_hObject, m_theAnima);
//    }
    
    // 1st check for walls/objects in front
	pServerDE->GetModelAnimUserDims(m_hObject, &vDims, pServerDE->GetModelAnimation(m_hObject));

//	pServerDE->GetObjectDims(m_hObject,&vDims);
	VEC_MULSCALAR(m_vLeft,m_vRight,-1);

	if(DistToWorld(m_hObject, m_vForward) - vDims.z < m_fSpeed + vDims.z ||
	   DistToWorld(m_hObject, m_vRight) - vDims.x < m_fSpeed + vDims.x ||
	   DistToWorld(m_hObject, m_vLeft) - vDims.x < m_fSpeed + vDims.x)
//	if(DistToWorldForward() < m_fSpeed) 
	{
        // Random, but if we where turning last time, them turn the same way!
//        TurnRorL(m_hObject);
		return 1;
	}

    // can only check for ledges if we have enough room to put a point in front!
    // if No walls then lets check for ledges
    if( IsLedge(m_hObject, m_fSpeed * 2.5f) )
    {
//        TurnL(m_hObject);
//        if ( IsLedge(m_hObject, m_fSpeed * 2.5f) )
//        {
//            TurnL(m_hObject);
//        }
//		return 2;
    }

//	m_bTurn = DFALSE;

//	DRotation rRot;
//	pServerDE->GetObjectRotation(m_hObject, &rRot);
//	pServerDE->GetRotationVectors(&rRot, &m_vUp, &m_vRight, &m_vForward);

	DVector vPos, vTemp, vLastPos;
	pServerDE->GetObjectPos(m_hObject, &vPos);

    // Save the current position as the Last Position...
	VEC_COPY(vLastPos, vPos);

	VEC_MULSCALAR(vTemp, m_vForward, m_fSpeed);
	VEC_ADD(vPos, vPos, vTemp);
	pServerDE->MoveObject(m_hObject, &vPos);


    // Check to see if we are stuck...
	if ( StuckOnSomething(m_hObject, m_fSpeed, vLastPos) )
	{
		// Random, but if we where turning last time, them turn the same way!
//        TurnRorL(m_hObject);
		return 3;
	}

    m_bTurnR = DFALSE;
    m_bTurnL = DFALSE;
    return 0;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AI_Shared::StuckOnSomething
//
//	PURPOSE:	Check to see if we've moved at all
//
// ----------------------------------------------------------------------- //

DBOOL AI_Shared::StuckOnSomething(HOBJECT m_hObject, DFLOAT m_fSpeed, DVector m_vLastPos)
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
    if (!pServerDE || !m_hObject ) return DFALSE;

	DVector vPos, vTemp;
    
	pServerDE->GetObjectPos(m_hObject, &vPos);
	VEC_SUB(vTemp, m_vLastPos, vPos);

	DFLOAT fDist = VEC_MAGSQR(vTemp);

	if (fDist < m_fSpeed /*|| DistToWorldForward(m_hObject) < 100*/)	//SCHLEGZ: didn't make any sense hence removed
	{
		return DTRUE;
	}

	return DFALSE;
}


void AI_Shared::Roll(HOBJECT m_hObject, DVector vDir, DFLOAT fSpeed)
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
    if (!pServerDE || !m_hObject ) return;

	DVector vPos, vTemp, vLastPos;
	pServerDE->GetObjectPos(m_hObject, &vPos);

    // Save the current position as the Last Position...
	VEC_COPY(vLastPos, vPos);

	VEC_MULSCALAR(vTemp, vDir, fSpeed);
	VEC_ADD(vPos, vPos, vTemp);
	pServerDE->MoveObject(m_hObject, &vPos);

	return;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AI_Shared::
//
//	PURPOSE:	
//
// ----------------------------------------------------------------------- //
void AI_Shared::TurnRorL(HOBJECT m_hObject, DFLOAT m_fRadius)
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE || !m_hObject ) return;
    
	// Pick a direction to turn
	// Rotate one direction some amount
    if (m_bTurnR)
	{
	    TurnR(m_hObject, m_fRadius);
	}
	else if (m_bTurnL)
	{
        TurnL(m_hObject, m_fRadius);
	}
    else if( (pServerDE->IntRandom(1, 2) == 1) )
	{
	    TurnR(m_hObject, m_fRadius);
	}
	else
	{
        TurnL(m_hObject, m_fRadius);
	}
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AI_Shared::
//
//	PURPOSE:	
//
// ----------------------------------------------------------------------- //
void AI_Shared::TurnR(HOBJECT m_hObject, DFLOAT m_fRadius)
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE || !m_hObject ) return;

//	DFLOAT m_fTurn = -0.35f;  // Right
    DFLOAT m_fTurn = m_fRadius * -1;

    DRotation rRot;
	pServerDE->GetObjectRotation(m_hObject, &rRot);
    pServerDE->EulerRotateY(&rRot, m_fTurn);
	pServerDE->SetObjectRotation(m_hObject, &rRot);	
    
    m_bTurnR = DTRUE;
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AI_Shared::
//
//	PURPOSE:	
//
// ----------------------------------------------------------------------- //
void AI_Shared::TurnL(HOBJECT m_hObject, DFLOAT m_fRadius)
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE || !m_hObject ) return;

//	DFLOAT m_fTurn = 0.35f;  // Left
    DFLOAT m_fTurn = m_fRadius;
    
    DRotation rRot;
	pServerDE->GetObjectRotation(m_hObject, &rRot);
    pServerDE->EulerRotateY(&rRot, m_fTurn);
	pServerDE->SetObjectRotation(m_hObject, &rRot);	
    
    m_bTurnL = DTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AI_Shared::FaceTarget()
//
//	PURPOSE:	Turn to face our target
//
// ----------------------------------------------------------------------- //
DBOOL AI_Shared::FaceTarget(HOBJECT m_hObject, HOBJECT m_hTarget)
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE || !m_hObject || !m_hTarget) 
    {
        return DFALSE;
    }

	DVector vDir, vPos, vTargetPos;
	
	pServerDE->GetObjectPos(m_hObject, &vPos);
	pServerDE->GetObjectPos(m_hTarget, &vTargetPos);

	VEC_SUB(vDir, vTargetPos, vPos);
	VEC_NORM(vDir);

	DRotation rRot;
	ROT_INIT(rRot);
    
	pServerDE->GetObjectRotation(m_hObject, &rRot);

//	m_Angles.y = ATan2(vDir.x, vDir.z);
	DFLOAT fAmount = (DFLOAT) atan2(vDir.x, vDir.z);
    
	DVector vU, vR, vF;
	pServerDE->GetRotationVectors(&rRot, &vU, &vR, &vF);

	DFLOAT fAmount2 = (DFLOAT) atan2(vF.x, vF.z);
    
	pServerDE->EulerRotateY(&rRot, fAmount2 - fAmount);
	pServerDE->SetObjectRotation(m_hObject, &rRot);
    
    return DTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AI_Shared::CreateGibs(HOBJECT hObject)
//
//	PURPOSE:
//
// ----------------------------------------------------------------------- //

void AI_Shared::CreateGibs(HOBJECT hObject,DDWORD dwFlags,int nCorpse, int nNode)
{
	CServerDE* g_pServerDE = BaseClass::GetServerDE();

	DVector vPos, vVel, vParentDims, vRotationPeriods;

	HCLASS hClass = g_pServerDE->GetClass( "CGib" );
	if( !hClass )
		return;

	// Get dims of this gib...
	g_pServerDE->GetObjectPos( hObject, &vPos );
	g_pServerDE->GetObjectDims( hObject, &vParentDims );


	ObjectCreateStruct theStruct;
	INIT_OBJECTCREATESTRUCT(theStruct);

	theStruct.m_UserData = dwFlags;

	if(dwFlags & GIB_CORPSE)
	{
	    DRotation rRot;
		g_pServerDE->GetObjectRotation(hObject, &rRot);
			
		VEC_COPY(theStruct.m_Pos,vPos);
		ROT_COPY(theStruct.m_Rotation,rRot);

		g_pServerDE->GetModelFilenames(hObject,theStruct.m_Filename,MAX_CS_FILENAME_LEN+1,
											   theStruct.m_SkinName,MAX_CS_FILENAME_LEN+1);

		// Allocate an object...
		BaseClass* pObj = g_pServerDE->CreateObject( hClass, &theStruct );
		if( !pObj )
			return;

		g_pServerDE->SetModelAnimation(pObj->m_hObject,nCorpse);
       	g_pServerDE->SetModelLooping(pObj->m_hObject, DFALSE);

		DVector vDims;
		g_pServerDE->GetModelAnimUserDims(pObj->m_hObject, &vDims, g_pServerDE->GetModelAnimation(pObj->m_hObject));
		
		vPos.y -= (vParentDims.y - vDims.y) - 0.001f;
		
		g_pServerDE->SetObjectDims(pObj->m_hObject,&vDims);
		g_pServerDE->SetObjectPos(pObj->m_hObject,&vPos);

		//SCHLEGZ 3/10/98 2:07:28 AM: remove missing limbs from corpse
		DBOOL bStatus = DFALSE;
		DVector vBPos;

		vPos.y -= (vDims.y - 0.001f);	//Put the blood flush with the floor

		//HEAD/NECK
		g_pServerDE->GetModelNodeHideStatus(hObject,szNodes[NODE_NECK],&bStatus);
		if(bStatus)
		{
			HideLimb(pObj->m_hObject, NODE_NECK);
			g_pServerDE->GetModelNodeTransform(hObject,szNodes[NODE_NECK],&vBPos,&rRot);
			vBPos.y = vPos.y;
			CreateScorchMark(&vBPos, 20.0f, 60.0f, 3.0f, 30.0f, 0.0f, 60.0f, 255.0f, 0.0f, 0.0f); 
		}

		//RIGHT ARM
		g_pServerDE->GetModelNodeHideStatus(hObject,szNodes[NODE_RARM],&bStatus);
		if(bStatus)
		{
			HideLimb(pObj->m_hObject, NODE_RARM);
			g_pServerDE->GetModelNodeTransform(hObject,szNodes[NODE_RARM],&vBPos,&rRot);
			vBPos.y = vPos.y;
			CreateScorchMark(&vBPos, 20.0f, 60.0f, 3.0f, 30.0f, 0.0f, 60.0f, 255.0f, 0.0f, 0.0f); 
		}

		//LEFT ARM
		g_pServerDE->GetModelNodeHideStatus(hObject,szNodes[NODE_LARM],&bStatus);
		if(bStatus)	
		{
			HideLimb(pObj->m_hObject, NODE_LARM);
			g_pServerDE->GetModelNodeTransform(hObject,szNodes[NODE_LARM],&vBPos,&rRot);
			vBPos.y = vPos.y;
			CreateScorchMark(&vBPos, 20.0f, 60.0f, 3.0f, 30.0f, 0.0f, 60.0f, 255.0f, 0.0f, 0.0f); 
		}

		//LEFT LEG
		g_pServerDE->GetModelNodeHideStatus(hObject,szNodes[NODE_LLEG],&bStatus);
		if(bStatus)	
		{
			HideLimb(pObj->m_hObject, NODE_LLEG);
			g_pServerDE->GetModelNodeTransform(hObject,szNodes[NODE_LLEG],&vBPos,&rRot);
			vBPos.y = vPos.y;
			CreateScorchMark(&vBPos, 20.0f, 60.0f, 3.0f, 30.0f, 0.0f, 60.0f, 255.0f, 0.0f, 0.0f); 
		}

		//RIGHT LEG
		g_pServerDE->GetModelNodeHideStatus(hObject,szNodes[NODE_RLEG],&bStatus);
		if(bStatus)	
		{
			HideLimb(pObj->m_hObject, NODE_RLEG);
			g_pServerDE->GetModelNodeTransform(hObject,szNodes[NODE_RLEG],&vBPos,&rRot);
			vBPos.y = vPos.y;
			CreateScorchMark(&vBPos, 20.0f, 60.0f, 3.0f, 30.0f, 0.0f, 60.0f, 255.0f, 0.0f, 0.0f); 
		}

	}
	else
	{		
		int nNumDebris = (int)g_pServerDE->Random(3,8);

		// Make the gibs...
		for( int nItem = 0; nItem < nNumDebris; nItem++ )
		{		
			VEC_SET( theStruct.m_Pos, g_pServerDE->Random( -vParentDims.x, vParentDims.x ), 
									  g_pServerDE->Random( 0.1f, vParentDims.y ), 
									  g_pServerDE->Random( -vParentDims.z, vParentDims.z ));
			VEC_ADD( theStruct.m_Pos, theStruct.m_Pos, vPos );

			// Set the velocity using a vector from the center of the parent gib to the 
			// center of where the debris starts...
			VEC_COPY( vVel, theStruct.m_Pos );
			VEC_SUB( vVel, vVel, vPos );
			// Make sure the velocity is somewhat upward...
			vVel.y += vParentDims.y;
			if( VEC_MAGSQR( vVel ) > 0.01f )
			{
				VEC_NORM( vVel );
			}
			else
			{
				VEC_SET( vVel, 0.0f, 1.0f, 0.0f );
			}
			VEC_MULSCALAR( vVel, vVel, g_pServerDE->Random( 200.0f, 500.0f ));

			// Set the rotation periods around the x, y and z axes...
			VEC_SET( vRotationPeriods, g_pServerDE->Random( -1.0f, 1.0f ), 
									   g_pServerDE->Random( -1.0f, 1.0f ), 
									   g_pServerDE->Random( -1.0f, 1.0f ));

			//Copy the rotation vector
			VEC_COPY(theStruct.m_Rotation.m_Vec,vRotationPeriods);
				
			// Allocate an object...
			BaseClass* pObj = g_pServerDE->CreateObject( hClass, &theStruct );
			if( !pObj )
				break;

			//SCHLEGZ 3/11/98 7:27:09 PM: set the dims correctly
			DVector vDims;
			g_pServerDE->GetModelAnimUserDims(pObj->m_hObject, &vDims, g_pServerDE->GetModelAnimation(pObj->m_hObject));

			g_pServerDE->SetObjectDims(pObj->m_hObject,&vDims);
			g_pServerDE->SetVelocity( pObj->m_hObject, &vVel );
		}
	}

	return;
}


// ----------------------------------------------------------------------- //
// ROUTINE		: AI_Shared::CreateLimb
// DESCRIPTION	: 
// RETURN TYPE	: void 
// PARAMS		: HOBJECT hObject
// PARAMS		: DDWORD dwFlags
// PARAMS		: int nNode
// PARAMS		: DVector vDir
// ----------------------------------------------------------------------- //

void AI_Shared::CreateLimb(HOBJECT hObject, int nNode, DVector vDir)
{
	CServerDE* g_pServerDE = BaseClass::GetServerDE();

	DVector vPos, vVel, vParentDims;
	DRotation rRot;

	HCLASS hClass = g_pServerDE->GetClass( "CGib" );
	if( !hClass )
		return;

	// Get dims of this gib...
	g_pServerDE->GetObjectDims( hObject, &vParentDims );

	g_pServerDE->GetModelNodeTransform(hObject,szNodes[nNode],&vPos,&rRot);
	g_pServerDE->GetObjectRotation( hObject, &rRot );	//temp
	
	ObjectCreateStruct theStruct;
	INIT_OBJECTCREATESTRUCT(theStruct);

	switch(nNode)
	{
		case NODE_NECK:		theStruct.m_UserData = GIB_HEAD;	break;
		case NODE_RARM:		theStruct.m_UserData = GIB_ARM;		break;
		case NODE_LARM:		theStruct.m_UserData = GIB_ARM;		break;
		case NODE_LLEG:		theStruct.m_UserData = GIB_LEG;		break;
		case NODE_RLEG:		theStruct.m_UserData = GIB_LEG;		break;
		default:	return;
	}

	g_pServerDE->GetModelFilenames(hObject,theStruct.m_Filename,MAX_CS_FILENAME_LEN+1,
										   theStruct.m_SkinName,MAX_CS_FILENAME_LEN+1);

	VEC_COPY(theStruct.m_Pos,vPos);

	// Set the velocity using a vector from the center of the parent gib to the 
	// center of where the debris starts...
	VEC_COPY(vVel,vDir);

	// Make sure the velocity is somewhat upward...
/*	if( VEC_MAGSQR( vVel ) > 0.01f )
	{
		VEC_NORM( vVel );
	}
	else
	{
		VEC_SET( vVel, 0.0f, 1.0f, 0.0f );
	}
	*/
	vVel.y = 1.0f;
	VEC_NORM(vVel);
	VEC_MULSCALAR( vVel, vVel, g_pServerDE->Random( 200.0f, 500.0f ));

	//Copy the rotation vector
	ROT_COPY(theStruct.m_Rotation, rRot);
		
	// Allocate an object...
	CGib* pObj = (CGib*)g_pServerDE->CreateObject( hClass, &theStruct );
	if( !pObj )
		return;

	//Set the right animation
	if(nNode == NODE_NECK)
	{
		g_pServerDE->SetModelAnimation(pObj->m_hObject, g_pServerDE->GetAnimIndex(pObj->m_hObject,ANIM_LIMB_HEAD));
	}
	else if(nNode == NODE_RARM || nNode == NODE_LARM)
	{
		pObj->SetKickable(DFALSE);
		g_pServerDE->SetModelAnimation(pObj->m_hObject, g_pServerDE->GetAnimIndex(pObj->m_hObject,ANIM_LIMB_ARM));
	}
	else if(nNode == NODE_LLEG || nNode == NODE_RLEG)
	{
		pObj->SetKickable(DFALSE);
		g_pServerDE->SetModelAnimation(pObj->m_hObject, g_pServerDE->GetAnimIndex(pObj->m_hObject,ANIM_LIMB_LEG));
	}
	else 
		return;

    g_pServerDE->SetModelLooping(pObj->m_hObject, DFALSE);

	//hide the rest of the body
	DBOOL bStatus = DFALSE;

	for(int i = 0; i < NUM_ALL_NODES; i++)
	{
		if(nNode == NODE_NECK && i <= NODE_NECK)	i = NODE_NECK + 1;
		else if((nNode == NODE_RARM || nNode == NODE_LARM) && i == NODE_LARM)	i = NODE_LLEG - 1;
		else if((nNode == NODE_LLEG || nNode == NODE_RLEG) && i == NODE_LLEG) i = NODE_LLEG + 4;

		HideNodeAndSubNodes(pObj->m_hObject, i, !bStatus);		
	}

	//SCHLEGZ 3/11/98 7:27:09 PM: set the dims correctly
	DVector vDims;
	g_pServerDE->GetModelAnimUserDims(pObj->m_hObject, &vDims, g_pServerDE->GetModelAnimation(pObj->m_hObject));

	g_pServerDE->SetObjectDims(pObj->m_hObject,&vDims);

	//throw the limb out there
	g_pServerDE->SetVelocity( pObj->m_hObject, &vVel );

	return;
}

// ----------------------------------------------------------------------- //
// ROUTINE		: AI_Shared::HideLimb
// DESCRIPTION	: Hide a given node
// RETURN TYPE	: int 
// PARAMS		: HOBJECT hObject
// PARAMS		: int nNode
// PARAMS		: DBOOL bInverse = DFALSE
// ----------------------------------------------------------------------- //

DBOOL AI_Shared::HideLimb(HOBJECT hObject, int nNode, DBOOL bInverse)
{
	CServerDE* pServerDE = BaseClass::GetServerDE();

	//is torso or pelvis?
	if(nNode == 2 || nNode == 3)
		return DFALSE;

	DBOOL bStatus = DFALSE;
	g_pServerDE->GetModelNodeHideStatus(hObject,szNodes[nNode],&bStatus);
	if(bStatus)
		return DFALSE;

	HideNodeAndSubNodes(hObject,nNode,DTRUE);		

	switch(nNode)
	{
		case NODE_NECK:			//NECK
		{	
			// Hide the head too
			HideNodeAndSubNodes(hObject,nNode-1,DTRUE);	
			break;
		}
		case NODE_RARM:			//RU_ARM
		{
			HideNodeAndSubNodes(hObject,nNode+1,DTRUE);	
			HideNodeAndSubNodes(hObject,nNode+2,DTRUE);
			HideNodeAndSubNodes(hObject,nNode+3,DTRUE);			
			break;
		}
		case NODE_LARM:			//LU_ARM	
		{
			HideNodeAndSubNodes(hObject,nNode+1,DTRUE);	
			HideNodeAndSubNodes(hObject,nNode+2,DTRUE);
			HideNodeAndSubNodes(hObject,nNode+3,DTRUE);						
			break;
		}
		case NODE_LLEG:			//LU_LEG
		{
			HideNodeAndSubNodes(hObject,nNode+1,DTRUE);
			HideNodeAndSubNodes(hObject,nNode+2,DTRUE);	
			HideNodeAndSubNodes(hObject,nNode+3,DTRUE);						
			break;
		}
		case NODE_RLEG:		//RU_LEG
		{
			HideNodeAndSubNodes(hObject,nNode+1,DTRUE);	
			HideNodeAndSubNodes(hObject,nNode+2,DTRUE);	
			HideNodeAndSubNodes(hObject,nNode+3,DTRUE);						
			break;
		}					
		default:	return DFALSE;
	}

	//inverse hide; hide the body and show the limb
	if(bInverse)
	{
		DBOOL bStatus = DFALSE;

		for(int i = 0; i < NUM_ALL_NODES; i++)
		{
			pServerDE->GetModelNodeHideStatus(hObject,szNodes[i],&bStatus);
			HideNodeAndSubNodes(hObject,i,!bStatus);	
		}
	}

	return DTRUE;
}

// ----------------------------------------------------------------------- //
// ROUTINE		: AI_Shared::HideNodeAndSubNodes
// DESCRIPTION	: Hides a node and any sub nodes ( with _extra# extension)
// RETURN TYPE	: BOOL 
// PARAMS		: HOBJECT hObject
// PARAMS		: int nNode
// PARAMS		: DBOOL bInverse = DFALSE
// ----------------------------------------------------------------------- //

DBOOL AI_Shared::HideNodeAndSubNodes(HOBJECT hObject, int nNode, DBOOL bHide)
{
	CServerDE* pServerDE = BaseClass::GetServerDE();

	if (!pServerDE || !hObject || nNode >= NUM_ALL_NODES) return DFALSE;

	// Hide the base node
	pServerDE->SetModelNodeHideStatus(hObject, szNodes[nNode], bHide);
	int i=1;
	char szNodeName[100];
	sprintf(szNodeName, "%s_extra%d", szNodes[nNode], i);

	// Loop until all nodes are hidden
	while (pServerDE->SetModelNodeHideStatus(hObject, szNodeName, bHide) == DE_OK)
	{
		i++;
		sprintf(szNodeName, "%s_extra%d", szNodes[nNode], i);
	}
	return DTRUE;
}
	

// ----------------------------------------------------------------------- //
// ROUTINE		: AI_Shared::HideLimb
// DESCRIPTION	: Resets all nodes to not hidden.
// RETURN TYPE	: void
// PARAMS		: HOBJECT hObject
// ----------------------------------------------------------------------- //

void AI_Shared::ResetNodes(HOBJECT hObject)
{
	CServerDE* pServerDE = BaseClass::GetServerDE();

	for(int i = 0; i < NUM_ALL_NODES; i++)
	{
		HideNodeAndSubNodes(hObject,i,DFALSE);	
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AI_Shared::SetAnimation
//
//	PURPOSE:	Set the current animation
//
// ----------------------------------------------------------------------- //
//void AI_Shared::SetAnimation(HOBJECT m_hObject, int nAni)
//{
//    CServerDE* pServerDE = BaseClass::GetServerDE();
//    if (!pServerDE || !m_hObject) return;
//
//    // only set the animation if it has not been set yet...
//    if (m_nCurAnimation != nAni)
//    {
//	    m_nCurAnimation = nAni;
//        pServerDE->SetModelAnimation(m_hObject, m_nCurAnimation);
//    }
//}


//void AI_Shared::ForceAnimation(HOBJECT m_hObject, int nAni)
//{
//    CServerDE* pServerDE = BaseClass::GetServerDE();
//    if (!pServerDE || !m_hObject) return;
//
//    // Force to different animation
//    pServerDE->SetModelAnimation(m_hObject, 0);
//
//	m_nCurAnimation = nAni;
//    pServerDE->SetModelAnimation(m_hObject, m_nCurAnimation);
//}



//
//
//
// Do we still need these functions????????????
//
//
// ****************************************************************
// ****************************************************************
// ****************************************************************

//void AI_Shared::GotoIdleAnimation()
//{
//	m_nState = STATE_INANIMATE;
//	SetAnimation(m_nAnim_IDLE);
//}


//void AI_Shared::GotoNextAnimation()
//{
//	m_nCurAnimation++;
//	if(m_nCurAnimation > m_nAnim_VICTORY1)
//		m_nCurAnimation = m_nAnim_IDLE;
//
//	SetAnimation(m_nCurAnimation);
//}


//void AI_Shared::GotoPrevAnimation()
//{
//	m_nCurAnimation--;
//	if(m_nCurAnimation < m_nAnim_IDLE)
//		m_nCurAnimation = m_nAnim_VICTORY1;
//
//	SetAnimation(m_nCurAnimation);
//}

// ****************************************************************
// ****************************************************************
// ****************************************************************




// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AI_Shared::
//
//	PURPOSE:	
//
// ----------------------------------------------------------------------- //
DBOOL AI_Shared::LoadBrains(char* pfilename) 
{

// Load in the valid behavior states
//

//    FILE *infile;
//    char line[128];
//      
//    DBOOL b_begin_state = DFALSE;
//    DBOOL b_begin_substate = DFALSE;
//    DBOOL b_begin_ppscore = DFALSE;
//      
//    int m_slot;
//    
//    AIState m_estate;
//    AISubState m_esubstate;
//    AIMetaCommand m_emetacmd;
//    
//
//    infile  = fopen(pfilename, "r");
//    
//    if (infile == NULL)
//        return DFALSE;
//
//    while (!feof(infile))
//    {
//          if (NULL != fgets(line, 128, infile))
//          {
//
//     	        BPrint(line);
//              // remove spaces and tabs from the beginning!
//              while ( _mbsncmp((const unsigned char*)line,(const unsigned char*)" ",1) == 0)
//              {
//                  _mbscpy((unsigned char*)line, (const unsigned char*)&line[1]);
//              }
//          
//              // Check for comments
//              if ( _mbsncmp((const unsigned char*)line, (const unsigned char*)";", 1) != 0 || _mbsncmp((const unsigned char*)line, "//", 2) != 0 )
//              {
//                  if ( _mbsncmp((const unsigned char*)line, (const unsigned char*)"BEGIN_STATE", 11) == 0)
//                  {
//                      b_begin_state = DTRUE;
//                      b_begin_substate = DFALSE;
//                      b_begin_ppscore = DFALSE;
//                      m_slot = 0;
//                      
//                      _mbscpy((unsigned char*)line, (const unsigned char*)&line[12]);  // only want the Name
//                      
//             	        BPrint(line);
//                      m_estate = BrainState(line);
//                  }
//                  else if ( _mbsncmp((const unsigned char*)line, (const unsigned char*)"BEGIN_SUBSTATE", 14) == 0)
//                  {
//                      b_begin_substate = DTRUE;
//                      b_begin_state = DFALSE;
//                      b_begin_ppscore = DFALSE;
//                      m_slot = 0;
//
//                      _mbscpy((unsigned char*)line, (const unsigned char*)&line[15]);  // only want the Name
//                      m_esubstate = BrainSubState(line);
//                  }
//                  else if ( _mbsncmp((const unsigned char*)line, (const unsigned char*)"BEGIN_PPSCORE", 13) == 0)
//                  {
//                      b_begin_ppscore = DTRUE;
//                      b_begin_substate = DFALSE;
//                      b_begin_state = DFALSE;
//                      m_slot = 0;
//                  }
//                  else if ( _mbsncmp((const unsigned char*)line, (const unsigned char*)"END", _mbstrlen("END")) == 0)
//                  {
//                      if (b_begin_state == DTRUE)
//                      {
//                          b_begin_state = DFALSE;
//                      }
//                      else if (b_begin_substate == DTRUE)
//                      {
//                          b_begin_substate = DFALSE;
//                      }
//                      else if (b_begin_ppscore == DTRUE)
//                      {
//                          b_begin_ppscore = DFALSE;
//                      }
//                      
//                      m_slot = 0;
//                      m_estate = STATE_INANIMATE;
//                      m_esubstate = SS_DONOTHING;
//                  }
//                  // Load the SubStates into the State Array
//                  else if (b_begin_state)
//                  {
//                      if (m_slot < 6)         // Only load up to 6 SubStates
//                      {
//                          m_esubstate = BrainSubState(line);
//                          m_Brain.m_eStateArray[m_estate][m_slot] = m_esubstate;
//                          
//                          char buf[80];
//                          sprintf(buf,"Set State(%d), with SubState(%d) in slot(%d)",(int)m_estate,(int)m_esubstate, m_slot);
//             	            BPrint(buf);
//                          
//                          m_slot++;    
//                      } 
//                      else
//                      {
//                          char buf[80];
//                          sprintf(buf,"MAX SubStates set for State(%d)",(int)m_estate);
//             	            BPrint(buf);
//                      }                           
//                  }   
//                  // Load the MetaCommands into the SubStates Array
//                  else if (b_begin_substate)
//                  {
//                      if (m_slot < 4)         // Only load up to 4 MetaCommands
//                      {
//                          m_emetacmd = BrainMetaCommand(line);
//                          m_Brain.m_eSSCmdArray[m_esubstate][m_slot] = m_emetacmd;
//                          
//                          char buf[80];
//                          sprintf(buf,"Set SubState(%d), with MetaCmd(%d) in slot(%d)",(int)m_esubstate,(int)m_emetacmd, m_slot);
//             	            BPrint(buf);
//                          
//                          m_slot++;    
//                      } 
//                      else
//                      {
//                          char buf[80];
//                          sprintf(buf,"MAX Metacmds set for SubState(%d)",(int)m_esubstate);
//             	            BPrint(buf);
//                      }                     
//                            
//                  }   
//                  // Load the PPScore
//                  else if (b_begin_ppscore)
//                  {
//                      if (m_slot < 3)         // Only load up to 3 Scores
//                      {
//                      
//                        DFLOAT m_ppvalue = (float)atof(line);
//                        m_Brain.m_PP_Score[m_slot] = m_ppvalue;
//                          
//                        char buf[80];
//                        sprintf(buf,"Set PPScore with(%f) in slot(%d)",m_ppvalue, m_slot);
//             	        BPrint(buf);
//                          
//                          m_slot++;    
//                      } 
//                      else
//                      {
//                        char buf[80];
//                        sprintf(buf,"MAX PPScores");
//                        BPrint(buf);
//                      }                     
//                            
//                  }   
//                  
//                  
//              }    // comment
//                
//                
//        }
//    }
//
//    fclose(infile);
//    
    return DTRUE;
}




// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AI_Shared::
//
//	PURPOSE:	Lookup States by NAME
//
// ----------------------------------------------------------------------- //
//AIState AI_Shared::BrainState(char* sName)
//{

//typedef struct StateData_t 
//{
//    char m_sStateName[64];
//    AIState m_eState;
//    
//} StateData;
//
//static StateData StateArray[STATE_MAX] =
//{
//        { "STATE_INANIMATE",    STATE_INANIMATE },
//        { "STATE_IDLE",         STATE_IDLE},
//        { "STATE_WORKING",      STATE_WORKING},
//        { "STATE_DEFENDING",    STATE_DEFENDING},
//        { "STATE_ATTACKING",    STATE_ATTACKING},
//        { "STATE_RETREATING",   STATE_RETREATING},
//        { "STATE_GUARDING",     STATE_GUARDING},
//        { "STATE_SCROUNGING",   STATE_SCROUNGING},
//        { "STATE_HITWALL",      STATE_HITWALL},
//        { "STATE_H_SHOTAT",     STATE_H_SHOTAT},
//        { "STATE_M_SHOTAT",     STATE_M_SHOTAT},
//        { "STATE_L_SHOTAT",     STATE_L_SHOTAT},
//        { "STATE_DYING",        STATE_DYING},
//        { "STATE_TREATASSESSMENT", STATE_TREATASSESSMENT},
//        { "STATE_GETHELP",      STATE_GETHELP}
//        
//};
 //   AIState m_return = STATE_INANIMATE;
//
//	for ( int m_index=0; m_index< STATE_MAX; m_index++)
//    {    
//        if ( _mbsncmp((const unsigned char*)sName, (const unsigned char*)StateArray[m_index].m_sStateName, _mbstrlen(StateArray[m_index].m_sStateName) ) == 0)
//        {
//            m_return = StateArray[m_index].m_eState;
//        }
//    }     
//    
//
//    return m_return;
//    return (AIState)0;
//}




// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AI_Shared::
//
//	PURPOSE:	Lookup Substates by NAME
//
// ----------------------------------------------------------------------- //
//AISubState AI_Shared::BrainSubState(char* sName)
//{

//typedef struct SubStateData_t 
//{
//    char m_sSubStateName[64];
//    AISubState m_eSubState;
//    
//} SubStateData;
//
//static SubStateData SubStateArray[SS_MAX] =
//{
//        { "SS_DONOTHING",            SS_DONOTHING},
//        { "SS_IDLE",                 SS_IDLE}, 
//        { "SS_IDLEJUMP",             SS_IDLEJUMP},
//        { "SS_WALK",                 SS_WALK},
//        { "SS_RUN",                  SS_RUN},
//        { "SS_WORKING",              SS_WORKING},
//        { "SS_DEFENDING",            SS_DEFENDING},
//        { "SS_ATTACKING",            SS_ATTACKING},
//        { "SS_SHOOTTARGET",          SS_SHOOTTARGET},
//        { "SS_ATTACKING_JUMP",       SS_ATTACKING_JUMP},
//        { "SS_ATTACKING_ROLL",       SS_ATTACKING_ROLL},
//        { "SS_SMARTCORNER",          SS_SMARTCORNER},
//        { "SS_FOLLOWPLAYER",         SS_FOLLOWPLAYER},
//        { "SS_FIREATLASTLOCATION",   SS_FIREATLASTLOCATION},
//        { "SS_WAITATLASTLOCATION",   SS_WAITATLASTLOCATION},
//        { "SS_ATTACKMORPH",          SS_ATTACKMORPH},
//        { "SS_KILLEDMORPH",          SS_KILLEDMORPH},
//        { "SS_IGNORETREAT",          SS_IGNORETREAT},
//        { "SS_SIMULANTPACK",         SS_SIMULANTPACK},
//        { "SS_RETREATING",           SS_RETREATING},
//        { "SS_FINDCOVER",            SS_FINDCOVER},
//        { "SS_GUARDING",             SS_GUARDING},
//        { "SS_SCROUNGING",           SS_SCROUNGING},
//        { "SS_HITWALL",              SS_HITWALL},
//        { "SS_SHOTAT",               SS_SHOTAT},
//        { "SS_DYING",                SS_DYING},
//        { "SS_TREATASSESSMENT",      SS_TREATASSESSMENT},
//        { "SS_FRIENDSDEAD",          SS_FRIENDSDEAD},
//        { "SS_GETHELP",              SS_GETHELP},
//        { "SS_TRIGGEROTHERS",        SS_TRIGGEROTHERS},
//        { "SS_TRIGGERSOUNDS",        SS_TRIGGERSOUNDS},
//        { "SS_TRIGGERDOORS",         SS_TRIGGERDOORS}
//
//};

  //  AISubState m_return = SS_DONOTHING;


//	for ( int m_index=0; m_index< SS_MAX; m_index++)
//    {    
//        if ( _mbsncmp((const unsigned char*)sName, (const unsigned char*)SubStateArray[m_index].m_sSubStateName, _mbstrlen(SubStateArray[m_index].m_sSubStateName) ) == 0)
//        {
//            m_return = SubStateArray[m_index].m_eSubState;
//        }
//    }     

//    return m_return;
//}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AI_Shared::
//
//	PURPOSE:	Lookup MetaCommands by NAME
//
// ----------------------------------------------------------------------- //
//AIMetaCommand AI_Shared::BrainMetaCommand(char* sName)
//{

//typedef struct MetaCommandData_t 
//{
//    char m_sMetaCmdName[64];
//    AIMetaCommand m_eMetaCmd;
//    
//} MetaCmdData;
//
//static MetaCmdData MetaCmdArray[MC_MAX] =
//{
//            // Basic Movements
//        { "MC_DONOTHING",           MC_DONOTHING},
//        { "MC_IDLE",                MC_IDLE},
//        { "MC_WALK",                MC_WALK},
//        { "MC_RUN",                 MC_RUN},
//        { "MC_TURNL",               MC_TURNL},
//        { "MC_TURNR",               MC_TURNR},
//        { "MC_TURNLR",              MC_TURNLR},
//        { "MC_ROLL",                MC_ROLL},
//        { "MC_DODGE",               MC_DODGE},
//        { "MC_STRAFE",              MC_STRAFE},
//        { "MC_JUMP",                MC_JUMP},
//// Attacking
//        { "MC_BESTWEAPON",          MC_BESTWEAPON},
//        { "MC_CHOOSETARGET",        MC_CHOOSETARGET},
//        { "MC_SHOT_TARGET",         MC_SHOT_TARGET},
//        { "MC_CHECKFORCOVER",       MC_CHECKFORCOVER},
//// Getting Help
//        { "MC_FINDTRIGGER",         MC_FINDTRIGGER},
//        { "MC_GO_TRIGGER",          MC_GO_TRIGGER},
//// PathPoints
//        { "MC_FINDNEARPATH",        MC_FINDNEARPATH},
//        { "MC_GO_PATH",             MC_GO_PATH},
//// Behavior change    
//        { "MC_STOP",                MC_STOP},
//        { "MC_EAT",                 MC_EAT},
//        { "MC_SLEEP",               MC_SLEEP},
//        { "MC_WAIT",                MC_WAIT},
//// Set Moods
//        { "MC_MOOD_FRIEND",         MC_MOOD_FRIEND},
//        { "MC_MOOD_NEUTRAL",        MC_MOOD_NEUTRAL},
//        { "MC_MOOD_FEARFUL",        MC_MOOD_FEARFUL},
//        { "MC_MOOD_HOSTILE",        MC_MOOD_HOSTILE},
//        { "MC_MOOD_PANIC",          MC_MOOD_PANIC},
//        { "MC_MOOD_PSYCHO",         MC_MOOD_PSYCHO},
//// Set States
//        { "MC_ATTACK",              MC_ATTACK},
//        { "MC_RETREAT",             MC_RETREAT},
//        { "MC_ALERT",               MC_ALERT},
//
//        { "MC_SHOOTTARGET",         MC_SHOOTTARGET},
//        { "MC_DYING",               MC_DYING}
//    
//    
//};

 //   AIMetaCommand m_return = MC_DONOTHING;

//	for ( int m_index=0; m_index< MC_MAX; m_index++)
//    {    
//        if ( _mbsncmp((const unsigned char*)sName, (const unsigned char*)MetaCmdArray[m_index].m_sMetaCmdName, _mbstrlen(MetaCmdArray[m_index].m_sMetaCmdName) ) == 0)
//        {
//            m_return = MetaCmdArray[m_index].m_eMetaCmd;
//        }
//    }     
//
//    return m_return;
//}




// Read in Attributes to set the States/SubStates!

// Fighter Strong/Weak/
// Tracker Strong/Weak
// Aiming Good/Bad
// Treat Assessment
// Senses see,smell,hear,sense
//
//
// 




// Text file:
// will have many different brains  ie: brain1.txt brain2.txt brain3.txt
/*
; Brain1.txt
;
; Brain Data:
; Comment line
;

; Select 6 possible SubStates for each State
BEGIN_STATE STATE_IDLE
    SS_IDLE
    SS_IDLE
    SS_IDLE
    SS_IDLE
    SS_IDLE
    SS_IDLE
END


; Select 4 possible MetaCommands for each SubState
BEGIN_SUBSTATE  SS_IDLE
    MC_IDLE
    MC_WALK
    MC_RUN
    MC_STOP
END

*/
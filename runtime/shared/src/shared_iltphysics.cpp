#include "bdefs.h"

#include "iltphysics.h"
#include "de_objects.h"
#include "serverobj.h"
#include "ltengineobjects.h"
#include "impl_common.h"
#include "ltsysoptim.h"
#include "clientmgr.h"

//---------------------------------------------------------------------------//

//OBSOLETE
LTRESULT ILTPhysics::GetFrictionCoefficient(HOBJECT hObj, float* u)
{
	if( hObj && u )
		*u = hObj->m_FrictionCoefficient;

	return LT_OK;
}

LTRESULT ILTPhysics::SetFrictionCoefficient(HOBJECT hObj, float u)
{
	if( hObj )
		hObj->m_FrictionCoefficient = u;

	return LT_OK;
}


LTRESULT ILTPhysics::GetVelocity(HOBJECT hObj, LTVector *pVel)
{
	LTObject *pObj;

	if(!hObj || !pVel)
	{
		RETURN_ERROR(1, CommonLT::GetVelocity, LT_INVALIDPARAMS);
	}
	
	pObj = HObjToLTObj(hObj);
	*pVel = pObj->m_Velocity;
	return LT_OK;
}


LTRESULT ILTPhysics::GetMass(HOBJECT hObj, float* m)
{
	if( hObj && m )
		*m = hObj->m_Mass;
	return LT_OK;
}


LTRESULT ILTPhysics::SetMass(HOBJECT hObj, float m)
{
	if( hObj )
		hObj->m_Mass = m;

	return LT_OK;
}


LTRESULT ILTPhysics::IsWorldObject(HOBJECT pObj)
{
	if(pObj && pObj->m_ObjectType == OT_WORLDMODEL && 
		pObj->ToWorldModel()->m_pOriginalBsp->GetWorldInfoFlags() & WIF_MAINWORLD)
	{
		return LT_YES;
	}
	else
	{
		return LT_NO;
	}
}

LTRESULT ILTPhysics::GetObjectDims(HOBJECT hObj, LTVector *pDims)
{
	LTObject *pObj;

	CHECK_PARAMS(hObj && pDims, ILTPhysics::GetObjectDims);

	pObj = (LTObject*)hObj;
	*pDims = pObj->GetDims();
	return LT_OK;
}

LTRESULT ILTPhysics::GetForceIgnoreLimit(HOBJECT hObj, float &limit)
{
	CHECK_PARAMS(hObj, ILTPhysics::GetForceIgnoreLimit);
	limit = ltsqrtf(hObj->m_ForceIgnoreLimitSqr);
	return LT_OK;
}


LTRESULT ILTPhysics::SetForceIgnoreLimit(HOBJECT hObj, float limit)
{
	CHECK_PARAMS(hObj, ILTPhysics::SetForceIgnoreLimit);
	hObj->m_ForceIgnoreLimitSqr = limit*limit;
	return LT_OK;
}


LTRESULT ILTPhysics::GetAcceleration(HOBJECT hObj, LTVector *pAccel)
{
	CHECK_PARAMS(hObj && pAccel, CommonLT::GetAcceleration);
	*pAccel = HObjToLTObj(hObj)->m_Acceleration;
	return LT_OK;
}


LTRESULT ILTPhysics::GetStandingOn(HOBJECT hObj, CollisionInfo *pInfo)
{
	LTObject *pStandingOn;
	LTObject *pObj;


	CHECK_PARAMS(hObj && pInfo, ILTPhysics::GetStandingOn);

	pObj = (LTObject*)hObj;
	
	// Stopping velocity only applies when MID_TOUCHNOTIFY is sent.
	pInfo->m_vStopVel.Init();

	// Check if we are standing on something...
	// pObj->m_pStandingOn can either be a world node or an object...
	if((pStandingOn = pObj->m_pStandingOn) != LTNULL)
	{
		// Check if it is a world node...
		if(pObj->m_pNodeStandingOn)
		{
			pInfo->m_Plane = *pObj->m_pNodeStandingOn->GetPlane();
			pInfo->m_hObject = pStandingOn;

			// Should be standing on an HPOLY..
			ASSERT(pStandingOn->HasWorldModel());
			if(pStandingOn->HasWorldModel())
			{
				pInfo->m_hPoly = pStandingOn->ToWorldModel()->MakeHPoly(pObj->m_pNodeStandingOn);
			}
			else
			{
				pInfo->m_hPoly = INVALID_HPOLY;
			}
		}
		// It's an object...
		else
		{
			pInfo->m_Plane.m_Normal.Init(0.0f, 1.0f, 0.0f);
			pInfo->m_Plane.m_Dist = pStandingOn->GetPos().y + pStandingOn->GetDims().y;
			pInfo->m_hObject = (HOBJECT)pStandingOn;
			pInfo->m_hPoly = INVALID_HPOLY;
		}
	}
	// Not standing on anything...
	else
	{
		pInfo->m_hObject = LTNULL;
	}

	return LT_OK;
}

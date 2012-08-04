//////////////////////////////////////////////////////////////////////////////
// Player-specific movement handling

#include "bdefs.h"
#include "moveobject.h"
#include "moveplayer.h"
#include "world_blocker_data.h"
#include "iltphysics.h"
#include "de_world.h"
#include "collision.h"

#include <stack>

#ifdef _WIN32
#define isnan _isnan
#endif


// Interface to the blocker data
static IWorldBlockerData *g_iWorldBlockerData = LTNULL;
define_holder(IWorldBlockerData, g_iWorldBlockerData);

// Floating-point constants
const float CPlayerMover::k_fSafetyZone = 0.1f;
const float CPlayerMover::k_fNotAStepDist = 0.5f;
const float CPlayerMover::k_fSteepAngle = 0.7071f;

CPlayerMover::CPlayerMover(const MoveState &cMoveState, uint32 nFlags) :
	m_pPlayer(cMoveState.m_pObj),
	m_pPhysics(cMoveState.m_pAbstract->GetPhysics()),
	m_pWorldTree(cMoveState.m_pWorldTree),
	m_bServer(cMoveState.m_bServer != 0),
	m_pMoveAbstract(cMoveState.m_pAbstract)
{
	m_nPlayerFlags = m_pPlayer->m_Flags;
	if (nFlags & MO_NOSLIDING)
		m_nPlayerFlags |= FLAG_NOSLIDING;
	if (nFlags & MO_GOTHRUWORLD)
		m_nPlayerFlags |= FLAG_GOTHRUWORLD;
	m_bDetachStandingOn = ((nFlags & MO_DETACHSTANDING) != 0);
	m_bTeleport = ((nFlags & MO_TELEPORT) != 0);

	ChangeDims(m_pPlayer->GetDims());
}

void CPlayerMover::ChangeDims(const LTVector &vDims)
{
	m_fPlayerRadius = LTMAX(vDims.x, vDims.z);
	m_fPlayerHeight = vDims.y;
	m_vPlayerDims = LTVector(m_fPlayerRadius, m_fPlayerHeight, m_fPlayerRadius);
	m_fPlayerSphereRadius = sqrtf((m_fPlayerRadius * m_fPlayerRadius) + (m_fPlayerHeight * m_fPlayerHeight));

	m_fPlayerStepMoveDistance = k_fNotAStepDist * m_fPlayerRadius;
	m_pPhysics->GetStairHeight(m_fPlayerStepHeight);
	m_fPlayerStepHeight += 0.5f;
}

bool CPlayerMover::ShouldCollideWithObject(const LTObject *pObj) const
{
	uint32 nObjFlags = pObj->m_Flags;

	// Stop hittin' yourself!
	if (pObj == m_pPlayer)
		return false;

	// Non-solid objects are... well.. non-solid.
	if (!IsSolid(pObj->m_Flags, m_bServer))
		return false;

	// If it's got a worldmodel, and we're supposed to go through the world, it's effectively non-solid
	if (pObj->HasWorldModel() && ((m_nPlayerFlags & FLAG_GOTHRUWORLD) != 0))
		return false;

	// If we're marked as specialnonsolid and so are they, don't intersect
	if (((pObj->m_Flags2 & FLAG2_SPECIALNONSOLID) != 0) &&
		((m_pPlayer->m_Flags2 & FLAG2_SPECIALNONSOLID) != 0))
		return false;

	return true;
}

bool CPlayerMover::ShouldTouchObject(const LTObject *pObj, bool bSolid) const
{
	// Stop touching yourself!
	if (pObj == m_pPlayer)
		return false;

	// Always touch non-solid stuff
	if (!bSolid)
		return true;

	// Do I want to touch you?
	if ((m_nPlayerFlags & FLAG_TOUCH_NOTIFY) != 0)
		return true;

	// Do you want me to touch you?
	if ((pObj->m_Flags & FLAG_TOUCH_NOTIFY) == 0)
		return false;

	// Is it hot in here, or is it just you?
	return true;
}

void CPlayerMover::CalculateLineCylinderIntersect(
		const LTVector &vBase,
		float fHeight,
		const LTVector &vStart, 
		const LTVector &vEnd, 
		SCollideResult *pResult) const
{
	// Get the edge extents
	float fEdgeMinY = vBase.y;
	float fEdgeMaxY = vBase.y + fHeight;

	// Get the ray extents
	float fRayMinY = LTMIN(vStart.y, vEnd.y);
	float fRayMaxY = LTMAX(vStart.y, vEnd.y);

	// Make sure they overlap
	if ((fRayMinY > fEdgeMaxY) || (fRayMaxY < fEdgeMinY))
		return;

	// Find the first point of collision
	CalculateLineCircleIntersect(vBase, vStart, vEnd, pResult);
	if (!pResult->m_bCollision)
		return;

	// Check for an end-cap intersection
	float fIntersectY = vStart.y + (vEnd.y - vStart.y) * pResult->m_fTime;
	if ((fIntersectY <= fEdgeMinY) || (fIntersectY >= fEdgeMaxY))
	{
		// We haven't intersected quite yet...
		pResult->m_bCollision = false;

		// Handle ray which can never hit an end cap
		if (vStart.y == vEnd.y)
			return;

		bool bRayUp = vEnd.y > vStart.y;
		float fEdgeCapY = bRayUp ? fEdgeMinY : fEdgeMaxY;
		float fIntersectTime = (fEdgeCapY - vStart.y) / (vEnd.y - vStart.y);
		// Handle a time outside the allowable range
		if ((fIntersectTime < 0.0f) || (fIntersectTime >= 1.0f))
		{
			// Try the other cap
			bRayUp = !bRayUp;
			fEdgeCapY = bRayUp ? fEdgeMinY : fEdgeMaxY;
			fIntersectTime = (fEdgeCapY - vStart.y) / (vEnd.y - vStart.y);
			// Still didn't intersect?
			if ((fIntersectTime < 0.0f) || (fIntersectTime >= 1.0f))
				return;
		}

		// Make sure it's within the radius
		LTVector vIntersectPt(vStart.x + (vEnd.x - vStart.x) * fIntersectTime, 0.0f, vStart.z + (vEnd.z - vStart.z) * fIntersectTime);
		LTVector vIntersectOfs(vIntersectPt.x - vBase.x, 0.0f, vIntersectPt.z - vBase.z);
		float fDistSqr = (vIntersectOfs.x * vIntersectOfs.x) + (vIntersectOfs.z * vIntersectOfs.z);
		if (fDistSqr >= m_fPlayerRadius * m_fPlayerRadius)
			return;
		
		// Return the end-cap result
		pResult->m_bCollision = true;
		pResult->m_fTime = fIntersectTime;
		pResult->m_cPlane = LTPlane(0.0f, (bRayUp) ? -1.0f : 1.0f, 0.0f, (bRayUp) ? -fEdgeCapY : fEdgeCapY);
	}

	return;
}

bool CPlayerMover::DoesLineIntersectCircle(
		const LTVector &vCenter, 
		const LTVector &vStart, 
		const LTVector &vEnd,
		float *pTime) const
{
	// Pre-calculate some information
	LTVector vMoveOfs(vEnd.x - vStart.x, 0.0f, vEnd.z - vStart.z);
	float fMoveOfsLenSqr = (vMoveOfs.x * vMoveOfs.x) + (vMoveOfs.z * vMoveOfs.z);
	LTVector vCenterOfs(vStart.x - vCenter.x, 0.0f, vStart.z - vCenter.z);
	float fCenterOfsLenSqr = (vCenterOfs.x * vCenterOfs.x) + (vCenterOfs.z * vCenterOfs.z);

	// Handle 0-length movement along the x/z plane
	const float k_fNoMovementEpsilon = 0.0001f;
	if (fMoveOfsLenSqr < k_fNoMovementEpsilon)
	{
		if (fCenterOfsLenSqr > m_fPlayerRadius * m_fPlayerRadius)
			return false;
		*pTime = 0.0f;
		return true;
	}

	// Check the distance
	float fA0 = fCenterOfsLenSqr - (m_fPlayerRadius * m_fPlayerRadius);
	float fA1 = vMoveOfs.x * vCenterOfs.x + vMoveOfs.z * vCenterOfs.z;
	float fA2 = fMoveOfsLenSqr;
	float fDiscr = fA1*fA1 - fA0*fA2;
	float fTime;
	if ( fDiscr > 0.0f )
	{
		float fInvA2 = 1.0f/fA2;
		fDiscr = sqrtf(fDiscr);
		float fT0 = (-fA1 - fDiscr)*fInvA2;
		float fT1 = (-fA1 + fDiscr)*fInvA2;
		// Jump out if any intersections were going to happen in the past
		// Note : This is different from CalcLineCircleIntersect, because
		// that needs to consider ray intersections in which the origin is
		// inside of the circle to be a non-intersection.
		if ((fT0 < 0.0f) && (fT1 < 0.0f))
			return false;
		fTime = LTMIN(fT0, fT1);
	}
	else if ( fDiscr < 0.0f )
	{
		// No intersection
		fTime = 2.0f;
	}
	else  // fDiscr == 0
	{
		fTime = -fA1/fA2;
		// Check for intersections in the past
		if (fTime < 0.0f)
			return false;
	}

	if (fTime < 0.0f)
		fTime = 0.0f;
	else if (fTime >= 1.0f)
		return false;

	// Yep
	*pTime = fTime;

	return true;
}

bool CPlayerMover::DoesLineIntersectCylinder(
		const LTVector &vBase,
		float fHeight,
		const LTVector &vStart, 
		const LTVector &vEnd) const
{
	// Get the edge extents
	float fEdgeMinY = vBase.y;
	float fEdgeMaxY = vBase.y + fHeight;

	// Get the ray extents
	float fRayMinY = LTMIN(vStart.y, vEnd.y);
	float fRayMaxY = LTMAX(vStart.y, vEnd.y);

	// Make sure they overlap
	if ((fRayMinY > fEdgeMaxY) || (fRayMaxY < fEdgeMinY))
		return false;

	// Find the first point of collision
	float fTime;
	if (!DoesLineIntersectCircle(vBase, vStart, vEnd, &fTime))
		return false;

	// Check for an end-cap intersection
	float fIntersectY = vStart.y + (vEnd.y - vStart.y) * fTime;
	if ((fIntersectY <= fEdgeMinY) || (fIntersectY >= fEdgeMaxY))
	{
		// Handle ray which can never hit an end cap
		if (vStart.y == vEnd.y)
			return false;

		bool bRayUp = vEnd.y > vStart.y;
		float fEdgeCapY = bRayUp ? fEdgeMinY : fEdgeMaxY;
		float fIntersectTime = (fEdgeCapY - vStart.y) / (vEnd.y - vStart.y);
		// Handle a time outside the allowable range
		if ((fIntersectTime < 0.0f) || (fIntersectTime >= 1.0f))
		{
			// Try the other cap
			bRayUp = !bRayUp;
			fEdgeCapY = bRayUp ? fEdgeMinY : fEdgeMaxY;
			fIntersectTime = (fEdgeCapY - vStart.y) / (vEnd.y - vStart.y);
			// Still didn't intersect?
			if ((fIntersectTime < 0.0f) || (fIntersectTime >= 1.0f))
				return false;
		}

		// Make sure it's within the radius
		LTVector vIntersectPt(vStart.x + (vEnd.x - vStart.x) * fIntersectTime, 0.0f, vStart.z + (vEnd.z - vStart.z) * fIntersectTime);
		LTVector vIntersectOfs(vIntersectPt.x - vBase.x, 0.0f, vIntersectPt.z - vBase.z);
		float fDistSqr = (vIntersectOfs.x * vIntersectOfs.x) + (vIntersectOfs.z * vIntersectOfs.z);
		if (fDistSqr >= m_fPlayerRadius * m_fPlayerRadius)
			return false;
	}

	return true;
}

void CPlayerMover::CalculateLineCircleIntersect(
		const LTVector &vCenter, 
		const LTVector &vStart, 
		const LTVector &vEnd, 
		SCollideResult *pResult) const
{
	// Pre-calculate some information
	LTVector vCenterOfs(vStart.x - vCenter.x, 0.0f, vStart.z - vCenter.z);
	LTVector vMoveOfs(vEnd.x - vStart.x, 0.0f, vEnd.z - vStart.z);
	float fMoveOfsLenSqr = (vMoveOfs.x * vMoveOfs.x) + (vMoveOfs.z * vMoveOfs.z);
	float fCenterOfsLenSqr = (vCenterOfs.x * vCenterOfs.x) + (vCenterOfs.z * vCenterOfs.z);

	// Handle 0-length movement along the x/z plane
	const float k_fNoMovementEpsilon = 0.0001f;
	if (fMoveOfsLenSqr < k_fNoMovementEpsilon)
	{
		if (fCenterOfsLenSqr > m_fPlayerRadius * m_fPlayerRadius)
			return;
		pResult->m_bCollision = true;
		pResult->m_fTime = 0.0f;
		pResult->m_cPlane = LTPlane(LTVector(vStart.x - vCenter.x, 0.0f, vStart.z - vCenter.z).Unit(), vStart);
		return;
	}

	// Check the distance
	float fA0 = fCenterOfsLenSqr - (m_fPlayerRadius * m_fPlayerRadius);
	float fA1 = vMoveOfs.x * vCenterOfs.x + vMoveOfs.z * vCenterOfs.z;
	float fA2 = fMoveOfsLenSqr;
	float fDiscr = fA1*fA1 - fA0*fA2;
	float fTime;
	if ( fDiscr > 0.0f )
	{
		float fInvA2 = 1.0f/fA2;
		fDiscr = sqrtf(fDiscr);
		float fT0 = (-fA1 - fDiscr)*fInvA2;
		float fT1 = (-fA1 + fDiscr)*fInvA2;
		// Jump out if any intersections were going to happen in the past
		if ((fT0 < 0.0f) && (fT1 < 0.0f))
			return;
		fTime = LTMIN(fT0, fT1);
	}
	else if ( fDiscr < 0.0f )
	{
		// No intersection
		fTime = 2.0f;
	}
	else  // fDiscr == 0
	{
		fTime = -fA1/fA2;
		// Check for intersections in the past
		if (fTime < 0.0f)
			return;
	}

	if (fTime < 0.0f)
		fTime = 0.0f;
	else if (fTime >= 1.0f)
		return;

	// Finalize the results
	pResult->m_bCollision = true;
	pResult->m_fTime = fTime;
	LTVector vCollisionPt(vStart.x + (vEnd.x - vStart.x) * fTime, 0.0f, vStart.z + (vEnd.z - vStart.z) * fTime);
	vCollisionPt.y = 0.0f;
	pResult->m_cPlane = LTPlane(LTVector(vCollisionPt.x - vCenter.x, 0.0f, vCollisionPt.z - vCenter.z).Unit(), vCollisionPt);
}

float CPlayerMover::GetPlaneProjection(const LTVector &vNormal) const
{
	float fPlaneCircle = sqrtf(vNormal.x * vNormal.x + vNormal.z * vNormal.z);
	// Truncate floating point error to avoid negative sqrts
	fPlaneCircle = LTMIN(fPlaneCircle, 1.0f);
	float fPlaneRadius = fPlaneCircle * m_fPlayerRadius;
	float fPlaneHeight = sqrtf(1 - fPlaneCircle * fPlaneCircle) * m_fPlayerHeight;
	return fPlaneRadius + fPlaneHeight;
}

float CPlayerMover::GetPlaneIntersection(const LTVector &vNormal) const
{
	float fPlaneCircle = sqrtf(vNormal.x * vNormal.x + vNormal.z * vNormal.z);
	// Truncate floating point error to avoid negative sqrts
	fPlaneCircle = LTMIN(fPlaneCircle, 1.0f);
	float fPlaneRadius = sqrtf(1 - fPlaneCircle * fPlaneCircle) * m_fPlayerRadius;
	float fPlaneHeight = fPlaneCircle * m_fPlayerHeight;
	return fPlaneRadius + fPlaneHeight;
}

PolySide CPlayerMover::GetPlaneSide(const LTPlane &cPlane, const LTVector &vPos, float *pPlaneProjection) const
{
	float fDistToPlane = cPlane.DistTo(vPos);

	// Handle the obvious cases...
	if (fDistToPlane > m_fPlayerSphereRadius)
		return FrontSide;
	else if (fDistToPlane < -m_fPlayerSphereRadius)
		return BackSide;

	// Handle the generic case
	if (*pPlaneProjection < 0.0f)
		*pPlaneProjection = GetPlaneProjection(cPlane.m_Normal);

	if (fDistToPlane > *pPlaneProjection)
		return FrontSide;
	else if (fDistToPlane < -*pPlaneProjection)
		return BackSide;
	else
		return Intersect;
}

void CPlayerMover::CWWM_Poly_CalcPlaneIntersect(
	const LTPlane &cPlane, 
	const LTVector &vStart, 
	const LTVector &vOffset, 
	float *pFrontTime, 
	float *pBackTime, 
	LTPlane *pFrontPlane) const
{
	float fPlaneExpand = GetPlaneProjection(cPlane.m_Normal);
	float fTime = cPlane.DistTo(vStart) - fPlaneExpand;
	float fDirection = cPlane.m_Normal.Dot(vOffset);
	if (fDirection == 0.0f)
		return;
	fTime = fTime / -fDirection;
	if (fDirection < 0.0f)
	{
		if (fTime > *pFrontTime)
		{
			*pFrontTime = fTime;
			*pFrontPlane = cPlane;
			pFrontPlane->m_Dist += fPlaneExpand;
		}
	}
	else
	{
		if (fTime > 0.0f)
			*pBackTime = LTMIN(*pBackTime, fTime);
	}
}

bool CPlayerMover::CWWM_Poly_CalcEdgeIntersect(
	const LTPlane &cPolyPlane,
	const LTVector &vEdgeStart, 
	const LTVector &vEdgeOffset, 
	const LTVector &vRayStart, 
	const LTVector &vRayOffset, 
	float *pIntersectTime, 
	LTPlane *pIntersectPlane) const
{
	const float k_fVerticalEdgeEpsilon = 0.001f;
	// Should we deal with it as a vertical edge?
	if ((fabsf(vEdgeOffset.x) < k_fVerticalEdgeEpsilon) && (fabsf(vEdgeOffset.z) < k_fVerticalEdgeEpsilon))
	{
		float fEdgeMinY = (vEdgeOffset.y < 0.0f) ? (vEdgeStart.y + vEdgeOffset.y) : vEdgeStart.y;
		// Collide with this as a cylinder
		SCollideResult sCollide;
		sCollide.m_bCollision = false;
		CalculateLineCylinderIntersect(
			LTVector(vEdgeStart.x, fEdgeMinY - m_fPlayerHeight, vEdgeStart.z),
			fabsf(vEdgeOffset.y) + m_fPlayerHeight * 2.0f,
			vRayStart,
			vRayStart + vRayOffset,
			&sCollide);
		if (!sCollide.m_bCollision)
			return false;

		// Return the result
		if (sCollide.m_fTime < *pIntersectTime)
		{
			*pIntersectTime = sCollide.m_fTime;
			*pIntersectPlane = sCollide.m_cPlane;
		}

		return true;
	}
	else
	{
		// Full swept cylinder intersection

		const float k_fVerticalPlaneEpsilon = 0.001f;
		LTVector vCylinderEdge;
		if (fabsf(cPolyPlane.m_Normal.y) > k_fVerticalPlaneEpsilon)
		{
			vCylinderEdge = LTVector(0.0f, (cPolyPlane.m_Normal.y > 0.0f) ? 1.0f : -1.0f, 0.0f).Cross(vEdgeOffset);
			if (vCylinderEdge.Dot(cPolyPlane.m_Normal.Cross(vEdgeOffset)) <= 0.0f)
				vCylinderEdge = -vCylinderEdge;
		}
		else
		{
			vCylinderEdge = cPolyPlane.m_Normal;
			vCylinderEdge.y = 0.0f;
		}
		vCylinderEdge.Normalize();
		LTPlane cCylinderEdgePlane(vCylinderEdge, vEdgeStart);

		float fCylinderEdgeDist = cCylinderEdgePlane.DistTo(vRayStart);
		float fCylinderEdgeRayAngle = cCylinderEdgePlane.m_Normal.Dot(vRayOffset);
		float fOutsideEdgeIntersectTime = fCylinderEdgeDist - m_fPlayerRadius;
		if (fCylinderEdgeRayAngle != 0.0f)
			fOutsideEdgeIntersectTime /= -fCylinderEdgeRayAngle;
		else
			fOutsideEdgeIntersectTime = (fCylinderEdgeDist >= m_fPlayerRadius) ? -FLT_MAX : FLT_MAX;

		// Is the outside edge a separating axis?
		/*
		if ((fCylinderEdgeDist >= m_fPlayerRadius) && ((fOutsideEdgeIntersectTime >= 1.0f) || (fOutsideEdgeIntersectTime < 0.0f)))
			return false;
			*/

		LTPlane cForwardPlane(LTVector(vEdgeOffset.x, 0.0f, vEdgeOffset.z).Unit(), vEdgeStart);

		float fForwardDist = cForwardPlane.DistTo(vRayStart);
		float fForwardRayAngle = cForwardPlane.m_Normal.Dot(vRayOffset);
		float fNegInvForwardRayAngle = -1.0f / fForwardRayAngle;
		float fEdgeStartIntersectTime = fForwardDist + m_fPlayerRadius;
		if (fForwardRayAngle != 0.0f)
			fEdgeStartIntersectTime *= fNegInvForwardRayAngle;
		else
			fEdgeStartIntersectTime = (fForwardDist <= -m_fPlayerRadius) ? -FLT_MAX : FLT_MAX;

		// Is the forward edge a separating axis at the edge start?
		/*
		if ((fForwardDist <= -m_fPlayerRadius) && ((fEdgeStartIntersectTime >= 1.0f) || (fEdgeStartIntersectTime < 0.0f)))
			return false;
			*/

		float fForwardSpan = vEdgeOffset.Dot(cForwardPlane.m_Normal);
		float fEdgeEndIntersectTime = fForwardDist - (fForwardSpan + m_fPlayerRadius);
		if (fForwardRayAngle != 0.0f)
			fEdgeEndIntersectTime *= fNegInvForwardRayAngle;
		else
			fEdgeEndIntersectTime = (fForwardDist >= (fForwardSpan + m_fPlayerRadius)) ? -FLT_MAX : FLT_MAX;

		// Is the forward edge a separating axis at the edge end?
		/*
		if ((fForwardDist >= (fForwardSpan + m_fPlayerRadius)) && ((fEdgeEndIntersectTime >= 1.0f) || (fEdgeEndIntersectTime < 0.0f)))
			return false;
			*/

		LTPlane cSweepPlane(vCylinderEdge.Cross(vEdgeOffset).Unit(), vEdgeStart);
		if (cSweepPlane.m_Normal.y < 0.0f)
			cSweepPlane = -cSweepPlane;

		float fCapSize = fabsf(cForwardPlane.m_Normal.Dot(cSweepPlane.m_Normal)) * m_fPlayerRadius;
		float fSweepPlayerHeight = m_fPlayerHeight * cSweepPlane.m_Normal.y;
		float fSweepCylinderHeight = fSweepPlayerHeight + fCapSize;

		float fSweepDist = cSweepPlane.DistTo(vRayStart);
		float fSweepRayAngle = cSweepPlane.m_Normal.Dot(vRayOffset);
		float fNegInvSweepRayAngle = -1.0f / fSweepRayAngle;
		float fSweepTopIntersectTime = fSweepDist - fSweepCylinderHeight;
		if (fSweepRayAngle != 0.0f)
			fSweepTopIntersectTime *= fNegInvSweepRayAngle;
		else
			fSweepTopIntersectTime = (fSweepDist >= fSweepCylinderHeight) ? -FLT_MAX : FLT_MAX;

		// Is the top edge a separating axis?
		/*
		if ((fSweepDist >= fSweepCylinderHeight) && ((fSweepTopIntersectTime >= 1.0f) || (fSweepTopIntersectTime < 0.0f)))
			return false;
			*/

		float fSweepBottomIntersectTime = fSweepDist + fSweepCylinderHeight;
		if (fSweepRayAngle != 0.0f)
			fSweepBottomIntersectTime *= fNegInvSweepRayAngle;
		else
			fSweepBottomIntersectTime = (fForwardDist <= -fSweepCylinderHeight) ? -FLT_MAX : FLT_MAX;

		// Is the bottom edge a separating axis?
		/*
		if ((fSweepDist <= -fSweepCylinderHeight) && ((fSweepBottomIntersectTime >= 1.0f) || (fSweepBottomIntersectTime < 0.0f)))
			return false;
			*/

		float fPolyDist = cPolyPlane.DistTo(vRayStart);
		float fPolyRayAngle = cPolyPlane.m_Normal.Dot(vRayOffset);
		float fNegInvPolyRayAngle = -1.0f / fPolyRayAngle;
		float fPolyPlaneThickness = GetPlaneProjection(cPolyPlane.m_Normal);
		float fPolyFrontIntersectTime = fPolyDist - fPolyPlaneThickness;
		if (fPolyRayAngle != 0.0f)
			fPolyFrontIntersectTime *= fNegInvPolyRayAngle;
		else
			fPolyFrontIntersectTime = (fPolyDist >= fPolyPlaneThickness) ? -FLT_MAX : FLT_MAX;

		float fPolyBackIntersectTime = fPolyDist + fPolyPlaneThickness;
		if (fPolyRayAngle != 0.0f)
			fPolyBackIntersectTime *= fNegInvPolyRayAngle;
		else
			fPolyBackIntersectTime = (fPolyDist <= -fPolyPlaneThickness) ? -FLT_MAX : FLT_MAX;

		// Do a CSG operation to determine if the ray intersects the planar bounds of the shape

		float fMaxTForward = -FLT_MAX;
		float fMinTBackward = FLT_MAX;
		LTPlane cMaxTPlane;

		// Plane CSG operation function
		// (Got tired of creating utility functions of utility functions of utility functions at the class scope....)
		struct SPlaneCSGOp
		{
			SPlaneCSGOp(float fIntersectTime, bool bFront, float *pMaxTFront, float *pMinTBack)
			{
				if (bFront)
					*pMaxTFront = LTMAX(*pMaxTFront, fIntersectTime);
				else
					*pMinTBack = LTMIN(*pMinTBack, fIntersectTime);
			}
		};

		SPlaneCSGOp(fOutsideEdgeIntersectTime, fCylinderEdgeRayAngle < 0.0f, &fMaxTForward, &fMinTBackward);
		SPlaneCSGOp(fEdgeStartIntersectTime, fForwardRayAngle > 0.0f, &fMaxTForward, &fMinTBackward);
		SPlaneCSGOp(fEdgeEndIntersectTime, fForwardRayAngle < 0.0f, &fMaxTForward, &fMinTBackward);
		bool bUpperEdge = cPolyPlane.m_Normal.Cross(vEdgeOffset).y > -k_fVerticalPlaneEpsilon;
		if (bUpperEdge)
			SPlaneCSGOp(fSweepTopIntersectTime, fSweepRayAngle < 0.0f, &fMaxTForward, &fMinTBackward);
		else
			SPlaneCSGOp(fSweepBottomIntersectTime, fSweepRayAngle > 0.0f, &fMaxTForward, &fMinTBackward);
		SPlaneCSGOp(fPolyFrontIntersectTime, fPolyRayAngle < 0.0f, &fMaxTForward, &fMinTBackward);
		SPlaneCSGOp(fPolyBackIntersectTime, fPolyRayAngle > 0.0f, &fMaxTForward, &fMinTBackward);

		if ((fMinTBackward <= fMaxTForward) || (fMaxTForward >= 1.0f))
			return false;

		// Adjust the intersection times of the rounded portions so we can determine where our intersection is (or isn't)
		if (fForwardRayAngle != 0.0f)
		{
			float fForwardIntersectOfs = m_fPlayerRadius * fNegInvForwardRayAngle;
			fEdgeStartIntersectTime -= fForwardIntersectOfs;
			fEdgeEndIntersectTime += fForwardIntersectOfs;
		}
		else if (fEdgeEndIntersectTime == FLT_MAX)
		{
			if ((fForwardDist > 0.0f) && (fForwardDist < fForwardSpan))
				fEdgeEndIntersectTime = -FLT_MAX;
			fEdgeStartIntersectTime = fEdgeEndIntersectTime * -1.0f;
		}
		if (fSweepRayAngle != 0.0f)
		{
			float fSweepIntersectOfs = fCapSize * fNegInvSweepRayAngle;
			fSweepTopIntersectTime += fSweepIntersectOfs;
			fSweepBottomIntersectTime -= fSweepIntersectOfs;
		}
		else if (fSweepBottomIntersectTime == FLT_MAX)
		{
			if ((fSweepDist < fSweepPlayerHeight) && (fSweepDist > -fSweepPlayerHeight))
				fSweepBottomIntersectTime = -FLT_MAX;
			fSweepTopIntersectTime = fSweepBottomIntersectTime * -1.0f;
		}

		float fForwardFrontIntersectTime = (fForwardRayAngle > 0.0f) ? fEdgeStartIntersectTime : fEdgeEndIntersectTime;
		float fSweepFrontIntersectTime = (fSweepRayAngle < 0.0f) ? fSweepTopIntersectTime : fSweepBottomIntersectTime;

		// Handle the outside edge being what we hit
		if ((fOutsideEdgeIntersectTime >= fForwardFrontIntersectTime) && 
			(fOutsideEdgeIntersectTime >= fSweepFrontIntersectTime) &&
			(fCylinderEdgeRayAngle < 0.0f) &&
			(fOutsideEdgeIntersectTime >= 0.0f))
		{
			// Double check..
			LTVector vOutsideIntersect = vRayStart + vRayOffset * fOutsideEdgeIntersectTime;
			float fSweepOutsideIntersectDist = cSweepPlane.DistTo(vOutsideIntersect);
			float fForwardOutsideIntersectDist = cForwardPlane.DistTo(vOutsideIntersect);
			if ((fSweepOutsideIntersectDist <= fSweepPlayerHeight) &&
				(fSweepOutsideIntersectDist >= -fSweepPlayerHeight) &&
				(fForwardOutsideIntersectDist <= fForwardSpan) &&
				(fForwardOutsideIntersectDist >= 0.0f))
			{
				if (fOutsideEdgeIntersectTime < *pIntersectTime)
				{
					*pIntersectTime = fOutsideEdgeIntersectTime;
					*pIntersectPlane = LTPlane(cCylinderEdgePlane.m_Normal, cCylinderEdgePlane.m_Dist + m_fPlayerRadius);
				}
				return true;
			}
		}

		SCollideResult sCollide;
		bool bFinalResult = false;

		// Intersect against the edge start/end cylinder
		bool bForwardStart;
		if (fForwardRayAngle == 0.0f)
			bForwardStart = (fForwardDist < 0.0f);
		else
			bForwardStart = (fForwardRayAngle > 0.0f);

		const float k_fEndPtCapOverlap = 0.001f;

		sCollide.m_bCollision = false;
		LTVector vCylinderOrigin = ((bForwardStart) ? (vEdgeStart) : (vEdgeStart + vEdgeOffset));
		CalculateLineCylinderIntersect(
			LTVector(vCylinderOrigin.x, vCylinderOrigin.y - (m_fPlayerHeight + k_fEndPtCapOverlap), vCylinderOrigin.z),
			(m_fPlayerHeight + k_fEndPtCapOverlap) * 2.0f,
			vRayStart,
			vRayStart + vRayOffset,
			&sCollide);
		if (sCollide.m_bCollision)
		{
			if (sCollide.m_fTime < *pIntersectTime)
			{
				*pIntersectTime = sCollide.m_fTime;
				*pIntersectPlane = sCollide.m_cPlane;
			}
			bFinalResult = true;
		}

		// Intersect against the swept cap
		// Note : The cap intersection selection may very well be quite incorrect in some cases...
		// It really should check both in that case instead of just assuming it's fine.
		bool bCapTop = (fSweepDist > 0.0f);

		float fCapStartY = vEdgeStart.y + ((bCapTop) ? m_fPlayerHeight : -m_fPlayerHeight);
		// Handle a flat cap
		if (vEdgeOffset.y == 0.0f)
		{
			if (vRayOffset.y != 0.0f)
			{
				// Get the intersection on the Y plane
				float fCapIntersectTime = vRayStart.y - fCapStartY;
				if (fCapIntersectTime != 0.0f)
					fCapIntersectTime /= -vRayOffset.y;
				fCapIntersectTime = LTMAX(fCapIntersectTime, 0.0f);
				LTVector vCapIntersect = vRayStart + vRayOffset * fCapIntersectTime;
				// Did we intersect inside of the range?
				float fCapIntersectForwardDist = cForwardPlane.DistTo(vCapIntersect);
				if ((fabsf(cCylinderEdgePlane.DistTo(vCapIntersect)) < m_fPlayerRadius) &&
					(fCapIntersectForwardDist > 0.0f) &&
					(fCapIntersectForwardDist < fForwardSpan))
				{
					if (fCapIntersectTime < *pIntersectTime)
					{
						*pIntersectTime = fCapIntersectTime;
						*pIntersectPlane = LTPlane(0.0f, (bCapTop) ? 1.0f : -1.0f, 0.0f, (bCapTop) ? fCapStartY : -fCapStartY);
					}
					bFinalResult = true;
				}
			}
		}
		else
		{
			// Skew the ray by the edge offset in Y
			float fCapSkewX = vEdgeOffset.x / vEdgeOffset.y;
			float fCapSkewZ = vEdgeOffset.z / vEdgeOffset.y;
			float fRayStartOfsY = vRayStart.y - fCapStartY;
			LTVector vSkewRayStart(vRayStart.x - fCapSkewX * fRayStartOfsY, vRayStart.y, vRayStart.z - fCapSkewZ * fRayStartOfsY);
			LTVector vSkewRayOfs(vRayOffset.x - fCapSkewX * vRayOffset.y, vRayOffset.y, vRayOffset.z - fCapSkewZ * vRayOffset.y);
			// Collide the skewed ray by the non-skewed edge
			sCollide.m_bCollision = false;
			CalculateLineCylinderIntersect(
				LTVector(vEdgeStart.x, fCapStartY + ((vEdgeOffset.y < 0.0f) ? vEdgeOffset.y : 0.0f), vEdgeStart.z),
				fabsf(vEdgeOffset.y),
				vSkewRayStart,
				vSkewRayStart + vSkewRayOfs,
				&sCollide);
			if (sCollide.m_bCollision)
			{
				if (sCollide.m_fTime < *pIntersectTime)
				{
					*pIntersectTime = sCollide.m_fTime;
					// Re-generate the plane, since the cylinder was skewed when we did the intersection
					LTVector vIntersectPt = vRayStart + vRayOffset * sCollide.m_fTime;
					LTVector vEdgeIntersectOfs = vIntersectPt - vEdgeStart;
					float fPtOnEdge = vEdgeOffset.Dot(vEdgeIntersectOfs) / vEdgeOffset.MagSqr();
					LTVector vPtOnEdge = vEdgeStart + vEdgeOffset * fPtOnEdge;
					LTVector vRawOffset = vIntersectPt - vPtOnEdge;
					// Clean up the normal (i.e. make it perpendicular to the edge)
					LTVector vFixedOffset = vEdgeOffset.Cross(vRawOffset.Cross(vEdgeOffset));
					*pIntersectPlane = LTPlane(vFixedOffset.Unit(), vPtOnEdge);
				}
				bFinalResult = true;
			}
		}

		// Not so fast!  The above stuff doesn't deal with a pre-existing intersection at all.
		// In theory that shouldn't happen, but it does.  (probably a float inaccuracy thing)
		// So here we check for an intersection with the edge at time 0, and then handle it 
		// as a normal intersection so we don't go making things worse.
		if (!bFinalResult)
		{
			sCollide.m_bCollision = false;
			CalculateLineCylinderIntersect(
				LTVector(vRayStart.x, vRayStart.y - m_fPlayerHeight, vRayStart.z),
				m_fPlayerHeight * 2.0f,
				vEdgeStart,
				vEdgeStart + vEdgeOffset,
				&sCollide);
			if (sCollide.m_bCollision)
			{
				if (sCollide.m_fTime < *pIntersectTime)
				{
					*pIntersectTime = sCollide.m_fTime;
					// Re-generate the plane, since the cylinder was skewed when we did the intersection
					LTVector vEdgeIntersectOfs = vRayStart - vEdgeStart;
					float fPtOnEdge = vEdgeOffset.Dot(vEdgeIntersectOfs) / vEdgeOffset.MagSqr();
					fPtOnEdge = LTCLAMP(fPtOnEdge, 0.0f, 1.0f);
					LTVector vPtOnEdge = vEdgeStart + vEdgeOffset * fPtOnEdge;
					LTVector vRawOffset = vRayStart - vPtOnEdge;
					// Clean up the normal (i.e. make it perpendicular to the edge)
					LTVector vFixedOffset = vEdgeOffset.Cross(vRawOffset.Cross(vEdgeOffset));
					*pIntersectPlane = LTPlane(vFixedOffset.Unit(), vPtOnEdge);
				}
				bFinalResult = true;
			}
		}

		// Tell the caller whether or not we hit the outside of the parallelogram
		return bFinalResult;
	}
}

void CPlayerMover::CollideWithWorldModel_Poly(const WorldPoly *pPoly, const LTVector &vStart, const LTVector &vOffset, float fPlaneProjection, SCollideResult *pResult) const
{
	const LTPlane &cPolyPlane = *pPoly->GetPlane();

	// Early-out if we're outside the polygon's sphere
	float fOffsetMagSqr = vOffset.MagSqr();
	LTVector vStartPolyCenterOffset = pPoly->GetCenter() - vStart;
	float fCenterOffsetMagSqr = vStartPolyCenterOffset.MagSqr();
	ASSERT(fOffsetMagSqr != 0.0f);
	if (fCenterOffsetMagSqr != 0.0f)
	{
		float fClosestOnOffsetTime = vOffset.Dot(vStartPolyCenterOffset) / (sqrtf(fCenterOffsetMagSqr) * fOffsetMagSqr);
		LTVector vClosestOnOffset = vStart + vOffset * fClosestOnOffsetTime;
		float fCombinedRadius = m_fPlayerSphereRadius + pPoly->GetRadius();
		if ((vClosestOnOffset - pPoly->GetCenter()).MagSqr() > (fCombinedRadius * fCombinedRadius))
			return;
	}

	// Find out how the starting position relates to the extension of the plane by the projection
	float fPlaneDist = cPolyPlane.DistTo(vStart);

	// Don't worry if it's on the wrong side of the plane
	if (fPlaneDist < 0.0f)
		return;

	if (fPlaneProjection < 0.0f)
		fPlaneProjection = GetPlaneProjection(cPolyPlane.m_Normal);

	float fPlaneIntersectTime = fPlaneDist - fPlaneProjection;
	float fRayPlaneAngle = cPolyPlane.m_Normal.Dot(vOffset);
	if (fRayPlaneAngle != 0.0f)
		fPlaneIntersectTime /= -fRayPlaneAngle;
	else
		fPlaneIntersectTime = (fPlaneDist > -fPlaneProjection) ? -FLT_MAX : FLT_MAX;
	
	// Jump out if the plane intersect isn't going to happen until too late..
	if (fPlaneIntersectTime >= pResult->m_fTime)
		return;

	float fBackPlaneIntersectTime = fPlaneDist + fPlaneProjection;
	if (fRayPlaneAngle != 0.0f)
		fBackPlaneIntersectTime /= -fRayPlaneAngle;
	else
		fBackPlaneIntersectTime = (fPlaneDist > -fPlaneProjection) ? -FLT_MAX : FLT_MAX;
	// Jump out if we're already on the wrong side (This really shouldn't happen...)
	if (fBackPlaneIntersectTime < 0.0f)
		return;

	// Don't worry about front plane intersections in the past
	fPlaneIntersectTime = LTMAX(0.0f, fPlaneIntersectTime);

	// Find the point of intersection
	LTVector vPlaneIntersect = vStart + vOffset * fPlaneIntersectTime;

	float fEarliestIntersect = fBackPlaneIntersectTime;
	LTPlane cIntersectPlane = cPolyPlane;

	const float k_fHorizPolyMinY = 0.9999f;
	const float k_fVertPolyMaxY = 0.0001f;
	bool bHorizPoly = fabsf(cPolyPlane.m_Normal.y) >= k_fHorizPolyMinY;
	bool bVertPoly = fabsf(cPolyPlane.m_Normal.y) < k_fVertPolyMaxY;

	const float k_fVerticalEdgeEpsilon = 0.001f;

	LTVector vPlaneOffset;
	if (!bHorizPoly && !bVertPoly)
	{
		vPlaneOffset = cPolyPlane.m_Normal;
		vPlaneOffset.y = 0.0f;
		vPlaneOffset *= m_fPlayerRadius / vPlaneOffset.Mag();
		if (cPolyPlane.m_Normal.y > 0.0f)
			vPlaneOffset.y = m_fPlayerHeight;
		else
			vPlaneOffset.y = -m_fPlayerHeight;
	}

	bool bEdgeIntersectRequired = false;

	// See if it's inside the poly
	LTVector vPrevPt = pPoly->GetVertex(pPoly->GetNumVertices() - 1);
	for (uint32 nCurIndex = 0; nCurIndex < pPoly->GetNumVertices(); ++nCurIndex)
	{
		LTVector vCurPt = pPoly->GetVertex(nCurIndex);
		LTVector vEdge = vCurPt - vPrevPt;
		LTVector vEdgeNormal;
		if (!bHorizPoly && !bVertPoly && 
			((fabsf(vEdge.x) > k_fVerticalEdgeEpsilon) || (fabsf(vEdge.z) > k_fVerticalEdgeEpsilon)))
			vEdgeNormal = vPlaneOffset.Cross(vEdge).Unit();
		else
			vEdgeNormal = cPolyPlane.m_Normal.Cross(vEdge).Unit();
		LTPlane cEdgePlane(vEdgeNormal, vCurPt);
		float fEdgeDist = cEdgePlane.DistTo(vPlaneIntersect);
		if (fEdgeDist > 0.0f)
		{
			// Skip out if we're obviously not going to intersect
			if (fEdgeDist > GetPlaneProjection(vEdgeNormal))
				return;
			float fPrevEarlyIntersect = fEarliestIntersect;
			// Check for an intersection with the edge
			bool bEdgeIntersect = CWWM_Poly_CalcEdgeIntersect(cPolyPlane, vPrevPt, vEdge, vStart, vOffset, &fEarliestIntersect, &cIntersectPlane);
#if 0
			// Double-check the result
			LTVector vEndPos = vStart + vOffset * (fEarliestIntersect + ((bEdgeIntersect) ? 0.001f : 0.0f));
			bool bDoubleCheck = DoesLineIntersectCylinder(
				LTVector(vEndPos.x, vEndPos.y - m_fPlayerHeight, vEndPos.z),
				m_fPlayerHeight * 2.0f,
				vPrevPt,
				vCurPt);
			ASSERT(bDoubleCheck == bEdgeIntersect);
#endif
			bEdgeIntersectRequired |= !bEdgeIntersect;
		}

		vPrevPt = vCurPt;
	}

	// If we didn't hit anything before the back plane, we hit the front plane
	if (fEarliestIntersect == fBackPlaneIntersectTime)
	{
		// If we were borderline in the edge intersection department, we didn't actually hit anything
		if (bEdgeIntersectRequired)
			return;
		fEarliestIntersect = fPlaneIntersectTime;
	}

	if (fEarliestIntersect >= pResult->m_fTime)
		return;

	// Forget about the collision if the collision normal was facing the wrong way
	if (cIntersectPlane.m_Normal.Dot(vOffset) >= 0.0f)
		return;

	// Return a result
	pResult->m_bCollision = true;
	pResult->m_cPlane = cIntersectPlane;
	pResult->m_fTime = LTMAX(fEarliestIntersect, 0.0f);
}

void CPlayerMover::CollideWithWorldModel(LTObject *pObject, const LTVector &vStart, const LTVector &vEnd, SCollideResult *pResult) const
{
	typedef std::stack<Node*> TWalkStack;
	static TWalkStack cWalkStack;

	WorldModelInstance *pWorldModel = pObject->ToWorldModel();
	if (!pWorldModel)
	{
		ASSERT(!"Invalid worldmodel object encountered");
		return;
	}

	const WorldBsp *pBSP = pWorldModel->GetValidBsp();
	if (!pBSP)
	{
		ASSERT(!"Invalid worldmodel object encountered");
		return;
	}

	// Pre-calculate some helpful acceleration information
	LTVector vCurMidPt = (vStart + vEnd) * 0.5f;
	LTVector vOffset = vEnd - vStart;
	float fMoveRadius = vOffset.Mag() + m_fPlayerSphereRadius;

	const Node *pRoot = pBSP->GetRootNode();

	const float k_fBacksideLimit = -0.0001f * vOffset.Mag();

	for (;;)
	{
		// If we're at a leaf node, pop the stack
		if ((pRoot==NODE_IN) || (pRoot==NODE_OUT))
		{
			if (cWalkStack.empty())
				break;

			pRoot = cWalkStack.top();
			cWalkStack.pop();
		}

		// Do the fast test on the whole movement area.
		float fMoveDot = pRoot->GetPlane()->DistTo(vCurMidPt);
		if (fMoveDot > fMoveRadius)
		{
			pRoot = pRoot->m_Sides[FrontSide];
			continue;
		}
		else if (fMoveDot < -fMoveRadius)
		{
			pRoot = pRoot->m_Sides[BackSide];
			continue;
		}
		
		// Decide which side of the plane we're on
		float fPlaneProjection = -1.0f;
		PolySide nStartSide = GetPlaneSide(*pRoot->GetPlane(), vStart, &fPlaneProjection);
		PolySide nEndSide = GetPlaneSide(*pRoot->GetPlane(), vEnd, &fPlaneProjection);

		// Check for a collision
		if (((nStartSide == nEndSide) && (nStartSide == Intersect)) || 
			((nStartSide != nEndSide) && (nStartSide != BackSide)))
		{
			if (((pRoot->m_pPoly->GetSurface()->GetFlags() & SURF_SOLID) != 0) &&
				(pRoot->GetPlane()->m_Normal.Dot(vOffset) < k_fBacksideLimit))
			{
				bool bOldCollision = pResult->m_bCollision;
				pResult->m_bCollision = false;
				CollideWithWorldModel_Poly(pRoot->m_pPoly, vStart, vOffset, fPlaneProjection, pResult);
				if (pResult->m_bCollision)
					pResult->m_pObjectNode = pRoot;
				else
					pResult->m_bCollision = bOldCollision;
			}
		}

		// Push the other side onto the stack if necessary
		if ((nStartSide != nEndSide) || ((nStartSide == nEndSide) && (nStartSide == Intersect)))
		{
			nStartSide = (pRoot->GetPlane()->DistTo(vStart) > -0.001f) ? FrontSide : BackSide;
			nEndSide = nStartSide ? BackSide : FrontSide;
			if ((pRoot->m_Sides[nEndSide] != NODE_IN) && (pRoot->m_Sides[nEndSide] != NODE_OUT))
				cWalkStack.push(pRoot->m_Sides[nEndSide]);
		}
		pRoot = pRoot->m_Sides[nStartSide];
	}

	if (pResult->m_bCollision)
		pResult->m_pObject = pObject;
}

void CPlayerMover::CollideWithAABB_GetCornerIntersect(
		const LTVector &vAABBCenter,
		const LTVector &vAABBDims,
		const LTVector &vStart,
		const LTVector &vEnd,
		SCollideResult *pResult) const
{
	// Check for vStart being in the extended dimension range (Note : Only checking
	// vStart is required, due to how this function gets called...)
	LTVector vStartOfs(vStart.x - vAABBCenter.x, 0.0f, vStart.z - vAABBCenter.z);
	if ((fabsf(vStartOfs.x) < vAABBDims.x) && (fabsf(vStartOfs.z) < vAABBDims.z))
		return;

	// Figure out which corner to check
	LTVector vCorner(
		(vStartOfs.x < 0.0f) ? (vAABBCenter.x - vAABBDims.x) : (vAABBCenter.x + vAABBDims.x),
		0.0f,
		(vStartOfs.z < 0.0f) ? (vAABBCenter.z - vAABBDims.z) : (vAABBCenter.z + vAABBDims.z));

	CalculateLineCircleIntersect(vCorner, vStart, vEnd, pResult);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	A method to compute a ray-AABB intersection.
 *	Based on algorithm by Andrew Woo, from "Graphics Gems", Academic Press, 1990
 *	Adapted code by Kevin Francis, 2002
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CPlayerMover::CollideWithAABB(const LTVector &vAABBCenter, const LTVector &vAABBDims, const LTVector &vStart, const LTVector &vEnd, SCollideResult *pResult) const
{
	// Expand the dims by the size of the player
	LTVector vExpandedDims = m_vPlayerDims;
	vExpandedDims.x = LTMAX(vExpandedDims.x, vExpandedDims.z);
	vExpandedDims.z = vExpandedDims.x;
	vExpandedDims += vAABBDims;

	bool bInside = true;
	LTVector MinB = vAABBCenter - vExpandedDims;
	LTVector MaxB = vAABBCenter + vExpandedDims;
	LTVector MaxT(-1.0f, -1.0f, -1.0f);
	LTVector vCoord;
	LTVector vOffset = vEnd - vStart;

	// Find candidate planes.
	for(uint32 i=0;i<3;i++)
	{
		if (vStart[i] < MinB[i])
		{
			vCoord[i]	= MinB[i];
			bInside		= false;

			// Calculate T distances to candidate planes
			if (vOffset[i])	
				MaxT[i] = (MinB[i] - vStart[i]) / vOffset[i];
		}
		else if (vStart[i] > MaxB[i])
		{
			vCoord[i]	= MaxB[i];
			bInside		= false;

			// Calculate T distances to candidate planes
			if (vOffset[i])	
				MaxT[i] = (MaxB[i] - vStart[i]) / vOffset[i];
		}
	}

	// Ray origin inside bounding box
	if(bInside)
	{
		// Allow movement away from the center
		LTVector vCenterOffset = vStart - vAABBCenter;
		if (vCenterOffset.Dot(vOffset) >= 0.0f)
			return;

		// Intersect the corners
		CollideWithAABB_GetCornerIntersect(vAABBCenter, vAABBDims, vStart, vEnd, pResult);
		// If we didn't hit a corner, figure out if we're still inside the box
		if (!pResult->m_bCollision)
		{
			if ((fabsf(vCenterOffset.x) < vAABBDims.x) ||
				(fabsf(vCenterOffset.z) < vAABBDims.z))
			{
				// We're inside the interior portion
				pResult->m_bCollision = true;
				pResult->m_cPlane.Init(-vOffset.Unit(), vStart);
				pResult->m_fTime = 0.0f;
			}
		}
	}
	else
	{
		// Get largest of the maxT's for final choice of intersection
		uint32 nWhichPlane = 0;
		if(MaxT[1] > MaxT[nWhichPlane])	nWhichPlane = 1;
		if(MaxT[2] > MaxT[nWhichPlane])	nWhichPlane = 2;

		// Check final candidate actually inside box
		bool bIntersect = (MaxT[nWhichPlane] > 0.0f) && (MaxT[nWhichPlane] < 1.0f);

		for(int i=0;bIntersect && i<3;i++)
		{
			if(i!=nWhichPlane)
			{
				vCoord[i] = vStart[i] + MaxT[nWhichPlane] * vOffset[i];
				if(vCoord[i] < MinB[i] || vCoord[i] > MaxB[i])	
					bIntersect = false;
			}
		}

		// Further check the intersection against the rounded corners
		if (bIntersect && (nWhichPlane != 1))
		{
			if ((fabsf(vCoord.x - vAABBCenter.x) > vAABBDims.x) && (fabsf(vCoord.z - vAABBCenter.z) > vAABBDims.z))
			{
				CollideWithAABB_GetCornerIntersect(vAABBCenter, vAABBDims, vCoord, vEnd, pResult);
				if (pResult->m_bCollision)
				{
					// Re-parameterize the time
					pResult->m_fTime = MaxT[nWhichPlane] + (1.0f - MaxT[nWhichPlane]) * pResult->m_fTime;
					// Jump out so we don't try to set the plane to an axis aligned one
					return;
				}
				else
					bIntersect = false;
			}
		}

		if (bIntersect)
		{
			pResult->m_bCollision = true;
			pResult->m_fTime = MaxT[nWhichPlane];
			pResult->m_cPlane = LTPlane(0.0f, 0.0f, 0.0f, 0.0f);
			pResult->m_cPlane.m_Normal[nWhichPlane] = (vCoord[nWhichPlane] > vAABBCenter[nWhichPlane]) ? 1.0f : -1.0f;
			pResult->m_cPlane.m_Dist = vAABBDims[nWhichPlane] + (vAABBCenter[nWhichPlane] * pResult->m_cPlane.m_Normal[nWhichPlane]);
		}
		else
		{
			// No collision
		}
	}
}

void CPlayerMover::CollideWithObject(LTObject *pObject, const LTVector &vStart, const LTVector &vEnd, SCollideResult *pResult) const
{
	// Initialize to a non-collision state, so we don't have to constantly worry about that
	// in the object-specific collision functions
	pResult->m_bCollision = false;
	pResult->m_fTime = 1.0f;
	pResult->m_cPlane = LTPlane(0.0f, 0.0f, 0.0f, 0.0f);
	pResult->m_pObject = 0;
	pResult->m_pObjectNode = 0;

	// Collision doesn't work unless we're moving...
	if (vStart == vEnd)
		return;

	if (pObject->HasWorldModel() && ((pObject->m_Flags & FLAG_BOXPHYSICS) == 0))
		CollideWithWorldModel(pObject, vStart, vEnd, pResult);
	else
		CollideWithAABB(pObject->GetPos(), pObject->GetDims(), vStart, vEnd, pResult);

	if (pResult->m_bCollision)
		pResult->m_pObject = pObject;
}

void CPlayerMover::FindIntersectingObjects_Callback(WorldTreeObj *pObj, void *pUser)
{
	FIOCallback_Data *pResult = (FIOCallback_Data *)pUser;
	if (*pResult->m_pNumObjects < pResult->m_nMaxObjects)
		pResult->m_pObjects[*pResult->m_pNumObjects] = (LTObject *)pObj;
	++(*pResult->m_pNumObjects);
}

void CPlayerMover::FindIntersectingObjects(const LTVector &vStart, const LTVector &vEnd, LTObject** pResultObjects, uint32 *pResultCount, uint32 nMaxObjects) const
{
	*pResultCount = 0;
	FIOCallback_Data sResultStruct(pResultObjects, pResultCount, nMaxObjects);

	FindObjInfo cFindInfo;
	VEC_MIN(cFindInfo.m_Min, vStart, vEnd);
	VEC_MAX(cFindInfo.m_Max, vStart, vEnd);
	cFindInfo.m_Min -= m_vPlayerDims;
	cFindInfo.m_Max += m_vPlayerDims;
	cFindInfo.m_CB = FindIntersectingObjects_Callback;
	cFindInfo.m_pCBUser = &sResultStruct;
	m_pWorldTree->FindObjectsInBox2(&cFindInfo);
}

void CPlayerMover::Collide(const LTVector &vStart, const LTVector &vEnd, SCollideResult *pResult) const
{
	// Start with a non-collision state
	pResult->m_fTime = 1.0f;
	pResult->m_cPlane = LTPlane(0.0f, 0.0f, 0.0f, 0.0f);
	pResult->m_pObject = 0;
	pResult->m_pObjectNode = 0;
	pResult->m_bCollision = false;

	// We can't collide unless we're moving
	if (vStart == vEnd)
		return;

	LTVector vCollideStart = vStart;
	LTVector vCollideEnd = vEnd;

	// Collide against the blocker polygons
	float fBlockerTime;
	LTVector vBlockerNormal;
	bool bHitBlocker = g_iWorldBlockerData->Intersect(vCollideStart, vCollideEnd, m_vPlayerDims, &fBlockerTime, &vBlockerNormal);
	if (bHitBlocker)
	{
		// Update the collision end point
		vCollideEnd = vCollideStart + (vCollideEnd - vCollideStart) * fBlockerTime;
		// Update the results
		pResult->m_bCollision = true;
		pResult->m_fTime = fBlockerTime;
		pResult->m_cPlane = LTPlane(vBlockerNormal, vCollideEnd);
	}

	// Find all intersecting objects
	static uint32 nMaxObjectCount = 32;
	LTObject **pObjects = (LTObject**)alloca(nMaxObjectCount * sizeof(LTObject*));
	uint32 nNumObjects;
	FindIntersectingObjects(vCollideStart, vCollideEnd, pObjects, &nNumObjects, nMaxObjectCount);
	if (nNumObjects > nMaxObjectCount)
	{
		// If we overflow our object intersection count, increase our static buffer size and try it again
		nMaxObjectCount = nNumObjects;
		pObjects = (LTObject**)alloca(nMaxObjectCount * sizeof(LTObject*));
		FindIntersectingObjects(vCollideStart, vCollideEnd, pObjects, &nNumObjects, nMaxObjectCount);
		ASSERT(nNumObjects == nMaxObjectCount);
	}

	// if we're not going to touch anything, we're done
	if (nNumObjects == 0)
		return;

	// How far to move away from the normal after a collision
	const float k_nMoveBackDist = k_fSafetyZone;
	// Cos of minimum allowable angle to attempt a move-back
	const float k_fMinMoveBackAngle = 0.001f;

	// This is our current destination
	LTVector vCurDest = vCollideEnd;

	// Movement offset
	LTVector vOffset = vCollideEnd - vCollideStart;
	float fMoveDist = vOffset.Mag();
	LTVector vMoveDir = vOffset / fMoveDist;

	LTObject **pEndObj = &pObjects[nNumObjects];
	for (LTObject **pCurObj = pObjects; pCurObj < pEndObj; ++pCurObj)
	{
		// Skip objects we shouldn't collide with
		if (!ShouldCollideWithObject(*pCurObj))
			continue;

		// Collide with this object
		SCollideResult sLocalCollide;
		CollideWithObject(*pCurObj, vCollideStart, vCurDest, &sLocalCollide);
		if (!sLocalCollide.m_bCollision)
			continue;

		// Remember the results
		pResult->m_fTime *= sLocalCollide.m_fTime;
		pResult->m_cPlane = sLocalCollide.m_cPlane;
		pResult->m_pObject = sLocalCollide.m_pObject;
		pResult->m_pObjectNode = sLocalCollide.m_pObjectNode;
		pResult->m_bCollision = true;

		// Back up a bit
		float fAngle = -vMoveDir.Dot(pResult->m_cPlane.m_Normal);
		if (fAngle > k_fMinMoveBackAngle)
		{
			float fMoveBackDist = k_nMoveBackDist / fAngle;
			float fTimeAdj = fMoveBackDist / fMoveDist;
			pResult->m_fTime -= LTCLAMP(fTimeAdj, 0.0f, pResult->m_fTime);
		}

		if (pResult->m_fTime <= 0.0f)
			break;

		// Adjust the current destination
		VEC_LERP(vCurDest, vCollideStart, vCollideEnd, pResult->m_fTime);
	}

	ASSERT((pResult->m_fTime >= 0.0f) && (pResult->m_fTime <= 1.0f));
}

bool CPlayerMover::Slide(const LTVector &vStart, const LTVector &vEnd, const LTVector *pRestrictionPlane, LTVector *pResult, bool *pHitNonStep) const
{
	// The original offset, precalculated for her pleasure
	LTVector vOffset = vEnd - vStart;

	// The current movement portion
	LTVector vCurStart = vStart;
	LTVector vCurDest = vEnd;

	// Maximum number of sliding iterations
	const uint32 k_nMaxSlideCount = 5;
	
	// Clipping normals
	const uint32 k_nMaxClipNormals = (k_nMaxSlideCount + 1) * (k_nMaxSlideCount + 1) + 2;
	LTVector aClipNormals[k_nMaxClipNormals];
	uint32 nNumClipNormals = 0;

	// Only slide along one plane if that's what they want
	/*
	if (pRestrictionPlane)
	{
		aClipNormals[nNumClipNormals] = *pRestrictionPlane;
		++nNumClipNormals;
		aClipNormals[nNumClipNormals] = -*pRestrictionPlane;
		++nNumClipNormals;
	}
	//*/

	// Epsilon indicating that no movement is going to happen (min magsqr of movement offset)
	const float k_nNonMovementEpsilon = 0.0001f;

	bool bResult = false;

	if (pHitNonStep)
		*pHitNonStep = false;

	// How much we can try to fix pre-existing intersections
	const float k_fIntersectionFixingAllowed = vOffset.Mag();
	float fIntersectionFixingRemaining = k_fIntersectionFixingAllowed;
	float fTimeSpent = 0.0f;
	LTVector vPrevNormal(0.0f, 0.0f, 0.0f);

	for (uint32 nCurSlide = 0; nCurSlide < k_nMaxSlideCount; ++nCurSlide)
	{
		// Find the first collision time
		SCollideResult sCollision;
		Collide(vCurStart, vCurDest, &sCollision);
		// If we didn't hit something, break out
		if (!sCollision.m_bCollision)
			break;

		bResult = true;

		// Did we hit a non-step?
		if (pHitNonStep && sCollision.m_pObjectNode && ((sCollision.m_pObjectNode->m_pPoly->GetSurface()->GetFlags() & SURF_NOTASTEP) != 0))
			*pHitNonStep = true;

		// Figure out where we stopped
		LTVector vCurOfs = vCurDest - vCurStart;
		LTVector vCollisionPt = vCurStart + vCurOfs * sCollision.m_fTime;

#if 0
		SCollideResult sDoubleCheckCollision;
		Collide(vStart, vCollisionPt, &sDoubleCheckCollision);
		ASSERT(!sDoubleCheckCollision.m_bCollision);
#endif

		LTVector vNewNormal = sCollision.m_cPlane.m_Normal;

		// Find out if we just hit this normal
		const float k_fDuplicateNormalEpsilon = 0.001f;
		bool bDuplicateNormal = false;
		if (nCurSlide)
			bDuplicateNormal = vNewNormal.NearlyEquals(vPrevNormal, k_fDuplicateNormalEpsilon);

		if (bDuplicateNormal)
		{
			if ((sCollision.m_fTime == 0.0f) && (fTimeSpent == 0.0f))
			{
				const float k_fIntersectionFixingPortion = k_fIntersectionFixingAllowed / 4.0f;
				if (fIntersectionFixingRemaining < k_fIntersectionFixingPortion)
				{
					// Ok, this is bad.  We started out intersecting with something
					// and we can't slide off of it.
					vCurDest = vCollisionPt;
					break;
				}
				fIntersectionFixingRemaining -= k_fIntersectionFixingPortion;
				// Move the starting and ending positions away from the plane
				LTVector vIntersectionFixingOffset = vNewNormal * k_fIntersectionFixingPortion;
				vCurStart += vIntersectionFixingOffset;
				vCurDest += vIntersectionFixingOffset;
				// Don't count this as one of our iterations
				--nCurSlide;
				// Start over
				continue;
			}
		}

		vPrevNormal = vNewNormal;

		/*
		// Project the resulting plane's normal against the pre-existing normal set
		const float k_fWedgedNormalEpsilon = 0.001f;
		const float k_fInverseNormalDot = -0.999f;
		bool bWedgedNormal = false;
		bool bPerpNormal = false;
		uint32 nPrevNumNormals = nNumClipNormals;
		for (uint32 nCheckNormal = 0; nCheckNormal < nPrevNumNormals; ++nCheckNormal)
		{
			float fNewNormalDot = vNewNormal.Dot(aClipNormals[nCheckNormal]);
			if ((fNewNormalDot < 0.0f) && (fNewNormalDot > k_fInverseNormalDot))
			{
				LTVector vPerpNormal = vNewNormal - fNewNormalDot * aClipNormals[nCheckNormal];
				float fNewNormalMagSqr = vPerpNormal.MagSqr();
				if (fNewNormalMagSqr < k_fWedgedNormalEpsilon)
				{
					bWedgedNormal = true;
					break;
				}
				else
					vPerpNormal /= sqrtf(fNewNormalMagSqr);

				// Add the collision normal to the clipping normals
				ASSERT(nNumClipNormals < k_nMaxClipNormals);
				aClipNormals[nNumClipNormals] = vPerpNormal;
				++nNumClipNormals;

				bPerpNormal = true;
			}
		}
		// If it's going to wedge us in, just stop
		if (bWedgedNormal)
		{
			vCurDest = vCollisionPt;
			break;
		}

		if (!bPerpNormal)
		{
			// Add the collision normal to the clipping normals
			ASSERT(nNumClipNormals < k_nMaxClipNormals);
			aClipNormals[nNumClipNormals] = vNewNormal;
			++nNumClipNormals;
		}

		// Project the movement onto the clipping normals
		const float k_fPlaneDeflection = 1.001f;
		const float k_fFacingPlaneEpsilon = 0.0f;
		for (uint32 nCurNormal = 0; nCurNormal < nNumClipNormals; ++nCurNormal)
		{
			float fOfsDotPlane = vCurOfs.Dot(aClipNormals[nCurNormal]);
			if (fOfsDotPlane < k_fFacingPlaneEpsilon)
				vCurOfs += fabsf(fOfsDotPlane * k_fPlaneDeflection) * aClipNormals[nCurNormal];
		}
		//*/

		//*
		// Handle the normals explicitly with slide cases for 1 or 2 planes, and stopping if we hit 3
		if (nCurSlide == 0)
		{
			aClipNormals[nNumClipNormals] = vNewNormal;
			++nNumClipNormals;

			// Project the movement onto the clipping normal
			const float k_fPlaneDeflection = 1.001f;
			const float k_fFacingPlaneEpsilon = 0.0f;
			float fOfsDotPlane = vCurOfs.Dot(aClipNormals[0]);
			if (fOfsDotPlane < k_fFacingPlaneEpsilon)
				vCurOfs += fabsf(fOfsDotPlane * k_fPlaneDeflection) * aClipNormals[0];
		}
		else if ((nCurSlide == 1) && (fabsf(vNewNormal.Dot(aClipNormals[0])) < 0.999f))
		{
			vNewNormal = vNewNormal.Cross(aClipNormals[0]).Unit();
			vCurOfs = vNewNormal * vCurOfs.Dot(vNewNormal);
		}
		else
		{
			vCurDest = vCollisionPt;
			break;
		}
		//*/

		// Figure out how much we've got left to move
		vCurOfs *= (1.0f - sCollision.m_fTime);
		fTimeSpent += sCollision.m_fTime;

		// If we're not going to move, break out
		LTVector vTotalOfs = vCurDest - vStart;
		if ((vCurOfs.MagSqr() < k_nNonMovementEpsilon) || 
			(nCurSlide == (k_nMaxSlideCount - 1)) || 
			(vOffset.Dot(vTotalOfs) < 0.0f))
		{
			vCurDest = vCollisionPt;
			break;
		}

		// Set up for the next loop
		vCurStart = vCollisionPt;
		if (nCurSlide != (k_nMaxSlideCount - 1))
			vCurDest = vCollisionPt + vCurOfs;
		else
			vCurDest = vCollisionPt;
	}

	// And this is where we ended up...
	*pResult = vCurDest;
	
	return bResult;
}

void CPlayerMover::StairStep(const LTVector &vStart, const LTVector &vEnd, LTVector *pResult) const
{
	SCollideResult sCollide;

	LTVector vOffset = vEnd - vStart;
	LTVector vFloorNormal;
	const LTVector *pFloorNormal = 0;

	// Figure out what we're standing on so we only slide along that plane
	if (m_pPlayer->m_pStandingOn)
	{
		Collide(vStart, LTVector(vStart.x, LTMIN(vEnd.y, vStart.y) - m_fPlayerHeight * k_fSafetyZone, vStart.z), &sCollide);
		if (sCollide.m_bCollision && (vOffset.Dot(sCollide.m_cPlane.m_Normal) <= 0.0f))
		{
			vFloorNormal = sCollide.m_cPlane.m_Normal;
			pFloorNormal = &vFloorNormal;
			// Hack for dealing with moving up steep slopes...
			LTVector vProjectedOffset = vOffset - vFloorNormal * vOffset.Dot(vFloorNormal);
			if ((vFloorNormal.y < k_fSteepAngle) && (vFloorNormal.y > 0.0f) && (vProjectedOffset.y > 0.0f))
			{
				LTVector vSideways = vFloorNormal.Cross(LTVector(0.0f, 1.0f, 0.0f)).Unit();
				vOffset = vSideways * vSideways.Dot(vOffset);
			}
		}
	}

	// Slide toward the destination
	bool bHitNonStep;
	LTVector vLowDest;
	bool bLowCollision = Slide(vStart, vStart + vOffset, pFloorNormal, &vLowDest, &bHitNonStep);

	// Jump out if we didn't touch anything at our current height, or if we hit a non-step at this height
	if (!bLowCollision || bHitNonStep)
	{
		*pResult = vLowDest;
		return;
	}

	// Move up by the stair step height (no slide)
	LTVector vHighStart(vStart.x, vStart.y + m_fPlayerStepHeight, vStart.z);
	Collide(vStart, vHighStart, &sCollide);
	// If we hit something, we're wedged, and can't stairstep
	if (sCollide.m_bCollision)
	{
		*pResult = vLowDest;
		return;
	}

	// Slide toward the high destination
	LTVector vHighSlideDest;
	bool bHighCollision = Slide(vHighStart, vHighStart + vOffset, pFloorNormal, &vHighSlideDest);

	// Move back down
	LTVector vHighSlideDownPos;
	vHighSlideDownPos = LTVector(vHighSlideDest.x, vHighSlideDest.y - m_fPlayerStepHeight, vHighSlideDest.z);
	Collide(vHighSlideDest, vHighSlideDownPos, &sCollide);
	// Remember where we ended up
	LTVector vHighDest;
	VEC_LERP(vHighDest, vHighSlideDest, vHighSlideDownPos, sCollide.m_fTime);

	// Handle steep slopes
	bool bSteepAngle = (sCollide.m_pObjectNode ? (sCollide.m_pObjectNode->GetPlane()->m_Normal.y < k_fSteepAngle) : (sCollide.m_cPlane.m_Normal.y < k_fSteepAngle));
	if (sCollide.m_bCollision && bSteepAngle)
	{
		// Just forget about it if we hit something on the high side
		if (bHighCollision)
		{
			*pResult = vLowDest;
			return;
		}
		// See if we can go over it.  This is to handle cases where high framerates
		// make the timestep too small to step forward over steep slopes
		const LTVector &vPlaneNormal = sCollide.m_pObjectNode ? sCollide.m_pObjectNode->GetPlane()->m_Normal : sCollide.m_cPlane.m_Normal;
		// This gets the up-slope direction of the plane's normal, projected onto the XZ plane
		LTVector vMovementDir(-vPlaneNormal.x * vPlaneNormal.y, 0.0f, -vPlaneNormal.z * vPlaneNormal.y);
		float fSlopeMovement = 2.0f * m_fPlayerStepHeight * (1.0f - sCollide.m_cPlane.m_Normal.y * sCollide.m_cPlane.m_Normal.y) + 0.1f;
		float fMoveForward = LTMAX(m_fPlayerStepMoveDistance, fSlopeMovement);
		Collide(vHighStart, vHighSlideDest + vMovementDir.Unit() * fMoveForward, &sCollide);
		bool bSteepAngle = (sCollide.m_pObjectNode ? (sCollide.m_pObjectNode->GetPlane()->m_Normal.y < k_fSteepAngle) : (sCollide.m_cPlane.m_Normal.y < k_fSteepAngle));
		if (sCollide.m_bCollision && bSteepAngle)
		{
			// Nope, not a step..
			*pResult = vLowDest;
			return;
		}
		// Don't worry about standing on this polygon.  SetStandingOn will handle it.
	}

	// Figure out which one of them moved us farther along the movement vector
	float fLowDot = LTMAX((vLowDest - vStart).Dot(vOffset), 0.0f);
	float fHighDot = (vHighDest - vStart).Dot(vOffset);

	// Return that position
	const float k_fDontStepBias = 1.01f;
	bool bStepUp = ((fLowDot * k_fDontStepBias) < fHighDot);
	*pResult = bStepUp ? vHighDest : vLowDest;
}

void CPlayerMover::SetStandingOn(const LTVector &vPos, const LTVector &vMovement) const
{
	const float k_fStandingOnMovementDist = k_fSafetyZone * 0.2f * m_fPlayerHeight;

	// Look down
	SCollideResult sCollide;
	Collide(vPos, LTVector(vPos.x, vPos.y - k_fStandingOnMovementDist, vPos.z), &sCollide);

	if (!sCollide.m_bCollision || !sCollide.m_pObject)
	{
		// We're not standing on anything
		if (m_bDetachStandingOn)
			DetachObjectStanding(m_pPlayer);
		return;
	}

	const float k_fVerticalCollisionEpsilon = 0.001f;
	
	bool bSteepAngle = (sCollide.m_pObjectNode ? (sCollide.m_pObjectNode->GetPlane()->m_Normal.y < k_fSteepAngle) : (sCollide.m_cPlane.m_Normal.y < k_fSteepAngle));
	if (bSteepAngle &&
		m_pPlayer->m_pNodeStandingOn && (m_pPlayer->m_pNodeStandingOn->GetPlane()->m_Normal.y >= k_fSteepAngle))
	{
		// See if we can go over it.
		LTVector vHighStart(vPos.x, vPos.y + m_fPlayerStepHeight, vPos.z);
		SCollideResult sStepOverCollide;
		const LTVector &vPlaneNormal = sCollide.m_pObjectNode ? sCollide.m_pObjectNode->GetPlane()->m_Normal : sCollide.m_cPlane.m_Normal;
		// This gets the up-slope direction of the plane's normal, projected onto the XZ plane
		LTVector vMovementDir(-vPlaneNormal.x * vPlaneNormal.y, 0.0f, -vPlaneNormal.z * vPlaneNormal.y);
		vMovementDir.Normalize();
		float fSlopeMovement = 2.0f * (m_fPlayerStepHeight + k_fStandingOnMovementDist * sCollide.m_fTime) * (1.0f - sCollide.m_cPlane.m_Normal.y * sCollide.m_cPlane.m_Normal.y) + k_fSafetyZone * 1.1f * m_fPlayerRadius;
		float fMoveForward = LTMAX(m_fPlayerStepMoveDistance, fSlopeMovement);
		Collide(vHighStart, vHighStart + vMovementDir * fMoveForward, &sStepOverCollide);
		// If there's nothing in front of us if we were to step up, stick with what we're currently standing on
		if (!sStepOverCollide.m_bCollision)
			return;
	}
	else if (sCollide.m_cPlane.m_Normal.y < k_fVerticalCollisionEpsilon)
	{
		// Avoid standing on things that are vertical
		if (m_bDetachStandingOn)
			DetachObjectStanding(m_pPlayer);
		return;
	}

	// Tell the player we're standing on that
	SetObjectStanding(m_pPlayer, sCollide.m_pObject, sCollide.m_pObjectNode);
}

void CPlayerMover::TouchObjects(const LTVector &vStart, const LTVector &vEnd) const
{
	// Find all intersecting objects
	static uint32 nMaxObjectCount = 32;
	LTObject **pObjects = (LTObject**)alloca(nMaxObjectCount * sizeof(LTObject*));
	uint32 nNumObjects;
	FindIntersectingObjects(vStart, vEnd, pObjects, &nNumObjects, nMaxObjectCount);
	if (nNumObjects > nMaxObjectCount)
	{
		// If we overflow our object intersection count, increase our static buffer size and try it again
		nMaxObjectCount = nNumObjects;
		pObjects = (LTObject**)alloca(nMaxObjectCount * sizeof(LTObject*));
		FindIntersectingObjects(vStart, vEnd, pObjects, &nNumObjects, nMaxObjectCount);
		ASSERT(nNumObjects == nMaxObjectCount);
	}

	// if we're not going to touch anything, we're done
	if (nNumObjects == 0)
		return;

	LTVector vMoveMin;
	LTVector vMoveMax;
	VEC_MIN(vMoveMin, vStart, vEnd);
	VEC_MAX(vMoveMax, vStart, vEnd);
	vMoveMin -= m_vPlayerDims;
	vMoveMax += m_vPlayerDims;

	// Go through the objects
	LTObject **pEndObj = &pObjects[nNumObjects];
	for (LTObject **pCurObj = pObjects; pCurObj < pEndObj; ++pCurObj)
	{
		bool bIsSolid = (IsSolid((*pCurObj)->m_Flags, m_bServer) != LTFALSE);

		// Skip objects that aren't interested in touch notification
		if (!ShouldTouchObject(*pCurObj, bIsSolid))
			continue;

		// Non-solid collision detection w/ a worldmodel requires an in/out test
		// to preserve old behavior
		if (!bIsSolid && (*pCurObj)->HasWorldModel() && (((*pCurObj)->m_Flags & FLAG_BOXPHYSICS) == 0))
		{
			WorldModelInstance *pWM = (*pCurObj)->ToWorldModel();
			if (DoesBoxIntersectBSP(pWM->GetValidBsp()->GetRootNode(),
				vMoveMin, vMoveMax,
				pWM->m_ObjectType == OT_CONTAINER))
			{
				DoNonsolidCollision(m_pMoveAbstract, m_pPlayer, *pCurObj);
			}
			continue;
		}

		// Check for an intersection
		SCollideResult sCollide;
		CollideWithObject(*pCurObj, vStart, vEnd, &sCollide);
		if (!sCollide.m_bCollision)
			continue;

		// Send a touch notification
		if (bIsSolid)
		{
			HPOLY hPoly = INVALID_HPOLY;
			if (sCollide.m_pObject && sCollide.m_pObjectNode)
			{
				hPoly = sCollide.m_pObject->ToWorldModel()->MakeHPoly(sCollide.m_pObjectNode);
			}
			DoInterObjectCollisionResponse(
				m_pMoveAbstract, 
				m_pPlayer, 
				*pCurObj, 
				&sCollide.m_cPlane.m_Normal, 
				sCollide.m_cPlane.m_Dist,
				hPoly
				);
		}
		else
			DoNonsolidCollision(m_pMoveAbstract, m_pPlayer, *pCurObj);
	}
}

void CPlayerMover::MoveTo(const LTVector &vEnd, LTVector *pResult)
{
	LTVector vOrigin = m_pPlayer->GetPos();

	if (!m_bTeleport && (vOrigin != vEnd))
	{
		// Handle FLAG_NOSLIDING
		if (m_nPlayerFlags & FLAG_NOSLIDING)
		{
			// Note : Can't stairstep without sliding
			SCollideResult sCollide;
			Collide(vOrigin, vEnd, &sCollide);
			*pResult = vOrigin + (vEnd - vOrigin) * sCollide.m_fTime;
		}
		else
		{
			// Figure out where we're going to end up
			if ((m_nPlayerFlags & FLAG_STAIRSTEP) && ((vOrigin.x != vEnd.x) || (vOrigin.z != vEnd.z)))
				StairStep(vOrigin, vEnd, pResult);
			else
				Slide(vOrigin, vEnd, 0, pResult);
		}
	}
	else
	{
		vOrigin = vEnd;
		*pResult = vEnd;
	}

	// Guard against NAN bugs making it out of this routine
	if (isnan(pResult->x) || isnan(pResult->y) || isnan(pResult->z))
	{
		ASSERT(!"Invalid result encountered in player movement");
		*pResult = vOrigin;
	}

	if ((vEnd.y >= vOrigin.y) || (pResult->y > vEnd.y))
	{
		// Find out if we're standing on something at that position
		SetStandingOn(*pResult, vEnd - vOrigin);
	}
	// If we're supposed to be mid-air, make sure we're not standing on anything
	else if (m_bDetachStandingOn)
		DetachObjectStanding(m_pPlayer);

	// Go back and touch everything along the path
	// Note : This has to happen even if teleporting, to preserve old physics affecting behavior
	const LTVector vSafetyDims(k_fSafetyZone, k_fSafetyZone, k_fSafetyZone);
	LTVector vOriginalDims = m_vPlayerDims;

	// Increase the dimensions of the player by the safety zone for touching
	ChangeDims(vOriginalDims + vSafetyDims * 2.0f);

	TouchObjects(vOrigin, *pResult);
	
	// Put back the player's dims
	ChangeDims(vOriginalDims);

	/*
	dsi_ConsolePrint("Movement: (%0.2f,%0.2f,%0.2f) - (%0.2f,%0.2f,%0.2f) = (%0.2f,%0.2f,%0.2f)",
		VEC_EXPAND(vOrigin), VEC_EXPAND(vEnd), VEC_EXPAND(*pResult));
	*/
}


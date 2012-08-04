#ifndef __MOVEPLAYER_H__
#define __MOVEPLAYER_H__

#include "de_objects.h" // Need LTObject for ShouldMoveObject to be inlined

class MoveState;
class WorldTreeObj;
class WorldPoly;

class CPlayerMover
{
public:
	CPlayerMover(const MoveState &cMoveState, uint32 nFlags);
	// Move the player to the specified destination, touching objects as necessary
	// Returns the position where they would end up
	void MoveTo(const LTVector &vEnd, LTVector *pResult);

	// Should this object be moved by this class?
	static bool ShouldMoveObject(const LTObject *pObj, bool bServer) {
		return (IsPhysical(pObj->m_Flags, bServer) &&
				(pObj->m_Flags2 & FLAG2_PLAYERCOLLIDE) &&
				(pObj->m_Flags & FLAG_SOLID));
	}

protected:
	// Safety zone around the dimensions of the player
	static const float k_fSafetyZone;
	// Percentage of the bounding box radius that has to be able to fit onto a step to step over a steep slope
	static const float k_fNotAStepDist;
	// Any plane's normal with a Y value above this value is considered "steep"
	static const float k_fSteepAngle;

	struct SCollideResult
	{
		// Time of the collision [0..1]
		float m_fTime;
		// The collision plane
		LTPlane m_cPlane;
		// Object we collided with (Note : May be null in the case of a blocker poly)
		LTObject *m_pObject;
		// BSP node we collided with
		const Node *m_pObjectNode;
		// Did we hit anything?
		bool m_bCollision;
	};
	// Collision function
	void Collide(const LTVector &vStart, const LTVector &vEnd, SCollideResult *pResult) const;
	// Move the player, sliding along surfaces
	// Returns false if nothing got touched along the way
	bool Slide(const LTVector &vStart, const LTVector &vEnd, const LTVector *pRestrictionPlane, LTVector *pResult, bool *pHitNonStep = 0) const;
	// Move the player, stair-stepping and sliding as needed
	void StairStep(const LTVector &vStart, const LTVector &vEnd, LTVector *pResult) const;
	// Figure out what the player should be standing on
	void SetStandingOn(const LTVector &vPos, const LTVector &vMovement) const;
	// Touch all of the objects along the movement path
	void TouchObjects(const LTVector &vStart, const LTVector &vEnd) const;

	// Should the player collide with this object?
	bool ShouldCollideWithObject(const LTObject *pObj) const;
	// Should the player touch this object?
	bool ShouldTouchObject(const LTObject *pObj, bool bSolid) const;

	// Test a collision against a single object
	// Note : Solid/physical/etc flags and bounds overlap are not checked by this function
	void CollideWithObject(LTObject *pObject, const LTVector &vStart, const LTVector &vEnd, SCollideResult *pResult) const;

	// Find all objects possibly intersecting the movement path
	// Note : *pResultCount will be set to a number > nMaxObjects in an overflow situation
	void FindIntersectingObjects(const LTVector &vStart, const LTVector &vEnd, LTObject** pResultObjects, uint32 *pResultCount, uint32 nMaxObjects) const;

	// Callback stuff for FindIntersectingObjects
	struct FIOCallback_Data
	{
		FIOCallback_Data(LTObject **pObjects, uint32 *pNumObjects, uint32 nMaxObjects) :
			m_pObjects(pObjects), 
			m_pNumObjects(pNumObjects),
			m_nMaxObjects(nMaxObjects)
		{}
		LTObject **m_pObjects;
		uint32 *m_pNumObjects;
		uint32 m_nMaxObjects;
	};
	static void FindIntersectingObjects_Callback(WorldTreeObj *pObj, void *pUser);

	// Test a collision against a worldmodel
	void CollideWithWorldModel(LTObject *pObject, const LTVector &vStart, const LTVector &vEnd, SCollideResult *pResult) const;
	// Test a collision against an AABB
	void CollideWithAABB(const LTVector &vAABBCenter, const LTVector &vAABBDims, const LTVector &vStart, const LTVector &vEnd, SCollideResult *pResult) const;

	// General utility functions
	void CalculateLineCircleIntersect(
		const LTVector &vCenter, 
		const LTVector &vStart, 
		const LTVector &vEnd, 
		SCollideResult *pResult) const;
	void CalculateLineCylinderIntersect(
		const LTVector &vBase,
		float fHeight,
		const LTVector &vStart, 
		const LTVector &vEnd, 
		SCollideResult *pResult) const;
	bool DoesLineIntersectCircle(
		const LTVector &vCenter, 
		const LTVector &vStart, 
		const LTVector &vEnd,
		float *pTime) const;
	bool DoesLineIntersectCylinder(
		const LTVector &vBase,
		float fHeight,
		const LTVector &vStart, 
		const LTVector &vEnd) const;

	// Utility function for CollideWithAABB
	void CollideWithAABB_GetCornerIntersect(
		const LTVector &vAABBCenter,
		const LTVector &vAABBDims,
		const LTVector &vStart,
		const LTVector &vEnd,
		SCollideResult *pResult) const;

	// Utility functions for CollideWithWorldModel
	float GetPlaneProjection(const LTVector &vNormal) const;
	float GetPlaneIntersection(const LTVector &vNormal) const;
	PolySide GetPlaneSide(const LTPlane &cPlane, const LTVector &vPos, float *pPlaneProjection) const;
	void CollideWithWorldModel_Poly(const WorldPoly *pPoly, const LTVector &vStart, const LTVector &vOffset, float fPlaneProjection, SCollideResult *pResult) const;
	void CWWM_Poly_CalcPlaneIntersect(const LTPlane &cPlane, const LTVector &vStart, const LTVector &vOffset, float *pFrontTime, float *pBackTime, LTPlane *pFrontPlane) const;
	bool CWWM_Poly_CalcEdgeIntersect(
		const LTPlane &cPolyPlane,
		const LTVector &vEdgeStart, 
		const LTVector &vEdgeOffset, 
		const LTVector &vRayStart, 
		const LTVector &vRayOffset, 
		float *pIntersectTime, 
		LTPlane *pIntersectPlane) const;

	// The player
	LTObject *m_pPlayer;
	// Player flags, mirrored for modifying the behavior of the functionality without changing the object
	uint32 m_nPlayerFlags;
	// Player dims, similar to m_nPlayerFlags
	// Set via ChangeDims()
	LTVector m_vPlayerDims;
	// The physics interface
	ILTPhysics *m_pPhysics;
	// The world tree
	WorldTree *m_pWorldTree;
	// The movement abstraction layer
	MoveAbstract *m_pMoveAbstract;
	// Are we on the server?
	bool m_bServer;
	// Should we detach the object standing-on stuff?
	bool m_bDetachStandingOn;
	// Is this a teleportation?
	bool m_bTeleport;

	// Change the internal representation of the dims of the player
	void ChangeDims(const LTVector &vDims);

	// Pre-calculated values
	float m_fPlayerRadius;
	float m_fPlayerHeight;
	float m_fPlayerSphereRadius;
	float m_fPlayerStepMoveDistance;
	float m_fPlayerStepHeight;
};

#endif //__MOVEPLAYER_H__

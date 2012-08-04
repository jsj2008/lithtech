// Defines some of the collision detection and response routines.

#ifndef __COLLISION_H__
#define __COLLISION_H__


#ifndef __DE_WORLD_H__
#include "de_world.h"
#endif

#ifndef __LTBASEDEFS_H__
#include "ltbasedefs.h"
#endif

class MoveAbstract;


// ------------------------------------------------------------------ //
// Definitions.
// ------------------------------------------------------------------ //

// This constant is how long a collision takes.  The smaller the time gets,
// the larger the impulse vector for the stopping force becomes, so we 
// just use this one constant.
#define IMPULSE_TIME_CONSTANT	0.01f


// ------------------------------------------------------------------ //
// Types.
// ------------------------------------------------------------------ //

//
// An axis-aligned bounding box
//
struct AABB
{
	LTVector Min, Max;//extents

	//default
	AABB()
	{}

	//from min and max
	AABB( const LTVector& min, const LTVector& max )
		:	Min(min), Max(max)
	{}

	//center of the box
	const LTVector center() const
	{
		return 0.5 * (Max + Min);
	}

	//positive half-dimensions of the box
	const LTVector half_dimensions() const
	{
		return 0.5 * (Max - Min);
	}

	//check if this box intersects the point 'p'
	bool intersects( const LTVector& p ) const
	{
		return	Min.x <= p.x && p.x <= Max.x
				&&
				Min.y <= p.y && p.y <= Max.y
				&&
				Min.z <= p.z && p.z <= Max.z;
	}

	//check if this box intersects the box 'b'
	bool intersects( const AABB& b ) const
	{
		//ALGORITHM: any two intervals a and b are
		//disjoint if( b_min>a_max || a_min>b_max )
		return !(	b.Min.x > Max.x || Min.x > b.Max.x ||
					b.Min.y > Max.y || Min.y > b.Max.y ||
					b.Min.z > Max.z || Min.z > b.Max.z);
	}

	//check if this box intersects the line segment [l0,l1]
	bool intersects_line_segment( const LTVector& l0, const LTVector& l1 ) const;
};


struct CollideRequest
{
	// Constructor
	CollideRequest() : m_nRestart(0) {};

	// Abstraction layer.
	MoveAbstract	*m_pAbstract;
	CollisionInfo	*m_pCollisionInfo;

	// The world to collide against.
	const WorldBsp	*m_pWorld;
	LTObject		*m_pWorldObj;

	// The movement.
	LTVector		m_OriginalPos;
	LTVector		m_NewPos;

	// Dimensions of the object trying to move.
	LTVector		m_Dims;

	// The LTObject doing the movement.  This MUST be set.  It will calculate 
	// collision response, modify its velocity, and notify the object.
	LTObject		*m_pObject;

	// Should the object slide along polygons
	LTBOOL			m_bSlide;

	// Iteration count
	int				m_nRestart;

};

struct CollideInfo
{
	// The final (unclipped) position the object wound up at.
	LTVector	m_FinalPos;
	
	// CollideWithWorld() will set this, telling you how many times it pushed the object off of something.
	uint32		m_nHits;

	// If FLAG_STAIRSTEP is specified, this will set m_pStandingOn to
	// NULL or the object that it is standing on.
	const Node	*m_pStandingOn;									 

	// Force of collisions
	LTVector	m_vForce;

	// This is the velocity offset.  You should apply this after calling CollideWithWorld.
	// This is here so you can do a touch notify on the object before changing the velocity.
	LTVector	m_VelOffset;
};

struct PhysicsSphere
{
	LTFLOAT		m_Radius;
	LTVector	m_Center;
};


// Does this box intersect this BSP tree?
bool DoesBoxIntersectBSP
(
	const Node*		pRoot,	// root of BSP tree
	const LTVector&	min,	// box extents
	const LTVector&	max,
	bool bSolid				// Should this BSP be treated as "solid" for this test
);


// Collides the axis-aligned box with the world.  If you specify
// pObj, it'll calculate collision responses and send them to the 
// object.
void CollideWithWorld
(
	CollideRequest&	request,
	CollideInfo*	pInfo
);


// Calculates the collision response.
// pVelAdd is what you need to add to their velocity to stop them on the plane.
// pForce is the force vector (pVelAdd * pObj->m_Mass * IMPULSE_TIME_CONSTANT).
// Returns LTFALSE if there should be no collision (the normal points away 
// from the object's velocity).
LTBOOL CalculateCollisionResponse
(
	LTObject *pObj,
	LTVector *pStopPlane,
	LTVector *pVelAdd,
	LTVector *pForce
);

// Does a collision response for the object on the plane.  Pass in the node's plane
// for pPlane.
// If bUseNodePlane is LTFALSE, then it assumes you've filled in pCollisionInfo->m_Plane
// already.
void DoObjectCollisionResponse
(
	CollideRequest&	request,
	CollideInfo*	pInfo,
	const Node*		pNode,
	bool			bUseNodePlane = true
);

// Does inter-object collision.  This function is from the viewpoint of pObj1 hitting pObj2
// at pPlane.  The opposite normal of pPlane is applied for pObj2.
void DoInterObjectCollisionResponse(MoveAbstract *pAbstract,
	LTObject *pObj1, LTObject *pObj2, LTVector *pPlane, float fPlaneDist, HPOLY hPoly );


	LTBOOL ClipPolyIntoBox(WorldPoly *pPoly);

#endif  // __COLLISION_H__

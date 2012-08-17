//------------------------------------------------------------------
//
//	FILE	  : GeomRoutine:s.h
//
//	PURPOSE	  : Defines several useful templated geometry functions.
//
//	CREATED	  : 1st May 1996
//
//	COPYRIGHT : Monolith Productions Inc. 1996-2000
//
//------------------------------------------------------------------

#ifndef __GEOMROUTINES_H__
#define __GEOMROUTINES_H__


// Includes....
#ifndef __GEOMETRY_H__
#include "geometry.h"
#endif

#ifndef __LTBASETYPES_H__
#include "ltbasetypes.h"
#endif



// Defines....
#define	LOCATE_POINT_STACKSIZE	4000
#define ALL_FLAGS		(uint32)0xFFFFFFFF

#ifndef ZERO
#define ZERO 0
#endif

#ifndef NF_IN
#define NF_IN 1
#endif

#ifndef NF_OUT
#define NF_OUT 2  
#endif

// Functions....

// These can be used to build a frame of reference given a direction vector.
// - pVec must specify the forward vector.  The output forward vector will be this value.
// - pUpRef is an up 'reference vector'.. it'll make the up vector as close to that as possible.
//   If pUpRef is LTNULL, it will use (0,1,0).
void gr_GetPerpendicularVector(LTVector *pVec, LTVector *pUpRef, LTVector *pPerp);
void gr_BuildFrameOfReference(LTVector *pVec, LTVector *pUpRef, LTVector *pRight, LTVector *pUp, LTVector *pForward);


// Setup a matrix from euler angles.
#define SetupMatrixEuler gr_SetupMatrixEuler
void gr_SetupMatrixEuler(const LTVector vAngles, float mat[4][4]); // Use this one.
void gr_SetupMatrixEuler(float mat[4][4], float pitch, float yaw, float roll); // Use the other form for consistency..

// Convert euler angles to a LTRotation.
void gr_EulerToRotation(const LTVector vAngles, LTRotation *pRot);
void gr_EulerToRotation(float pitch, float yaw, float roll, LTRotation *pRot);

// Convert euler angles to orientation vectors.
void gr_GetEulerVectors(
	const LTVector vAngles,
	LTVector &vRight,
	LTVector &vUp,
	LTVector &vForward);


// Make a LTMatrix from a LTRotation.
void RotationToMatrix(LTRotation *pRot, LTMatrix *pMatrix);
void MatrixToRotation(LTMatrix *pMatrix, LTRotation *pRot);


// Get vectors from a rotation.
void gr_GetRotationVectors(LTRotation *pRot, LTVector *pRight, LTVector *pUp, LTVector *pForward);

// Interpolate between 2 rotations.
void gr_InterpolateRotation(LTRotation *pDest, LTRotation *pRot1, LTRotation *pRot2, float t);


// ----------------------------------------------------------------------- //
//   Routine:  SetupTransformation
//   Purpose:  Sets up a transformation matrix given a position and angles.
// ----------------------------------------------------------------------- //
void gr_SetupTransformation(LTVector *pPos, LTRotation *pQuat, LTVector *pScale, LTMatrix *pMat);

void gr_FindClosestPointOnVector( LTVector *pvA, LTVector *pvB, LTVector *pvPt, float *pT );


// Setup a rotation matrix that goes around the specified vector with the specified angle.
void gr_SetupRotationAroundVector(LTMatrix *pMat, LTVector v, float angle);

// Setup a world model transform.  Since WorldModel transforms need to take the
// world's translation into account, this routine simplifies building its 
// transformation matrix.
void gr_SetupWMTransform(
	LTVector *pWorldTranslation,
	const LTVector *pPos,			// Position and rotation.
	LTRotation *pRot,
	LTMatrix *pOutForward,	// Output forward and backwards transforms.
	LTMatrix *pOutBack);

// Fills in the point of intersection of the 3 planes.
// Returns LTFALSE if any of them are parallel.
LTBOOL gr_IntersectPlanes(
	LTPlane &plane0,
	LTPlane &plane1,
	LTPlane &plane2,
	LTVector &vOut);



// ----------------------------------------------------------------------- //
//
//      Routine:        g_PtInCube
//
//      Purpose:        Tells if the given point is inside the cube.
//
// ----------------------------------------------------------------------- //

template<class V>
inline LTBOOL g_PtInCube( V *pPt, V *pMin, V *pMax )
{
	if( (pPt->x < pMin->x) || (pPt->x > pMax->x) )	return LTFALSE;
	if( (pPt->y < pMin->y) || (pPt->y > pMax->y) )	return LTFALSE;
	if( (pPt->z < pMin->z) || (pPt->z > pMax->z) )	return LTFALSE;
	return LTTRUE;
}



// ----------------------------------------------------------------------- //
//
//      Routine:        g_TriArea2
//
//      Purpose:        Returns twice the signed area of the triangle
//                      defined by A, B, C, in the plane with normal N.
//
// ----------------------------------------------------------------------- //

template<class T>
inline T g_TriArea2( const TVector3<T> N, const TVector3<T> A, const TVector3<T> B, const TVector3<T> C )
{
	return N.Dot( (B - A).Cross(C - A) );
}




// ----------------------------------------------------------------------- //
//
//      Routine:        g_GenerateNormal
//
//      Purpose:        Generates the normal for the given polygon using
//                      Newell's method.
//
// ----------------------------------------------------------------------- //

template<class P>
void g_GenerateNormal( P *pPoly )
{
	uint32		prev, cur;

	
	ASSERT( pPoly->NumVerts() >= 3 );

	pPoly->Normal().Init();
	prev = pPoly->NumVerts()-1;
	for( cur=0; cur < pPoly->NumVerts(); cur++ )
	{
		pPoly->Normal().x +=	(pPoly->Pt(prev).y - pPoly->Pt(cur).y) * 
								(pPoly->Pt(prev).z + pPoly->Pt(cur).z);
		
		pPoly->Normal().y +=	(pPoly->Pt(prev).z - pPoly->Pt(cur).z) * 
								(pPoly->Pt(prev).x + pPoly->Pt(cur).x);
		
		pPoly->Normal().z +=	(pPoly->Pt(prev).x - pPoly->Pt(cur).x) * 
								(pPoly->Pt(prev).y + pPoly->Pt(cur).y);
	
		prev = cur;
	}

	pPoly->Normal().Norm();
	pPoly->Dist() = pPoly->Normal().Dot( pPoly->Pt(0) );
}



template<class T, class F>
inline LTBOOL g_IsInsideTheTree( T *pRoot, TVector3<F> &pt )
{
	F		dot;

	while(1)
	{
		dot = pRoot->Normal().Dot(pt) - pRoot->Dist();

		if( dot > POINT_SIDE_EPSILON )
		{
			pRoot = pRoot->m_Sides[FrontSide];
			if( !pRoot )
				return LTTRUE;
		}
		else
		{
			pRoot = pRoot->m_Sides[BackSide];
			if( !pRoot )
				return LTFALSE;
		}				
	}
}


/*
template<class T, class F>
LTBOOL g_InsideConvex( T *pRoot, TVector3<F> &pt )
{
	uint32			i, nextI;
	_CPlane<F>		edgePlane;
	F				edgeDot;
	WorldPoly		*pPoly = pRoot->m_pPoly;
	LTVector vTemp;

	for( i=0; i < pPoly->m_nVertices; i++ )
	{
		nextI = (i+1) % pPoly->m_nVertices;

		vTemp = pPoly->m_Vertices[i]->m_Vec - pPoly->m_Vertices[nextI]->m_Vec;
		VEC_CROSS(edgePlane.m_Normal, pRoot->m_pPlane->m_Normal, vTemp);
		edgePlane.m_Normal /= edgePlane.m_Normal.Mag();
		edgePlane.m_Dist = edgePlane.m_Normal.Dot(pPoly->m_Vertices[i]->m_Vec);
		
		edgeDot = edgePlane.m_Normal.Dot(pt) - edgePlane.m_Dist;
		
		if( edgeDot < 0.0f )
			return LTFALSE;
	}

	return LTTRUE;
}
*/



template<class T, class F>
T* g_IntersectRay( T *pRoot, TVector3<F> &pt, TVector3<F> &dir, F &t, TVector3<F> &intersection, uint32 flags )
{
	F dot, dirDot, dot2;
	T *pRet;
	LTVector vTemp;


	dot = DIST_TO_PLANE(pt, *pRoot->m_pPlane);
	dirDot = pRoot->m_pPlane->m_Normal.Dot(dir);
	
	if( dot > POINT_SIDE_EPSILON )
	{
		// Front side.
		if( !(pRoot->m_Sides[FrontSide]->m_Flags & (NF_IN|NF_OUT)) )
		{
			if( (pRet = g_IntersectRay(pRoot->m_Sides[FrontSide], pt, dir, t, intersection, flags)) )
				return pRet;
		}
										  
		// Test the node's polygon.
		if( dirDot < ZERO )
		{
			VEC_ADD(vTemp, pt, dir);
			dot2 = DIST_TO_PLANE(vTemp, *pRoot->m_pPlane);

			t = -dot / (dot2 - dot);
			intersection = pt + (dir * t);

			if( g_InsideConvex(pRoot, intersection) )
				return pRoot;
/*
			if( !pRoot->m_Sides[BackSide] )
				return pRoot;
			else if( !g_IsInsideTheTree(pRoot->m_Sides[BackSide], intersection) )
				return pRoot;	
*/
		}
		
		// Back side.
		if( !(pRoot->m_Sides[BackSide]->m_Flags & (NF_IN|NF_OUT)) )
			if( dirDot <= ZERO )
				if( (pRet = g_IntersectRay(pRoot->m_Sides[BackSide], pt, dir, t, intersection, flags)) )
					return pRet;
	}
	else
	{
		// Back side.
		if( !(pRoot->m_Sides[BackSide]->m_Flags & (NF_IN|NF_OUT)) )
			if( (pRet = g_IntersectRay(pRoot->m_Sides[BackSide], pt, dir, t, intersection, flags)) )
				return pRet;

		// Front side.
		if( !(pRoot->m_Sides[FrontSide]->m_Flags & (NF_IN|NF_OUT)) )
			if( dirDot >= ZERO )
				if( (pRet = g_IntersectRay(pRoot->m_Sides[FrontSide], pt, dir, t, intersection, flags)) )
					return pRet;
	}

	return LTNULL;
}

#ifndef _MSC_VER
#define PVector LTVector
#define PReal float
#include "node.h"
#endif

// ----------------------------------------------------------------------- //
//      Routine:        g_LocatePointInTree
//      Purpose:        Finds where the given point is in the BSP tree.
//						Returns the node it's in if it's inside, or NODE_OUT if it's outside.
// ----------------------------------------------------------------------- //

template<class M, class F>
int g_LocatePointInTree( M *pWorld, TVector3<F> &point, int iRoot )
{
	F			dot;
	CNode		*pRoot;
	int			side;
	
	while(1)
	{
		pRoot = pWorld->GetNode( iRoot );
		dot = pRoot->Normal().Dot(point) - pRoot->Dist();

		side = (dot > 0.0f) ? FrontSide : BackSide;
		
		if( pRoot->m_Sides[side] >= 0 )
			iRoot = pRoot->m_Sides[side];
		else
			return (side == FrontSide) ? iRoot : (int)NODE_OUT;
	}
}

#undef NODE_IN
#undef NODE_OUT


// ----------------------------------------------------------------------- //
//
//      Routine:        g_DistToClosestEdge
//
//      Purpose:        Finds the distance to the closest edge in the polygon.
//
// ----------------------------------------------------------------------- //

template<class M, class P, class F>
inline F g_DistToClosestEdge( M *pModel, P *pRoot, TVector3<F> &pos )
{
	F				dot, minDist = (F)MAX_CREAL;
	TVector3<F>		normal;
	uint32			i;


	for( i=0; i < pRoot->m_Indices; i++ )
	{
		normal = pRoot->m_Plane.m_Normal.Cross( pRoot->NextPt(pModel, i) - pRoot->Pt(pModel, i) );
		normal.NormApprox();

		dot = normal.Dot( pos - pRoot->Pt(pModel,i) );
		dot = ABS(dot);
		if( dot < minDist )
			minDist = dot;
	}

	
	return minDist;
}

// FindClosestPointOnVector
//
// Finds the paramterization value to find the closest point on a vector given two points.
// The parameterized value T can be used with functions like VEC_LERP.
// If the point on the vector is outside the segment AB, then the parameter T will be less than
// zero or greater than 1.
//
// Given the following diagram:
//
//      a--------x-------b
//        .      |     .
//          .    |   .
//            .  | .
//               p
//
// The following equations are true:
//
// Vap^2 = Vpx^2 + Vax^2
// Vbp^2 = Vpx^2 + Vbx^2
// Vab   = Vax   + Vbx
//
// The following equations can be solved for:
//
// Vax = ( 1 / ( 2Vab )) * ( Vap^2 + Vab^2 - Vbp^2 )
// Vbx = ( 1 / ( 2Vab )) * ( Vbp^2 + Vab^2 - Vap^2 )
// Vpx^2 = Vap^2 - Vax^2
//
inline void FindClosestPointOnVector( LTVector *pvA, LTVector *pvB, LTVector *pvPt, float *pT )
{
	float fMagSqrAB, fMagSqrAP, fMagSqrBP;

	fMagSqrAB = VEC_DISTSQR( *pvB, *pvA );
	fMagSqrAP = VEC_DISTSQR( *pvPt, *pvA );
	fMagSqrBP = VEC_DISTSQR( *pvPt, *pvB );

	*pT = ( 0.5f / fMagSqrAB ) * ( fMagSqrAP + fMagSqrAB - fMagSqrBP );
}

#endif


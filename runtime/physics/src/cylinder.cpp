#include "cylinder.h"
#include "gjk.h"
#include "obb.h"


//---------------------------------------------------------------------------//
bool SphereCylinderClosestPoints
(
	LTVector3f&			a,
	LTVector3f&			b,
	const LTVector3f&	A,
	const float			ra,
	const LTCoordinateFrameQ& B,
	const float			rb,
	const float			hh
)
{
	//if A and B intersect, then a and b cannot be found
	//TEMP:  treat cylinder as an OBB for intersection
	const LTVector3f dim( rb, hh, rb );//x,y,z dims

		if( SphereOBBIntersect(A,ra,B,dim) )
			return false;

	//compute the closest points a and b
	LTSphereSupportMap Sa( A, ra );
	LTCylinderSupportMap Sb( B.m_O, B, rb, hh );

		GJK_ClosestPoints( a, b, Sa, Sb );

	return true;
}


//---------------------------------------------------------------------------//
bool SphereCylinderSweep
(
	float&				u,
	LTVector3f&			n,
	LTVector3f&			a,
	LTVector3f&			b,
	const LTVector3f&	A0,
	const LTVector3f&	A1,
	const float			ra,
	const LTCoordinateFrameQ& B0,
	const LTCoordinateFrameQ& B1,
	const float			rb,
	const float			hh
)
{
	//ALGORITHM:  Use a line-OBB test with expanded box dims.

	//if A and B do not intersect at t1,
	//then assume they didn't hit
	//TEMP:  treat cylinder as an OBB for intersection
	const LTVector3f dim( rb, hh, rb );//x,y,z dims

	if( SphereOBBSweep(u,n,A0,A1,ra,B0,B1,dim) )
	{
		const LTVector3f Ai = Lerp(A0,A1,u);
		const LTCoordinateFrameQ Bi = Interpolate(B0,B1,u);

		//calculate n using the closest points between A and B
		if( !SphereCylinderClosestPoints(a,b,Ai,ra,Bi,rb,hh) )
			return false;

		const LTVector3f v = a - b;//separating axis
		const float d = v.Length();//separating distance

		if( d != 0 )
			n =  (1/d) * v;
		else
			n.Init();//don't divide by 0

		return true;
	}

	return false;
}


//---------------------------------------------------------------------------//
bool CylinderBoxClosestPoints
(
	LTVector3f&			a,
	LTVector3f&			b,
	const LTCoordinateFrameQ& A,
	const float			r,
	const float			hh,
	const LTCoordinateFrameQ& B,
	const LTVector3f&	dim
)
{
	//if A and B intersect, then a and b cannot be found
	//TEMP:  treat cylinder as an OBB for intersection
	const LTVector3f da( r, hh, r );//x,y,z dims

		if( OBBIntersect(A,da,B,dim) )
			return false;

	//compute the closest points a and b
	LTCylinderSupportMap Sa( A.m_O, A, r, hh );
	LTBoxSupportMap Sb( B.m_O, B, dim );

		GJK_ClosestPoints( a, b, Sa, Sb );

	return true;
}


//---------------------------------------------------------------------------//
bool CylinderBoxSweep
(
	float&				u,
	LTVector3f&			n,
	LTVector3f&			a,
	LTVector3f&			b,
	const LTCoordinateFrameQ& A0,
	const LTCoordinateFrameQ& A1,
	const float			r,
	const float			hh,
	const LTCoordinateFrameQ& B0,
	const LTCoordinateFrameQ& B1,
	const LTVector3f&	dim
)
{
	//ALGORITHM:  If A and B intersect at time t1, find the closest points
	//at time t0.

	//if A and B do not intersect at t1,
	//then assume they didn't hit
	//TEMP:  treat cylinder as an OBB for intersection
	const LTVector3f da( r, hh, r );//x,y,z dims

	if( !OBBIntersect(A1,da,B1,dim) )
		return false;

	//find the closest points between A and B
	if( !CylinderBoxClosestPoints(a,b,A0,r,hh,B0,dim) )
		return false;

	u = 0;//TODO:  iterate a more accurate u

	const LTVector3f v = a - b;//separating axis
	const float d = v.Length();//separating distance

	if( d != 0 )
		n =  (1/d) * v;
	else
		n.Init();//don't divide by 0

	return true;
}


//---------------------------------------------------------------------------//
bool CylinderClosestPoints
(
	LTVector3f&			a,
	LTVector3f&			b,
	const LTCoordinateFrameQ& A,
	const float			ra,
	const float			ha,
	const LTCoordinateFrameQ& B,
	const float			rb,
	const float			hb
)
{
	//if A and B intersect, then a and b cannot be found
	//TEMP:  treat cylinder as an OBB for intersection
	const LTVector3f da( ra, ha, ra );//x,y,z dims
	const LTVector3f db( rb, hb, rb );//x,y,z dims

		if( OBBIntersect(A,da,B,db) )
			return false;

	//compute the closest points a and b
	LTCylinderSupportMap Sa( A.m_O, A, ra, ha );
	LTCylinderSupportMap Sb( B.m_O, B, rb, hb );

		GJK_ClosestPoints( a, b, Sa, Sb );

	return true;
}


//---------------------------------------------------------------------------//
bool CylinderSweep
(
	float&				u,
	LTVector3f&			n,
	LTVector3f&			a,
	LTVector3f&			b,
	const LTCoordinateFrameQ& A0,
	const LTCoordinateFrameQ& A1,
	const float			ra,
	const float			ha,
	const LTCoordinateFrameQ& B0,
	const LTCoordinateFrameQ& B1,
	const float			rb,
	const float			hb
)
{
	//ALGORITHM:  If A and B intersect at time t1, find the closest points
	//at time t0.

	//if A and B do not intersect at t1,
	//then assume they didn't hit
	//TEMP:  treat cylinder as an OBB for intersection
	const LTVector3f da( ra, ha, ra );//x,y,z dims
	const LTVector3f db( rb, hb, rb );//x,y,z dims

	if( !OBBIntersect(A1,da,B1,db) )
		return false;

	//find the closest points between A and B
	if( !CylinderClosestPoints(a,b,A0,ra,ha,B0,rb,hb) )
		return false;

	u = 0;//TODO:  iterate a more accurate u

	const LTVector3f v = a - b;//separating axis
	const float d = v.Length();//separating distance

	if( d != 0 )
		n =  (1/d) * v;
	else
		n.Init();//don't divide by 0

	return true;
}


//EOF

#include "obb.h"
#include "aabb.h"
#include "sphere.h"
#include "gjk.h"


//---------------------------------------------------------------------------//
bool SphereOBBSweep
(
	float&				u,	//normalized time of collision
	LTVector3f&			n,	//unit contact normal
	const LTVector3f&	C0,	//first position of sphere
	const LTVector3f&	C1,	//second position of sphere
	const float			r,	//radius of sphere
	const LTCoordinateFrameQ&	B0,	//first position/orientation of OBB
	const LTCoordinateFrameQ&	B1,	//second position/orientation of OBB
	const LTVector3f&	d	//half-dimensions of OBB
)
{
	//ALGORITHM:  transform the sphere into the OBB's local frame,
	//then cast a ray at an expanded AABB to find u, the normalized
	//time of collision at which the collision occured.  If r!=0,
	//compute n by normalizing their separating axis.

	//if the sphere and the OBB intersect at u=0, return false
	if( SphereOBBIntersect( C0, r, B0, d ) )
		return false;

	//relative motion is linearly approximated by transforming
	//C0 to B0's frame and C1 to B1's frame, because relative
	//motion in B's frame is Rb^T*(vc-vb) = Rb^T*(C1-B1) - Rb^T*(C0-B0)
	const LTVector3f c0 = B0.TransformPointToLocal(C0);
	const LTVector3f c1 = B1.TransformPointToLocal(C1);
	//expand box extents by r (conservative estimate)
	const LTVector3f dim( d.x + r, d.y + r, d.z + r );

	//treat the swept sphere as a ray (in the
	//box's frame) cast at the expanded box
	if( CastRayAtAABB(u,n,c0,c1,-dim,dim) )
	{
		//if r=0, then the 'n' obtained above is correct,
		//otherwise sphere might be next to an edge or a
		//corner of the box
		if( 0!=r )
		{
			//use the normalized time of collision 'u' to compute
			//the position of the sphere when it hit the box, and
			//the correct contact normal 'n'
			const LTVector3f c = Lerp( c0, c1, u );

			//clear components, set them below
			n.Init();

			//modification of "Graphics Gems" pp. 336
			for( int32 i=0 ; i<3 ; i++ )
			{
				//extents along ith axis
				const float min = -d[i];
				const float max = d[i];

				//compute the separating axis
				//pointing from box -> sphere
				if( c[i] < min )
					n[i] = c[i] - min;
				else if( c[i] > max )
					n[i] = c[i] - max;
			}

			//unit contact normal
			n.Normalize();//if r!=0, then n!=(0,0,0)
		}

		//interpolate box orientation at u
		const LTCoordinateFrameQ Bi = Interpolate( B0, B1, u );

		//transform n back to the world frame
		n = Bi.TransformVectorToParent( n );

		return true;
	}

	return false;
}


//---------------------------------------------------------------------------//
bool SphereOBBIntersect
(
	const LTVector3f&			C,
	const float					r,
	const LTCoordinateFrameQ&	F,
	const LTVector3f&			d
)
{
	//transform sphere center to box's local frame
	const LTVector3f c = F.TransformPointToLocal( C );
	float d2 = 0;//sqr of dist from sphere to box

	//ALGORITHM:  "Graphics Gems,"  pp. 336
	for( int32 i=0 ; i<3 ; i++ )
	{
		//extents along ith axis
		const float min = -d[i];
		const float max = d[i];

		if( c[i] < min )
		{
			const float s = c[i]-min;
			d2 += s*s;
		}
		else if( c[i] > max )
		{
			const float s = c[i]-max;
			d2 += s*s;
		}
	}

	return d2 < r*r;
}


//---------------------------------------------------------------------------//
bool SphereOBBIntersect
(
	LTVector3f&					T,
	const LTVector3f&			C,
	const float					r,
	const LTCoordinateFrameQ&	F,
	const LTVector3f&			d
)
{
	//transform sphere center to box's local frame
	const LTVector3f c = F.TransformPointToLocal( C );
	float d2 = 0;//sqr of dist from sphere to box

	//ALGORITHM:  "Graphics Gems,"  pp. 336
	for( int32 i=0 ; i<3 ; i++ )
	{
		//extents along ith axis
		const float min = -d[i];
		const float max = d[i];

		if( c[i] < min )
		{
			const float s = c[i]-min;
			d2 += s*s;
		}
		else if( c[i] > max )
		{
			const float s = c[i]-max;
			d2 += s*s;
		}
	}

	//TODO:  find a better estimate of penetration
	LTVector3f n = c - F.m_O;//from box->sphere

	if( n.LengthSqr()>0 )
		n.Normalize();
	else
		n = LTVector3f(0,0,1);//arbitrary

	T = (r - sqrtf(d2)) * n;
	//transform back to world space
	T = F.TransformPointToParent( T );

	return d2 < r*r;
}


//---------------------------------------------------------------------------//
bool OBBIntersect
(
	const LTCoordinateFrameQ&	Fa,
	const LTVector3f&			a,
	const LTCoordinateFrameQ&	Fb,
	const LTVector3f&			b
)
{
	//B's position and orientation in A's frame
	const LTCoordinateFrameQ Fba = Fb.TransformToLocal( Fa );
	const LTVector3f T = Fba.m_O;
	const LTMatrix3f R = Fba.m_Q;
	float ra, rb, t;

	//ALGORITHM:  Use the separating axis test for all
	//15 potential separating axes.  If a separating
	//axis could not be found, the two boxes overlap.

	//NOTE:  The intervals are closed, so the algorithm
	//returns true even if the boxes are just barely
	//touching.

	//A's basis vectors
	for( int32 i=0 ; i<3 ; i++ )
	{
		ra = a[i];
		rb = b[0]*fabsf(R(i,0)) + b[1]*fabsf(R(i,1)) + b[2]*fabsf(R(i,2));
		t = fabsf( T[i] );

		if( t > ra + rb )
			return false;
	}

	//B's basis vectors
	for( int32 k=0 ; k<3 ; k++ )
	{
		ra = a[0]*fabsf(R(0,k)) + a[1]*fabsf(R(1,k)) + a[2]*fabsf(R(2,k));
		rb = b[k];
		t = fabsf( T[0]*R(0,k) + T[1]*R(1,k) + T[2]*R(2,k) );

		if( t > ra + rb )
			return false;
	}

	//9 cross products
	//L = A0 x B0
	ra = a[1]*fabsf(R(2,0)) + a[2]*fabsf(R(1,0));
	rb = b[1]*fabsf(R(0,2)) + b[2]*fabsf(R(0,1));
	t = fabsf( T.z*R(1,0) - T.y*R(2,0) );

	if( t > ra + rb )
		return false;

	//L = A0 x B1
	ra = a[1]*fabsf(R(2,1)) + a[2]*fabsf(R(1,1));
	rb = b[0]*fabsf(R(0,2)) + b[2]*fabsf(R(0,0));
	t = fabsf( T.z*R(1,1) - T.y*R(2,1) );

	if( t > ra + rb )
		return false;

	//L = A0 x B2
	ra = a[1]*fabsf(R(2,2)) + a[2]*fabsf(R(1,2));
	rb = b[0]*fabsf(R(0,1)) + b[1]*fabsf(R(0,0));
	t = fabsf( T.z*R(1,2) - T.y*R(2,2) );

	if( t > ra + rb )
		return false;

	//L = A1 x B0
	ra = a[0]*fabsf(R(2,0)) + a[2]*fabsf(R(0,0));
	rb = b[1]*fabsf(R(1,2)) + b[2]*fabsf(R(1,1));
	t = fabsf( T.x*R(2,0) - T.z*R(0,0) );

	if( t > ra + rb )
		return false;

	//L = A1 x B1
	ra = a[0]*fabsf(R(2,1)) + a[2]*fabsf(R(0,1));
	rb = b[0]*fabsf(R(1,2)) + b[2]*fabsf(R(1,0));
	t = fabsf( T.x*R(2,1) - T.z*R(0,1) );

	if( t > ra + rb )
		return false;

	//L = A1 x B2
	ra = a[0]*fabsf(R(2,2)) + a[2]*fabsf(R(0,2));
	rb = b[0]*fabsf(R(1,1)) + b[1]*fabsf(R(1,0));
	t = fabsf( T.x*R(2,2) - T.z*R(0,2) );

	if( t > ra + rb )
		return false;

	//L = A2 x B0
	ra = a[0]*fabsf(R(1,0)) + a[1]*fabsf(R(0,0));
	rb = b[1]*fabsf(R(2,2)) + b[2]*fabsf(R(2,1));
	t = fabsf( T.y*R(0,0) - T.x*R(1,0) );

	if( t > ra + rb )
		return false;

	//L = A2 x B1
	ra = a[0]*fabsf(R(1,1)) + a[1]*fabsf(R(0,1));
	rb = b[0]*fabsf(R(2,2)) + b[2]*fabsf(R(2,0));
	t = fabsf( T.y*R(0,1) - T.x*R(1,1) );

	if( t > ra + rb )
		return false;

	//L = A2 x B2
	ra = a[0]*fabsf(R(1,2)) + a[1]*fabsf(R(0,2));
	rb = b[0]*fabsf(R(2,1)) + b[1]*fabsf(R(2,0));
	t = fabsf( T.y*R(0,2) - T.x*R(1,2) );

	if( t > ra + rb )
		return false;

	//no separating axis found,
	//so the two boxes overlap
	return true;
}


//---------------------------------------------------------------------------//
bool OBBIntersect
(
	LTVector3f&					T,
	const LTCoordinateFrameQ&	Fa,
	const LTVector3f&			a,
	const LTCoordinateFrameQ&	Fb,
	const LTVector3f&			b
)
{
	//Use separating axis test
	if( OBBIntersect( Fa, a, Fb, b ) )
	{
		//TODO:  Use GJK to find T, minimum translational distance
		const float amax = Max( a.x,Max(a.y,a.z));
		const float bmax = Max( b.x,Max(b.y,b.z));
		//very conservative penetration depth estimate
		const float d = Max( amax, bmax );
		LTVector3f n = Fa.Position() - Fb.Position();

		if( n.LengthSqr() )
			n.Normalize();
		else
			n = LTVector3f(0,0,1);//ambiguous

		//penetration vector
		T = d * n;

		return true;
	}

	return false;
}


//---------------------------------------------------------------------------//
bool OBBClosestPoints
(
	LTVector3f&					a,
	LTVector3f&					b,
	const LTCoordinateFrameQ&	A,
	const LTVector3f&			da,
	const LTCoordinateFrameQ&	B,
	const LTVector3f&			db
)
{
		//if A and B intersect, then a and b cannot be found
		if( OBBIntersect(A,da,B,db) )
			return false;

	//compute the closest points a and b
	LTBoxSupportMap Sa( A.m_O, A, da );
	LTBoxSupportMap Sb( B.m_O, B, db );

		GJK_ClosestPoints( a, b, Sa, Sb );

	return true;
}


//---------------------------------------------------------------------------//
bool OBBSweep
(
	float&						u,	//normalized time of collision
	LTVector3f&					n,	//unit contact normal
	LTVector3f&					a,	//closest point on A to B, world frame
	LTVector3f&					b,	//closest point on B to A, world frame
	const LTCoordinateFrameQ&	A0,	//first position/orientation of A
	const LTCoordinateFrameQ&	A1,	//second position/orientation of A
	const LTVector3f&			da,	//half-dimensions of A
	const LTCoordinateFrameQ&	B0,	//first position/orientation of B
	const LTCoordinateFrameQ&	B1,	//second position/orientation of B
	const LTVector3f&			db	//half-dimensions of B
)
{
	//ALGORITHM:  If A and B intersect at time t1, find the closest points
	//at time t0.

	//if A and B do not intersect at t1,
	//then assume they didn't hit
	//TODO:  swept volume test
	if( !OBBIntersect(A1,da,B1,db) )
		return false;

	//find the closest points between A and B
	if( !OBBClosestPoints(a,b,A0,da,B0,db) )
		return false;

	u = 0;//for now, this is always true

	const LTVector3f v = a - b;//separating axis
	const float d = v.Length();//separating distance

	if( d != 0 )
		n =  (1/d) * v;
	else
		n.Init();//don't divide by 0

	return true;
}


//EOF

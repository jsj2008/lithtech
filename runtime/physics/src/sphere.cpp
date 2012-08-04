#include "sphere.h"
#include "triangle.h"
#include <assert.h>


//---------------------------------------------------------------------------//
bool SphereSphereSweep
(
	float&				u,	//normalized time of collision
	const LTVector3f&	A0,	//first position of A
	const LTVector3f&	A1,	//second position of A
	const float			ra,	//radius of A
	const LTVector3f&	B0,	//first position B
	const LTVector3f&	B1,	//second position B
	const float			rb	//radius of B
)
{
	//Solve to see if they came within (ra + rb) distance of one another
	const LTVector3f	va = A1 - A0;//from A0 to A1
	const LTVector3f	vb = B1 - B0;
	const LTVector3f	vab = va - vb;
	const LTVector3f	ab = A0 - B0;
	const float		d = ra + rb;
	//quadratic coefficients
	const float a = vab.Dot(vab);//|va-vb|^2
	const float b = 2 * ab.Dot(vab);
	const float c = ab.Dot(ab) - d*d;//|A0-B0|^2 - (ra+rb)^2
	//quadratic solutions
	float u1, u2;

		//if solutions are real, then a collision occurred
		if( QuadraticFormula( u1, u2, a, b, c ) )
		{
			//first time of collision
			u = Min( u1, u2 );

			//must be in the interval [0,1]
			return 0<=u && u<=1;
		}
		else
		{
			return false;
		}
}


//---------------------------------------------------------------------------//
bool SphereTriangleSweep
(
	float&				uc,		//normalized time of collision
	LTVector3f&			nc,		//normal of the collision plane
	const LTVector3f&	C0,		//first sphere position
	const LTVector3f&	C1,		//second sphere position
	const float			r,		//sphere radius
	const LTVector3f&	v0,		//triangle vertices
	const LTVector3f&	v1,		//triangle vertices
	const LTVector3f&	v2		//triangle vertices
)
{
	//ALGORITHM:  The algorithm is has three general steps:
	//1)  Did the sphere hit the face in the plane?
	//2)  If not, which edge did it hit first?
	//3)  If not, which vertex did it hit first?
	const int32 count = 3;
	LTVector3f pv[count] = {v0,v1,v2};
	LTVector3f n = (pv[1] - pv[0]).Cross(pv[2] - pv[0]).Unit();//Unit plane normal
	const float d0 = n.Dot(C0 - pv[0]);//distances from C0 and C1
	const float d1 = n.Dot(C1 - pv[0]);//to the polygon's plane
	bool bInside = false;//until proven otherwise

		//only consider face collisions from the + half-space
		if( d0>=r && d1<r )
		{
			LTVector3f Cu;
			float ua=0, ub=1;
			int32 iter=23;
			float u, d;

			//use bisection to stably iterate u to machine precision
			do
			{
				u = 0.5f*(ua + ub);//halfway
				Cu = (1-u)*C0 + u*C1;//interpolate
				//dist from plane to Cu
				d = n.Dot(Cu - pv[0]);
				if( d>r )
					ua=u;//upper bound
				else if( d<r )
					ub=u;//lower bound
				else
					break;//perfect!
			}
			while( iter-- );

			//if last d < r, then use ua
			if( d<r )
			{
				u = ua;//garaunteed d>=r
				Cu = (1-u)*C0 + u*C1;//interpolate
			}

			const LTVector3f Pc = Cu - r*n;//point of contact
			const LTVector3f* a = &pv[count-1];//first vertex on edge

				if( bInside = PointInTriangle( Pc, v0, v1, v2 ) )
				{
					//a collision with the face in the plane
					//will occur before any edge or vertex
					//collision, so it's ok to exit early
					uc = u;
					nc = n;
					return true;
				}
		}

		//*
		//TODO:  Something in code below causes
		//collisions with flat surfaces to mess up.
		//Could be same numerical problems solved
		//above with iterative algorithm.
		//(repeat with r = 1/32)

		//check for edge and vertex collisions if the
		//above test failed, or if the center of the
		//sphere started off on the plane's positive
		//side
		if( (0<=d0 ) && !bInside  && fabsf(r)>FLT_EPSILON )
		{
			const LTVector3f* a = &pv[count-1];
			bool bHit = false;

			uc = 1;//replace on first iteration

			//check each edge and vertex
			for( long i=0 ; i<count ; i++ )
			{
				const LTVector3f* b = &pv[i];
				float u;

				//edge
				if( SphereSegmentSweep( u, n, C0, C1, r, *a, *b ) )
				{
					//replace this one if it occurred
					//before the last one
					if( u < uc )
					{
						uc = u;
						nc = n;
						bHit = true;
					}
				}

				//vertex
				if( SpherePointSweep( u, n, C0, C1, r, *a ) )
				{
					if( u < uc )
					{
						uc = u;
						nc = n;
						bHit = true;
					}
				}

				a = b;//for the next edge
			}

			if( bHit )
				return true;
		}
		//*/

	return false;
}


//---------------------------------------------------------------------------//
bool SphereSegmentSweep
(
	float&				u,	//normalized time of collision
	LTVector3f&			n,	//normal of the collision plane
	const LTVector3f&	C0,	//first sphere position
	const LTVector3f&	C1,	//second sphere position
	const float			r,	//sphere radius
	const LTVector3f&	E0,	//first edge vertex
	const LTVector3f&	E1	//second edge vertex
)
{
	//calculate first point on linear trajectory
	//such that distance from edge to pi is radius
	const LTVector3f v = C1 - C0;//displacement
	const LTVector3f edge = E1 - E0;//vector from E0 to E1
	const float e_length = edge.Length();//Length of edge
	const LTVector3f e = edge * (1 / e_length);//Unit edge direction
	const LTVector3f d = C0 - E0;
	const float ve = v.Dot(e);
	const float de = d.Dot(e);
	const float dv = d.Dot(v);
	//quadratic coefficients
	const float a = v.Dot(v) - ve * ve;
	const float b = 2 * (dv - de*ve);
	const float c = d.Dot(d) - de*de - r*r;
	float u1, u2;

		//check if the trajectory comes close enough
		if( QuadraticFormula( u1, u2, a, b, c ) )
		{
			//don't consider "glancing" collisions,
			//since they don't affect trajectory
			if( u1 != u2 )
			{
				u = Min( u1, u2 );//choose the lesser solution

				//did the collision occur between C0 and C1?
				if( 0<=u && u<=1 )
				{
					//projection of C(u=C0+u*v along edge
					const float proj = de + u*ve;

					//was the point of contact between E0 and E1?
					if( (0<=proj) && (proj<=e_length) )
					{
						//vector from E0 to C(u)
						const LTVector3f EC = d + u*v;

						n = EC - EC.Dot(e)*e;//perp. vector from edge to Ci
						n.Normalize();

						return true;
					}
				}
			}
		}

	return false;
}


//---------------------------------------------------------------------------//
bool SpherePointSweep
(
	float&				u,	//normalized time of collision
	LTVector3f&			n,	//normal of the collision plane
	const LTVector3f&	C0,	//first sphere position
	const LTVector3f&	C1,	//second sphere position
	const float			r,	//sphere radius
	const LTVector3f&	V0	//vertex
)
{
	//calculate when the sphere first hits the vertex,
	//this is when (pi - A).mag() == radius

	const LTVector3f v = C1 - C0;//displacement
	const LTVector3f d = C0 - V0;
	//quadratic coefficients
	const float a = v.Dot(v);
	const float b = 2 * d.Dot(v);
	const float c = d.Dot(d) - r*r;
	float u1, u2;

		//check if the trajectory comes close enough
		if( QuadraticFormula( u1, u2, a, b, c ) )
		{
			//don't consider "glancing" collisions,
			//since they don't affect trajectory
			if( u1 != u2 )
			{
				//take the lesser solution
				u = Min( u1, u2 );

				//did the collision occur between C0 and C1, inclusive?
				if( 0<=u && u<=1 )
				{
					//the collision normal points from V0 to C(u)
					n = d + u*v;//n = C(u) - V0 = C0 - V0 + u*v
					n.Normalize();

					return true;
				}
			}
		}

	return false;
}


//---------------------------------------------------------------------------//
bool SphereTriangleIntersect
(
	LTVector3f&			T,
	const LTVector3f&	C,
	const float			r,
	const LTVector3f&	v0,
	const LTVector3f&	v1,
	const LTVector3f&	v2
)
{
	//ALGORITHM:  The algorithm is has three general steps:
	//1)  Did the sphere hit the face in the plane?
	//2)  If not, which edge did it hit first?
	//3)  If not, which vertex did it hit first?
	LTVector3f n = (v1 - v0).Cross(v2 - v0).Unit();//unit plane normal
	const float d = n.Dot(C - v0);//distance from C to triangle's plane

	if( fabsf(d)<=r )//sphere intersects plane
	{
		const float r2 = r*r;//radius squared

		//case 1:  sphere contains triangle
		if( C.DistSqr(v0)<=r2 && C.DistSqr(v1)<=r2 && C.DistSqr(v0)<=r2 )
		{
			//translate sphere into triangle's (+) half-space
			T = (r-d)*n;
			return true;
		}

		//case 2:  sphere intersects interior of triangle
		//NOTE:  no need to project C onto plane, since
		//any perturbation to a point C along n gives same
		//sign:
		//n.Dot(r x (p+c*n)) = n.Dot(r x p)
		if( PointInTriangle( C, v0, v1, v2 ) )
		{
			//translate sphere into triangle's (+) half-space
			T = (r-d)*n;
			return true;
		}

		//case 3: sphere intersects an edge
		if( SphereSegmentIntersect(T,C,r,v0,v1) )
			return true;
		if( SphereSegmentIntersect(T,C,r,v1,v2)	)
			return true;
		if( SphereSegmentIntersect(T,C,r,v2,v0) )
			return true;
	}

	//no intersection
	return false;
}


//---------------------------------------------------------------------------//
bool SphereSegmentIntersect
(
	LTVector3f&			T,
	const LTVector3f&	C,
	const float			r,
	const LTVector3f&	E0,
	const LTVector3f&	E1
)
{
	//ALGORITHM:  Find the point pc on the line segment
	//closest to C.  If ||C - pc|| <= r, then find the
	//penetration vector T.

	const float r2 = r*r;
	//segment length
	const float le = (E1-E0).Length();

		assert( le!=0 );//degenerate edge -> bad data

	//unit segment direction
	const LTVector3f e = (1/le)*(E1-E0);
	//scalar projection along e
	const float de = e.Dot(C-E0);
	//find point on edge closest to C
	LTVector3f pc;

		if( de <= 0 )
			pc = E0;//left end
		else if( le <= de )
			pc = E1;//right end
		else
			pc = E0 + de*e;//between E0 and E1

	//vector from pc to C
	const LTVector3f v = C - pc;
	//distance from pc to C
	const float d = v.Length();

		if( d<r )//point is in sphere
		{
			if( d!=0 )//don't divide by 0
				//(r-d) = distance pc has penetrated sphere
				T = (r-d)/d * v;//normalize v with /d
			else
				T.Init(0,0,0);

			return true;//intersection
		}

	return false;//no intersection
}


//EOF

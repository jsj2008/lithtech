#include "aabb.h"


//---------------------------------------------------------------------------//
bool AABBSegmentIntersect
(
	const LTVector3f& min,//Extents
	const LTVector3f& max,
	const LTVector3f& l0,	//line segment
	const LTVector3f& l1
)
{
	//ALGORITHM:  Use the separating axis theorem to
	//see if the line segment and the box intersect. A
	//line segment is a degenerate OBB.
	//NOTE:  the multiplies by 0.5 cancel out

	const LTVector3f l = l1 - l0;//unnormalized direction
	const float fx = fabsf(l.x);
	const float fy = fabsf(l.y);
	const float fz = fabsf(l.z);
	const LTVector3f T = (l0 + l1) - (max + min);//translation
	const LTVector3f E = max - min;//Extents

		//NOTE:  dropping out early with else/if
		//statements doesn't seem to speed up
		//this routine at all

		//do any of the principal axes form a separating axis?
		if( (fabsf(T.x) > fx + E.x)
			||
			(fabsf(T.y) > fy + E.y)
			||
			(fabsf(T.z) > fz + E.z) )
		{
			return false;
		}

		//do l.Cross(x), l.Cross(y), or l.Cross(z)
		//form a separating axis?
		if( (fabsf(T.y*l.z - T.z*l.y) > E.y*fz + E.z*fy)
			||
			(fabsf(T.z*l.x - T.x*l.z) > E.x*fz + E.z*fx)
			||
			(fabsf(T.x*l.y - T.y*l.x) > E.x*fy + E.y*fx)
			)
		{
			return false;
		}

	return true;
}


//---------------------------------------------------------------------------//
bool AABBSegmentIntersect
(
	LTVector3f			p[],//0, 1 or 2 intersection points
	int32&				n,
	const LTVector3f&	min,//Extents
	const LTVector3f&	max,
	const LTVector3f&	l0,	//line segment
	const LTVector3f&	l1
)
{
	//if the line segment intersects the box,
	//calculate the points of intersection by
	//succesively clipping against the faces
	if( AABBSegmentIntersect(min,max,l0,l1) )
	{
		const LTAABB box( min, max );
		const bool contains_l0 = box.Contains(l0);
		const bool contains_l1 = box.Contains(l1);
		LTVector3f a=l0, b=l1;//computed intersection points

		//if the box contains both l0 and l1, then
		//intersection points are not calculated
		if( !(contains_l0 && contains_l1) )
		{
			//1 or 2 points are outside box:
			//clip line segment against faces
			for( int32 i=0 ; i<3 ; i++ )
			{
				//min face
				{
					//signed distances to face (+ is outside)
					const float da = min[i] - a[i];
					const float db = min[i] - b[i];

					//a and b are on opposite sides of face
					if( da*db <= 0 )
					{
						const float u = da / (da - db);
						const LTVector3f ip = Lerp(a,b,u);

						if( da>0 )//a is outside box
							a = ip;
						else
							b = ip;
					}
				}

				//max face
				{
					//signed distances to face (+ is outside)
					const float da = a[i] - max[i];
					const float db = b[i] - max[i];

					//a and b are on opposite sides of face
					if( da*db <= 0 )
					{
						const float u = da / (da - db);
						const LTVector3f ip = Lerp(a,b,u);

						if( da>0 )//a is outside box
							a = ip;
						else
							b = ip;
					}
				}
			}

			//are there 1 or 2 intersection points?
			if( !contains_l0 && !contains_l1 )//l0 and l1 outside box
			{
				//two intersection points
				p[0] = a;
				p[1] = b;
				n = 2;
			}
			else//one point was interior
			{
				//only one intersection point
				n = 1;

				if( contains_l0 )
					p[0] = a;
				else
					p[0] = b;
			}

			return true;//intersection
		}
	}

	return false;//no intersection
}


//---------------------------------------------------------------------------//
bool CastRayAtAABB
(
	float&		u,
	LTVector3f&	n,
	const LTVector3f& p0,
	const LTVector3f& p1,
	const LTVector3f& min,
	const LTVector3f& max
)
{
	//ALGORITHM:  If p0 is on a face, then return true with u=0 and an
	//appropriate n.  Otherwise, find when the line segment first intersects
	//a face:  if p0 is inside the box, then look for u_min; if p0 is outside
	//look for u_max.  Calculate the appropriate n in the process.
	LTAABB b( min, max );

		u = 2;//invalid time
		n.Init(0,0,0);//determined below

		//if both ends of the segment intersect the box
		//and p0 is on one of the faces, return true
		//with u=0 and n set appropriately
		if( b.Contains(p0) && b.Contains(p1) )
		{
			u=0;

			//p0
			for( int32 i=0 ; i<3 ; i++ )
			{
				if( p0[i] == min[i] )
				{
					n[i] = -1;
					return true;
				}
				else if( p0[i] == max[i] )
				{
					n[i] = 1;
					return true;
				}
			}

			u=1;

			//p1
			for( int32 i=0 ; i<3 ; i++ )
			{
				if( p1[i] == min[i] )
				{
					n[i] = -1;
					return true;
				}
				else if( p1[i] == max[i] )
				{
					n[i] = 1;
					return true;
				}
			}

			//not on boundary and doesn't intersect boundary
			return false;
		}

		//if segment is degenerate, return false at this point
		if( p0==p1 )
			return false;

		//if the line segment does not intersect
		//the box, return false
		if( !b.Intersects( p0, p1 ) )//separating axis test
			return false;

	//if p0 is inside the box, then the point of intersection is given by
	//u_min (first point of exit).  Otherwise, point of intersection is given
	//by u_max (first point of entrance).
	const LTVector3f v = p1-p0;//ray displacement, won't be (0,0,0), see above

		if( b.Contains( p0 ) )//p0 inside box
		{
			u = 1;

			for( int32 i=0 ; i<3 ; i++ )
			{
				if( p0[i]<max[i] && v[i]>0 )
				{
					const float u0 = (max[i] - p0[i]) / v[i];//u0 >0

					if( u0 <= u )//min u
					{
						u = u0;//normalized time of collision
						n[i] = 1;//n is the ith axis direction
						//set other components of n to 0
						n[(i+1)%3] = 0;
						n[(i+2)%3] = 0;
					}
				}
				else if( p0[i]>min[i] && v[i]<0 )
				{
					const float u0 = (min[i] - p0[i]) / v[i];//u0 >0

					if( u0 <= u )//min u
					{
						u = u0;//normalized time of collision
						n[i] = -1;//n is the ith axis direction
						//set other components of n to 0
						n[(i+1)%3] = 0;
						n[(i+2)%3] = 0;
					}
				}
			}
		}
		else//p0 outside
		{
			u = 0;

			for( int32 i=0 ; i<3 ; i++ )
			{
				if( p0[i]>max[i] && v[i]<0 )
				{
					const float u0 = (max[i] - p0[i]) / v[i];//u0 >0

					if( u0 >= u )//max u
					{
						u = u0;//normalized time of collision
						n[i] = 1;//n is the ith axis direction
						//set other components of n to 0
						n[(i+1)%3] = 0;
						n[(i+2)%3] = 0;
					}
				}
				else if( p0[i]<min[i] && v[i]>0 )
				{
					const float u0 = (min[i] - p0[i]) / v[i];//u0 >0

					if( u0 >= u )//max u
					{
						u = u0;//normalized time of collision
						n[i] = -1;//n is the ith axis direction
						//set other components of n to 0
						n[(i+1)%3] = 0;
						n[(i+2)%3] = 0;
					}
				}
			}
		}

	return true;
}


//EOF

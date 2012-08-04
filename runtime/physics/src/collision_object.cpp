#include "collision_object.h"
#include "sphere.h"
#include "obb.h"
#include "cylinder.h"


//---------------------------------------------------------------------------//
inline void SwapContactInfo( LTContactInfo& ci, const ILTCollisionObject& o )
{
	//make sure object handle, normal and
	//surfaces correspond to correct object
	if( ci.m_hObj != o.m_hObj )
	{
		ci.m_hObj = o.m_hObj;	//B's object handle
		ci.m_N = -ci.m_N;		//normal points from B->A
		Swap(ci.m_Sa,ci.m_Sb);	//swap surface info
	}
}


//---------------------------------------------------------------------------//
inline void SwapIntersectInfo( LTIntersectInfo& ii, const ILTCollisionObject& o )
{
	//make sure normal and surfaces
	//correspond to correct object
	if( ii.m_hObj != o.m_hObj )
	{
		ii.m_hObj = o.m_hObj;	//B's object handle
		ii.m_T = -ii.m_T;		//translation vector points from B->A
	}
}


//---------------------------------------------------------------------------//
bool LTCollisionSphere::Hit
(
	LTContactInfo& ci, const ILTCollisionObject& o,
	const LTContactInfo::Filter& cf
) const
{
	const bool bHit = o.HitSphere( ci, *this, cf );

	//swap contact info if necessary
	SwapContactInfo(ci,o);

	return bHit;
}

//---------------------------------------------------------------------------//
bool LTCollisionSphere::HitSphere
(
	LTContactInfo& ci, const LTCollisionSphere& s,
	const LTContactInfo::Filter& cf
) const
{
	float u;

	//check for collision
	if(SphereSphereSweep(u, m_P0, m_P1, m_Radius, s.m_P0, s.m_P1, s.m_Radius))
	{
		//centers of A and B at the time of contact, parent frame
		const LTVector3f Au = (1-u)*m_P0 + u*m_P1;
		const LTVector3f Bu = (1-u)*s.m_P0 + u*s.m_P1;
		//unit contact normal (points from B to A), parent frame
		const LTVector3f n = (Au - Bu).Unit();

		//contact information
		ci.m_hObj = s.m_hObj;//object handle
		ci.m_U	= u;//normalized time of collision, [0,1]
		ci.m_N	= n;//unit contact normal
		ci.m_P	= Au - m_Radius*n;//point of contact
		ci.m_Sa	= this->m_Surf;//surface properties
		ci.m_Sb	= s.m_Surf;//surface properties

		//apply filter
		if( cf.Condition( ci ) )
			return true;
	}

	//no collision
	return false;
}

//---------------------------------------------------------------------------//
bool LTCollisionSphere::HitBox
(
	LTContactInfo& ci, const LTCollisionBox& b,
	const LTContactInfo::Filter& cf
) const
{
	float u;//normalized time of collision
	LTVector3f n;//unit contact normal
	const float r = m_Radius;
	const LTCoordinateFrameQ B0( b.m_P0, b.m_R0 );
	const LTCoordinateFrameQ B1( b.m_P1, b.m_R1 );
	const LTVector3f& dim = b.m_Dim;//half-dimensions

	if( SphereOBBSweep( u, n, m_P0, m_P1, r, B0, B1, dim ) )
	{
		ci.m_hObj = b.m_hObj;//object handle
		ci.m_U = u;
		ci.m_N = n;
		ci.m_P = Lerp(m_P0,m_P1,u) - r*n;
		ci.m_Sa = this->m_Surf;
		ci.m_Sb = b.m_Surf;

		//apply filter
		if( cf.Condition( ci ) )
			return true;
	}

	//no collision
	return false;
}

//---------------------------------------------------------------------------//
bool LTCollisionSphere::HitCylinder
(
	LTContactInfo& ci, const LTCollisionCylinder& c,
	const LTContactInfo::Filter& cf
) const
{
	const bool bHit = c.HitSphere( ci, *this, cf );

	//swap contact info if necessary
	SwapContactInfo(ci,c);

	return bHit;
}

//---------------------------------------------------------------------------//
bool LTCollisionSphere::HitMesh
(
	LTContactInfo& ci, const LTCollisionMesh& mesh,
	const LTContactInfo::Filter& cf
) const
{
	const float r = this->m_Radius;
	const LTVector3f d(r,r,r);//use an aabb to approximate the sphere
	//transform sphere to mesh's local frame
	const LTCoordinateFrameQ F0( mesh.m_P0, mesh.m_R0 );
	const LTCoordinateFrameQ F1( mesh.m_P1, mesh.m_R1 );
	//C1-C0 provides a 1st order approximation to the curvilinear
	//trajectory traversed by the sphere in the mesh's local frame
	const LTVector3f C0 = F0.TransformPointToLocal( m_P0 );
	const LTVector3f C1 = F1.TransformPointToLocal( m_P1 );
	//mesh extents
	const LTVector3f& min = mesh.m_pData->m_Min;
	const LTVector3f& max = mesh.m_pData->m_Max;
	//aabb node array
	const LTAABB_Node* node = mesh.m_pData->Nodes();
	//TODO:  use a dynamic array
	const uint16 tc_max = 128;//max number of triangle indices
	uint16 ti[tc_max];//array of triangle indices
	uint16 tc = 0;//number of potential Triangles
	bool bHit = false;

	//set object handle (who it hit)
	ci.m_hObj = mesh.m_hObj;
	//sphere's surface properties
	ci.m_Sa = this->m_Surf;

	//replace u if a triangle was hit
	ci.m_U = 2.f;

	//get the indices of the triangles that the sphere
	//might have hit along its path from p0 to p1, and
	//report the first contact
	if( AABBTreeBoxSweep( ti, tc, tc_max, min, max, node, C0, C1, d ) )
	{
		const LTTriangle* tri = mesh.m_pData->Triangles();//Triangles
		const LTVector3u16* V = mesh.m_pData->Vertices();//packed Vertices
		const LTVector3f e = max - min;//dimensions of root node
		const float s = 1 / float(0xFFFF);//scale factor

		//sweep the sphere against each triangle
		for( uint16 i=0 ; i<tc ; i++ )
		{
			const LTTriangle& t = tri[ ti[i] ];
			LTVector3f v[3];//unpacked Vertices

			v[0] = UnpackVector( V[t.v[0]], min, e, s );
			v[1] = UnpackVector( V[t.v[1]], min, e, s );
			v[2] = UnpackVector( V[t.v[2]], min, e, s );

			LTVector3f n;//unit contact normal, parent frame
			float u;//normalized time of collision

			//mesh's local frame
			if( SphereTriangleSweep( u, n, C0, C1, r, v[0], v[1], v[2] ) )
			{
				//if this collision occurred before the
				//previous one, replace u, n and pc
				if( u < ci.m_U )
				{
					//interpolate the mesh's local coordinate frame
					//and transform the normal to the parent frame
					const LTCoordinateFrameQ Fu = Interpolate( F0, F1, u );
					n = Fu.TransformVectorToParent(n);
					//unit contact normal
					ci.m_N = n;
					//normalized time of contact
					ci.m_U = u;
					//contact point, world frame
					ci.m_P = (1-u)*m_P0 + u*m_P1 - r*n;
					//this triangle's surface properties
					ci.m_Sb = mesh.m_pData->FindSurface( ti[i] );

					//apply filter
					if( cf.Condition( ci ) )
						bHit = true;
				}
			}
		}
	}

	return bHit;
}

//---------------------------------------------------------------------------//
bool LTCollisionSphere::Intersect
(
	LTIntersectInfo&				ii,
	const ILTCollisionObject&		o,
	const LTIntersectInfo::Filter&	iif
) const
{
	const bool bIntersect = o.IntersectSphere( ii, *this, iif );

	SwapIntersectInfo(ii,o);

	return bIntersect;
}

//---------------------------------------------------------------------------//
bool LTCollisionSphere::IntersectSphere
(
	LTIntersectInfo& ii, const LTCollisionSphere& s,
	const LTIntersectInfo::Filter& iif
) const
{
	const LTVector3f& A = this->m_P1;
	const LTVector3f& B = s.m_P1;
	const LTVector3f v = A - B;//from B to A
	const float d2 = v.LengthSqr();//square of distance
	const float ra = this->m_Radius;
	const float rb = s.m_Radius;
	const float rab = ra + rb;

		//if distance between centers <= sum of radii, they intersect
		if( d2 <= rab*rab )
		{
			LTVector3f n;

			//penetration normal
			if( d2 )//from B to A
				n = v.Unit();
			else//arbitrary, don't divide by 0
				n = LTVector3f(1,0,0);

			//penetration vector
			ii.m_T = (rab - sqrtf(d2))*n;
			//object handle (who was intersected)
			ii.m_hObj = s.m_hObj;
			//approximate centroid of intersection
			ii.m_P = 0.5f * ((B+A) + (rb-ra)*n);

			//apply filter
			if( iif.Condition(ii) )
				return true;
		}

	//no valid intersection
	return false;
}

//---------------------------------------------------------------------------//
bool LTCollisionSphere::IntersectBox
(
	LTIntersectInfo& ii, const LTCollisionBox& b,
	const LTIntersectInfo::Filter& iif
) const
{
	const LTCoordinateFrameQ F( b.m_P1, b.m_R1 );

	if( SphereOBBIntersect(ii.m_T, m_P1, m_Radius, F, b.m_Dim) )
	{
		//who I intersected
		ii.m_hObj = b.m_hObj;
		//approximate the centroid of intersection
		const float t = ii.m_T.Length();
		LTVector3f n;//n points away from sphere
		if( t>0 )
			n = -(1.f/t)*ii.m_T;
		else
			n = (b.m_P1-m_P1).Unit();

		ii.m_P = m_P1 + (m_Radius-0.5f*t)*n;

		//apply filter
		if( iif.Condition( ii ) )
			return true;
	}

	//no valid intersection
	return false;
}

//---------------------------------------------------------------------------//
bool LTCollisionSphere::IntersectCylinder
(
	LTIntersectInfo& ii, const LTCollisionCylinder& c,
	const LTIntersectInfo::Filter& iif
) const
{
	const bool bIntersect = c.IntersectSphere( ii, *this, iif );

	SwapIntersectInfo(ii,c);

	return bIntersect;
}

//---------------------------------------------------------------------------//
bool LTCollisionSphere::IntersectMesh
(
	LTIntersectInfo& ii, const LTCollisionMesh& mesh,
	const LTIntersectInfo::Filter& iif
) const
{
	const float r = this->m_Radius;
	const LTVector3f dim(r,r,r);
	//transform sphere center to mesh's local frame
	const LTCoordinateFrameQ F1( mesh.m_P1, mesh.m_R1 );
	const LTVector3f C = F1.TransformPointToLocal( m_P1 );
	//approximate the sphere with an AABB
	const LTAABB box( C - dim, C + dim );
	//mesh extents
	const LTVector3f& min = mesh.m_pData->m_Min;
	const LTVector3f& max = mesh.m_pData->m_Max;
	//aabb nodes
	const LTAABB_Node* node = mesh.m_pData->Nodes();
	//TODO:  use a dynamic array
	const uint16 tc_max = 128;//max number of triangle indices
	uint16 ti[tc_max];//array of triangle indices
	uint16 tc = 0;//number of potential Triangles

	//get a list of triangles the
	//sphere might be intersecting
	if( AABBTreeBoxIntersect( ti, tc, tc_max, min, max, node, box ) )
	{
		const LTTriangle* tri = mesh.m_pData->Triangles();//Triangles
		const LTVector3u16* V = mesh.m_pData->Vertices();//packed Vertices
		const LTVector3f e = max - min;//dimensions of root node
		const float s = 1 / float(0xFFFF);//scale factor
		LTVector3f T_net(0,0,0);//net translation vector
		LTVector3f P_net(0,0,0);//ave. intersect. pt.
		LTIntersectInfo info;
		int32 N=0;//intersection count

		//who I intersected
		info.m_hObj = mesh.m_hObj;

		//check sphere against each triangle
		for( int32 i=0 ; i<tc ; i++ )
		{
			const LTTriangle& t = tri[ ti[i] ];
			LTVector3f v[3];//unpacked Vertices
			LTVector3f T;//pentration vector

			v[0] = UnpackVector( V[t.v[0]], min, e, s );
			v[1] = UnpackVector( V[t.v[1]], min, e, s );
			v[2] = UnpackVector( V[t.v[2]], min, e, s );

			if( SphereTriangleIntersect( T, C, r, v[0], v[1], v[2] ) )
			{
				//transform T to the mesh's parent frame
				T = F1.TransformVectorToParent( T );
				info.m_T = T;
				//approximate the centroid of intersection
				const float t = T.Length();
				LTVector3f n;//n points away from sphere
				if( t>0 )
					n = -(1.f/t)*T;
				else
					n = (mesh.m_P1-m_P1).Unit();

				info.m_P = m_P1 + (m_Radius-0.5f*t)*n;

				//apply filter
				if( iif.Condition( info ) )
				{
					T_net += info.m_T;//net
					P_net += info.m_P;//to compute average
					N++;
				}
			}
		}

		//intersection info
		if( N )
		{
			ii.m_hObj = mesh.m_hObj;//who I intersected
			ii.m_T = T_net;//net
			ii.m_P = P_net / float(N);//average
			return true;
		}
	}

	//no valid intersection
	return false;
}

//---------------------------------------------------------------------------//
bool LTCollisionSphere::IntersectSegment
(
	LTIntersectInfo					ii[],
	int32&							n,
	const int32						sz,
	const LTVector3f&				p0,
	const LTVector3f&				p1,
	const LTIntersectInfo::Filter&	iif
) const
{
	//solve quadratic to see if the segment came within
	//a distance r of the sphere at its position at u=1
	const int32 n0 = n;//original count
	const LTVector3f	v = p1 - p0;
	const LTVector3f	ab = m_P1 - p0;
	const float r = m_Radius;
	//quadratic coefficients
	const float a = v.Dot(v);
	const float b = -2 * ab.Dot(v);
	const float c = ab.Dot(ab) - r*r;
	//quadratic solutions
	float u1, u2;

		//if solutions are real, then a collision occurred
		if( QuadraticFormula( u1, u2, a, b, c ) )
		{
			//must be in the interval [0,1]
			if( 0<=u1 && u1<=1 )
			{
				//first intersection point
				ii[n].m_hObj = m_hObj;
				ii[n].m_P = Lerp( p0, p1, u1 );
				ii[n].m_T = p0 - ii[n].m_P;

				//apply filter
				if( iif.Condition(ii[n]) )
					n++;//increment intersection count
			}

			//second intersection point
			if( 0<=u2 && u2<=1 && u2!=u1 )//unique
			{
				ii[n].m_hObj = m_hObj;
				ii[n].m_P = Lerp( p0, p1, u2 );
				ii[n].m_T = p0 - ii[n].m_P;

				//apply filter
				if( iif.Condition(ii[n]) )
					n++;//increment intersection count
			}
		}

	//was either intersection valid?
	return n > n0;
}


//---------------------------------------------------------------------------//
//---------------------------------------------------------------------------//
//---------------------------------------------------------------------------//
bool LTCollisionBox::Hit
(
	LTContactInfo& ci, const ILTCollisionObject& o,
	const LTContactInfo::Filter& cf
) const
{
	const bool bHit = o.HitBox( ci, *this, cf );

	//swap contact info if necessary
	SwapContactInfo(ci,o);

	return bHit;
}

//---------------------------------------------------------------------------//
bool LTCollisionBox::HitSphere
(
	LTContactInfo& ci, const LTCollisionSphere& s,
	const LTContactInfo::Filter& cf
) const
{
	const bool bHit = s.HitBox( ci, *this, cf );

	//swap contact info if necessary
	SwapContactInfo(ci,s);

	return bHit;
}

//---------------------------------------------------------------------------//
bool LTCollisionBox::HitBox
(
	LTContactInfo& ci, const LTCollisionBox& b,
	const LTContactInfo::Filter& cf
) const
{
	//A's local coordinate frame at times u=0 and u=1, respectively
	const LTCoordinateFrameQ A0(m_P0,m_R0);
	const LTCoordinateFrameQ A1(m_P1,m_R1);
	//B's local coordinate frame at times u=0 and u=1, respectively
	const LTCoordinateFrameQ B0(b.m_P0,b.m_R0);
	const LTCoordinateFrameQ B1(b.m_P1,b.m_R1);
	LTVector3f pa, pb, n;
	float u;

	if( OBBSweep(u,n,pa,pb,A0,A1,m_Dim,B0,B1,b.m_Dim) )
	{
		ci.m_hObj = b.m_hObj;//other object
		ci.m_U = u;//normalized time of contact
		ci.m_N = n;//unit contact normal
		ci.m_P = 0.5f * (pa + pb);//contact point (1/2way between a and b)
		ci.m_Sa = m_Surf;//A's surface properties
		ci.m_Sb = b.m_Surf;//B's surface properties

		//apply filter
		if( cf.Condition(ci) )
			return true;
	}

	//no valid intersection
	return false;
}

//---------------------------------------------------------------------------//
bool LTCollisionBox::HitCylinder

(
	LTContactInfo& ci, const LTCollisionCylinder& c,
	const LTContactInfo::Filter& cf
) const
{
	const bool bHit = c.HitBox( ci, *this, cf );

	//swap contact info if necessary
	SwapContactInfo(ci,c);

	return bHit;
}

//---------------------------------------------------------------------------//
bool LTCollisionBox::HitMesh
(
	LTContactInfo& ci, const LTCollisionMesh& mesh,
	const LTContactInfo::Filter& cf
) const
{
	//box's local coordinate frames at u=0 and u=1
	LTCoordinateFrameM F0( m_P0, m_R0 );
	LTCoordinateFrameM F1( m_P1, m_R1 );

	ci.m_U = 2.f;//assume no collision

	//TEMP:  use spheres at each corner
	LTVector3f ts = m_Dim;//translation of sphere to box corners
	const float r = Min(ts.x,Min(ts.y,ts.z));//radius of sphere
	//index of min box dimension
	int32 i_min = 0;

	if( r == ts.y )
		i_min = 1;
	else if( r== ts.z )
		i_min = 2;

	//translation along min dimension is 0
	ts[i_min] = 0;
	//translation along other axes
	const int32 i1 = (i_min+1)%3;
	const int32 i2 = (i_min+2)%3;

	ts[i1] -= r;
	ts[i2] -= r;

	//upper right
	{
		//sphere displacement
		const LTVector3f p0 = F0.TransformPointToParent( ts );
		const LTVector3f p1 = F1.TransformPointToParent( ts );
		const LTCollisionSphere s(r,p0,p1,m_Surf,m_hObj);
		LTContactInfo info;

		if( mesh.HitSphere( info, s, cf ) )
			if( info.m_U < ci.m_U )
				ci = info;
	}

	//upper left
	{
		//sphere displacement
		LTVector3f t = ts;

		t[i1] = -t[i1];//opposite side

		const LTVector3f p0 = F0.TransformPointToParent( t );
		const LTVector3f p1 = F1.TransformPointToParent( t );
		const LTCollisionSphere s(r,p0,p1,m_Surf,m_hObj);
		LTContactInfo info;

		if( mesh.HitSphere( info, s, cf ) )
			if( info.m_U < ci.m_U )
				ci = info;
	}

	//lower left
	{
		//sphere displacement
		LTVector3f t = ts;

		t[i1] = -t[i1];//opposite side
		t[i2] = -t[i2];//opposite side

		const LTVector3f p0 = F0.TransformPointToParent( t );
		const LTVector3f p1 = F1.TransformPointToParent( t );
		const LTCollisionSphere s(r,p0,p1,m_Surf,m_hObj);
		LTContactInfo info;

		if( mesh.HitSphere( info, s, cf ) )
			if( info.m_U < ci.m_U )
				ci = info;
	}

	//lower right
	{
		//sphere displacement
		LTVector3f t = ts;

		t[i2] = -t[i2];//opposite side

		const LTVector3f p0 = F0.TransformPointToParent( t );
		const LTVector3f p1 = F1.TransformPointToParent( t );
		const LTCollisionSphere s(r,p0,p1,m_Surf,m_hObj);
		LTContactInfo info;

		if( mesh.HitSphere( info, s, cf ) )
			if( info.m_U < ci.m_U )
				ci = info;
	}

	//valid collision
	return ci.m_U <= 1;
}

//---------------------------------------------------------------------------//
bool LTCollisionBox::Intersect
(
	LTIntersectInfo&				ii,
	const ILTCollisionObject&		o,
	const LTIntersectInfo::Filter&	iif
) const
{
	const bool bIntersect = o.IntersectBox( ii, *this, iif );

	SwapIntersectInfo(ii,o);

	return bIntersect;
}

//---------------------------------------------------------------------------//
bool LTCollisionBox::IntersectSphere
(
	LTIntersectInfo& ii, const LTCollisionSphere& s,
	const LTIntersectInfo::Filter& iif
) const
{
	const bool bIntersect = s.IntersectBox( ii, *this, iif );

	SwapIntersectInfo(ii,s);

	return bIntersect;
}

//---------------------------------------------------------------------------//
bool LTCollisionBox::IntersectBox
(
	LTIntersectInfo& ii, const LTCollisionBox& b,
	const LTIntersectInfo::Filter& iif
) const
{
	const LTCoordinateFrameQ Fa( m_P1, m_R1 );
	const LTCoordinateFrameQ Fb( b.m_P1, b.m_R1 );
	
	if( OBBIntersect( ii.m_T, Fa, m_Dim, Fb, b.m_Dim  ) )
	{
		ii.m_hObj = b.m_hObj;//object handle
		ii.m_P = 0.5f*(m_P1+b.m_P1);

		//apply filter
		if( iif.Condition(ii) )
			return true;
	}

	//no valid intersection
	return false;
}

//---------------------------------------------------------------------------//
bool LTCollisionBox::IntersectCylinder
(
	LTIntersectInfo& ii, const LTCollisionCylinder& c,
	const LTIntersectInfo::Filter& iif
) const
{
	const bool bIntersect = c.IntersectBox( ii, *this, iif );

	SwapIntersectInfo(ii,c);

	return bIntersect;
}

//---------------------------------------------------------------------------//
bool LTCollisionBox::IntersectMesh
(
	LTIntersectInfo& ii, const LTCollisionMesh& mesh,
	const LTIntersectInfo::Filter& iif
) const
{
	//local coordinate frame at u=1
	LTCoordinateFrameM f1( m_P1, m_R1 );
	bool bIntersect = false;

	//TEMP:  use spheres at each corner
	LTVector3f ts = m_Dim;//translation of sphere to box corners
	const float r = Min(ts.x,Min(ts.y,ts.z));//radius of sphere
	//index of min box dimension
	int32 i_min = 0;

	if( r == ts.y )
		i_min = 1;
	else if( r== ts.z )
		i_min = 2;

	//translation along min dimension is 0
	ts[i_min] = 0;
	//translation along other axes
	const int32 i1 = (i_min+1)%3;
	const int32 i2 = (i_min+2)%3;

	ts[i1] -= r;
	ts[i2] -= r;

	//upper right
	{
		//sphere displacement
		const LTVector3f p1 = f1.TransformPointToParent( ts );
		const LTCollisionSphere s(r,p1,p1,m_Surf,m_hObj);
		LTIntersectInfo info;

		if( mesh.IntersectSphere( info, s, iif ) )
		{
			ii = info;
			bIntersect = true;
		}
	}

	//upper left
	{
		//sphere displacement
		LTVector3f t = ts;

		t[i1] = -t[i1];//opposite side

		const LTVector3f p1 = f1.TransformPointToParent( t );
		const LTCollisionSphere s(r,p1,p1,m_Surf,m_hObj);
		LTIntersectInfo info;

		if( mesh.IntersectSphere( info, s, iif ) )
		{
			ii = info;
			bIntersect = true;
		}
	}

	//lower left
	{
		//sphere displacement
		LTVector3f t = ts;

		t[i1] = -t[i1];//opposite side
		t[i2] = -t[i2];//opposite side

		const LTVector3f p1 = f1.TransformPointToParent( t );
		const LTCollisionSphere s(r,p1,p1,m_Surf,m_hObj);
		LTIntersectInfo info;

		if( mesh.IntersectSphere( info, s, iif ) )
		{
			ii = info;
			bIntersect = true;
		}
	}

	//lower right
	{
		//sphere displacement
		LTVector3f t = ts;

		t[i2] = -t[i2];//opposite side

		const LTVector3f p1 = f1.TransformPointToParent( t );
		const LTCollisionSphere s(r,p1,p1,m_Surf,m_hObj);
		LTIntersectInfo info;

		if( mesh.IntersectSphere( info, s, iif ) )
		{
			ii = info;
			bIntersect = true;
		}
	}

	//valid collision
	return bIntersect;
}


//---------------------------------------------------------------------------//
bool LTCollisionBox::IntersectSegment
(
	LTIntersectInfo					ii[],
	int32&							n,
	const int32						sz,
	const LTVector3f&				p0,
	const LTVector3f&				p1,
	const LTIntersectInfo::Filter&	iif
) const
{
	//check if the line segment and the OBB intersect
	const int32 n0 = n;//original count

	//transform the line segment to the OBB's local space
	//so that it can be treated as an AABB
	const LTCoordinateFrameQ F(m_P1,m_R1);
	const LTVector3f l0 = F.TransformPointToLocal( p0 );
	const LTVector3f l1 = F.TransformPointToLocal( p1 );
	LTVector3f pi[2];//intersection points
	int32 ic=0;//number of intersection points (may be 1 or 2)

	if( AABBSegmentIntersect( pi, ic, -m_Dim, m_Dim, l0, l1 ) )
	{
		for( int32 j=0 ; j<ic ; j++ )
		{
			//put in parent frame
			pi[j] = F.TransformPointToParent(pi[j]);

			ii[n].m_hObj = m_hObj;
			ii[n].m_T = pi[j] - p1;
			ii[n].m_P = pi[j];

			//apply filter
			if( iif.Condition(ii[n]) )
				n++;//increment intersection count
		}
	}

	//was either intersection valid?
	return n > n0;
}


//---------------------------------------------------------------------------//
//---------------------------------------------------------------------------//
//---------------------------------------------------------------------------//
bool LTCollisionCylinder::Hit
(
	LTContactInfo& ci, const ILTCollisionObject& o,
	const LTContactInfo::Filter& cf
) const
{
	const bool bHit = o.HitCylinder( ci, *this, cf );

	//swap contact info if necessary
	SwapContactInfo(ci,o);

	return bHit;
}

//---------------------------------------------------------------------------//
bool LTCollisionCylinder::HitSphere
(
	LTContactInfo& ci, const LTCollisionSphere& s,
	const LTContactInfo::Filter& cf
) const
{
	//Sphere's positions at times u=0 and u=1, respectively
	const LTVector3f& A0 = s.m_P0;
	const LTVector3f& A1 = s.m_P1;
	//Sphere's radius
	const float ra = s.m_Radius;
	//Cylinder's local coordinate frame at times u=0 and u=1, respectively
	const LTCoordinateFrameQ B0(this->m_P0,this->m_R0);
	const LTCoordinateFrameQ B1(this->m_P1,this->m_R1);
	//Cylinder's dimensions
	const float rb = this->m_Radius;
	const float hh = this->m_HHeight;
	//closest points and unit contact normal
	LTVector3f pa, pb, n;
	//normalized time of collision
	float u;

		if( SphereCylinderSweep(u,n,pa,pb,A0,A1,ra,B0,B1,rb,hh) )
		{
			ci.m_hObj = s.m_hObj;//other object
			ci.m_U = u;//normalized time of contact
			ci.m_N = -n;//normal should point from sphere -> cylinder
			ci.m_P = 0.5f * (pa + pb);//contact point (1/2way between a and b)
			ci.m_Sa = m_Surf;//cylinder's surface properties
			ci.m_Sb = s.m_Surf;//sphere's surface properties

			//apply filter
			if( cf.Condition( ci ) )
				return true;
		}

	//no valid collision
	return false;
}

//---------------------------------------------------------------------------//
bool LTCollisionCylinder::HitBox
(
	LTContactInfo& ci, const LTCollisionBox& b,
	const LTContactInfo::Filter& cf
) const
{
	//Cylinder's local coordinate frame at times u=0 and u=1, respectively
	const LTCoordinateFrameQ A0(this->m_P0,this->m_R0);
	const LTCoordinateFrameQ A1(this->m_P1,this->m_R1);
	//Cylinder's dimensions
	const float r = this->m_Radius;
	const float hh = this->m_HHeight;
	//Box's local coordinate frame at times u=0 and u=1, respectively
	const LTCoordinateFrameQ B0(b.m_P0,b.m_R0);
	const LTCoordinateFrameQ B1(b.m_P1,b.m_R1);
	//Box's dimensions
	const LTVector3f& dim = b.m_Dim;
	//closest points and unit contact normal
	LTVector3f pa, pb, n;
	//normalized time of collision
	float u;

		if( CylinderBoxSweep(u,n,pa,pb,A0,A1,r,hh,B0,B1,dim) )
		{
			ci.m_hObj = b.m_hObj;//other object
			ci.m_U = u;//normalized time of contact
			ci.m_N = n;//unit contact normal
			ci.m_P = 0.5f * (pa + pb);//contact point (1/2way between a and b)
			ci.m_Sa = m_Surf;//A's surface properties
			ci.m_Sb = b.m_Surf;//B's surface properties

			//apply filter
			if( cf.Condition( ci ) )
				return true;
		}

	//no valid collision
	return false;
}

//---------------------------------------------------------------------------//
bool LTCollisionCylinder::HitCylinder
(
	LTContactInfo& ci, const LTCollisionCylinder& c,
	const LTContactInfo::Filter& cf
) const
{
	//A's local coordinate frame at times u=0 and u=1, respectively
	const LTCoordinateFrameQ A0(this->m_P0,this->m_R0);
	const LTCoordinateFrameQ A1(this->m_P1,this->m_R1);
	//A's dimensions
	const float ra = this->m_Radius;
	const float ha = this->m_HHeight;
	//B's local coordinate frame at times u=0 and u=1, respectively
	const LTCoordinateFrameQ B0(c.m_P0,c.m_R0);
	const LTCoordinateFrameQ B1(c.m_P1,c.m_R1);
	//B's dimensions
	const float rb = c.m_Radius;
	const float hb = c.m_HHeight;
	//closest points and unit contact normal
	LTVector3f pa, pb, n;
	//normalized time of collision
	float u;

		if( CylinderSweep(u,n,pa,pb,A0,A1,ra,ha,B0,B1,rb,hb) )
		{
			ci.m_hObj = c.m_hObj;//other object
			ci.m_U = u;//normalized time of contact
			ci.m_N = n;//unit contact normal
			ci.m_P = 0.5f * (pa + pb);//contact point (1/2way between a and b)
			ci.m_Sa = m_Surf;//A's surface properties
			ci.m_Sb = c.m_Surf;//B's surface properties

			//apply filter
			if( cf.Condition( ci ) )
				return true;
		}

	//no valid collision
	return false;
}

//---------------------------------------------------------------------------//
bool LTCollisionCylinder::HitMesh
(
	LTContactInfo& ci, const LTCollisionMesh& m,
	const LTContactInfo::Filter& cf
) const
{
	const bool bHit = m.HitCylinder( ci, *this, cf );

	//swap contact info if necessary
	SwapContactInfo(ci,m);

	return bHit;
}

//---------------------------------------------------------------------------//
bool LTCollisionCylinder::Intersect
(
	LTIntersectInfo&				ii,
	const ILTCollisionObject&		o,
	const LTIntersectInfo::Filter&	iif
) const
{
	const bool bIntersect = o.IntersectCylinder( ii, *this, iif );

	SwapIntersectInfo(ii,o);

	return bIntersect;
}

//---------------------------------------------------------------------------//
bool LTCollisionCylinder::IntersectSphere
(
	LTIntersectInfo& ii, const LTCollisionSphere& s,
	const LTIntersectInfo::Filter& iif
) const
{
	//TEMP:  treat cylinder as a box
	const LTCoordinateFrameQ F( m_P1, m_R1 );
	const LTVector3f dim(m_Radius,m_HHeight,m_Radius);
	LTVector3f T;//penetration vector

	if( SphereOBBIntersect(T, s.m_P1, s.m_Radius, F, dim) )
	{
		//who I intersected
		ii.m_hObj = s.m_hObj;
		//approximate the centroid of intersection
		const float t = T.Length();
		LTVector3f n;//n points away from sphere
		if( t>0 )
			n = -(1.f/t)*T;
		else
			n = (s.m_P1-m_P1).Unit();

		ii.m_T = T;
		ii.m_P = s.m_P1 + (s.m_Radius-0.5f*t)*n;

		//apply filter
		if( iif.Condition( ii ) )
			return true;
	}

	//no valid intersection
	return false;
}

//---------------------------------------------------------------------------//
bool LTCollisionCylinder::IntersectBox
(
	LTIntersectInfo& ii, const LTCollisionBox& b,
	const LTIntersectInfo::Filter& iif
) const
{
	//cylinder
	//TEMP:  treat cylinder as a box
	const LTCoordinateFrameQ Fa( m_P1, m_R1 );
	const LTVector3f dim(m_Radius,m_HHeight,m_Radius);
	//box
	const LTCoordinateFrameQ Fb( b.m_P1, b.m_R1 );

	if( OBBIntersect( ii.m_T, Fa, dim, Fb, b.m_Dim  ) )
	{
		ii.m_hObj = b.m_hObj;//object handle
		ii.m_P = 0.5f*(m_P1+b.m_P1);

		//apply filter
		if( iif.Condition(ii) )
			return true;
	}

	//no valid intersection
	return false;
}

//---------------------------------------------------------------------------//
bool LTCollisionCylinder::IntersectCylinder
(
	LTIntersectInfo& ii, const LTCollisionCylinder& c,
	const LTIntersectInfo::Filter& iif
) const
{
	//TEMP:  treat cylinders as a boxes
	//A
	const LTCoordinateFrameQ Fa( m_P1, m_R1 );
	const LTVector3f da( m_Radius, m_HHeight, m_Radius );
	//B
	const LTCoordinateFrameQ Fb( c.m_P1, c.m_R1 );
	const LTVector3f db( c.m_Radius, c.m_HHeight, c.m_Radius );

	if( OBBIntersect( ii.m_T, Fa, da, Fb, db  ) )
	{
		ii.m_hObj = c.m_hObj;//object handle
		ii.m_P = 0.5f*(m_P1+c.m_P1);

		//apply filter
		if( iif.Condition(ii) )
			return true;
	}

	//no valid intersection
	return false;
}

//---------------------------------------------------------------------------//
bool LTCollisionCylinder::IntersectMesh
(
	LTIntersectInfo& ii, const LTCollisionMesh& m,
	const LTIntersectInfo::Filter& iif
) const
{
	const bool bIntersect = m.IntersectCylinder( ii, *this, iif );

	SwapIntersectInfo(ii,m);

	return bIntersect;
}

//---------------------------------------------------------------------------//
bool LTCollisionCylinder::IntersectSegment
(
	LTIntersectInfo					ii[],
	int32&							n,
	const int32						sz,
	const LTVector3f&				p0,
	const LTVector3f&				p1,
	const LTIntersectInfo::Filter&	iif
) const
{
	//TEMP:  use an OBB for the cylinder
	const int32 n0 = n;//original count

	//transform the line segment to the Cylinder's local space
	//so that it can be treated as an AABB
	const LTCoordinateFrameQ F(m_P1,m_R1);
	const LTVector3f l0 = F.TransformPointToLocal( p0 );
	const LTVector3f l1 = F.TransformPointToLocal( p1 );
	const LTVector3f dim( m_Radius, m_HHeight, m_Radius );
	LTVector3f pi[2];//intersection points
	int32 ic=0;//number of intersection points (may be 1 or 2)

	if( AABBSegmentIntersect( pi, ic, -dim, dim, l0, l1 ) )
	{
		for( int32 j=0 ; j<ic ; j++ )
		{
			//put in parent frame
			pi[j] = F.TransformPointToParent(pi[j]);

			ii[n].m_hObj = m_hObj;
			ii[n].m_T = pi[j] - p1;
			ii[n].m_P = pi[j];

			//apply filter
			if( iif.Condition(ii[n]) )
				n++;//increment intersection count
		}
	}

	//was either intersection valid?
	return n > n0;
}


//---------------------------------------------------------------------------//
//---------------------------------------------------------------------------//
//---------------------------------------------------------------------------//
LTCollisionMesh::~LTCollisionMesh()
{
	//delete memory
	if( m_pData && m_Delete )
		delete m_pData;
}

//---------------------------------------------------------------------------//
bool LTCollisionMesh::Hit
(
	LTContactInfo& ci, const ILTCollisionObject& o,
	const LTContactInfo::Filter& cf
) const
{
	const bool bHit = o.HitMesh( ci, *this, cf );

	//swap contact info if necessary
	SwapContactInfo(ci,o);

	return bHit;
}

//---------------------------------------------------------------------------//
bool LTCollisionMesh::HitSphere
(
	LTContactInfo& ci, const LTCollisionSphere& s,
	const LTContactInfo::Filter& cf
) const
{
	const bool bHit = s.HitMesh( ci, *this, cf );

	//swap contact info if necessary
	SwapContactInfo(ci,s);

	return bHit;
}

//---------------------------------------------------------------------------//
bool LTCollisionMesh::HitBox
(
	LTContactInfo& ci, const LTCollisionBox& b,
	const LTContactInfo::Filter& cf
) const
{
	const bool bHit = b.HitMesh( ci, *this, cf );

	//swap contact info if necessary
	SwapContactInfo(ci,b);

	return bHit;
}

//---------------------------------------------------------------------------//
bool LTCollisionMesh::HitCylinder
(
	LTContactInfo& ci, const LTCollisionCylinder& c,
	const LTContactInfo::Filter& cf
) const
{
	const float r = c.m_Radius;
	const float h = c.m_HHeight;
	const float hr = h / r;

	ci.m_U = 2.f;//assume no collision

	if( hr > 1.f )
	{
		//treat the cylinder as a line of spheres along the longitudonal axis

		//cylinder's coordinate frame at times u=0 and u=1
		const LTCoordinateFrameM B0(c.m_P0,c.m_R0);
		const LTCoordinateFrameM B1(c.m_P1,c.m_R1);
		const int32 N = int32(hr) + 1;
		const float d = 2.f*(h-r)/float(N-1);

		for( int32 j=0 ; j<N ; j++ )
		{
			//sphere center, cylinder frame
			const LTVector3f C( 0.f, r + j*d - h, 0.f );
			//center at times u=0 and u=1, parent frame
			const LTVector3f C0 = B0.TransformPointToParent(C);
			const LTVector3f C1 = B1.TransformPointToParent(C);
			//collision object
			const LTCollisionSphere sphere(r,C0,C1,c.m_Surf,c.m_hObj);
			LTContactInfo info;

			if( sphere.HitMesh( info, *this, cf ) )
				if( info.m_U < ci.m_U )
					ci = info;
		}
	}
	else//cylinder is short and squatty
	{
		//treat the cylinder as an OBB
		const LTVector3f dim(r,h,r);
		const LTCollisionBox box(dim,c.m_P0,c.m_P1,c.m_R0,c.m_R1,c.m_Surf,c.m_hObj);

		box.HitMesh( ci, *this, cf );
	}

	//valid collision
	return ci.m_U <= 1.f;
}

//---------------------------------------------------------------------------//
bool LTCollisionMesh::Intersect
(
	LTIntersectInfo&				ii,
	const ILTCollisionObject&		o,
	const LTIntersectInfo::Filter&	iif
) const
{
	const bool bIntersect = o.IntersectMesh( ii, *this, iif );

	SwapIntersectInfo(ii,o);

	return bIntersect;
}

//---------------------------------------------------------------------------//
bool LTCollisionMesh::IntersectSphere
(
	LTIntersectInfo& ii, const LTCollisionSphere& s,
	const LTIntersectInfo::Filter& iif
) const
{
	const bool bIntersect = s.IntersectMesh( ii, *this, iif );

	SwapIntersectInfo(ii,s);

	return bIntersect;
}

//---------------------------------------------------------------------------//
bool LTCollisionMesh::IntersectBox
(
	LTIntersectInfo& ii, const LTCollisionBox& b,
	const LTIntersectInfo::Filter& iif
) const
{
	const bool bIntersect = b.IntersectMesh( ii, *this, iif );

	SwapIntersectInfo(ii,b);

	return bIntersect;
}

//---------------------------------------------------------------------------//
bool LTCollisionMesh::IntersectCylinder
(
	LTIntersectInfo& ii, const LTCollisionCylinder& c,
	const LTIntersectInfo::Filter& iif
) const
{
	const float r = c.m_Radius;
	const float h = c.m_HHeight;
	const float hr = h / r;
	bool bIntersect = false;

	ii.m_T.Init();

	if( hr > 1.f )
	{
		//treat the cylinder as a line of spheres along the longitudonal axis

		//cylinder's coordinate frame at time u=1
		const LTCoordinateFrameM B1(c.m_P1,c.m_R1);
		const int32 N = int32(hr) + 1;
		const float d = 2.f*(h-r)/float(N-1);

		for( int32 j=0 ; j<N ; j++ )
		{
			//sphere center, cylinder frame
			const LTVector3f C( 0.f, r + j*d - h, 0.f );
			//center at time u=1, parent frame
			const LTVector3f C1 = B1.TransformPointToParent(C);
			//collision object
			const LTCollisionSphere sphere(r,C1,C1,c.m_Surf,c.m_hObj);
			LTIntersectInfo info;

			if( sphere.IntersectMesh( info, *this, iif ) )
			{
				bIntersect = true;

				//use info with the largest penetration
				if( info.m_T.LengthSqr() > ii.m_T.LengthSqr() )
					ii = info;
			}
		}
	}
	else//cylinder is short and squatty
	{
		//treat the cylinder as an OBB
		const LTVector3f dim(r,h,r);
		const LTCollisionBox box(dim,c.m_P0,c.m_P1,c.m_R0,c.m_R1,c.m_Surf,c.m_hObj);

		bIntersect = box.IntersectMesh( ii, *this, iif );
	}

	return bIntersect;
}

//---------------------------------------------------------------------------//
bool LTCollisionMesh::IntersectSegment
(
	LTIntersectInfo					ii[],
	int32&							n,
	const int32						sz,
	const LTVector3f&				p0,
	const LTVector3f&				p1,
	const LTIntersectInfo::Filter&	iif
) const
{
	//find all intersections will the mesh
	const int32 n0 = n;

	//transform the line segment to the mesh's local space
	//and treat it as a swept aabb with 0 dimensions
	const LTCoordinateFrameQ F1(m_P1,m_R1);
	const LTVector3f l0 = F1.TransformPointToLocal( p0 );
	const LTVector3f l1 = F1.TransformPointToLocal( p1 );
	const LTVector3f dim(0,0,0);
	//get the aabb tree data
	const LTVector3f& min = this->m_pData->m_Min;//root extents
	const LTVector3f& max = this->m_pData->m_Max;
	const LTAABB_Node* node = this->m_pData->Nodes();//aabb node array
	//TODO:  use a dynamic array
	const uint16 tc_max = 128;//max number of triangle indices
	uint16 ti[tc_max];//array of triangle indices
	uint16 tc = 0;//number of potential Triangles

	//get a list of potential triangles then
	//check each of them for intersection
	if( AABBTreeBoxSweep( ti, tc, tc_max, min, max, node, l0, l1, dim ) )
	{
		const LTTriangle* tri = this->m_pData->Triangles();//Triangles
		const LTVector3u16* V = this->m_pData->Vertices();//packed Vertices
		const LTVector3f e = max - min;//dimensions of root node
		const float s = 1 / float(0xFFFF);//scale factor

		//check line segment against each triangle
		for( int32 i=0 ; i<tc ; i++ )
		{
			const LTTriangle& t = tri[ ti[i] ];
			const LTVector3f v0 = UnpackVector( V[t.v[0]], min, e, s );
			const LTVector3f v1 = UnpackVector( V[t.v[1]], min, e, s );
			const LTVector3f v2 = UnpackVector( V[t.v[2]], min, e, s );
			LTVector3f pi;//point of intersection

			if( TriangleSegmentIntersection( pi, v0, v1, v2, l0, l1 ) )
			{
				ii[n].m_hObj	= this->m_hObj;//who I intersected
				ii[n].m_T		= pi - l1;//penetration estimate
				ii[n].m_P		= pi;//point of intersection

				if( iif.Condition( ii[n] ) )
					n++;//increment intersection count
			}
		}
	}

	//were there valid intersections?
	return n>n0;
}


//EOF

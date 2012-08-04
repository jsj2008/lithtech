#include "bdefs.h"
#include "uvtoopq.h"

// double the area of the passed in tri, used for barycentric coordinate computation
static float BaryCoordsArea( const LTVector& p0, const LTVector& p1, const LTVector& p2 )
{
	LTVector e0 = p1 - p0;
	LTVector e1 = p2 - p0;

	return (e0.x * e1.y - e1.x * e0.y);
}

// given a triangle p0,p1,p2 find the barycentric coordinates of point p which lies in the plane of tri
static LTVector BaryCoords( const LTVector& p0, const LTVector& p1, const LTVector& p2, const LTVector& p )
{
	float n = BaryCoordsArea( p0, p1, p2 );

	if( fabs(n) < 0.001 )
		return LTVector( 1.0f, 0.0f, 0.0f );

	float u = BaryCoordsArea( p1, p2, p ) / n;
	float v = BaryCoordsArea( p2, p0, p ) / n;
	float w = 1.0f - u - v;

	return LTVector( u, v, w );
}

// sets up OPQs based on uv coordinates for each vertex
// takes 3 vertex positions as well as their UV coordinates
// takes an array of 6 floats, 2 per vertex for the first 3 verts plus the width and height of the texture
bool ConvertUVToOPQ(	LTVector *pos, const float* coords, 
						const uint32 texWidth, const uint32 texHeight,
						LTVector& O, LTVector& P, LTVector& Q)
{
	// vertex positions in texture space
	const LTVector tv0( coords[0], -coords[1], 0.0f );
	const LTVector tv1( coords[2], -coords[3], 0.0f );
	const LTVector tv2( coords[4], -coords[5], 0.0f );

	// vertex positions in world space
	const LTVector& v0 = pos[0];
	const LTVector& v1 = pos[1];
	const LTVector& v2 = pos[2];

	// determine barycentric coordinates of OPQ in texture space
	const LTVector bcO = BaryCoords( tv0, tv1, tv2, LTVector(0.0f,0.0f,0.0f) );
	const LTVector bcP = BaryCoords( tv0, tv1, tv2, LTVector(1.0f,0.0f,0.0f) );
	const LTVector bcQ = BaryCoords( tv0, tv1, tv2, LTVector(0.0f,1.0f,0.0f) );

	// calculate OPQ in world space
	O = bcO.x * v0 + bcO.y * v1 + bcO.z * v2;
	P = bcP.x * v0 + bcP.y * v1 + bcP.z * v2;
	Q = bcQ.x * v0 + bcQ.y * v1 + bcQ.z * v2;
	P = P - O;
	Q = Q - O;

	// obj importer assumes 256x256 textures
	float tp = P.Mag();
	tp *= 1.0f / (float)texWidth;
	tp = 1.0f / tp;
	float tq = Q.Mag();
	tq *= 1.0f / (float)texHeight;
	tq = 1.0f / tq;

	P.Normalize();
	Q.Normalize();

	// fix up p and q to be what DEdit really wants
	LTVector R = Q.Cross( P );
	LTVector PNew = R.Cross( Q );
	LTVector QNew = P.Cross( R );

	// fix up scale factors for new p and q
	PNew.Normalize();
	QNew.Normalize();
	float pscale = 1.0f / P.Dot( PNew );
	float qscale = 1.0f / Q.Dot( QNew );

	R = QNew.Cross( PNew );

	PNew *= tp * pscale;
	QNew *= tq * qscale;

	// orthogonalize P and Q
	R.Normalize();
	P = PNew + R;
	Q = QNew - (PNew.Dot( QNew ) * R);

	return true;
}
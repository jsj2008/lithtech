// 3D gradient noise implementation
// from "Texturing and Modeling: A Procedural Approach" by Darwyn Peachey

#include "bdefs.h"


#define TABLE_SIZE		256
#define TABLE_MASK		(TABLE_SIZE-1)


static float gradientTable[TABLE_SIZE*3];
static unsigned char permTable[TABLE_SIZE];


static inline double drand()
{
	return (double)rand() / (double)RAND_MAX;
}


static inline float SmoothStep( float x )
{
	return x * x * (3.0f - (2.0f * x));
}


static inline float Lerp( float t, float x0, float x1 )
{
	return x0 + t * (x1 - x0);
}


static inline int Perm( int x )
{
	return permTable[x & TABLE_MASK];
}


static inline int Index( int ix, int iy, int iz )
{
	return Perm( ix + Perm( iy + Perm( iz ) ) );
}


static inline float Lattice( int ix, int iy, int iz, float fx, float fy, float fz )
{
	float* g = &gradientTable[Index(ix,iy,iz)*3];
	return g[0]*fx + g[1]*fy + g[2]*fz;
}


static void InitTables( int seed )
{
	float* table = gradientTable;
	float z, r, theta;
	int i;

	srand( seed );

	for( i = 0; i < TABLE_SIZE; i++ )
	{
		z = 1.0f - 2.0f * (float)drand();

		// r is radius of x,y circle
		r = sqrtf( 1.0f - z*z );

		// theta is angle in (x,y)
		theta = (float)(2.0 * MATH_PI * drand());
		*table++ = r * cosf( theta );
		*table++ = r * sinf( theta );
		*table++ = z;
	}

	// initialize the perm table (a table containing one of each number in [0-TABLE_SIZE) in random order)
	bool hasValue[TABLE_SIZE];

	for( i = 0; i < TABLE_SIZE; i++ )
	{
		hasValue[i] = false;
	}

	for( i = 0; i < TABLE_SIZE; i++ )
	{
		// pick a random starting point in the table
		int pos = (int)(drand() * (TABLE_SIZE-1));

		// loop through the table til we find an entry that hasn't been set yet
		while( 1 )
		{
			// found an empty entry, fill it with i and exit the loop
			if( !hasValue[pos] )
			{
				permTable[pos] = i;
				hasValue[pos] = true;
				break;
			}

			// position is full, advance to the next one
			++pos;

			if( pos >= TABLE_SIZE )
				pos = 0;
		}
	}
}


// given a 3D point, return a value in the range -1 to 1 (always returns 0 at integer lattice points)
float Noise( const LTVector& pos )
{
	int ix, iy, iz;
	float fx0, fx1, fy0, fy1, fz0, fz1;
	float wx, wy, wz;
	float vx0, vx1, vy0, vy1, vz0, vz1;
	static bool initialized = false;

	if( !initialized )
	{
		InitTables( 665 );
		initialized = true;
	}

	ix = (int)floor( pos.x );
	fx0 = pos.x - ix;
	fx1 = fx0 - 1;
	wx = SmoothStep( fx0 );

	iy = (int)floor( pos.y );
	fy0 = pos.y - iy;
	fy1 = fy0 - 1;
	wy = SmoothStep( fy0 );

	iz = (int)floor( pos.z );
	fz0 = pos.z - iz;
	fz1 = fz0 - 1;
	wz = SmoothStep( fz0 );

	vx0 = Lattice( ix, iy, iz, fx0, fy0, fz0 );
	vx1 = Lattice( ix+1, iy, iz, fx1, fy0, fz0 );
	vy0 = Lerp( wx, vx0, vx1 );
	vx0 = Lattice( ix, iy+1, iz, fx0, fy1, fz0 );
	vx1 = Lattice( ix+1, iy+1, iz, fx1, fy1, fz0 );
	vy1 = Lerp( wx, vx0, vx1 );
	vz0 = Lerp( wy, vy0, vy1 );

	vx0 = Lattice( ix, iy, iz+1, fx0, fy0, fz1 );
	vx1 = Lattice( ix+1, iy, iz+1, fx1, fy0, fz1 );
	vy0 = Lerp( wx, vx0, vx1 );
	vx0 = Lattice( ix, iy+1, iz+1, fx0, fy1, fz1 );
	vx1 = Lattice( ix+1, iy+1, iz+1, fx1, fy1, fz1 );
	vy1 = Lerp( wx, vx0, vx1 );
	vz1 = Lerp( wy, vy0, vy1 );

	return Lerp( wz, vz0, vz1 );
}

#include "gjk.h"


//---------------------------------------------------------------------------//
// GJK References:
//
// S. Cameron.  "Enhancing GJK:  Computing Minimum and Penetration Distances
//		between Convex Polyhedra," In Proc. IEEE Int. Conf. on Robotics and
//		Automation, pp. 3112-3117, 1997
//
// E.G. Gilbert and C.-P. Foo.  "Computing the Distance Between General Convex
//		Objects in Three-Dimensional Space," IEEE Transactions on Robotics
//		and Automation, 6(1): pp. 53-61, 1990.
//
// E.G. Gilbert, D.W. Johnson, and S.S. Keerthi.  "A Fast Procedure for
//		Computing the Distance Between Complex Objects in Three-Dimensional
//		Space," IEEE Journal of Robotics and Automation, 4(2): pp. 193-203,
//		1988.
//
// R.T. Rockafellar.  Convex Analysis.  Princeton University Press, 1997
//
// J.J. Tuma, R.A. Walsh.  Engineering Mathematics Handbook, 4th Ed.  McGraw-
//		Hill, 1998.
//
// G. van den Bergen.  Collision Detection in Interactive 3D Computer Animation.
//		Ph.D. Thesis, Eindhoven University of Technology, 1999.
//
// G. van den Bergen.  "A Fast and Robust GJK Implementation for Collision
//		Detection of Convex Objects,"  In ACM Journal of Graphics Tools,
//		4(2): pp. 7-25, 1999.
//


//---------------------------------------------------------------------------//
//for debugging
union LTBitfield32
{
	struct wbits
	{
		unsigned b0:	1;
		unsigned b1:	1;
		unsigned b2:	1;
		unsigned b3:	1;
		unsigned b4:	1;
		unsigned b5:	1;
		unsigned b6:	1;
		unsigned b7:	1;
	};

	int32	n;//value
	wbits	b;//wbits

	LTBitfield32( const int32 i )
		:	n(i)
	{}

	//cast to int32
	operator int32() const
	{
		return n;
	}

	//shift
	const LTBitfield32 operator <<= ( const int32 s )
	{
		n <<= s;
		return n;
	}
};

//make sure lower-order cofactors
//are computed before higher-order
const int32 xbit_table[14] = 
{
	//verts are all 1
	1,2,4,8,			//verts
	3,5,9,6,0x0A,0x0C,	//edges
	7,11,13,14			//faces
};

//---------------------------------------------------------------------------//
//compute the cofactors of the matrices corresponding
//to each feature on the simplex Y
void compute_cofactors( float del[4][16], const LTVector3f y[4], const int32 N )
{
	float dy[4][4];

	//precompute dot products y[i].y[j]
	for( int32 i=0 ; i<N ; i++ )
	{
		for( int32 j=0 ; j<N ; j++ )
		{
			dy[i][j] = y[i].Dot(y[j]);
		}
	}

	float* pc = (float*)del;

	//initialize c[][] to 0, or else closest_feature()
	//may fail since higher order cofactors may be >0
	for( int32 i=0 ; i<64 ; i++ )
		pc[i] = 0.f;

	//set cofactors that correspond
	//to single vertices to 1
	for( int32 i=0 ; i<4 ; i++ )
		del[i][1<<i] = 1;

	int32 xicount;

	if( 2==N )
		xicount = 4;
	else if( 3==N )
		xicount = 10;
	else//4==N )
		xicount = 14;

	//compute cofactors for each X configuration
	//garbage goes in some entries, but they won't
	//be checked
	for( int32 xi = 0 ; xi < xicount ; xi++ )
	{
		const int32 xbits = xbit_table[xi];
		int32 k;//min( Ix )

		//index of min bit of xbits
		for( k=0 ; !(xbits&(1<<k)) ; k++ )
		{}

		//for each j not in Ix compute del[j]( X U {y[j]} )
		for( int32 j=0 ; j<4 ; j++ )
		{
			int32 jbit = 1<<j;

			if( !(xbits & jbit) )
			{
				const int32 xybits = xbits | jbit;//X U {y[j]}

				del[j][xybits] = 0;

				//compute del[j]( X U {y[j]} ) =
				//Sum( del[i](X) * (y[i].y[k] - y[i].y[j]), for all i in Ix )
				for( int32 i=0 ; i<4 ; i++ )
				{
					if( xbits & (1<<i) )//i is in Ix
						del[j][xybits] += del[i][xbits]*(dy[i][k] - dy[i][j]);
				}

				int kd=0;//DEBUG
			}
		}
	}
}

//---------------------------------------------------------------------------//
bool closest_feature
(
	const int32 xbits,		//defines simplex X
	const float del[4][16],	//cofactors del[i]( X )
	const int32 N			//number of points in Y
)
{
	for( int32 j=0 ; j<N ; j++ )
	{
		int32 jbit = 1<<j;

		if( xbits & jbit )//j is in Ix
		{
			//del[i]( X )
			if( del[j][xbits] <= 0 )
				return false;
		}
		//j is not in Ix
		else if( 0x0F != xbits )//don't check these for a tetrahedron
		{
			//del[j]( X U {y[j]} )
			if( del[j][xbits | jbit] > 0 )
				return false;
		}
	}

	return true;
}

//---------------------------------------------------------------------------//
const LTVector3f determine_X_and_v
(
	float		del[4][16],
	LTVector3f	p[4],
	LTVector3f	q[4],
	LTVector3f	y[4],
	float		lam[4],
	int32&		N
)
{
	///////////////////
	//DEBUG:  These numbers create an ill-conditioned
	//simplex and 7 digits are not enough to resolve
	//the system.  The highest level cofactors are
	//basically garbage
//	N = 4;
//	y[0].Init( -4.47f, -65.0f, -1.13f );
//	y[1].Init(  6.16f,  94.7f,  2.87f );
//	y[2].Init( -4.24f, -65.8f, -2.38f );
//	y[3].Init( -4.19f, -65.9f, -2.67f );
	///////////////////

	if( 1==N )
	{
		lam[0] = 1.f;
		return y[0];
	}

	//compute the cofactors of the matrices corresponding
	//to each feature on the simplex Y
	compute_cofactors( del, y, N );

	const int32 ybits = (1<<N) - 1;

	//check the signs of the cofactors to determine
	//the closest feature on Y to the origin; check
	//the largest features first
	for( int32 xbits = ybits ; xbits>0 ; xbits-- )
	{
		//check if X, the subset of Y defined by xbits,
		//is the closest feature on the simplex to the
		//origin of configuration space
		if( closest_feature( xbits, del, N ) )
		{
			float detA = 0;

			//det(A) = Sum( del[i]( X ) )
			for( int32 i=0 ; i<N ; i++ )
				if( xbits & (1<<i) )//i is in Ix
					detA += del[i][xbits];

			LTVector3f v(0,0,0);//separating axis = v(conv(X)) = v(conv(Y))
			const float s = 1/detA;

			N=0;//reset N OUTSIDE of loop

			//shuffle p, q, and y; compute lam's and v
			for( int32 i=0 ; i<4 ; i++ )
			{
				if( xbits & (1<<i) )
				{
					//shuffle pts
					p[N]	= p[i];
					q[N]	= q[i];
					y[N]	= y[i];
					//interpolation parameter
					lam[N]	= s * del[i][xbits];
					//separating axis
					v		+= lam[N] * y[N];
					//point count
					N++;
				}
			}

			if( 4 == N )//contains origin
				v.Init();

			return v;//found it, stop
		}
	}

	//NOTE:  Because we're using only 7 digits of precision,
	//we might not be able to resolve highly ill-conditioned
	//simplices.  In this case, just return (0,0,0) so the
	//algorithm terminates.
	return LTVector3f(0,0,0);
}


//---------------------------------------------------------------------------//
//return true if the last point added to Y is equal to any of its other points
bool degenerate_Y( const LTVector3f y[4], const int32 N, const float eps )
{
	for( int32 i=0 ; i<N-1 ; i++ )
	{
		if( y[i].NearlyEquals( y[N-1], eps ) )
			return true;
	}

	return false;
}


//---------------------------------------------------------------------------//
//given two support mappings sA and sB, compute the closest points between
//objects A and B, return the distance d between them
float GJK_ClosestPoints
(
	LTVector3f& a,
	LTVector3f& b,
	ILTSupportMap& sA,
	ILTSupportMap& sB,
	const float eps
)
{
	//p[] and q[] (the simplex vertices of A and B in their
	//respective local coordinates) are used to compute the
	//closest points a and b when the algorithm terminates
	LTVector3f p[4], q[4];
	LTVector3f y[4];//simplex vertices of A-B in parent coordinates
	int32 N = 0;//number of points in p[], q[] and y[]
	LTVector3f v = sA.Position() - sB.Position();//current separating axis
	float lam[4] = {1.f,0.f,0.f,0.f};//interpolation parameters
	float d = v.Length();//distance between A and B
	float mu = 0.f;//lower bound on d
	float del[4][16];//cofactors for features of simplex
	int32 iter = 0;

	while( d>eps && iter<50 )
	{
		//keep track of iterations
		iter++;

		//support points in their respective local frames
		p[N] = sA( sA.TransformVectorToLocal(-v) );
		q[N] = sB( sB.TransformVectorToLocal(v) );

		//transform a and b to the same parent frame
		a = sA.TransformPointToParent( p[N] );
		b = sB.TransformPointToParent( q[N] );

		//kth support point, wk = sAB( -vk )
		const LTVector3f w = a - b;

		//add wk to simplex Y
		y[N++] = w;

		//if the difference between d and its lower bound mu
		//is less than a tolerable relative error, then w is
		//the closest point on A-B to O
		mu = Max( mu, v.Dot(w) / d );

		if( d-mu <= d*eps )//relative error
			break;

		//if w is equal to any of the points already in the simplex
		//Y, then w is the closest point in A-B to origin and d has
		//been found to within eps
		if( degenerate_Y(y,N,eps) )
			break;
		//NOTE:  To guarantee maximum accuracy for {a,b}, the
		//degeneracy condition can be used to terminate, but the
		//accuracy gains seem to be negligable and the iteration
		//count doubles

		//y[] goes in as Y={yi} and comes out as X={xi},
		//N is adjusted accordingly, shuffle p[] and q[]
		//as well and determine lam[i]
		v = determine_X_and_v( del, p, q, y, lam, N );

		//distance from closest feature on simplex
		//to origin, 0 if simplex contains origin
		d = v.Length();
	}

	//set a and b to 0
	a.Init();
	b.Init();

	//closest points in local frame
	for( int32 i=0 ; i<N ; i++ )
	{
		a += lam[i] * p[i];
		b += lam[i] * q[i];
	}

	//transform a and b to parent frame
	a = sA.TransformPointToParent( a );
	b = sB.TransformPointToParent( b );

	//distance from a to b
	return d;
}

//EOF

#include "quaternion.h"


//---------------------------------------------------------------------------//
LTQuaternionf::LTQuaternionf( const LTMatrix3f& M )
{
	float tr, s;
	float& x = v.x;
	float& y = v.y;
	float& z = v.z;

		//
		// Taken from David Baraff's "Intro to Rigid Body Sim I".
		//

		tr = M(0,0) + M(1,1) + M(2,2);

		if( tr >= 0 )
		{
			s = sqrtf( tr + 1 );
			r = 0.5f * s;
			s = 0.5f / s;
			x = (M(2,1) - M(1,2)) * s;
			y = (M(0,2) - M(2,0)) * s;
			z = (M(1,0) - M(0,1)) * s;
		}
		else
		{
			int32 i=0;

			if( M(1,1) > M(0,0) )
				i = 1;

			if( M(2,2) > M(i,i) )
				i = 2;

			switch( i )
			{
			case 0:
				{
					s = sqrtf( (M(0,0) - (M(1,1) + M(2,2))) + 1 );
					x = 0.5f * s;
					s = 0.5f / s;
					y = (M(0,1) + M(1,0)) * s;
					z = (M(2,0) + M(0,2)) * s;
					r = (M(2,1) - M(1,2)) * s;
				}
				break;
			case 1:
				{
					s = sqrtf( (M(1,1) - (M(2,2) + M(0,0))) + 1 );
					y = 0.5f * s;
					s = 0.5f / s;
					z = (M(1,2) + M(2,1)) * s;
					x = (M(0,1) + M(1,0)) * s;
					r = (M(0,2) - M(2,0)) * s;
				}
				break;
			case 2:
				{
					s = sqrtf( (M(2,2) - (M(0,0) + M(1,1))) + 1 );
					z = 0.5f * s;
					s = 0.5f / s;
					x = (M(2,0) + M(0,2)) * s;
					y = (M(1,2) + M(2,1)) * s;
					r = (M(1,0) - M(0,1)) * s;
				}
				break;
			};
		}
}


//EOF

// ----------------------------------------------------------------------- //
//
// MODULE  : CommonUtilities.cpp
//
// PURPOSE : Utility functions
//
// CREATED : 9/25/97
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "ltbasedefs.h"
#include "CommonUtilities.h"

// Globally available interface pointers

ILTModel*		g_pModelLT = LTNULL;
ILTTransform*	g_pTransLT = LTNULL;
ILTPhysics*		g_pPhysicsLT = LTNULL;
ILTCSBase*		g_pLTBase = LTNULL;

#ifdef _CLIENTBUILD
	ILTCommon *g_pCommonLT = NULL;
	define_holder_to_instance(ILTCommon, g_pCommonLT, Client);
#else // _CLIENTBUILD
	ILTCommon *g_pCommonLT = NULL;
	define_holder_to_instance(ILTCommon, g_pCommonLT, Server);
#endif // _CLIENTBUILD

// Temp buffer...

char s_FileBuffer[_MAX_PATH];

// Stolen from gamework.h...

int GetRandom()
{
	return(rand());
}

int GetRandom(int range)
{
	if (range == -1)	// check for divide-by-zero case
	{
		return((rand() % 2) - 1);
	}

	return(rand() % (range + 1));
}

int GetRandom(int lo, int hi)
{
	if ((hi - lo + 1) == 0)		// check for divide-by-zero case
	{
		if (rand() & 1) return(lo);
		else return(hi);
	}

	return((rand() % (hi - lo + 1)) + lo);
}

float GetRandom(float min, float max)
{
	float randNum = (float)rand() / RAND_MAX;
	float num = min + (max - min) * randNum;
	return num;
}


//-------------------------------------------------------------------------------------------
// Color255VectorToWord
//
// Converts a color in vector format to a word in 5-6-5 format.  Color ranges are 0-255.
// Arguments:
//		pVal - Color vector
// Return:
//      uint16 - converted color.
//-------------------------------------------------------------------------------------------
uint16 Color255VectorToWord( LTVector *pVal )
{
    uint16 wColor;

	// For red, multiply by 5 bits and divide by 8, which is a net division of 3 bits.  Then shift it
	// to the left 11 bits to fit into result, which is a net shift of 8 to left.
    wColor = ( uint16 )(((( uint32 )pVal->x & 0xFF ) << 8 ) & 0xF800 );

	// For green, multiply by 6 bits and divide by 8, which is a net division of 2 bits.  Then shift it
	// to the left 5 bits to fit into result, which is a net shift of 3 to left.
    wColor |= ( uint16 )(((( uint32 )pVal->y & 0xFF ) << 3 ) & 0x07E0 );

	// For blue, multiply by 5 bits and divide by 8 = divide by 3.
    wColor |= ( uint16 )((( uint32 )pVal->z & 0xFF ) >> 3 );

	return wColor;
}

//-------------------------------------------------------------------------------------------
// Color255WordToVector
//
// Converts a color in word format to a vector in 5-6-5 format.  Color ranges are 0-255.
// Arguments:
//		wVal - color word
//		pVal - Color vector
// Return:
//		void
//-------------------------------------------------------------------------------------------
void Color255WordToVector( uint16 wVal, LTVector *pVal )
{
	// For red, divide by 11 bits then multiply by 8 bits and divide by 5 bits = divide by 8 bits...
    pVal->x = ( LTFLOAT )(( wVal & 0xF800 ) >> 8 );

	// For green, divide by 5 bits, multiply by 8 bits, divide by 6 bits = divide by 3 bits.
    pVal->y = ( LTFLOAT )(( wVal & 0x07E0 ) >> 3 );

	// For blue, divide by 5 bits, multiply by 8 bits = multiply by 3 bits
    pVal->z = ( LTFLOAT )(( wVal & 0x001F ) << 3 );
}

// 
uint8 CompressRotationByte(LTRotation const *pRotation)
{
    LTVector forward = pRotation->Forward();
	float angle;
	char cAngle;

	angle = (float)atan2(forward.x, forward.z);
	cAngle = (char)(angle * (127.0f / MATH_PI));
    return (uint8)cAngle;
}


void UncompressRotationByte(uint8 rot, LTRotation *pRotation)
{
	float angle;

	angle = (float)(char)rot / 127.0f;
	angle *= MATH_PI;
	LTRotation rRotation;
	rRotation.Rotate(LTVector(0.0f, 1.0f, 0.0f), angle);
	*pRotation = rRotation;
}


uint16 CompressRotationShort( LTRotation const *pRotation )
{
	// send a more accurate version of the player's rotation
	float fAngle;
	int16 wYaw;

	// Get the forward vector...
	LTVector vForward = pRotation->Forward();

	// ...convert the xz rotation to an angle...
	fAngle = static_cast< float >( atan2( vForward.x, vForward.z ) );

	// ...scale it to the size of a short...
	wYaw = static_cast< int16 >( fAngle * SHRT_MAX / ( MATH_CIRCLE ) );

	// ...and return it.
	return static_cast< uint16 >( wYaw );
}


void UncompressRotationShort( uint16 uwRotation, LTRotation *pRotation )
{
	int16 wYaw;
	float fYaw;

	// get the yaw
	wYaw = static_cast< int16 >( uwRotation );

	// find the float representation of the new rotation
	fYaw = wYaw * MATH_CIRCLE / SHRT_MAX;

	// find the new rotation
	LTRotation rNewRotation( LTVector( 0.0f, 1.0f, 0.0f ), fYaw );
	*pRotation = rNewRotation;
}


uint8 CompressAngleToByte( float fPitch )
{
	//
	// pitch
	//

	// convert the pitch to the size of a unsigned char
	int8 cPitch =
		static_cast< int8 >( 
			fPitch *
			static_cast< float >( SCHAR_MAX ) /
			MATH_HALFPI
		);

	return static_cast< uint8 >( cPitch );
}



void UncompressAngleFromByte( uint8 cCompactedPitch, float *pfPitch )
{
	int8 cPitch;
	
	// get the pitch
	cPitch = static_cast< int8 >( cCompactedPitch );

	// remap it to a float
	*pfPitch = static_cast< float >( cPitch ) *
			MATH_HALFPI /
			static_cast< float >( SCHAR_MAX );
}


uint16 CompressAngleToShort( float fPitch )
{
	//
	// pitch
	//

	// convert the pitch to the size of a unsigned char
	int16 cPitch =
		static_cast< int16 >( 
			fPitch *
			static_cast< float >( SHRT_MAX ) /
			MATH_HALFPI
		);

	return static_cast< uint16 >( cPitch );
}



void UncompressAngleFromShort( uint16 cCompactedPitch, float *pfPitch )
{
	int16 cPitch;
	
	// get the pitch
	cPitch = static_cast< int16 >( cCompactedPitch );

	// remap it to a float
	*pfPitch = static_cast< float >( cPitch ) *
			MATH_HALFPI /
			static_cast< float >( SHRT_MAX );
}


bool CompressOffset( TVector3< short > *pCompressedOffset,
                     LTVector const &vOffset,
                     int nMaxVal )
{
	if ( 0 == pCompressedOffset )
	{
		// nowhere to put answer
		return false;
	}

	if ( ( ltfabsf( vOffset.x ) >= static_cast< float >( nMaxVal ) ) ||
	     ( ltfabsf( vOffset.y ) >= static_cast< float >( nMaxVal ) ) ||
	     ( ltfabsf( vOffset.z ) >= static_cast< float >( nMaxVal ) ) )
	{
		// source vector is too big
		return false;
	}

	pCompressedOffset->x = 
		static_cast< short >(
			SHRT_MAX * 
				( 
					vOffset.x /
					static_cast< float >( nMaxVal )
				)
		);

	pCompressedOffset->y = 
		static_cast< short >(
			SHRT_MAX * 
				( 
					vOffset.y /
					static_cast< float >( nMaxVal )
				)
		);

	pCompressedOffset->z = 
		static_cast< short >(
			SHRT_MAX * 
				( 
					vOffset.z /
					static_cast< float >( nMaxVal )
				)
		);

	return true;
}


bool UncompressOffset( LTVector *pOffset,
                       TVector3< short > const &pCompressedOffset,
                       int nMaxVal )
{
	if ( 0 == pOffset )
	{
		// nowhere to put answer
		return false;
	}

	pOffset->x = 
		static_cast< float >(
			static_cast< float >( nMaxVal ) *
			static_cast< float >( pCompressedOffset.x ) /
			static_cast< float >( SHRT_MAX )
		);

	pOffset->y = 
		static_cast< float >(
			static_cast< float >( nMaxVal ) *
			static_cast< float >( pCompressedOffset.y ) /
			static_cast< float >( SHRT_MAX )
		);

	pOffset->z = 
		static_cast< float >(
			static_cast< float >( nMaxVal ) *
			static_cast< float >( pCompressedOffset.z ) /
			static_cast< float >( SHRT_MAX )
		);

	return true;
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	GetVectorToLine()
//
//  PURPOSE:	Given a line defined by a start point and a vector,
//				and noncolinear 3rd point, return the shortest vector from
//				that 3rd point to the line.
//
// ----------------------------------------------------------------------- //

bool GetVectorToLine( LTVector const &vLineStart,
                      LTVector const &vLineDirection,
                      LTVector const &vPoint,
                      LTVector *vPointToLine )
{
	//
	// parameter check
	//

	// make sure the the direction is nonzero
	if ( vLineDirection.NearlyEquals( LTVector( 0.0f, 0.0f, 0.0f ) ) )
	{
		return false;
	}

	// make sure the result pointer is OK
	if ( 0 == vPointToLine )
	{
		return false;
	}

	//
	// find the point on the line closest to the target point
	//
	// Starting with the parametric equation:
	//    (1)    P1 = P0 + t * ( P2 - P0 )
	// where:
	//    P2 and P0 are known points on the line
	//    P1 is an unknown point on the line
	//    t is a number representing % along line from P0 to P1
	//
	// We want to find:
	//    (2)    ( P1 - P3 ) dot ( P2 - P0 ) = 0
	// where:
	//    P3 is a noncolinear point
	//    ( the rest of the variables are the same )
	//
	// Substituting (2) into (1) and solving for t yeilds:
	//    t = ( ( P3.x - P0.x ) * ( P2.x - P0.x ) +
	//          ( P3.y - P0.y ) * ( P2.y - P0.y ) +
	//          ( P3.z - P0.z ) * ( P2.z - P0.z ) ) /
	//        ( ( mag( P2 - P0 ) ^ 2 )
	//
	// For this function:
	//    P0 = vLineStart
	//    P1 = vTargetPoint
	//    P2 = vLineEnd
	//    P3 = vPoint
	//     t = fT
	// The answer is:
	//    P3 - P1 = vPointToLine

	// find another point on the line
	LTVector vLineEnd = vLineStart + vLineDirection;

	// find t
	LTFLOAT fT;
	fT = ( ( vPoint.x - vLineStart.x ) * ( vLineEnd.x - vLineStart.x ) +
	       ( vPoint.y - vLineStart.y ) * ( vLineEnd.y - vLineStart.y ) +
	       ( vPoint.z - vLineStart.z ) * ( vLineEnd.z - vLineStart.z ) ) /
	     ( VEC_MAGSQR( vLineEnd - vLineStart ) );

	// find P1
	LTVector vTargetPoint = vLineStart + fT * ( vLineEnd - vLineStart );

	// find a point a long way away for testing purposes
	LTVector vLongLineEnd = 500.0f * vLineDirection + vLineStart;

	// set the return value
	*vPointToLine = vTargetPoint - vPoint;

	return true;
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CaseInsensitiveCompare
//
//  PURPOSE:	Comparison function passed to qsort to sort an array of
//				strings in alphabetical order.
//
// ----------------------------------------------------------------------- //

int CaseInsensitiveCompare(const void *entry1, const void *entry2)
{
    return _stricmp(*(char **)entry1,*(char **)entry2);
}
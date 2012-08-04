// ----------------------------------------------------------------------- //
//
// MODULE  : RiotObjectUtilities.cpp
//
// PURPOSE : Utility functions
//
// CREATED : 9/25/97
//
// ----------------------------------------------------------------------- //

#include "RiotCommonUtilities.h"
#include "clientheaders.h"

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
//		uint16 - converted color.
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



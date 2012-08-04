// ----------------------------------------------------------------------- //
//
// MODULE  : RiotCommonUtilities.h
//
// PURPOSE : Utility functions
//
// CREATED : 5/4/98
//
// ----------------------------------------------------------------------- //

#ifndef __RIOTCOMMONUTILITIES_H__
#define __RIOTCOMMONUTILITIES_H__

#include "ltbasetypes.h"
#include "clientheaders.h"


uint16 Color255VectorToWord( LTVector *pVal );
void Color255WordToVector( uint16 wVal, LTVector *pVal );

int GetRandom();
int GetRandom(int range);
int GetRandom(int lo, int hi);
float GetRandom(float min, float max);


// Compress/decompress a rotation into a single byte.  This only accounts for 
// rotation around the Y axis.
uint8 CompressRotationByte(LTRotation *pRotation);
void UncompressRotationByte(uint8 rot, LTRotation *pRotation);



#endif // __RIOTCOMMONUTILITIES_H__



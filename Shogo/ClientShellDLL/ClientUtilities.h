// ----------------------------------------------------------------------- //
//
// MODULE  : ClientUtilities.h
//
// PURPOSE : Utility functions
//
// CREATED : 9/25/97
//
// ----------------------------------------------------------------------- //

#ifndef __CLIENT_UTILITIES_H__
#define __CLIENT_UTILITIES_H__

#include "RiotSoundTypes.h"
#include "clientheaders.h"
#include "RiotCommonUtilities.h"

#define NUM_COMMANDS 28

struct CommandID
{
	int		nStringID;
	int		nCommandID;
};

struct CSize
{
	CSize()		{ cx = 0; cy = 0; }
	
	uint32	cx;
	uint32	cy;
};

int CommandToArrayPos(int nCommand);
char* CommandName(int nCommand);

HLTSOUND PlaySoundFromObject( HOBJECT hObject, char *pSoundName, LTFLOAT fRadius, uint8 nSoundPriority, 
							 LTBOOL bLoop = LTFALSE, LTBOOL bHandle = LTFALSE, LTBOOL bTime = LTFALSE, uint8 nVolume = 100 );
HLTSOUND PlaySoundFromPos( LTVector *vPos, char *pSoundName, LTFLOAT fRadius, uint8 nSoundPriority, 
						  LTBOOL bLoop = LTFALSE, LTBOOL bHandle = LTFALSE, LTBOOL bTime = LTFALSE, uint8 nVolume = 100 );

HLTSOUND PlaySoundLocal( char *pSoundName, uint8 nSoundPriority, LTBOOL bLoop = LTFALSE, LTBOOL bHandle = LTFALSE, uint8 nVolume = 100, LTBOOL bReverb = LTFALSE );

HSURFACE CropSurface ( HSURFACE hSurf, HLTCOLOR hBorderColor );

#endif // __CLIENT_UTILITIES_H__
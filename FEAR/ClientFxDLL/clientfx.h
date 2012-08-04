//------------------------------------------------------------------
//
//   MODULE  : CLIENTFX.H
//
//   PURPOSE : Defines client fx stuff
//
//   CREATED : On 10/6/98 At 9:21:50 PM
//
//------------------------------------------------------------------

#ifndef __CLIENTFX_H__
#define __CLIENTFX_H__

// Includes....

#include "basefx.h"
#include "debugnew.h"
#include "iltdrawprim.h"
#include "iltsoundmgr.h"
#include "ClientFxProp.h"

// GetRandom(float, float) is defined in another module in SEM configurations
#if defined(PLATFORM_SEM)

float GetRandom(float min, float max);

#else // PLATFORM_SEM

inline float GetRandom( float min, float max )
{
	float randNum = (float)rand() / RAND_MAX;
	return (min + (max - min) * randNum);
}

#endif // !PLATFORM_SEM

inline uint32 GetRandom( uint32 min, uint32 max )
{
	LTASSERT(max >= min, "Error: Invalid parameters passed to GetRandom");
	return( (rand() % (max - min + 1)) + min );
}

inline void FindPerps(const LTVector& vPlaneDir, LTVector& vPerp1, LTVector& vPerp2)
{
	// Get coplanar perp vector to normalized direction
	vPerp1 = vPlaneDir.BuildOrthonormal();
	vPerp2 = vPerp1.Cross( vPlaneDir );
}

//this function will be called to create the named effect. The creation structure is used
//to provide parent and placement information, but the name is not used. Instead the name
//is passed in separately in the first parameter. This name will be parsed as a semicolon
//delimited list of client fx and created accordingly with the information that is provided.
//This will return true if all of the effects were able to be created, or false if any failed.
bool CreateNewFX(IClientFXMgr* pFxMgr, const char* pszEffectName, 
				 const CLIENTFX_CREATESTRUCT& CreateInfo, bool bStartInst);

//this function should be used to determine visibility between the object and the viewer. This will
//only test visibility against world models, and will properly handle when the object is in the sky
bool IsPointVisible(const LTVector& vSrc, const LTVector& vDest, bool bDestInSky);

//given a polygon, this will determine if it is a sky polygon or not.
bool IsSkyPoly(HPOLY hPoly);

#endif
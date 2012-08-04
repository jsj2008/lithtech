// ----------------------------------------------------------------------- //
//
// MODULE  : CommonUtilities.h
//
// PURPOSE : Utility functions
//
// CREATED : 5/4/98
//
// (c) 1998-2001 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __COMMON_UTILITIES_H__
#define __COMMON_UTILITIES_H__

#include "ltbasetypes.h"
#include "iltcommon.h"
#include "iltmodel.h"
#include "iltphysics.h"
#include "ilttransform.h"
#include "iltcsbase.h"

#pragma warning( disable : 4786 )
#include <set>
#include <string>
#include <list>
#include <vector>

#define ARRAY_LEN(array) (sizeof((array)) / sizeof((array)[0]))

#define WAVESTR_LINEAR	"LINEAR"
#define WAVESTR_SINE	"SINE"
#define WAVESTR_SLOWOFF	"SLOWOFF"
#define WAVESTR_SLOWON	"SLOWON"

#define DEFAULT_STAIRSTEP_HEIGHT	16.0
#define STAIR_STEP_HEIGHT_CVAR		"StairStepHeight"

// Externs

extern ILTModel* g_pModelLT;
extern ILTTransform* g_pTransLT;
extern ILTPhysics* g_pPhysicsLT;
extern ILTCommon* g_pCommonLT;
// Note : This is either g_pLTServer or g_pLTClient cast to ILTCSBase.
// (That's why it's not g_pBaseLT instead..)  This should NOT be confused with
// g_pCommonLT, which is the ILTCommon interface from g_pLTServer/g_pLTClient
extern ILTCSBase* g_pLTBase;

inline LTBOOL IsMainWorld(HOBJECT hObj)
{
	if (!g_pPhysicsLT) return LTFALSE;

	return (LT_YES == g_pPhysicsLT->IsWorldObject(hObj));
}

uint16 Color255VectorToWord( LTVector *pVal );
void Color255WordToVector( uint16 wVal, LTVector *pVal );

int GetRandom();
int GetRandom(int range);
int GetRandom(int lo, int hi);
float GetRandom(float min, float max);

// Compress/decompress a rotation into a single byte.  This only accounts for
// rotation around the Y axis.
uint8 CompressRotationByte(LTRotation const *pRotation);
void UncompressRotationByte(uint8 rot, LTRotation *pRotation);
uint16 CompressRotationShort(LTRotation const *pRotation);
void UncompressRotationShort( uint16 uwRotation, LTRotation *pRotation );
uint8 CompressAngleToByte(float const fPitch);
void UncompressAngleFromByte(uint8 cCompactedPitch, float *pfPitch);
uint16 CompressAngleToShort(float const fPitch);
void UncompressAngleFromShort(uint16 cCompactedPitch, float *pfPitch);

bool CompressOffset( TVector3< short > *pCompressedOffset,
                     LTVector const &vOffset,
                     int nMaxVal = 1000 );
bool UncompressOffset( LTVector *pOffset,
                       TVector3< short > const &pCompressedOffset,
                       int nMaxVal = 1000 );


#define SET_ARGB(a,r,g,b) (((uint32)(a) << 24) | (uint32)(r) << 16) | ((uint32)(g) << 8) | ((uint32)(b))
#define GET_A(val) (((val) >> 24) & 0xFF)
#define GET_ARGB(val, a, r, g, b) \
{\
    (a) = GET_A(val);\
    (r) = GETR(val);\
    (g) = GETG(val);\
    (b) = GETB(val);\
}


// String containers
class CaselessGreater
{
public:
	
	bool operator()(const std::string & x, const std::string & y) const
	{
		return (stricmp(x.c_str(), y.c_str()) > 0 );
	}
};

class CaselessLesser
{
public:
	
	bool operator()(const std::string & x, const std::string & y) const
	{
		return (stricmp(x.c_str(), y.c_str()) < 0 );
	}
};

typedef std::set<std::string,CaselessLesser> StringSet;
typedef std::list<std::string> StringList;
typedef std::vector<std::string> StringArray;



// ----------------------------------------------------------------------- //
//
//  ROUTINE:	uint32 str_Hash
//
//  PURPOSE:	Create a hash value for a string
//
// ----------------------------------------------------------------------- //

inline uint32 str_Hash( const char *pString )
{
	uint32 dwHash = 0;

	while( *pString )
	{
		dwHash *= 31;
		dwHash += (uint8)*pString;
		++pString;
	}

	return dwHash;
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

bool GetVectorToLine( LTVector const &vLineStartPoint,
                      LTVector const &vLineDirection,
                      LTVector const &vPoint,
                      LTVector *vPointToLine );

//
// LINKFROM_MODULE
// LINKTO_MODULE
//
// These macros can be used to ensure a module does not get optimized out by the linker when
// linking a static lib into a dll or exe.  If a module in a static lib contains symbols
// that are never referenced by the dll or exe linking with it, then the module will not
// be used in the link.  To ensure a module gets used establish a link between the unreferenced
// module and a known referenced module.  The referenced module can be part of the static lib
// as well, but it must be somehow connected to modules in the dll or exe.  The need for
// this comes from the use of class factory like architecture, where dummy objects use
// their constructors to do work, but the objects themselves are never referenced.
//
// To use, put the LINKFROM_MODULE(linkfrom_module_name) into the unreferenced module cpp, where 
// linkfrom_module_name is the name unreferenced module.  Then put the LINKTO_MODULE(linkto_module_name) into the
// known referenced module where the linkto_module_name is the name of the unreferenced module.
// 
#define LINKFROM_MODULE(linkfrom_module_name)					\
	int g_n##linkfrom_module_name = 0;

#define LINKTO_MODULE(linkto_module_name)						\
	extern int g_n##linkto_module_name;							\
	static int* s_pn##linkto_module_name = &g_n##linkto_module_name;



//
// PURE_VIRTUAL
//
// Use this macro instead of the '= 0' to designate a pure virtual function.  This gives you an easy
// way to catch unimplemented pure virtual calls.  Otherwise you have to debug into the CRT
// in the _purecall.
#ifdef _DEBUG
#define PURE_VIRTUAL	{ ASSERT( !"Pure virtual call made." ); }
#else
#define PURE_VIRTUAL	= 0;
#endif

inline LTVector ConvertToDEditPos( LTVector &vOrigPosition )
{
	LTVector vWorldOffsetFromDEditToRuntime = vOrigPosition;

#ifdef _CLIENTBUILD
	g_pLTClient->GetSourceWorldOffset( vWorldOffsetFromDEditToRuntime );
#else // !_CLIENTBUILD
	g_pLTServer->GetSourceWorldOffset( vWorldOffsetFromDEditToRuntime );
#endif // _CLIENTBUILD

	return vWorldOffsetFromDEditToRuntime + vOrigPosition;
}


int CaseInsensitiveCompare(const void *entry1, const void *entry2);


#endif // __COMMON_UTILITIES_H__
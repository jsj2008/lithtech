// ----------------------------------------------------------------------- //
//
// MODULE  : CommonUtilities.h
//
// PURPOSE : Utility functions
//
// CREATED : 5/4/98
//
// (c) 1998-2004 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __COMMON_UTILITIES_H__
#define __COMMON_UTILITIES_H__

#include "platform.h"
#include "ltbasetypes.h"
#include "iltcommon.h"
#include "iltmodel.h"
#include "iltphysics.h"
#include "iltcsbase.h"
#include "ColorUtilities.h"

// Make sure g_pLTServer & g_pLTClient are both declared in SEM configs
// Note: This is explicitly to get the inlined GetCurExecutionShellContext
// function to link properly.  If it operates based on the _CLIENTBUILD
// and _SERVERBUILD defines, it will sometimes link with the hard-wired 
// version and always report the wrong context.
#if defined(PLATFORM_SEM)
#include "iltclient.h"
extern ILTClient *g_pLTClient;
#include "iltserver.h"
extern ILTServer *g_pLTServer;
#endif

#pragma warning( disable : 4786 )
#include <set>
#include <string>
#include <list>
#include <vector>

#if defined(PLATFORM_LINUX)
#include <sys/linux/linux_stlcompat.h>
#else
#include <hash_map>
#endif

#define DEG2RAD(x)		(((x)*MATH_PI)/180.0f)
#define RAD2DEG(x)		(((x)*180.0f)/MATH_PI)

#define ARRAY_LEN(array) (sizeof((array)) / sizeof((array)[0]))

#define WAVESTR_LINEAR	"LINEAR"
#define WAVESTR_SINE	"SINE"
#define WAVESTR_SLOWOFF	"SLOWOFF"
#define WAVESTR_SLOWON	"SLOWON"

#define DEFAULT_STAIRSTEP_HEIGHT	40.0
#define STAIR_STEP_HEIGHT_CVAR		"StairStepHeight"

// Since there is no actual invalid animation tracker id the main tracker is used
// to signify a tracker has not been set up yet.  The define is just used to convey
// the invalid concept.
#define INVALID_TRACKER						MAIN_TRACKER

// Externs

extern ILTModel* g_pModelLT;
extern ILTPhysics* g_pPhysicsLT;
extern ILTCommon* g_pCommonLT;
// Note : This is either g_pLTServer or g_pLTClient cast to ILTCSBase.
// (That's why it's not g_pBaseLT instead..)  This should NOT be confused with
// g_pCommonLT, which is the ILTCommon interface from g_pLTServer/g_pLTClient
extern ILTCSBase* g_pLTBase;

inline bool IsMainWorld(HOBJECT hObj)
{
	if (!g_pPhysicsLT) return false;

	return (LT_YES == g_pPhysicsLT->IsWorldObject(hObj));
}

uint16 Color255VectorToWord( LTVector *pVal );
void Color255WordToVector( uint16 wVal, LTVector *pVal );

int GetRandom();
int GetRandom(int range);
int GetRandom(int lo, int hi);
float GetRandom(float min, float max);

//get a value from [0..1] based on the given frequency
float GetSinCycle( float fFrequency);


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

// Compress/decompress a rotation as pitch and roll into a 16 bit value...
uint16 CompressPitchRoll16( float fPitch, float fYaw );
void UncompressPitchRoll16( uint16 wRot, LTVector2 &v2PY );

// Compress/decompress a rotation as pitch and roll into a 32 bit value...
uint32 CompressPitchRoll32( float fPitch, float fYaw );
void UncompressPitchRoll32( uint32, LTVector2 &v2PY );

// String containers
class CaselessGreater
{
public:
	
	bool operator()(const std::string & x, const std::string & y) const
	{
		return (LTStrICmp(x.c_str(), y.c_str()) > 0 );
	}
};

class CaselessLesser
{
public:
	
	bool operator()(const std::string & x, const std::string & y) const
	{
		return (LTStrICmp(x.c_str(), y.c_str()) < 0 );
	}
	bool operator()(const std::wstring & x, const std::wstring & y) const
	{
		return (LTStrICmp(x.c_str(), y.c_str()) < 0 );
	}
};

// KEFTODO - Filter the category specification into the users of these types
typedef std::set<std::string,CaselessLesser,LTAllocator<std::string, LT_MEM_TYPE_GAMECODE> > StringSet;
typedef std::list<std::string,LTAllocator<std::string, LT_MEM_TYPE_GAMECODE> > StringList;
typedef std::vector<std::string,LTAllocator<std::string, LT_MEM_TYPE_GAMECODE> > StringArray;

typedef std::set<std::wstring,CaselessLesser,LTAllocator<std::string, LT_MEM_TYPE_GAMECODE> > WStringSet;
typedef std::list<std::wstring,LTAllocator<std::wstring, LT_MEM_TYPE_GAMECODE> > WStringList;
typedef std::vector<std::wstring,LTAllocator<std::wstring, LT_MEM_TYPE_GAMECODE> > WStringArray;




// ----------------------------------------------------------------------- //
//
//  ROUTINE:	uint32 str_Hash
//
//  PURPOSE:	Create a hash value for a string
//
// ----------------------------------------------------------------------- //

inline uint32 str_Hash( const wchar_t *pString )
{
	uint32 dwHash = 0;

	while( *pString )
	{
		dwHash *= 31;
		dwHash += (uint16)*pString;
		++pString;
	}

	return dwHash;
}

// ----------------------------------------------------------------------- //
//
// hash_map caseless string Traits structure.
//
// ----------------------------------------------------------------------- //

struct hash_map_str_nocase : public stdext::hash_compare< char const* >
{
	bool operator()(
		char const* pszLeft,
		char const* pszRight
	) const
	{
		return LTStrICmp( pszLeft, pszRight ) < 0;
	}

	size_t operator()(const char* str) const 
	{
	  size_t hash = 0; 
	  for ( ; *str; ++str)
		  hash = 5*hash + tolower(*str);
  
	  return hash;
	}
};


struct hash_map_stdstring_nocase : public stdext::hash_compare< std::string const >
{
	bool operator()(
		std::string const& sLeft,
		std::string const& sRight
	) const
	{
		return LTStrICmp( sLeft.c_str( ), sRight.c_str( )) < 0;
	}

	size_t operator()( std::string const& sValue ) const 
	{
		size_t hash = 0; 
		char const* pszValue = sValue.c_str();
		for ( ; *pszValue; ++pszValue )
			hash = 5*hash + tolower( *pszValue );
		
		return hash;
	}
};

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

int CaseInsensitiveCompare(const void *entry1, const void *entry2);


// Load/Save game defines...

#define LOAD_NEW_GAME				1	// Start from scratch - no saving or restoring
#define LOAD_NEW_LEVEL				2	// Save keep alives and level objects
#define LOAD_RESTORE_GAME			3	// No saving, but restore saved object
#define LOAD_TRANSITION				4	// Save transition objects to be loaded in new transition area

// Execution context, based on the active game shell
enum EExecutionShellContext 
{
	eExecutionShellContext_Client,
	eExecutionShellContext_Server,
	eExecutionShellContext_Unknown
};

// Retrieves the currently executing game shell context
inline EExecutionShellContext GetCurExecutionShellContext() 
{
#if defined(PLATFORM_SEM)
	// Compare g_pLTBase against the client and server interfaces to
	// decide which is active. Note: This checks for NULL first so it
	// will return _Unknown if either of the interfaces are also NULL.
	if (g_pLTBase == NULL)
		return eExecutionShellContext_Unknown;
	else if (g_pLTBase == g_pLTClient)
		return eExecutionShellContext_Client;
	else if (g_pLTBase == g_pLTServer)
		return eExecutionShellContext_Server;
	else
		return eExecutionShellContext_Unknown;
#elif defined(_CLIENTBUILD)
	return eExecutionShellContext_Client;
#elif defined(_SERVERBUILD)
	return eExecutionShellContext_Server;
#else
	return eExecutionShellContext_Unknown;
#endif
}

// Convenience macros to clean up checks for server / client builds
// Only appropriate where an if statement won't conflict with the underlying code
// On Xenon, both _SERVERBUILD and _CLIENTBUILD are defined, so
// be sure to check the shell context

#ifdef _SERVERBUILD
	#define SERVER_CODE(a) if (GetCurExecutionShellContext() == eExecutionShellContext_Server) { a }
#else
	#define SERVER_CODE(a)
#endif

#ifdef _CLIENTBUILD
	#define CLIENT_CODE(a) if (GetCurExecutionShellContext() == eExecutionShellContext_Client) { a }
#else
	#define CLIENT_CODE(a)
#endif

#if defined(_SERVERBUILD) || defined(_CLIENTBUILD)
	#define SERVER_AND_CLIENT_CODE(a) if (GetCurExecutionShellContext() != eExecutionShellContext_Unknown) { a }
#else
	#define SERVER_AND_CLIENT_CODE(a)
#endif


#if !defined(TRACE)
#ifdef _DEBUG
#define TRACE TraceHelper
#else
#define TRACE 
#endif

#if defined(PLATFORM_LINUX)
inline void TraceHelper(const char* pFormat, ...)
{
	char szOut[500];
	va_list marker;

	va_start(marker, pFormat);
	vsnprintf(szOut, 499, pFormat, marker);
	va_end(marker);

	printf( szOut );
}

#else

inline void TraceHelper(const char *pFormat, ...)
{
#ifdef OutputDebugString
	char szOut[500];
	va_list marker;

	va_start(marker, pFormat);
	_vsnprintf(szOut, 499, pFormat, marker);
	va_end(marker);

	OutputDebugString( szOut );
#endif // OutputDebugString
}

inline void	DebugAnim(const char* szFunction, HOBJECT hObj, ANIMTRACKERID nTrkID)
{
	if (!hObj)
	{
		return;
	}
	if (!szFunction)
	{
		szFunction = " ";
	}
	HMODELANIM hAnim = INVALID_MODEL_ANIM;
	HMODELWEIGHTSET hWeight = INVALID_MODEL_WEIGHTSET;

	LTRESULT res = g_pModelLT->GetCurAnim(hObj, nTrkID, hAnim);
	if (LT_OK != res)
	{
		TraceHelper("DebugAnim(%s): object tracker %d, unable to get cur anim - %d\n", szFunction, nTrkID, res);
		return;
	}
	res = g_pModelLT->GetWeightSet(hObj, nTrkID, hWeight);
	if (LT_OK != res)
	{
		TraceHelper("DebugAnim(%s): object tracker %d, unable to get weightset - %d\n", szFunction, nTrkID, res);
		return;
	}
	res = g_pModelLT->GetPlaying(hObj, nTrkID);


	if (LT_YES == res)
	{
		TraceHelper("DebugAnim(%s): object tracker %d, playing anim %d, weightset %d\n", szFunction, nTrkID, hAnim, hWeight);
	}
	else
	{
		TraceHelper("DebugAnim(%s): object tracker %d, not playing anim %d, weightset %d\n", szFunction, nTrkID, hAnim, hWeight);
	}


}

#endif // PLATFORM_LINUX
#endif // TRACE

// Internal macro used by DECLARE_SINGLETONxxx.  Must turn off
// inlining on the Instance method because of compiler bug which 
// ends up calling the constructor ever time Instance is called.
// See: http://groups.google.com/groups?hl=en&lr=&ie=UTF-8&oe=UTF-8&threadm=u0nRp5Qr%23GA.233%40cppssbbsa03&rnum=2&prev=/groups%3Fhl%3Den%26lr%3D%26ie%3DUTF-8%26oe%3DUTF-8%26q%3Dsingleton%2Bstatic%2Binline%2Bgroup%253Amicrosoft.public.vc.*%26btnG%3DSearch%26meta%3Dgroup%253Dmicrosoft.public.*
#define _DECLARE_SINGLETON_COMMON( type_name ) \
	private: \
		type_name( const type_name &other ); \
		type_name& operator=( const type_name &other ); \
	public: \
		NO_INLINE static type_name& type_name::Instance() \
		{ \
			static type_name s; \
			return s; \
		}

// Use this macro for declaring a class singleton.
#define DECLARE_SINGLETON( type_name ) \
	_DECLARE_SINGLETON_COMMON( type_name ); \
	private: \
		type_name(); \
		~type_name();

// Use this macro for declaring a class singleton that doesn't need a ctor/dtor.
#define DECLARE_SINGLETON_SIMPLE( type_name ) \
	_DECLARE_SINGLETON_COMMON( type_name ); \
	private: \
		type_name() { } \
		~type_name() { }

void PrintObjectFlags(HOBJECT hObj, char* pMsg=NULL);

bool GroundFilterFn(HOBJECT hObj, void *pUserData);
bool WorldFilterFn(HOBJECT hObj, void *pUserData);

// Retrieves the display name for a mod.
bool ModNameToModDisplayName( char const* pszModName, char* pszModDisplayName, uint32 nModDisplayNameSize );

// Makes gamevariant to/from modname.
bool ModNameToGameVariant( char const* pszModName, char* pszGameVariant, uint32 nGameVariantLen );
bool GameVariantToModName( char const* pszGameVariant, char* pszModName, uint32 nModNameLen );

// The CONTENT_WARNING macro is used to wrap any reporting that should not be
// done in release builds. This allows selectivly compiling out string
// generation in particular configurations.
// 
// An example use would be:
//
//	HRECORD hSoundTemplate = g_pModelsDB->GetModelSoundTemplate(hModel);
//	if (!hSoundTemplate)
//	{
//		CONTENT_WARNING(
//			const char* pszModelName = g_pLTDatabase->GetRecordName(hModel);
//			g_pLTBase->CPrint( "Failed to play CharacterSound '%s' (Model: '%s') - Could not get SoundTemplate from model.", 
//				pName, pszModelName ? pszModelName : "<invalid>" );
//		)
//		return NULL;
//	}

#ifndef _FINAL
	#define CONTENT_WARNING( x ) x
#else
	#define CONTENT_WARNING( x )
#endif

#endif // __COMMON_UTILITIES_H__

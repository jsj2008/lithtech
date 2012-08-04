// ----------------------------------------------------------------------- //
//
// MODULE  : CommonUtilities.cpp
//
// PURPOSE : Utility functions
//
// CREATED : 9/25/97
//
// (c) 1997-2005 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "ltbasedefs.h"
#include "CommonUtilities.h"
#include "ltobjectcreate.h"
#include "lttimeutils.h"
#include "ltfileoperations.h"

// Globally available interface pointers

ILTModel*		g_pModelLT = NULL;
ILTPhysics*		g_pPhysicsLT = NULL;
ILTCSBase*		g_pLTBase = NULL;
ILTCommon*		g_pCommonLT = NULL;


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
    pVal->x = ( float )(( wVal & 0xF800 ) >> 8 );

	// For green, divide by 5 bits, multiply by 8 bits, divide by 6 bits = divide by 3 bits.
    pVal->y = ( float )(( wVal & 0x07E0 ) >> 3 );

	// For blue, divide by 5 bits, multiply by 8 bits = multiply by 3 bits
    pVal->z = ( float )(( wVal & 0x001F ) << 3 );
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

	if ( ( LTAbs( vOffset.x ) >= static_cast< float >( nMaxVal ) ) ||
	     ( LTAbs( vOffset.y ) >= static_cast< float >( nMaxVal ) ) ||
	     ( LTAbs( vOffset.z ) >= static_cast< float >( nMaxVal ) ) )
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
//  ROUTINE:	CompressPitchRoll16
//
//  PURPOSE:	Compress a rotation as pitch and yaw into a 16 bit value...
//
// ----------------------------------------------------------------------- //

uint16 CompressPitchRoll16( float fPitch, float fYaw)
{
	// Clamp the values between -PI and PI...
	fPitch = LTMAX<float>( -MATH_PI/2.0f, LTMIN<float>( fPitch, MATH_PI/2.0f ));
	fYaw = LTMAX<float>( -MATH_PI/2.0f, LTMIN<float>( fYaw, MATH_PI/2.0f ));

	// Scale the values to [1,-1]...
	fPitch /= MATH_PI/2.0f;
	fYaw /= MATH_PI/2.0f;

	uint16 wRot = 0;

	// pack the pitch from the loword and yaw from the hiword.
	// Engine doesn't send hiword if zero.  Lean is less common,
	// so we put it in the hi.
	wRot = ((int8)(fYaw * SCHAR_MAX) << 8) & 0xFF00;
	wRot |= ((int8)(fPitch * SCHAR_MAX)) & 0x00FF;

	return wRot;
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	UncompressPitchRoll16
//
//  PURPOSE:	Decompress a rotation as pitch and yaw into a 16 bit value...
//
// ----------------------------------------------------------------------- //

void UncompressPitchRoll16( uint16 wRot, LTVector2 &v2PY )
{
	// pack the pitch from the loword and yaw from the hiword.
	// Engine doesn't send hiword if zero.  Lean is less common,
	// so we put it in the hi.
	float fYaw = (int8)((wRot & 0xFF00) >> 8) / (float)SCHAR_MAX;
	float fPitch = (int8)(wRot & 0x00FF) / (float)SCHAR_MAX;

	// Expnad pitch and yaw from scaled values...
	v2PY.Init( fPitch * MATH_PI / 2.0f, fYaw * MATH_PI / 2.0f );
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CompressPitchRoll32
//
//  PURPOSE:	Compress a rotation as pitch and yaw into a 32 bit value...
//
// ----------------------------------------------------------------------- //

uint32 CompressPitchRoll32( float fPitch, float fYaw)
{
	// Clamp the values between -PI and PI...
	fPitch = LTMAX<float>( -MATH_PI / 2.0f, LTMIN<float>( fPitch, MATH_PI / 2.0f ));
	fYaw = LTMAX<float>( -MATH_PI / 2.0f , LTMIN<float>( fYaw, MATH_PI / 2.0f ));

	// Scale the values to [1,-1]...
	fPitch /= MATH_PI / 2.0f;
	fYaw /= MATH_PI / 2.0f;

	uint32 wRot = 0;

	// pack the pitch from the loword and yaw from the hiword.
	// Engine doesn't send hiword if zero.  Lean is less common,
	// so we put it in the hi.
	wRot = ((int16)( fYaw * SHRT_MAX) << 16) & 0xFFFF0000;
	wRot |= ((int16)(fPitch * SHRT_MAX)) & 0x0000FFFF;

	return wRot;
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	UncompressPitchRoll32
//
//  PURPOSE:	Decompress a rotation as pitch and yaw into a 32 bit value...
//
// ----------------------------------------------------------------------- //

void UncompressPitchRoll32( uint32 wRot, LTVector2 &v2PY )
{
	// Unpack the pitch from the loword and yaw from the hiword.
	// Engine doesn't send hiword if zero.  Lean is less common,
	// so we put it in the hi.
	float fYaw = (int16)((wRot & 0xFFFF0000) >> 16) / (float)SHRT_MAX;
	float fPitch = (int16)(wRot & 0x0000FFFF) / (float)SHRT_MAX;

	// Expnad pitch and yaw from scaled values...
	v2PY.Init( fPitch * MATH_PI / 2.0f, fYaw * MATH_PI / 2.0f);
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
    return LTStrICmp(*(char **)entry1,*(char **)entry2);
}

//get a value from [0..1] based on the given frequency
float GetSinCycle(float fFrequency)
{
	static float fRadiansPerTick = MATH_TWOPI / 1000.0f;
	float fRadians = (float)LTTimeUtils::GetTimeMS() * fFrequency * fRadiansPerTick;
	return (1.0f + sinf(fRadians)) / 2.0f;
}

// Print out the passed in object's flags and flags2 to the console in a 
// human readable format...
void PrintObjectFlags(HOBJECT hObj, char* pMsg/*=NULL*/)
{
#ifdef _DEBUG
	if (!hObj)
	{
		g_pLTBase->CPrint("Invalid Object passed to PrintObjectFlags(%s)!", pMsg ? pMsg : "NULL");
		return;
	}

	// For now just print out the normal flags...in the future this should
	// probably be updated to support printing out the user, and client
	// flags..

	SERVER_CODE
	(
		char szName[64];
		szName[0] = '\0';
		g_pLTServer->GetObjectName(hObj, szName, sizeof(szName));
		g_pLTBase->CPrint("%s (%s) (%i) Flags:", (pMsg ? pMsg : "Object"), szName, hObj );
	)
	CLIENT_CODE
	(
		g_pLTBase->CPrint("%s <name not available on client> (%i) Flags:", (pMsg ? pMsg : "Object"), hObj );
	)

	uint32 dwFlags;
	g_pCommonLT->GetObjectFlags(hObj, OFT_Flags, dwFlags);

	if (!dwFlags)
	{
		g_pLTBase->CPrint("  No Flags Set!");
		return;
	}

	// Yes this could be cleaner, but I'm in a hurry...
	if (dwFlags & FLAG_VISIBLE)
	{
		g_pLTBase->CPrint("  Visible");
	}
	if (dwFlags & FLAG_FULLPOSITIONRES)
	{
		g_pLTBase->CPrint("  Full Position Res");
	}
	if (dwFlags & FLAG_NOLIGHT)
	{
		g_pLTBase->CPrint("  No Light");
	}
	if (dwFlags & FLAG_YROTATION)
	{ 
		g_pLTBase->CPrint("  Y Rotation");
	}
	if (dwFlags & FLAG_RAYHIT)
	{
		g_pLTBase->CPrint("  Ray Hit");
	}
	if (dwFlags & FLAG_SOLID)
	{
		g_pLTBase->CPrint("  Solid");
	}
	if (dwFlags & FLAG_BOXPHYSICS)
	{
		g_pLTBase->CPrint("  Box Physics");
	}
	if (dwFlags & FLAG_CLIENTNONSOLID)
	{
		g_pLTBase->CPrint("  Client Non Solid");
	}
	if (dwFlags & FLAG_TOUCH_NOTIFY)
	{
		g_pLTBase->CPrint("  Touch Notify");
	}
	if (dwFlags & FLAG_GRAVITY)
	{
		g_pLTBase->CPrint("  Gravity");
	}
	if (dwFlags & FLAG_GOTHRUWORLD)
	{
		g_pLTBase->CPrint("  Go Thru World");
	}
	if (dwFlags & FLAG_DONTFOLLOWSTANDING)
	{
		g_pLTBase->CPrint("  Don't Follow Standing");
	}
	if (dwFlags & FLAG_NOSLIDING)
	{
		g_pLTBase->CPrint("  No Sliding");
	}
	if (dwFlags & FLAG_POINTCOLLIDE)
	{
		g_pLTBase->CPrint("  Point Collide");
	}
	if (dwFlags & FLAG_MODELKEYS)
	{
		g_pLTBase->CPrint("  Model Keys");
	}
	if (dwFlags & FLAG_TOUCHABLE)
	{
		g_pLTBase->CPrint("  Touchable");
	}
	if (dwFlags & FLAG_FORCECLIENTUPDATE)
	{
		g_pLTBase->CPrint("  Force Client Update");
	}
	if (dwFlags & FLAG_REMOVEIFOUTSIDE)
	{
		g_pLTBase->CPrint("  Remove If Outside");
	}
	if (dwFlags & FLAG_NOTINWORLDTREE)
	{
		g_pLTBase->CPrint("  Not In World Tree");
	}
	if (dwFlags & FLAG_CONTAINER)
	{
		g_pLTBase->CPrint("  Container");
	}

	g_pCommonLT->GetObjectFlags(hObj, OFT_Flags2, dwFlags);

	if (!dwFlags)
	{
		g_pLTBase->CPrint(" No Flags 2 Set!");
		return;
	}
	else
	{
		g_pLTBase->CPrint(" Flags 2:");
	}
	if (dwFlags & FLAG2_PLAYERCOLLIDE)
	{
		g_pLTBase->CPrint("  Player Collide");
	}
	if (dwFlags & FLAG2_SKYOBJECT)
	{
		g_pLTBase->CPrint("  Sky Object");
	}
	if (dwFlags & FLAG2_FORCETRANSLUCENT)
	{
		g_pLTBase->CPrint("  Force Translucent");
	}
	if (dwFlags & FLAG2_DISABLEPREDICTION)
	{
		g_pLTBase->CPrint("  Disable Prediction");
	}
	if (dwFlags & FLAG2_SERVERDIMS)
	{
		g_pLTBase->CPrint("  Server Dims");
	}
	if (dwFlags & FLAG2_SPECIALNONSOLID)
	{
		g_pLTBase->CPrint("  Special Non Solid");
	}
	if (dwFlags & FLAG2_RIGIDBODY)
	{
		g_pLTBase->CPrint("  Rigid Body");
	}
	if (dwFlags & FLAG2_CLIENTRIGIDBODY)
	{
		g_pLTBase->CPrint("  Client Rigid Body");
	}
	if (dwFlags & FLAG2_PLAYERSTAIRSTEP)
	{
		g_pLTBase->CPrint("  Stair Step");
	}

#endif // _DEBUG
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	WordFilterFn
//
//	PURPOSE:	Filters out everything but the world
//
// ----------------------------------------------------------------------- //

bool WorldFilterFn(HOBJECT hObj, void* /*pUserData*/)
{
	if ( IsMainWorld(hObj) )
	{
		return true;
	}

	return false;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	GroundFilterFn
//
//	PURPOSE:	Filters out everything but potential ground candidates
//
// ----------------------------------------------------------------------- //

bool GroundFilterFn(HOBJECT hObj, void* /*pUserData*/)
{
	SERVER_CODE
	(
		// Filter out ladders.
		HCLASS hLadder = g_pLTServer->GetClass("Ladder");
		if (g_pLTServer->IsKindOf(g_pLTServer->GetObjectClass(hObj), hLadder))
		{
			return false;
		}

		// Filter out special move objects.
		HCLASS hSpecialMove = g_pLTServer->GetClass("SpecialMove");
		if (g_pLTServer->IsKindOf(g_pLTServer->GetObjectClass(hObj), hSpecialMove))
		{
			return false;
		}

		//!!ARL: Filter out forensic objects?
	)

	// Filter out anything that is physically simulated.  Objects that are 
	// simulated aren't 'ground', as they could potentially move.

	uint32 dwFlags = 0;
	g_pCommonLT->GetObjectFlags(hObj, OFT_Flags2, dwFlags );
	if ( 0 != (dwFlags & FLAG2_RIGIDBODY ) )
	{
		return false;
	}

	if ( IsMainWorld(hObj) || (OT_WORLDMODEL == GetObjectType(hObj)) )
	{
		return true;
	}

	return false;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ModNameToModDisplayName
//
//	PURPOSE:	Retrieves the display name for a mod from the mod values.
//
// ----------------------------------------------------------------------- //

bool ModNameToModDisplayName( char const* pszModName, char* pszModDisplayName, uint32 nModDisplayNameSize )
{
	if( !pszModDisplayName || nModDisplayNameSize == 0 )
		return false;

	// Initialize the out parameter
	pszModDisplayName[0] = '\0';

	char szFileTitle[MAX_PATH*2];
	LTFileOperations::SplitPath( pszModName, NULL, szFileTitle, NULL );

	LTStrCpy( pszModDisplayName, szFileTitle, nModDisplayNameSize );
	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ModNameToGameVariant
//
//	PURPOSE:	Makes gamevariant from modname.
//
// ----------------------------------------------------------------------- //

bool ModNameToGameVariant( char const* pszModName, char* pszGameVariant, uint32 nGameVariantLen )
{
	if( !pszGameVariant || nGameVariantLen == 0 )
		return false;

	// Initialize the out parameter
	pszGameVariant[0] = '\0';

	LTSNPrintF( pszGameVariant, nGameVariantLen, "-archcfg %s", pszModName );

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	GameVariantToModName
//
//	PURPOSE:	Makes modname from gamevariant
//
// ----------------------------------------------------------------------- //

bool GameVariantToModName( char const* pszGameVariant, char* pszModName, uint32 nModNameLen )
{
	if( !pszModName || nModNameLen == 0 )
		return false;

	// Initialize the out parameter
	pszModName[0] = '\0';

	ConParse parse;
	parse.Init(pszGameVariant);
	if( g_pCommonLT->Parse(&parse) == LT_OK && parse.m_nArgs >= 2 )
	{
		LTStrCpy( pszModName, parse.m_Args[1], nModNameLen );
	}

	return true;
}

// EOF


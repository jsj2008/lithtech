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
#include "CommonUtilities.h"
#include "ltbasedefs.h"

// Model and transform interface pointers

ILTMath*             g_pMathLT = LTNULL;
ILTModel*            g_pModelLT = LTNULL;
ILTTransform*        g_pTransLT = LTNULL;
ILTPhysics*          g_pPhysicsLT = LTNULL;
ILTCSBase*			 g_pBaseLT = LTNULL;

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
// WAVE FUNCTIONS.
//-------------------------------------------------------------------------------------------
WaveFn GetWaveFn(WaveType waveType)
{
	if(waveType == Wave_Linear)
		return WaveFn_Linear;
	else if(waveType == Wave_Sine)
		return WaveFn_Sine;
	else if(waveType == Wave_SlowOff)
		return WaveFn_SlowOff;
	else
		return WaveFn_SlowOn;
}

WaveType ParseWaveType(char *pStr)
{
	if(stricmp(pStr, WAVESTR_LINEAR) == 0)
	{
		return Wave_Linear;
	}
	else if(stricmp(pStr, WAVESTR_SINE) == 0)
	{
		return Wave_Sine;
	}
	else if(stricmp(pStr, WAVESTR_SLOWOFF) == 0)
	{
		return Wave_SlowOff;
	}
	else if(stricmp(pStr, WAVESTR_SLOWON) == 0)
	{
		return Wave_SlowOn;
	}
	else
	{
		// Default..
		return Wave_Linear;
	}
}

float WaveFn_Linear(float val)
{
	return val;
}

float WaveFn_Sine(float val)
{
	val = MATH_HALFPI + val * MATH_PI;	// PI/2 to 3PI/2
	val = (float)sin(val);				// 1 to -1
	val = -val;							// -1 to 1
	val = (val + 1.0f) * 0.5f;			// 0 to 1
	return val;
}

float WaveFn_SlowOff(float val)
{
	if(val < 0.5f)
		return WaveFn_Linear(val);
	else
		return WaveFn_Sine(val);
}

float WaveFn_SlowOn(float val)
{
	if(val < 0.5f)
		return WaveFn_Sine(val);
	else
		return WaveFn_Linear(val);
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

uint8 CompressRotationByte(ILTCommon *pCommon, LTRotation *pRotation)
{
    LTVector up, right, forward;
	float angle;
	char cAngle;

	pCommon->GetRotationVectors(*pRotation, up, right, forward);

	angle = (float)atan2(forward.x, forward.z);
	cAngle = (char)(angle * (127.0f / MATH_PI));
    return (uint8)cAngle;
}


void UncompressRotationByte(ILTCommon *pCommon, uint8 rot, LTRotation *pRotation)
{
	float angle;

	angle = (float)(char)rot / 127.0f;
	angle *= MATH_PI;
	pCommon->SetupEuler(*pRotation, 0.0f, angle, 0.0f);
}



void ButeToConsoleFloat( CButeMgr &bute, const char *pszButeTag, const char *pszButeAttr, const char *pszConsoleVar )
{
	float fVal;

	fVal = bute.GetFloat( pszButeTag, pszButeAttr );
	if( bute.Success( ))
		WriteConsoleFloat(( char * )pszConsoleVar, fVal );
}

void ButeToConsoleString( CButeMgr &bute, const char *pszButeTag, const char *pszButeAttr, const char *pszConsoleVar )
{
	CString sVal;

	sVal = bute.GetString( pszButeTag, pszButeAttr );
	if( bute.Success( ))
		WriteConsoleString(( char * )pszConsoleVar, ( char * )( const char * )sVal );
}

void ConsoleToButeFloat( CButeMgr &bute, const char *pszButeTag, const char *pszButeAttr, const char *pszConsoleVar )
{
	float fVal;

	fVal = GetConsoleFloat(( char * )pszConsoleVar, 0.0f );
	bute.SetFloat( pszButeTag, pszButeAttr, fVal );
}

void ConsoleToButeString( CButeMgr &bute, const char *pszButeTag, const char *pszButeAttr, const char *pszConsoleVar )
{
	char szVal[MAX_PATH*2];

	GetConsoleString(( char * )pszConsoleVar, szVal, "" );
	bute.SetString( pszButeTag, pszButeAttr, szVal );
}

void ReadNetHostSettings( )
{
	CButeMgr hostBute;
	int nVal;
	int nOption;
	OPTION *pOption;
	CString sVal;

	// Read the host settings from the bute file and put them into console variables.
	if( !hostBute.Parse( "NetHost.txt" ))
		return;

	ButeToConsoleFloat( hostBute, "Host", "NetGameType", "NetGameType" );
	ButeToConsoleFloat( hostBute, "Host", "NetMaxPlayers", "NetMaxPlayers" );
	ButeToConsoleString( hostBute, "Host", "NetSessionName", "NetSessionName" );
	ButeToConsoleString( hostBute, "Host", "NetPassword", "NetPassword" );
	ButeToConsoleFloat( hostBute, "Host", "NetUsePassword", "NetUsePassword" );
	ButeToConsoleFloat( hostBute, "Host", "NetCANumLevels", "NetCANumLevels" );
	ButeToConsoleFloat( hostBute, "Host", "NetNumLevels", "NetNumLevels" );
	ButeToConsoleFloat( hostBute, "Host", "SendBandwidth", "SendBandwidth" );

	nVal = GetConsoleInt( "NetNumLevels", 0 );
	while( nVal >= 0 )
	{
		sVal.Format( "NetLevel%i", nVal );
		ButeToConsoleString( hostBute, "Host", sVal, sVal );
		nVal--;
	}

	nVal = GetConsoleInt( "NetCANumLevels", 0 );
	while( nVal >= 0 )
	{
		sVal.Format( "NetCALevel%i", nVal );
		ButeToConsoleString( hostBute, "Host", sVal, sVal );
		nVal--;
	}

	if( g_pServerOptionMgr )
	{
		for( nOption = 0; nOption < g_pServerOptionMgr->GetNumOptions( ); nOption++ )
		{
			pOption = g_pServerOptionMgr->GetOption( nOption );
			if( !pOption )
			{
				ASSERT( FALSE );
				continue;
			}

			ButeToConsoleFloat( hostBute, "Host", pOption->szVariable, pOption->szVariable );
		}
	}
}

void WriteNetHostSettings( )
{
	CButeMgr hostBute;
	int nVal;
	int nOption;
	OPTION *pOption;
	CString sVal;

	// Write the host settings from the console variables and put them in the bute file.
	hostBute.Parse( "NetHost.txt" );

	ConsoleToButeFloat( hostBute, "Host", "NetGameType", "NetGameType" );
	ConsoleToButeFloat( hostBute, "Host", "NetMaxPlayers", "NetMaxPlayers" );
	ConsoleToButeString( hostBute, "Host", "NetSessionName", "NetSessionName" );
	ConsoleToButeString( hostBute, "Host", "NetPassword", "NetPassword" );
	ConsoleToButeFloat( hostBute, "Host", "NetUsePassword", "NetUsePassword" );
	ConsoleToButeFloat( hostBute, "Host", "NetCANumLevels", "NetCANumLevels" );
	ConsoleToButeFloat( hostBute, "Host", "NetNumLevels", "NetNumLevels" );
	ConsoleToButeFloat( hostBute, "Host", "SendBandwidth", "SendBandwidth" );

	nVal = GetConsoleInt( "NetNumLevels", 0 );
	while( nVal >= 0 )
	{
		sVal.Format( "NetLevel%i", nVal );
		ConsoleToButeString( hostBute, "Host", sVal, sVal );
		nVal--;
	}

	nVal = GetConsoleInt( "NetCANumLevels", 0 );
	while( nVal >= 0 )
	{
		sVal.Format( "NetCALevel%i", nVal );
		ConsoleToButeString( hostBute, "Host", sVal, sVal );
		nVal--;
	}

	if( g_pServerOptionMgr )
	{
		for( nOption = 0; nOption < g_pServerOptionMgr->GetNumOptions( ); nOption++ )
		{
			pOption = g_pServerOptionMgr->GetOption( nOption );
			if( !pOption )
			{
				ASSERT( FALSE );
				continue;
			}

			ConsoleToButeFloat( hostBute, "Host", pOption->szVariable, pOption->szVariable );
		}
	}

	hostBute.Save( "NetHost.txt" );
}

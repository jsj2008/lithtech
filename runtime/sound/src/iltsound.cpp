//	===========================================================================

#include <windows.h>
#include "iltsound.h"
// ---
#include "string.h"

#ifdef USE_ABSTRACT_SOUND_INTERFACES

// sound factory abstract base class
// used to generate ILTSound* platform dependent interface instances

ILTSoundFactory* ILTSoundFactory::m_pSoundFactory = NULL;

ILTSoundFactory* ILTSoundFactory::GetSoundFactory( )
{
	return m_pSoundFactory;
}

// generics

bool ILTSoundFactory::EnumSoundSystems( FnEnumSoundSysCallback fnEnumCallback, void* pUserData )
{
	char acSoundSysNames[1024] = { 0, 0 };
	
	if( !fnEnumCallback )
		return false;
	
	bool bSuccess = FillSoundSystems( &acSoundSysNames[0], 1023 );
	if( !bSuccess )
		return false;
	
	char* pcSoundSysNames = &acSoundSysNames[0];
	while( pcSoundSysNames[0] != 0 )
	{
		char* pcSoundSysName = pcSoundSysNames;
		size_t nSoundSysNameLen = strlen( pcSoundSysName );
		
		char* pcSoundSysDesc = pcSoundSysName + nSoundSysNameLen + 1;
		size_t nSoundSysDescLen = strlen( pcSoundSysDesc );

		bool bContinue = fnEnumCallback( pcSoundSysName, pcSoundSysDesc, pUserData );
		if( !bContinue )
			return true;

		pcSoundSysNames = pcSoundSysDesc + nSoundSysDescLen + 1;
	}

	return true;
}

// helper class for caching existing sound systems

struct soundSysNode_t
{
public:
	soundSysNode_t( const char* pcSoundSysName, ILTSoundSys* pSoundSys, soundSysNode_t* pNext )
	{
		m_pNext = pNext;
		m_pSoundSys = pSoundSys;

		size_t soundSysNameLen = strlen( pcSoundSysName );
		LT_MEM_TRACK_ALLOC(m_pcSoundSysName = new char[ soundSysNameLen + 1 ],LT_MEM_TYPE_SOUND);
		strcpy( m_pcSoundSysName, pcSoundSysName );
	}

	~soundSysNode_t( )
	{
		if( m_pNext != NULL )
			delete m_pNext;
		
		delete[] m_pcSoundSysName;
	}

public:
	ILTSoundSys* Find( const char* pcSoundSysName )
	{
		if( pcSoundSysName == NULL )
			return NULL;
		
		if( stricmp( pcSoundSysName, m_pcSoundSysName ) == 0 )
			return m_pSoundSys;
		
		if( m_pNext != NULL )
			return m_pNext->Find( pcSoundSysName );

		return NULL;
	}

public:
	char* m_pcSoundSysName;
	ILTSoundSys* m_pSoundSys;
	soundSysNode_t* m_pNext;
};

// helper macro for creating new ILTSoundFactory implementors

#define DECLARE_SOUND_FACTORY( _factName_ )	\
	class C##_factName_##SoundFactory : public ILTSoundFactory	\
	{	\
	public:	\
		C##_factName_##SoundFactory( );	\
		virtual ~C##_factName_##SoundFactory( );	\
		virtual bool FillSoundSystems( char* pcSoundSysNames, uint uiMaxStringLen );	\
		virtual ILTSoundSys* MakeSoundSystem( const char* pcSoundSystemName );	\
	public:	\
		soundSysNode_t* m_pSoundSysNodes;	\
		static C##_factName_##SoundFactory m_soundFactory;	\
	};	\
	C##_factName_##SoundFactory C##_factName_##SoundFactory::m_soundFactory;	\
	C##_factName_##SoundFactory::C##_factName_##SoundFactory( )	\
	{	\
		m_pSoundFactory = this;	\
		m_pSoundSysNodes = NULL;	\
	}	\
	C##_factName_##SoundFactory::~C##_factName_##SoundFactory( )	\
	{	\
		m_pSoundFactory = NULL;	\
		delete m_pSoundSysNodes;	\
	}	\

// ============================================================================

#ifdef WIN32
#ifndef __XBOX

// ============================================================================

// WIN32 implementation of the sound factory class
// uses the WIN32 APIs to interface with DLLs for generating ILTSound* interface instances

#include "windows.h"
#include "stdio.h"

typedef char*			( *SoundSysDescFn )( );
typedef ILTSoundSys*	( *SoundSysMakeFn )( );

extern "C"
{
	__declspec( dllimport ) char*			SoundSysDesc( );
	__declspec( dllimport ) ILTSoundSys*	SoundSysMake( );
}

#define DESC_FN_NAME	"SoundSysDesc"
#define MAKE_FN_NAME	"SoundSysMake"

DECLARE_SOUND_FACTORY( Win32 )

// the enumerator override

#define SOUND_DRIVER_DLL	"SndDrv.dll";

bool CWin32SoundFactory::FillSoundSystems( char* pcSoundSysNames, uint uiMaxStringLen )
{

	if( uiMaxStringLen < 2 || pcSoundSysNames == NULL )
		return false;

	static char* pcSoundSysNameWildcard = SOUND_DRIVER_DLL;

	WIN32_FIND_DATA tFileFindData;
	memset( &tFileFindData, 0, sizeof( tFileFindData ) );

	// start a search for sound DLL files

	char cCurrentDir[_MAX_PATH];
	char cProcessDir[_MAX_PATH];
	
	char cDrive[_MAX_PATH];
	char cDir[_MAX_PATH];
	char cFName[_MAX_PATH];
	char cExt[_MAX_PATH];

	GetCurrentDirectory(_MAX_PATH, cCurrentDir);
	GetModuleFileName(NULL, cProcessDir, _MAX_PATH);

	_splitpath(cProcessDir, cDrive, cDir, cFName, cExt);

	SetCurrentDirectory(cDir);

	HANDLE hFileFindHandle = ::FindFirstFile( pcSoundSysNameWildcard, &tFileFindData );
	if( hFileFindHandle == INVALID_HANDLE_VALUE )
	{
		SetCurrentDirectory(cCurrentDir);

		// Try looking in the current directory
		hFileFindHandle = ::FindFirstFile( pcSoundSysNameWildcard, &tFileFindData );
		if( hFileFindHandle == INVALID_HANDLE_VALUE )
			return false;
	}

	// stop the search for files
	FindClose( hFileFindHandle );

	// try to load the sound library DLL

	HINSTANCE hLibInstance = ::LoadLibrary( tFileFindData.cFileName );
	if( hLibInstance == NULL )
		return false;

	// try to find the DLL's descriptor function

	SoundSysDescFn fnSoundSysDesc = ( SoundSysDescFn )::GetProcAddress( hLibInstance, DESC_FN_NAME );
	if( fnSoundSysDesc == NULL )
		return false;

	// append the sound library DLL name

	strcpy( pcSoundSysNames, tFileFindData.cFileName );
	pcSoundSysNames += strlen( pcSoundSysNames ) + 1;

	// append the DLL descriptor

	strcpy( pcSoundSysNames, fnSoundSysDesc( ) );
	pcSoundSysNames += strlen( pcSoundSysNames ) + 1;

	pcSoundSysNames[0] = 0;


	// Set the current working directory back to where it was originally
	SetCurrentDirectory(cCurrentDir);

	return true;

}

// the creator override

bool EnumSoundSysCallback( const char* pcSoundSysName, const char* pcSoundSysDesc, void* pUserData )
{
	char* pcEarliestSoundSysName = ( char* )pUserData;

	if( pcEarliestSoundSysName[0] == 0 || 
		strcmp( pcSoundSysName, pcEarliestSoundSysName ) < 0)
		strcpy( pcEarliestSoundSysName, pcSoundSysName );

	return true;
}

void DisplaySoundSysError( const char* pcErrorString )
{
	MessageBox( NULL, pcErrorString, "ILTSoundSys error ...", MB_OK );
}

#define TERM_MSG	"Execution may terminate..."

ILTSoundSys* CWin32SoundFactory::MakeSoundSystem( const char* pcSoundSystemName )
{
	static char pcDefaultSoundSysName[1024] = { 0 };

	// if no name provided, enumerate and choose the first

	if( pcSoundSystemName == NULL )
	{
		EnumSoundSystems( EnumSoundSysCallback, &pcDefaultSoundSysName[0] );
		if( pcDefaultSoundSysName[0] == 0 )
		{
			char acErrorString[1024];
			sprintf( acErrorString, "Could not find suitable .SND sound driver.\n%s",
				TERM_MSG );
			DisplaySoundSysError( acErrorString );
			return NULL;
		}
		pcSoundSystemName = &pcDefaultSoundSysName[0];
	}

	// see if we already have an interface made for that name of sound system

	if( m_pSoundSysNodes != NULL )
	{
		ILTSoundSys* pSoundSys = m_pSoundSysNodes->Find( pcSoundSystemName );
		if( pSoundSys != NULL )
		{
			return pSoundSys;
		}
	}

	// try to load the sound library DLL

	HINSTANCE hLibInstance = ::LoadLibrary( pcSoundSystemName );
	if( hLibInstance == NULL )
	{
		char acErrorString[1024];
		sprintf( acErrorString, "Failed to load sound driver \'%s\'.\n%s",
			pcSoundSystemName,
			TERM_MSG );
		DisplaySoundSysError( acErrorString );
		return NULL;
	}

	// try to find the DLL's creator function

	SoundSysMakeFn fnSoundSysMake = ( SoundSysMakeFn )::GetProcAddress( hLibInstance, MAKE_FN_NAME );
	if( fnSoundSysMake == NULL )
	{
		char acErrorString[1024];
		sprintf( acErrorString, "Failed to find func \'%s\' for sound driver \'%s\'.\n%s",
			MAKE_FN_NAME,
			pcSoundSystemName,
			TERM_MSG );
		DisplaySoundSysError( acErrorString );
		return NULL;
	}

	// add it to the list of used sound systems

	ILTSoundSys* pSoundSys = fnSoundSysMake( );
	if( pSoundSys == NULL )
	{
		char acErrorString[1024];
		sprintf( acErrorString, "Failed to instantiate sound driver \'%s\'.\n%s",
			pcSoundSystemName,
			TERM_MSG );
		DisplaySoundSysError( acErrorString );
		return NULL;
	}

	if( !pSoundSys->Init( ) )
	{
		char acErrorString[1024];
		sprintf( acErrorString, "Failed to initialize sound driver \'%s\'.\n%s",
			pcSoundSystemName,
			TERM_MSG );
		DisplaySoundSysError( acErrorString );
		return NULL;
	}
	LT_MEM_TRACK_ALLOC(m_pSoundSysNodes = new soundSysNode_t( pcSoundSystemName, pSoundSys, m_pSoundSysNodes ),LT_MEM_TYPE_SOUND);
	return pSoundSys;

}

// ============================================================================
// ADD additional ITLSoundFactory implementors here

#elif defined( FOO_OS )

DECLARE_SOUND_FACTORY( Foo )

// the enumerator override

bool CFooSoundFactory::FillSoundSystems( char* pcSoundSysNames, uint uiMaxStringLen )
{
	return false;
}

// the creator override

ILTSoundSys* CFooSoundFactory::MakeSoundSystem( const char* pcSoundSystemName )
{
	return NULL;
}

#endif
#endif

//	===========================================================================
#endif	// USE_ABSTRACT_SOUND_INTERFACES

#ifdef USE_DX8_SOFTWARE_FILTERS

// filter support
void LTFILTERCHORUS::SetParam( const char* pszParam, float fValue )
{
	if ( !strcmp( pszParam, "Delay" ) )
	{
		fDelay = fValue;				
		uiFilterParamFlags |= SET_CHORUS_DELAY;
	}
	else if ( !strcmp( pszParam, "Depth" ) )
	{
		fDepth = fValue;				
		uiFilterParamFlags |= SET_CHORUS_DEPTH;
	}
	else if ( !strcmp( pszParam, "Feedback" ) )
	{
		fFeedback = fValue;				
		uiFilterParamFlags |= SET_CHORUS_FEEDBACK;
	}
	else if ( !strcmp( pszParam, "Frequency" )	)
	{
		fFrequency = fValue;				
		uiFilterParamFlags |= SET_CHORUS_FREQUENCY;
	}
	else if ( !strcmp( pszParam, "WetDryMix" ) )
	{
		fWetDryMix = fValue;				
		uiFilterParamFlags |= SET_CHORUS_WETDRYMIX;
	}
	else if ( !strcmp( pszParam, "Phase" ) )
	{
		lPhase = (int) fValue;				
		uiFilterParamFlags |= SET_CHORUS_PHASE;
	}
	else if ( !strcmp( pszParam, "Waveform" ) )
	{
		lWaveform = (int) fValue;				
		uiFilterParamFlags |= SET_CHORUS_WAVEFORM;
	}
}

void LTFILTERCOMPRESSOR::SetParam( const char* pszParam, float fValue )
{
	if ( !strcmp( pszParam, "Attack" ) )
	{
		fAttack = fValue;
		uiFilterParamFlags |= SET_COMPRESSOR_ATTACK;
	}
	else if ( !strcmp( pszParam, "Gain" ) )
	{
		fGain = fValue;
		uiFilterParamFlags |= SET_COMPRESSOR_GAIN;
	}
	else if ( !strcmp( pszParam, "Predelay" ) )
	{
		fPredelay = fValue;
		uiFilterParamFlags |= SET_COMPRESSOR_PREDELAY;
	}
	else if ( !strcmp( pszParam, "Ratio" ) )
	{
		fRatio = fValue;
		uiFilterParamFlags |= SET_COMPRESSOR_RATIO;
	}
	else if ( !strcmp( pszParam, "Release" ) )
	{
		fRelease = fValue;
		uiFilterParamFlags |= SET_COMPRESSOR_RELEASE;
	}
	else if ( !strcmp( pszParam, "Threshold" ) )
	{
		fThreshold = fValue;
		uiFilterParamFlags |= SET_COMPRESSOR_THRESHOLD;
	}
}

void LTFILTERDISTORTION::SetParam( const char* pszParam, float fValue )
{
	if ( !strcmp( pszParam, "Edge" ) )
	{
   		fEdge = fValue;
	   	uiFilterParamFlags |= SET_DISTORTION_EDGE;
	}
    else if ( !strcmp( pszParam, "Gain" ) )
    {
    	fGain = fValue;
    	uiFilterParamFlags |= SET_DISTORTION_GAIN;
    }
    else if ( !strcmp( pszParam, "PostEQBandwidth" ) )
    {
   		fPostEQBandwidth = fValue;
		uiFilterParamFlags |= SET_DISTORTION_POSTEQBANDWIDTH;
	}
	else if ( !strcmp( pszParam, "PostEQCenterFreq" ) )
	{
   		fPostEQCenterFrequency = fValue;
   		uiFilterParamFlags |= SET_DISTORTION_POSTEQCENTERFREQ;
	}
	else if ( !strcmp( pszParam, "PreLowpassCutoff" ) )
	{
   		fPreLowpassCutoff = fValue;
	   	uiFilterParamFlags |= SET_DISTORTION_PRELOWPASSCUTOFF;
    }
}

void LTFILTERECHO::SetParam( const char* pszParam, float fValue )
{
	if ( !strcmp( pszParam, "Feedback" ) )
	{
		fFeedback = fValue;
		uiFilterParamFlags |= SET_ECHO_FEEDBACK;
	}
	else if ( !strcmp( pszParam, "LeftDelay" ) )
	{
		fLeftDelay = fValue;
		uiFilterParamFlags |= SET_ECHO_LEFTDELAY;
	}
	else if ( !strcmp( pszParam, "RightDelay" ) )
	{
		fRightDelay = fValue;
		uiFilterParamFlags |= SET_ECHO_RIGHTDELAY;
	}
	else if ( !strcmp( pszParam, "PanDelay" ) )
	{
		lPanDelay = (int) fValue;
		uiFilterParamFlags |= SET_ECHO_PANDELAY;
	}
}

void LTFILTERFLANGE::SetParam( const char* pszParam, float fValue )
{
	if ( !strcmp( pszParam, "Delay" ) )
	{
		fDelay = fValue;
		uiFilterParamFlags |= SET_FLANGE_DELAY;
	}
	else if ( !strcmp( pszParam, "Depth" ) )
	{
		fDepth = fValue;
		uiFilterParamFlags |= SET_FLANGE_DEPTH;
	}
	else if ( !strcmp( pszParam, "Feedback" ) )
	{
		fFeedback = fValue;
		uiFilterParamFlags |= SET_FLANGE_FEEDBACK;
	}
	else if ( !strcmp( pszParam, "Frequency" ) )
	{
		fFrequency = fValue;
		uiFilterParamFlags |= SET_FLANGE_FREQUENCY;
	}
	else if ( !strcmp( pszParam, "WetDryMix" ) )
	{
		fWetDryMix = fValue;
		uiFilterParamFlags |= SET_FLANGE_WETDRYMIX;
	}
	else if ( !strcmp( pszParam, "Phase" ) )
	{
		lPhase = (int) fValue;
		uiFilterParamFlags |= SET_FLANGE_PHASE;
	}
	else if ( !strcmp( pszParam, "Waveform" ) )
	{
		lWaveform = (int) fValue;
		uiFilterParamFlags |= SET_FLANGE_WAVEFORM;
	}
}

void LTFILTERPARAMEQ::SetParam( const char* pszParam, float fValue )
{
	if ( !strcmp( pszParam, "Bandwidth" ) )
	{
		fBandwidth = fValue;
		uiFilterParamFlags |= SET_PARAMEQ_BANDWIDTH;
	}
	else if ( !strcmp( pszParam, "Center" ) )
	{
		fCenter = fValue;
		uiFilterParamFlags |= SET_PARAMEQ_CENTER;
	}
	else if ( !strcmp( pszParam, "Gain" ) )
	{
		fGain = fValue;
		uiFilterParamFlags |= SET_PARAMEQ_GAIN;
	}
}

void LTFILTERREVERB::SetParam( const char* pszParam, float fValue )
{
	if ( !strcmp( pszParam, "DecayHFRatio" ) )
	{
		fDecayHFRatio = fValue;
		uiFilterParamFlags |= SET_REVERB_DECAYHFRATIO;
	}
	else if ( !strcmp( pszParam, "DecayTime" ) )
	{
		fDecayTime = fValue;
		uiFilterParamFlags |= SET_REVERB_DECAYTIME;
	}
	else if ( !strcmp( pszParam, "Density" ) )
	{
		fDensity = fValue;
		uiFilterParamFlags |= SET_REVERB_DENSITY;
	}
	else if ( !strcmp( pszParam, "Diffusion" ) )
	{
		fDiffusion = fValue;
		uiFilterParamFlags |= SET_REVERB_DIFFUSION;
	}
	else if ( !strcmp( pszParam, "HFReference" ) )
	{
		fHFReference = fValue;
		uiFilterParamFlags |= SET_REVERB_HFREFERENCE;
	}
	else if ( !strcmp( pszParam, "ReflectionsDelay" ) )
	{
		fReflectionsDelay = fValue;
		uiFilterParamFlags |= SET_REVERB_REFLECTIONSDELAY;
	}
	else if ( !strcmp( pszParam, "ReverbDelay" ) )
	{
		fReverbDelay = fValue;
		uiFilterParamFlags |= SET_REVERB_REVERBDELAY;
	}
	else if ( !strcmp( pszParam, "RoomRolloffFactor" ) )
	{
		fRoomRolloffFactor = fValue;
		uiFilterParamFlags |= SET_REVERB_ROOMROLLOFFFACTOR;
	}
	else if ( !strcmp( pszParam, "Reflections" ) )
	{
		lReflections = (int) fValue;
		uiFilterParamFlags |= SET_REVERB_REFLECTIONS;
	}
	else if ( !strcmp( pszParam, "Reverb" ) )
	{
		lReverb = (int) fValue;
		uiFilterParamFlags |= SET_REVERB_REVERB;
	}
	else if ( !strcmp( pszParam, "Room" ) )
	{
		lRoom = (int) fValue;
		uiFilterParamFlags |= SET_REVERB_ROOM;
	}
	else if ( !strcmp( pszParam, "RoomHF" ) )
	{
		lRoomHF = (int) fValue;
		uiFilterParamFlags |= SET_REVERB_ROOMHF;
	}
}

#endif

#ifdef USE_EAX20_HARDWARE_FILTERS

void LTFILTERREVERB::SetParam( const char* pszParam, float fValue )
{
	if ( !strcmp( pszParam, "DecayHFRatio" ) )
	{
		fDecayHFRatio = fValue;
		uiFilterParamFlags |= SET_REVERB_DECAYHFRATIO;
	}
	else if ( !strcmp( pszParam, "DecayTime" ) )
	{
		fDecayTime = fValue;
		uiFilterParamFlags |= SET_REVERB_DECAYTIME;
	}
	else if ( !strcmp( pszParam, "Size" ) )
	{
		fSize = fValue;
		uiFilterParamFlags |= SET_REVERB_SIZE;
	}
	else if ( !strcmp( pszParam, "Diffusion" ) )
	{
		fDiffusion = fValue;
		uiFilterParamFlags |= SET_REVERB_DIFFUSION;
	}
	else if ( !strcmp( pszParam, "AirAbsorptionHF" ) )
	{
		fAirAbsorptionHF = fValue;
		uiFilterParamFlags |= SET_REVERB_AIRABSORPTIONHF;
	}
	else if ( !strcmp( pszParam, "ReflectionsDelay" ) )
	{
		fReflectionsDelay = fValue;
		uiFilterParamFlags |= SET_REVERB_REFLECTIONSDELAY;
	}
	else if ( !strcmp( pszParam, "ReverbDelay" ) )
	{
		fReverbDelay = fValue;
		uiFilterParamFlags |= SET_REVERB_REVERBDELAY;
	}
	else if ( !strcmp( pszParam, "RoomRolloffFactor" ) )
	{
		fRoomRolloffFactor = fValue;
		uiFilterParamFlags |= SET_REVERB_ROOMROLLOFFFACTOR;
	}
	else if ( !strcmp( pszParam, "Reflections" ) )
	{
		lReflections = (int) fValue;
		uiFilterParamFlags |= SET_REVERB_REFLECTIONS;
	}
	else if ( !strcmp( pszParam, "Reverb" ) )
	{
		lReverb = (int) fValue;
		uiFilterParamFlags |= SET_REVERB_REVERB;
	}
	else if ( !strcmp( pszParam, "Room" ) )
	{
		lRoom = (int) fValue;
		uiFilterParamFlags |= SET_REVERB_ROOM;
	}
	else if ( !strcmp( pszParam, "RoomHF" ) )
	{
		lRoomHF = (int) fValue;
		uiFilterParamFlags |= SET_REVERB_ROOMHF;
	}
	else if ( !strcmp( pszParam, "Direct" ) )
	{
		lDirect = (int) fValue;
		uiFilterParamFlags |= SET_REVERB_DIRECT;
	}
	else if ( !strcmp( pszParam, "Environment" ) )
	{
		lEnvironment = (int) fValue;
		uiFilterParamFlags |= SET_REVERB_ENVIRONMENT;
	}
}

#endif

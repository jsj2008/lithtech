#ifndef __ILTSOUND_H__
#define __ILTSOUND_H__

#ifndef __LTBASETYPES_H__
#include "ltbasetypes.h"
#endif

#ifndef __LTBASEDEFS_H__
#include "ltbasedefs.h"
#endif

#ifndef NULL
#define NULL			0x0
#endif	// NULL

#ifndef MAKE_BOOL
#define MAKE_BOOL( ltBoolVal )	( ( ( ltBoolVal ) == 0 ) ? false : true )
#define MAKE_LTBOOL( bBoolVal )	( ( ( bBoolVal ) == false ) ? LTFALSE : LTTRUE )
#endif	// MAKE_BOOL

#define LS_8_MONO_PCM			0
#define LS_16					1
#define LS_STEREO				2
#define	LS_ADPCM				4
#define LS_ASI					16

#define LS_FREE					0x0001		// Sample is available for allocation
#define LS_DONE					0x0002		// Sample has finished playing, or has never been started
#define LS_PLAYING				0x0004		// Sample is playing
#define LS_STOPPED				0x0008		// Sample has been stopped

#define LS_PROENUM_FIRST		0

#define LS_ROOM_GENERIC			0			// factory default
#define LS_ROOM_TYPE_COUNT		26			// total number of environments

#define LS_PREF_MIXER_CHANNELS		1
#define LS_PREF_USE_WAVEOUT			15
#define LS_PREF_LOCK_PROTECTION		18
#define LS_PREF_ENABLE_MMX			27		// Enable MMX support if present
#define LS_PREF_REVERB_BUFFER_SIZE	40

typedef short			sint16;				//S16;
typedef unsigned short	uint16;				//U16;
typedef long			sint32;				//S32;
typedef unsigned int	uint32;				//U32;
typedef void*			lpvoid;				//PTR;

#define LS_OK			0x0
#define LS_ERROR		0x1

typedef sint32			LSRESULT;			//M3DRESULT;
typedef sint32			( *LTLENGTHYCB )	( uint32 uiState, uint32 uiUser );	//AILLENGTHYCB

typedef uint32			LHPROVIDER;			//HPROVIDER;
typedef uint32			LHPROENUM;			//HPROENUM;

typedef lpvoid			LHSAMPLE;			//HSAMPLE;
typedef lpvoid			LH3DPOBJECT;		//H3DPOBJECT;
typedef lpvoid			LH3DSAMPLE;			//H3DSAMPLE;
typedef lpvoid			LHDIGDRIVER;		//HDIGDRIVER;
typedef lpvoid			LHSTREAM;			//HSTREAM;

typedef lpvoid			PHWAVEOUT;
typedef lpvoid			PTDIRECTSOUND;
typedef lpvoid			PTDIRECTSOUNDBUFFER;

typedef struct
{
	sint32 format;
	lpvoid data_ptr;
	uint32 data_len;
	uint32 rate;
	sint32 bits;
	sint32 channels;
	uint32 samples;
	uint32 block_size;
	lpvoid initial_ptr;
} LTSOUNDINFO;			//AILSOUNDINFO;
typedef LTSOUNDINFO*	PTSOUNDINFO;

typedef struct {
    uint16 wFormatTag;        
    uint16 nChannels;        
    uint32 nSamplesPerSec;    
    uint32 nAvgBytesPerSec;   
    uint16 nBlockAlign;      
} LTWAVEFORMAT;			//WAVEFORMAT;
typedef LTWAVEFORMAT*	PTWAVEFORMAT;

typedef struct 
{
    uint16 wFormatTag;        
    uint16 nChannels;        
    uint32 nSamplesPerSec;    
    uint32 nAvgBytesPerSec;   
    uint16 nBlockAlign;      
	uint16 wBitsPerSample;
	uint16 cbSize;
} LTWAVEFORMATEX;		//WAVEFORMATEX;
typedef LTWAVEFORMATEX*	PTWAVEFORAMTEX;


#ifndef DDI_ID_LPDIRECTSOUND

#define DDI_ID_LPDIRECTSOUND	0x1
#define DDI_ID_HDIGDRIVER		0x2

#endif	// DDIID_LPDIRECTSOUND

//! streamBufferParams_t

enum
{
	SBP_BUFFER_SIZE,
	SBP_BITS_PER_CHANNEL,
	SBP_CHANNELS_PER_SAMPLE,
	SBP_SAMPLES_PER_SEC,
	//
	SBP_NUM_PARAMS

};

struct streamBufferParams_t
{
	sint32	m_siParams[ SBP_NUM_PARAMS ];
};

// sound system abstract base class
// used by the CSoundMgr class to access and control the platform dependent sound system

class ILTSoundSys
{
protected:
	ILTSoundSys( ) {}
	virtual ~ILTSoundSys( ) {}

public:
	virtual bool		Init( ) = 0;
	virtual void		Term( ) = 0;

public:
	virtual void*		GetDDInterface( uint uiDDInterfaceId ) = 0;

public:
	// system wide functions
	virtual void		Lock( void ) = 0;
	virtual void		Unlock( void ) = 0;
	virtual sint32		Startup( void ) = 0;
	virtual void		Shutdown( void ) = 0;
	virtual uint32		MsCount( void ) = 0;
	virtual sint32		SetPreference( uint32 uiNumber, sint32 siValue ) = 0;
	virtual sint32		GetPreference( uint32 uiNumber ) = 0;
	virtual void		MemFreeLock( void* ptr ) = 0;
	virtual void*		MemAllocLock( uint32 uiSize ) = 0;
	virtual char*		LastError( void ) = 0;

	// digital sound driver functions
	virtual sint32		WaveOutOpen( LHDIGDRIVER* phDriver, PHWAVEOUT* pphWaveOut, sint32 siDeviceId, PTWAVEFORMAT pWaveFormat ) = 0;
	virtual void		WaveOutClose( LHDIGDRIVER hDriver ) = 0;
	virtual void		SetDigitalMasterVolume( LHDIGDRIVER hDig, sint32 siMasterVolume ) = 0;
	virtual sint32		GetDigitalMasterVolume( LHDIGDRIVER hDig ) = 0;
	virtual sint32		DigitalHandleRelease( LHDIGDRIVER hDriver ) = 0;
	virtual sint32		DigitalHandleReacquire( LHDIGDRIVER hDriver ) = 0;

	// 3d sound provider functions
	virtual LSRESULT	Open3DProvider( LHPROVIDER hLib ) = 0;
	virtual void		Close3DProvider( LHPROVIDER hLib ) = 0;
	virtual void		Set3DProviderPreference( LHPROVIDER hLib, char* sName, void* pVal ) = 0;
	virtual void		Get3DProviderAttribute( LHPROVIDER hLib, char* sName, void* pVal ) = 0;
	virtual sint32		Enumerate3DProviders( LHPROENUM* phNext, LHPROVIDER* phDest, char** psName) = 0;
	virtual sint32		Get3DRoomType( LHPROVIDER hLib ) = 0;
	virtual void		Set3DRoomType( LHPROVIDER hLib, sint32 siRoomType ) = 0;

	// 3d listener functions
	virtual LH3DPOBJECT	Open3DListener( LHPROVIDER hLib ) = 0;
	virtual void		Close3DListener( LH3DPOBJECT hListener ) = 0;

	// 3d sound object functions
	virtual void		Set3DPosition( LH3DPOBJECT hObj, float fX, float fY, float fZ) = 0;
	virtual void		Set3DVelocityVector( LH3DPOBJECT hObj, float fDX_per_ms, float fDY_per_ms, float fDZ_per_ms ) = 0;
	virtual void		Set3DOrientation( LH3DPOBJECT hObj, float fX_face, float fY_face, float fZ_face, float fX_up, float fY_up, float fZ_up ) = 0;
	virtual void		Set3DUserData( LH3DPOBJECT hObj, uint32 uiIndex, sint32 siValue ) = 0;
	virtual void		Get3DPosition( LH3DPOBJECT hObj, float* pfX, float* pfY, float* pfZ) = 0;
	virtual void		Get3DVelocity( LH3DPOBJECT hObj, float* pfDX_per_ms, float* pfDY_per_ms, float* pfDZ_per_ms ) = 0;
	virtual void		Get3DOrientation( LH3DPOBJECT hObj, float* pfX_face, float* pfY_face, float* pfZ_face, float* pfX_up, float* pfY_up, float* pfZ_up ) = 0;
	virtual sint32		Get3DUserData( LH3DPOBJECT hObj, uint32 uiIndex) = 0;

	// 3d sound sample functions
	virtual LH3DSAMPLE	Allocate3DSampleHandle( LHPROVIDER hLib ) = 0;
	virtual void		Release3DSampleHandle( LH3DSAMPLE hS ) = 0;
	virtual void		Stop3DSample( LH3DSAMPLE hS ) = 0;
	virtual void		Start3DSample( LH3DSAMPLE hS ) = 0;
	virtual void		Resume3DSample( LH3DSAMPLE hS ) = 0;
	virtual void		End3DSample( LH3DSAMPLE hS ) = 0;
	virtual void		Set3DSampleVolume( LH3DSAMPLE hS, sint32 siVolume ) = 0;
	virtual uint32		Get3DSampleStatus( LH3DSAMPLE hS ) = 0;
	virtual void		Set3DSampleOffset( LH3DSAMPLE hS, uint32 uiOffset ) = 0;
	virtual sint32		Set3DSampleInfo( LH3DSAMPLE hS, LTSOUNDINFO* pInfo ) = 0;
	virtual void		Set3DSamplePlaybackRate( LH3DSAMPLE hS, sint32 siPlayback_rate ) = 0;
	virtual void		Set3DSampleDistances( LH3DSAMPLE hS, float fMax_dist, float fMin_dist ) = 0;
	virtual void		Set3DSamplePreference( LH3DSAMPLE hSample, char* sName, void* pVal ) = 0;
	virtual void		Set3DSampleLoopBlock( LH3DSAMPLE hS, sint32 siLoop_start_offset, sint32 siLoop_end_offset ) = 0;
	virtual void		Set3DSampleLoopCount( LH3DSAMPLE hS, uint32 uiLoops ) = 0;
	virtual void		Set3DSampleObstruction( LH3DSAMPLE hS, float fObstruction ) = 0;
	virtual float		Get3DSampleObstruction( LH3DSAMPLE hS ) = 0;
	virtual void		Set3DSampleOcclusion( LH3DSAMPLE hS, float fOcclusion ) = 0;
	virtual float		Get3DSampleOcclusion( LH3DSAMPLE hS ) = 0;

	// 2d sound sample functions
	virtual LHSAMPLE	AllocateSampleHandle( LHDIGDRIVER hDig ) = 0;
	virtual void		ReleaseSampleHandle( LHSAMPLE hS ) = 0;
	virtual void		InitSample( LHSAMPLE hS ) = 0;
	virtual void		StopSample( LHSAMPLE hS ) = 0;
	virtual void		StartSample( LHSAMPLE hS ) = 0;
	virtual void		ResumeSample( LHSAMPLE hS ) = 0;
	virtual void		EndSample( LHSAMPLE hS ) = 0;
	virtual void		SetSampleVolume( LHSAMPLE hS, sint32 siVolume ) = 0;
	virtual void		SetSamplePan( LHSAMPLE hS, sint32 siPan ) = 0;
	virtual sint32		GetSampleVolume( LHSAMPLE hS ) = 0;
	virtual sint32		GetSamplePan( LHSAMPLE hS ) = 0;
	virtual void		SetSampleUserData( LHSAMPLE hS, uint32 uiIndex, sint32 siValue ) = 0;
	virtual void		GetDirectSoundInfo( LHSAMPLE hS, PTDIRECTSOUND* ppDS, PTDIRECTSOUNDBUFFER* ppDSB ) = 0;
	virtual void		SetSampleType( LHSAMPLE hS, sint32 siFormat, uint32 uiFlags ) = 0;
	virtual void		SetSampleReverb( LHSAMPLE hS, float fReverb_level, float fReverb_reflect_time, float fReverb_decay_time ) = 0;
	virtual void		SetSampleAddress( LHSAMPLE hS, void* pStart, uint32 uiLen ) = 0;
	virtual sint32		SetSampleFile( LHSAMPLE hS, void* pFile_image, sint32 siBlock ) = 0;
	virtual void		SetSamplePlaybackRate( LHSAMPLE hS, sint32 siPlayback_rate ) = 0;
	virtual void		SetSampleLoopBlock( LHSAMPLE hS, sint32 siLoop_start_offset, sint32 siLoop_end_offset ) = 0;
	virtual void		SetSampleLoopCount( LHSAMPLE hS, sint32 siLoop_count ) = 0;
	virtual void		SetSampleMsPosition( LHSAMPLE hS, sint32 siMilliseconds ) = 0;
	virtual sint32		GetSampleUserData( LHSAMPLE hS, uint32 uiIndex ) = 0;

	// old 2d sound stream functions
	virtual LHSTREAM	OpenStream( LHDIGDRIVER hDig, char* sFilename, sint32 siStream_mem ) = 0;
	virtual void		SetStreamLoopCount( LHSTREAM hStream, sint32 siCount ) = 0;
	virtual void		SetStreamPlaybackRate( LHSTREAM hStream, sint32 siRate ) = 0;
	virtual void		SetStreamMsPosition( LHSTREAM hS, sint32 siMilliseconds ) = 0;
	virtual void		SetStreamUserData( LHSTREAM hS, uint32 uiIndex, sint32 siValue) = 0;
	virtual sint32		GetStreamUserData( LHSTREAM hS, uint32 uiIndex) = 0;

	// new 2d sound stream functions
	virtual LHSTREAM	OpenStream( streamBufferParams_t* pStreamBufferParams ) = 0;
	virtual void		CloseStream( LHSTREAM hStream ) = 0;
	virtual void		StartStream( LHSTREAM hStream ) = 0;
	virtual void		PauseStream( LHSTREAM hStream, sint32 siOnOff ) = 0;
	virtual void		ResetStream( LHSTREAM hStream ) = 0;
	virtual void		SetStreamVolume( LHSTREAM hStream, sint32 siVolume ) = 0;
	virtual void		SetStreamPan( LHSTREAM hStream, sint32 siPan ) = 0;
	virtual sint32		GetStreamVolume( LHSTREAM hStream ) = 0;
	virtual sint32		GetStreamPan( LHSTREAM hStream ) = 0;
	virtual sint32		GetStreamStatus( LHSTREAM hStream ) = 0;
	virtual sint32		GetStreamBufferParam( LHSTREAM hStream, uint32 uiParam ) = 0;
	virtual void		ClearStreamBuffer( LHSTREAM hStream ) = 0;
	virtual bool		QueueStreamData( LHSTREAM, uint8* pucPCMSoundData, uint uiNumBytes ) = 0;

	// wave file decompression functons
	virtual sint32		DecompressADPCM( LTSOUNDINFO* pInfo, void** ppOutData, uint32* puiOutSize ) = 0;
	virtual sint32		DecompressASI( void* pInData, uint32 uiInSize, char* sFilename_ext, void** ppWav, uint32* puiWavSize, LTLENGTHYCB fnCallback ) = 0;

public:
};

// sound factory abstract base class
// used to generate ILTSoundSys platform dependent interface instances

typedef bool ( *FnEnumSoundSysCallback )( const char* pcSoundSysName, const char* pcSoundSysDesc, void* pUserData );

class ILTSoundFactory
{
protected:
	ILTSoundFactory( ) {}
	virtual ~ILTSoundFactory( ) {}

public:
	virtual bool EnumSoundSystems( FnEnumSoundSysCallback fnEnumCallback, void* pUserData = NULL );
	virtual bool FillSoundSystems( char* pcSoundSysNames, uint uiMaxStringLen ) = 0;
	virtual ILTSoundSys* MakeSoundSystem( const char* pcSoundSystemName ) = 0;

public:
	static ILTSoundFactory* GetSoundFactory( );

protected:
	static ILTSoundFactory* m_pSoundFactory;
};

#endif	// __ILTSOUND_H_

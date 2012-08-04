//#include "stdafx.h"
// ---
#include "iltsound.h"

typedef sint16	S16;
typedef uint16	U16;
typedef sint32	S32;
typedef uint32	U32;
typedef lpvoid	PTR;

//! CNulSoundSys

class CNulSoundSys : public ILTSoundSys
{
public:
	CNulSoundSys( ) {}
	virtual ~CNulSoundSys( ) {}

public:
	virtual bool		Init( );
	virtual void		Term( );

public:
	virtual void*		GetDDInterface( uint uiDDInterfaceId );

public:
	// system wide functions
	virtual void		Lock( void );
	virtual void		Unlock( void );
	virtual S32			Startup( void );
	virtual void		Shutdown( void );
	virtual U32			MsCount( void );
	virtual S32			SetPreference( U32 uiNumber, S32 siValue );
	virtual S32			GetPreference( U32 uiNumber );
	virtual void		MemFreeLock( void* ptr );
	virtual void*		MemAllocLock( U32 uiSize );
	virtual char*		LastError( void );

	// digital sound driver functions
	virtual S32			WaveOutOpen( LHDIGDRIVER* phDriver, PHWAVEOUT* pphWaveOut, S32 siDeviceId, PTWAVEFORMAT pWaveFormat );
	virtual void		WaveOutClose( LHDIGDRIVER hDriver );
	virtual void		SetDigitalMasterVolume( LHDIGDRIVER hDig, S32 siMasterVolume );
	virtual S32			GetDigitalMasterVolume( LHDIGDRIVER hDig );
	virtual S32			DigitalHandleRelease( LHDIGDRIVER hDriver );
	virtual S32			DigitalHandleReacquire( LHDIGDRIVER hDriver );

	// 3d sound provider functions
	virtual LSRESULT	Open3DProvider( LHPROVIDER hLib );
	virtual void		Close3DProvider( LHPROVIDER hLib );
	virtual void		Set3DProviderPreference( LHPROVIDER hLib, char* sName, void* pVal );
	virtual void		Get3DProviderAttribute( LHPROVIDER hLib, char* sName, void* pVal );
	virtual S32			Enumerate3DProviders( LHPROENUM* phNext, LHPROVIDER* phDest, char** psName);
	virtual S32			Get3DRoomType( LHPROVIDER hLib );
	virtual void		Set3DRoomType( LHPROVIDER hLib, S32 siRoomType );

	// 3d listener functions
	virtual LH3DPOBJECT	Open3DListener( LHPROVIDER hLib );
	virtual void		Close3DListener( LH3DPOBJECT hListener );

	// 3d sound object functions
	virtual void		Set3DPosition( LH3DPOBJECT hObj, float fX, float fY, float fZ);
	virtual void		Set3DVelocityVector( LH3DPOBJECT hObj, float fDX_per_ms, float fDY_per_ms, float fDZ_per_ms );
	virtual void		Set3DOrientation( LH3DPOBJECT hObj, float fX_face, float fY_face, float fZ_face, float fX_up, float fY_up, float fZ_up );
	virtual void		Set3DUserData( LH3DPOBJECT hObj, U32 uiIndex, S32 siValue );
	virtual void		Get3DPosition( LH3DPOBJECT hObj, float* pfX, float* pfY, float* pfZ);
	virtual void		Get3DVelocity( LH3DPOBJECT hObj, float* pfDX_per_ms, float* pfDY_per_ms, float* pfDZ_per_ms );
	virtual void		Get3DOrientation( LH3DPOBJECT hObj, float* pfX_face, float* pfY_face, float* pfZ_face, float* pfX_up, float* pfY_up, float* pfZ_up );
	virtual S32			Get3DUserData( LH3DPOBJECT hObj, U32 uiIndex);

	// 3d sound sample functions
	virtual LH3DSAMPLE	Allocate3DSampleHandle( LHPROVIDER hLib );
	virtual void		Release3DSampleHandle( LH3DSAMPLE hS );
	virtual void		Stop3DSample( LH3DSAMPLE hS );
	virtual void		Start3DSample( LH3DSAMPLE hS );
	virtual void		Resume3DSample( LH3DSAMPLE hS );
	virtual void		End3DSample( LH3DSAMPLE hS );
	virtual void		Set3DSampleVolume( LH3DSAMPLE hS, S32 siVolume );
	virtual U32			Get3DSampleStatus( LH3DSAMPLE hS );
	virtual void		Set3DSampleOffset( LH3DSAMPLE hS, U32 uiOffset );
	virtual S32			Set3DSampleInfo( LH3DSAMPLE hS, LTSOUNDINFO* pInfo );
	virtual void		Set3DSamplePlaybackRate( LH3DSAMPLE hS, S32 siPlayback_rate );
	virtual void		Set3DSampleDistances( LH3DSAMPLE hS, float fMax_dist, float fMin_dist );
	virtual void		Set3DSamplePreference( LH3DSAMPLE hSample, char* sName, void* pVal );
	virtual void		Set3DSampleLoopBlock( LH3DSAMPLE hS, S32 siLoop_start_offset, S32 siLoop_end_offset );
	virtual void		Set3DSampleLoopCount( LH3DSAMPLE hS, U32 uiLoops );
	virtual void		Set3DSampleObstruction( LH3DSAMPLE hS, float fObstruction );
	virtual float		Get3DSampleObstruction( LH3DSAMPLE hS );
	virtual void		Set3DSampleOcclusion( LH3DSAMPLE hS, float fOcclusion );
	virtual float		Get3DSampleOcclusion( LH3DSAMPLE hS );

	// 2d sound sample functions
	virtual LHSAMPLE	AllocateSampleHandle( LHDIGDRIVER hDig );
	virtual void		ReleaseSampleHandle( LHSAMPLE hS );
	virtual void		InitSample( LHSAMPLE hS );
	virtual void		StopSample( LHSAMPLE hS );
	virtual void		StartSample( LHSAMPLE hS );
	virtual void		ResumeSample( LHSAMPLE hS );
	virtual void		EndSample( LHSAMPLE hS );
	virtual void		SetSampleVolume( LHSAMPLE hS, S32 siVolume );
	virtual void		SetSamplePan( LHSAMPLE hS, S32 siPan );
	virtual S32			GetSampleVolume( LHSAMPLE hS );
	virtual S32			GetSamplePan( LHSAMPLE hS );
	virtual void		SetSampleUserData( LHSAMPLE hS, U32 uiIndex, S32 siValue );
	virtual void		GetDirectSoundInfo( LHSAMPLE hS, PTDIRECTSOUND* ppDS, PTDIRECTSOUNDBUFFER* ppDSB );
	virtual void		SetSampleType( LHSAMPLE hS, S32 siFormat, U32 uiFlags );
	virtual void		SetSampleReverb( LHSAMPLE hS, float fReverb_level, float fReverb_reflect_time, float fReverb_decay_time );
	virtual void		SetSampleAddress( LHSAMPLE hS, void* pStart, U32 uiLen );
	virtual S32			SetSampleFile( LHSAMPLE hS, void* pFile_image, S32 siBlock );
	virtual void		SetSamplePlaybackRate( LHSAMPLE hS, S32 siPlayback_rate );
	virtual void		SetSampleLoopBlock( LHSAMPLE hS, S32 siLoop_start_offset, S32 siLoop_end_offset );
	virtual void		SetSampleLoopCount( LHSAMPLE hS, S32 siLoop_count );
	virtual void		SetSampleMsPosition( LHSAMPLE hS, S32 siMilliseconds );
	virtual S32			GetSampleUserData( LHSAMPLE hS, U32 uiIndex );

	// old 2d sound stream functions
	virtual LHSTREAM	OpenStream( char* sFilename, uint32 nOffset, LHDIGDRIVER hDig, char* sStream, S32 siStream_mem );
	virtual void		SetStreamLoopCount( LHSTREAM hStream, S32 siCount );
	virtual void		SetStreamPlaybackRate( LHSTREAM hStream, S32 siRate );
	virtual void		SetStreamMsPosition( LHSTREAM hS, S32 siMilliseconds );
	virtual void		SetStreamUserData( LHSTREAM hS, U32 uiIndex, S32 siValue);
	virtual S32			GetStreamUserData( LHSTREAM hS, U32 uiIndex);

	// new 2d sound stream functions
	virtual LHSTREAM	OpenStream( streamBufferParams_t* pStreamBufferParams );
	virtual void		CloseStream( LHSTREAM hStream );
	virtual void		StartStream( LHSTREAM hStream );
	virtual void		PauseStream( LHSTREAM hStream, sint32 siOnOff );
	virtual void		ResetStream( LHSTREAM hStream );
	virtual void		SetStreamVolume( LHSTREAM hStream, sint32 siVolume );
	virtual void		SetStreamPan( LHSTREAM hStream, sint32 siPan );
	virtual sint32		GetStreamVolume( LHSTREAM hStream );
	virtual sint32		GetStreamPan( LHSTREAM hStream );
	virtual sint32		GetStreamStatus( LHSTREAM hStream );
	virtual sint32		GetStreamBufferParam( LHSTREAM hStream, uint32 uiParam );
	virtual void		ClearStreamBuffer( LHSTREAM hStream, bool bClearStreamDataQueue = true);
	virtual bool		QueueStreamData( LHSTREAM, uint8* pucPCMSoundData, uint uiNumBytes );
	virtual	sint32		GetStreamDataQueueSize( LHSTREAM hStream );
	virtual sint32		GetStreamCount( );

	// wave file decompression functons
	virtual S32			DecompressADPCM( LTSOUNDINFO* pInfo, void** ppOutData, U32* puiOutSize );
	virtual S32			DecompressASI( void* pInData, U32 uiInSize, char* sFilename_ext, void** ppWav, U32* puiWavSize, LTLENGTHYCB fnCallback );

public:
	static CNulSoundSys m_NulSoundSys;
	static char*		m_pcNulSoundSysDesc;
};

//	===========================================================================
//	CNulSoundSys methods

bool CNulSoundSys::Init( )
{
	return true;
}

void CNulSoundSys::Term( )
{
}

void* CNulSoundSys::GetDDInterface( uint uiDDInterfaceId )
{
	return NULL;
}

// system wide functions
void CNulSoundSys::Lock( void ) 
{ 
}

void CNulSoundSys::Unlock( void ) 
{ 
}

S32	CNulSoundSys::Startup( void ) 
{ 
	return 0; 
}

void CNulSoundSys::Shutdown( void ) 
{ 
}

U32	CNulSoundSys::MsCount( void ) 
{ 
	return 0; 
}

S32	CNulSoundSys::SetPreference( U32 uiNumber, S32 siValue ) 
{ 
	return 0; 
}

S32	CNulSoundSys::GetPreference( U32 uiNumber ) 
{ 
	return 0; 
}

void CNulSoundSys::MemFreeLock( void* ptr ) 
{ 
	LTMemFree( ptr );
}

void* CNulSoundSys::MemAllocLock( U32 uiSize ) 
{ 
	void* p;
	LT_MEM_TRACK_ALLOC(p = LTMemAlloc( uiSize ),LT_MEM_TYPE_SOUND);
	return p;
}

char* CNulSoundSys::LastError( void ) 
{ 
	return "no error";
}

// digital sound driver functions
S32	CNulSoundSys::WaveOutOpen( LHDIGDRIVER* phDriver, PHWAVEOUT* pphWaveOut, S32 siDeviceId, PTWAVEFORMAT pWaveFormat ) 
{ 
	return 0; 
}

void CNulSoundSys::WaveOutClose( LHDIGDRIVER hDriver ) 
{ 
}

void CNulSoundSys::SetDigitalMasterVolume( LHDIGDRIVER hDig, S32 siMasterVolume ) 
{ 
}

S32	CNulSoundSys::GetDigitalMasterVolume( LHDIGDRIVER hDig ) 
{ 
	return 0; 
}

S32	CNulSoundSys::DigitalHandleRelease( LHDIGDRIVER hDriver ) 
{ 
	return 0; 
}

S32	CNulSoundSys::DigitalHandleReacquire( LHDIGDRIVER hDriver ) 
{ 
	return 0; 
}

// 3d sound provider functions
LSRESULT CNulSoundSys::Open3DProvider( LHPROVIDER hLib ) 
{ 
	return 0; 
}

void CNulSoundSys::Close3DProvider( LHPROVIDER hLib ) 
{ 
}

void CNulSoundSys::Set3DProviderPreference( LHPROVIDER hLib, char* sName, void* pVal ) 
{ 
}

void CNulSoundSys::Get3DProviderAttribute( LHPROVIDER hLib, char* sName, void* pVal ) 
{ 
}

S32	CNulSoundSys::Enumerate3DProviders( LHPROENUM* phNext, LHPROVIDER* phDest, char** psName) 
{ 
	return 0; 
}

S32	CNulSoundSys::Get3DRoomType( LHPROVIDER hLib ) 
{ 
	return 0; 
}

void CNulSoundSys::Set3DRoomType( LHPROVIDER hLib, S32 siRoomType ) 
{ 
}

// 3d listener functions
LH3DPOBJECT	CNulSoundSys::Open3DListener( LHPROVIDER hLib ) 
{ 
	return NULL;
}

void CNulSoundSys::Close3DListener( LH3DPOBJECT hListener ) 
{ 
}

// 3d sound object functions
void CNulSoundSys::Set3DPosition( LH3DPOBJECT hObj, float fX, float fY, float fZ) 
{ 
}

void CNulSoundSys::Set3DVelocityVector( LH3DPOBJECT hObj, float fDX_per_ms, float fDY_per_ms, float fDZ_per_ms ) 
{ 
}

void CNulSoundSys::Set3DOrientation( LH3DPOBJECT hObj, float fX_face, float fY_face, float fZ_face, float fX_up, float fY_up, float fZ_up ) 
{ 
}

void CNulSoundSys::Set3DUserData( LH3DPOBJECT hObj, U32 uiIndex, S32 siValue ) 
{ 
}

void CNulSoundSys::Get3DPosition( LH3DPOBJECT hObj, float* pfX, float* pfY, float* pfZ) 
{ 
}

void CNulSoundSys::Get3DVelocity( LH3DPOBJECT hObj, float* pfDX_per_ms, float* pfDY_per_ms, float* pfDZ_per_ms ) 
{ 
}

void CNulSoundSys::Get3DOrientation( LH3DPOBJECT hObj, float* pfX_face, float* pfY_face, float* pfZ_face, float* pfX_up, float* pfY_up, float* pfZ_up ) 
{ 
}

S32	CNulSoundSys::Get3DUserData( LH3DPOBJECT hObj, U32 uiIndex) 
{ 
	return 0; 
}

// 3d sound sample functions
LH3DSAMPLE CNulSoundSys::Allocate3DSampleHandle( LHPROVIDER hLib ) 
{ 
	return NULL;
}

void CNulSoundSys::Release3DSampleHandle( LH3DSAMPLE hS ) 
{ 
}

void CNulSoundSys::Stop3DSample( LH3DSAMPLE hS ) 
{ 
}

void CNulSoundSys::Start3DSample( LH3DSAMPLE hS ) 
{ 
}

void CNulSoundSys::Resume3DSample( LH3DSAMPLE hS ) 
{ 
}

void CNulSoundSys::End3DSample( LH3DSAMPLE hS ) 
{ 
}

void CNulSoundSys::Set3DSampleVolume( LH3DSAMPLE hS, S32 siVolume ) 
{ 
}

U32	CNulSoundSys::Get3DSampleStatus( LH3DSAMPLE hS ) 
{ 
	return 0; 
}

void CNulSoundSys::Set3DSampleOffset( LH3DSAMPLE hS, U32 uiOffset ) 
{ 
}

S32	CNulSoundSys::Set3DSampleInfo( LH3DSAMPLE hS, LTSOUNDINFO* pInfo ) 
{ 
	return 0; 
}

void CNulSoundSys::Set3DSamplePlaybackRate( LH3DSAMPLE hS, S32 siPlayback_rate ) 
{ 
}

void CNulSoundSys::Set3DSampleDistances( LH3DSAMPLE hS, float fMax_dist, float fMin_dist ) 
{ 
}

void CNulSoundSys::Set3DSamplePreference( LH3DSAMPLE hSample, char* sName, void* pVal ) 
{ 
}

void CNulSoundSys::Set3DSampleLoopBlock( LH3DSAMPLE hS, S32 siLoop_start_offset, S32 siLoop_end_offset ) 
{ 
}

void CNulSoundSys::Set3DSampleLoopCount( LH3DSAMPLE hS, U32 uiLoops ) 
{ 
}

void CNulSoundSys::Set3DSampleObstruction( LH3DSAMPLE hS, float fObstruction ) 
{ 
}

float CNulSoundSys::Get3DSampleObstruction( LH3DSAMPLE hS ) 
{ 
	return 0; 
}

void CNulSoundSys::Set3DSampleOcclusion( LH3DSAMPLE hS, float fOcclusion ) 
{ 
}

float CNulSoundSys::Get3DSampleOcclusion( LH3DSAMPLE hS ) 
{ 
	return 0; 
}

// 2d sound sample functions
LHSAMPLE CNulSoundSys::AllocateSampleHandle( LHDIGDRIVER hDig ) 
{ 
	return NULL;
}

void CNulSoundSys::ReleaseSampleHandle( LHSAMPLE hS ) 
{ 
}

void CNulSoundSys::InitSample( LHSAMPLE hS ) 
{ 
}

void CNulSoundSys::StopSample( LHSAMPLE hS ) 
{ 
}

void CNulSoundSys::StartSample( LHSAMPLE hS )
{
}

void CNulSoundSys::ResumeSample( LHSAMPLE hS ) 
{ 
}

void CNulSoundSys::EndSample( LHSAMPLE hS )
{
}

void CNulSoundSys::SetSampleVolume( LHSAMPLE hS, S32 siVolume ) 
{ 
}

void CNulSoundSys::SetSamplePan( LHSAMPLE hS, S32 siPan ) 
{ 
}

S32	CNulSoundSys::GetSampleVolume( LHSAMPLE hS ) 
{ 
	return 0; 
}

S32	CNulSoundSys::GetSamplePan( LHSAMPLE hS ) 
{ 
	return 0; 
}

void CNulSoundSys::SetSampleUserData( LHSAMPLE hS, U32 uiIndex, S32 siValue ) 
{ 
}

void CNulSoundSys::GetDirectSoundInfo( LHSAMPLE hS, PTDIRECTSOUND* ppDS, PTDIRECTSOUNDBUFFER* ppDSB ) 
{ 
}

void CNulSoundSys::SetSampleType( LHSAMPLE hS, S32 siFormat, U32 uiFlags ) 
{ 
}

void CNulSoundSys::SetSampleReverb( LHSAMPLE hS, float fReverb_level, float fReverb_reflect_time, float fReverb_decay_time ) 
{ 
}

void CNulSoundSys::SetSampleAddress( LHSAMPLE hS, void* pStart, U32 uiLen ) 
{ 
}

S32	CNulSoundSys::SetSampleFile( LHSAMPLE hS, void* pFile_image, S32 siBlock ) 
{ 
	return 0; 
}

void CNulSoundSys::SetSamplePlaybackRate( LHSAMPLE hS, S32 siPlayback_rate ) 
{ 
}

void CNulSoundSys::SetSampleLoopBlock( LHSAMPLE hS, S32 siLoop_start_offset, S32 siLoop_end_offset ) 
{ 
}

void CNulSoundSys::SetSampleLoopCount( LHSAMPLE hS, S32 siLoop_count ) 
{ 
}

void CNulSoundSys::SetSampleMsPosition( LHSAMPLE hS, S32 siMilliseconds ) 
{ 
}

S32	CNulSoundSys::GetSampleUserData( LHSAMPLE hS, U32 uiIndex ) 
{ 
	return 0; 
}

// old 2d sound stream functions
LHSTREAM CNulSoundSys::OpenStream( char* sFilename, uint32 nOffset, LHDIGDRIVER hDig, char* sStream, S32 siStream_mem ) 
{ 
	return NULL;
}

void CNulSoundSys::SetStreamLoopCount( LHSTREAM hStream, S32 siCount ) 
{ 
}

void CNulSoundSys::SetStreamPlaybackRate( LHSTREAM hStream, S32 siRate ) 
{ 
}

void CNulSoundSys::SetStreamMsPosition( LHSTREAM hS, S32 siMilliseconds ) 
{ 
}

void CNulSoundSys::SetStreamUserData( LHSTREAM hS, U32 uiIndex, S32 siValue) 
{ 
}

S32	CNulSoundSys::GetStreamUserData( LHSTREAM hS, U32 uiIndex) 
{ 
	return 0; 
}

// new 2d sound stream functions
LHSTREAM CNulSoundSys::OpenStream( streamBufferParams_t* pStreamBufferParams )
{
	return NULL;
}

void CNulSoundSys::CloseStream( LHSTREAM hStream ) 
{ 
}

void CNulSoundSys::StartStream( LHSTREAM hStream ) 
{ 
}

void CNulSoundSys::PauseStream( LHSTREAM hStream, S32 siOnOff ) 
{ 
}

void CNulSoundSys::ResetStream( LHSTREAM hStream )
{
}

void CNulSoundSys::SetStreamVolume( LHSTREAM hStream, S32 siVolume ) 
{ 
}

void CNulSoundSys::SetStreamPan( LHSTREAM hStream, S32 siPan ) 
{ 
}

S32	CNulSoundSys::GetStreamVolume( LHSTREAM hStream ) 
{ 
	return 0; 
}

S32	CNulSoundSys::GetStreamPan( LHSTREAM hStream ) 
{ 
	return 0; 
}

sint32 CNulSoundSys::GetStreamStatus( LHSTREAM hStream )
{
	return 0;
}

sint32 CNulSoundSys::GetStreamBufferParam( LHSTREAM hStream, uint32 uiParam )
{
	return 0;
}

void CNulSoundSys::ClearStreamBuffer( LHSTREAM hStream, bool bClearStreamDataQueue )
{
}

bool CNulSoundSys::QueueStreamData( LHSTREAM, uint8* pucPCMSoundData, uint uiNumBytes )
{
	return false;
}

sint32 CNulSoundSys::GetStreamDataQueueSize( LHSTREAM hStream )
{
	return 0;
}

sint32 CNulSoundSys::GetStreamCount( )
{
	return 0;
}

// wave file decompression functons
S32	CNulSoundSys::DecompressADPCM( LTSOUNDINFO* pInfo, void** ppOutData, U32* puiOutSize ) 
{ 
	return 0; 
}

S32	CNulSoundSys::DecompressASI( void* pInData, U32 uiInSize, char* sFilename_ext, void** ppWav, U32* puiWavSize, LTLENGTHYCB fnCallback ) 
{ 
	return 0; 
}

//	===========================================================================

CNulSoundSys CNulSoundSys::m_NulSoundSys;
char* CNulSoundSys::m_pcNulSoundSysDesc = "*** null sound driver ***";

extern "C"
{
	__declspec( dllexport ) char*			SoundSysDesc( );
	__declspec( dllexport ) ILTSoundSys*	SoundSysMake( );
}

char* SoundSysDesc( )
{
	return CNulSoundSys::m_pcNulSoundSysDesc;
}

ILTSoundSys* SoundSysMake( )
{
	return &CNulSoundSys::m_NulSoundSys;
}

//bool APIENTRY DllMain( HANDLE hModule, DWORD ul_reason_for_call, LPVOID lpReserved )
//{
//    return true;
//}

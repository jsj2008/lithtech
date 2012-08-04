#ifndef __SOUNDMGR_H__
#define __SOUNDMGR_H__

//	===========================================================================
#ifdef USE_ABSTRACT_SOUND_INTERFACES
//	===========================================================================

#ifndef __ILTSOUND_H__
#include "iltsound.h"
#endif

typedef sint16	S16;
typedef uint16	U16;
typedef sint32	S32;
typedef uint32	U32;
typedef lpvoid	PTR;

extern ILTSoundSys* SoundSys( bool bTerminate = false );
#define GetSoundSys SoundSys

#ifdef __DSOUND_H__
#include "dsound.h"
#define __DSOUND_H__
#endif

//	===========================================================================
#else
//	===========================================================================

#ifndef __LTDIRECTSOUND_H__
#include "ltdirectsound.h"
#define __LTDIRECTSOUND_H__
#endif

#ifdef __DSOUND_H__
#include "dsound.h"
#define __DSOUND_H__
#endif

//	===========================================================================
#endif
//	===========================================================================


#ifndef __ILTSOUNDMGR_H__
#include "iltsoundmgr.h"
#endif

#ifndef __NEWPACKET_H__
#include "packet.h"
#endif

#ifndef __SOUNDBUFFER_H__
#include "soundbuffer.h"
#endif

#ifndef __DMUSICI_H__
#include <dmusici.h>
#define __DMUSICI_H__
#endif

#ifndef __SOUNDINSTANCE_H__
#include "soundinstance.h"
#endif



class CSoundBuffer;
struct FileIdentifier;
class CSoundInstance;
class CLocalSoundInstance;
class CAmbientSoundInstance;
class C3DSoundInstance;


// SoundMgr defines
#define SOUNDMGR_MAXSOUNDINSTANCES		255
#define SOUNDMGR_MINSTREAMBUFFERSIZE	10240L		// Minimum buffer

// Sample types
#define SAMPLETYPE_SW		0
#define SAMPLETYPE_3D		1

// Sample user data indices
#define SAMPLE_INSTANCE			0
#define SAMPLE_TYPE				1
#define SAMPLE_LISTITEM			2
#define SAMPLE_BUFFER			3

// Time between committing positions
#define COMMITTIME				100



struct CProvider
{
//	===========================================================================
#ifdef USE_ABSTRACT_SOUND_INTERFACES
//	===========================================================================

	LHPROVIDER	m_hProvider;

//	===========================================================================
#else
//	===========================================================================

	HPROVIDER	m_hProvider;

//	===========================================================================
#endif
//	===========================================================================

	uint32		m_dwProviderID;
	char		m_szProviderName[_MAX_PATH+1];
	uint32		m_dwCaps;
	CProvider *	m_pNextProvider;
};

struct CSample
{
	union
	{
//	===========================================================================
#ifdef USE_ABSTRACT_SOUND_INTERFACES
//	===========================================================================

		LHSAMPLE	m_hSample;
		LH3DSAMPLE	m_h3DSample;

//	===========================================================================
#else
//	===========================================================================

		HSAMPLE		m_hSample;
		H3DSAMPLE	m_h3DSample;

//	===========================================================================
#endif
//	===========================================================================
	};

	LTLink			m_Link;
};

class CSoundMgr : public ILTClientSoundMgr
{
public:
    declare_interface(CSoundMgr);


	CSoundMgr( )
	;

	~CSoundMgr( )
	;

	LTRESULT	Init( InitSoundInfo &soundInit )
	;

	void		Term( bool bRemoveSounds = true )
	;

	bool		IsValid( )
	{ return m_bValid; }

	void		SetEnable( bool bEnable )
	{ m_bEnabled = bEnable; }

	bool		IsEnabled( )
	{ return m_bEnabled; }


//	===========================================================================
#ifdef USE_ABSTRACT_SOUND_INTERFACES
//	===========================================================================

	LPDIRECTSOUND8 GetDirectSound( );
	IDirectMusicPerformance8* GetDirectMusicPerformance( );
	IDirectMusic* GetDirectMusic();
	LHDIGDRIVER	GetDigDriver( ) const { return m_hDigDriver; }

//	===========================================================================
#else
//	===========================================================================

	LPDIRECTSOUND GetDirectSound( );
	IDirectMusicPerformance8* GetDirectMusicPerformance( );
	IDirectMusic* GetDirectMusic();
	HDIGDRIVER	GetDigDriver( ) const { return m_hDigDriver; }

//	===========================================================================
#endif
//	===========================================================================

	CSoundBuffer *CreateBuffer( FileIdentifier &fileIdent );

	LTRESULT	RemoveBuffer( FileIdentifier &fileIdent );

	LTRESULT	UntagAllSoundBuffers( );

	LTRESULT	RemoveAllUntaggedSoundBuffers( );

	LTRESULT	RemoveAllUnusedSoundInstances( );

	LTRESULT	PlaySound( PlaySoundInfo &playSoundInfo, FileIdentifier &fileIdent, uint32 dwOffsetTime = 0 );

	CSoundInstance *FindSoundInstance( HLTSOUND hSound, bool bClientSound );

	LTRESULT	RemoveInstance( CSoundInstance &soundInstance );

	LTRESULT	StopAllSounds( );

	LTRESULT	Update( );

	bool		IsDigitalHandleReleased( ) const
	{ return m_bDigitalHandleReleased; }

	LTRESULT	ReleaseDigitalHandle( );

	LTRESULT	ReacquireDigitalHandle( );

	LTRESULT	PauseSounds( );

	LTRESULT	ResumeSounds( );

	LTRESULT	SetSoundClassMultiplier( uint8 nSoundClass, float fMultiplier, bool bUseGlobalVolume=true );

	LTRESULT	GetSoundClassMultiplier( uint8 nSoundClass, float* pfMultiplier );

	LTRESULT	GetSoundClassUseGlobalVolume( uint8 nSoundClass, bool* pbUseGlobalVolume );

	LTRESULT	UpdateVolumeSettings();

	bool		IsListenerInClient( ) const
	{ return m_bListenerInClient; }

	LTRESULT	SetListener( bool bListenerInClient, LTVector *pvListenerPos, LTVector *pvListenerFront, LTVector *pvListenerRight, bool bTeleport = LTFALSE );

	const LTVector &	GetListenerPosition( ) const
	{ return m_vListenerPosition; }

	const LTVector &	GetListenerVelocity( ) const
	{ return m_vListenerVelocity; }

	const LTVector &	GetLastListenerPosition( ) const
	{ return m_vLastListenerPosition; }

	const LTVector &	GetListenerFront( ) const
	{ return m_vListenerForward; }

	const LTVector &	GetListenerRight( ) const
	{ return m_vListenerRight; }

	virtual LTRESULT	SetListenerDoppler( float fDoppler );

	virtual LTRESULT	SetVolume( uint16 nVolume )
	;

	virtual LTRESULT	GetVolume( uint16 &nVolume )
	;

	virtual LTRESULT	GetVolumeMultiplier( float &fVolume );
//	===========================================================================
#ifdef USE_ABSTRACT_SOUND_INTERFACES
//	===========================================================================

	LH3DPOBJECT	GetListenerObject( ) const { return m_h3DListener; }
	LHSAMPLE	GetFreeSWSample( );
	LH3DSAMPLE	GetFree3DSample( CSoundInstance *pSoundInstance = LTNULL );
	void		ReleaseSWSample( LHSAMPLE hSample );
	void		Release3DSample( LH3DSAMPLE h3DSample );
	LTRESULT		LinkSampleSoundInstance( LHSAMPLE hSample, CSoundInstance *pSoundInstance );
	CSoundInstance *GetLinkSampleSoundInstance( LHSAMPLE hSample );
	LTRESULT		Link3DSampleSoundInstance( LH3DSAMPLE h3DSample, CSoundInstance *pSoundInstance );
	CSoundInstance *GetLink3DSampleSoundInstance( LH3DSAMPLE h3DSample );
	LTRESULT		LinkStreamSoundInstance( LHSTREAM hStream, CSoundInstance *pSoundInstance );
	CSoundInstance *GetLinkStreamSoundInstance( LHSTREAM hStream );

//	===========================================================================
#else
//	===========================================================================

	H3DPOBJECT	GetListenerObject( ) const { return m_h3DListener; }
	HSAMPLE		GetFreeSWSample( );
	H3DSAMPLE	GetFree3DSample( CSoundInstance *pSoundInstance = LTNULL );
	void		ReleaseSWSample( HSAMPLE hSample );
	void		Release3DSample( H3DSAMPLE h3DSample );
	LTRESULT		LinkSampleSoundInstance( HSAMPLE hSample, CSoundInstance *pSoundInstance );
	CSoundInstance *GetLinkSampleSoundInstance( HSAMPLE hSample );
	LTRESULT		Link3DSampleSoundInstance( H3DSAMPLE h3DSample, CSoundInstance *pSoundInstance );
	CSoundInstance *GetLink3DSampleSoundInstance( H3DSAMPLE h3DSample );
	LTRESULT		LinkStreamSoundInstance( HSTREAM hStream, CSoundInstance *pSoundInstance );
	CSoundInstance *GetLinkStreamSoundInstance( HSTREAM hStream );

//	===========================================================================
#endif
//	===========================================================================

	const CPacket_Read &GetSoundUpdatePacket( )
	{ return m_SoundUpdatePacket; }

	uint32		GetNumSoundsPlaying( )
	{ return m_dwNumSoundInstances; }

	uint32		GetNumSoundsHeard( )
	{ return m_nNumSoundsHeard; }

	bool		GetConvert16to8( ) const
	{ return m_bConvert16to8; }

	float		GetDistanceFactor( ) const
	{ return m_fDistanceFactor; }
	
	static int	CompareSoundInstances( const void *pElem1, const void *pElem2 )
	;

	bool		UseSWReverb( ) const
	{ return m_bSWReverb; }

	bool		Use3DReverb( ) const
	{ return m_b3DReverb; }

	LTRESULT	SetReverbProperties( ReverbProperties *pReverbProperties )
	;

	LTRESULT	GetReverbProperties( ReverbProperties *pReverbProperties )
	;

	uint32		CommitChanges( ) const
	{ return m_bCommitChanges; }

//// ILTClientSoundMgr Implementation /////////////////////////////////////
public:
	virtual LTRESULT	PlaySound( PlaySoundInfo *pPlaySoundInfo, HLTSOUND &hResult )
	;

	virtual LTRESULT	GetSoundDuration( HLTSOUND hSound, LTFLOAT &fDuration )
	;

	virtual LTRESULT	IsSoundDone( HLTSOUND hSound, bool &bDone )
	;

	virtual LTRESULT	KillSound( HLTSOUND hSound )
	;

	virtual LTRESULT	KillSoundLoop( HLTSOUND hSound )
	;

	virtual LTRESULT	GetSound3DProviderLists( Sound3DProvider *&pSound3DProviderList, bool bVerify, uint32 uiMax3DVoices )
	;

	virtual LTRESULT	ReleaseSound3DProviderList( Sound3DProvider *pSound3DProviderList )
	;

	virtual LTRESULT	InitSound( InitSoundInfo *pSoundInfo )
	;

	virtual	LTRESULT	SetSoundOcclusion( HLTSOUND hSound, LTFLOAT fLevel )
	;

	virtual LTRESULT	GetSoundOcclusion( HLTSOUND hSound, LTFLOAT *pLevel )
	;

	virtual	LTRESULT	SetSoundObstruction( HLTSOUND hSound, LTFLOAT fLevel )
	;

	virtual LTRESULT	GetSoundObstruction( HLTSOUND hSound, LTFLOAT *pLevel )
	;

	virtual LTRESULT	GetSoundPosition( HLTSOUND hSound, LTVector *pPos )
	;

	virtual LTRESULT	SetSoundPosition( HLTSOUND hSound, LTVector *pPos )
	;

	virtual LTRESULT	SetListener( bool bListenerInClient, LTVector *pPos, LTRotation *pRot, bool bTeleport )
	;

#ifdef USE_DX8_SOFTWARE_FILTERS
	virtual LTRESULT	SetSoundFilter( HLTSOUND hSound, const char *pFilter );
	virtual LTRESULT	SetSoundFilterParam( HLTSOUND hSound, const char *pParam, float fValue );
#endif

#ifdef USE_EAX20_HARDWARE_FILTERS
	virtual LTRESULT	SetSoundFilter( const char *pFilter );
	virtual LTRESULT	SetSoundFilterParam( const char *pParam, float fValue );
	virtual LTRESULT	EnableSoundFilter( bool bEnable );
#endif

public:

	ObjectBank< CSoundBuffer > &GetSoundBufferBank( ) 
	{ return m_SoundBufferBank; }

private:
	
	LTRESULT	Get3DProviderLists( CProvider *&p3DProviderList, bool bVerifyOpens = TRUE, uint32 uiMax3DVoices = 0 )
	;

	CProvider *	EnumerateAllProviders( bool bVefifyOpens = TRUE )
	;

	void		ReleaseProviderList( CProvider *pProviderList )
	;

	LTRESULT	Set3DProvider( char *psz3DProviderName )
	;

	LTRESULT	Create3DSamples( )
	;

	LTRESULT	Remove3DSamples( )
	;

	LTRESULT	CreateSWSamples( )
	;

	LTRESULT	RemoveSWSamples( )
	;

	LTRESULT	RemoveBuffer( CSoundBuffer &soundBuffer )
	;

private:

	InitSoundInfo	m_InitSoundInfo;

	bool		m_bInited;
	bool		m_bFailedInit;
	bool		m_bValid;
	bool		m_bEnabled;

	CProvider *	m_p3DProviderList;

//	===========================================================================
#ifdef USE_ABSTRACT_SOUND_INTERFACES
//	===========================================================================

	LHDIGDRIVER	m_hDigDriver;

//	===========================================================================
#else
//	===========================================================================

	HDIGDRIVER	m_hDigDriver;

//	===========================================================================
#endif
//	===========================================================================

	bool		m_bDigitalHandleReleased;
	bool		m_bReacquireDigitalHandle;

	CProvider 	m_3DProvider;
	uint8		m_nNum3DSamples;
	uint8		m_nMax3DSamples;
	CSample *	m_p3DSampleList;
	LTList		m_3DFreeSampleList;

	uint8		m_nNumSWSamples;
	uint8		m_nMaxSWSamples;
	CSample *	m_pSWSampleList;
	LTList		m_SWFreeSampleList;

	ObjectBank< CSoundBuffer > m_SoundBufferBank;
	LTList		m_SoundBufferList;

	ObjectBank< CLocalSoundInstance > m_LocalSoundInstanceBank;
	ObjectBank< CAmbientSoundInstance > m_AmbientSoundInstanceBank;
	ObjectBank< C3DSoundInstance > m_3DSoundInstanceBank;

	CSoundInstance *	m_SoundInstanceList[SOUNDMGR_MAXSOUNDINSTANCES];
	uint32		m_dwNumSoundInstances;

    WAVEFORMATEX m_PrimaryBufferWaveFormat;

//	===========================================================================
#ifdef USE_ABSTRACT_SOUND_INTERFACES
//	===========================================================================

	LH3DPOBJECT	m_h3DListener;

//	===========================================================================
#else
//	===========================================================================

	H3DPOBJECT	m_h3DListener;

//	===========================================================================
#endif
//	===========================================================================

	LTVector	m_vListenerPosition; 
	LTVector	m_vLastListenerPosition; 
	LTVector	m_vListenerVelocity;
	LTVector	m_vListenerForward;
	LTVector	m_vListenerRight;
	LTVector	m_vListenerUp;
	bool		m_bListenerInClient;

	float		m_fDistanceFactor;

	uint8		m_nNumSoundsHeard;

	CPacket_Read m_SoundUpdatePacket;

	bool		m_bConvert16to8;

	bool		m_bSWReverb;
	bool		m_b3DReverb;
	uint32		m_dwReverbAcoustics;
	float		m_fReverbReflectTime;
	float		m_fReverbVolume;
	float		m_fReverbDecayTime;
	float		m_fReverbDamping;

	uint16		m_nGlobalSoundVolume;	

#ifdef USE_EAX20_HARDWARE_FILTERS
	// for filter support
	LTSOUNDFILTERDATA m_FilterData;
#endif

	uint32		m_dwCurTime;
	uint32		m_dwCommitTime;
	bool		m_bCommitChanges;
	float		m_afSoundClassMultipliers[MAX_SOUND_VOLUME_CLASSES + 1];
	bool		m_abUseGlobalVolume[MAX_SOUND_VOLUME_CLASSES + 1];
};

//
//Use define_holder to get at the ILTSoundMgr interface, but if you need
//the CSoundMgr implementation, use this function.
//

CSoundMgr *GetClientILTSoundMgrImpl();


#endif __SOUNDMGR_H__

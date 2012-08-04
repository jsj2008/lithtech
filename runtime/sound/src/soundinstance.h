#ifndef __SOUNDINSTANCE_H__
#define __SOUNDINSTANCE_H__

class CSoundBuffer;

#ifndef __SOUNDMGR_H__
#include "soundmgr.h"
#endif

enum SoundType
{
	SOUNDTYPE_LOCAL = 0,
	SOUNDTYPE_AMBIENT,
	SOUNDTYPE_3D,
	SOUNDTYPE_NUMTYPES
};




// Sound instance flags
#define	SOUNDINSTANCEFLAG_READY			0
#define	SOUNDINSTANCEFLAG_FIRSTUPDATE	(1<<0)
#define	SOUNDINSTANCEFLAG_PLAYING		(1<<1)
#define	SOUNDINSTANCEFLAG_DONE			(1<<2)
#define	SOUNDINSTANCEFLAG_WASPLAYING	(1<<3)
#define SOUNDINSTANCEFLAG_EARSHOT		(1<<4)
#define SOUNDINSTANCEFLAG_PAUSED		(1<<5)
#define SOUNDINSTANCEFLAG_ENDLOOP		(1<<6)

//class CStreamingSoundBuffer;

class CSoundInstance
{
public:

	CSoundInstance( )
	;

	virtual ~CSoundInstance( )
	;

	virtual LTRESULT	Init( CSoundBuffer &soundBuffer, PlaySoundInfo &playSoundInfo, uint32 dwOffsetTime = 0 )
	;

	virtual void	Term( )
	;

	SoundType		GetType( ) const
	{ return m_eType; }

	HLTSOUND		GetHSoundDE( ) const
	{ return m_hSound; }

	uint8			GetPriority( ) const
	{ return m_nPriority; }

	float			GetModifiedPriority( ) const
	{ return m_fModifiedPriority; }

	uint32			GetPlaySoundFlags( ) const
	{ return m_dwPlaySoundFlags; }

	void			SetSoundInstanceFlags( uint32 dwSoundInstanceFlags )
	{ m_dwSoundInstanceFlags = dwSoundInstanceFlags; }

	uint32			GetSoundInstanceFlags( ) const
	{ return m_dwSoundInstanceFlags; }

	CSoundBuffer *	GetSoundBuffer( ) const
	{ return m_pSoundBuffer; }

	uint32			GetOffsetTime( ) const
	{ return m_dwOffsetTime; }

	uint32			GetTimer( ) const
	{ return m_dwTimer; }

	void			SetTimer( uint32 dwTimer )
	{ m_dwTimer = dwTimer; }

	LTBOOL			UpdateTimer( uint32 dwFrameTime )
	;

	uint32			GetDuration( ) const
	{ return m_dwDuration; }

	LTRESULT		AcquireSample( )
	;

	virtual LTRESULT	Acquire3DSample( )
	;

	LTRESULT		AcquireStream( )
	;

//	===========================================================================
#ifdef USE_ABSTRACT_SOUND_INTERFACES
//	===========================================================================

	LHSAMPLE		GetSample( ) const { return m_hSample; }
	LH3DSAMPLE		Get3DSample( ) const { return m_h3DSample; }
	LHSTREAM		GetStream( ) const { return m_hStream; }

//	===========================================================================
#else
//	===========================================================================

	HSAMPLE			GetSample( ) const { return m_hSample; }
	H3DSAMPLE		Get3DSample( ) const { return m_h3DSample; }
	HSTREAM			GetStream( ) const { return m_hStream; }

//	===========================================================================
#endif
//	===========================================================================

	virtual LTRESULT	UpdateOutput( uint32 dwFrameTime )
	= 0;

	virtual LTRESULT	Preupdate( LTVector const& vListenerPos )
	= 0;

	virtual LTRESULT	Stop( LTBOOL bForce = LTFALSE )
	;

	virtual LTRESULT	Silence( LTBOOL bForce = LTFALSE )
	;

	virtual LTRESULT	Pause( )
	;

	virtual LTRESULT	Resume( )
	;

	LTRESULT			EndLoop( )
	;

	LTRESULT			DisconnectFromServer( )
	;

	virtual LTRESULT	SetObstruction( LTFLOAT fLevel)
	{ return LT_ERROR; };

	virtual LTRESULT	GetObstruction( LTFLOAT &fLevel) 
	{ return LT_ERROR; };

	virtual LTRESULT	SetOcclusion( LTFLOAT fLevel) 
	{ return LT_ERROR; };

	virtual LTRESULT	GetOcclusion( LTFLOAT &fLevel) 
	{ return LT_ERROR; };

	virtual LTRESULT	SetPosition( const LTVector &vPos, LTBOOL bTeleport = LTFALSE )
	;

	virtual LTRESULT	SetVolume(uint16 nOrigVolume);

	virtual LTRESULT	GetVolume(uint16 &nVolume);

	virtual LTRESULT	SetPan(uint16 nPan);

	virtual LTRESULT	GetPosition( LTVector &vPos )
	{ vPos = m_vPosition; return LT_OK; }

	float				GetOuterRadius( ) { return m_fOuterRadius; }
	float				GetInnerRadius( ) { return m_fInnerRadius; }

	virtual LTRESULT	Get3DSamplePosition( LTVector &vPos )
	{ return LT_OK; }

	uint32			GetListIndex( ) const
	{ return m_dwListIndex; }

	void			SetListIndex( uint32 dwIndex )
	{ m_dwListIndex = dwIndex; }

	const LTLink *	GetSoundBufferLink( ) const
	{ return &m_BufferLink; }

	LTRESULT		Unload( )
	;

	LTRESULT		Reload( )
	;

	uint16			GetCollisions( )
	{ return m_nNumCollisions; }

	void			SetCollisions( uint16 nCollisions )
	{ m_nNumCollisions = nCollisions; }

protected:

	virtual LTRESULT StartRendering( )
	;

	LTRESULT		PreUpdatePositionalSound( LTVector const& vListenerPos );

protected:

#ifdef USE_DX8_SOFTWARE_FILTERS
	// helper function to convert filter description to filterdata
	bool GetSoundFilterType( LTSOUNDFILTERDATA* pFilterData, const char* strFilterName );
	virtual LTRESULT	SetFilter( const char* pszFilter );
	virtual LTRESULT	SetFilterParam( const char* pParam, float fValue );
#endif 


	SoundType		m_eType;

	CSoundBuffer *	m_pSoundBuffer;
	FileIdentifier *m_pFileIdent;

	uint32			m_dwPlaySoundFlags;
	HLTSOUND		m_hSound;

	uint8			m_nPriority;
	float			m_fModifiedPriority;
	uint8			m_nVolume;
	uint16			m_nCurRawVolume;
	uint16			m_nCurPan;
	uint32			m_dwTimer;			// In ms
	uint32			m_dwOffsetTime;		// In ms
	uint32			m_dwDuration;
	uint32			m_dwLastTime;
	float			m_fPitchShift;

	uint32			m_dwPauseCount;		// Reference count of number of times paused
	uint32			m_dwResumeTime;		// In ms

	uint32			m_dwSoundInstanceFlags;

	LHSAMPLE		m_hSample;
	LH3DSAMPLE		m_h3DSample;
	LHSTREAM		m_hStream;

	uint16			m_nNumCollisions;

	uint32			m_dwListIndex;
	LTLink			m_BufferLink;

	LTVector		m_vPosition;

	float			m_fInnerRadius;
	float			m_fOuterRadius;

#ifdef USE_DX8_SOFTWARE_FILTERS
	// for filter support
	LTSOUNDFILTERDATA m_FilterData;
#endif

	// for sound type support
	uint8			m_nSoundClass;
};

class CLocalSoundInstance : public CSoundInstance
{
public:

	virtual LTRESULT	Init( CSoundBuffer &soundBuffer, PlaySoundInfo &playSoundInfo, uint32 dwOffsetTime = 0 )
	;

	virtual LTRESULT	UpdateOutput( uint32 dwFrameTime )
	;

	virtual LTRESULT	Preupdate( LTVector const& vListenerPos );

	virtual LTRESULT	Get3DSamplePosition( LTVector &vPos )
	;
};

class CAmbientSoundInstance : public CSoundInstance
{
public:

	CAmbientSoundInstance( )
	;

	virtual LTRESULT	Init( CSoundBuffer &soundBuffer, PlaySoundInfo &playSoundInfo, uint32 dwOffsetTime = 0 )
	;

	virtual LTRESULT	UpdateOutput( uint32 dwFrameTime )
	;

	virtual LTRESULT	Preupdate( LTVector const& vListenerPos )
	;

	virtual LTRESULT	Get3DSamplePosition( LTVector &vPos )
	;

};

class C3DSoundInstance : public CSoundInstance
{
public:

	C3DSoundInstance( )
	;

	virtual LTRESULT	Init( CSoundBuffer &soundBuffer, PlaySoundInfo &playSoundInfo, uint32 dwOffsetTime = 0 )
	;

	virtual LTRESULT	UpdateOutput( uint32 dwFrameTime )
	;

	virtual LTRESULT	Preupdate( LTVector const& vListenerPos )
	;

	virtual LTRESULT	SetObstruction( LTFLOAT fLevel)
	;

	virtual LTRESULT	GetObstruction( LTFLOAT &fLevel)
	;

	virtual LTRESULT	SetOcclusion( LTFLOAT fLevel)
	;

	virtual LTRESULT	GetOcclusion( LTFLOAT &fLevel)
	;

	virtual LTRESULT	SetPosition( const LTVector &vPos, LTBOOL bTeleport = LTFALSE )
	;


	virtual LTRESULT	Get3DSamplePosition( LTVector &vPos )
	;

protected:

	LTVector			m_vLastPosition;
	LTVector			m_vVelocity;

};

inline LTRESULT CSoundInstance::SetPosition( const LTVector &vPos, LTBOOL bTeleport )
{
	m_vPosition = vPos;
	return LT_OK;
}


inline LTRESULT C3DSoundInstance::SetPosition( const LTVector &vPos, LTBOOL bTeleport )
{
	m_vPosition = vPos;
	if( bTeleport )
	{
		m_vLastPosition = vPos;
		m_vVelocity.Init();
	}

	return LT_OK;
}

#endif // __SOUNDINSTANCE_H__

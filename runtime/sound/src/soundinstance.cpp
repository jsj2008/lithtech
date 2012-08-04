#include "bdefs.h"

#include "soundinstance.h"
#include "soundbuffer.h"
#include "de_objects.h"
#include "clientmgr.h"
#include "clientshell.h"

//	===========================================================================
#ifdef USE_ABSTRACT_SOUND_INTERFACES
//	===========================================================================

typedef LHSAMPLE				HSAMPLE;
typedef LH3DPOBJECT				H3DPOBJECT;
typedef LH3DSAMPLE				H3DSAMPLE;
typedef LHDIGDRIVER				HDIGDRIVER;
typedef LHSTREAM				HSTREAM;

typedef	LTSOUNDINFO				AILSOUNDINFO;

 
#define GetSoundSys SoundSys

#endif
//	===========================================================================


#ifdef USE_DX8_SOFTWARE_FILTERS

// filter stuff
typedef struct
{
	int FilterID;
	char* FilterName;
} FilterDesc;

FilterDesc FilterList[] =
{
	FilterChorus,		"Chorus Filter",
	FilterCompressor,	"Compressor Filter",
	FilterDistortion,	"Distortion Filter",
	FilterEcho,			"Echo Filter",
	FilterFlange,		"Flange Filter",
	FilterParamEQ,		"ParamEQ Filter",
	FilterReverb,		"Reverb Filter"
};

// one of each type of sound filter, we'll only need one to work
// with at any given time
static LTFILTERCHORUS g_FilterChorus;
static LTFILTERCOMPRESSOR g_FilterCompressor;
static LTFILTERDISTORTION g_FilterDistortion;
static LTFILTERECHO	g_FilterEcho;
static LTFILTERFLANGE g_FilterFlange;
static LTFILTERPARAMEQ g_FilterParamEQ;
static LTFILTERREVERB g_FilterReverb;

#endif



CSoundInstance::CSoundInstance( )
{
	m_eType = SOUNDTYPE_LOCAL;
	m_pSoundBuffer = LTNULL;
	m_dwPlaySoundFlags = 0;
	m_dwSoundInstanceFlags = SOUNDINSTANCEFLAG_READY;
	m_nPriority = 0;
	m_nVolume = 100;
	m_nCurRawVolume = 0;
	m_fPitchShift = 1.0f;
	m_dwOffsetTime = 0;
	m_dwTimer = 0;
	m_dwLastTime = 0;
	m_dwPauseCount = 0;
	m_dwResumeTime = 0;
	m_hSound = ( HLTSOUND )INVALID_OBJECTID;
	m_hSample = LTNULL;
	m_h3DSample = LTNULL;
	m_hStream = LTNULL;
 
	m_nNumCollisions = 0;
	m_fModifiedPriority = 0;
	m_dwListIndex = 0;
	dl_TieOff( &m_BufferLink );

#ifdef USE_DX8_SOFTWARE_FILTERS
	// no filtering at start
	m_FilterData.bUseFilter = false;
#endif
}

CSoundInstance::~CSoundInstance( )
{
	CSoundInstance::Term( );
}

LTRESULT	CSoundInstance::Init( CSoundBuffer &soundBuffer, PlaySoundInfo &playSoundInfo, uint32 dwOffsetTime )
{
	// Start fresh
	Term( );

	m_dwSoundInstanceFlags = SOUNDINSTANCEFLAG_READY;

	m_pSoundBuffer = &soundBuffer;
	m_pFileIdent = ( FileIdentifier * )m_pSoundBuffer->GetFileIdent( );

	m_dwPlaySoundFlags = playSoundInfo.m_dwFlags;
	m_nPriority = playSoundInfo.m_nPriority;
	if( m_dwPlaySoundFlags & PLAYSOUND_CTRL_VOL )
		m_nVolume = playSoundInfo.m_nVolume;
	if( m_dwPlaySoundFlags & PLAYSOUND_CTRL_PITCH )
		m_fPitchShift = playSoundInfo.m_fPitchShift;

	// Add in optional random pitch.
	m_fPitchShift *= ( 1.0f + m_pSoundBuffer->RandomPitchMod( ));
	if( m_fPitchShift <= 0.0f )
		m_fPitchShift = 1.0f;

	// store sound type information
	m_nSoundClass = playSoundInfo.m_nSoundVolumeClass;

	m_dwOffsetTime = dwOffsetTime;
	m_nNumCollisions = 0;
	m_hSample = LTNULL;
	m_h3DSample = LTNULL;
	m_hStream = LTNULL;
	m_fModifiedPriority = ( float )m_nPriority + 1.0f;
	m_dwResumeTime = 0;

	// If the sound is client side, then set the sound handle as this object
	if( m_dwPlaySoundFlags & PLAYSOUND_CLIENT )
	{
		m_hSound = (HLTSOUND)INVALID_OBJECTID;
		//playSoundInfo.m_hSound = (HLTSOUND)this;
	}
	// If server side sound, then the hsound comes from the playsoundinfo
	else
	{ 
 		m_hSound = ( HLTSOUND )playSoundInfo.m_hSound;
		g_pClientMgr->AddToObjectMap((uint16)playSoundInfo.m_hSound );
		g_pClientMgr->m_ObjectMap[( uint16 )playSoundInfo.m_hSound].m_nRecordType = RECORDTYPE_SOUND;
		g_pClientMgr->m_ObjectMap[( uint16 )playSoundInfo.m_hSound].m_pRecordData = this;
	}

	// Set this so the game can use it...
	playSoundInfo.m_hSound = (HLTSOUND)this;

	// Get instance and force decompress if 3d.
	if( m_pSoundBuffer->AddInstance( *this ) != LT_OK )
		return LT_ERROR;

	m_dwListIndex = 0;

	// Set timer.  Adjust for pitch shift.
	m_dwDuration = ( uint32 )(( float )m_pSoundBuffer->GetDuration( ) / m_fPitchShift );
	SetTimer( m_dwDuration );

	// Check if there is an offset into the sound
	if( m_dwOffsetTime > 0 )
	{
		// Check if offset goes past the end of the sound
		if( m_dwOffsetTime > m_dwDuration )
		{
			// Check if it should wrap
			if( m_dwPlaySoundFlags & PLAYSOUND_LOOP )
			{
				SetTimer( m_dwTimer - (( m_dwOffsetTime - m_dwDuration ) % m_dwDuration ));
			}
			else
			{
				m_dwSoundInstanceFlags &= ~SOUNDINSTANCEFLAG_PLAYING;
				m_dwSoundInstanceFlags |= SOUNDINSTANCEFLAG_DONE;
			}
		}
		else
		{
			SetTimer( m_dwTimer - m_dwOffsetTime );
		}
	}

	m_dwSoundInstanceFlags |= SOUNDINSTANCEFLAG_FIRSTUPDATE;

	return LT_OK;
}

void CSoundInstance::Term( )
{
	// Make sure we're stopped
	Stop( );

	DisconnectFromServer( );

	// Kill any open stream.
	if( m_hStream )
	{
		GetSoundSys()->CloseStream( m_hStream );
		m_hStream = LTNULL;
	}

 
	m_dwSoundInstanceFlags = SOUNDINSTANCEFLAG_READY;
	if( m_pSoundBuffer )
	{
//		dl_RemoveAt(( DList * )m_pSoundBuffer->GetInstanceList( ), &m_BufferLink );
		m_pSoundBuffer->RemoveInstance( *this );
		// Unload the buffer if we're only playing it once
		if ((m_dwPlaySoundFlags & PLAYSOUND_ONCE) != 0)
		{
			// Unload if nobody's using it
			if (m_pSoundBuffer->GetInstanceList()->m_nElements == 0)
				m_pSoundBuffer->Unload();
		}
		m_pSoundBuffer = LTNULL;
	}
	dl_TieOff( &m_BufferLink );
	m_pFileIdent = LTNULL;

	m_dwPlaySoundFlags = 0;
	m_nPriority = 0;
	m_nVolume = 100;
	m_fPitchShift = 1.0f;
	m_dwOffsetTime = 0;
	m_nNumCollisions = 0;
	m_dwResumeTime = 0;

	m_hSample = LTNULL;
	m_h3DSample = LTNULL;
	m_hStream = LTNULL;

	m_fModifiedPriority = 0;
	m_dwListIndex = 0;

#ifdef USE_DX8_SOFTWARE_FILTERS
	m_FilterData.bUseFilter = LTFALSE;
	m_FilterData.uiFilterType = NULL_FILTER;
	m_FilterData.pSoundFilter = NULL;
#endif
}

LTRESULT CSoundInstance::DisconnectFromServer( )
{
	// If this is a server sound, then remove it from the id list...
	if(( uint16 )m_hSound != INVALID_OBJECTID && !( m_dwPlaySoundFlags & PLAYSOUND_CLIENT ))
	{
		g_pClientMgr->ClearObjectMapEntry((uint16)m_hSound);
		m_hSound = ( HLTSOUND )INVALID_OBJECTID;
	}

	return LT_OK;
}

LTRESULT CSoundInstance::Stop( LTBOOL bForce )
{
	m_dwTimer = 0;
	m_dwSoundInstanceFlags |= SOUNDINSTANCEFLAG_DONE;
	Silence( bForce );

	return LT_OK;
}

LTRESULT CSoundInstance::Silence( LTBOOL bForce )
{
	m_dwSoundInstanceFlags &= ~SOUNDINSTANCEFLAG_PLAYING;

	if( m_hSample )
	{
		GetSoundSys()->StopSample( m_hSample );
		GetClientILTSoundMgrImpl()->LinkSampleSoundInstance( m_hSample, LTNULL );
		GetClientILTSoundMgrImpl()->ReleaseSWSample( m_hSample );
		m_hSample = LTNULL;
	}
	else if( m_h3DSample )
	{
		// Only stop the sound if it's still going
		if( bForce || GetSoundSys()->Get3DSampleStatus( m_h3DSample ) == LS_PLAYING )
			GetSoundSys()->Stop3DSample( m_h3DSample );

		GetClientILTSoundMgrImpl()->Link3DSampleSoundInstance( m_h3DSample, LTNULL );
		GetClientILTSoundMgrImpl()->Release3DSample( m_h3DSample );
		m_h3DSample = LTNULL;
	}
	else if( m_hStream )
	{
		GetSoundSys()->CloseStream( m_hStream );
		m_hStream = LTNULL;
	}

 
	return LT_OK;
}

LTRESULT CSoundInstance::Pause( )
{
	if( m_dwSoundInstanceFlags & SOUNDINSTANCEFLAG_PLAYING )
		Silence( LTTRUE );

	if( m_dwPauseCount == 0 )
	{
		m_dwSoundInstanceFlags |= SOUNDINSTANCEFLAG_PAUSED;
		m_dwResumeTime = m_dwTimer;
	}
	m_dwPauseCount++;

	return LT_OK;
}

LTRESULT CSoundInstance::Resume( )
{
	if( !( m_dwSoundInstanceFlags & SOUNDINSTANCEFLAG_PAUSED ) || m_dwPauseCount == 0 )
		return LT_ERROR;

	m_dwPauseCount--;
	if( m_dwPauseCount == 0 )
	{
		m_dwSoundInstanceFlags &= ~SOUNDINSTANCEFLAG_PAUSED;
		SetTimer( m_dwResumeTime );
		m_dwResumeTime = 0;
	}

	return LT_OK;
}

LTBOOL CSoundInstance::UpdateTimer( uint32 dwFrameTime )
{
	uint32 dwCurTime, dwModLoopPoint[2];
	int nTime;

	// Don't update the timer if paused
	if( m_dwSoundInstanceFlags & SOUNDINSTANCEFLAG_PAUSED )
	{
		return LTTRUE;
	}

	// The first update doesn't update the timer because the full frametime wasn't 
	// spent playing the sound
	if( m_dwSoundInstanceFlags & SOUNDINSTANCEFLAG_FIRSTUPDATE )
	{
		m_dwSoundInstanceFlags &= ~SOUNDINSTANCEFLAG_FIRSTUPDATE;
		return LTTRUE;
	}

	// Check if the sound is playing through channel
	if( m_dwSoundInstanceFlags & SOUNDINSTANCEFLAG_PLAYING )
	{
		// /Base frame time on a timer
		dwCurTime = GetSoundSys()->MsCount( );

		dwFrameTime = dwCurTime - m_dwLastTime;
		m_dwLastTime = dwCurTime;
	}

	ASSERT( m_pSoundBuffer );
	if( !m_pSoundBuffer )
	{
		SetTimer( 0 );
		return LTFALSE;
	}

	// Check if timer has an initial delay.  If the delay just ran out, then
	// start the sound at the beginning, ignoring any time into the sound.  This
	// is done because of the granularity of the framerate could chop the
	// beginning of the sound off
	if( m_dwTimer > m_dwDuration && m_dwTimer - m_dwDuration < dwFrameTime )
		SetTimer( m_dwDuration );
	else
	{
		// If looping and not heading for the end, then make sure the timer stays within the loop period.
		if( GetPlaySoundFlags( ) & PLAYSOUND_LOOP && !( m_dwSoundInstanceFlags & SOUNDINSTANCEFLAG_ENDLOOP ))
		{
			// Get ascending timer.
			nTime = ( int )m_dwDuration - ( int )m_dwTimer;
			dwModLoopPoint[0] = ( uint32 )(( float )m_pSoundBuffer->GetLoopPoint( 0 ) / m_fPitchShift );
			dwModLoopPoint[1] = ( uint32 )(( float )m_pSoundBuffer->GetLoopPoint( 1 ) / m_fPitchShift );
			if( dwModLoopPoint[1] - nTime <= dwFrameTime )
			{
				nTime = nTime + dwFrameTime - dwModLoopPoint[0];
				nTime %= dwModLoopPoint[1] - dwModLoopPoint[0];
				nTime += dwModLoopPoint[0];
				m_dwTimer = m_dwDuration - nTime;
			}
			else if( m_nNumCollisions != 1 )
			{
				m_dwTimer -= dwFrameTime;
			}
		}
		// Check if sound just timed out
		else if( m_dwTimer <= dwFrameTime )
		{
			m_dwTimer = 0;
		}
		// Skip updating the timer if sound just had a collision.  This allows
		// the sound to start on the next frame.  Otherwise update it
		else if( m_nNumCollisions != 1 )
		{
			m_dwTimer -= dwFrameTime;
		}
	}

	// Check if our timer ran out.
	if( m_dwTimer == 0 )
	{
		// If it's not looping or ending a loop, then check if it's supposed to be still playing.
		// If it is, then check the actual dx buffer to see if it's playing.
		if( !( m_dwPlaySoundFlags & PLAYSOUND_LOOP ) || ( m_dwSoundInstanceFlags & SOUNDINSTANCEFLAG_ENDLOOP ))
		{
			if( m_dwSoundInstanceFlags & SOUNDINSTANCEFLAG_PLAYING )
			{
				S32 nStatus = LS_STOPPED;
				if( m_hStream )
				{
					nStatus = GetSoundSys( )->GetStreamStatus( m_hStream );
				}
				else if( m_h3DSample )
				{
					nStatus = GetSoundSys( )->Get3DSampleStatus( m_h3DSample );
				}
				else if( m_hSample )
				{
					nStatus = GetSoundSys( )->GetSampleStatus( m_hSample );
				}

				// If the buffer is still playing, don't kill it yet.  Sometimes the timer
				// calculations can be wrong on the compressed waves.
				if( nStatus == LS_PLAYING )
				{
					return LTTRUE;
				}
				else
				{
					return LTFALSE;
				}
				
			}
			else
			{
				return LTFALSE;
			}
		}
	}

	return LTTRUE;

}

LTRESULT CSoundInstance::GetVolume(uint16 &nVolume)
{
	nVolume = m_nVolume;

	return LT_OK;
}

LTRESULT CSoundInstance::SetVolume(uint16 nOrigVolume)
{
	uint16 nVolume;
	float fVolumeClassMultiplier, fVolumeGlobalMultiplier;
	bool bUseGlobalMultiplier = true;

	m_nCurRawVolume = nOrigVolume;

	GetClientILTSoundMgrImpl()->GetSoundClassMultiplier( m_nSoundClass, &fVolumeClassMultiplier );
	GetClientILTSoundMgrImpl()->GetVolumeMultiplier( fVolumeGlobalMultiplier );
	GetClientILTSoundMgrImpl()->GetSoundClassUseGlobalVolume( m_nSoundClass, &bUseGlobalMultiplier );
	
	nVolume = (uint16)((float) nOrigVolume * fVolumeClassMultiplier * (bUseGlobalMultiplier ? fVolumeGlobalMultiplier : 1.0));
	nVolume *= 127;
	nVolume /= 100;

	if ( m_pSoundBuffer->GetSoundBufferFlags() & SOUNDBUFFERFLAG_STREAM)
	{
		// handle stream
		if ( m_hStream )
			GetSoundSys()->SetStreamVolume( m_hStream, nVolume );
	}
	else if ( m_h3DSample )
	{
		// handle 3D sound
		GetSoundSys()->Set3DSampleVolume( m_h3DSample, nVolume );
	}
	else if ( m_hSample )
	{
		// normal sample
		GetSoundSys()->SetSampleVolume( m_hSample, nVolume );
	}
	else
		return LT_ERROR;

	return LT_OK;
}

LTRESULT CSoundInstance::SetPan(uint16 nPan)
{
	m_nCurPan = nPan;
	if ( m_hSample )
	{
		GetSoundSys()->SetSamplePan( m_hSample, nPan );
	}
	else if ( m_hStream )
	{
		GetSoundSys()->SetStreamPan( m_hStream, nPan );
	}

	return LT_OK;
}

LTRESULT CSoundInstance::AcquireSample( )
{
	S32 nPos1, nPos2;
	float fModPoint;
	S32 nPlayBackRate;

	ASSERT( m_pSoundBuffer );

	// Release old sample
	if( !m_hSample )
	{
		m_hSample = GetClientILTSoundMgrImpl()->GetFreeSWSample( );
		if( !m_hSample )
			return LT_ERROR;
	}

	nPlayBackRate = m_pSoundBuffer->GetPlaybackRate( );
	nPlayBackRate = ( S32 )(( float )nPlayBackRate * m_fPitchShift + 0.5f );

	// set up any filter info
	LTSOUNDFILTERDATA* pFilterData;

#ifdef USE_DX8_SOFTWARE_FILTERS
	if ( m_FilterData.bUseFilter )
		pFilterData = &m_FilterData;
	else
#endif
		pFilterData = NULL;

	// If the sample is not compressed or if it has been decompressed, we can use the faster
	// call.
	if( !m_pSoundBuffer->IsCompressed( ) || m_pSoundBuffer->GetDecompressedSoundBuffer( ))
	{
		if( !GetSoundSys()->InitSampleFromAddress( m_hSample, m_pSoundBuffer->GetSoundData( ), m_pSoundBuffer->GetSoundDataLen( ), m_pSoundBuffer->GetWaveFormat(), nPlayBackRate, pFilterData ))
		{
			GetClientILTSoundMgrImpl()->ReleaseSWSample( m_hSample );
			m_hSample = LTNULL;
			return LT_ERROR;
		}
	}
	else
	{
		if( !GetSoundSys()->InitSampleFromFile( m_hSample, m_pSoundBuffer->GetFileData( ), 0, nPlayBackRate, pFilterData ))
		{
			GetClientILTSoundMgrImpl()->ReleaseSWSample( m_hSample );
			m_hSample = LTNULL;
			return LT_ERROR;
		}
	}

  	// Check if we're to loop the sound.
	if( m_dwPlaySoundFlags & PLAYSOUND_LOOP )
	{
		bool bEnableLoopBlock = false;

		// Check if there's a valid cue point region.
		if( m_pSoundBuffer->GetWaveHeader( LTFALSE ).m_CuePoint.m_bValid )
		{
			fModPoint = ( float )m_pSoundBuffer->GetLoopPoint( 0 ) / m_fPitchShift;
			nPos1 = ( S32 )(( fModPoint / ( float )m_dwDuration ) * 
				( float )m_pSoundBuffer->GetSoundDataLen( ));
			nPos1 = LTCLAMP( nPos1, ( S32 )0, ( S32 )m_pSoundBuffer->GetSoundDataLen( ));
			fModPoint = ( float )m_pSoundBuffer->GetLoopPoint( 1 ) / m_fPitchShift;
			nPos2 = ( S32 )(( fModPoint / ( float )m_dwDuration ) * 
				( float )m_pSoundBuffer->GetSoundDataLen( ));
			nPos2 = LTCLAMP( nPos2, nPos1, ( S32 )m_pSoundBuffer->GetSoundDataLen( ));
			bEnableLoopBlock = true;
		}
		// No cue points, so loop the whole thing.
		else
		{
			nPos1 = 0;
			nPos2 = -1;
		}
		GetSoundSys()->SetSampleLoopBlock( m_hSample, nPos1, nPos2, bEnableLoopBlock );
		GetSoundSys()->SetSampleLoop( m_hSample, true );
	}
	else
	{
		GetSoundSys()->SetSampleLoopBlock( m_hSample, 0, -1, false );
		GetSoundSys()->SetSampleLoop( m_hSample, false );
	}

  	// Check if this sample should not have reverb
	if( !GetClientILTSoundMgrImpl()->UseSWReverb( ) || !( m_dwPlaySoundFlags & PLAYSOUND_REVERB ))
	{
		GetSoundSys()->SetSampleReverb( m_hSample, 0.0f, 0.0f, 0.0f );
	}

	SetPan( 64 );
//	GetSoundSys()->SetSamplePan( m_hSample, 64 );

	SetVolume( m_nVolume );

	GetClientILTSoundMgrImpl()->LinkSampleSoundInstance( m_hSample, this );

	return LT_OK;
}

LTRESULT CSoundInstance::Acquire3DSample( )
{
   	S32 nPos1, nPos2;
	float fModPoint;
	LTVector vPos, vTemp;
	S32 nPlayBackRate;

	ASSERT( m_pSoundBuffer );

	if( m_pSoundBuffer->GetSoundBufferFlags( ) & SOUNDBUFFERFLAG_STREAM )
		return LT_ERROR;

	if( !m_h3DSample )
	{
		m_h3DSample = GetClientILTSoundMgrImpl()->GetFree3DSample( this );
		if( !m_h3DSample )
			return LT_ERROR;
	}

	// get the playback rate including any pitch-shifting
	nPlayBackRate = m_pSoundBuffer->GetPlaybackRate( );
	nPlayBackRate = ( S32 )(( float )nPlayBackRate * m_fPitchShift + 0.5f );

	// If this sample isn't already setup with this buffer or the hardware doesn't use onboard memory
	// then we'll just re-initialize the sample.  Re-initializing the sample avoids a driver bug with 
	// creative sblive cards on win2k.
	if( !GetSoundSys()->HasOnBoardMemory( ) ||
		( CSoundBuffer* )GetSoundSys()->Get3DUserData( m_h3DSample, SAMPLE_BUFFER ) != m_pSoundBuffer )
	{
		// set up any filtering
		LTSOUNDFILTERDATA *pFilterData;
#ifdef USE_DX8_SOFTWARE_FILTERS
		if ( m_FilterData.bUseFilter )
			pFilterData = &m_FilterData;
		else
#endif
			pFilterData = NULL;

		// Clear the buffer this sample may have been using before.
		GetSoundSys()->Set3DUserData( m_h3DSample, SAMPLE_BUFFER, 0 );

		// If the sample is not compressed or if it has been decompressed, we can use the faster
		// call.
		if( !m_pSoundBuffer->IsCompressed( ) || m_pSoundBuffer->GetDecompressedSoundBuffer( ))
		{
			if ( !GetSoundSys()->Init3DSampleFromAddress( m_h3DSample, m_pSoundBuffer->GetSoundData( ), m_pSoundBuffer->GetSoundDataLen( ), m_pSoundBuffer->GetWaveFormat(), nPlayBackRate, pFilterData ) )
			{
				GetClientILTSoundMgrImpl()->Release3DSample( m_h3DSample );
				m_h3DSample = LTNULL;
				return LT_ERROR;
			}
		}
		else
		{
			if( !GetSoundSys()->Init3DSampleFromFile( m_h3DSample, m_pSoundBuffer->GetFileData( ), 0, nPlayBackRate, pFilterData ))
			{
				GetClientILTSoundMgrImpl()->Release3DSample( m_h3DSample );
				m_h3DSample = LTNULL;
				return LT_ERROR;
			}
		}

		GetSoundSys()->Set3DUserData( m_h3DSample, SAMPLE_BUFFER, ( uint32 )m_pSoundBuffer );
	}

	GetSoundSys()->Set3DSampleDistances( m_h3DSample, m_fOuterRadius, m_fInnerRadius );

	SetVolume( m_nVolume );

	GetSoundSys()->Get3DPosition( m_h3DSample, &vTemp.x, &vTemp.y, &vTemp.z );
	Get3DSamplePosition( vPos );
	if( vTemp.DistSqr(vPos) > 0.001f )
		GetSoundSys()->Set3DPosition( m_h3DSample, vPos.x, vPos.y, vPos.z );
	GetSoundSys()->Set3DVelocityVector( m_h3DSample, 0.0f, 0.0f, 0.0f );

	GetClientILTSoundMgrImpl()->Link3DSampleSoundInstance( m_h3DSample, this );

  	// Check if we're to loop the sound.
	if( m_dwPlaySoundFlags & PLAYSOUND_LOOP )
	{
		bool bEnableLoopBlock = false;

		// Check if there's a valid cue point region.
		if( m_pSoundBuffer->GetWaveHeader( LTFALSE ).m_CuePoint.m_bValid )
		{
			fModPoint = ( float )m_pSoundBuffer->GetLoopPoint( 0 ) / m_fPitchShift;
			nPos1 = ( S32 )(( fModPoint / ( float )m_dwDuration ) * 
				( float )m_pSoundBuffer->GetSoundDataLen( ));
			nPos1 = LTCLAMP( nPos1, ( S32 )0, ( S32 )m_pSoundBuffer->GetSoundDataLen( ));
			fModPoint = ( float )m_pSoundBuffer->GetLoopPoint( 1 ) / m_fPitchShift;
			nPos2 = ( S32 )(( fModPoint / ( float )m_dwDuration ) * 
				( float )m_pSoundBuffer->GetSoundDataLen( ));
			nPos2 = LTCLAMP( nPos2, nPos1, ( S32 )m_pSoundBuffer->GetSoundDataLen( ));
			bEnableLoopBlock = true;
		}
		// No cue points, so loop the whole thing.
		else
		{
			nPos1 = 0;
			nPos2 = -1;
		}
		GetSoundSys()->Set3DSampleLoopBlock( m_h3DSample, nPos1, nPos2, bEnableLoopBlock );
		GetSoundSys()->Set3DSampleLoop( m_h3DSample, true );
	}
	else
	{
		GetSoundSys()->Set3DSampleLoopBlock( m_h3DSample, 0, -1, false );
		GetSoundSys()->Set3DSampleLoop( m_h3DSample, false );
	}

	return LT_OK;
}


LTRESULT CSoundInstance::AcquireStream( )
{
	S32 nPos1, nPos2;
	float fModPoint;
	char szFileName[_MAX_PATH+1];
	uint32 nFilePos, nFileSize;
	S32 nPlayBackRate;

	ASSERT( m_pSoundBuffer );
	if( !m_pSoundBuffer )
		return LT_ERROR;

	if( !m_hStream )
	{
		// Open the file directly.
		if( !df_GetRawInfo( m_pSoundBuffer->GetFileIdent( )->m_hFileTree, 
			m_pSoundBuffer->GetFileIdent( )->m_Filename, szFileName, _MAX_PATH, &nFilePos, &nFileSize ))
			return LT_ERROR;

		m_hStream = GetSoundSys()->OpenStream( szFileName, nFilePos, GetClientILTSoundMgrImpl()->GetDigDriver( ), 0, 0 );

		if( !m_hStream )
		{
			return LT_ERROR;
		}
	}
	
	// Check if we're to loop the sound.
	if( m_dwPlaySoundFlags & PLAYSOUND_LOOP )
	{
		// Check if there's a valid cue point region.
		if( m_pSoundBuffer->GetWaveHeader( LTFALSE ).m_CuePoint.m_bValid )
		{
			fModPoint = ( float )m_pSoundBuffer->GetLoopPoint( 0 ) / m_fPitchShift;
			nPos1 = ( S32 )(( fModPoint / ( float )m_dwDuration ) * 
				( float )m_pSoundBuffer->GetSoundDataLen( ));
			nPos1 = LTCLAMP( nPos1, ( S32 )0, ( S32 )m_pSoundBuffer->GetSoundDataLen( ));
			fModPoint = ( float )m_pSoundBuffer->GetLoopPoint( 1 ) / m_fPitchShift;
			nPos2 = ( S32 )(( fModPoint / ( float )m_dwDuration ) * 
				( float )m_pSoundBuffer->GetSoundDataLen( ));
			nPos2 = LTCLAMP( nPos2, nPos1, ( S32 )m_pSoundBuffer->GetSoundDataLen( ));
		}
		// No cue points, so loop the whole thing.
		else
		{
			nPos1 = 0;
			nPos2 = -1;
		}
		GetSoundSys()->SetStreamLoop( m_hStream, true );
	}

	// get the playback rate including any pitch-shifting
	nPlayBackRate = m_pSoundBuffer->GetPlaybackRate( );
	nPlayBackRate = ( S32 )(( float )nPlayBackRate * m_fPitchShift + 0.5f );

	GetSoundSys()->SetStreamPlaybackRate( m_hStream, nPlayBackRate );
	SetPan( 64 );
	//GetSoundSys()->SetStreamPan( m_hStream, 64 );
	SetVolume( m_nVolume );
	GetClientILTSoundMgrImpl()->LinkStreamSoundInstance( m_hStream, this );

	return LT_OK;
}

LTRESULT CSoundInstance::StartRendering( )
{
	const WAVEFORMATEX *pWaveFormat;
	uint32 dwCurPos;

	ASSERT( m_hSample || m_h3DSample || m_hStream );
	if( !m_hSample && !m_h3DSample && !m_hStream )
	{
		return LT_ERROR;
	}

	ASSERT( m_pSoundBuffer );
	if( !m_pSoundBuffer )
		return LT_ERROR;

	pWaveFormat = m_pSoundBuffer->GetWaveFormat( );
	ASSERT( pWaveFormat );
	if( !pWaveFormat )
	{
		return LT_ERROR;
	}

  
	m_nNumCollisions = 0;

	// Handle streaming sounds.
	if( m_hStream )
	{
		// If the timer is at the beginning, then we have the data for the first read in the streamingsoundbuffer
		if( m_dwTimer == m_dwDuration )
		{
			GetSoundSys()->StartStream( m_hStream );				
		}
		else
		{
			dwCurPos = m_dwDuration - m_dwTimer;
			GetSoundSys()->SetStreamMsPosition( m_hStream, dwCurPos );
			GetSoundSys()->PauseStream( m_hStream, 0 );
		}
	}
	else if( m_h3DSample )
	{
		dwCurPos = m_dwDuration - m_dwTimer;
		if( m_dwTimer == m_dwDuration )
			GetSoundSys()->Start3DSample( m_h3DSample );
		else
		{
			GetSoundSys()->Set3DSampleMsPosition( m_h3DSample, dwCurPos );
			GetSoundSys()->Resume3DSample( m_h3DSample );
		}
	}
	else if( m_hSample )
	{
		dwCurPos = m_dwDuration - m_dwTimer;
		GetSoundSys()->SetSampleMsPosition( m_hSample, dwCurPos );
		GetSoundSys()->ResumeSample( m_hSample );
	}

	m_dwSoundInstanceFlags |= SOUNDINSTANCEFLAG_PLAYING | SOUNDINSTANCEFLAG_WASPLAYING;
//	m_dwSoundInstanceFlags &= ~SOUNDINSTANCEFLAG_EOSREACHED;

	// Record time
	m_dwLastTime = GetSoundSys()->MsCount( );

	return LT_OK;
}

LTRESULT CSoundInstance::EndLoop( )
{
	// Check if we're to loop the sound.
	if( !( m_dwPlaySoundFlags & PLAYSOUND_LOOP ))
		return LT_ERROR;

	// Stop looping sound.
	if( m_hSample )
	{
		GetSoundSys()->SetSampleLoopBlock( m_hSample, 0, -1, false );
		GetSoundSys()->SetSampleLoop( m_hSample, false );
	}
	else if( m_h3DSample )
	{
		GetSoundSys()->Set3DSampleLoopBlock( m_h3DSample, 0, -1, false );
		GetSoundSys()->Set3DSampleLoop( m_h3DSample, false );
	}
	else if( m_hStream )
	{
		GetSoundSys()->SetStreamLoop( m_hStream, false );
	}

	m_dwSoundInstanceFlags |= SOUNDINSTANCEFLAG_ENDLOOP;

	return LT_OK;
}


LTRESULT CSoundInstance::Unload( )
{
	Silence( LTTRUE );

	if( m_pSoundBuffer )
	{
		m_pSoundBuffer->RemoveInstance( *this );
		m_pSoundBuffer = LTNULL;
	}
	dl_TieOff( &m_BufferLink );

	return LT_OK;
}


LTRESULT CSoundInstance::Reload( )
{
	// Start fresh
	Unload( );

	// Make sure our file still exists
	if( !m_pFileIdent || !m_pFileIdent->m_pData )
		return LT_ERROR;

	m_pSoundBuffer = ( CSoundBuffer * )m_pFileIdent->m_pData;
	if( !m_pSoundBuffer )
		return LT_ERROR;

	// Get instance and force decompress if 3d.
	if( m_pSoundBuffer->AddInstance( *this ) != LT_OK )
		return LT_ERROR;

	return LT_OK;
}

LTRESULT CSoundInstance::PreUpdatePositionalSound( LTVector const& vListenerPos )
{
	float fDist;

	if( m_dwPlaySoundFlags & PLAYSOUND_CLIENTLOCAL )
	{
		if( g_pClientMgr->m_pCurShell && g_pClientMgr->m_pCurShell->m_pFrameClientObject )
			m_vPosition = g_pClientMgr->m_pCurShell->m_pFrameClientObject->GetPos();
	}

	// Calculate distance from listener to sound...
	fDist = vListenerPos.Dist(m_vPosition);

	// Put priority between 0 and 1
	if( fDist > 1.0f )
		m_fModifiedPriority = m_nVolume / 100.0f / fDist;
	else
		m_fModifiedPriority = m_nVolume / 100.0f;

	// Offset by the real priority
	m_fModifiedPriority += ( float )m_nPriority;

	if( fDist <= 1.5f * m_fOuterRadius && m_nVolume > 0 )
	{
		m_dwSoundInstanceFlags |= SOUNDINSTANCEFLAG_EARSHOT;
	}
	else
	{
		m_dwSoundInstanceFlags &= ~SOUNDINSTANCEFLAG_EARSHOT;
	}

	return LT_OK;
}



LTRESULT CLocalSoundInstance::Init( CSoundBuffer &soundBuffer, PlaySoundInfo &playSoundInfo, uint32 dwOffsetTime )
{
	LTRESULT dResult;

	if(( dResult = CSoundInstance::Init( soundBuffer, playSoundInfo, dwOffsetTime )) != LT_OK )
		return dResult;

	// Local sounds are always in earshot
	m_dwSoundInstanceFlags |= SOUNDINSTANCEFLAG_EARSHOT;

	SetPosition( GetClientILTSoundMgrImpl()->GetListenerPosition( ), LTTRUE );
	m_fInnerRadius = 5.0f;
	m_fOuterRadius = 10.0f;

	return LT_OK;
}

LTRESULT CLocalSoundInstance::Get3DSamplePosition( LTVector &vPos )
{
	// Put the sound right in front of the listener
	// Position is relative to listener, not absolute
	vPos = GetClientILTSoundMgrImpl()->GetListenerFront( );
	vPos *= 5.0f;

	return LT_OK;
}



LTRESULT CLocalSoundInstance::Preupdate( LTVector const& vListenerPos )
{
	if( m_nVolume > 0 )
	{
		m_dwSoundInstanceFlags |= SOUNDINSTANCEFLAG_EARSHOT;
	}
	else
	{
		m_dwSoundInstanceFlags &= ~SOUNDINSTANCEFLAG_EARSHOT;
	}

	return LT_OK;
}

LTRESULT CLocalSoundInstance::UpdateOutput( uint32 dwFrameTime )
{
	ReverbProperties reverbProperties;
	LTVector vPosition, vUp;
	LTVector vTemp;

	H3DPOBJECT h3DListener;

	// Make sure we have a channel
	ASSERT( m_hSample || m_h3DSample || m_hStream );
	if( !m_hSample && !m_h3DSample && !m_hStream )
		return LT_ERROR;

	if( GetClientILTSoundMgrImpl()->CommitChanges( ) || !( m_dwSoundInstanceFlags & SOUNDINSTANCEFLAG_PLAYING ))
	{
		if( m_hSample )
		{
			// Set new reverb
			if( GetClientILTSoundMgrImpl()->UseSWReverb( ) && ( m_dwPlaySoundFlags & PLAYSOUND_REVERB ))
			{
				reverbProperties.m_dwParams = REVERBPARAM_VOLUME | REVERBPARAM_REFLECTTIME | REVERBPARAM_DECAYTIME;
				GetClientILTSoundMgrImpl()->GetReverbProperties( &reverbProperties );
				GetSoundSys()->SetSampleReverb( m_hSample, reverbProperties.m_fVolume, reverbProperties.m_fReflectTime, 
					reverbProperties.m_fDecayTime );
			}
		}
		else if( m_hStream )
		{
		}
		else if( m_h3DSample )
		{
			// Put the sound right in front of the listener
			// Position is relative to listener, not absolute
			h3DListener = GetClientILTSoundMgrImpl()->GetListenerObject( );
			GetSoundSys()->Get3DOrientation( h3DListener, &vPosition.x, &vPosition.y, &vPosition.z, &vUp.x, &vUp.y, &vUp.z );
			vPosition *= 5.0f;

			// Update the position & velocity
			GetSoundSys()->Get3DPosition( m_h3DSample, &vTemp.x, &vTemp.y, &vTemp.z );
			if( vTemp.DistSqr(vPosition) > 0.5f )
				GetSoundSys()->Set3DPosition( m_h3DSample, vPosition.x, vPosition.y, vPosition.z );
		}
	}

	// Check if not played yet
	if( !( m_dwSoundInstanceFlags & SOUNDINSTANCEFLAG_PLAYING ))
	{
		if( StartRendering( ) != LT_OK )
			return LT_ERROR;
	}

	return LT_OK;
}

#ifdef USE_DX8_SOFTWARE_FILTERS

LTRESULT CSoundInstance::SetFilter( const char* pszFilter )
{
	// query the sound engine and get the ID of the 
	// filter that matches the requested one
	if ( pszFilter == NULL || !GetSoundFilterType( &m_FilterData, pszFilter ) )
	{
		m_FilterData.bUseFilter = false;
		m_FilterData.uiFilterType = NULL_FILTER;

		return LT_ERROR;
	}
	else
	{
		m_FilterData.bUseFilter = true;
	}

	return LT_OK;
}

LTRESULT CSoundInstance::SetFilterParam( const char* pszParam, float fValue )
{
	if ( pszParam == NULL || !m_FilterData.bUseFilter || m_FilterData.pSoundFilter == NULL )
	{
		return LT_ERROR;
	}
	else
	{
		m_FilterData.pSoundFilter->SetParam( pszParam, fValue );
	}

	return LT_OK;
}

// helper function to convert filter names to types
// filters are stored as strings with the object
// so we need to convert them
bool CSoundInstance::GetSoundFilterType( LTSOUNDFILTERDATA* pFilterData, const char* strFilterName )
{
	int i;

	if ( strFilterName == NULL || pFilterData == NULL )
		return false;

	pFilterData->uiFilterType = NULL_FILTER;

	// see what type of filter it is
	for ( i = 0; i < NUM_SOUND_FILTER_TYPES; ++i )
	{
		if ( !strcmp( strFilterName, FilterList[i].FilterName ) )
		{
			// found a match
			pFilterData->uiFilterType = i;
			switch ( i )
			{
			case FilterChorus:
				pFilterData->pSoundFilter = &g_FilterChorus;
				break;
			case FilterCompressor:
				pFilterData->pSoundFilter = &g_FilterCompressor;
				break;
			case FilterDistortion:
				pFilterData->pSoundFilter = &g_FilterDistortion;
				break;
			case FilterEcho:
				pFilterData->pSoundFilter = &g_FilterEcho;
				break;
			case FilterFlange:
				pFilterData->pSoundFilter = &g_FilterFlange;
				break;
			case FilterParamEQ:
				pFilterData->pSoundFilter = &g_FilterParamEQ;
				break;
			case FilterReverb:
				pFilterData->pSoundFilter = &g_FilterReverb;
				break;
			}
			return true;
		}
	}

	// no match
	return false;
}

#endif 

CAmbientSoundInstance::CAmbientSoundInstance( )
{
	m_eType = SOUNDTYPE_AMBIENT;
	m_vPosition.Init();
	m_fInnerRadius = 0.0f;
	m_fOuterRadius = 0.0f;
}

LTRESULT	CAmbientSoundInstance::Init( CSoundBuffer &soundBuffer, PlaySoundInfo &playSoundInfo, uint32 dwOffsetTime )
{
	LTRESULT dResult;
	if(( dResult = CSoundInstance::Init( soundBuffer, playSoundInfo, dwOffsetTime )) != LT_OK )
		return dResult;

	SetPosition( playSoundInfo.m_vPosition );
	m_fInnerRadius = playSoundInfo.m_fInnerRadius;
	m_fOuterRadius = playSoundInfo.m_fOuterRadius + 0.01f;

	return LT_OK;
}

LTRESULT CAmbientSoundInstance::Get3DSamplePosition( LTVector &vPos )
{
	LTVector vUp;
	float fDist;

	fDist = m_vPosition.Dist( GetClientILTSoundMgrImpl()->GetListenerPosition( ) );

	// Put the sound in front of the listener at a distance
	// Position is relative to listener not absolute
	vPos = GetClientILTSoundMgrImpl()->GetListenerFront( );
	vUp = vPos.Cross(GetClientILTSoundMgrImpl()->GetListenerRight( ));
	vPos *= fDist;

	return LT_OK;
}


LTRESULT CAmbientSoundInstance::Preupdate( LTVector const& vListenerPos )
{
	return PreUpdatePositionalSound( vListenerPos );
}


LTRESULT CAmbientSoundInstance::UpdateOutput( uint32 dwFrameTime )
{
	uint8 nScaledVolume;
	float fDist, fDistSqrd, fMinDistSqrd, fMaxDistSqrd, fScale;
	LTVector vPosition, vFront, vUp;
	LTVector vTemp;

	// Make sure we have a channel
	ASSERT( m_hSample || m_h3DSample || m_hStream );
	if( !m_hSample && !m_h3DSample && !m_hStream )
		return LT_ERROR;

	if( GetClientILTSoundMgrImpl()->CommitChanges( ) || !( m_dwSoundInstanceFlags & SOUNDINSTANCEFLAG_PLAYING ))
	{
		if( m_hSample || m_hStream )
		{
			// Calculate distance from listener to sound...
			fDistSqrd = m_vPosition.DistSqr(GetClientILTSoundMgrImpl()->GetListenerPosition( ));
			fMinDistSqrd = m_fInnerRadius * m_fInnerRadius;
			fMaxDistSqrd = m_fOuterRadius * m_fOuterRadius;
			
			// Find volume scale based on distance...
			if( fDistSqrd <= fMinDistSqrd )
				fScale = 1.0;
			else if( fDistSqrd >= fMaxDistSqrd )
				fScale = 0.0;
			else
			{
				// These calculations fall off from 1.0 at mindist to 0 at maxdist linearly...
				fDist = ( float )sqrt( fDistSqrd );
				fScale = 1.0f - (( fDist - m_fInnerRadius ) / ( m_fOuterRadius - m_fInnerRadius ));
			}

			// Scale volume for roll-off...
			nScaledVolume = ( uint8 )( m_nVolume * fScale );
			nScaledVolume = LTMIN( nScaledVolume, 100 );

			// Set new volume...
			if( m_hSample )
			{
//				if( nScaledVolume != ( uint8 )GetSoundSys()->GetSampleVolume( m_hSample ))
				if( nScaledVolume != m_nCurRawVolume )
					SetVolume( nScaledVolume );
			}
			else
			{
//				if( nScaledVolume != ( uint8 )GetSoundSys()->GetStreamVolume( m_hStream ))
				if( nScaledVolume != m_nCurRawVolume )
					SetVolume( nScaledVolume );
			}
		}
		else if ( m_h3DSample )
		{
			float fScale;
			LTVector vTemp;
			
			fDist = m_vPosition.Dist(GetClientILTSoundMgrImpl()->GetListenerPosition( ));
			
			// Put the sound in front of the listener at a distance
			// Position is relative to listener not absolute
			vPosition = GetClientILTSoundMgrImpl()->GetListenerFront( );
			vUp = vPosition.Cross(GetClientILTSoundMgrImpl()->GetListenerRight( ));
			vPosition *= fDist;
			
			fDistSqrd = fDist * fDist;
			fMinDistSqrd = m_fInnerRadius * m_fInnerRadius;
			fMaxDistSqrd = m_fOuterRadius * m_fOuterRadius;
			
			// Find volume scale based on distance...
			if( fDistSqrd <= fMinDistSqrd )
				fScale = 1.0;
			else if( fDistSqrd >= fMaxDistSqrd )
				fScale = 0.0;
			else
			{
				// These calculations fall off from 1.0 at mindist to 0 at maxdist and have a 1/x curve...
				fDist = ( float )sqrt( fDistSqrd );
				fScale = 1.0f - (( fDist - m_fInnerRadius ) / ( m_fOuterRadius - m_fInnerRadius ));
			}

			GetSoundSys()->Get3DPosition( m_h3DSample, &vTemp.x, &vTemp.y, &vTemp.z );
			if( vTemp.DistSqr( vPosition ) > 0.5f )
				GetSoundSys()->Set3DPosition( m_h3DSample, vPosition.x, vPosition.y, vPosition.z );

			// set the volume of the sound so we can avoid DirectSound's exponential volume curve
			uint32 nVolume = ( uint32 )( m_nVolume * fScale );
			nVolume = LTMIN( nVolume, 100 );
			if( nVolume != m_nCurRawVolume )
			{
				SetVolume( nVolume );
			}
		}
	}

	// Check if not played yet
	if( !( m_dwSoundInstanceFlags & SOUNDINSTANCEFLAG_PLAYING ))
	{
		if( StartRendering( ) != LT_OK )
			return LT_ERROR;
	}

	return LT_OK;
}

C3DSoundInstance::C3DSoundInstance( )
{
	m_eType = SOUNDTYPE_3D;
	m_vPosition.Init();
	m_vLastPosition.Init();
	m_vVelocity.Init();
	m_fInnerRadius = 0.0f;
	m_fOuterRadius = 0.0f;
}


LTRESULT	C3DSoundInstance::Init( CSoundBuffer &soundBuffer, PlaySoundInfo &playSoundInfo, uint32 dwOffsetTime )
{
	LTRESULT dResult;
	if(( dResult = CSoundInstance::Init( soundBuffer, playSoundInfo, dwOffsetTime )) != LT_OK )
		return dResult;

	SetPosition( playSoundInfo.m_vPosition, LTTRUE );
	m_fInnerRadius = playSoundInfo.m_fInnerRadius;
	m_fOuterRadius = playSoundInfo.m_fOuterRadius + 0.01f;

	return LT_OK;
}

LTRESULT C3DSoundInstance::Get3DSamplePosition( LTVector &vPos )
{

	// Update the position & velocity
	// Position is relative to listener not absolute
	vPos = m_vPosition - GetClientILTSoundMgrImpl()->GetListenerPosition( );

	return LT_OK;
}


LTRESULT C3DSoundInstance::Preupdate( LTVector const& vListenerPos )
{
	return PreUpdatePositionalSound( vListenerPos );
}

LTRESULT C3DSoundInstance::UpdateOutput( uint32 dwFrameTime )
{
	uint32 nVolume;
	LTVector vRelPos, vNormRelPos, vUp, vTemp, vListener;
	float fDistSqrd, fMinDistSqrd, fMaxDistSqrd, fScale, fFrontScale, fDist;
	uint16 nPan;
	LTVector vPosition, vVelocity;

	// Make sure we have a channel
	ASSERT( m_hSample || m_h3DSample || m_hStream );
	if( !m_hSample && !m_h3DSample && !m_hStream )
		return LT_ERROR;

	// Update the velocity every frame.  Use game time, since the object
	// is moving in gametime, not system time.  The dwFrameTime is in system time.
	m_vVelocity = m_vPosition - m_vLastPosition;
	float fGameFrameTime = g_pClientMgr->m_FrameTime;
	if( fGameFrameTime > 0.0f )
	{
		// to get to meters per second
		m_vVelocity /= ( float )fGameFrameTime;
	}
	m_vLastPosition = m_vPosition;


	if( GetClientILTSoundMgrImpl()->CommitChanges( ) || !( m_dwSoundInstanceFlags & SOUNDINSTANCEFLAG_PLAYING ))
	{
		// Calculate distance from listener to sound...
		vRelPos = GetClientILTSoundMgrImpl()->GetListenerPosition( ) - m_vPosition;
		vNormRelPos = vRelPos;
		vNormRelPos.Norm();

		// Handle panning 3d using 2d buffers.
		if( m_hSample || m_hStream )
		{
			//------------------------
			// Volume section...
			//------------------------

			fDistSqrd = vRelPos.MagSqr();
			fMinDistSqrd = m_fInnerRadius * m_fInnerRadius;
			fMaxDistSqrd = m_fOuterRadius * m_fOuterRadius;
			
			// Find volume scale based on distance...
			if( fDistSqrd <= fMinDistSqrd )
				fScale = 1.0;
			else if( fDistSqrd >= fMaxDistSqrd )
				fScale = 0.0;
			else
			{
				// These calculations fall off from 1.0 at mindist to 0 at maxdist and have a 1/x curve...
				fDist = ( float )sqrt( fDistSqrd );
				fScale = 1.0f - (( fDist - m_fInnerRadius ) / ( m_fOuterRadius - m_fInnerRadius ));
			}

			// Calculate how much the sound is in back of listener.  Sounds will be quieter when
			// they come from behind.  vRelPos's direction is relative to the sound, so if the listener
			// is pointing at sound, then dot product will be negative...
			fFrontScale = GetClientILTSoundMgrImpl()->GetListenerFront( ).Dot(vNormRelPos);

			// Sound is in front of listener, no volume loss...
			if( fFrontScale < 0.0f )
			{
				fFrontScale = 1.0f;
			}
			// Sound is behind listener, so scale it down by at most 25%...
			else
			{
				fFrontScale = 1.0f - 0.25f * fFrontScale;
			}

			nVolume = ( uint32 )( m_nVolume * fScale * fFrontScale );
			nVolume = LTMIN( nVolume, 100 );
			
			if( m_hSample )
			{
//				if( nVolume != ( uint32 )GetSoundSys()->GetSampleVolume( m_hSample ))
				if( nVolume != m_nCurRawVolume )
				{
					SetVolume( nVolume );
				}
			}
			else
			{
//				if( nVolume != ( uint32 )GetSoundSys()->GetStreamVolume( m_hStream ))
				if( nVolume != m_nCurRawVolume)
				{
					SetVolume( nVolume );
				}
			}

			//------------------------
			// Pan section...
			//------------------------

			// Get pan scale factor...
			fScale = -GetClientILTSoundMgrImpl()->GetListenerRight( ).Dot(vNormRelPos);
			// Always play a little in the other side...
//			fScale = LTCLAMP( fScale, -0.75f, 0.75f );
			nPan = ( uint16 )( fScale * 64.0f + 64.0f );

			// Set pan...
			if( m_hSample )
			{
//				if( nPan != GetSoundSys()->GetSamplePan( m_hSample ))
				if( nPan != m_nCurPan )
					SetPan( nPan );
//					GetSoundSys()->SetSamplePan( m_hSample, nPan );
			}
			else
			{
//				if( nPan != GetSoundSys()->GetStreamPan( m_hStream ))
				if( nPan != m_nCurPan )
					SetPan( nPan );
//					GetSoundSys()->SetStreamPan( m_hStream, nPan );
			}
		}
		// Handle true 3d
		else if( m_h3DSample )
		{
			// Update the position & velocity
			// Position is relative to listener not absolute
			vPosition = -vRelPos;
			fDistSqrd = vRelPos.MagSqr();
			fMinDistSqrd = m_fInnerRadius * m_fInnerRadius;
			fMaxDistSqrd = m_fOuterRadius * m_fOuterRadius;
			
			// Find volume scale based on distance...
			if( fDistSqrd <= fMinDistSqrd )
				fScale = 1.0;
			else if( fDistSqrd >= fMaxDistSqrd )
				fScale = 0.0;
			else
			{
				// These calculations fall off from 1.0 at mindist to 0 at maxdist and have a 1/x curve...
				fDist = ( float )sqrt( fDistSqrd );
				fScale = 1.0f - (( fDist - m_fInnerRadius ) / ( m_fOuterRadius - m_fInnerRadius ));
			}

			GetSoundSys()->Get3DPosition( m_h3DSample, &vTemp.x, &vTemp.y, &vTemp.z );
			if( vTemp.DistSqr( vPosition ) > 0.5f )
				GetSoundSys()->Set3DPosition( m_h3DSample, vPosition.x, vPosition.y, vPosition.z );
			float fDistFactor = GetClientILTSoundMgrImpl()->GetDistanceFactor();
			vVelocity = m_vVelocity * fDistFactor;
			GetSoundSys()->Get3DVelocity( m_h3DSample, &vTemp.x, &vTemp.y, &vTemp.z );
			if( vTemp.DistSqr( vVelocity ) > 0.5f )
			{
				GetSoundSys()->Set3DVelocityVector( m_h3DSample, vVelocity.x, 
					vVelocity.y, vVelocity.z);
			}

			// set the volume of the sound so we can avoid DirectSound's exponential volume curve
			nVolume = ( uint32 )( m_nVolume * fScale );
			nVolume = LTMIN( nVolume, 100 );
//			if( nVolume != ( uint32 )GetSoundSys()->Get3DSampleVolume( m_h3DSample ))
			if( nVolume != m_nCurRawVolume )
			{
				SetVolume( nVolume );
			}
		}
	}

	// Check if not played yet
	if( !( m_dwSoundInstanceFlags & SOUNDINSTANCEFLAG_PLAYING ))
	{
		if( StartRendering( ) != LT_OK )
			return LT_ERROR;
	}

	return LT_OK;
}

LTRESULT C3DSoundInstance::SetObstruction( LTFLOAT fLevel)
{
	GetSoundSys()->Set3DSampleObstruction(m_h3DSample, fLevel);
	return LT_OK;
}

LTRESULT C3DSoundInstance::GetObstruction( LTFLOAT &fLevel)
{
	fLevel = GetSoundSys()->Get3DSampleObstruction(m_h3DSample);
	return LT_OK;
}

LTRESULT C3DSoundInstance::SetOcclusion( LTFLOAT fLevel)
{
	GetSoundSys()->Set3DSampleOcclusion(m_h3DSample, fLevel);
	return LT_OK;
}

LTRESULT C3DSoundInstance::GetOcclusion( LTFLOAT &fLevel)
{
	fLevel = GetSoundSys()->Get3DSampleOcclusion( m_h3DSample );
	return LT_OK;
}


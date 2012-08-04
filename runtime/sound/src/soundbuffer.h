#ifndef __SOUNDBUFFER_H__
#define __SOUNDBUFFER_H__

#ifndef __CLIENT_FILEMGR_H__
#include "client_filemgr.h"
#endif

#ifndef __WAVE_H__
#include "wave.h"
#endif

class CSoundInstance;

inline float GetRandom(float min, float max)
{
	float randNum = (float)rand() / RAND_MAX;
	float num = min + (max - min) * randNum;
	return num;
}

class CSoundBuffer
{

public:

	CSoundBuffer();

	virtual ~CSoundBuffer();

	virtual LTRESULT	Init( FileIdentifier &fileIdent );

	virtual LTRESULT	InitFromCompressed( CSoundBuffer &compressedSoundBuffer );

	virtual void	Term();

	uint8 *			GetFileData( LTBOOL bDelegate = LTTRUE ) const
	{ return ( bDelegate && m_pDecompressedSoundBuffer ) ? m_pDecompressedSoundBuffer->m_pFileData : m_pFileData; }

	uint32			GetFileDataLen( LTBOOL bDelegate = LTTRUE ) const
	{ return ( bDelegate && m_pDecompressedSoundBuffer ) ? m_pDecompressedSoundBuffer->m_dwFileSize: m_dwFileSize; }

	uint8 *			GetSoundData( LTBOOL bDelegate = LTTRUE ) const
	{ return ( bDelegate && m_pDecompressedSoundBuffer ) ? m_pDecompressedSoundBuffer->m_pSoundData : m_pSoundData; }

	uint32			GetSoundDataLen( LTBOOL bDelegate = LTTRUE ) const
	{ return ( bDelegate && m_pDecompressedSoundBuffer ) ? m_pDecompressedSoundBuffer->m_WaveHeader.m_dwDataSize : m_WaveHeader.m_dwDataSize; }

	uint32			GetSoundBufferFlags( ) const
	{ return m_WaveHeader.m_lith.m_dwSoundBufferFlags; }

	void			SetTouched( LTBOOL bTouched ) 
	{ m_bTouched = bTouched; }

	LTBOOL			IsTouched( )
        { return m_bTouched; }

	uint32			GetDuration( ) const
        { return m_dwDuration; } // In milliseconds

	WAVEFORMATEX *	GetWaveFormat( LTBOOL bDelegate = LTTRUE );



	float			RandomPitchMod( );

	S32				GetSampleType( LTBOOL bDelegate = LTTRUE )
	{ return ( bDelegate && m_pDecompressedSoundBuffer ) ? m_pDecompressedSoundBuffer->m_nSampleType : m_nSampleType; }

	S32				GetPlaybackRate( LTBOOL bDelegate = LTTRUE )
	{ return GetWaveFormat( bDelegate )->nSamplesPerSec; }

//	===========================================================================
#ifdef USE_ABSTRACT_SOUND_INTERFACES
//	===========================================================================

	const LTSOUNDINFO *GetSampleInfo( LTBOOL bDelegate = LTTRUE )
	{ return ( bDelegate && m_pDecompressedSoundBuffer ) ? &m_pDecompressedSoundBuffer->m_SoundInfo : &m_SoundInfo; }

//	===========================================================================
#else
//	===========================================================================

	const AILSOUNDINFO *GetSampleInfo( LTBOOL bDelegate = LTTRUE )
	{ return ( bDelegate && m_pDecompressedSoundBuffer ) ? &m_pDecompressedSoundBuffer->m_SoundInfo : &m_SoundInfo; }

//	===========================================================================
#endif
//	===========================================================================

	LTBOOL			IsCompressed( )
	{ return ( m_WaveHeader.m_WaveFormat.wFormatTag == WAVE_FORMAT_IMA_ADPCM || m_WaveHeader.m_WaveFormat.wFormatTag == WAVE_FORMAT_MPEGLAYER3 ); }

	CSoundBuffer *	GetDecompressedSoundBuffer( )
	{ return m_pDecompressedSoundBuffer; }

	LTRESULT		DecompressData( )
	;

	const FileIdentifier *GetFileIdent( ) const
	{ return m_pFileIdent; }

	virtual LTRESULT	Unload( )
	;

	virtual LTRESULT	Reload( )
	;

	const LTLink *	GetLink( ) const
	{ return &m_Link; }

	const LTList *	GetInstanceList( ) const
	{ return &m_InstanceList; }

	LTRESULT		AddInstance( CSoundInstance &soundInstance )
	;

	LTRESULT		RemoveInstance( CSoundInstance &soundInstance )
	;

	LTBOOL			CanPlay( CSoundInstance &soundInstance )
	;

	uint32			GetLoopPoint( int i )
	{ return m_dwLoopPoints[i]; }

public:

	CWaveHeader	&	GetWaveHeader( LTBOOL bDelegate = LTTRUE ) 
	{
		if ( bDelegate && m_pDecompressedSoundBuffer ) {
			return m_pDecompressedSoundBuffer->m_WaveHeader;
		} else {
			return m_WaveHeader;
		}
	}
			
			
//		const 
//	{ return ( bDelegate && m_pDecompressedSoundBuffer ) ? m_pDecompressedSoundBuffer->m_WaveHeader : m_WaveHeader; };

protected:

	LTRESULT			CopySoundData16to8( uint8 *pDest, uint8 *pSource, uint32 nSize );

	virtual LTRESULT	LoadData( );

	LTRESULT			LoadDataFromDecompressed( )	;

	void				CalcSampleType( S32 &sampleType, WAVEFORMATEX &waveFormat );

protected:

	FileIdentifier *m_pFileIdent;

	CWaveHeader	m_WaveHeader;

	LTBOOL	m_bTouched;

	uint8 *	m_pFileData;
	uint32	m_dwFileSize;
	uint8 *	m_pSoundData;

	CSoundBuffer *	m_pDecompressedSoundBuffer;

	uint32	m_dwDuration;
	uint32	m_dwLoopPoints[2];

	S32		m_nSampleType;

//	===========================================================================
#ifdef USE_ABSTRACT_SOUND_INTERFACES
//	===========================================================================

	LTSOUNDINFO m_SoundInfo;

//	===========================================================================
#else
//	===========================================================================

	AILSOUNDINFO m_SoundInfo;

//	===========================================================================
#endif
//	===========================================================================

	LTList	m_InstanceList;

	LTLink	m_Link;

};

inline float CSoundBuffer::RandomPitchMod( )
{
	return GetRandom( -m_WaveHeader.m_lith.m_fPitchMod, m_WaveHeader.m_lith.m_fPitchMod );
}

inline WAVEFORMATEX *CSoundBuffer::GetWaveFormat( LTBOOL bDelegate )
{
    return ( bDelegate && m_pDecompressedSoundBuffer ) ?
        &m_pDecompressedSoundBuffer->m_WaveHeader.m_WaveFormat : &m_WaveHeader.m_WaveFormat;
}


#endif // __SOUNDBUFFER_H__



//------------------------------------------------------------------
//
//	FILE	  : SOUNDDATA.CPP
//
//	PURPOSE	  : Defines CSoundData and CSoundTrack class
//
//	CREATED	  : 10/23/97
//
//	COPYRIGHT : Microsoft 1997 All Rights Reserved
//
//------------------------------------------------------------------

// Includes....

#include "bdefs.h"
#include "sounddata.h"
#include "iltstream.h"
#include "wave.h"

//------------------------------------------------------------------
//
//	FUNCTION : CSoundData::Init()
//
//  PURPOSE	 : Initialises the sound object
//
//------------------------------------------------------------------

LTBOOL CSoundData::Init( UsedFile *pFile, ILTStream *pStream, uint32 dwFileSize )
{
	CWaveHeader waveHeader;

	if( !pFile || !pStream || dwFileSize == 0 )
		return LTFALSE;

	Term( );

	m_pFile = pFile;

	if( !GetWaveInfo( *pStream, waveHeader ))
	{
		return LTFALSE;
	}

	m_fDuration = (( float )waveHeader.m_dwDataSize / ( float )waveHeader.m_WaveFormat.nAvgBytesPerSec );

	// Inflate it a little so we don't tell the client to stop a sound premuturely due to lag...
	m_fDuration += 0.1f;

	// Mark this file as needed
	m_bTouched = LTFALSE;

	m_dwFlags = 0;

	return LTTRUE;
}

//------------------------------------------------------------------
//
//	FUNCTION : CSoundData::Term()
//
//  PURPOSE	 : Terminates the sound
//
//------------------------------------------------------------------

void CSoundData::Term()
{
	m_fDuration = 0.0f;
	m_dwFlags = 0;
}


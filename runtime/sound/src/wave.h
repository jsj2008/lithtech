#ifndef __WAVE_H__
#define __WAVE_H__


#ifdef __LINUX

#ifndef WORD
typedef unsigned short int WORD;
#endif

#ifndef DWORD
typedef unsigned long int DWORD;
#endif


#ifndef WAVE_FORMAT_PCM

/* general waveform format structure (information common to all formats) */
typedef struct waveformat_tag {
    WORD    wFormatTag;        /* format type */
    WORD    nChannels;         /* number of channels (i.e. mono, stereo...) */
    DWORD   nSamplesPerSec;    /* sample rate */
    DWORD   nAvgBytesPerSec;   /* for buffer estimation */
    WORD    nBlockAlign;       /* block size of data */
} WAVEFORMAT;

typedef WAVEFORMAT       *PWAVEFORMAT;

/* flags for wFormatTag field of WAVEFORMAT */
#define WAVE_FORMAT_PCM     1

/* specific waveform format structure for PCM data */
typedef struct pcmwaveformat_tag {
    WAVEFORMAT  wf;
    WORD        wBitsPerSample;
} PCMWAVEFORMAT;

typedef PCMWAVEFORMAT       *PPCMWAVEFORMAT;

#endif /* WAVE_FORMAT_PCM */

/* general extended waveform format structure
   Use this for all NON PCM formats
   (information common to all formats)
*/
#ifndef _WAVEFORMATEX_
#define _WAVEFORMATEX_
typedef struct tWAVEFORMATEX
{
    WORD    wFormatTag;        /* format type */
    WORD    nChannels;         /* number of channels (i.e. mono, stereo...) */
    DWORD   nSamplesPerSec;    /* sample rate */
    DWORD   nAvgBytesPerSec;   /* for buffer estimation */
    WORD    nBlockAlign;       /* block size of data */
    WORD    wBitsPerSample;    /* Number of bits per sample of mono data */
    WORD    cbSize;            /* The count in bytes of the size of
                                    extra information (after cbSize) */

} WAVEFORMATEX;

typedef WAVEFORMATEX       *PWAVEFORMATEX;

#endif /* _WAVEFORMATEX_ */


#ifndef WAVE_FORMAT_PCM
#define WAVE_FORMAT_PCM			1
#endif	// WAVE_FORMAT_PCM

#ifndef WAVE_MAPPER
#define	WAVE_MAPPER				((uint32)-1)
#endif	// WAVE_MAPPER

#ifndef WAVE_FORMAT_IMA_ADPCM
#define WAVE_FORMAT_IMA_ADPCM	0x0011
#endif	// WAVE_FORMAT_IMA_ADPCM

#ifndef MAKEFOURCC
#define MAKEFOURCC(ch0, ch1, ch2, ch3)								\
		((uint32)(uint8)(ch0) | ((uint32)(uint8)(ch1) << 8) |		\
		((uint32)(uint8)(ch2) << 16) | ((uint32)(uint8)(ch3) << 24))
#endif

#ifndef mmioFOURCC	
#define mmioFOURCC(w,x,y,z) MAKEFOURCC(w,x,y,z)
#endif

#endif


#ifndef __ILTSOUND_H__
#include "iltsound.h"
#endif


//	===========================================================================
#ifdef USE_ABSTRACT_SOUND_INTERFACES
//	===========================================================================

#ifndef WAVE_FORMAT_PCM
#define WAVE_FORMAT_PCM			1
#endif	// WAVE_FORMAT_PCM

#ifndef WAVE_MAPPER
#define	WAVE_MAPPER				((uint32)-1)
#endif	// WAVE_MAPPER

#ifndef WAVE_FORMAT_IMA_ADPCM
#define WAVE_FORMAT_IMA_ADPCM	0x0011
#endif	// WAVE_FORMAT_IMA_ADPCM

#ifndef MAKEFOURCC
#define MAKEFOURCC(ch0, ch1, ch2, ch3)								\
		((uint32)(uint8)(ch0) | ((uint32)(uint8)(ch1) << 8) |		\
		((uint32)(uint8)(ch2) << 16) | ((uint32)(uint8)(ch3) << 24))
#endif

#ifndef mmioFOURCC	
#define mmioFOURCC(w,x,y,z) MAKEFOURCC(w,x,y,z)
#endif

//	===========================================================================
#else
//	===========================================================================

#ifndef __LINUX

#ifndef __LTDIRECTSOUND_H__
#include "ltdirectsound.h"
#define __LTDIRECTSOUND_H__
#endif

#endif

//	===========================================================================
#endif
//	===========================================================================



class ILTStream;
class SSBufStream;

// Defines used with CLithSoundFormat::m_dwSoundBufferFlags
// Compressed sound files default to decompressing the file as it plays.  These files cannot
// be used for true 3d sounds.
#define	SOUNDBUFFERFLAG_STREAM				(1<<0)		// Buffer used for streaming sound
#define SOUNDBUFFERFLAG_DECOMPRESSONLOAD	(1<<1)		// Decompress the file when loaded.
#define SOUNDBUFFERFLAG_DECOMPRESSATSTART	(1<<2)		// Decompress the whole when first played.

#define WAVE_FORMAT_MPEGLAYER3	85

#define FOURCC uint32

struct WAVEHEADER_cuepoint
{
	uint32		m_dwName;
	uint32		m_dwPosition;
	FOURCC		m_fccChunk;
	uint32		m_dwChunkStart;
	uint32		m_dwBlockStart;
	uint32		m_dwSampleOffset;
};

struct WAVEHEADER_cue
{
	uint32					m_dwNumCuePoints;
	WAVEHEADER_cuepoint *	m_pCuePoints;
};

struct WAVEHEADER_ltxt
{
	uint32		m_dwName;
	uint32		m_dwSampleLength;
	uint32		m_dwPurpose;
	uint16		m_wCountry;
	uint16		m_wLanguage;
	uint16		m_wDialect;
	uint16		m_wCodePage;
};

struct WAVEHEADER_lith
{
	uint32 m_dwSoundBufferFlags;
	float m_fPitchMod;
};

struct CCuePoint
{
	LTBOOL		m_bValid;
	uint32		m_dwName;
	uint32		m_dwPosition;
	uint32		m_dwLength;
};

struct CWaveHeader
{
	WAVEFORMATEX				m_WaveFormat;
	uint32						m_dwDataPos;
	uint32						m_dwDataSize;
	uint32						m_dwSamples;
	WAVEHEADER_lith				m_lith;
	CCuePoint					m_CuePoint;
};


LTBOOL GetWaveInfo( ILTStream &inStream, CWaveHeader &waveHeader );
LTBOOL WriteWave( SSBufStream &inStream, ILTStream &outStream, CWaveHeader &waveHeader );


#endif // __WAVE_H__

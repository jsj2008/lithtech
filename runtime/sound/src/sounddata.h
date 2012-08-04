//------------------------------------------------------------------
//
//	FILE	  : SoundData.h
//
//	PURPOSE	  : Defines the CSoundData and CSoundTrack classes 
//				so the server can keep track of certain sounds
//
//	CREATED	  : 10/23/97
//
//	COPYRIGHT : MONOLITH Inc 1997 All Rights Reserved
//
//------------------------------------------------------------------

#ifndef __SOUNDDATA_H__
#define __SOUNDDATA_H__


struct UsedFile;


//------------------------------------------------------------------------
//
//	CLASS:		CSoundData
//
//	PURPOSE:	Reads the wave file and holds the duration of the file.
//
//------------------------------------------------------------------------
class CSoundData
{
	public:

		CSoundData( );
		~CSoundData( );

		LTBOOL						Init( UsedFile *pFile, ILTStream *pStream, uint32 szFile );
		void						Term( );

		float						GetDuration( );
		UsedFile *					GetFile( );

		// Any of the SOUNDBUFFERFLAG_X
		uint32						GetFlags( ) { return m_dwFlags; }
		void						SetFlags( uint32 dwFlags ) { m_dwFlags = dwFlags; }

		void						SetTouched( LTBOOL bTouched ) { m_bTouched = bTouched; }
		LTBOOL						IsTouched( ) { return m_bTouched; }

		LTLink						m_Link;
		
	private:

		UsedFile					*m_pFile;
		float						m_fDuration;
		uint32						m_dwFlags;
		LTBOOL						m_bTouched;

};


// Inlines...
	
inline CSoundData::CSoundData( )
{
	m_fDuration = 0.0f;
	m_pFile = LTNULL;
	m_dwFlags = 0;
	m_bTouched = LTFALSE;
}
  	

inline CSoundData::~CSoundData( )
{
	Term( );
}

inline float CSoundData::GetDuration( )
{
	return m_fDuration;
}

inline UsedFile *CSoundData::GetFile( )
{
	return m_pFile;
}



#endif  // __SOUNDDATA_H__


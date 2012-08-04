//------------------------------------------------------------------
//
//	FILE	  : FileIO.cpp
//
//	PURPOSE	  : Implementation for the RiotFileIO class.
//
//	CREATED	  : 1st May 1996
//
//	COPYRIGHT : Microsoft 1996 All Rights Reserved
//
//------------------------------------------------------------------

#include "fileio.h"



// ----------------------------------------------------------------------- //
//
//      ROUTINE:        Constructor
//
//      PURPOSE:        
//
// ----------------------------------------------------------------------- //

CMoFileIO::CMoFileIO()
{
	m_pFile = NULL;
}
			


// ----------------------------------------------------------------------- //
//
//      ROUTINE:        Destructor
//
//      PURPOSE:        
//
// ----------------------------------------------------------------------- //

CMoFileIO::~CMoFileIO()
{
	Close();
}



// ----------------------------------------------------------------------- //
//
//      ROUTINE:        CMoFileIO::Open
//
//      PURPOSE:        Opens a file .. use the same access string as fopen().
//
// ----------------------------------------------------------------------- //

LTBOOL CMoFileIO::Open( const char *pFilename, const char *pAccess )
{
	// Make sure to close if we're already open.
	if(m_pFile)
	{
		Close();
	}

	if( !(m_pFile = fopen(pFilename, pAccess)) )
		return FALSE;

	fseek( m_pFile, 0, SEEK_END );
	m_FileLen = ftell( m_pFile );
	fseek( m_pFile, 0, SEEK_SET );

	m_FileMin = 0;
	m_FileMax = m_FileLen;

	return TRUE;
}


// ----------------------------------------------------------------------- //
//
//      ROUTINE:        CMoFileIO::Close
//
//      PURPOSE:        Closes the open file.
//
// ----------------------------------------------------------------------- //

void CMoFileIO::Close()
{
	if(m_pFile)
	{
		fclose( m_pFile );
		m_pFile = NULL;	
	}
}


// ----------------------------------------------------------------------- //
//      ROUTINE:        CMoFileIO::IsOpen
//      PURPOSE:        Just tells if the file is open.
// ----------------------------------------------------------------------- //
LTBOOL CMoFileIO::IsOpen()
{
	return !!m_pFile;
}
		

// ----------------------------------------------------------------------- //
//      ROUTINE:        CMoFileIO::SetBoundaries
//      PURPOSE:        Sets boundaries in the file, so you can have a
//                      'virtual file' inside another file.
// ----------------------------------------------------------------------- //

void CMoFileIO::SetBoundaries( uint32 min, uint32 max )
{
	m_FileMin = 0;
	m_FileMax = m_FileLen;

	ASSERT( min < max );
	
	if( min > m_FileLen )
		min = 0;

	if( max > m_FileLen )
		max = m_FileLen;

	SeekTo( min );
	
	m_FileMin = min;
	m_FileMax = max;
	m_FileLen = m_FileMax - m_FileMin;
}




LTBOOL CMoFileIO::Write(const void *pBlock, uint32 blockSize )
{
	ASSERT( m_pFile );
	
	if( fwrite(pBlock, 1, blockSize, m_pFile) == blockSize )
	{
		return TRUE;
	}
	else
	{
		MaybeThrowIOException( MoWriteError );
		return FALSE;
	}
}


LTBOOL CMoFileIO::Read( void *pBlock, uint32 blockSize )
{
	uint32		nBytesRead;
	
	ASSERT( m_pFile );
	
	nBytesRead = (uint32)fread( pBlock, 1, blockSize, m_pFile );
	if( nBytesRead == blockSize )
	{
		return TRUE;
	}
	else
	{
		MaybeThrowIOException( MoReadError );
		return FALSE;
	}
}


// ----------------------------------------------------------------------- //
//
//      ROUTINE:        CMoFileIO::GetCurPos
//
//      PURPOSE:        Returns the current offset in the file.
//
// ----------------------------------------------------------------------- //

uint32 CMoFileIO::GetCurPos()
{
	ASSERT( m_pFile );
	return ftell( m_pFile ) - m_FileMin;
}
	
		
// ----------------------------------------------------------------------- //
//
//      ROUTINE:        CMoFileIO::GetLen
//
//      PURPOSE:        Returns the length of the file.
//
// ----------------------------------------------------------------------- //

uint32 CMoFileIO::GetLen()
{
	return m_FileMax - m_FileMin;
}



// ----------------------------------------------------------------------- //
//
//      ROUTINE:        CMoFileIO::SeekTo
//
//      PURPOSE:        Seeks to the given position in the file.
//
// ----------------------------------------------------------------------- //

LTBOOL CMoFileIO::SeekTo( uint32 pos )
{
	ASSERT( m_pFile );
	
	if( fseek(m_pFile, pos+m_FileMin, SEEK_SET) == 0 )
	{
		return TRUE;
	}
	else
	{
		MaybeThrowIOException( MoSeekError );
		return FALSE;
	}
}






//------------------------------------------------------------------
//
//	FILE	  : MemoryIO.cpp
//
//	PURPOSE	  : Implements the CMemoryIO class.
//
//	CREATED	  : July 25 1996
//
//	COPYRIGHT : Microsoft 1996 All Rights Reserved
//
//------------------------------------------------------------------

// Includes....
#include "memoryio.h"



CMemoryIO::CMemoryIO()
{
	m_bRanOutOfMemory = FALSE;
	m_Pos = 0;
	m_Data.SetCacheSize( 200 );
}



LTBOOL CMemoryIO::Write(const void *pBlock, uint32 blockSize )
{
	uint32		i, initialSize;

	
	initialSize = m_Data.GetSize();
	if( (m_Pos+blockSize) > initialSize )
	{
		for( i=0; i < ((m_Pos+blockSize)-initialSize); i++ )
		{
			if( !m_Data.Append(0) )
			{
				m_bRanOutOfMemory = TRUE;
				MaybeThrowIOException( MoWriteError );
				return FALSE;
			}
		}
	}

	memcpy( &(m_Data.GetArray()[m_Pos]), pBlock, blockSize );
	m_Pos += blockSize;
	
	return TRUE;
}


LTBOOL CMemoryIO::Read( void *pBlock, uint32 blockSize )
{
	if( (m_Pos+blockSize) > m_Data.GetSize() )
	{
		MaybeThrowIOException( MoReadError );
		return FALSE;
	}

	memcpy( pBlock, &(m_Data.GetArray()[m_Pos]), blockSize );
	m_Pos += blockSize;
	
	return TRUE;		
}


uint32 CMemoryIO::GetCurPos()
{
	return m_Pos;
}

uint32 CMemoryIO::GetLen()
{
	return m_Data.GetSize();
}

LTBOOL CMemoryIO::SeekTo( uint32 pos )
{
	if( pos > m_Data.GetSize() )
	{
		MaybeThrowIOException( MoSeekError );
		return FALSE;
	}

	m_Pos = pos;
	return TRUE;
}


void CMemoryIO::Clear()
{
	m_Data.Term();
	m_Pos=0;
	m_bRanOutOfMemory = FALSE;
}

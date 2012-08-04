//------------------------------------------------------------------
//
//	FILE	  : AbstractIO.cpp
//
//	PURPOSE	  : Implements the CAbstractFileIO class.
//
//	CREATED	  : November 12 1996
//
//	COPYRIGHT : Microsoft 1996 All Rights Reserved
//
//------------------------------------------------------------------

// Includes....
#include "abstractio.h"


CAbstractIO::CAbstractIO()
{
	m_bExceptionsEnabled = TRUE;
}


CAbstractIO::~CAbstractIO()
{
}


void CAbstractIO::EnableExceptions( LTBOOL bEnable )
{
	m_bExceptionsEnabled = bEnable;
}


LTBOOL CAbstractIO::IsExceptionsEnabled()
{
	return m_bExceptionsEnabled;
}


LTBOOL CAbstractIO::WriteString(const char *pStr )
{
	uint16	len = (uint16)strlen(pStr);
	
	if( !Write(&len, sizeof(len)) )
		return FALSE;

	return Write( pStr, len );
}


LTBOOL CAbstractIO::ReadString( char *pStr, uint32 maxLen )
{
	uint16		len;

	if( !Read(&len, sizeof(len)) )
		return FALSE;

	if( (uint16)(len+1) > maxLen )
		return FALSE;

	if( !Read(pStr, len) )
		return FALSE;
	
	pStr[len] = 0;
	return TRUE;
}

LTBOOL CAbstractIO::ReadTextString(char *pStr, uint32 maxLen)
{
	if (maxLen > 4096) maxLen = 4096;

	memset(pStr, 0, maxLen);

	uint32 i = 0;

	while (i < maxLen)
	{		
		Read(&pStr[i], 1);

		if (pStr[i] == 0x0A)
		{
			if (pStr[i - 1] == 0x0D)
			{
				pStr[i - 1] = 0;
				
				return TRUE;
			}
		}
			
		i ++;
	}

	return TRUE;
}


void CAbstractIO::MaybeThrowIOException( LithIOExceptionType code )
{
#ifndef NO_EXCEPTIONS
	if( m_bExceptionsEnabled )
		throw CLithIOException( code );
#endif
}






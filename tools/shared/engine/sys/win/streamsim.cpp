
#include "bdefs.h"
#include "streamsim.h"



// ----------------------------------------------------------------------------- //
// Structures.
// ----------------------------------------------------------------------------- //

class SSFile : public CGenLTStream
{
public:
			
			SSFile() 
			{
				m_bError = LTFALSE; 
				m_pFile = LTNULL;
			}
			
			~SSFile()
			{
				if(m_pFile)
					fclose(m_pFile);
			}
	
	void	Release()
	{
		delete this;
	}

	LTRESULT	Read(void *pData, uint32 size)
	{
		size_t ret;

		if(!m_pFile)
			return LT_ERROR;

		if(size == 0)
		{
			return LT_OK;
		}
		else
		{
			ret = fread(pData, 1, size, m_pFile);
			if(ret == size)
			{
				return LT_OK;
			}
			else
			{
				memset(pData, 0, size);
				m_bError = TRUE;
				return LT_ERROR;
			}
		}
	}

	LTRESULT Write(const void *pData, uint32 size)
	{
		size_t ret;

		if(!m_pFile)
			return LT_ERROR;

		if(size == 0)
		{
			return LT_OK;
		}
		else
		{
			ret = fwrite(pData, 1, size, m_pFile);
			if(ret == size)
			{
				return LT_OK;
			}
			else
			{
				m_bError = TRUE;
				return LT_ERROR;
			}
		}
	}

	LTRESULT	ErrorStatus()
	{
		return m_bError ? LT_ERROR : LT_OK;
	}
	
	LTRESULT	SeekTo(uint32 offset)
	{
		if(!m_pFile)
			return LT_ERROR;

		if(fseek(m_pFile, offset, SEEK_SET) == 0)
		{
			return LT_OK;
		}
		else
		{
			return LT_ERROR;
		}
	}
	
	LTRESULT	GetPos(uint32 *offset)
	{
		long theOffset;

		if(!m_pFile)
		{
			*offset = 0;
			return LT_ERROR;
		}

		theOffset = ftell(m_pFile);
		*offset = (uint32)theOffset;
		return (theOffset == -1) ? LT_ERROR : LT_OK;
	}
	
	LTRESULT	GetLen(uint32 *len)
	{
		long curPos;

		if(!m_pFile)
		{
			*len = 0;
			return LT_ERROR;
		}

		curPos = ftell(m_pFile);
		fseek(m_pFile, 0, SEEK_END);
		*len = (uint32)ftell(m_pFile);
		fseek(m_pFile, curPos, SEEK_SET);
		return LT_OK;
	}

	FILE	*m_pFile;
	LTBOOL	m_bError;
};


class SSMemStream : public CGenLTStream
{
// Overrides.
public:

				SSMemStream(uint32 cacheSize)
				{
					m_Buffer.SetCacheSize(cacheSize);
					m_Pos = 0;
					m_bError = LTFALSE;
				}

	LTRESULT	Read(void *pData, uint32 dataLen)
	{
		// Prevent an unneeded assertion...
		if(dataLen == 0)
			return LT_OK;

		if((m_Pos+dataLen) > m_Buffer.GetSize())
		{
			m_Pos = m_Buffer.GetSize();
			m_bError = TRUE;
			memset(pData, 0, dataLen);
			return LT_ERROR;
		}

		memcpy(pData, &m_Buffer[m_Pos], dataLen);
		m_Pos += dataLen;
		return LT_OK;
	}

	LTRESULT	Write(const void *pData, uint32 dataLen)
	{
		LTRESULT dResult;

		// Prevent an unneeded assertion...
		if(dataLen == 0)
			return LT_OK;

		dResult = Size(m_Pos+dataLen);
		if(dResult != LT_OK)
			return dResult;

		memcpy(&m_Buffer[m_Pos], pData, dataLen);
		m_Pos += dataLen;
		return LT_OK;
	}

	LTRESULT	ErrorStatus()
	{
		return m_bError ? LT_ERROR : LT_OK;
	}
	
	LTRESULT	SeekTo(uint32 offset)
	{
		if(offset > m_Buffer.GetSize())
			return LT_ERROR;

		m_Pos = offset;
		return LT_OK;
	}
	
	LTRESULT	GetPos(uint32 *offset)
	{
		*offset = m_Pos;
		return LT_OK;
	}
	
	LTRESULT	GetLen(uint32 *len)
	{
		*len = m_Buffer.GetSize();
		return LT_OK;
	}

	void			Release()
	{
		delete this;
	}

// Helpers.
public:

	LTRESULT			Size(uint32 size)
	{
		if(m_Buffer.GetSize() < size)
		{
			return m_Buffer.Fast_NiceSetSize(size) ? LT_OK : LT_ERROR;
		}
		else
		{
			return LT_OK;
		}
	}

	CMoArray<uint8>	m_Buffer;
	uint32			m_Pos;
	LTBOOL			m_bError;
};


class SSWinFileHandle : public CGenLTStream
{
public:
			
			SSWinFileHandle() 
			{
				m_bError = LTFALSE; 
				m_hFile = INVALID_HANDLE_VALUE;
			}
			
			~SSWinFileHandle()
			{
				if( m_hFile != INVALID_HANDLE_VALUE )
					CloseHandle( m_hFile );
			}
	
	void	Release()
	{
		delete this;
	}

	LTRESULT	Read(void *pData, uint32 size)
	{
		unsigned long dwBytesRead;

		if( m_hFile == INVALID_HANDLE_VALUE )
			return LT_ERROR;

		if(size == 0)
		{
			return LT_OK;
		}
		else
		{
			if( ReadFile( m_hFile, pData, size, &dwBytesRead, LTNULL ) && dwBytesRead == size )
			{
				return LT_OK;
			}
			else
			{
				memset(pData, 0, size);
				m_bError = TRUE;
				return LT_ERROR;
			}
		}
	}

	LTRESULT Write( const void *pData, uint32 size )
	{
		unsigned long dwBytesWritten;

		if( m_hFile == INVALID_HANDLE_VALUE )
			return LT_ERROR;

		if(size == 0)
		{
			return LT_OK;
		}
		else
		{
			if( WriteFile( m_hFile, pData, size, &dwBytesWritten, LTNULL ) && dwBytesWritten == size )
			{
				return LT_OK;
			}
			else
			{
				m_bError = TRUE;
				return LT_ERROR;
			}
		}
	}

	LTRESULT	ErrorStatus()
	{
		return m_bError ? LT_ERROR : LT_OK;
	}
	
	LTRESULT	SeekTo(uint32 offset)
	{
		if( m_hFile == INVALID_HANDLE_VALUE )
			return LT_ERROR;

		if( SetFilePointer( m_hFile, offset, 0, FILE_BEGIN ) != ( uint32 )-1 )
		{
			return LT_OK;
		}
		else
		{
			return LT_ERROR;
		}
	}
	
	LTRESULT	GetPos(uint32 *offset)
	{
		uint32 dwOffset;

		if( m_hFile == INVALID_HANDLE_VALUE )
		{
			*offset = 0;
			return LT_ERROR;
		}

		dwOffset = SetFilePointer( m_hFile, 0, 0, FILE_CURRENT );
		if( dwOffset == ( uint32 )-1 )
		{
			*offset = 0;
			return LT_ERROR;
		}

		*offset = dwOffset;
		return LT_OK;
	}
	
	LTRESULT	GetLen(uint32 *len)
	{
		uint32 dwSize;

		if( m_hFile == INVALID_HANDLE_VALUE )
		{
			*len = 0;
			return LT_ERROR;
		}

		dwSize = GetFileSize( m_hFile, LTNULL );
		if( dwSize == 0xFFFFFFFF )
		{
			*len = 0;
			return LT_ERROR;
		}

		*len = dwSize;
		return LT_OK;
	}

	HANDLE	m_hFile;
	LTBOOL	m_bError;
};


// ----------------------------------------------------------------------------- //
// Main interface functions.
// ----------------------------------------------------------------------------- //

ILTStream* streamsim_Open(const char *pFilename, const char *pAccess)
{
	FILE *fp;
	SSFile *pFile;

	fp = fopen(pFilename, pAccess);
	if(!fp)
		return 0;

	pFile = new SSFile;
	if(pFile)
		pFile->m_pFile = fp;
	
	return pFile;
}


ILTStream* streamsim_OpenMemStream(uint32 cacheSize)
{
	return new SSMemStream(cacheSize);
}


ILTStream* streamsim_MemStreamFromFile(char *pFilename)
{
	FILE *fp;
	SSMemStream *pRet;
	long len, amtRead;

	pRet = LTNULL;
	fp = fopen(pFilename, "rb");
	if(fp)
	{
		fseek(fp, 0, SEEK_END);
		len = ftell(fp);
		fseek(fp, 0, SEEK_SET);
	
		pRet = new SSMemStream(256);
		if(pRet->m_Buffer.SetSize(len))
		{
			amtRead = fread(pRet->m_Buffer.GetArray(), 1, len, fp);
			if(amtRead != len)
			{
				delete pRet;
				pRet = LTNULL;
			}
		}
		else
		{
			delete pRet;
			pRet = LTNULL;
		}
		
		fclose(fp);
	}

	return pRet;
}


ILTStream* streamsim_AbstractIOWrapper(CAbstractIO *pIO)
{
	return new SSAbstractIOWrapper(pIO);
}

ILTStream* streamsim_MemStreamFromBuffer( uint8 *pData, uint32 dwDataSize )
{
	SSBufStream *pRet;

	pRet = new SSBufStream( pData, dwDataSize );

	return pRet;
}


ILTStream* streamsim_OpenWinFileHandle( const char *pszFilename, uint32 dwDesiredAccess )
{
	HANDLE hFile;
	SSWinFileHandle *pFile;

	hFile = CreateFile( pszFilename, dwDesiredAccess, FILE_SHARE_READ, LTNULL, OPEN_ALWAYS, 0, LTNULL );
	if( hFile == INVALID_HANDLE_VALUE )
		return LTNULL;

	pFile = new SSWinFileHandle ;
	if(pFile)
		pFile->m_hFile = hFile;
	
	return pFile;
}


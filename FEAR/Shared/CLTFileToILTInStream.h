//-----------------------------------------------------------------
// CLTFileRead to ILTInStream adapter class
//-----------------------------------------------------------------

#ifndef __CLTFILETOILTINSTREAM_H__
#define __CLTFILETOILTINSTREAM_H__

#include "ltfileread.h"

class CLTFileToILTInStream :
	public ILTInStream
{
public:

	CLTFileToILTInStream() : m_bError(true)		{}
	~CLTFileToILTInStream()						{}

	bool Open(const char* pszAbsoluteFile)
	{
		//open up our file, and reset our error state
		m_bError = !m_InFile.Open(pszAbsoluteFile);
		return !m_bError;
	}	

	virtual void Release()
	{
		delete this;
	}

	virtual LTRESULT Read(void *pData, uint32 size)
	{
		//if we haven't hit an error, 
		if(m_bError || !m_InFile.Read(pData, size))
		{
			m_bError = true;
			memset(pData, 0, size);
		}

		return (m_bError) ? LT_ERROR : LT_OK;
	}

	virtual bool HasErrorOccurred()
	{
		return m_bError;
	}

	virtual bool CanSeek()
	{
		return true;
	}

	virtual LTRESULT SeekTo(uint64 offset)
	{
		if(m_bError || !m_InFile.Seek(offset))
			m_bError = true;

		return (m_bError) ? LT_ERROR : LT_OK;
	}

	virtual uint64 GetPos()
	{
		uint64 nPos = 0;
		m_InFile.GetPos(nPos);
		return nPos;
	}

	virtual uint64 GetLen()
	{
		uint64 nFileSize = 0;
		m_InFile.GetFileSize(nFileSize);
		return nFileSize;
	}

	virtual LTRESULT ReadString(char *pStr, uint32 maxBytes)
	{
		uint16 len;
		*this >> len;

#if defined(PLATFORM_XENON)
		// XENON: Swap data at runtime
		LittleEndianToNative(&len);
#endif // PLATFORM_XENON

		if(maxBytes == 0)
		{
			char dummy;
			for(uint32 i=0; i < len; i++)
				*this >> dummy;

			return LT_OK;
		}
		else
		{
			uint32 maxChars = maxBytes - 1;

			LTRESULT result;
			if(len > maxChars)
			{
				result = Read(pStr, maxChars);
				pStr[maxChars] = 0;
				len -= (uint16)maxChars;

				char dummy;
				while(len--)
				{
					*this >> dummy;
				}
			}
			else
			{
				result = Read(pStr, len);
				pStr[len] = 0;
			}

			return result;
		}
	}

private:

	CLTFileRead	m_InFile;
	bool		m_bError;
};

#endif //__CLTFILETOILTINSTREAM_H__

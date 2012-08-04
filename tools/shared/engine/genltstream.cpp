
#include "bdefs.h"
#include "genltstream.h"



#define WRITESTREAM_BLOCKSIZE	512



LTRESULT CGenLTStream::ReadString(char *pStr, uint32 maxBytes)
{
	LTRESULT LTRESULT;
	uint32 i, maxChars;
	uint16 len;
	char dummy;

	*this >> len;
	if(len == 0)
	{
		pStr[0] = 0;
		return LT_OK;
	}

	if(maxBytes == 0)
	{
		for(i=0; i < len; i++)
			*this >> dummy;
	
		return LT_OK;
	}
	else
	{
		maxChars = maxBytes - 1;

		if(len > maxChars)
		{
			LTRESULT = Read(pStr, maxChars);
			pStr[maxChars] = 0;
			len -= (uint16)maxChars;
			while(len--)
			{
				*this >> dummy;
			}
		}
		else
		{
			LTRESULT = Read(pStr, len);
			pStr[len] = 0;
		}

		return LTRESULT;
	}
}
	

LTRESULT CGenLTStream::WriteString(const char *pStr)
{
	uint16 len;

	len = (uint16)strlen(pStr);
	*this << len;
	return Write(pStr, len);
}


LTRESULT CGenLTStream::WriteStream(ILTStream &dsSource, uint32 dwMin, uint32 dwMax)
{
	uint32 blockSize, nLength;
	char tempBlock[WRITESTREAM_BLOCKSIZE];

	if(dwMin == WRITESTREAM_DEFAULT)
		dwMin = 0;

	if(dwMax == WRITESTREAM_DEFAULT)
		dwMax = dsSource.GetLen();

	if(dwMax < dwMin)
		return LT_ERROR;

	nLength = dwMax - dwMin;
	while(nLength > 0)
	{
		blockSize = (nLength >= WRITESTREAM_BLOCKSIZE) ? WRITESTREAM_BLOCKSIZE : nLength;
		
		if(dsSource.Read(tempBlock, blockSize) != LT_OK)
			return LT_ERROR;

		if(Write(tempBlock, blockSize) != LT_OK)
			return LT_ERROR;

		nLength -= blockSize;
	}

	if(dsSource.ErrorStatus() != LT_OK || ErrorStatus() != LT_OK)
		return LT_ERROR;

	return LT_OK;
}



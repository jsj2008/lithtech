// ----------------------------------------------------------------------- //
//
// MODULE  : AIStreamSim.cpp
//
// PURPOSE : Defines the AIStreamSim classes.
//
// CREATED : 12/13/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "AIStreamSim.h"

#if !defined( PLATFORM_XENON )

/*
 *	CGenLTOutStream
 */

LTRESULT CGenLTOutStream::WriteString(const char *pStr)
{
	if(pStr)
	{
		uint16 len = (uint16)LTStrLen(pStr);
		*this << len;
		return Write(pStr, len);
	}
	else
	{
		*this << (uint16)0;
		return LT_OK;
	}
}

#endif

/*
 *	COutMemStream
 */

class COutMemStream :
	public CGenLTOutStream
{
public:

	COutMemStream(std::vector<uint8>& OutVec) :
		m_pBuffer(&OutVec),
		m_nWritePos((uint32)OutVec.size())
	{
	}

	virtual void Release()
	{
		delete this;
	}

	LTRESULT Write(const void *pData, uint32 size)
	{
		const uint8* pFirst = (uint8*)pData;
		const uint8* pLast  = pFirst + size;

		m_pBuffer->insert(m_pBuffer->begin() + m_nWritePos, pFirst, pLast);
		m_nWritePos += size;
		return LT_OK;
	}	

	virtual bool HasErrorOccurred()
	{
		return false;
	}

	virtual bool CanSeek()
	{
		return true;
	}

	virtual LTRESULT SeekTo(uint64 offset)
	{
		m_nWritePos = (uint32)offset;
		return LT_OK;
	}

	virtual uint64 GetPos()
	{
		return m_nWritePos;
	}

private:

	uint32				m_nWritePos;
	std::vector<uint8>*	m_pBuffer;
};




ILTOutStream* streamsim_OutMemStream(std::vector<uint8>& OutVec)
{
	return new COutMemStream(OutVec);
}

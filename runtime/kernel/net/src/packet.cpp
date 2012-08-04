
#include "bdefs.h"
#include "packet.h"
#include "syslthread.h"


// special interlock for LINUX
#ifdef __LINUX

#include <pthread.h>

static pthread_mutex_t g_hInterlockedMutex = PTHREAD_MUTEX_INITIALIZER;

void InterlockedIncrement(long* pAddend)
{
	::pthread_mutex_lock(&g_hInterlockedMutex);
	++(*pAddend);
	::pthread_mutex_unlock(&g_hInterlockedMutex);
}

long InterlockedDecrement(long * pAddend)
{
	::pthread_mutex_lock(&g_hInterlockedMutex);
	long nRV = --(*pAddend);
	::pthread_mutex_unlock(&g_hInterlockedMutex);

	return nRV;
}

#endif



//////////////////////////////////////////////////////////////////////////////
// Utility functions

// Count the number of set bits in a value
uint32 CountSetBits(uint32 nValue)
{
	// Non-parallel version, requires loops
	/*
	uint32 nResult = 0;
	while (nValue)
	{
		++nResult;
		nValue &= nValue - 1;
	}
	return nResult;
	*/

	// Constant-time operation
	nValue = ((nValue & 0xAAAAAAAA) >> 1) + (nValue & 0x55555555);
	nValue = ((nValue & 0xCCCCCCCC) >> 2) + (nValue & 0x33333333);
	nValue = ((nValue & 0xF0F0F0F0) >> 4) + (nValue & 0x0F0F0F0F);
	nValue = ((nValue & 0xFF00FF00) >> 8) + (nValue & 0x00FF00FF);
	nValue = ((nValue & 0xFFFF0000) >> 16) + (nValue & 0x0000FFFF);

	return nValue;
}

//////////////////////////////////////////////////////////////////////////////
// Packet data allocation handling

CSysSerialVar g_cPacket_CS_Trash;
CPacket_Data *CPacket_Data::s_pTrash_Data = 0;

CPacket_Data CPacket_Data::s_cTrash_Chunk;

CSysSerialVar g_cPacket_CS_Tracking;

uint32 s_nAllocatedChunks = 0;
uint32 s_nActiveChunks = 0;
uint32 s_nAllocatedPackets = 0;
uint32 s_nActivePackets = 0;

// Gimmie some chunk, baby...
CPacket_Data::SChunk *CPacket_Data::Allocate_Chunk(uint32 nOffset)
{
	g_cPacket_CS_Trash.Lock();
	
	g_cPacket_CS_Tracking.Lock();
	++s_nActiveChunks;
	g_cPacket_CS_Tracking.Unlock();

	if (s_cTrash_Chunk.m_pFirstChunk)
	{
		SChunk *pResult = s_cTrash_Chunk.m_pFirstChunk;
		s_cTrash_Chunk.m_pFirstChunk = s_cTrash_Chunk.m_pFirstChunk->m_pNext;
		
		g_cPacket_CS_Trash.Unlock();

		pResult->Init(nOffset);
	
		return pResult;
	}
	else
	{
		g_cPacket_CS_Tracking.Lock();
		++s_nAllocatedChunks;
		g_cPacket_CS_Tracking.Unlock();
		
		g_cPacket_CS_Trash.Unlock();

		SChunk* pNewChunk;
		LT_MEM_TRACK_ALLOC(pNewChunk = new SChunk(nOffset), LT_MEM_TYPE_NETWORKING);
		return pNewChunk;
	}
}

// Dump the chunk onto the free chunk list
void CPacket_Data::Free_Chunk(SChunk *pChunk)
{
	g_cPacket_CS_Trash.Lock();

	g_cPacket_CS_Tracking.Lock();
	ASSERT(s_nActiveChunks);
	--s_nActiveChunks;
	g_cPacket_CS_Tracking.Unlock();

	pChunk->m_pNext = s_cTrash_Chunk.m_pFirstChunk;
	s_cTrash_Chunk.m_pFirstChunk = pChunk;

	g_cPacket_CS_Trash.Unlock();
}

CPacket_Data* CPacket_Data::Allocate()
{
	// Insert packet tracking here
	// NYI
	g_cPacket_CS_Tracking.Lock();
	++s_nActivePackets;
	g_cPacket_CS_Tracking.Unlock();

	g_cPacket_CS_Trash.Lock();

	CPacket_Data *pResult;
	if (s_pTrash_Data)
	{
		pResult = s_pTrash_Data;
		s_pTrash_Data = (CPacket_Data*)s_pTrash_Data->m_pFirstChunk;
	    pResult->Init();
	}
	else
	{
		g_cPacket_CS_Tracking.Lock();
		++s_nAllocatedPackets;
		g_cPacket_CS_Tracking.Unlock();

		LT_MEM_TRACK_ALLOC(pResult = new CPacket_Data, LT_MEM_TYPE_NETWORKING);
	}

	g_cPacket_CS_Trash.Unlock();

	return pResult;
}

void CPacket_Data::Free()
{
	g_cPacket_CS_Tracking.Lock();

	// Insert packet tracking here
	// NYI
	ASSERT(s_nActivePackets);
	--s_nActivePackets;

	g_cPacket_CS_Tracking.Unlock();

	// Dump our chunks
	while (m_pFirstChunk)
	{
		SChunk *pTemp = m_pFirstChunk;
		m_pFirstChunk = m_pFirstChunk->m_pNext;
		Free_Chunk(pTemp);
	}

	g_cPacket_CS_Trash.Lock();

	// Put ourselves in the trash list
	m_pFirstChunk = (SChunk*)s_pTrash_Data;
	s_pTrash_Data = this;

	g_cPacket_CS_Trash.Unlock();
}

//////////////////////////////////////////////////////////////////////////////
// CPacket_Write implementation

void CPacket_Write::WriteBits(uint32 nValue, uint32 nBits)
{
	ASSERT(nBits <= 32);
	uint32 nWriteMask = (nBits < 32) ? (1 << nBits) - 1 : -1;
	uint32 nWriteValue = nValue & nWriteMask;
	m_nBitAccumulator |= nWriteValue << m_nBitsAccumulated;
	m_nBitsAccumulated += nBits;
	if (m_nBitsAccumulated >= 32)
	{
		if (!m_pData)
		{
			m_pData = CPacket_Data::Allocate();
			m_pData->IncRef();
		}
		m_pData->Append(m_nBitAccumulator, 32);
		m_nBitsAccumulated -= 32;
		if (m_nBitsAccumulated)
			m_nBitAccumulator = nWriteValue >> (nBits - m_nBitsAccumulated);
		else
			m_nBitAccumulator = 0;
	}
}

void CPacket_Write::WriteBits64(uint64 nValue, uint32 nBits)
{
	ASSERT(nBits <= 64);
	WriteBits((uint32)nValue, LTMIN(nBits, 32));
	if (nBits > 32)
	{
		nBits -= 32;
		WriteBits((uint32)(nValue >> 32), nBits);
	}
}

void CPacket_Write::WriteData(const void *pData, uint32 nBits)
{
	// Write it out 32 bits at a time
	const uint32 *pData32 = reinterpret_cast<const uint32*>(pData);
	while (nBits >= 32)
	{
		WriteBits(*pData32, 32);
		++pData32;
		nBits -= 32;
	}
	// Write out whatever's left
	if (nBits)
	{
		uint32 nData8Accumulator = 0;
		uint32 nWriteMask = (nBits < 32) ? (1 << nBits) - 1 : -1;
		uint32 nShift = 0;
		const uint8 *pData8 = reinterpret_cast<const uint8*>(pData32);
		while (nWriteMask)
		{
			nData8Accumulator |= (*pData8 & nWriteMask) << nShift;
			nWriteMask >>= 8;
			nShift += 8;
			++pData8;
		}
		WriteBits(nData8Accumulator, nBits);
	}
}



//-------------------------------------------------------------------------------------
//
//
//	NOTE: Special routine that assume a empty write packet and wants to create the 
//			entire packet structure etc at once. 
//
//
//-------------------------------------------------------------------------------------

void CPacket_Write::WriteDataRaw( void *pData, uint32 nBytes )
{
	if (!m_pData)
	{
		m_pData = CPacket_Data::Allocate();
		m_pData->IncRef();

		uint8 * pData8 = reinterpret_cast<uint8*>(pData);

		m_pData->CreateWriteRaw( pData8, nBytes ); 

	}
	else
	{
		return;
	}

}



void CPacket_Write::WritePacket(const CPacket_Read &cRead)
{
	uint32 nBits = cRead.Size();
	if (!nBits)
		return;
	// Do it the fast way if it's not a sub-packet.
	if (cRead.m_nStart == 0)
	{
		CPacket_Data::TConstIterator iCur = cRead.m_pData->Begin();
		for (; nBits >= 32; ++iCur, nBits -= 32)
			WriteBits(*iCur, 32);
		if (nBits)
			WriteBits(*iCur, nBits);
	}
	else
	{
		CPacket_Read cTempPacket(cRead);
		for (; nBits >= 32; nBits -= 32)
			Writeuint32(cTempPacket.Readuint32());
		if (nBits)
			WriteBits(cTempPacket.ReadBits(nBits), nBits);
	}
}

void CPacket_Write::WriteString(const char *pString)
{
	if (!pString)
	{
		Writeint8(0);
		return;
	}
	while (*pString)
	{
		Writeint8(*pString);
		++pString;
	}
	Writeint8(0);
}

//////////////////////////////////////////////////////////////////////////////
// CPacket_Read implementation

uint32 CPacket_Read::ReadBits(uint32 nBits)
{
	ASSERT(nBits <= 32);
	nBits = LTMIN(nBits, Size() - Tell());
	// Implementation note: shl is masked to 0-31, so if you send in 32, you end up with 0 out of this equation.
	// That's why there's a ternary in here.
	uint32 nReadMask = (nBits < 32) ? ((1 << nBits) - 1) : -1;
	uint32 nCurOffset = (m_nOffset + m_nStart) & 31;
	uint32 nCurLoaded = 32 - nCurOffset;
	uint32 nResult = (m_nCurData >> nCurOffset) & nReadMask;
	m_nOffset += nBits;
	if (nBits >= nCurLoaded)
	{
		++m_iCurData;
		m_nCurData = *m_iCurData;
		if (nBits > nCurLoaded)
		{
			nReadMask >>= nCurLoaded;
			nResult |= (m_nCurData & nReadMask) << nCurLoaded;
		}
	}
	return nResult;
}

uint64 CPacket_Read::ReadBits64(uint32 nBits)
{
	ASSERT(nBits <= 64);
	uint64 nResult = (uint64)ReadBits(LTMIN(nBits, 32));
	if (nBits > 32)
	{
		nBits -= 32;
		nResult |= (uint64)ReadBits(nBits) << 32;
	}
	return nResult;
}

void CPacket_Read::ReadData(void *pData, uint32 nBits)
{
	// Read it out 32 bits at a time
	uint32 *pData32 = reinterpret_cast<uint32*>(pData);
	while (nBits >= 32)
	{
		*pData32 = ReadBits(32);
		++pData32;
		nBits -= 32;
	}
	// Read out whatever's left
	if (nBits)
	{
		uint8 *pData8 = reinterpret_cast<uint8*>(pData32);
		while (nBits)
		{
			uint32 nNumRead = LTMIN(8, nBits);
			*pData8 = (uint8)ReadBits(nNumRead);
			++pData8;
			nBits -= nNumRead;
		}
	}
}

//--------------------------------------------------------------------------------------------
//
//	Note: !!!! This is a special routine that reads the packet into a byte buffer
//			It doesn't do a bunch of checking so make sure the packet and pData are nBytes
//
//
//--------------------------------------------------------------------------------------------
void CPacket_Read::ReadDataRaw(void *pData, uint32 nBytes)
{
	uint32 *pData32 = reinterpret_cast<uint32*>(pData);

	CPacket_Data::TConstIterator iTer = m_pData->Begin();
	uint32 nByteCap = m_pData->GetByteCapacity();

	while ( nBytes )
	{
		memcpy ( pData32, iTer.m_pChunk->m_aData, (nBytes >  nByteCap ? nByteCap : ((nBytes+3)/4)*4));

		if ( nBytes <= nByteCap )
			return;

		nBytes -= nByteCap;
		pData32 += nByteCap/4;
		iTer.NextChunk();
	}
}


uint32 CPacket_Read::ReadString(char *pDest, uint32 nMaxLen)
{
	uint32 nResult = 0;
	char nNextChar;
	char *pEnd = (pDest) ? (pDest + nMaxLen) : 0;
	do
	{
		nNextChar = Readint8();
		++nResult;
		if (pDest != pEnd)
		{
			*pDest = nNextChar;
			++pDest;
		}
	} while (nNextChar != 0);
	return nResult - 1;
}

// Table management class
class CRC32Table
{
public:
	CRC32Table();
	enum { k_Polynomial = 0xedb88320L };
	enum { k_CRCTableSize = 256 };
	uint32 m_Table[k_CRCTableSize];
	inline void Calc(uint32 &nCurCRC, uint8 nData) { 
		nCurCRC = m_Table[(nCurCRC ^ nData) & 0xFF] ^ (nCurCRC >> 8);
	}
};

CRC32Table::CRC32Table()
{
	for (uint32 nCurEntry = 0; nCurEntry < k_CRCTableSize; ++nCurEntry)
	{
		uint32 nAccumulator = nCurEntry;
		for (uint32 nModEntry = 0; nModEntry < 8; ++nModEntry)
		{
			if (nAccumulator & 1)
				nAccumulator = k_Polynomial ^ (nAccumulator >> 1);
			else
				nAccumulator = nAccumulator >> 1;
		}
		m_Table[nCurEntry] = nAccumulator;
	}
}

// The global, static table
CRC32Table g_CRCTable;

uint32 CPacket_Read::CalcChecksum() const
{
	CPacket_Read cChecksumPacket(*this);
	cChecksumPacket.SeekTo(0);
	uint32 nResult = 0xFFFFFFFF;
	while (!cChecksumPacket.EOP())
	{
		g_CRCTable.Calc(nResult, cChecksumPacket.Readuint8());
	}
	return nResult ^ 0xFFFFFFFF;

}


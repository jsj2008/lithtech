
#ifndef __NEWPACKET_H__
#define __NEWPACKET_H__

#include "ltbasetypes.h"

// forward declarations
class CPacket_Read;
class CPacket_Write;
class CPacket_Data;


#ifdef __LINUX
void InterlockedIncrement(long* pAddend);
long InterlockedDecrement(long * pAddend);
#endif

//////////////////////////////////////////////////////////////////////////////
// Packet data class.  Please ignore this class.  It has to be visible in order
// to use its iterator in CPacket_Read.  (And it helps performance a bit
// for it to be visible...)

class CPacket_Data
{
public:
	struct SIterator;
	friend struct SIterator;
	struct SIterator_Const;
	friend struct SIterator_Const;
private:
	struct SChunk;
	friend struct SChunk;




public:
	CPacket_Data() 
	{
		Init();
	}

	void	Init ( )
	{
		m_pFirstChunk = 0;
		m_pLastChunk = 0;
		m_nRefCount = 0;
		m_nSize = 0;
	}
	
	// Allocate a new CPacket_Data
	static CPacket_Data *Allocate();

	// Reference counting...
	void IncRef() const { InterlockedIncrement(( long* )&m_nRefCount ); }
	void DecRef() const { if (!InterlockedDecrement(( long* )&m_nRefCount )) const_cast<CPacket_Data*>(this)->Free(); }
	uint32 GetRefCount() const { return m_nRefCount; }

	bool Append(uint32 nData, uint32 nBits) {
		// We're never supposed to append after an unaligned append
		ASSERT((m_nSize & 31) == 0);
		m_nSize += nBits;
		if (!m_pFirstChunk)
		{
			m_pFirstChunk = Allocate_Chunk();
			m_pLastChunk = m_pFirstChunk;
		}
		m_pLastChunk = m_pLastChunk->Append(nData);
		return true;
	}

	bool CreateWriteRaw ( uint8 * pData, uint32 nBytes )
	{
		m_nSize = nBytes * 8;

		if (!m_pFirstChunk)
		{
			m_pFirstChunk = Allocate_Chunk();
			m_pLastChunk = m_pFirstChunk;
		}

		uint32 nByteCap = GetByteCapacity();

		while ( nBytes )
		{
			m_pLastChunk = m_pLastChunk->WriteRaw( pData, ( nBytes >  nByteCap ? nByteCap : nBytes) );

			if ( nBytes <= nByteCap )
				return true;

			nBytes -= nByteCap;
			pData += nByteCap;
		}

		return false;
	}



	// The iterator type
	typedef SIterator TIterator;
	typedef SIterator_Const TConstIterator;
	// Beginning and ending of the data
	TIterator Begin() { return SIterator(m_pFirstChunk, 0); }
	TIterator End() { return SIterator(0, 0); }
	TConstIterator Begin() const { return SIterator_Const(m_pFirstChunk, 0); }
	TConstIterator End() const { return SIterator_Const(0, 0); }

	uint32 Size() const { return m_nSize; }
	bool Empty() const { return m_nSize == 0; }

	uint32 GetByteCapacity( ) const { return SChunk::k_nByteCapacity; }


public:
	// Don't access this structure directly..  Use TIterator
	struct SIterator
	{
		SIterator(SChunk *pChunk = 0, uint32 nOffset = 0) :
			m_pChunk(pChunk),
			m_nOffset(nOffset)
		{}
		SIterator(const SIterator &sOther) :
			m_pChunk(sOther.m_pChunk),
			m_nOffset(sOther.m_nOffset)
		{}
		SIterator &operator=(const SIterator &sOther) {
			m_pChunk = sOther.m_pChunk;
			m_nOffset = sOther.m_nOffset;
			return *this;
		}
		SIterator &operator++() {
			if (!m_pChunk)
				return *this;
			if (m_nOffset >= SChunk::k_nCapacity)
			{
				m_pChunk = m_pChunk->m_pNext;
				m_nOffset = 0;
			}
			else
				++m_nOffset;
			return *this;
		}
		SIterator operator+(uint32 nOffset) {
			SIterator sResult(*this);
			sResult.m_nOffset += nOffset;
			while (sResult.m_pChunk && (sResult.m_nOffset >= sResult.m_pChunk->m_nInUse))
			{
				sResult.m_nOffset -= sResult.m_pChunk->m_nInUse;
				sResult.m_pChunk = sResult.m_pChunk->m_pNext;
			}
			if (!sResult.m_pChunk)
				sResult.m_nOffset = 0;
			return sResult;
		}
		uint32& operator*() {
			if (!m_pChunk)
			{
				static uint32 nDummyResult;
				nDummyResult = 0;
				return nDummyResult;
			}
			return m_pChunk->m_aData[m_nOffset];
		}
		bool operator==(const SIterator &sOther) {
			return (sOther.m_pChunk == m_pChunk) && (sOther.m_nOffset == m_nOffset);
		}
		SChunk *m_pChunk;
		uint32 m_nOffset;
	};

	struct SIterator_Const
	{
		SIterator_Const(const SChunk *pChunk = 0, uint32 nOffset = 0) :
			m_pChunk(pChunk),
			m_nOffset(nOffset)
		{}
		SIterator_Const(const SIterator_Const &sOther) :
			m_pChunk(sOther.m_pChunk),
			m_nOffset(sOther.m_nOffset)
		{}
		SIterator_Const &operator=(const SIterator_Const &sOther) {
			m_pChunk = sOther.m_pChunk;
			m_nOffset = sOther.m_nOffset;
			return *this;
		}
		SIterator_Const &operator++() {
			if (!m_pChunk)
				return *this;
			++m_nOffset;
			if (m_nOffset >= m_pChunk->m_nInUse)
			{
				m_pChunk = m_pChunk->m_pNext;
				m_nOffset = 0;
			}
			return *this;
		}
		SIterator_Const operator+(uint32 nOffset) {
			SIterator_Const sResult(*this);
			sResult.m_nOffset += nOffset;
			while (sResult.m_pChunk && (sResult.m_nOffset >= sResult.m_pChunk->m_nInUse))
			{
				sResult.m_nOffset -= sResult.m_pChunk->m_nInUse;
				sResult.m_pChunk = sResult.m_pChunk->m_pNext;
			}
			if (!sResult.m_pChunk)
				sResult.m_nOffset = 0;
			return sResult;
		}
		uint32 operator*() {
			if (!m_pChunk)
				return 0;
			return m_pChunk->m_aData[m_nOffset];
		}
		bool operator==(const SIterator_Const &sOther) {
			return (sOther.m_pChunk == m_pChunk) && (sOther.m_nOffset == m_nOffset);
		}

		SIterator_Const &NextChunk() {
			if (!m_pChunk)
				return *this;

			m_pChunk = m_pChunk->m_pNext;
			m_nOffset = 0;

			return *this;
		}


		const SChunk *m_pChunk;
		uint32 m_nOffset;
	};

private:
	// Data chunk structure
	struct SChunk
	{
		SChunk(uint32 nOffset = 0) 
		{
			Init ( nOffset);
		}

		void Init(uint32 nOffset = 0) 
		{
			m_nOffset = nOffset;
			m_nInUse = 0;
			m_pNext = 0;
		}
				
		SChunk *Append(uint32 nValue) {
			ASSERT(m_nInUse < k_nCapacity);
			m_aData[m_nInUse] = nValue;
			++m_nInUse;
			if (m_nInUse == k_nCapacity)
			{
				m_pNext = CPacket_Data::Allocate_Chunk(m_nOffset + m_nInUse);
				return m_pNext;
			}
			else
				return this;
		}

		SChunk *WriteRaw ( uint8 * pData, uint32 nBytes ) 
		{
			m_nInUse = ( nBytes + 3 ) / 4;

			memcpy ( m_aData, (uint32*)pData, (m_nInUse*4) );

			if ( m_nInUse == k_nCapacity )
			{
				m_pNext = CPacket_Data::Allocate_Chunk(m_nOffset + m_nInUse);
				return m_pNext;
			}
			else
				return this;

		}

		enum {
			k_nOverhead = sizeof(uint32) + sizeof(SChunk*),
			k_nSize = 256,
			k_nCapacity = (k_nSize - k_nOverhead) / sizeof(uint32),
			k_nBitCapacity = k_nCapacity * 32,
			k_nByteCapacity = k_nCapacity * 4
		};
		uint32 m_aData[k_nCapacity];
		uint32 m_nOffset;
		uint32 m_nInUse;
		SChunk *m_pNext;
	};

	// Free this data
	void Free();

	// Beginning and ending of the data chunks
	SChunk *m_pFirstChunk, *m_pLastChunk;

	// How many people know about me?
	mutable uint32 m_nRefCount;
	// Size of the data
	uint32 m_nSize;

	// Allocate a new chunk
	static SChunk *Allocate_Chunk(uint32 nOffset = 0);
	// Free a chunk
	static void Free_Chunk(SChunk *pChunk);
	// Static storage for dead chunks
	static CPacket_Data s_cTrash_Chunk;
	// Static storage for dead data
	static CPacket_Data *s_pTrash_Data;
};


//////////////////////////////////////////////////////////////////////////////
// Packet writing class

class CPacket_Write
{

	friend class CPacket_Read;
	
public:
	CPacket_Write() :
		m_pData(0)
	{
		m_nBitAccumulator = 0;
		m_nBitsAccumulated = 0;
	}
	~CPacket_Write()
	{
		if (m_pData)
		{
			ASSERT(m_pData->GetRefCount());
			m_pData->DecRef();
		}
	}

	// Forget everything that's been written
	void Reset() { if (m_pData) m_pData->DecRef(); m_pData = 0; }

	// How much data have we written?
	uint32 Size() const { return ((m_pData) ? (m_pData->Size()) : 0) + m_nBitsAccumulated; }
	bool Empty() const { return Size() == 0; }

	// Data writing functions
	void WriteBits(uint32 nValue, uint32 nBits);
	void WriteBits64(uint64 nValue, uint32 nBits);
	void WriteData(const void *pData, uint32 nBits);

	void WriteDataRaw( void *pData, uint32 nBytes );

	void WritePacket(const CPacket_Read &cRead);

	// Convenience functions
	template <class T>
	void WriteType(const T &tValue) { 
		switch (sizeof(T))
		{
			case 1 : Writeuint8(reinterpret_cast<const uint8 &>(tValue)); break;
			case 2 : Writeuint16(reinterpret_cast<const uint16 &>(tValue)); break; 
			case 4 : Writeuint32(reinterpret_cast<const uint32 &>(tValue)); break;
			case 8 : Writeuint64(reinterpret_cast<const uint64 &>(tValue)); break;
			default : WriteData(&tValue, sizeof(T) * 8); break;
		}
	}
	void Writebool(bool bValue) { WriteBits(bValue ? 1 : 0, 1); }
	void Writeuint8(uint8 nValue) { WriteBits(nValue, 8); }
	void Writeuint16(uint16 nValue) { WriteBits(nValue, 16); }
	void Writeuint32(uint32 nValue) { WriteBits(nValue, 32); }
	void Writeuint64(uint64 nValue) { WriteBits64(nValue, 64); }
	void Writeint8(int8 nValue) { WriteBits((uint32)nValue, 8); }
	void Writeint16(int16 nValue) { WriteBits((uint32)nValue, 16); }
	void Writeint32(int32 nValue) { WriteBits((uint32)nValue, 32); }
	void Writeint64(uint64 nValue) { WriteBits64(nValue, 64); }
	void Writefloat(float fValue) { WriteBits(reinterpret_cast<const uint32&>(fValue), 32); }
	void Writedouble(double fValue) { WriteBits64(reinterpret_cast<const uint64&>(fValue), 64); }
	void WriteLTVector(const LTVector &vValue) { 
		Writefloat(vValue.x); 
		Writefloat(vValue.y); 
		Writefloat(vValue.z); 
	}
	void WriteString(const char *pString);
private:
	// Flush the buffer into the packet data
	void Flush() { 
		if (!m_nBitsAccumulated)
			return;
		if (!m_pData)
		{
			m_pData = CPacket_Data::Allocate();
			m_pData->IncRef(); 
		}
		m_pData->Append(m_nBitAccumulator, m_nBitsAccumulated);
		m_nBitsAccumulated = 0;
		m_nBitAccumulator = 0;
	}

	CPacket_Data *m_pData;

	// Bit buffer for fast & simple writing
	uint32 m_nBitAccumulator;
	uint32 m_nBitsAccumulated;
};

//////////////////////////////////////////////////////////////////////////////
// Packet reading class

class CPacket_Read
{

	friend class CPacket_Write;
	
public:

	CPacket_Read()
		: m_nStart(0)
	{
#ifdef _MSC_VER
		Init(CPacket_Write());
#else	
		CPacket_Write cPacketWrite;
		Init(cPacketWrite);
#endif

	}
	
	// Create a packet reader based on the data in a packet writer.
	// Note : The writing packet will be reset by this operation.  (i.e. the packet
	// will become read-only.)
	explicit CPacket_Read(CPacket_Write &cOther)
		: m_nStart(0)
	{
		Init(cOther);
	}
	
	CPacket_Read(const CPacket_Read &cOther, uint32 nStart = 0)
		: m_pData(cOther.m_pData),
		  m_nStart(cOther.m_nStart + nStart),
		  m_nSize(cOther.Size())
	{
		Init(cOther, nStart, cOther.Size());
	}
	
	// Create a packet reader from another packet reader.  Starts the new reader
	// at the same position as the old reader.
	// nStart is relative to cOther's starting position
	CPacket_Read(const CPacket_Read &cOther, uint32 nStart, uint32 nSize)
		: m_pData(cOther.m_pData),
		  m_nStart(cOther.m_nStart + nStart),
		  m_nSize(nSize)
	{
		Init(cOther, nStart, nSize);
	}
		
	~CPacket_Read()
	{
		if (m_pData)
		{
			ASSERT(m_pData->GetRefCount());
			m_pData->DecRef();
		}
	}
	
	CPacket_Read &operator=(const CPacket_Read &cOther) 
	{
		if (this == &cOther)
			return *this;
		if (m_pData)
			m_pData->DecRef();
		m_pData = cOther.m_pData;
		if (m_pData)
			m_pData->IncRef();
		m_nStart = cOther.m_nStart;
		m_nSize = cOther.m_nSize;
		SeekTo(cOther.m_nOffset);
		return *this;
	}

	// This is the only situation in which you can write to a CPacket_Read -- by clearing it
	void Clear() { *this = CPacket_Read(); }

	// This packet is SOOO big!
	uint32 Size() const { return m_nSize; }
	bool Empty() const { return m_nSize == 0; }
	// Move by this much
	void Seek(int32 nOffset) 
	{ 
		m_nOffset = LTCLAMP((int32)m_nOffset + nOffset, 0, (int32)Size()); 
		RefreshIterator();
	}
	// Go here
	void SeekTo(uint32 nPos) 
	{ 
		m_nOffset = LTMIN(Size(), nPos); 
		RefreshIterator();
	}
	// Where are you?
	uint32 Tell() const { return m_nOffset; }
	// How many bits are left in the packet
	uint32 TellEnd() const { return Size() - Tell(); }
	// Are you done?
	bool EOP() const { return m_nOffset >= Size(); }

	// Base data read functions
	uint32 ReadBits(uint32 nBits);
	uint64 ReadBits64(uint32 nBits);
	void ReadData(void *pData, uint32 nBits);
	void ReadDataRaw(void *pData, uint32 nBits );

	// Convenience functions
	template <class T>
	void ReadType(T *pValue) 
	{ 
		switch (sizeof(T))
		{
			case 1 : { uint8 nTemp = Readuint8(); *pValue = reinterpret_cast<const T &>(nTemp); break; }
			case 2 : { uint16 nTemp = Readuint16(); *pValue = reinterpret_cast<const T &>(nTemp); break; }
			case 4 : { uint32 nTemp = Readuint32(); *pValue = reinterpret_cast<const T &>(nTemp); break; }
			case 8 : { uint64 nTemp = Readuint64(); *pValue = reinterpret_cast<const T &>(nTemp); break; }
			default : { ReadData(pValue, sizeof(T) * 8); break; }
		}
	}
	
	bool Readbool() { return ReadBits(1) != 0; }
	uint8 Readuint8() { return (uint8)ReadBits(8); }
	uint16 Readuint16() { return (uint16)ReadBits(16); }
	uint32 Readuint32() { return (uint32)ReadBits(32); }
	uint64 Readuint64() { return (uint64)ReadBits64(64); }
	int8 Readint8() { return (int8)ReadBits(8); }
	int16 Readint16() { return (int16)ReadBits(16); }
	int32 Readint32() { return (int32)ReadBits(32); }
	int64 Readint64() { return (int64)ReadBits64(64); }
	float Readfloat() { uint32 nTemp = Readuint32(); return reinterpret_cast<const float&>(nTemp); }
	double Readdouble() { uint64 nTemp = Readuint64(); return reinterpret_cast<const double&>(nTemp); }
	
	LTVector ReadLTVector() 
	{ 
		LTVector vResult; 
		vResult.x = Readfloat(); 
		vResult.y = Readfloat(); 
		vResult.z = Readfloat(); 
		return vResult;
	}
	
	// Returns the length of the string (w/o null terminator)
	uint32 ReadString(char *pDest, uint32 nMaxLen);

	// Data peek functions (Versions of the read functions which don't update the read position)
	uint32 PeekBits(uint32 nBits) const { return CPacket_Read(*this).ReadBits(nBits); }
	uint64 PeekBits64(uint32 nBits) const { return CPacket_Read(*this).ReadBits64(nBits); }
	void PeekData(void *pData, uint32 nBits) const { CPacket_Read(*this).ReadData(pData, nBits); }
	template <class T>
	void PeekType(T *pValue) const { CPacket_Read(*this).ReadType(pValue); }
	bool Peekbool() const { return CPacket_Read(*this).Readbool(); }
	uint8 Peekuint8() const { return CPacket_Read(*this).Readuint8(); }
	uint16 Peekuint16() const { return CPacket_Read(*this).Readuint16(); }
	uint32 Peekuint32() const { return CPacket_Read(*this).Readuint32(); }
	uint64 Peekuint64() const { return CPacket_Read(*this).Readuint64(); }
	int8 Peekint8() const { return CPacket_Read(*this).Readint8(); }
	int16 Peekint16() const { return CPacket_Read(*this).Readint16(); }
	int32 Peekint32() const { return CPacket_Read(*this).Readint32(); }
	int64 Peekint64() const { return CPacket_Read(*this).Readint64(); }
	float Peekfloat() const { return CPacket_Read(*this).Readfloat(); }
	double Peekdouble() const { return CPacket_Read(*this).Readdouble(); }
	LTVector PeekLTVector() const { return CPacket_Read(*this).ReadLTVector(); }
	uint32 PeekString(char *pDest, uint32 nMaxLen) const { return CPacket_Read(*this).ReadString(pDest, nMaxLen); }

	// Return the checksum on the packet
	uint32 CalcChecksum() const;
	
private:

	void Init(CPacket_Write &cOther)
	{
		cOther.Flush();
		m_pData = cOther.m_pData;
		if (m_pData)
		{
			m_nSize = m_pData->Size();
			m_pData->IncRef();
		}
		else
		{
			m_nSize = 0;
		}
		cOther.Reset();
		SeekTo(0);
	}
	
	void Init(const CPacket_Read &cOther, uint32 nStart, uint32 nSize)
	{
		// Do some range checking...
		if (m_nStart > (cOther.m_nStart + cOther.m_nSize))
		{
			m_nStart = (cOther.m_nStart + cOther.m_nSize);
			m_nSize = 0;
		}

		m_nSize = LTMIN(m_nSize, cOther.m_nSize - (m_nStart - cOther.m_nStart));

		if (m_pData)
		{
			m_pData->IncRef();
		}
		if ((cOther.m_nOffset + cOther.m_nStart) > m_nStart)
		{
			SeekTo((cOther.m_nOffset + cOther.m_nStart) - m_nStart);
		}
		else
		{
			SeekTo(0);
		}
	}
	
	// Refresh the current iterator position and read buffer
	void RefreshIterator() 
	{
		if (EOP())
		{
			m_nCurData = 0;
			return;
		}
		ASSERT(m_pData); // You should always have a packet if EOP() returns false
		m_iCurData = m_pData->Begin() + ((m_nOffset + m_nStart) / 32); 
		m_nCurData = *m_iCurData;
	}
	
private:

	// Our data
	const CPacket_Data *m_pData;
	// Where our packet starts
	uint32 m_nStart;
	// How big our packet is
	uint32 m_nSize;
	// Where we are
	uint32 m_nOffset;
	// Data iterator for where we are
	CPacket_Data::TConstIterator m_iCurData;
	// Read buffer from where we are
	uint32 m_nCurData;
};

#endif //__NEWPACKET_H__

#ifndef __LTT_BITSET_H__
#define __LTT_BITSET_H__

// CBitSet - a dynamic bitset class
//
// Probably not suitable for use for more than 65536 bits
// (a 16-bit ID space) because GrowBy and AllocFirstFree return int32's 
// with -1 to indicate the maximum size has been exceeded.
//
// Neat feature: SetNewBits allows you to tell the bitset to initialize
// new bits to ON after growth, so you can "virtually" set the entire
// bitset ON (including future bits!) if desired.

// (internal use enums divide by BITS_PER_WORD
//  so pass in numbers of bits as the template parameters)
template< int SIZE = 128, int MAX = 8192, int GROW = 64 >
class CBitSet {
public:
	enum { BITS_PER_WORD = (sizeof(uint32) << 3),
		   INITIAL_SIZE = SIZE / BITS_PER_WORD,  // bits 
		   SIZE_MAX = MAX / BITS_PER_WORD,		// used for DEBUGGING only ! 
		   GROWBY = GROW / BITS_PER_WORD			// bits 
		 };
	CBitSet()
	  : m_bSetNewWords(false),
	    m_curSize(INITIAL_SIZE)
		{ 
			ASSERT( SIZE_MAX <= 65536 );
			if(!m_curSize)
				m_curSize = 1;	// weird - someone wants less than a word's worth of bits!
			LT_MEM_TRACK_ALLOC(m_bitWords = new uint32[ m_curSize ],LT_MEM_TYPE_MISC);
			memset(m_bitWords, '\0', sizeof(m_bitWords) * m_curSize);
		}
	CBitSet(const CBitSet& bs)
		{
			// copy the simple stuff
			m_curSize = bs.m_curSize;
			m_bSetNewWords = bs.m_bSetNewWords;
			// allocate a new bitset for us rather than sharing the pointer
			LT_MEM_TRACK_ALLOC(m_bitWords = new uint32[ m_curSize ],LT_MEM_TYPE_MISC);
			memcpy(m_bitWords, bs.m_bitWords, m_curSize * sizeof(uint32));
		}
	~CBitSet()
		{
			delete[] m_bitWords;
		}
	bool IsSet(uint32 bitOfst) const
		{
			uint32 wordIx = (bitOfst / BITS_PER_WORD);
			if (wordIx < m_curSize)
				return ((m_bitWords[ wordIx ] & (1 << (bitOfst % BITS_PER_WORD))) != 0);
			return m_bSetNewWords;
		}
	bool operator[](int32 bitOfst) const { return bitOfst >= 0 && IsSet((uint32)bitOfst); }
	bool operator[](uint32 bitOfst) const { return IsSet(bitOfst); }
	// turn bit on/off according to bVal
	void Set(uint32 bitOfst, bool bVal)
		{
			const uint32 wordIx = GetExpandedWordIndex(bitOfst);
 			if (bVal)
 				m_bitWords[ wordIx ] |= GetBitMask(bitOfst);
 			else
 				m_bitWords[ wordIx ] &= ~GetBitMask(bitOfst);
		}
	// turn on bit
	void Set(uint32 bitOfst)
		{
			const uint32 wordIx = GetExpandedWordIndex(bitOfst);
			m_bitWords[ wordIx ] |= GetBitMask(bitOfst);
		}
	// turn off bit
	void Reset(uint32 bitOfst)
		{
			const uint32 wordIx = GetExpandedWordIndex(bitOfst);
			m_bitWords[ wordIx ] &= ~GetBitMask(bitOfst);
		}
	// set/reset ALL bits
	void Set()
		{
			memset(m_bitWords, 0xFF, sizeof(m_bitWords) * m_curSize);
		}
			
	void Reset()
		{
			memset(m_bitWords, '\0', sizeof(m_bitWords) * m_curSize);
		}

	// whether new bits (after growth) default to on or off
	void SetNewBits(bool bOn)
		{
			m_bSetNewWords = bOn;
		}

	int32 GrowTo(uint32 size)
		{
			ASSERT(size > m_curSize);
			if(size > m_curSize)
				return GrowBy(size - m_curSize);
		}
	int32 GrowBy(uint32 addAmt)
		{
			uint16 newSize = m_curSize + addAmt;
			#ifdef _DEBUG
			if (newSize > SIZE_MAX)
			{
				ASSERT(!"RCC mgr bit set maximum exceeded");
				return -1;
			}
			#endif
			uint32 * pOldWords = m_bitWords;
			LT_MEM_TRACK_ALLOC(m_bitWords = new uint32[ newSize ],LT_MEM_TYPE_MISC);
			if (!m_bitWords) // not supposed to be able to happen in the engine!
				return -1; 
			// copy over the old words...
			memcpy(m_bitWords, pOldWords, sizeof(m_bitWords[ 0 ]) * m_curSize);
			// zot (or set) the new words...
			memset(&m_bitWords[ m_curSize ], (m_bSetNewWords ? 0xFF : '\0'), sizeof(m_bitWords[ 0 ]) * addAmt);
			// delete the old array
			delete [] pOldWords;
			// set new current size
			m_curSize = newSize;

			return 0;	// why are we returning?
		}

	// find and allocate the first free bit, returning the offset.
	// grow the bit set (up to SIZE_MAX) as necessary.  returns -1
	// if exceeds capacity (_DEBUG only).
	int32 AllocFirstFree()
		{
			uint32 oldSize = m_curSize;
			uint32 i;
			for (i = 0; i < m_curSize; i++)
			{
				register uint32 bitword = m_bitWords[ i ];
				if (bitword != (uint32)(-1))
				{
					// found a word with some free bits...
					uint32 mask = 1;
					uint32 ofst = 0;
					while (ofst < BITS_PER_WORD)
					{
						if ((bitword & mask) == 0)  // bit is zero?
						{
							m_bitWords[ i ] |= mask;
							return ((i * BITS_PER_WORD) + ofst);
						}
						mask <<= 1;
						ofst++;
					}
				}					
			}

			// none free - we need to grow
		#ifdef _DEBUG
			if( GrowBy(GROWBY) == -1 )
				return -1;
		#else
			GrowBy(GROWBY);
		#endif

			ASSERT(!m_bSetNewWords);	// if true, there are NO free bits!

			// allocate the low-order bit of the first of the newly-allocated words
			m_bitWords[ oldSize ] = 1;
			// return the zero-relative bitset offset of that bit.  
			// (e.g. 0 for the first bit allocated and 128 for the 129th bit allocated)
			return (oldSize * BITS_PER_WORD);
		} // CBitSet::AllocFirstFree
	//
	class Reference {
	public:
		Reference(CBitSet& bs, uint32 bitOfst) : m_bitSet(bs), m_bitOfst(bitOfst) {}
		Reference& operator=(bool b) { m_bitSet.Set(m_bitOfst, b); return *this; }
	private:
		uint32 m_bitOfst;
		CBitSet& m_bitSet;
	}; // Reference

	Reference operator[](uint32 bitOfst) { return Reference(*this, bitOfst); }
private:
	uint32 GetBitMask(uint32 bitOfst) { return (1 << (bitOfst % BITS_PER_WORD)); }
	uint32 GetExpandedWordIndex(uint32 bitOfst) 
	{
		uint32 wordIx = (bitOfst / BITS_PER_WORD);
		if (wordIx >= m_curSize)
 		{
			GrowBy(LTMAX(GROWBY, (wordIx - m_curSize) + 1));
			ASSERT(wordIx < m_curSize);
		}
		return wordIx;
	}

	uint32 * m_bitWords;
	bool   m_bSetNewWords;
	uint32 m_curSize;
}; // CBitSet



#endif // __LTT_BITSET_H__

#include "bdefs.h"

// ----------------------------------------------------------
// SHAME ON ME.  I just ripped this STL stuff right outta MFC
// ----------------------------------------------------------

#define POSITION int32
#define BEFORE_START_POSITION ((POSITION)-1)


struct CPlex     // warning variable length structure
{
	CPlex* pNext;
	int32 dwReserved[1];    // align on 8 byte boundary
	void* data() { return this+1; }

	static CPlex* Create(CPlex*& head, uint32 nMax, uint32 cbElement);

	void FreeDataChain();       // free this one and links
};

class CMapWordToPtr
{
protected:
	// Association
	struct CAssoc
	{
		CAssoc* pNext;

		int32 key;
		void* value;
	};

public:

// Construction
	CMapWordToPtr(int16 nBlockSize = 10);

// Attributes
	// number of elements
	int16 GetCount() const;
	LTBOOL IsEmpty() const;

	// Lookup
	LTBOOL Lookup(int32 key, void*& rValue) const;

// Operations
	// Lookup and add if not there
	void*& operator[](int32 key);

	// add a new (key, value) pair
	void SetAt(int32 key, void* newValue);

	// removing existing (key, ?) pair
	LTBOOL RemoveKey(int32 key);
	void RemoveAll();

	// iterating all (key, value) pairs
	POSITION GetStartPosition() const;
	void GetNextAssoc(POSITION& rNextPosition, int32& rKey, void*& rValue) const;

	// advanced features for derived classes
	uint16 GetHashTableSize() const;
	void InitHashTable(uint16 hashSize, LTBOOL bAllocNow = LTTRUE);

// Overridables: special non-virtual (see map implementation for details)
	// Routine used to user-provided hash keys
	uint16 HashKey(uint32 key) const;

// Implementation
protected:
	CAssoc** m_pHashTable;
	uint16 m_nHashTableSize;
	int16 m_nCount;
	CAssoc* m_pFreeList;
	struct CPlex* m_pBlocks;
	int16 m_nBlockSize;

	CAssoc* NewAssoc();
	void FreeAssoc(CAssoc*);
	CAssoc* GetAssocAt(int32, uint16&) const;

public:
	~CMapWordToPtr();


protected:
	// local typedefs for CTypedPtrMap class template
	typedef int32 BASE_KEY;
	typedef int32 BASE_ARG_KEY;
	typedef void* BASE_VALUE;
	typedef void* BASE_ARG_VALUE;
};

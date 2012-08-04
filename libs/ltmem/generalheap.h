// ----------------------------------------------------------------------- //
//
// MODULE  : generalheap.h
//
// PURPOSE : 
//			Create a fast heap to work on the PS2 with NOLF.
//			It doesn't need to deal with very small allocations.
//			We need to minimize fragementation as much as possible.
//			
// CHANGES FOR PC  :
//			Min allocation size is 32 bytes
//			We don't have to do best fit search because fragementation is not our
//			biggest problem.  So just pick the first available size that will
//			fit in our similar size bin.  Is this the right approach?  Lock memory
//			on PC.
//
//
// CREATED : July-17-2001
//
// TO DO :
//			Test in separate test app.
//			Add lots of debugging functionality.
//			Add efficient realloc function that checks for free memory already being there
//			
// ----------------------------------------------------------------------- //

#ifndef __GENERALHEAP_H__
#define __GENERALHEAP_H__

// if this is defined then do not keep searching for the best fit memory
// this improves performance but creates more fragementation
#define GENERALHEAPNOBESTFIT

// number of free lists to keep
#define GENERALHEAPFREELISTARRAYSIZE 12

// space reserved for allocated memory
//#define GENERALHEAPHEADERSIZE 8
#define GENERALHEAPHEADERSIZE 16

// minimum allocation size 
// this can not be smaller than 12 bytes because it must contain the
// free list information
#define GENERALHEAPMINALLOCATIONSIZE 32

// flag mask for bits in header
#define GENERALHEAPFLAGMASK 0x0fffffff

// flags
#define  GENERALHEAPBLOCKFREEFLAG 0x80000000

#ifndef SUPERMEMFILLCHECKING
//#define GENERALHEAPFILLMEMORY
#define GENERALHEAPFILLMEMORYVALUE 0xEAEAEAEA
#else
#define GENERALHEAPFILLMEMORY
extern uint32* g_SuperMemFillCheckBlockFree;
#define GENERALHEAPFILLMEMORYVALUE (uint32)g_SuperMemFillCheckBlockFree
#endif

#ifdef _DEBUG
#define GENERALHEAPINLINE inline
#else
#define GENERALHEAPINLINE inline
#endif

// header structure for heap blocks
// the top 4 bits of the prev mem block are reserved for flags
// accessor functions should always be used so these bits are not messed up
// the highest bit is used to denote if this is a free block 0 = used 1 = free
struct CGeneralHeapHeader
{
	// part of header that is present in allocated blocks
	// the high 4 bits of each of these can be used for flags
	// so they must be accessed and set using the accessor functions
	CGeneralHeapHeader* m_pPrevMemBlock;
	CGeneralHeapHeader* m_pNextMemBlock;

	// additional part of header that is only present in free blocks
	CGeneralHeapHeader* m_pNextFreeBlock;
	CGeneralHeapHeader* m_pPrevFreeBlock;
	uint32 m_nFreeListIndex;
	
	// accessor functions
	CGeneralHeapHeader* GetPrevMemBlock() 
	{ 
		return (CGeneralHeapHeader*)((uint32)m_pPrevMemBlock & GENERALHEAPFLAGMASK); 
	};
	
	void SetPrevMemBlock(CGeneralHeapHeader* pPrevMemBlock) 
	{ 
		m_pPrevMemBlock = (CGeneralHeapHeader*)(((uint32)pPrevMemBlock & GENERALHEAPFLAGMASK) | 
						  ((uint32)m_pPrevMemBlock & ~GENERALHEAPFLAGMASK));
	}

	bool GetFreeFlag()
	{
		if (((uint32)m_pPrevMemBlock & GENERALHEAPBLOCKFREEFLAG) == 0) return false;
		else return true;
	}

	void SetFreeFlag(bool bFree)
	{
		if (bFree) 
		{
			m_pPrevMemBlock = (CGeneralHeapHeader*)(((uint32)m_pPrevMemBlock & GENERALHEAPFLAGMASK) | GENERALHEAPBLOCKFREEFLAG);
		}
		else
		{
			m_pPrevMemBlock = (CGeneralHeapHeader*)(((uint32)m_pPrevMemBlock & GENERALHEAPFLAGMASK) & (~GENERALHEAPBLOCKFREEFLAG));
		}
	}

	
	CGeneralHeapHeader* GetNextMemBlock()
	{
		return m_pNextMemBlock;
	}
	
	void SetNextMemBlock(CGeneralHeapHeader* pNextMemBlock)
	{
		m_pNextMemBlock = pNextMemBlock;
	}
};


// the general heap class
class CGeneralHeap 
{
public:
	// constructors
	CGeneralHeap() { m_bInitialized = false; };
	CGeneralHeap(uint32 nBaseAddress, uint32 nSize, uint32 nAlign) { m_bInitialized = false; Init(nBaseAddress,nSize,nAlign); };
	
	// destructor
	~CGeneralHeap() { Term(); };

	// must be called to initialize the class before it can be used
	GENERALHEAPINLINE void Init(uint32 nBaseAddress, uint32 nSize, uint32 nAlign);
	
	// must be called to terminate the class
	GENERALHEAPINLINE void Term();

	// call to allocate the specified amount of memory from the heap
	GENERALHEAPINLINE void* Alloc(uint32 nSize);
	
	// call to free the specified pointer from the heap
	// the pointer must have been allocated from a previous call to alloc
	GENERALHEAPINLINE void Free(void* free);

	// returns true if the specified pointer is inside the heap or false if it is not
	GENERALHEAPINLINE bool InHeap(void* mem);
	
	// return true if this is valid allocated memory
	GENERALHEAPINLINE bool ValidMemory(void* mem);

	// dump the contents of the heap
	GENERALHEAPINLINE void DumpHeap();
	
	// dump info about a specific piece of memory
	GENERALHEAPINLINE void DumpMemInfo(void* mem);
	
	// get the size of a piece of allocated memory
	GENERALHEAPINLINE uint32 GetSize(void* mem);
	
	// get the first heap element
	GENERALHEAPINLINE CGeneralHeapHeader* GetHeapStart() { return m_pHeapStart; };

	// returns the size of the data portion of the specified block of memory
	GENERALHEAPINLINE uint32 GetMemBlockSize(CGeneralHeapHeader* pMemBlock);
	
	// get the pointer to user data in this memory piece
	GENERALHEAPINLINE void* GetMemBlockData(CGeneralHeapHeader* pMemBlock);
	
#ifdef GENERALHEAPFILLMEMORY
	// check fill memory values for one piece of memory
	GENERALHEAPINLINE bool CheckFillMemoryValue(CGeneralHeapHeader* pHeader);

	// check fill memory values for whole heap
	GENERALHEAPINLINE bool CheckFillMemoryValues();
#endif

	// memory allocations for this class will go though system malloc and free
    void* operator new(size_t size) {	return malloc(size); }
    void operator delete(void* p) { free(p); };

private:

	// inserts the specified block of memory into the approiate free list
	GENERALHEAPINLINE void InsertInFreeList(CGeneralHeapHeader* pMemBlock);

	// removes the specified block of memory from the free list
	// the previous free list block is required to make this operation fast
	GENERALHEAPINLINE void DeleteFromFreeList(CGeneralHeapHeader* pMemBlock);

	// true if class is initialized
	bool m_bInitialized;
	
	// the size of the heap
	uint32 m_nSize;
	
	// alignment to keep all allocations to in the heap
	uint32 m_nAlign;
	
	// the starting location of heap memory
	CGeneralHeapHeader* m_pHeapStart;
	
	// ending location + 1 of heap memory
	CGeneralHeapHeader* m_pHeapEnd;
	
	// array of free lists for heap memory
	CGeneralHeapHeader* m_pFreeList[GENERALHEAPFREELISTARRAYSIZE];
};


// returns the actual data size of the specified memory block
GENERALHEAPINLINE uint32 CGeneralHeap::GetMemBlockSize(CGeneralHeapHeader* pMemBlock)
{
	if (pMemBlock->GetNextMemBlock() != NULL)
	{
		return (uint32)pMemBlock->GetNextMemBlock() - (uint32)pMemBlock - GENERALHEAPHEADERSIZE;
	}
	else
	{
		return (uint32)m_pHeapEnd - (uint32)pMemBlock - GENERALHEAPHEADERSIZE;
	}
}


// get the pointer to user data in this memory piece
GENERALHEAPINLINE void* CGeneralHeap::GetMemBlockData(CGeneralHeapHeader* pMemBlock) 
{ 
	return (void*)((uint32)pMemBlock + GENERALHEAPHEADERSIZE); 
};

	
// finds the correct free list and inserts the specified memory block into the list
GENERALHEAPINLINE void CGeneralHeap::InsertInFreeList(CGeneralHeapHeader* pMemBlock)
{
	// get the size of this memory
	uint32 nSize = GetMemBlockSize(pMemBlock);

	// look for free memory in the free lists
	uint32 nMinSize = 0;
	uint32 nMaxSize = 256;
	for (uint32 i = 1; i < GENERALHEAPFREELISTARRAYSIZE; i++)
	{
		// is this the right size slot for this memory block
		if (nSize < nMaxSize)
		{
			// insert block at start of free list
			pMemBlock->m_pNextFreeBlock = m_pFreeList[i];
			pMemBlock->m_pPrevFreeBlock = NULL;
			if (m_pFreeList[i] != NULL) m_pFreeList[i]->m_pPrevFreeBlock = pMemBlock;
			m_pFreeList[i] = pMemBlock;
			pMemBlock->m_nFreeListIndex = i;
			
			// exit loop we have found our free list
			break;
		}
	
		// go to next size memory free list
		nMinSize = nMaxSize;
		if (i == GENERALHEAPFREELISTARRAYSIZE-2) nMaxSize = 0xffffffff;
		else nMaxSize = nMaxSize <<= 1;
	}
}


// removes the specified memory block from the approiate free list	
GENERALHEAPINLINE void CGeneralHeap::DeleteFromFreeList(CGeneralHeapHeader* pMemBlock)
{
	// if the next item exists fix its prev pointer
	if (pMemBlock->m_pNextFreeBlock != NULL)
	{
		pMemBlock->m_pNextFreeBlock->m_pPrevFreeBlock = pMemBlock->m_pPrevFreeBlock;
	}
	
	// is this the first item in the list
	if (pMemBlock->m_pPrevFreeBlock == NULL)
	{
		ASSERT( ( pMemBlock->m_nFreeListIndex >= 0 ) && ( pMemBlock->m_nFreeListIndex < GENERALHEAPFREELISTARRAYSIZE ) );
		m_pFreeList[pMemBlock->m_nFreeListIndex] = pMemBlock->m_pNextFreeBlock;
	}
	// not the first item
	else
	{
		pMemBlock->m_pPrevFreeBlock->m_pNextFreeBlock = pMemBlock->m_pNextFreeBlock; 
	}
}


GENERALHEAPINLINE void CGeneralHeap::Init(uint32 nBaseAddress, uint32 nSize, uint32 nAlign)
{
	// base address and size must be of proper alignment
	if ((nBaseAddress % nAlign) != 0) return;
	if ((nSize % nAlign) != 0) return;

	// set up basic member variables
	m_nSize = nSize;
	m_nAlign = nAlign;
	m_pHeapStart = (CGeneralHeapHeader*)nBaseAddress;
	m_pHeapEnd = (CGeneralHeapHeader*)(nBaseAddress + nSize);

	// set up the heap free lists
	uint32 nMinSize = 0;
	uint32 nMaxSize = 256;
	for (uint32 i = 1; i < GENERALHEAPFREELISTARRAYSIZE; i++)
	{
		if ((nSize >= nMinSize) && (nSize < nMaxSize))
		{
			// set up the pointer to the base memory
			m_pFreeList[i] = m_pHeapStart;
			
			// set up the values for the header for the base memory
			CGeneralHeapHeader* pHeader = m_pFreeList[i];
			pHeader->SetPrevMemBlock(NULL);
			pHeader->SetNextMemBlock(NULL);
			pHeader->m_pNextFreeBlock = NULL;
			pHeader->m_pPrevFreeBlock = NULL;
			pHeader->m_nFreeListIndex = i;
			pHeader->SetFreeFlag(true);
			
#ifdef GENERALHEAPFILLMEMORY
			// fill the memory with the fill value
			void *memStart = ( void * ) ( ( ( uint32 ) pHeader ) + ( ( uint32 ) sizeof( CGeneralHeapHeader ) ) );
			void *memEnd = ( void * ) ( pHeader->GetNextMemBlock() );
			if (pHeader->GetNextMemBlock() != NULL)
			{
				memEnd = pHeader->GetNextMemBlock();
			}
			else
			{
				memEnd = m_pHeapEnd;
			}
			unsigned int memSize = (( unsigned int ) memEnd - ( unsigned int ) memStart) / 4;
//			memset( memStart, GENERALHEAPFILLMEMORYVALUE, memSize );
			uint32* pMem = (uint32*)memStart;
			for (uint32 n = 0; n < memSize; n++)
			{
				pMem[n] = GENERALHEAPFILLMEMORYVALUE;
			}
#endif // GENERALHEAPFILLMEMORY
	
		}
		else 
		{
			m_pFreeList[i] = NULL;
		}
		nMinSize = nMaxSize;
		if (i == GENERALHEAPFREELISTARRAYSIZE-2) nMaxSize = 0xffffffff;
		else nMaxSize <<= 1;
	}

	m_bInitialized = true;
};


GENERALHEAPINLINE void CGeneralHeap::Term()
{
	if (!m_bInitialized) return;

	m_bInitialized = false;
};


// this will find the piece of memory that most closely fits the size we are requesting
// this helps fragementation!
GENERALHEAPINLINE void* CGeneralHeap::Alloc(uint32 nSize)
{
	// hold the best possible fit free space we can find
	uint32 nBestSizeFound = 0;
	uint32 nBestSizeFoundDiff = 0xffffffff;
	CGeneralHeapHeader* pBestHeader = NULL;

	ASSERT(m_bInitialized);
	
	// adjust size based on the specified alignment
	if ((nSize % m_nAlign) != 0) nSize += m_nAlign - (nSize % m_nAlign);

#ifdef USEFREEZESCREEN
	uint32 nFreezeCount = 0;
#endif

	// look for free memory in the free lists
	uint32 nMinSize = 0;
	uint32 nMaxSize = 256;
	for (uint32 i = 1; i < GENERALHEAPFREELISTARRAYSIZE; i++)
	{
		// if the memory in this free list is possibly big enough check for a free piece
		if (nSize < nMaxSize)
		{
			// get pointer to header for this block
			CGeneralHeapHeader* pHeader = m_pFreeList[i];

			// holds size of the free data block
			uint32 nThisSize;
			
			// loop through all items in free list			
			while (pHeader != NULL)
			{
			
				// get data size of this block
				nThisSize = GetMemBlockSize(pHeader);

				// check if size is large enough for our needs
				if (nThisSize >= nSize)
				{
					uint32 nNewDiff = nThisSize - nSize;
				
					// is it better than our current size
					if (nNewDiff < nBestSizeFoundDiff)
					{
						// update this one to the best one
						nBestSizeFoundDiff = nNewDiff;
						nBestSizeFound = nThisSize;
						pBestHeader = pHeader;
#ifdef GENERALHEAPNOBESTFIT
						// if we found something quit looking
						break;
#endif
					}
				}
				
				// go to next item in free list
				pHeader = pHeader->m_pNextFreeBlock;
#ifdef USEFREEZESCREEN
				nFreezeCount++;
				if (nFreezeCount > 1000000)
				{
					FreezeScreen(0x009000F0, 0x00f0f0f0);
				}
#endif
			}

#ifdef GENERALHEAPNOBESTFIT
			// if we found something quit looking
			if (pBestHeader != NULL) break;
#endif
		}

		// if we have found an exact match stop searching
		if (nBestSizeFoundDiff == 0) break;
		
		// compute new max size for next free list
		nMinSize = nMaxSize;
		if (i >= GENERALHEAPFREELISTARRAYSIZE-2) nMaxSize = 0xffffffff;
		else nMaxSize = nMaxSize <<= 1;;
	}

	// did we find some memory
	if (pBestHeader != NULL)
	{
		// if we need to break up the block then do so
		if (nBestSizeFound > (nSize + GENERALHEAPMINALLOCATIONSIZE))
		{
			// create new free block using the remainder of the space
			CGeneralHeapHeader* pNewBlock = (CGeneralHeapHeader*)((uint32)pBestHeader + 
				nSize + GENERALHEAPHEADERSIZE);
			pNewBlock->SetPrevMemBlock(pBestHeader);
			pNewBlock->SetNextMemBlock(pBestHeader->GetNextMemBlock());
					
			// insert new block into approiate free list
			InsertInFreeList(pNewBlock);				

			// mark new block as free
			pNewBlock->SetFreeFlag(true);
					
			// fix next pointer in previous item
			pBestHeader->SetNextMemBlock(pNewBlock);
			
			// fix prev pointer for next item if it exists
			if (pNewBlock->GetNextMemBlock() != NULL)
			{
				pNewBlock->GetNextMemBlock()->SetPrevMemBlock(pNewBlock);
			}
		}
#ifdef GENERALHEAPFILLMEMORY
		// check fill memory values for this piece of memory
		CheckFillMemoryValue(pBestHeader);
#endif				
		// remove this block from the free list
		DeleteFromFreeList(pBestHeader);
					
		// this block is no longer free so set the free flag to false
		pBestHeader->SetFreeFlag(false);

//		printf("CGeneralHeap::Alloc size=%u location=%x\n",nSize,((uint32)pBestHeader + GENERALHEAPHEADERSIZE));

		// return the new memory address
		return (void*)((uint32)pBestHeader + GENERALHEAPHEADERSIZE);
	}
					
	// we failed to allocate memory	
	else return NULL;
};


GENERALHEAPINLINE void CGeneralHeap::Free(void* free)
{
	// get the pointer to the header for this memory and the previous and next headers
	CGeneralHeapHeader* pHeader = (CGeneralHeapHeader*)((uint32)free - GENERALHEAPHEADERSIZE);
	CGeneralHeapHeader* pPrevHeader = pHeader->GetPrevMemBlock();
	CGeneralHeapHeader* pNextHeader = pHeader->GetNextMemBlock();

#ifdef GENERALHEAPFILLMEMORY
	// igure out farthes value we have to free up
	void* memLimit = m_pHeapEnd;
	if (pHeader->GetNextMemBlock() != NULL)
	{
		memLimit = ( void * ) ( ( ( uint32 ) pHeader->GetNextMemBlock() ) + ( ( uint32 ) sizeof( CGeneralHeapHeader ) ) );
	}
#endif

		
	// check if memory before this memory is free
	// and combine it with this block if it is
	// Note : This is currently causing a crash.
	/*
	if (pPrevHeader != NULL)
	{
		if (pPrevHeader->GetFreeFlag() == true)
		{
			// remove previous block from old free list
			DeleteFromFreeList(pPrevHeader);
			
			// we should combine with the previous memory block
			pPrevHeader->SetNextMemBlock(pHeader->GetNextMemBlock());
			
			// adjust next mem block if it exists
			if (pNextHeader != NULL)
			{
				pNextHeader->SetPrevMemBlock(pPrevHeader);
			}
			
			// make current block now just point to previous block
			pHeader = pPrevHeader;
		}
	}
	//*/
	
	
	// check if memory after this memory is free
	if (pNextHeader != NULL)
	{
		if (pNextHeader->GetFreeFlag() == true)
		{
			// remove next mem block from old free list
			DeleteFromFreeList(pNextHeader);
		
			// get next memory block after our current next one
			CGeneralHeapHeader* pNextNextHeader = pNextHeader->GetNextMemBlock();

			// set new next on previous block			
			pHeader->SetNextMemBlock(pNextNextHeader);

			// set new prev for next block if it exists			
			if (pNextNextHeader != NULL)
			{
				pNextNextHeader->SetPrevMemBlock(pHeader);
			}
		}
	}
	
	// put the new free block back in the free list
	InsertInFreeList(pHeader);
	
	// set the free flag of the block to true
	pHeader->SetFreeFlag(true);
	
#ifdef GENERALHEAPFILLMEMORY
	// fill the memory with the fill value
	void *memStart = ( void * ) ( ( ( uint32 ) pHeader ) + ( ( uint32 ) sizeof( CGeneralHeapHeader ) ) );
	void *memEnd = ( void * ) ( pHeader->GetNextMemBlock() );
	if (pHeader->GetNextMemBlock() != NULL)
	{
		memEnd = pHeader->GetNextMemBlock();
	}
	else
	{
		memEnd = m_pHeapEnd;
	}
	if (memEnd > memLimit) memEnd = memLimit;
	unsigned int memSize = (( unsigned int ) memEnd - ( unsigned int ) memStart) / 4;
//	memset( memStart, GENERALHEAPFILLMEMORYVALUE, memSize );
	uint32* pMem = (uint32*)memStart;
	for (uint32 n = 0; n < memSize; n++)
	{
		pMem[n] = GENERALHEAPFILLMEMORYVALUE;
	}
#endif // GENERALHEAPFILLMEMORY
};


GENERALHEAPINLINE bool CGeneralHeap::InHeap(void* mem)
{
	if ((mem >= (void*)m_pHeapStart) && (mem < (void*)m_pHeapEnd)) 
	{
		return true;
	}
	else 
	{
		return false;
	}
};


GENERALHEAPINLINE void CGeneralHeap::DumpHeap()
{
	CGeneralHeapHeader* p = m_pHeapStart;
/*
	printf("Heap dump :\n");

	while (p != NULL)
	{
		printf("%08x : ", (uint32)p);
		printf("next = %08x prev = %08x data = %08x\n", (uint32)p->GetNextMemBlock(), (uint32)p->GetPrevMemBlock(), (uint32)p+GENERALHEAPHEADERSIZE);
		printf("           size=%08x (%u) ", GetMemBlockSize(p), GetMemBlockSize(p));
				
		if (p->GetFreeFlag()) 
		{
			printf("free ");
			printf("\n           freenext = %08x freeprev=%08x ", (uint32)p->m_pNextFreeBlock, (uint32)p->m_pPrevFreeBlock);
		}
		else printf("used ");

		printf("\n");
		p = p->GetNextMemBlock();
	}
*/
}

// get the size of a piece of allocated memory
GENERALHEAPINLINE uint32 CGeneralHeap::GetSize(void* mem)
{
	CGeneralHeapHeader* pHeader = (CGeneralHeapHeader*)((uint32)mem - GENERALHEAPHEADERSIZE);
	return GetMemBlockSize(pHeader);
}
	

// return true if this is valid allocated memory
GENERALHEAPINLINE bool CGeneralHeap::ValidMemory(void* mem)
{
	CGeneralHeapHeader* pHeader = (CGeneralHeapHeader*)((uint32)mem - GENERALHEAPHEADERSIZE);

	if (pHeader->GetFreeFlag() == true) return false;
	else
	{
		CGeneralHeapHeader* pNextHeader = pHeader->GetNextMemBlock();
		if (pNextHeader != NULL)
		{
			if (pNextHeader->GetPrevMemBlock() != pHeader) return false;
		}
	
		CGeneralHeapHeader* pPrevHeader = pHeader->GetPrevMemBlock();
		if (pPrevHeader != NULL)
		{
			if (pPrevHeader->GetNextMemBlock() != pHeader) return false;
		}
	}
	
	return true;	
}

	
// dump info about a specific piece of memory
GENERALHEAPINLINE void CGeneralHeap::DumpMemInfo(void* mem)
{
	CGeneralHeapHeader* p = (CGeneralHeapHeader*)((uint32)mem - GENERALHEAPHEADERSIZE);
/*	
	printf("%08x : ", (uint32)p);
	printf("next = %08x prev = %08x data = %08x\n", (uint32)p->GetNextMemBlock(), (uint32)p->GetPrevMemBlock(), (uint32)p+GENERALHEAPHEADERSIZE);
	printf("           size=%08x (%u) ", GetMemBlockSize(p), GetMemBlockSize(p));

	if (p->GetFreeFlag()) 
	{
		printf("free ");
		printf("\n           freenext = %08x freeprev=%08x ", (uint32)p->m_pNextFreeBlock, (uint32)p->m_pPrevFreeBlock);
	}
	else printf("used ");
	
	printf("\n");
*/
}


#ifdef GENERALHEAPFILLMEMORY

// check all of the free areas of memory for the free memory value
GENERALHEAPINLINE bool CGeneralHeap::CheckFillMemoryValue(CGeneralHeapHeader* pHeader)
{
	bool bRetVal = true;
	
	// check all of the values to make sure they match
	void *memStart = ( void * ) ( ( ( uint32 ) pHeader ) + ( ( uint32 ) sizeof( CGeneralHeapHeader ) ) );
	void *memEnd = ( void * ) ( pHeader->GetNextMemBlock() );
	if (pHeader->GetNextMemBlock() != NULL)
	{
		memEnd = pHeader->GetNextMemBlock();
	}
	else
	{
		memEnd = m_pHeapEnd;
	}
			
	uint32* pMem = (uint32*)memStart;
	unsigned int memSize = (( unsigned int ) memEnd - ( unsigned int ) memStart) / 4;
			
	for (uint32 n = 0; n < memSize; n++)
	{
		if (pMem[n] != GENERALHEAPFILLMEMORYVALUE)
		{
			bRetVal = false;
			unsigned char* pBadMem = (unsigned char*)&(pMem[n]);
#ifdef _DEBUG_OUTPUT
//			printf("Free'd memory modified at %08x should be=%08x is =%08x\n",
//					pBadMem, (uint32)GENERALHEAPFILLMEMORYVALUE, pMem[n]);
#endif
			ASSERT(FALSE);
			// uncomment this if you want to quit after finding a problem
			// normally we want to find all problems
			// break;
		}
	}
					
	return bRetVal;
}

// check all of the free areas of memory for the free memory value
GENERALHEAPINLINE bool CGeneralHeap::CheckFillMemoryValues()
{
	bool bRetVal = true;
	
	// loop through all the free list size arrays
	for (uint32 i = 1; i < GENERALHEAPFREELISTARRAYSIZE; i++)
	{
		// get pointer to header for this block
		CGeneralHeapHeader* pHeader = m_pFreeList[i];

		// loop through all items in free list			
		while (pHeader != NULL)
		{
			// check this memory piece		
			if (!CheckFillMemoryValue(pHeader))
			{
				// uncomment this if you want to quit after finding a problem
				// normally we want to find all problems
				// break;
			}

			// go to next item in free list
			pHeader = pHeader->m_pNextFreeBlock;
		}
	}
	
	return bRetVal;
}
#endif

#endif

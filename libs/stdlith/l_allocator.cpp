
#include "l_allocator.h"


LAlloc g_DefAlloc;



// -------------------------------------------------------------------------------- //
// LAllocCount implementation.
// -------------------------------------------------------------------------------- //

LAllocCount::LAllocCount(LAlloc *pDelegate)
{
	m_pDelegate = pDelegate;
	
	ClearCounts();
}


void LAllocCount::ClearCounts()
{
	m_nTotalAllocations = 0;
	m_nTotalFrees = 0;
	
	m_TotalMemoryAllocated = 0;
	m_nCurrentAllocations = 0;

	m_nAllocationFailures = 0;
}


void* LAllocCount::Alloc(uint32 size, bool bQuadWordAlign)
{
	void *pRet;
	
	if(size == 0)
		return NULL;

	pRet = m_pDelegate->Alloc(size);
	if(pRet)
	{
		m_nTotalAllocations++;
		m_TotalMemoryAllocated += size;
		m_nCurrentAllocations++;
	}
	else
	{
		m_nAllocationFailures++;
	}

	return pRet;
}


void LAllocCount::Free(void *ptr)
{
	if(!ptr)
		return;

	m_pDelegate->Free(ptr);
	m_nCurrentAllocations--;
	m_nTotalFrees++;
}



// -------------------------------------------------------------------------------- //
// LAllocSimpleBlock implementation.
// -------------------------------------------------------------------------------- //

LAllocSimpleBlock::LAllocSimpleBlock()
{
	Clear();
}


LAllocSimpleBlock::~LAllocSimpleBlock()
{
	Term();
}


LTBOOL LAllocSimpleBlock::Init(LAlloc *pDelegate, uint32 blockSize)
{
	Term();

	blockSize = (blockSize + 3) & ~3;

	if(blockSize > 0)
	{
		m_pBlock = (uint8*)pDelegate->Alloc(blockSize);
		if(!m_pBlock)
			return FALSE;
	}

	m_pDelegate = pDelegate;
	m_BlockSize = blockSize;
	m_CurBlockPos = 0;

	return TRUE;
}


void LAllocSimpleBlock::Term()
{
	if(m_pDelegate)
	{
		m_pDelegate->Free(m_pBlock);
	}

	Clear();
}


void* LAllocSimpleBlock::Alloc(uint32 size, bool bQuadWordAlign)
{
	uint8 *pRet;

	if(size == 0)
		return NULL;

	if (bQuadWordAlign) {	// QuadWord Align (We've over alloced a bit to account for this - if we're not QWAligned, force it to be)...
		pRet = &m_pBlock[m_CurBlockPos];
		m_CurBlockPos += (((uint32)pRet + 0xf) & ~0xf) - (uint32)pRet; }
	else {					// DWord Align by default...
		size = ((size + 3) & ~3); }

	if((m_CurBlockPos + size) > m_BlockSize)
		return NULL;

	pRet = &m_pBlock[m_CurBlockPos];
	m_CurBlockPos += size;
	assert(!bQuadWordAlign || ((uint32)pRet & 0xf) == 0);
	return pRet;
}


void LAllocSimpleBlock::Free(void *ptr)
{
}


void LAllocSimpleBlock::Clear()
{
	m_pDelegate = NULL;
	m_pBlock = NULL;
	m_CurBlockPos = 0;
	m_BlockSize = 0;
}

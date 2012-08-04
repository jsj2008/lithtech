//-----------------------------------------------------------------------------
// ParticleSimulation.h
//
// Provides the base class for an abstract particle simulation that allows for
// a consistant foundation to build particle effects from to allow for sharing
// of features and properties across all particle effects.
//
//-----------------------------------------------------------------------------

#ifndef __PARTICLESIMULATION_H__
#define __PARTICLESIMULATION_H__

#ifndef __MEMORYPAGEMGR_H__
#	include "MemoryPageMgr.h"
#endif

//an iterator that will run through a listing of particles, entirely inlined for
//performance reasons
class CParticleIterator
{
public:

	CParticleIterator(CMemoryPage* pPage, uint32 nStride) :
		m_nStride(nStride)
	{
		SetPage(pPage);
	}

	CParticleIterator(const CParticleIterator& rhs) :
		m_pPage(rhs.m_pPage),
		m_pOffset(rhs.m_pOffset),
		m_nStride(rhs.m_nStride)
	{
	}

	//determines if this iterator is at the end of the list or not
	bool IsDone() const
	{
		return (m_pPage == NULL);
	}

	//moves to the next particle in the list. This will handle page boundaries. Note that this
	//assumes that the current position is valid
	void Next()
	{
		m_pOffset += m_nStride;

		//see if we are done with this page
		if(m_pOffset >= m_pPage->GetAllocationPtr())
		{
			SetPage(m_pPage->m_pNextPage);
		}
	}

	//access the particle we are currently at, this can be NULL if the iterator is at the end
	uint8*	GetParticle()
	{
		return m_pOffset;
	}

private:

	//sets up the iterator to point to the beginning of the specified page
	void	SetPage(CMemoryPage* pPage)
	{
		m_pPage		= pPage;
		m_pOffset	= (pPage) ? pPage->GetMemoryBlock() : NULL;
	}

	CMemoryPage*		m_pPage;
	uint8*				m_pOffset;
	uint32				m_nStride;
};

//an iterator that will run through a listing of particles, entirely inlined for
//performance reasons
class CParticleReverseIterator
{
public:

	CParticleReverseIterator(CMemoryPage* pPage, uint32 nStride) :
		m_nStride(nStride)
	{
		SetPage(pPage);
	}

	CParticleReverseIterator(const CParticleReverseIterator& rhs) :
		m_pPage(rhs.m_pPage),
		m_pOffset(rhs.m_pOffset),
		m_nStride(rhs.m_nStride)
	{
	}

	//determines if this iterator is at the end of the list or not
	bool IsDone() const
	{
		return (m_pPage == NULL);
	}

	//moves to the next particle in the list. This will handle page boundaries. Note that this
	//assumes that the current position is valid
	void Prev()
	{
		m_pOffset -= m_nStride;

		//see if we are done with this page
		if(m_pOffset < m_pPage->GetMemoryBlock())
		{
			SetPage(m_pPage->m_pPrevPage);
		}
	}

	//access the particle we are currently at, this can be NULL if the iterator is at the end
	uint8*	GetParticle()
	{
		return m_pOffset;
	}

private:

	//sets up the iterator to point to the beginning of the specified page
	void	SetPage(CMemoryPage* pPage)
	{
		m_pPage		= pPage;
		m_pOffset	= (pPage) ? (uint8*)(pPage->GetAllocationPtr() - m_nStride) : NULL;
	}

	CMemoryPage*		m_pPage;
	uint8*				m_pOffset;
	uint32				m_nStride;
};

//a collection of particle memory
class CParticleSimulation
{
public:

	//lifetime operations
	CParticleSimulation(uint32 nStride = 0);
	~CParticleSimulation();

	//sets the stride of the particle simulation. This will fail if any particles are allocated
	bool				SetStride(uint32 nStride);

	//gets the number of particles currently allocated
	uint32				GetNumParticles() const	{ return m_nNumParticles; }

	//access to the stride (this cannot be changed during the lifetime)
	uint32				GetStride() const		{ return m_nParticleStride; }

	//provides an iterator to run through the particles
	CParticleIterator	GetIterator()			{ return CParticleIterator(m_pPageList, GetStride()); }

	//provides an iterator to run through the particles in reverse
	CParticleReverseIterator	GetReverseIterator()	{ return CParticleReverseIterator(m_pLastPage, GetStride()); }

	//frees all the particles in the simulation
	void				FreeAllParticles();

	//called to allocate a block of memory for a particle. This will be added on the end, and will not
	//cause any issues with valid iterators
	uint8*				AllocateParticle();

	//called to free a specified particle (this will perform a swap list remove). This will invalidate
	//any iterators that occur beyond the particle that is being removed.
	void				RemoveParticle(uint8* pRemoveAt);

	//performs the same as the above, but uses an iterator to find the particle to remove, and will
	//also return the iterator for the particle after the one that was just removed
	CParticleReverseIterator	RemoveParticle(CParticleReverseIterator& Iterator);

private:

	//the number of currently allocated particles
	uint32				m_nNumParticles;

	//the stride of the particle simulation
	uint32				m_nParticleStride;

	//our list of memory pages
	CMemoryPage*		m_pPageList;

	//the pointer to the last page
	CMemoryPage*		m_pLastPage;
};

#endif


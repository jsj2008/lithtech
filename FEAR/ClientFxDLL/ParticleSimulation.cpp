#include "StdAfx.h"
#include "ParticleSimulation.h"

//-------------------------------------------------------------------------
// Particle System Memory Management
//-------------------------------------------------------------------------

//the size of the memory pages that will be allocated. Particles must not be larger than this
//size
#define PARTICLE_PAGE_SIZE		(4 * 1024)		//4k

//global particle page manager. This handles all of the memory allocation for the particles
static CMemoryPageMgr	g_ParticlePageMgr(PARTICLE_PAGE_SIZE);

//-------------------------------------------------------------------------
// CParticleSimulation
//-------------------------------------------------------------------------

//lifetime operations
CParticleSimulation::CParticleSimulation(uint32 nStride) :
	m_nParticleStride(nStride),
	m_pPageList(NULL),
	m_pLastPage(NULL),
	m_nNumParticles(0)
{
	LTASSERT(nStride <= PARTICLE_PAGE_SIZE, "Error: Invalid particle size");
}

CParticleSimulation::~CParticleSimulation()
{
	FreeAllParticles();
}

//frees all the particles in the simulation
void CParticleSimulation::FreeAllParticles()
{
	while(m_pPageList)
	{
		RemoveParticle(m_pPageList->GetMemoryBlock());
	}
}

//sets the stride of the particle simulation. This will fail if any particles are allocated
bool CParticleSimulation::SetStride(uint32 nStride)
{
	//fail if there are any particles (because changing the stride will not let us free or
	//iterate over them)
	if(m_nNumParticles > 0)
	{
		//we still have particles
		return false;
	}

	m_nParticleStride = nStride;
	return true;
}

//called to allocate a block of memory for a particle
uint8* CParticleSimulation::AllocateParticle()
{
	//this one is fairly simple, just see if there is enough room in our last page, and if
	//it is not, we need to add a page and allocate
	if(m_pLastPage)
	{
		uint8* pParticle = m_pLastPage->Allocate(GetStride());

		//see if there was memory
		if(pParticle)
		{
			//success, return the particle
			m_nNumParticles++;
			return pParticle;
		}
	}

	//we don't have room, we need to add a new page
	CMemoryPage* pNewPage = g_ParticlePageMgr.AllocatePage();

	//see if we ran out of memory
	if(!pNewPage)
		return NULL;

	//and now allocate our particle
	uint8* pParticle = pNewPage->Allocate(GetStride());

	//if we failed to allocate a particle, we need to release this page to ensure that there is at least
	//one particle on the last page
	if(!pParticle)
	{
		g_ParticlePageMgr.FreePage(pNewPage);
		return NULL;
	}

	//we have a new particle page, add it to our list
	if(m_pLastPage)
	{
		m_pLastPage->m_pNextPage = pNewPage;
	}
	pNewPage->m_pPrevPage = m_pLastPage;

	//this is now our last page
	m_pLastPage = pNewPage;

	//and possibly our first
	if(!m_pPageList)
		m_pPageList = pNewPage;

	m_nNumParticles++;
	return pParticle;
}

//called to free a specified particle (this will perform a swap list remove)
void CParticleSimulation::RemoveParticle(uint8* pRemoveAt)
{
	//fail if any pointer is invalid
	if(!pRemoveAt || !m_pLastPage)
		return;

	//sanity check
	LTASSERT(m_pLastPage->GetAllocationOffset() >= GetStride(), "Error: Attempted to free more memory than was allocated");

	//we need to obtain the pointer to the last element
	const uint8* pLastElement = m_pLastPage->GetMemoryBlock() + (m_pLastPage->GetAllocationOffset() - m_nParticleStride);

	//copy memory over as long as the items aren't the same
	if(pLastElement != pRemoveAt)
	{
		memcpy(pRemoveAt, pLastElement, m_nParticleStride);
	}

	//now free this particle
	m_pLastPage->Free(GetStride());

	//see if we cleared this page out
	if(m_pLastPage->IsEmpty())
	{
		//we need to remove this page from our list
		CMemoryPage* pRemove = m_pLastPage;
		m_pLastPage = pRemove->m_pPrevPage;

		//break the links
		if(m_pLastPage)
		{
			m_pLastPage->m_pNextPage = NULL;
		}
		else
		{
			//no previous, we have flushed our list
			m_pPageList = NULL;
		}
		pRemove->m_pPrevPage = NULL;

		g_ParticlePageMgr.FreePage(pRemove);
	}

	m_nNumParticles--;
}

//performs the same as the above, but uses an iterator to find the particle to remove, and will
//also return the iterator for the particle after the one that was just removed
CParticleReverseIterator CParticleSimulation::RemoveParticle(CParticleReverseIterator& Iterator)
{
	//get the next particle in this list
	CParticleReverseIterator Prev(Iterator);
	Prev.Prev();

	// Disable this particle
	RemoveParticle(Iterator.GetParticle());
	return Prev;
}




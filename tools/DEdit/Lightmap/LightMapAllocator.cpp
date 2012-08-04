#include "bdefs.h"
#include "LightMapAllocator.h"



CLightMapAllocator::CLightMapAllocator() :
	m_nMaxSize(32),
	m_nMinSize(4),
	m_fMinTexelSize(32)
{
}

CLightMapAllocator::~CLightMapAllocator()
{
}

//called to determine the dimensions of a lightmap of the specified extents
CLightMapData* CLightMapAllocator::AllocateLightMap(float fXExtent, float fYExtent)
{
	//first off we need to determine the size of these lightmaps
	uint32 nWidth	= (uint32)ceil(fXExtent / m_fMinTexelSize);
	uint32 nHeight	= (uint32)ceil(fYExtent / m_fMinTexelSize);

	//now we need to clamp them appropriately
	uint32 nLMWidth  = LTMAX(2, LTMIN(nWidth, GetMaxSize()));
	uint32 nLMHeight = LTMAX(2, LTMIN(nHeight, GetMaxSize()));

	//don't allocate it if it is too small
	if((nLMWidth < GetMinSize()) && (nLMHeight < GetMinSize()))
	{
		return NULL;
	}

	//now allocate the actual object
	CLightMapData* pRV = new CLightMapData;

	if(pRV == NULL)
		return NULL;

	if(!pRV->SetSize(nLMWidth, nLMHeight))
	{
		//failed to set size
		delete pRV;
		return NULL;
	}

	//success
	return pRV;
}

//set the maximum size of a lightmap
void CLightMapAllocator::SetMaxSize(uint8 nMax)
{
	//sanity check
	ASSERT(nMax > 0);

	m_nMaxSize = nMax;
}

//retreives the maximum size of a lightmap
uint8 CLightMapAllocator::GetMaxSize() const
{
	return m_nMaxSize;
}

//set the minumum size a LM texel can cover
void CLightMapAllocator::SetMinTexelSize(float fMinSize)
{
	ASSERT(fMinSize > 1.0f);

	m_fMinTexelSize = fMinSize;
}

//retreive the minimum size
float CLightMapAllocator::GetMinTexelSize() const
{
	return m_fMinTexelSize;
}

//sets the minimum size of a lightmap
void CLightMapAllocator::SetMinSize(uint8 nMin)
{
	m_nMinSize = nMin;
}

//gets the minimum size of a lightmap
uint8 CLightMapAllocator::GetMinSize() const
{
	return m_nMinSize;
}

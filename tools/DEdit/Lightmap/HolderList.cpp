#include "bdefs.h"
#include "HolderList.h"
#include "SortableHolder.h"
#include <memory.h>
#include <assert.h>

//class used to store the score of a holder
struct CHolderScore
{
public:
	uint32					m_nScore;
	ISortableHolder*		m_pHolder;
};

//callback for qsort when sorting the holder list
static int SortHoldersCB(const void *pElem1, const void *pElem2 ) 
{
	//convert the pointers to data we can use
	CHolderScore* pHolder1 = (CHolderScore*)pElem1;
	CHolderScore* pHolder2 = (CHolderScore*)pElem2;

	//just use the scores
	return (int)pHolder1->m_nScore - (int)pHolder2->m_nScore;
}



CHolderList::CHolderList() :
	m_ppHolders(NULL),
	m_nNumHolders(0),
	m_nBufferSize(0)
{
}

CHolderList::~CHolderList()
{
	Free();
}

//adds a holder onto the list. If the holder is already in the list, no
//duplicate is added
bool CHolderList::AddHolder(ISortableHolder* pHolder, bool bSearchForDuplicates)
{
	//see if it is already in the list
	if(bSearchForDuplicates)
	{
		for(uint32 nCurrHolder = 0; nCurrHolder < GetNumHolders(); nCurrHolder++)
		{
			if(m_ppHolders[nCurrHolder] == pHolder)
				return true;
		}
	}
	
	if(m_nNumHolders + 1 >= m_nBufferSize)
	{
		if(!ResizeArray(m_nNumHolders + 1))
		{
			return false;
		}
	}

	m_ppHolders[m_nNumHolders] = pHolder;
	m_nNumHolders++;

	return true;
}

//determines if this holder is the active holder
bool CHolderList::IsActiveHolder(const ISortableHolder* pHolder) const
{
	return (((CHolderList*)this)->GetActiveHolder() == pHolder);
}

//gets the active holder
ISortableHolder* CHolderList::GetActiveHolder()
{
	if(m_nNumHolders > 0)
	{
		return m_ppHolders[0];
	}

	return NULL;
}


//removes a holder from the list
bool CHolderList::RemoveHolder(ISortableHolder* pHolder)
{
	//see if it is already in the list
	for(uint32 nCurrHolder = 0; nCurrHolder < GetNumHolders(); nCurrHolder++)
	{
		if(m_ppHolders[nCurrHolder] == pHolder)
		{
			//move everything past this point back
			for(uint32 nMove = nCurrHolder; nMove < GetNumHolders() - 1; nMove++)
			{
				m_ppHolders[nMove] = m_ppHolders[nMove + 1];
			}

			//decrement the count
			m_nNumHolders--;

			//shrink the array
			ResizeArray(m_nNumHolders);

			//success
			return true;
		}
	}

	return false;
}


//gets the specified holder
ISortableHolder* CHolderList::GetHolder(uint32 nIndex)
{
	ASSERT(nIndex < GetNumHolders());
	return m_ppHolders[nIndex];
}

//gets the number of holders
uint32 CHolderList::GetNumHolders() const
{
	return m_nNumHolders;
}

//frees everything assocaited with the list
void CHolderList::Free()
{
	//clear out the array
	delete [] m_ppHolders;
	m_ppHolders = NULL;

	m_nNumHolders = 0;
	m_nBufferSize = 0;
}


#define BUFFER_RESOLUTION		1024

bool CHolderList::ResizeArray(uint32 nSize)
{
	//see if the buffer will accomodate the size
	if(nSize < m_nBufferSize)
	{
		//if we aren't wasting a lot of space, don't worry about it
		if(m_nBufferSize - nSize < (BUFFER_RESOLUTION * 3) / 2)
		{
			return true;
		}
	}

	//we need to resize this to the right size, bumping it up one buffer resolution
	uint32 nFinalSize = ((nSize + BUFFER_RESOLUTION - 1) / BUFFER_RESOLUTION) * BUFFER_RESOLUTION; 

	//alright, allocate the list
	ISortableHolder** ppNewList = new ISortableHolder* [nFinalSize];

	//check the allocation
	if(ppNewList == NULL)
	{
		//failure
		Free();
		return false;
	}

	assert(nSize >= m_nNumHolders);

	//copy over as much of the list as possible
	memcpy(ppNewList, m_ppHolders, m_nNumHolders * sizeof(ISortableHolder*));

	//free the old array
	delete [] m_ppHolders;

	m_ppHolders = ppNewList;
	m_nBufferSize = nFinalSize;

	//success
	return true;
}

//sorts the holders in the list by the given camera position and orientation
bool CHolderList::Sort(const LTVector& vCameraPos, const LTVector& vCameraDir)
{
	//bail if not enough elements to actually sort
	if(GetNumHolders() < 3)
		return true;

	//create a list of the scores and holders
	CHolderScore* pScore = new CHolderScore[m_nNumHolders];

	//check the allocation
	if(pScore == NULL)
		return false;

	//find the score of each holder
	uint32 nCurrHolder;

	for(nCurrHolder = 0; nCurrHolder < m_nNumHolders; nCurrHolder++)
	{
		//get the holder
		pScore[nCurrHolder].m_pHolder = m_ppHolders[nCurrHolder];
		pScore[nCurrHolder].m_nScore  = m_ppHolders[nCurrHolder]->CalcScore(vCameraPos, vCameraDir);
	}

	//one important thing though, we can't have it moving around the first element, so that must have a score
	//of 0 in order to not be rearranged
	pScore[0].m_nScore = 0;

	qsort(pScore, m_nNumHolders, sizeof(CHolderScore), SortHoldersCB);

	//now we need to update the array to reflect the new order
	for(nCurrHolder = 0; nCurrHolder < m_nNumHolders; nCurrHolder++)
	{
		m_ppHolders[nCurrHolder] = pScore[nCurrHolder].m_pHolder;
	}

	//clean up the array
	delete [] pScore;

	//success
	return true;
}


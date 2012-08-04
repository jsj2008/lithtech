#include "meshshapelist.h"
#include "meshshape.h"

CMeshShapeList::CMeshShapeList() :
	m_ppShapeList(NULL),
	m_nNumShapes(0)
{
}

CMeshShapeList::~CMeshShapeList()
{
	Free();
}

//determines the amount of memory occupied by this list
uint32 CMeshShapeList::GetMemoryFootprint() const
{
	uint32 nTotal = 0;
	for(uint32 nCurrShape = 0; nCurrShape < GetNumShapes(); nCurrShape++)
	{
		nTotal += m_ppShapeList[nCurrShape]->GetMemoryFootprint();
	}

	return nTotal;
}

//gets the number of shapes in the list
uint32 CMeshShapeList::GetNumShapes() const
{
	return m_nNumShapes;
}

//gets a specific shape
CMeshShape*	CMeshShapeList::GetShape(uint32 nShape)
{
	if(nShape >= GetNumShapes())
	{
		ASSERT(false);
		return NULL;
	}
	return m_ppShapeList[nShape];
}

//sets the number of shapes available
bool CMeshShapeList::SetNumShapes(uint32 nNumShapes)
{
	Free();

	m_ppShapeList = new CMeshShape* [nNumShapes];

	if(m_ppShapeList == NULL)
	{
		return false;
	}

	for(uint32 nCurrShape = 0; nCurrShape < GetNumShapes(); nCurrShape++)
	{
		m_ppShapeList[nCurrShape] = NULL;
	}

	m_nNumShapes = nNumShapes;

	return true;
}

//sets a specific shape, orphaning it
bool CMeshShapeList::SetShape(uint32 nShape, CMeshShape* pShape)
{
	if(nShape >= GetNumShapes())
	{
		ASSERT(false);
		return false;
	}
	m_ppShapeList[nShape] = pShape;

	return true;
}

//clears all associated memory
void CMeshShapeList::Free()
{
	for(uint32 nCurrShape = 0; nCurrShape < GetNumShapes(); nCurrShape++)
	{
		delete m_ppShapeList[nCurrShape];
	}

	delete [] m_ppShapeList;

	m_ppShapeList	= NULL;
	m_nNumShapes	= 0;
}

//gets the dimension for a bounding box that encompasses the model
bool CMeshShapeList::GetBoundingBox(LTVector& vMin, LTVector& vMax)
{
	//see if we have any shapes
	if(GetNumShapes() == 0)
	{
		return false;
	}

	uint32 nNumBoxesFound = 0;

	//we have shapes, so get the bounding box that encompasses all of them
	LTVector vShapeMin, vShapeMax;

	for(uint32 nCurrShape = 0; nCurrShape < GetNumShapes(); nCurrShape++)
	{
		CMeshShape* pShape = GetShape(nCurrShape);
		ASSERT(pShape);

		if(pShape->GetBoundingBox(vShapeMin, vShapeMax))
		{
			//see if this is the starting box
			if(nNumBoxesFound == 0)
			{
				vMin = vShapeMin;
				vMax = vShapeMax;
			}
			else
			{
				VEC_MIN(vMin, vShapeMin, vMin);
				VEC_MAX(vMax, vShapeMax, vMax);
			}
			nNumBoxesFound++;
		}
	}

	return (nNumBoxesFound > 0) ? true : false;
}



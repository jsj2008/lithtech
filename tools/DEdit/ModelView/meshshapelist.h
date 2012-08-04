#ifndef __MESHSHAPELIST_H__
#define __MESHSHAPELIST_H__

#include <ltbasedefs.h>

class CMeshShape;

class CMeshShapeList
{
public:

	CMeshShapeList();
	~CMeshShapeList();

	//determines the amount of memory occupied by this list
	uint32		GetMemoryFootprint() const;

	//gets the number of shapes in the list
	uint32		GetNumShapes() const;

	//gets a specific shape
	CMeshShape*	GetShape(uint32 nShape);

	//sets the number of shapes available
	bool		SetNumShapes(uint32 nNumShapes);

	//sets a specific shape
	bool		SetShape(uint32 nShape, CMeshShape* pShape);

	//gets the dimension for a bounding box that encompasses the model
	//returns false if no bounding box could be determined
	bool		GetBoundingBox(LTVector& vMin, LTVector& vMax);

private:

	//clears all associated memory
	void		Free();

	CMeshShape**	m_ppShapeList;
	uint32			m_nNumShapes;
};

#endif

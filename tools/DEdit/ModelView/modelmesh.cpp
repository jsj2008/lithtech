#include "modelmesh.h"
#include "ltamgr.h"
#include "meshshapelist.h"

CModelMesh::CModelMesh(const char* pszFilename, const CModelHandle& Handle) :
	m_pShapeList(NULL),
	m_bValid(false),
	m_vDimensions(0, 0, 0),
	m_nObjectRefCount(0),
	m_nShapeListRefCount(0),
	m_vBoundingMin(0, 0, 0),
	m_vBoundingMax(0, 0, 0)
{
	SetHandle(Handle);
	SetFilename(pszFilename);
}

CModelMesh::~CModelMesh()
{
	Free();
}


//determines if the shapes associated with this mesh are in memory
//or need to be loaded from the disk again
bool CModelMesh::AreShapesLoaded() const
{
	//only permit exclusive acess
	return (m_pShapeList) ? true  : false;
}


//gets the size that this current model's shapes are occupying
uint32 CModelMesh::GetMemoryFootprint() const
{
	if(AreShapesLoaded())
	{
		ASSERT(m_pShapeList);
		return m_pShapeList->GetMemoryFootprint();
	}

	//no shapes loaded, so no memory footprint
	return 0;
}


//causes all the shapes to be discarded from memory
bool CModelMesh::UnloadShapes()
{
	//clean up the shape list
	if(AreShapesLoaded())
	{
		m_CSShapeList.Lock();
		//clear the shapes
		delete m_pShapeList;
		m_pShapeList = NULL;

		m_CSShapeList.Unlock();
	}

	return true;
}

//opens up the file and attempts to find the user dimensions of this model
bool CModelMesh::LoadDimensions(const char* pszFilename)
{
	//open up the reader
	CLTAReader InFile;
	InFile.Open(pszFilename, CLTAUtil::IsFileCompressed(pszFilename));

	//just set the user dimensions for now
	m_vDimensions.Init(10, 10, 10);

	return InFile.IsValid();	
}


//sets the filename that this model represents. Will return false if it
//cannot find the file or get the appropriate data from it
bool CModelMesh::SetFilename(const char* pszFilename)
{
	//clear any old state
	Free();

	//now we need to load the dimensions in from the file
	if(LoadDimensions(pszFilename) == false)
	{
		Free();
		return false;
	}

	strncpy(m_pszFilename, pszFilename, MAX_PATH);

	//success
	m_bValid = true;
	return true;
}


//gets the filename associated with this model
const char* CModelMesh::GetFilename() const
{
	return m_pszFilename;
}


//gets the user dimensions of this model
LTVector CModelMesh::GetDimensions() const
{
	return m_vDimensions;
}


//determines if this object is valid
bool CModelMesh::IsValid() const
{
	return m_bValid;
}


//gets the handle associated with this model
CModelHandle CModelMesh::GetHandle() const
{
	return m_Handle;
}


//sets the handle of this object
void CModelMesh::SetHandle(const CModelHandle& Handle)
{
	m_Handle = Handle;
}


//gets the shape list associated with the model
CMeshShapeList* CModelMesh::GetShapeList()
{
	//only permit exclusive acess
	m_CSShapeList.Lock();
	CMeshShapeList* pRV = m_pShapeList;
	m_CSShapeList.Unlock();

	return pRV;
}


//threadsafe way of setting the loaded flag
void CModelMesh::SetShapeList(CMeshShapeList* pShapeList)
{
	//only permit exclusive acess
	m_CSShapeList.Lock();
	m_pShapeList = pShapeList;

	//update the bounding box of the shape list
	if(m_pShapeList)
	{
		if(m_pShapeList->GetBoundingBox(m_vBoundingMin, m_vBoundingMax) == false)
		{
			//failed to get the bounding box, so reset it
			m_vBoundingMin.Init(0, 0, 0);
			m_vBoundingMax.Init(0, 0, 0);
		}
	}

	m_CSShapeList.Unlock();
}


//resets it to an initial state
void CModelMesh::Free()
{
	ASSERT(GetNumReferences() == 0);
	ASSERT(GetNumShapeListReferences() == 0);

	UnloadShapes();

	m_vDimensions.Init(0, 0, 0);

	m_vBoundingMin.Init(0, 0, 0);
	m_vBoundingMax.Init(0, 0, 0);

	m_nObjectRefCount		= 0;
	m_nShapeListRefCount	= 0;

	m_pShapeList			= NULL;
	m_bValid				= false;
}

//add a reference to the model
void CModelMesh::AddReference()
{
	m_nObjectRefCount++;
}

//rease a reference to the model
void CModelMesh::ReleaseReference()
{
	if(m_nObjectRefCount > 0)
	{
		m_nObjectRefCount--;
	}
	else
	{
		ASSERT(false);
	}
}

//add a reference to the shape list
void CModelMesh::AddShapeListReference()
{
	m_nShapeListRefCount++;
}
	
//release a reference to the shape list
void CModelMesh::ReleaseShapeListReference()
{
	if(m_nShapeListRefCount > 0)
	{
		m_nShapeListRefCount--;
	}
	else
	{
		ASSERT(false);
	}
}

//get the number of references to the model
uint32 CModelMesh::GetNumReferences() const
{
	return m_nObjectRefCount;
}

//gets the number of references to the shape list
uint32 CModelMesh::GetNumShapeListReferences() const
{
	return m_nShapeListRefCount;
}

//gets the bounding box of the model
bool CModelMesh::GetBoundingBox(LTVector& vMin, LTVector& vMax)
{
	vMin = m_vBoundingMin;
	vMax = m_vBoundingMax;

	return true;
}



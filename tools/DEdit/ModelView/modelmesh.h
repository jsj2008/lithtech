//-------------------------------------------------------------
// ModelMesh.h
//
// Provides the internal representation for a model's mesh
// inside of DEdit.
//
// Author: John O'Rorke
// Created: 3/12/01
// Modification History:
//
//-------------------------------------------------------------
#ifndef __MODELMESH_H__
#define __MODELMESH_H__

#include <afxmt.h>			//for CCriticalSection
#include "ltbasedefs.h"		//for the int types and LTVector

#ifndef __MODELHANDLE_H__
#	include "modelhandle.h"
#endif

//forward declarations
class CMeshShapeList;

class CModelMesh
{
public:

	CModelMesh(const char* pszFilename, const CModelHandle& Handle);
	~CModelMesh();

	//determines if the shapes associated with this mesh are in memory
	//or need to be loaded from the disk again
	bool			AreShapesLoaded() const;

	//gets the size that this current model's shapes are occupying
	uint32			GetMemoryFootprint() const;

	//causes all the shapes to be discarded from memory
	bool			UnloadShapes();

	//gets the filename associated with this model
	const char*		GetFilename() const;

	//gets the user dimensions of this model
	LTVector		GetDimensions() const;

	//gets the bounding box of the model
	bool			GetBoundingBox(LTVector& vMin, LTVector& vMax);

	//determines if this object is valid
	bool			IsValid() const;

	//gets the handle associated with this model
	CModelHandle	GetHandle() const;

	//sets the handle of this object
	void			SetHandle(const CModelHandle& Handle);

	//gets the shape list associated with the model
	CMeshShapeList*	GetShapeList();

	//add a reference to the model
	void			AddReference();

	//rease a reference to the model
	void			ReleaseReference();

	//get the number of references to the model
	uint32			GetNumReferences() const;

	//add a reference to the shape list
	void			AddShapeListReference();
	
	//release a reference to the shape list
	void			ReleaseShapeListReference();

	//gets the number of references to the shape list
	uint32			GetNumShapeListReferences() const;

	//threadsafe way of setting the loaded flag
	void			SetShapeList(CMeshShapeList* pShapeList);

private:

	//sets the filename that this model represents. Will return false if it
	//cannot find the file or get the appropriate data from it
	bool			SetFilename(const char* pszFilename);

	//opens up the file and attempts to find the user dimensions of this model
	bool			LoadDimensions(const char* pszFilename);

	//resets it to an initial state
	void			Free();



	//the user dimensions
	LTVector		m_vDimensions;

	//the bounding box of the model
	LTVector		m_vBoundingMin;
	LTVector		m_vBoundingMax;

	//flag used to indicate if this object is valid or not
	bool			m_bValid;

	//the handle for this model
	CModelHandle	m_Handle;

	//the internal mesh list. This is NULL when no data is loaded
	CMeshShapeList*	m_pShapeList;

	//the filename of this mesh
	char			m_pszFilename[MAX_PATH];

	//the number of objects that use this model
	uint32			m_nObjectRefCount;

	//the number of references to the shape list
	uint32			m_nShapeListRefCount;

	//critical sections for controlling access to thread safe members
	CCriticalSection	m_CSShapeList;

};


#endif


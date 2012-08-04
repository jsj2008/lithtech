//--------------------------------------------------------
// ModelMgr.h
//
// Handles management of models inside of DEdit. This 
// allows for tracking of models, and intelligent memory
// management and referencing.
//
// Author: John O'Rorke
// Created: 3/12/01
// Modification History:
//
//--------------------------------------------------------

#ifndef __MODELMGR_H__
#define __MODELMGR_H__

#ifndef __MODELMESH_H__
#	include "modelmesh.h"
#endif

//the message that is sent once a model has been loaded
#define WM_USER_MODEL_LOADED		(0x400 + 0x100)		//WM_USER + some offset

//forward declarations
class CModelLink;

class CModelMgr
{
public:

	//default memory usage in bytes (currently 8 meg)
	enum	{	DEFAULT_MAX_MEMORY_USAGE	= 8 * 1024 * 1024	};

	CModelMgr();
	CModelMgr(bool bLowPriorityLoading, uint32 nMaxMemoryUsage = DEFAULT_MAX_MEMORY_USAGE);
	~CModelMgr();

	//creates a model manager
	bool	Create(bool bLowPriorityLoading, uint32 nMaxMemoryUsage = DEFAULT_MAX_MEMORY_USAGE);

	//adds a model into the list to be managed. This returns
	//the success. If it fails, the handle is undefined
	bool	AddModel(const char* pszFilename, CModelHandle& Handle);

	//releases a reference to the model so that it can be reclaimed
	//if needed
	bool	ReleaseHandle(const CModelHandle& Handle);

	//gets the filename of the specified model
	const char* GetFilename(const CModelHandle& Handle);

	//gets the dimensions of a model
	bool	GetDimensions(const CModelHandle& Handle, LTVector& vDims);

	//gets a bounding box for a specified model
	bool	GetBoundingBox(const CModelHandle& Handle, LTVector& vMin, LTVector& vMax);

	//gets the shape list of the model. 
	bool	GetShapeList(const CModelHandle& Handle, CMeshShapeList** pShapeList);

	//releases a reference to the shape list
	bool	ReleaseShapeList(const CModelHandle& Handle);

	//sets the maximum memory usage. Will try and flush down to that limit, and
	//returns the success of that operation
	bool	SetMaxMemoryUsage(uint32 nMaxMemUsage);

	//gets the current memory usage
	uint32	GetCurrentMemoryUsage() const;

	//will attempt to shrink the memory footprint to the specified size
	bool	ReclaimMemory(uint32 nReclaimTo);

	//determines if this manager is valid or not (has been created)
	bool	IsValid() const;

	//THREAD FUNCTIONS-------------

	//retreives the next job for loading
	bool			GetNextThreadJob(char* pszFilename, uint32 nBufferLen);

	//finishes a thread job, and attaches the shape list to the specified mesh
	bool			FinishThreadJob(CMeshShapeList* pShapes, const char* pszFilename);

	//specifies that the object is trying to shut down
	bool			IsShuttingDown();

private:

	//creates a new handle
	CModelHandle	GenerateNewHandle();

	//creates the loading thread
	bool			CreateLoadingThread(bool bLowPriorityLoading);

	//destroys the loading thread, waiting for it to shut down
	bool			DestroyLoadingThread();

	//specifies that this object is trying to shut down
	void			SetShuttingDown(bool bVal);

	//frees all memory associated with this manager
	void	Free();

	//converts a handle to a model
	CModelMesh*		HandleToModel(const CModelHandle& Handle);

	//converts a handle to a model link
	CModelLink*		HandleToModelLink(const CModelHandle& Handle);

	//remove the specified model from the list of models to be loaded
	bool			RemoveModelFromJobQueue(CModelMesh* pModel);

	//queues up a model to be loaded
	bool			QueueModelToLoad(CModelMesh* pModel);

	//deletes a link and its model, will also clean it up from the loading list
	bool			DeleteModelAndLink(CModelLink* pModel);

	//the list of objects
	CModelLink*			m_pModelList;

	//maximum memory to use
	uint32				m_nMaxMemoryUsage;

	//the list of objects being loaded
	CModelLink*			m_pJobQueue;

	//if we are shutting down
	bool				m_bShuttingDown;

	//determines if this is valid
	bool				m_bIsValid;

	//the next model handle
	uint32				m_nNextModelHandle;

	//the loading thread handle
	HANDLE				m_hThread;

	//critical sections for thread synchronization
	CCriticalSection	m_CSJobQueue;
	CCriticalSection	m_CSShuttingDown;

};

#endif


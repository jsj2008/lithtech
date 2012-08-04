//turn off the stl warnings
#pragma warning (disable:4786)

#include "modelmgr.h"
#include "meshshapelist.h"
#include "meshshape.h"
#include "ltascene.h"
#include "stdafx.h"
#include "bdefs.h"
#include "edithelpers.h"
#include "optionsmodels.h"
#include "mainfrm.h"

#undef DWORD

//-------------------------------------------
// CModelLink
//
// Doubly linked list node for holding a model
class CModelLink
{
public:

	CModelLink(CModelMesh* pModel, CModelLink* pLeft = NULL, CModelLink* pRight = NULL) : 
		m_pModel(pModel), m_pLeft(pLeft), m_pRight(pRight)		
	{
	}

	~CModelLink()
	{
		Detach();
	}

	//detaches this link from its surroundings
	void Detach()
	{
		if(m_pLeft)
			m_pLeft->m_pRight = m_pRight;
		if(m_pRight)
			m_pRight->m_pLeft = m_pLeft;

		m_pRight = NULL;
		m_pLeft  = NULL;
	}

	//adds it into the list after a certain point
	CModelLink* GetTail()
	{
		if(m_pRight == NULL)
			return this;

		return m_pRight->GetTail();
	}


	//the model it holds
	CModelMesh*		m_pModel;

	//links in the list
	CModelLink*		m_pLeft;
	CModelLink*		m_pRight;
};


//------------------------
// Model Loading Thread
//------------------------

//given a mesh that has the positions and triangles set up, it will calculate
//the normal of a specified triangle
static LTVector TriangleNormal(CMeshShape* pMesh, uint32 nTriangle)
{
	ASSERT(pMesh);

	//get the vertex list
	LTVector* pVerts = pMesh->GetOriginalPosList();

	//get the triangle indices list
	uint16*	  pTris = pMesh->GetTriangleList();

	//create the two edge vectors
	LTVector vE1 = pVerts[pTris[nTriangle * 3 + 0]] - pVerts[pTris[nTriangle * 3 + 1]];
	LTVector vE2 = pVerts[pTris[nTriangle * 3 + 0]] - pVerts[pTris[nTriangle * 3 + 2]];

	LTVector vNormal = vE2.Cross(vE1);

	float fMag = vNormal.Mag();

	if(fMag < 0.001f)
	{
		return LTVector(0, 0, 0);
	}

	vNormal /= fMag;

	return vNormal;
}

CMeshShape* ShapeToMesh(Shape* pShape)
{
	if(pShape == NULL)
	{
		ASSERT(false);
		return NULL;
	}

	//first off, try and allocate memory for the mesh
	CMeshShape* pMesh = new CMeshShape;

	if(pMesh == NULL)
	{
		return NULL;
	}

	//now convert the shape over
	CLTAMesh& Source = pShape->GetMesh();

	if(	(pMesh->SetNumTriangles(Source.tri_FaceSet.size() / 3) == false) ||
		(pMesh->SetNumVertices(Source.tri_FaceSet.size()) == false))
	{
		delete pMesh;
		return NULL;
	}

	//do a quick sanity check
	ASSERT(Source.tri_FaceSet.size() == Source.tri_UVSet.size());
	ASSERT(Source.tri_FaceSet.size() % 3 == 0);
	
	//now set up all the verts
	uint32 nCurrVert;
	for(nCurrVert = 0; nCurrVert < Source.tri_FaceSet.size(); nCurrVert++)
	{
		//first set up the position info
		uint32 nPosVert = Source.tri_FaceSet[nCurrVert];
		pMesh->SetVertexPos(nCurrVert, Source.vertex[nPosVert]);

		//now set up the texture info
		uint32 nUVVert = Source.tri_UVSet[nCurrVert];
		pMesh->SetVertexUV(nCurrVert, Source.tcoords[nUVVert].x, Source.tcoords[nUVVert].y);
	}

	//now set up all the triangles
	for(uint32 nCurrTri = 0; nCurrTri < Source.tri_FaceSet.size() / 3; nCurrTri++)
	{
		pMesh->SetTriangleIndices(nCurrTri, nCurrTri * 3 + 0,
											nCurrTri * 3 + 1,
											nCurrTri * 3 + 2);
	}

	//now that we have the triangles and vertices set up, we need to set up the normals
	//(this comes last because it may have to generate them, and that relies on the other
	//two parts)
	for(nCurrVert = 0; nCurrVert < Source.tri_FaceSet.size(); nCurrVert++)
	{
		//find our index
		uint32 nPosVert = Source.tri_FaceSet[nCurrVert];

		//see if we can just steal the normal
		if(nPosVert < Source.normals.size())
		{
			//we can, no need to generate
			pMesh->SetVertexNormal(nCurrVert, Source.normals[nPosVert]);
		}
		else
		{
			LTVector vNormal(0, 0, 0);

			//ugh. Slow part. We need to run through the polies and see if the
			//triangle refers to this point
			uint16* pTriList = pMesh->GetTriangleList();

			//run through all the vertices and add all the components that contribute
			for(uint32 nCurrIndex = 0; nCurrIndex < pMesh->GetNumTriangles() * 3; nCurrIndex++)
			{
				//see if this refers to the point
				if(nPosVert == Source.tri_FaceSet[pTriList[nCurrIndex]])
				{
					//it refers to it, so let us find the normal of that triangle and add it
					vNormal += TriangleNormal(pMesh, nCurrIndex / 3);
				}
			}

			//normalize the normal
			vNormal.Norm();

			//set this normal
			pMesh->SetVertexNormal(nCurrVert, vNormal);
		}
	}

	return pMesh;
}

//function that will load the shape from the LTA file. Located in ltaScene.cpp
void ParseShapeNode( Shape *shape, CLTANode *pnroot );

#define	MAX_MESHES_PER_MODEL		512

CMeshShapeList*	LoadShapeList(const char* pszFilename, CModelMgr* pModelMgr)
{
	//the list of meshes that we have created
	CMeshShape*	pMeshList[MAX_MESHES_PER_MODEL];
	uint32		nMeshCount = 0;

	//the list of textures and their names
	uint32		*pTexIDList		= NULL;
	const char	**pTexFileList	= NULL;

	//the number of items in the texture list
	uint32		nNumTextures	= 0;

	//open up the file first off
	CLTAReader InFile;

	if(InFile.Open(pszFilename, CLTAUtil::IsFileCompressed(pszFilename)) == false)
	{
		return NULL;
	}

	//first off we need to get the list of textures that map to the pieces
	CLTALoadOnlyAlloc Allocator(1024 * 256);

	CLTANode* pTextureList = CLTANodeReader::LoadNode(&InFile, "texture-bindings", &Allocator);

	//now we need to load in the list of texture bindings
	if(pTextureList && (pTextureList->GetNumElements() >= 2) && pTextureList->GetElement(1)->IsList())
	{
		//get the actual list
		CLTANode* pActualList = pTextureList->GetElement(1);

		//allocate the buffers for the actual list
		pTexIDList		= new uint32 [pActualList->GetNumElements()];
		pTexFileList	= new const char* [pActualList->GetNumElements()];

		if(pTexIDList && pTexFileList)
		{

			for(uint32 nCurrTex = 0; nCurrTex < pActualList->GetNumElements(); nCurrTex++)
			{
				if(	pActualList->GetElement(nCurrTex)->IsList() && 
					(pActualList->GetElement(nCurrTex)->GetNumElements() >= 2))
				{
					CLTANode* pTexture = pActualList->GetElement(nCurrTex);
					pTexIDList  [nNumTextures] = atoi(pTexture->GetElement(0)->GetValue());
					pTexFileList[nNumTextures] = pTexture->GetElement(1)->GetValue();
					nNumTextures++;
				}
			}
		}
	}

	//Note: Don't clean up the texture node until the end. The values are still needed
	//to get the texture names

	//we have loaded in the list of textures. We need to reset the file now
	InFile.Close();
	if(InFile.Open(pszFilename, CLTAUtil::IsFileCompressed(pszFilename)) == false)
	{
		delete [] pTexIDList;
		delete [] pTexFileList;
		delete pTextureList;
		return NULL;
	}

	//now create the mesh list
	CMeshShapeList* pList = new CMeshShapeList;

	if(pList == NULL)
	{
		delete [] pTexIDList;
		delete [] pTexFileList;
		Allocator.FreeNode(pTextureList);
		return NULL;
	}


	//now we need to find all the shapes in the file
	do
	{
		//we should also check between loading shapes if the manager
		//is trying to shut down. This will give a quicker response time
		//on close
		if(pModelMgr->IsShuttingDown())
		{
			//clean up all the meshes we have loaded so far
			for(uint32 nCurrMesh = 0; nCurrMesh < nMeshCount; nCurrMesh++)
				delete pMeshList[nCurrMesh];

			//clean up the list as well
			delete pList;

			delete [] pTexIDList;
			delete [] pTexFileList;

			Allocator.FreeNode(pTextureList);

			//we failed
			return NULL;
		}


		CLTANode* pShape = CLTANodeReader::LoadNode(&InFile, "shape", &Allocator);

		//see if we found a shape
		if(pShape == NULL)
		{
			break;
		}

		//now we need to convert this shape 
		Shape CurrShape;

		ParseShapeNode(&CurrShape, pShape);
		//delete pShape;

		//convert the shape to our format
		pMeshList[nMeshCount] = ShapeToMesh(&CurrShape);

		// get the renderstyle from shape (for texture index);
		CLTARenderStyle *pRS = (CLTARenderStyle*)CurrShape.GetAppearance();

		//now we need to get the texture information for this mesh
		for(uint32 nCurrTex = 0; nCurrTex < nNumTextures; nCurrTex++)
		{
			if(pRS->m_iTextures[0] == pTexIDList[nCurrTex])
			{
				pMeshList[nMeshCount]->SetTextureFilename(pTexFileList[nCurrTex]);
				break;
			}
		}

		nMeshCount++;

	}while(nMeshCount < MAX_MESHES_PER_MODEL);

	//resize the list
	pList->SetNumShapes(nMeshCount);

	//now set up the list with the shapes we loaded
	for(uint32 nCurrMesh = 0; nCurrMesh < nMeshCount; nCurrMesh++)
	{
		pList->SetShape(nCurrMesh, pMeshList[nCurrMesh]);
	}

	//clean up the lists
	delete [] pTexIDList;
	delete [] pTexFileList;

	//clean up the memory associated with the nodes
	Allocator.FreeNode(pTextureList);
	Allocator.FreeAllMemory();

	return pList;
}

#define MODEL_MGR_THREAD_DELAY		10

DWORD WINAPI ModelMgrThreadMain(void* pParam)
{
	ASSERT(pParam);

	char pszFilename[MAX_PATH];

	CModelMgr* pModelMgr = (CModelMgr*)pParam;

	//just continue as long as the manager isn't trying to shut the
	//thread down
	while(pModelMgr->IsShuttingDown() == false)
	{
		if(pModelMgr->GetNextThreadJob(pszFilename, MAX_PATH))
		{
			//now we actually need to load the shape list
			CMeshShapeList* pShapeList = LoadShapeList(pszFilename, pModelMgr);

			if(pShapeList)
			{
				pModelMgr->FinishThreadJob(pShapeList, pszFilename);
			}
		}

		//still don't have a job, wait a while
		Sleep(MODEL_MGR_THREAD_DELAY);		
	}

	return 0;
}



//------------------------
// CModelMgr
//------------------------
CModelMgr::CModelMgr() :
	m_pModelList(NULL),
	m_nMaxMemoryUsage(DEFAULT_MAX_MEMORY_USAGE),
	m_bShuttingDown(false),
	m_hThread(NULL),
	m_pJobQueue(NULL),
	m_nNextModelHandle(0),
	m_bIsValid(false)
{
}

CModelMgr::CModelMgr(bool bLowPriorityLoading, uint32 nMaxMemoryUsage) :
	m_pModelList(NULL),
	m_nMaxMemoryUsage(nMaxMemoryUsage),
	m_bShuttingDown(false),
	m_hThread(NULL),
	m_pJobQueue(NULL),
	m_nNextModelHandle(0),
	m_bIsValid(false)
{
	Create(bLowPriorityLoading, nMaxMemoryUsage);
}

CModelMgr::~CModelMgr()
{
	DestroyLoadingThread();
	Free();
}

//creates a new handle
CModelHandle CModelMgr::GenerateNewHandle()
{
	CModelHandle rv;
	
	do
	{
		rv.m_nHandle = m_nNextModelHandle++;
	}
	while(!rv.IsValid());

	return rv;
}


//adds a model into the list to be managed. This returns
//the success. If it fails, the handle is undefined
bool CModelMgr::AddModel(const char* pszFilename, CModelHandle& Handle)
{
	if(IsValid() == false)
	{
		return false;
	}

	//see if this model is already registered
	CModelLink* pCurr = m_pModelList;

	while(pCurr)
	{
		ASSERT(pCurr->m_pModel);

		if(stricmp(pszFilename, pCurr->m_pModel->GetFilename()) == 0)
		{
			//found a match.
			pCurr->m_pModel->AddReference();
			Handle = pCurr->m_pModel->GetHandle();

			return true;
		}
		pCurr = pCurr->m_pRight;
	}

	//no match, so we need to make a new one

	//get the new handle
	CModelHandle NewHandle = GenerateNewHandle();

	//first allocate the model
	CModelMesh* pNewModel = new CModelMesh(pszFilename, NewHandle);

	//check the allocation
	if(pNewModel == NULL)
	{
		return false;
	}

	//check the object itself
	if(pNewModel->IsValid() == false)
	{
		delete pNewModel;
		return false;
	}

	//allocate the link for it
	CModelLink* pNewLink = new CModelLink(pNewModel, NULL, m_pModelList);

	//check the allocation
	if(pNewLink == NULL)
	{
		delete pNewModel;
		return false;
	}

	if(m_pModelList)
		m_pModelList->m_pLeft = pNewLink;

	m_pModelList = pNewLink;
	
	//add the reference to the model
	pNewModel->AddReference();

	//give the callee a handle
	Handle = NewHandle;

	//success
	return true;
}

//releases a reference to the model so that it can be reclaimed
//if needed
bool CModelMgr::ReleaseHandle(const CModelHandle& Handle)
{
	if(IsValid() == false)
	{
		return false;
	}

	if(!Handle.IsValid())
		return false;

	CModelLink* pLink = HandleToModelLink(Handle);

	if(pLink)
	{
		ASSERT(pLink->m_pModel);
		pLink->m_pModel->ReleaseReference();

		//we released the reference, so see if anyone refers to it anymore. If not,
		//we should chuck the model
		if(pLink->m_pModel->GetNumReferences() == 0)
		{
			//update our list holders
			if(m_pModelList == pLink)
				m_pModelList = pLink->m_pRight;

			pLink->Detach();

			DeleteModelAndLink(pLink);
		}

		//success
		return true;
	}

	//couldn't find it. Invalid handle
	return false;	
}

//gets a bounding box for a specified model
bool CModelMgr::GetBoundingBox(const CModelHandle& Handle, LTVector& vMin, LTVector& vMax)
{
	if(!Handle.IsValid())
		return false;

	//find the model
	CModelMesh* pModel = HandleToModel(Handle);

	if(pModel)
	{
		pModel->GetBoundingBox(vMin, vMax);
		return true;
	}

	return false;
}

//gets the dimensions of a model
bool CModelMgr::GetDimensions(const CModelHandle& Handle, LTVector& vDims)
{
	if(!Handle.IsValid())
		return false;

	//find the model
	CModelMesh* pModel = HandleToModel(Handle);

	if(pModel)
	{
		vDims = pModel->GetDimensions();
		return true;
	}

	return false;
}

//gets the shape list of the model. 
bool CModelMgr::GetShapeList(const CModelHandle& Handle, CMeshShapeList** ppShapeList)
{
	if(!Handle.IsValid())
		return false;

	ASSERT(ppShapeList);

	//get the model
	CModelLink* pLink = HandleToModelLink(Handle);

	if(pLink == NULL)
	{
		return false;
	}

	CModelMesh* pModel = pLink->m_pModel;
	ASSERT(pModel);

	//see if the shapes are loaded
	if(pModel->AreShapesLoaded() == false)
	{
		//the shapes aren't loaded, we need to queue them to be
		QueueModelToLoad(pModel);
		return false;
	}

	//add a reference to the shape
	*ppShapeList = pModel->GetShapeList();

	pModel->AddShapeListReference();

	//we don't need to worry about moving it to the front if it already is
	if(m_pModelList != pLink)
	{
		//now move this item to the front of the list
		pLink->Detach();

		if(m_pModelList)
			m_pModelList->m_pLeft = pLink;

		pLink->m_pRight = m_pModelList;
		m_pModelList = pLink;
	}

	return true;
}

//releases a reference to the shape list
bool CModelMgr::ReleaseShapeList(const CModelHandle& Handle)
{
	if(!Handle.IsValid())
		return false;

	//get the model
	CModelMesh* pModel = HandleToModel(Handle);

	//remove the reference
	if(pModel)
	{
		pModel->ReleaseShapeListReference();
		return true;
	}

	return false;
}

//sets the maximum memory usage. Will try and flush down to that limit, and
//returns the success of that operation
bool CModelMgr::SetMaxMemoryUsage(uint32 nMaxMemUsage)
{
	m_nMaxMemoryUsage = nMaxMemUsage;

	return ReclaimMemory(nMaxMemUsage);
}

//will attempt to shrink the memory footprint to the specified size
bool CModelMgr::ReclaimMemory(uint32 nReclaimTo)
{
	if(m_pModelList == NULL)
		return true;

	//start at the end
	CModelLink* pCurr = m_pModelList->GetTail();

	int32 nCurrMemory = (int32)GetCurrentMemoryUsage();

	//work the way back freeing what memory we can until we either hit
	//the memory threshold size, or the end of the list
	while(pCurr)
	{
		CModelMesh* pModel = pCurr->m_pModel;

		ASSERT(pModel);

		//see if anyone has the shapes locked, or if there are any shapes associated
		if(	pModel->AreShapesLoaded() && 
			(pModel->GetNumShapeListReferences() == 0))
		{
			//we can free this one
			uint32 nSize = pModel->GetMemoryFootprint();
			pModel->UnloadShapes();

			nCurrMemory -= (int32)nSize;

			//see if we have free'd enough memory
			if(nCurrMemory < nReclaimTo)
			{
				return true;
			}
		}

		pCurr = pCurr->m_pLeft;
	}

	//didn't free enough memory
	return false;
}

//gets the current memory usage
uint32 CModelMgr::GetCurrentMemoryUsage() const
{
	CModelLink* pCurr = m_pModelList;

	//add up the size
	uint32 nMemUsed = 0;

	while(pCurr)
	{
		ASSERT(pCurr->m_pModel);

		nMemUsed += pCurr->m_pModel->GetMemoryFootprint();

		pCurr = pCurr->m_pRight;
	}

	return nMemUsed;
}


//frees all memory associated with this manager
void CModelMgr::Free()
{
	CModelLink* pCurr = m_pModelList;
	CModelLink* pNext = NULL;

	while(pCurr)
	{
		pNext = pCurr->m_pRight;
		DeleteModelAndLink(pCurr);
		pCurr = pNext;
	}

	m_pModelList	= NULL;
	m_bIsValid		= false;
}

//converts a handle to a model
CModelMesh* CModelMgr::HandleToModel(const CModelHandle& Handle)
{
	CModelLink* pLink = HandleToModelLink(Handle);

	if(pLink)
	{
		return pLink->m_pModel;
	}

	return NULL;
}

//converts a handle to a model link
CModelLink* CModelMgr::HandleToModelLink(const CModelHandle& Handle)
{
	if(IsValid() == false)
	{
		return NULL;
	}

	CModelLink* pCurr = m_pModelList;

	while(pCurr)
	{
		ASSERT(pCurr->m_pModel);

		if(pCurr->m_pModel->GetHandle() == Handle)
		{
			return pCurr;
		}

		pCurr = pCurr->m_pRight;
	}

	//nothing found
	return NULL;
}

//queues up a model to be loaded
bool CModelMgr::QueueModelToLoad(CModelMesh* pModel)
{
	//first off, see if this model is already queued
	m_CSJobQueue.Lock();
	CModelLink* pCurr = m_pJobQueue;

	while(pCurr)
	{
		if(pCurr->m_pModel == pModel)
		{
			//already in the list
			m_CSJobQueue.Unlock();
			return true;
		}
		pCurr = pCurr->m_pRight;
	}
	m_CSJobQueue.Unlock();

	CModelLink* pNewLink = new CModelLink(pModel, NULL, m_pJobQueue);

	if(pNewLink == NULL)
	{
		return false;
	}

	m_CSJobQueue.Lock();

	if(m_pJobQueue)
		m_pJobQueue->m_pLeft = pNewLink;

	m_pJobQueue = pNewLink;

	m_CSJobQueue.Unlock();

	return true;
	
}

//remove the specified model from the list of models to be loaded
bool CModelMgr::RemoveModelFromJobQueue(CModelMesh* pModel)
{
	m_CSJobQueue.Lock();

	bool bFound = false;

	CModelLink* pCurr = m_pJobQueue;

	while(pCurr)
	{
		ASSERT(pCurr->m_pModel);

		if(pCurr->m_pModel == pModel)
		{
			if(pCurr == m_pJobQueue)
				m_pJobQueue = pCurr->m_pRight;

			delete pCurr;
			bFound = true;
			break;
		}

		pCurr = pCurr->m_pRight;
	}

	m_CSJobQueue.Unlock();

	return bFound;
}

//creates the loading thread
bool CModelMgr::CreateLoadingThread(bool bLowPriorityLoading)
{
	//destroy the old thread
	DestroyLoadingThread();

	DWORD nThreadID;
	m_hThread = CreateThread(NULL, 0, ModelMgrThreadMain, (void*)this, 0, &nThreadID);

	if(m_hThread == NULL)
	{
		return false;
	}

	//set the thread priority to somewhat low to prevent users from not being able
	//to work
	if(bLowPriorityLoading)
	{
		SetThreadPriority(m_hThread, THREAD_PRIORITY_BELOW_NORMAL);
	}


	return true;
}

//destroys the loading thread, waiting for it to shut down
bool CModelMgr::DestroyLoadingThread()
{
	if(m_hThread == NULL)
	{
		return true;
	}

	//set the terminate flag
	SetShuttingDown(true);

	//wait for the thread to die
	DWORD nStatus;

	while(1)
	{
		GetExitCodeThread(m_hThread, &nStatus);

		if(nStatus != STILL_ACTIVE)
			break;

		Sleep(MODEL_MGR_THREAD_DELAY);
	}

	SetShuttingDown(false);

	return true;	
}

//deletes a link and its model, will also clean it up from the loading list
bool CModelMgr::DeleteModelAndLink(CModelLink* pLink)
{
	ASSERT(pLink);

	CModelMesh* pModel = pLink->m_pModel;

	//remove it from the loading queue
	RemoveModelFromJobQueue(pModel);

	//now remove it from the normal model list

	//first off remove it from the list
	if(m_pModelList == pLink)
		m_pModelList = pLink->m_pRight;
	delete pLink;

	//now delete the model
	delete pModel;

	return true;
}

//gets the filename of the specified model
const char* CModelMgr::GetFilename(const CModelHandle& Handle)
{
	CModelMesh* pModel = HandleToModel(Handle);

	if(pModel)
	{
		return pModel->GetFilename();
	}

	return NULL;
}

//creates a model manager
bool CModelMgr::Create(bool bLowPriorityLoading, uint32 nMaxMemoryUsage)
{
	Free();

	m_bIsValid = CreateLoadingThread(bLowPriorityLoading);	

	return IsValid();
}


//determines if this manager is valid
bool CModelMgr::IsValid() const
{
	return m_bIsValid;
}

//THREAD FUNCTIONS-------------

//retreives the next job for loading
bool CModelMgr::GetNextThreadJob(char* pszBuffer, uint32 nBufferLen)
{
	m_CSJobQueue.Lock();

	//get the last item in the list
	CModelMesh* pModel = NULL;

	if(m_pJobQueue)
	{
		pModel = (m_pJobQueue->GetTail())->m_pModel;
	}

	m_CSJobQueue.Unlock();

	if(pModel)
	{
		strncpy(pszBuffer, pModel->GetFilename(), nBufferLen);
		return true;
	}

	return false;
}

//finishes a thread job, and attaches the shape list to the specified mesh
bool CModelMgr::FinishThreadJob(CMeshShapeList* pShapes, const char* pszFilename)
{
	m_CSJobQueue.Lock();

	bool bFound = false;

	CModelLink* pCurr = m_pJobQueue;
	CModelMesh* pMesh;

	while(pCurr)
	{
		ASSERT(pCurr->m_pModel);

		if(stricmp(pCurr->m_pModel->GetFilename(), pszFilename) == 0)
		{
			if(pCurr == m_pJobQueue)
				m_pJobQueue = pCurr->m_pRight;

			//save this shape list
			pMesh = pCurr->m_pModel;
			pMesh->SetShapeList(pShapes);

			//remove the node
			delete pCurr;
			bFound = true;
			break;
		}

		pCurr = pCurr->m_pRight;
	}

	m_CSJobQueue.Unlock();

	//if it wasn't found, we need to free the memory
	if(bFound == false)
	{
		delete pShapes;
	}
	else
	{
		//the model has been loaded, we need to get the main window to update
		PostMessage(GetMainFrame()->m_hWnd, WM_USER_MODEL_LOADED, 0, (LPARAM)pMesh);
	}

	return bFound;
}

//specifies that the object is trying to shut down
bool CModelMgr::IsShuttingDown()
{
	m_CSShuttingDown.Lock();
	bool bRV = m_bShuttingDown;
	m_CSShuttingDown.Unlock();

	return bRV;
}

//specifies that this object is trying to shut down
void CModelMgr::SetShuttingDown(bool bVal)
{
	m_CSShuttingDown.Lock();
	m_bShuttingDown = bVal;
	m_CSShuttingDown.Unlock();
}



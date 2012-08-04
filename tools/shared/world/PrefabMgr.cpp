//------------------------------------------------------
// PrefabMgr.cpp
//
// Prefab management class implementation
//
// Author: Kevin Francis
// Created: 03/15/2001
// Modification History: 
//		03/15/2001 - First implementation
//
//------------------------------------------------------


#include "bdefs.h"

#include "prefabmgr.h"
#include "prefabref.h"
#include "editregion.h"
#include "node_ops.h"
#include "geomroutines.h"
#include <float.h>	//for FLT_MAX and FLT_MIN
#ifdef DIRECTEDITOR_BUILD
#include "edithelpers.h"
#endif

//////////////////////////////////////////////////////////////////////////////
// CLoadedPrefab -  Holder class for a prefab that has been loaded

class CLoadedPrefab
{
public:
	CLoadedPrefab();
	~CLoadedPrefab();
	// Load a file into the prefab
	bool		Load(const char *pFilename, CPrefabMgr* pMgr);
	// Calculate the dims of the prefab
	void		CalcDims();

	CString		m_sName;
	CEditRegion m_cRegion;

	//the minimum and maximum extents of a prefab specified in the prefab world's space
	LTVector	m_vMin;
	LTVector	m_vMax;
};

CLoadedPrefab::CLoadedPrefab()
{
}

CLoadedPrefab::~CLoadedPrefab() 
{ 
}

static void RecurseAndGrowDims(CWorldNode *pNode, LTVector &vMin, LTVector &vMax, LTMatrix& mTransMat)
{
	//sanity check
	if(pNode == NULL)
	{
		return;
	}

	// Grow the dims for this node
	switch (pNode->GetType())
	{
		case Node_Object :
		{
			CBaseEditObj *pObject = pNode->AsObject();
			LTVector vCenter;
			mTransMat.Apply(pObject->GetPos(), vCenter);

			//always include at least the object center
			VEC_MIN(vMin, vMin, vCenter);
			VEC_MAX(vMax, vMax, vCenter);

#ifdef DIRECTEDITOR_BUILD			
			
			//see if there are other dims we need
			for (uint32 nCurDim = pObject->GetNumDims(); nCurDim > 0; --nCurDim)
			{
				LTVector vDims = *pObject->GetDim(nCurDim - 1);
				LTVector vObjMin = vCenter - vDims;
				LTVector vObjMax = vCenter + vDims;
				VEC_MIN(vMin, vMin, vObjMin);
				VEC_MAX(vMax, vMax, vObjMax);
			}
#endif
		}
		break;
		case Node_Brush:
		{
			CEditBrush *pBrush = pNode->AsBrush();
			
			CBoundingBox BBox = pBrush->CalcBoundingBox();

			//transform the bounding box
			LTVector vBoxMin, vBoxMax;
			mTransMat.Apply(BBox.m_Min, vBoxMin);
			mTransMat.Apply(BBox.m_Max, vBoxMax);

			VEC_MIN(vMin, vMin, vBoxMin);
			VEC_MAX(vMax, vMax, vBoxMax);
		}
		break;
		case Node_PrefabRef:
		{
			//create the new transformation matrix
			LTMatrix mRot;
			::gr_SetupMatrixEuler(pNode->GetOr(), mRot.m);

			LTMatrix mTranslate;
			mTranslate.Identity();
			mTranslate.SetTranslation(pNode->GetPos());

			LTMatrix mNewTransMat = mTransMat * mTranslate * mRot;
			RecurseAndGrowDims((CWorldNode*)((CPrefabRef*)pNode)->GetPrefabTree(), vMin, vMax, mNewTransMat);
		}
		break;
	}

	// Go through the children
	GPOS iFinger = pNode->m_Children.GetHeadPosition();
	while (iFinger)
	{
		CWorldNode *pChild = pNode->m_Children.GetNext(iFinger);
		RecurseAndGrowDims(pChild, vMin, vMax, mTransMat);
	}
}

void CLoadedPrefab::CalcDims()
{
	LTVector vMin, vMax;
	vMin.Init(FLT_MAX, FLT_MAX, FLT_MAX);
	vMax.Init(FLT_MIN, FLT_MIN, FLT_MIN);

	LTMatrix mIdent;
	mIdent.Identity();
	RecurseAndGrowDims(m_cRegion.GetRootNode(), vMin, vMax, mIdent);

	// If one of the members didn't get modified, none of them did...
	if (vMin.x == FLT_MAX)
	{
		ASSERT(vMin.y == FLT_MAX && vMin.z == FLT_MAX && 
			vMax.x == FLT_MIN && vMax.y == FLT_MIN && vMax.y == FLT_MIN);
		// Treat it as an empty set (Which it probably is...  All null nodes or something..)
		vMin.Init();
		vMax.Init();
	}

	m_vMin = vMin;
	m_vMax = vMax;
}

bool SwapLTALTC(CString &sFilename)
{
	// If it's too short, it's not an option
	if (sFilename.GetLength() < 4)
		return false;

	// Point at the end of the string
	char *pBuffer = sFilename.LockBuffer();
	char *pLastChar = &pBuffer[sFilename.GetLength() - 1];

	bool bResult = true;

	// Make sure it's LT? for the extension
	if ((pLastChar[-3] != '.') ||
		(toupper(pLastChar[-2]) != 'L') ||
		(toupper(pLastChar[-1]) != 'T'))
	{
		bResult = false;
	}
	// Switch it
	else if (toupper(*pLastChar) == 'A')
		*pLastChar = 'C';
	else if (toupper(*pLastChar) == 'C')
		*pLastChar = 'A';
	else
		bResult = false;

	sFilename.UnlockBuffer();
	return bResult;
}

bool CLoadedPrefab::Load(const char *pFilename, CPrefabMgr* pMgr)
{
	CString sFilename = pMgr->GetRootPath();
	if(sFilename.GetLength() && (sFilename[sFilename.GetLength() - 1] != '\\'))
		sFilename += '\\';
	sFilename += pFilename;

	uint32 nFileVersion;

#ifdef DIRECTEDITOR_BUILD

	bool bBinary = false;
	if (m_cRegion.LoadFile(sFilename, GetProject(), nFileVersion, bBinary) != REGIONLOAD_OK)
	{
		if (bBinary || !SwapLTALTC(sFilename))
			return LTFALSE;
		if (m_cRegion.LoadLTA(sFilename, GetProject(), nFileVersion) != REGIONLOAD_OK)
			return LTFALSE;
	}

	// Update all the texture IDs and polygon pieces.
	m_cRegion.UpdateTextureIDs( GetProject() );
#else
	bool bBinary = false;
	if (m_cRegion.LoadFile(sFilename, NULL, nFileVersion, bBinary) != REGIONLOAD_OK)
	{
		if (bBinary || !SwapLTALTC(sFilename))
			return LTFALSE;
		if (m_cRegion.LoadLTA(sFilename, NULL, nFileVersion) != REGIONLOAD_OK)
			return LTFALSE;
	}
#endif

	//resolve any children
	m_cRegion.GetPrefabMgr()->SetRootPath(pMgr->GetRootPath());
	m_cRegion.GetPrefabMgr()->BindAllRefs(m_cRegion.GetRootNode());

	bool bModified;
	m_cRegion.PostLoadUpdateVersion(nFileVersion, bModified);

	// Save the name
	m_sName = pFilename;

	// Figure out the dims
	CalcDims();

	return true;
}

//////////////////////////////////////////////////////////////////////////////
// CPrefabMgr implementation

CPrefabMgr::CPrefabMgr()
{
	m_pszRootPath[0] = '\0';
}

CPrefabMgr::~CPrefabMgr()
{
	Term();
}

bool CPrefabMgr::Init()
{
	Term();

	return true;
}

void CPrefabMgr::Term()
{
	for (uint32 nClearLoop = 0; nClearLoop < m_aPrefabs.GetSize(); ++nClearLoop)
	{
		delete m_aPrefabs[nClearLoop];
	}
	m_aPrefabs.SetSize(0);
}

CLoadedPrefab *CPrefabMgr::Find(const char *pFilename)
{
	CString sName(pFilename);

	uint32 nFindLoop;
	for (nFindLoop = 0; nFindLoop < m_aPrefabs.GetSize(); ++nFindLoop)
	{
		if (sName.CompareNoCase(m_aPrefabs[nFindLoop]->m_sName) == 0)
			return m_aPrefabs[nFindLoop]; 
	}

	// Try the .lta/.ltc switch
	if (!SwapLTALTC(sName))
		return LTNULL;

	for (nFindLoop = 0; nFindLoop < m_aPrefabs.GetSize(); ++nFindLoop)
	{
		if (sName.CompareNoCase(m_aPrefabs[nFindLoop]->m_sName) == 0)
			return m_aPrefabs[nFindLoop]; 
	}

	return LTNULL;
}

CLoadedPrefab *CPrefabMgr::Load(const char *pFilename)
{
	CLoadedPrefab *pResult = new CLoadedPrefab;

	if (!pResult->Load(pFilename, this))
	{
		delete pResult;
		return LTNULL;
	}

	return pResult;
}

CPrefabRef *CPrefabMgr::CreateRef(CEditRegion *pRegion, CWorldNode *pParent, CLoadedPrefab *pPrefab, const char *pName)
{
	// Create the prefab ref
	CPrefabRef *pResult = new CPrefabRef;
	// Set the object's name correctly
	pResult->SetName(pName);
	// Set its filename for later re-loading
	pResult->SetPrefabFilename(pPrefab->m_sName);
	// Point at the prefab
	pResult->SetPrefabTree(pPrefab->m_cRegion.GetRootNode());
	pResult->SetPrefabDims(pPrefab->m_vMin, pPrefab->m_vMax);
	// Clear out the position and orientation
	pResult->SetPos(LTVector(0.0f, 0.0f, 0.0f));
	pResult->SetOr(LTVector(0.0f, 0.0f, 0.0f));
	// Add it to the tree
	no_InitializeNewNode(pRegion, pResult, pParent);
	return pResult;
}
	
CPrefabRef *CPrefabMgr::CreateRef(CEditRegion *pRegion, CWorldNode *pParent, const char *pFilename, const char *pName)
{
	// Look for a loaded prefab with that name
	CLoadedPrefab *pPrefab = Find(pFilename);
	if (pPrefab)
		return CreateRef(pRegion, pParent, pPrefab, pName);

	// Try to load it
	pPrefab = Load(pFilename);
	// Did we find the prefab you're talking about?
	if (!pPrefab)
		return LTNULL;

	// Add it to the list
	m_aPrefabs.Append(pPrefab);
	
	return CreateRef(pRegion, pParent, pPrefab, pName);
}

CPrefabRef *CPrefabMgr::CreateUnboundRef(CEditRegion *pRegion, CWorldNode *pParent, const char *pFilename, const char *pName)
{
	CPrefabRef *pResult = new CPrefabRef;
	// Set the object's name correctly
	pResult->SetName(pName);
	// Set its filename
	pResult->SetPrefabFilename(pFilename);
	// Clear out the position and orientation
	pResult->SetPos(LTVector(0.0f, 0.0f, 0.0f));
	pResult->SetOr(LTVector(0.0f, 0.0f, 0.0f));
	// Add it to the tree
	no_InitializeNewNode(pRegion, pResult, pParent);
	return pResult;
}

bool CPrefabMgr::BindRef(CPrefabRef *pPrefab)
{
	if (!pPrefab)
		return false;

	// Is it already bound?
	if (pPrefab->GetPrefabTree() != LTNULL)
		return true;

	const char *pFilename = pPrefab->GetPrefabFilename();
	CLoadedPrefab *pLoadedPrefab = Find(pFilename);
	if (!pLoadedPrefab)
	{
		pLoadedPrefab = Load(pFilename);
		// Did we find the prefab you're talking about?
		if (!pLoadedPrefab)
			return false;

		//add it to the list of loaded prefabs
		m_aPrefabs.Append(pLoadedPrefab);
	}

	// Hook it up
	pPrefab->SetPrefabTree(pLoadedPrefab->m_cRegion.GetRootNode());
	pPrefab->SetPrefabDims(pLoadedPrefab->m_vMin, pLoadedPrefab->m_vMax);

	return true;
}

bool CPrefabMgr::BindAllRefs(CWorldNode *pRoot)
{
	if (!pRoot)
		return true;

	if (pRoot->GetType() == Node_PrefabRef)
	{
		//now we bind this object
		if (!BindRef((CPrefabRef*)pRoot))
			return false;
	}

	bool bResult = LTTRUE;

	GPOS iCurChild = pRoot->m_Children.GetHeadPosition();
	while (iCurChild)
	{
		CPrefabRef *pChild = (CPrefabRef *)pRoot->m_Children.GetNext(iCurChild);
		bResult &= BindAllRefs(pChild);
	}

	return bResult;
}

//sets the root directory that the prefabs will be under (the resource directory)
void CPrefabMgr::SetRootPath(const char* pszPath)
{
	strncpy(m_pszRootPath, pszPath, MAX_PATH);
}

//gets the root resource directory
const char* CPrefabMgr::GetRootPath() const
{
	return m_pszRootPath;
}

// looks at the loaded prefab by filename and then looks at it's region for the object
uint32 CPrefabMgr::FindObjectsByNameInPrefab( const char* pPrefabFilename, const char* pObjectName, CBaseEditObj **objectList, uint32 maxObjects )
{
	if( !pPrefabFilename || !pObjectName )
		return 0;

	CLoadedPrefab *pPrefab = Find( pPrefabFilename );
	if( !pPrefab )
	{
		return 0;
	}

	return pPrefab->m_cRegion.FindObjectsByName( pObjectName, objectList, maxObjects );
}

const CEditRegion* CPrefabMgr::GetPrefabRegion( const char* pPrefabFilename )
{
	if( !pPrefabFilename )
		return NULL;

	CLoadedPrefab *pPrefab = Find( pPrefabFilename );
	if( !pPrefab )
		return NULL;

	return &pPrefab->m_cRegion;
}
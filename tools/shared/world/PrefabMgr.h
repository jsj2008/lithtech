//------------------------------------------------------
// PrefabMgr.h
//
// Prefab management class header
//
// Author: Kevin Francis
// Created: 03/15/2001
// Modification History: 
//		03/15/2001 - First implementation
//
//------------------------------------------------------


#ifndef __PREFABMGR_H__
#define __PREFABMGR_H__

#include "prefabref.h"

class CLoadedPrefab;

class CEditRegion;

class CPrefabMgr
{
public:
	CPrefabMgr();
	~CPrefabMgr();
	
	bool Init();
	void Term();

	// Create a prefab reference for a prefab file
	CPrefabRef *		CreateRef(CEditRegion *pRegion, CWorldNode *pParent, const char *pFilename, const char *pName);

	// Create a prefab which refers to a prefab file without actually loading it
	CPrefabRef *		CreateUnboundRef(CEditRegion *pRegion, CWorldNode *pParent, const char *pFilename, const char *pName);

	// Bind an unbound prefabref to its final prefab file (i.e. load the prefab)
	bool				BindRef(CPrefabRef *pPrefab);

	// Bind all unbound prefabrefs to their final prefab files
	bool				BindAllRefs(CWorldNode *pRoot);

	//sets the root directory that the prefabs will be under (the resource directory)
	void				SetRootPath(const char* pszPath);

	//gets the root resource directory
	const char*			GetRootPath() const;
	
	// looks at the loaded prefab by filename and then looks at it's region for the object
	uint32				FindObjectsByNameInPrefab( const char* pPrefabFilename, const char* pObjectName, CBaseEditObj **objectList, uint32 maxObjects );

	const CEditRegion*  GetPrefabRegion( const char* pPrefabFilename );

private:
	// Find a prefab by filename
	CLoadedPrefab *		Find(const char *pFilename);

	// Load a prefab by filename (Implementation assumes the filename isn't 
	// already in the list.)
	CLoadedPrefab *		Load(const char *pFilename);

	// Create a CPrefabRef given a CLoadedPrefab*
	CPrefabRef *		CreateRef(CEditRegion *pRegion, CWorldNode *pParent, CLoadedPrefab *pPrefab, const char *pName);

	// Our list of known prefabs
	CMoArray<CLoadedPrefab *>	m_aPrefabs;

	char						m_pszRootPath[MAX_PATH];
};

#endif //__PREFABMGR_H__

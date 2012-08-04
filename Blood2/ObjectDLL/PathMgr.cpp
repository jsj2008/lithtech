// ----------------------------------------------------------------------- //
//
// MODULE  : PathMgr.cpp
//
// PURPOSE : PathMgr implementation
//
// CREATED : 2/9/98
//
// ----------------------------------------------------------------------- //

#include "PathMgr.h"
#include "PathListData.h"
#include "cpp_engineobjects_de.h"
#include <ctype.h>


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	PathMgr::BuildPathList
//
//	PURPOSE:	Create a list of all the PathPoints in the level.
//
// ----------------------------------------------------------------------- //

DBOOL PathMgr::BuildPathList()
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE) return DFALSE;
	
	ClearPathList();

	// Iterate through the world building our list of keys, and removing 
	// all the PathPoints found...

	HCLASS  hPathPoint = pServerDE->GetClass("PathPoint");
	HOBJECT	hCurObject = DNULL;
	while (hCurObject = pServerDE->GetNextObject(hCurObject))
	{
		if (pServerDE->IsKindOf(pServerDE->GetObjectClass(hCurObject), hPathPoint))
		{
            // Make sure its not a SMELL pathpoint!
			PathListData* pKeyData = new PathListData;
			if (!pKeyData) return DFALSE;

			pKeyData->Copy(hCurObject);

			m_pathList.Add(pKeyData);

            // Check for Debug console var, if found do not REMOVE from world!
//			pServerDE->RemoveObject(hCurObject);
		}
	}

	return DTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	PathMgr::GetPath
//
//	PURPOSE:	Get the path associated with the BasePathName
//
// ----------------------------------------------------------------------- //

DBOOL PathMgr::GetPath(char* pBasePathName, PathList* pReturnList)
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE) return DFALSE;

	if (!pBasePathName || !pReturnList || pBasePathName[0] == '\0') return DFALSE;

	// Make sure we've got some keys to look through...

	if (m_pathList.IsEmpty()) return DFALSE;


	// Make sure we start with a clean slate...

	if (!pReturnList->IsEmpty()) pReturnList->RemoveAll();


	int nNumItems = m_pathList.GetNumItems();
	int nNameLen  = _mbstrlen(pBasePathName);
	if (nNameLen < 1) return DFALSE;

	DList sortList;
	dl_InitList(&sortList);
	sortList.m_Head.m_pData = DNULL;

	for (int i=0; i < nNumItems; i++)
	{
		PathListData* pCurData = m_pathList[i];
		if (!pCurData || !pCurData->m_hstrName) return DFALSE;

		char* pName = pServerDE->GetStringData(pCurData->m_hstrName);
		if (!pName) return DFALSE;

		if (strnicmp(pName, pBasePathName, nNameLen) == 0)
		{
			int nTestLen = _mbstrlen(pName);
			
			// Make sure the name is of the form BaseXXX where XXX is
			// a number...

			if (nTestLen > nNameLen)
			{
				if (isdigit(pName[nNameLen]))
				{
					AddToSortedList(&sortList, pCurData);
				}
			}
		}
	}

	if (sortList.m_nElements < 1) return DFALSE;


	// Copy the sorted list to the return list...

	DLink* pLink = sortList.m_Head.m_pNext;

	while (sortList.m_nElements > 0)
	{
		PathListData* pData = (PathListData*)pLink->m_pData;
		if (pData)
		{
			pReturnList->Add(pData);
		}

		dl_RemoveAt(&sortList, pLink);
		delete pLink;

		pLink = sortList.m_Head.m_pNext;
	}


	return DTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	PathMgr::AddToSortedList
//
//	PURPOSE:	Add the PathListData to the list in sorted (on name) order
//				(i.e., "Name0, Name1, Name2, etc.")
//
// ----------------------------------------------------------------------- //

void PathMgr::AddToSortedList(DList* pSortList, PathListData* pCurData)
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE || !pSortList || !pCurData) return;

	DLink* pLink = new DLink;
	if (!pLink) return;

	pLink->m_pNext = pLink->m_pPrev = DNULL;
	pLink->m_pData = DNULL;

	// Find where in the list we belong...

	DLink* pCurPos    = &pSortList->m_Head;
	DLink* pInsertPos = pCurPos;

	while (pCurPos)
	{
		PathListData* pData = (PathListData*)pCurPos->m_pData;
		if (!pData) break;

		if (pServerDE->CompareStringsUpper(pCurData->m_hstrName, pData->m_hstrName) < 0)
		{
			break;
		}

		pInsertPos = pCurPos;
		pCurPos	   = pCurPos->m_pNext;
	}

	dl_AddAfter(pSortList, pInsertPos, pLink, (void*)pCurData);
}

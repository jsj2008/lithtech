// ----------------------------------------------------------------------- //
//
// MODULE  : CAIPathMgr.cpp
//
// PURPOSE : CAIPathMgr implementation
//
// CREATED : 2/9/98
//
// ----------------------------------------------------------------------- //

#include "AIPathMgr.h"
#include "AIKeyData.h"
#include "cpp_engineobjects_de.h"
#include "ctype.h"


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIPathMgr::BuildPathList
//
//	PURPOSE:	Create a list of all the AIKeys in the level.
//
// ----------------------------------------------------------------------- //

DBOOL CAIPathMgr::BuildPathList()
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE) return DFALSE;
	
	ClearPathList();

	// Iterate through the world building our list of keys, and removing 
	// all the AIKeys found...

	HCLASS  hAIKey = pServerDE->GetClass("AIKey");
	HOBJECT	hCurObject = DNULL;
	while (hCurObject = pServerDE->GetNextObject(hCurObject))
	{
		if (pServerDE->IsKindOf(pServerDE->GetObjectClass(hCurObject), hAIKey))
		{
			CAIKeyData* pKeyData = new CAIKeyData;
			if (!pKeyData) return DFALSE;

			pKeyData->Copy(hCurObject);

			m_pathList.Add(pKeyData);

			pServerDE->RemoveObject(hCurObject);
		}
	}

	hCurObject = DNULL;
	while (hCurObject = pServerDE->GetNextInactiveObject(hCurObject))
	{
		if (pServerDE->IsKindOf(pServerDE->GetObjectClass(hCurObject), hAIKey))
		{
			CAIKeyData* pKeyData = new CAIKeyData;
			if (!pKeyData) return DFALSE;

			pKeyData->Copy(hCurObject);

			m_pathList.Add(pKeyData);

			pServerDE->RemoveObject(hCurObject);
		}
	}

	return DTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIPathMgr::GetPath
//
//	PURPOSE:	Get the path associated with the BaseKeyName
//
// ----------------------------------------------------------------------- //

DBOOL CAIPathMgr::GetPath(char* pBaseKeyName, CAIPathList* pReturnList)
{
	if (!pBaseKeyName || !pReturnList || pBaseKeyName[0] == '\0') return DFALSE;

	// Make sure we've got some keys to look through...

	if (m_pathList.IsEmpty()) return DFALSE;


	// Make sure we start with a clean slate...

	if (!pReturnList->IsEmpty()) pReturnList->RemoveAll();


	int nNumItems = m_pathList.GetNumItems();
	int nNameLen  = strlen(pBaseKeyName);
	if (nNameLen < 1) return DFALSE;

	DList sortList;
	dl_InitList(&sortList);
	sortList.m_Head.m_pData = DNULL;

	for (int i=0; i < nNumItems; i++)
	{
		CAIKeyData* pCurData = m_pathList[i];
		if (!pCurData) return DFALSE;

		if (strnicmp(pCurData->m_Name, pBaseKeyName, nNameLen) == 0)
		{
			int nTestLen = strlen(pCurData->m_Name);
			
			// Make sure the name is of the form BaseXXX where XXX is
			// a number...

			if (nTestLen > nNameLen)
			{
				if (isdigit(pCurData->m_Name[nNameLen]))
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
		CAIKeyData* pData = (CAIKeyData*)pLink->m_pData;
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
//	ROUTINE:	CAIPathMgr::AddToSortedList
//
//	PURPOSE:	Add the AIKeyData to the list in sorted (on name) order
//				(i.e., "Name0, Name1, Name2, etc.")
//
// ----------------------------------------------------------------------- //

void CAIPathMgr::AddToSortedList(DList* pSortList, CAIKeyData* pCurData)
{
	if (!pSortList || !pCurData) return;

	DLink* pLink = new DLink;
	if (!pLink) return;

	pLink->m_pNext = pLink->m_pPrev = DNULL;
	pLink->m_pData = DNULL;

	// Find where in the list we belong...

	DLink* pCurPos    = pSortList->m_Head.m_pNext;
	DLink* pInsertPos = &(pSortList->m_Head);

	while (pCurPos)
	{
		CAIKeyData* pData = (CAIKeyData*)pCurPos->m_pData;
		if (!pData) break;

		if (stricmp(pCurData->m_Name, pData->m_Name) < 0)
		{
			break;
		}

		pInsertPos = pCurPos;
		pCurPos	   = pCurPos->m_pNext;
	}

	dl_AddAfter(pSortList, pInsertPos, pLink, (void*)pCurData);
}

// ----------------------------------------------------------------------- //
//
// MODULE  : LiteObjectMgr.cpp
//
// PURPOSE : Lite Object Manager
//
// CREATED : 7/12/02
//
// (c) 2002 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "LiteObjectMgr.h"
#include "GameBaseLite.h"

#include <set>
#include <algorithm>

CVarTrack g_ShowLiteObjectInfoTrack;

CLiteObjectMgr::CLiteObjectMgr() :
	m_nNumActiveObjects(0),
	m_nNumInactiveObjects(0),
	m_bSerializeIDsDirty(false),
	m_bObjectListsDirty(false)
{
}

CLiteObjectMgr::~CLiteObjectMgr()
{
	// Don't leak...
	Clear();
}

uint32 CLiteObjectMgr::GetSerializeID(GameBaseLite *pObject)
{
	if (!pObject)
		return k_nInvalidSerializeID;

	// Transition objects or keepalive objects can't access liteobjects.  Liteobjects
	// are only saved in full save games.
	if( g_pGameServerShell->GetLGFlags( ) == LOAD_TRANSITION || g_pGameServerShell->GetLGFlags( ) == LOAD_NEW_LEVEL)
		return k_nInvalidSerializeID;

	CleanSerializeIDs();

	return pObject->GetSerializeID();
}

GameBaseLite *CLiteObjectMgr::GetSerializeObject(uint32 nID)
{
	if (nID == k_nInvalidSerializeID)
		return 0;

	// Is it an inactive object?
	if (nID >= GetNumActiveObjects())
	{
		// Adjust the index...
		nID -= GetNumActiveObjects();
		// Is it actually within range?
		if (nID < GetNumInactiveObjects())
		{
			return m_aInactiveObjects[nID];
		}
		else
		{
			ASSERT(!"Invalid serialization ID encountered");
			return 0;
		}
	}

	return m_aActiveObjects[nID];
}

void CLiteObjectMgr::AddObject(GameBaseLite *pObject)
{
	if (!pObject)
	{
		ASSERT(!"NULL lite object insertion encountered");
		return;
	}

	// Add it to the name map
	if (FindObjectByName(pObject->GetName()) != 0)
	{
		Warn("Duplicate lite object name %s encountered", pObject->GetName());
		if (IsObjectInList(m_aActiveObjects, pObject) ||
			IsObjectInList(m_aInactiveObjects, pObject))
		{
			ASSERT(!"Double-insertion encountered in lite object mgr");
			return;
		}
	}
	m_aNameMap[pObject->GetName()] = pObject;

	// Add it to the proper list
	if (pObject->IsActive())
		AddActiveObject(pObject);
	else
		AddInactiveObject(pObject);

	// Queue up an initial update
	m_aInitialUpdateObjects.push_back(pObject);

	// Your serialization IDs are dirty
	SetDirty(eDirty_SerializeIDs);
}

void CLiteObjectMgr::RemoveObject(GameBaseLite *pObject)
{
	// Remove it from our list
	bool bRemoveResult;
	if (pObject->IsActive())
	{
		bRemoveResult = RemoveObjectFromList(m_aActiveObjects, pObject);
		--m_nNumActiveObjects;
		ASSERT(!IsObjectInList(m_aInactiveObjects, pObject));
	}
	else
	{
		bRemoveResult = RemoveObjectFromList(m_aInactiveObjects, pObject);
		--m_nNumInactiveObjects;
		ASSERT(!IsObjectInList(m_aActiveObjects, pObject));
	}

	// Remove it from the map
	TNameMap::iterator iObjectNamePos = m_aNameMap.find(pObject->GetName());
	if ((iObjectNamePos != m_aNameMap.end()) && (iObjectNamePos->second == pObject))
		m_aNameMap.erase(iObjectNamePos);

	// Make sure it's not in the initialupdate list
	RemoveObjectFromList(m_aInitialUpdateObjects, pObject);
	RemoveObjectFromList(m_aInitialUpdateObjectsLoad, pObject);

	// Clean it up in the engine
	if (bRemoveResult)
	{
		g_pLTServer->RemoveObject(pObject->GetClass(), pObject);
	}
	else
	{
		ASSERT(!"Invalid lite object removal encountered");
		return;
	}

	// You're dirty...
	SetDirty(eDirty_ObjectLists);
}

void CLiteObjectMgr::RenameObject(GameBaseLite *pObject, const char *pName)
{
	// Note: The order of operations is important here, because we're maintaining a pointer to the object's name data

	// Remove the object from the name map
	TNameMap::iterator iCurPos = m_aNameMap.find(pObject->GetName());
	if (iCurPos != m_aNameMap.end())
	{
		if (iCurPos->second == pObject)
			m_aNameMap.erase(iCurPos);
	}
	else
	{
		// I don't know you anyway.
		// This happens during the process of renaming an object by calling the
		// RenameObject function from outside of the GameBaseLite object in question.
		// So we should exit in order to avoid double-setting the name.
		return;
	}

	// Rename the object
	pObject->SetName(pName);

	// Remember the new name of the object
	m_aNameMap[pObject->GetName()] = pObject;
}

bool CLiteObjectMgr::HasObject(GameBaseLite *pObject)
{
	return IsObjectInList(m_aActiveObjects, pObject) ||
		IsObjectInList(m_aInactiveObjects, pObject);
}

void CLiteObjectMgr::Clear()
{
	HandlePendingDeletes();

	// Clear the object lists
	RemoveObjectList(m_aActiveObjects);
	m_nNumActiveObjects = 0;
	RemoveObjectList(m_aInactiveObjects);
	m_nNumInactiveObjects = 0;

	// Clear the name map
	m_aNameMap.clear();

	// Clear the initial update list
	m_aInitialUpdateObjects.swap(TObjectList());
	m_aInitialUpdateObjectsLoad.swap(TObjectList());

	// You're clean, now
	SetDirty(eDirty_All, false);
}

void CLiteObjectMgr::ActivateObject(GameBaseLite *pObject)
{
	// Make sure we're going about this the right way
	if (!pObject->IsActive())
	{
		pObject->Activate();
		return;
	}

	// Remove them from the inactive list
	if (!RemoveObjectFromList(m_aInactiveObjects, pObject))
	{
		ASSERT(IsObjectInList(m_aActiveObjects, pObject));
		return;
	}
	--m_nNumInactiveObjects;

	// Put it in the active list
	AddActiveObject(pObject);

	SetDirty();
}

void CLiteObjectMgr::DeactivateObject(GameBaseLite *pObject)
{
	// Make sure we're going about this the right way
	if (pObject->IsActive())
	{
		pObject->Deactivate();
		return;
	}

	// Remove them from the inactive list
	if (!RemoveObjectFromList(m_aActiveObjects, pObject))
	{
		ASSERT(IsObjectInList(m_aInactiveObjects, pObject));
		return;
	}
	--m_nNumActiveObjects;

	// Put it in the active list
	AddInactiveObject(pObject);

	SetDirty();
}

void CLiteObjectMgr::Load(ILTMessage_Read *pMsg)
{
	// Get rid of the old stuff...
	Clear();

	// Read the active object count
	uint32 nNumActiveObjectsRemaining = pMsg->Readuint32();
	m_nNumActiveObjects = nNumActiveObjectsRemaining;

	// Read the inactive object count
	uint32 nNumInactiveObjectsRemaining = pMsg->Readuint32();
	m_nNumInactiveObjects = nNumInactiveObjectsRemaining;

	// Reserve some space
	m_aActiveObjects.reserve(m_nNumActiveObjects);
	m_aInactiveObjects.reserve(m_nNumInactiveObjects);

	// Load the active objects
	uint32 nCurSerializeID = 0;
	for (; nNumActiveObjectsRemaining; --nNumActiveObjectsRemaining)
	{
		m_aActiveObjects.push_back(LoadObjectInfo(pMsg, nCurSerializeID++));
		ASSERT(!pMsg->EOM());
	}
	ASSERT(nCurSerializeID == m_aActiveObjects.size());

	// Load the inactive objects
	for (; nNumInactiveObjectsRemaining; --nNumInactiveObjectsRemaining)
	{
		m_aInactiveObjects.push_back(LoadObjectInfo(pMsg, nCurSerializeID++));
		ASSERT(!pMsg->EOM());
	}

	// Tell the active objects to load
	TObjectList::iterator iCurObj = m_aActiveObjects.begin();
	for (; iCurObj != m_aActiveObjects.end(); ++iCurObj)
	{
		(*iCurObj)->Load(pMsg);
	}

	// Tell the inactive objects to load
	for (iCurObj = m_aInactiveObjects.begin(); iCurObj != m_aInactiveObjects.end(); ++iCurObj)
	{
		(*iCurObj)->Load(pMsg);
	}
}

void CLiteObjectMgr::Save(ILTMessage_Write *pMsg)
{
	// Make sure we're not saving anything dirty
	CleanObjectLists();

	// Clean up the serialization IDs if they're dirty
	CleanSerializeIDs();

	// Write the object counts
	pMsg->Writeuint32(m_nNumActiveObjects);
	pMsg->Writeuint32(m_nNumInactiveObjects);

	// Save the active objects
	TObjectList::iterator iCurObj = m_aActiveObjects.begin();
	for (; iCurObj != m_aActiveObjects.end(); ++iCurObj)
	{
		SaveObjectInfo(pMsg, *iCurObj);
	}

	// Save the inactive objects
	for (iCurObj = m_aInactiveObjects.begin(); iCurObj != m_aInactiveObjects.end(); ++iCurObj)
	{
		SaveObjectInfo(pMsg, *iCurObj);
	}

	// Tell the active objects to save
	for (iCurObj = m_aActiveObjects.begin(); iCurObj != m_aActiveObjects.end(); ++iCurObj)
	{
		(*iCurObj)->Save(pMsg);
	}

	// Tell the inactive objects to save
	for (iCurObj = m_aInactiveObjects.begin(); iCurObj != m_aInactiveObjects.end(); ++iCurObj)
	{
		(*iCurObj)->Save(pMsg);
	}
}

void CLiteObjectMgr::Update()
{
	LTCounter cUpdateTime;
	g_pLTServer->StartCounter(&cUpdateTime);
	
	HandlePendingDeletes();
	HandlePendingInitialUpdates();

	// Update the active objects
	// Note : This can't use an iterator, since the update might add objects or something like that...
	uint32 nActiveObjectSize = m_aActiveObjects.size();
	for (uint32 nCurObj = 0; nCurObj < nActiveObjectSize; ++nCurObj)
	{
		GameBaseLite *pCurObj = m_aActiveObjects[nCurObj];
		if (pCurObj)
			pCurObj->Update();
		ASSERT(nCurObj < m_aActiveObjects.size());
	}

	// Clean up, if we need to
	CleanObjectLists();

	uint32 nUpdateTime = g_pLTServer->EndCounter(&cUpdateTime);

	if (g_ShowLiteObjectInfoTrack.GetFloat())
	{
		ShowInfo(nUpdateTime);
	}
}

void CLiteObjectMgr::PreStartWorld(bool bSwitchingWorlds)
{
	// Can't do this in the ctor due to g_pLTServer not being valid yet...
	if (!g_ShowLiteObjectInfoTrack.IsInitted())
		g_ShowLiteObjectInfoTrack.Init(g_pLTServer, "ShowLiteObjectInfo", LTNULL, 0.0f);

	// Move the objects into the delete list
	if (m_aDeleteObjects.empty())
		m_aActiveObjects.swap(m_aDeleteObjects);
	else
	{
		m_aDeleteObjects.insert(m_aDeleteObjects.end(), m_aActiveObjects.begin(), m_aActiveObjects.end());
		m_aActiveObjects.swap(TObjectList());
	}
	m_nNumActiveObjects = 0;

	if (m_aDeleteObjects.empty())
		m_aInactiveObjects.swap(m_aDeleteObjects);
	else
	{
		m_aDeleteObjects.insert(m_aDeleteObjects.end(), m_aInactiveObjects.begin(), m_aInactiveObjects.end());
		m_aInactiveObjects.swap(TObjectList());
	}
	m_nNumInactiveObjects = 0;

	// Clear the name map
	m_aNameMap.clear();

	// Clear the initial update list
	m_aInitialUpdateObjects.swap(TObjectList());

	// You're clean, now
	SetDirty(eDirty_All, false);

}

void CLiteObjectMgr::PostStartWorld()
{
	HandlePendingDeletes();
	HandlePendingInitialUpdates();
}

GameBaseLite *CLiteObjectMgr::FindObjectByName(const char *pName)
{
	// Do a look-up
	TNameMap::iterator iObj = m_aNameMap.find(pName);
	if (iObj == m_aNameMap.end())
		return 0;

	return iObj->second;
}

bool CLiteObjectMgr::RemoveObjectFromList(TObjectList &aList, GameBaseLite *pObject)
{
	TObjectList::iterator iObj = std::find(aList.begin(), aList.end(), pObject);
	if (iObj == aList.end())
		return false;
	*iObj = 0;
	return true;
}

bool CLiteObjectMgr::IsObjectInList(TObjectList &aList, GameBaseLite *pObject)
{
	return std::find(aList.begin(), aList.end(), pObject) != aList.end();
}

void CLiteObjectMgr::CleanList(TObjectList &aList)
{
	// Clear nulls from the end of the list
	while (!aList.empty() && (aList.back() == 0))
		aList.pop_back();

	// Swap empties to the end of the list and get rid of them
	TObjectList::iterator iCurObj = aList.begin();
	while (iCurObj != aList.end())
	{
		if (!*iCurObj)
		{
			*iCurObj = aList.back();
			aList.pop_back();
		}
		else
			++iCurObj;
	}
}

void CLiteObjectMgr::CleanObjectLists()
{
	if (!m_bObjectListsDirty)
		return;

	// Clean the active objects
	if (m_aActiveObjects.size() != m_nNumActiveObjects)
		CleanList(m_aActiveObjects);
	ASSERT(m_aActiveObjects.size() == m_nNumActiveObjects);

	// Clean the inactive objects
	if (m_aInactiveObjects.size() != m_nNumInactiveObjects)
		CleanList(m_aInactiveObjects);
	ASSERT(m_aInactiveObjects.size() == m_nNumInactiveObjects);

	// You are clean
	m_bObjectListsDirty = false;

	// But the serialization IDs are now dirty
	SetDirty(eDirty_SerializeIDs);
}

void CLiteObjectMgr::CleanSerializeIDs()
{
	if (!m_bSerializeIDsDirty)
		return;

	uint32 nCurID = 0;

	// Set the active object serialize ID's
	TObjectList::iterator iCurObj = m_aActiveObjects.begin();
	for (; iCurObj != m_aActiveObjects.end(); ++iCurObj)
		(*iCurObj)->SetSerializeID(nCurID++);
	ASSERT(nCurID == m_aActiveObjects.size());

	// Set the inactive object serialize ID's
	iCurObj = m_aInactiveObjects.begin();
	for (; iCurObj != m_aInactiveObjects.end(); ++iCurObj)
		(*iCurObj)->SetSerializeID(nCurID++);

	// You're clean
	m_bSerializeIDsDirty = false;
}

void CLiteObjectMgr::SetDirty(EDirtyFlags eDirty, bool bYesNo)
{
	if (eDirty & eDirty_SerializeIDs)
		m_bSerializeIDsDirty = bYesNo;
	if (eDirty & eDirty_ObjectLists)
		m_bObjectListsDirty = bYesNo;
}

void CLiteObjectMgr::AddActiveObject(GameBaseLite *pObject)
{
	m_aActiveObjects.push_back(pObject);
	++m_nNumActiveObjects;
}

void CLiteObjectMgr::AddInactiveObject(GameBaseLite *pObject)
{
	m_aInactiveObjects.push_back(pObject);
	++m_nNumInactiveObjects;
}

void CLiteObjectMgr::RemoveObjectList(TObjectList &aList)
{
	// Remove the objects in the engine
	TObjectList::iterator iCurObj = aList.begin();
	for (; iCurObj != aList.end(); ++iCurObj)
	{
		if (*iCurObj)
		{
			GameBaseLite *pObj = *iCurObj;
			// Clear out the entry in the list so we don't think we have it any more
			*iCurObj = 0;
			g_pLTServer->RemoveObject(pObj->GetClass(), pObj);
		}
	}
	// Forget...
	aList.swap(TObjectList());
}

GameBaseLite *CLiteObjectMgr::LoadObjectInfo(ILTMessage_Read *pMsg, uint32 nSerializeID)
{
	// Class?
	uint32 nClassNameLen = pMsg->PeekString(0,0) + 1;
	char *pClassName = (char *)alloca(nClassNameLen);
	pMsg->ReadString(pClassName, nClassNameLen);
	
	// Name?
	uint32 nObjectNameLen = pMsg->PeekString(0,0) + 1;
	char *pObjectName = (char *)alloca(nObjectNameLen);
	pMsg->ReadString(pObjectName, nObjectNameLen);

	// Do you have class?
	HCLASS hClass = g_pLTServer->GetClass(pClassName);
	if (!hClass)
	{
		ASSERT(!"Invalid class in lite obect load");
		return 0;
	}

	ASSERT(g_pLTServer->IsKindOf(hClass, g_pLTServer->GetClass("GameBaseLite")));

	// Tell the server to create an empty object for us
	GameBaseLite *pObject = (GameBaseLite*)g_pLTServer->CreateObject(hClass, LTNULL);

	// Tell the object what's what
	pObject->SetSerializeID(nSerializeID);
	pObject->SetClass(hClass);
	pObject->SetName(pObjectName);

	// Add them to the name map
	m_aNameMap[pObject->GetName()] = pObject;

	// Queue it up for an initial update
	m_aInitialUpdateObjectsLoad.push_back(pObject);

	return pObject;
}

void CLiteObjectMgr::SaveObjectInfo(ILTMessage_Write *pMsg, GameBaseLite *pObject)
{
	// Write the class name
	char aClassNameBuff[256];
	g_pLTServer->GetClassName(pObject->GetClass(), aClassNameBuff, sizeof(aClassNameBuff));
	pMsg->WriteString(aClassNameBuff);

	// Write the object name
	pMsg->WriteString(pObject->GetName());
}

void CLiteObjectMgr::HandlePendingInitialUpdates()
{
	// Send initial updates to all the objects that are waiting for one
	TObjectList::iterator iCurObj = m_aInitialUpdateObjectsLoad.begin();
	for (; iCurObj != m_aInitialUpdateObjectsLoad.end(); ++iCurObj)
	{
		if (*iCurObj)
			(*iCurObj)->InitialUpdate(INITIALUPDATE_SAVEGAME);
	}
	// Clear the list
	m_aInitialUpdateObjectsLoad.swap(TObjectList());
	// Send initial updates to all the objects that are waiting for one
	iCurObj = m_aInitialUpdateObjects.begin();
	for (; iCurObj != m_aInitialUpdateObjects.end(); ++iCurObj)
	{
		if (*iCurObj)
			(*iCurObj)->InitialUpdate(INITIALUPDATE_NORMAL);
	}
	// Clear the list
	m_aInitialUpdateObjects.swap(TObjectList());
}

void CLiteObjectMgr::HandlePendingDeletes()
{
	if (m_aDeleteObjects.empty())
		return;

	RemoveObjectList(m_aDeleteObjects);
}

uint32 CLiteObjectMgr::GetObjectsOfClass(HCLASS hClass, TObjectList *pResults) const
{
	// Save us some trouble if it's obvious...
	if (!hClass)
		return 0;

	uint32 nResult = 0;

	// Search through the active object list
	TObjectList::const_iterator iCurObj = m_aActiveObjects.begin();
	for (; iCurObj != m_aActiveObjects.end(); ++iCurObj)
	{
		if ((*iCurObj) && g_pLTServer->IsKindOf((*iCurObj)->GetClass(), hClass))
		{
			if (pResults)
				pResults->push_back(*iCurObj);
			++nResult;
		}
	}

	// Search through the inactive object list
	iCurObj = m_aInactiveObjects.begin();
	for (; iCurObj != m_aInactiveObjects.end(); ++iCurObj)
	{
		if ((*iCurObj) && g_pLTServer->IsKindOf((*iCurObj)->GetClass(), hClass))
		{
			if (pResults)
				pResults->push_back(*iCurObj);
			++nResult;
		}
	}

	return nResult;
}

void CLiteObjectMgr::ShowInfo(uint32 nUpdateTime)
{
	g_pLTServer->CPrint("\nLiteObjectMgr stats : (%d ticks)", nUpdateTime);

	typedef std::set<HCLASS> TClassSet;
	TClassSet aClasses;

	TObjectList::const_iterator iCurObject;

	// Set up the class set
	iCurObject = m_aActiveObjects.begin();
	for (; iCurObject != m_aActiveObjects.end(); ++iCurObject)
	{
		aClasses.insert((*iCurObject)->GetClass());
	}

	iCurObject = m_aInactiveObjects.begin();
	for (; iCurObject != m_aInactiveObjects.end(); ++iCurObject)
	{
		aClasses.insert((*iCurObject)->GetClass());
	}

	TClassSet::const_iterator iCurClass;

	// Figure out how wide the class name field should be
	uint32 nMaxClassNameWidth = 6;
	iCurClass = aClasses.begin();
	for (; iCurClass != aClasses.end(); ++iCurClass)
	{
		char aClassNameBuff[256];
		g_pLTServer->GetClassName(*iCurClass, aClassNameBuff, sizeof(aClassNameBuff));
		nMaxClassNameWidth = LTMAX(nMaxClassNameWidth, strlen(aClassNameBuff));
	}

	// Print a header
	char *aClassUnderline = (char*)alloca(nMaxClassNameWidth + 1);
	memset(aClassUnderline, '-', nMaxClassNameWidth);
	aClassUnderline[nMaxClassNameWidth] = 0;
	g_pLTServer->CPrint("   -%s-|--------|----------|-------", aClassUnderline);
	g_pLTServer->CPrint("    Class %*s | Active | Inactive | Total", nMaxClassNameWidth - 6,"");
	g_pLTServer->CPrint("   -%s-|--------|----------|-------", aClassUnderline);

	// Run through the classes and accumulate stats
	iCurClass = aClasses.begin();
	for (; iCurClass != aClasses.end(); ++iCurClass)
	{
		uint32 nNumActive = 0;
		iCurObject = m_aActiveObjects.begin();
		for (; iCurObject != m_aActiveObjects.end(); ++iCurObject)
		{
			if ((*iCurObject)->GetClass() == *iCurClass)
				++nNumActive;
		}

		uint32 nNumInactive = 0;
		iCurObject = m_aInactiveObjects.begin();
		for (; iCurObject != m_aInactiveObjects.end(); ++iCurObject)
		{
			if ((*iCurObject)->GetClass() == *iCurClass)
				++nNumInactive;
		}
		
		char aClassNameBuff[256];
		g_pLTServer->GetClassName(*iCurClass, aClassNameBuff, sizeof(aClassNameBuff));

		// Here's your output...
		g_pLTServer->CPrint("    %-*s |   %4d |     %4d |  %4d",
			nMaxClassNameWidth, aClassNameBuff, nNumActive, nNumInactive, nNumActive + nNumInactive);
	}

	// Print the totals
	g_pLTServer->CPrint("   -%s-|--------|----------|-------", aClassUnderline);
	g_pLTServer->CPrint("    %-*s |   %4d |     %4d |  %4d",
		nMaxClassNameWidth, "Total", 
		GetNumActiveObjects(), GetNumInactiveObjects(), GetNumObjects());
}

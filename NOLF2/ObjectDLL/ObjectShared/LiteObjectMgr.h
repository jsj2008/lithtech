// ----------------------------------------------------------------------- //
//
// MODULE  : LiteObjectMgr.h
//
// PURPOSE : Lite Object Manager
//
// CREATED : 7/12/02
//
// (c) 2002 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __LITE_OBJECT_MGR_H__
#define __LITE_OBJECT_MGR_H__

#include "ButeMgr.h"
#include <vector>

class GameBaseLite;

class CLiteObjectMgr
{
public:
	CLiteObjectMgr();
	~CLiteObjectMgr();

	uint32 GetSerializeID(GameBaseLite *pObject);
	GameBaseLite *GetSerializeObject(uint32 nID);

	// Add an object to the object mgr
	void AddObject(GameBaseLite *pObject);
	// Delete an object
	void RemoveObject(GameBaseLite *pObject);
	// Rename an object
	void RenameObject(GameBaseLite *pObject, const char *pName);
	// Is the mgr currently tracking this object?
	bool HasObject(GameBaseLite *pObject);

	// Remove all objects, and start over
	void Clear();

	// Activation - called by GameBaseLite
	void ActivateObject(GameBaseLite *pObject);
	void DeactivateObject(GameBaseLite *pObject);

	// Serialization
	void Load(ILTMessage_Read *pMsg);
	void Save(ILTMessage_Write *pMsg);

	// Update - Must be called once per frame
	void Update();

	// The world is about to start - Must be called during PreStartWorld processing
	void PreStartWorld(bool bSwitchingWorlds);
	// The world is done loading, so go do stuff - Must be called during PostStartWorld processing
	void PostStartWorld();

	// Find an object by name
	GameBaseLite *FindObjectByName(const char *pName);

	// Information
	uint32 GetNumObjects() const { return m_nNumActiveObjects + m_nNumInactiveObjects; }
	uint32 GetNumActiveObjects() const { return m_nNumActiveObjects; }
	uint32 GetNumInactiveObjects() const { return m_nNumInactiveObjects; }

	typedef std::vector<GameBaseLite*> TObjectList;

	// Query functions
	uint32 GetObjectsOfClass(HCLASS hClass, TObjectList *pResults) const;

private:

#if _MSC_VER == 1300

	typedef std::hash_map< const char *, GameBaseLite *, ButeMgrHashCompare > TNameMap;

#elif _MSC_VER > 1300

	typedef stdext::hash_map< const char *, GameBaseLite *, ButeMgrHashCompare > TNameMap;

#else

	struct eqstr_nocase
	{
		bool operator()(const char* s1, const char* s2) const
		{
			return stricmp(s1, s2) == 0;
		}
	};

	struct hash_str_nocase
	{
		unsigned long operator()(const char* str) const
		{
			unsigned long hash = 0;
			for ( ; *str; ++str)
				hash = 5*hash + tolower(*str);

			return hash;
		}
	};

	// The map for storing the name/GameBaseLite* association
	// Note : I happen to know that the lite game object names are stored in the object...
	// If that changes, the const char * part of this would need changing...
	typedef std::hash_map<const char *, GameBaseLite *, hash_str_nocase, eqstr_nocase> TNameMap;

#endif // VC7

private:
	enum { k_nInvalidSerializeID = 0xFFFFFFFF };

private:

	bool RemoveObjectFromList(TObjectList &aList, GameBaseLite *pObject);
	bool IsObjectInList(TObjectList &aList, GameBaseLite *pObject);

	void CleanList(TObjectList &aList);

	void CleanObjectLists();

	void CleanSerializeIDs();

	enum EDirtyFlags {
		eDirty_SerializeIDs = 1,
		eDirty_ObjectLists = 2,
		eDirty_All = 0xFFFFFFFF
	};
	void SetDirty(EDirtyFlags eDirty = eDirty_All, bool bYesNo = true);

	void AddActiveObject(GameBaseLite *pObject);

	void AddInactiveObject(GameBaseLite *pObject);

	void RemoveObjectList(TObjectList &aList);

	GameBaseLite *LoadObjectInfo(ILTMessage_Read *pMsg, uint32 nSerializeID);
	void SaveObjectInfo(ILTMessage_Write *pMsg, GameBaseLite *pObject);

	void HandlePendingInitialUpdates();
	void HandlePendingDeletes();

	void ShowInfo(uint32 nUpdateTime);
private:
	// The main object lists
	TObjectList m_aActiveObjects;
	uint32 m_nNumActiveObjects;
	TObjectList m_aInactiveObjects;
	uint32 m_nNumInactiveObjects;

	// The initial update list
	TObjectList m_aInitialUpdateObjects;
	TObjectList m_aInitialUpdateObjectsLoad;

	// The pending delete list
	TObjectList m_aDeleteObjects;

	// The object dictionary
	TNameMap m_aNameMap;

	// Are the object serialization ID's dirty?
	bool m_bSerializeIDsDirty;
	// Are the object lists dirty?
	bool m_bObjectListsDirty;
};

#endif // __LITE_OBJECT_MGR_H__

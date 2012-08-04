////////////////////////////////////////////////////////////////
//
// hotkeydb.h
//
// This is the maintainer of the hotkeys, that basically maintains
// a list of all the hotkeys. It supports most list management
// features such as adding, removing, and finding elements. In
// addition it supports loading and saving to a stream, file
// or the registry.
//
// Author: John O'Rorke
// Created: 7/14/00
// Modification History:
//
////////////////////////////////////////////////////////////////
#ifndef __HOTKEYDB_H__
#define __HOTKEYDB_H__

#include "hotkey.h"
#include "uitracker.h"
#include "trackermgr.h"
#include "genregmgr.h"

class CHotKeyDB
{
public:

	CHotKeyDB();
	CHotKeyDB(const CHotKeyDB& rhs);

	~CHotKeyDB();

	//sets the specified HotKey to the given key shortcut, and can
	//fail if not enough room is available
	bool			SetHotKey(const CHotKey& HotKey);

	//gets the specified HotKey. Note that it is constant since it
	//cannot be directly modified, it must instead go through SetHotKey,
	//so the actual ideal method is to get a hotkey, copy it into a different
	//one, make modifications, and then call SetHotKey, this is to ensure that
	//the UITracker never gets out of sync
	//Can return LTNULL if the hotkey is not found
	inline const CHotKey*	GetHotKey(const char* pszHotKeyName) const;

	//returns the specified hot key. Note, this is only for iteration through
	//and if a hotkey is added or removed, the order cannot be guranteed,
	//see comments from above function for further issues
	const CHotKey*	GetHotKey(uint32 nHotKey) const;

	//retrieves the number of hotkeys currently registered
	uint32			GetNumHotKeys() const		{ return m_nNumHotKeys; }

	//removes the specified hotkey from the list. Returns false if it
	//cannot find it
	bool			RemoveHotKey(const char* pszHotKeyName);
	
	//clears all keys in the database
	void			ClearKeys();

	//saves the database to a specified file
	bool			Save(const char* pszFilename) const;

	//saves the database to a specified stream
#if _MSC_VER >= 1300
	void			Save(std::ostream& OutFile) const;
#else
	void			Save(ostream& OutFile) const;
#endif

	//loads the database from a specified file. Will have keys of same value
	//overwrite each other, so it can be initialized from the global database,
	//then have keys loaded over the top, ensuring that the configuration is not
	//lost, and that new keys can be added. If this is not desired, pass true in
	//for clear keys
	bool			Load(const char* pszFilename, bool bClearKeys);

	//loads the database from a specified stream. See comments above
#if _MSC_VER >= 1300
	bool			Load(std::istream& InFile, bool bClearKeys);
#else
	bool			Load(istream& InFile, bool bClearKeys);
#endif
	//loads the database from the registry, the reg dir is in
	//the form of \dir\dir\DirToUse
	//If it finds registry items with the same names as keys already in the
	//DB, it will use those keys to overwrite the old keys, so that the
	//registry does not have to contain all keys, and new keys can be added
	//without requiring the user to reset the DB. If this is not desired,
	//pass TRUE to clear keys, so only the registry items will be listed
	bool			LoadFromRegistry(CGenRegMgr& RegMgr, const char* pszRegDir, bool bClearKeys);

	//saves the database to the registry
	//the form of \dir\dir\DirToUse
	bool			SaveToRegistry(CGenRegMgr& RegMgr, const char* pszRegDir) const;

	//assignment operator
	const CHotKeyDB&	operator=(const CHotKeyDB& rhs);

private:

	enum		{	MAX_HOT_KEYS		= 350	};

	uint32			m_nNumHotKeys;
	CHotKey			m_HotKeys[MAX_HOT_KEYS];

};

//gets the specified HotKey. Note that it is constant since it
//cannot be directly modified, it must instead go through SetHotKey,
//so the actual ideal method is to get a hotkey, copy it into a different
//one, make modifications, and then call SetHotKey, this is to ensure that
//the UITracker never gets out of sync
//Can return LTNULL if the hotkey is not found
const CHotKey* CHotKeyDB::GetHotKey(const char* pszHotKeyName) const
{
	for(uint32 nCurrKey = 0; nCurrKey < GetNumHotKeys(); nCurrKey++)
	{
		if(strcmp(pszHotKeyName, m_HotKeys[nCurrKey].GetEventName()) == 0)
		{
			//found it, lets tell them
			return &m_HotKeys[nCurrKey];
		}
	}

	//didn't actually find it
	return LTNULL;
}

#endif


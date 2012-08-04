#include "hotkeydb.h"
#include "rvtracker.h"
#include "globalhotkeydb.h"
#if _MSC_VER >= 1300
#include <fstream>
#include <istream>
#else
#include <fstream.h>
#include <istream.h>
#endif

#define REG_NUM_HOTKEYS		"NumHotKeys"

CHotKeyDB::CHotKeyDB() :
	m_nNumHotKeys(0)
{
}

CHotKeyDB::CHotKeyDB(const CHotKeyDB& rhs) :
	m_nNumHotKeys(0)
{
	*this = rhs;
}
	

CHotKeyDB::~CHotKeyDB()
{
}

//sets the specified HotKey to the given key shortcut, and can
//fail if not enough room is available
bool CHotKeyDB::SetHotKey(const CHotKey& HotKey)
{
	//see if there is already an entry in place for the hotkey
	for(uint32 nCurrKey = 0; nCurrKey < GetNumHotKeys(); nCurrKey++)
	{
		if(stricmp(HotKey.GetEventName(), m_HotKeys[nCurrKey].GetEventName()) == 0)
		{
			//there is one already, set it and signal success
			m_HotKeys[nCurrKey] = HotKey;

			return true;
		}
	}
	//didn't find one, so we need to add this event
	if(GetNumHotKeys() >= MAX_HOT_KEYS)
	{
		//don't have room to add it though
		return false;
	}

	m_HotKeys[GetNumHotKeys()] = HotKey;
	m_nNumHotKeys++;

	return true;
}

//returns the specified hot key. Note, this is only for iteration through
//and if a hotkey is added or removed, the order cannot be guranteed,
//see comments from above function for further issues
const CHotKey* CHotKeyDB::GetHotKey(uint32 nHotKey) const
{
	//just make sure the index is within range
	if(nHotKey < GetNumHotKeys())
	{
		return &m_HotKeys[nHotKey];
	}

	return LTNULL;
}

//removes the specified hotkey from the list. Returns false if it
//cannot find it
bool CHotKeyDB::RemoveHotKey(const char* pszHotKeyName)
{
	//first lets try and find it to remove it
	for(uint32 nCurrKey = 0; nCurrKey < GetNumHotKeys(); nCurrKey++)
	{
		if(stricmp(pszHotKeyName, m_HotKeys[nCurrKey].GetEventName()) == 0)
		{
			//now we need to go through the rest of the list and move all
			//the hotkeys back
			for(uint32 nMoveKey = nCurrKey + 1; nMoveKey < GetNumHotKeys(); nMoveKey++)
			{
				m_HotKeys[nMoveKey - 1] = m_HotKeys[nMoveKey];
			}
			
			m_nNumHotKeys--;

			return true;
		}
	}
	//didn't find it, signal failure
	return false;

}

//saves the database to a specified file
bool CHotKeyDB::Save(const char* pszFilename) const
{
	//open the file
#if _MSC_VER >= 1300
	std::ofstream OutFile(pszFilename);
#else
	ofstream OutFile(pszFilename);
#endif

	//make sure it could open
	if(OutFile.fail())
	{
		return false;
	}

	//save to the stream
	Save(OutFile);

	//close it
	OutFile.close();

	return true;
}

//saves the database to a specified stream
#if _MSC_VER >= 1300
void CHotKeyDB::Save(std::ostream& OutFile) const
#else
void CHotKeyDB::Save(ostream& OutFile) const
#endif
{

#if _MSC_VER >= 1300
	OutFile << GetNumHotKeys() << std::endl;
#else
	OutFile << GetNumHotKeys() << endl;
#endif

	for(uint32 nCurrKey = 0; nCurrKey < GetNumHotKeys(); nCurrKey++)
	{
		m_HotKeys[nCurrKey].Save(OutFile);
	}
}

//loads the database from a specified file
bool CHotKeyDB::Load(const char* pszFilename, bool bClearKeys)
{
	//open the file
#if _MSC_VER >= 1300
	std::ifstream InFile(pszFilename);
#else
	ifstream InFile(pszFilename);
#endif

	//make sure it could open
	if(InFile.fail())
	{
		return false;
	}

	//save to the stream
	Load(InFile, bClearKeys);

	//close it
	InFile.close();

	return true;
}

//loads the database from a specified stream
#if _MSC_VER >= 1300
bool CHotKeyDB::Load(std::istream& InFile, bool bClearKeys)
#else
bool CHotKeyDB::Load(istream& InFile, bool bClearKeys)
#endif
{
	if(bClearKeys)
	{
		ClearKeys();
	}

	uint32 nNumHotKeys;
	InFile >> nNumHotKeys;

	for(uint32 nCurrKey = 0; nCurrKey < nNumHotKeys; nCurrKey++)
	{
		//load in the name of this key
		//read in the event name
		uint32 nEventNameLen;
		InFile >> nEventNameLen;

		CString sNameBuffer;

		//skip past the newline
		InFile.get();

		//read in the specified number of characters
		for(uint32 nCurrChar = 0; nCurrChar < nEventNameLen; nCurrChar++)
		{
			sNameBuffer += (char)InFile.get();
		}

		//now we need to see if this is a valid key
		if(!CGlobalHotKeyDB::IsValidKeyName(sNameBuffer))
		{
			//invalid key name, don't add it
			continue;
		}

		//see if this hotkey already exists in the database
		CHotKey* pHotKey = (CHotKey*)GetHotKey(sNameBuffer);

		if(pHotKey)
		{
			//we have a match, just have that one load anew
			if(pHotKey->Load(InFile, sNameBuffer) == false)
			{
				return false;
			}
		}
		else
		{
			//make sure we have room to load it
			if(GetNumHotKeys() >= MAX_HOT_KEYS)
			{
				//the max needs to be increased
				ASSERT(false);
				return false;
			}

			//no match, we need to make a new one
			if(m_HotKeys[GetNumHotKeys()].Load(InFile, sNameBuffer) == false)
			{
				return false;
			}
			m_nNumHotKeys++;
		}
	}
	return true;
}


//loads the database from the registry, the reg dir is in
//the form of \dir\dir\DirToUse
bool CHotKeyDB::LoadFromRegistry(CGenRegMgr& RegMgr, const char* pszRegDir, bool bClearKeys)
{
	//see if we want to clear the existing keys
	if(bClearKeys)
	{
		ClearKeys();
	}

	//read in the number of keys
	DWORD nNumKeys;
	if(RegMgr.GetDwordValue (pszRegDir, REG_NUM_HOTKEYS, &nNumKeys) == false)
	{
		return false;
	}

	//now read in each key
	for(uint32 nCurrKey = 0; nCurrKey < nNumKeys; nCurrKey++)
	{
		CString sHotKeyName;
		sHotKeyName.Format("HotKey%d", nCurrKey);

		static const uint32 nBuffSize = 512;
		char pszTextBuff[nBuffSize];

		//read in the name of this hotkey
		if(RegMgr.GetValue(pszRegDir, sHotKeyName + "Name", pszTextBuff, nBuffSize))
		{
			//now we need to see if this is a valid key
			if(!CGlobalHotKeyDB::IsValidKeyName(pszTextBuff))
			{
				//invalid key name, don't add it
				continue;
			}

			//see if we already have one in the listing
			CHotKey* pMatch = (CHotKey*)GetHotKey(pszTextBuff);

			if(pMatch)
			{
				//we have a match, we want to load this from the registry over the old key
				pMatch->LoadFromRegistry(RegMgr, pszRegDir, sHotKeyName);
			}
			else
			{
				//make sure that we have room to add this hotkey
				if(m_nNumHotKeys >= MAX_HOT_KEYS)
				{
					//the max needs to be increased
					ASSERT(false);
					return false;
				}

				//load this new key in
				if(m_HotKeys[m_nNumHotKeys].LoadFromRegistry(RegMgr, pszRegDir, sHotKeyName) == false)
				{
					return false;
				}
				//we added a key
				m_nNumHotKeys++;
			}
		}		
	}

	return true;
}

//saves the database to the registry
//the form of \dir\dir\DirToUse
bool CHotKeyDB::SaveToRegistry(CGenRegMgr& RegMgr, const char* pszRegDir) const
{
	//first create the directory key
	RegMgr.CreateKey(pszRegDir);

	//now save out the number of keys
	if(RegMgr.SetDwordValue(pszRegDir, REG_NUM_HOTKEYS, GetNumHotKeys()) == false)
	{
		return false;
	}

	//now write out each key
	for(uint32 nCurrKey = 0; nCurrKey < GetNumHotKeys(); nCurrKey++)
	{
		CString sHotKeyName;
		sHotKeyName.Format("HotKey%d", nCurrKey);

		if(m_HotKeys[nCurrKey].SaveToRegistry(RegMgr, pszRegDir, sHotKeyName) == false)
		{
			return false;
		}
	}
	
	return true;
}


//clears all keys in the database
void CHotKeyDB::ClearKeys()
{
	m_nNumHotKeys = 0;
}

//assignment operator
const CHotKeyDB& CHotKeyDB::operator=(const CHotKeyDB& rhs)
{
	//start fresh
	ClearKeys();

	//just run through and copy over each element in the array
	for(UINT32 nCurrKey = 0; nCurrKey < rhs.GetNumHotKeys(); nCurrKey++)
	{
		m_HotKeys[nCurrKey] = *(rhs.GetHotKey(nCurrKey));
	}

	//make sure the number of keys is in sync
	m_nNumHotKeys = rhs.GetNumHotKeys();

	return *this;
}




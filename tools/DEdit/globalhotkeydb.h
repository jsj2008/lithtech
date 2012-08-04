////////////////////////////////////////////////////////////////
//
// globalhotkeydb.h
//
// functions for manipulating the global hot key database, as well as 
// setting defaults for variables, and initializing a database
//
// Author: John O'Rorke
// Created: 7/10/00
// Modification History:
//
////////////////////////////////////////////////////////////////
#ifndef __GLOBALHOTKEYDB_H__
#define __GLOBALHOTKEYDB_H__

#include "hotkeydb.h"

class CKeyDefaultAggregate;

class CGlobalHotKeyDB
{
public:
	//initializes the global hotkey database to the defaults
	static void				Init();

	//sets the specified database to the defaults
	static void				SetDefaults(CHotKeyDB& DB);

	//sets the default to a single key. This will return false if it fails
	//to match the key with any of the registered UIE's
	static bool			SetKeyDefault(CHotKey& Key);

	//gets the text for a menu item from the hotkey database
	static CString			GetMenuTextFromHotKey(const char* pszHotKeyName);

	//this will clear the list of aggregates, and free their memory.
	static void				ClearAggregateList();

	//this will add an aggregate into the list. This will orphan the 
	//object, which must be allocated with new
	static void				AddAggregate(CKeyDefaultAggregate* pAggregate);

	//determines if the name passed in is a valid hotkey for the default configuration
	static bool			IsValidKeyName(const char* pszKeyName);

	//the global hotkey database
	static CHotKeyDB		m_DB;

private:

	//the aggregate default key setter (this allows for different
	//default configurations)
	static CKeyDefaultAggregate*	m_pAggregate;

};

#endif


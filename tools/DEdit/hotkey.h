////////////////////////////////////////////////////////////////
//
// hotkey.h
//
// The hotkey class manages handles a single event, and the 
// appropriate event trigger to fire that event. It maintains
// two lists, one for starting, and one for ending. By default,
// most keys don't need to worry about lists UNLESS the end list
// is not the direct opposite of the start list. It also manages
// saving and loading to a stream or the registry
//
// Author: John O'Rorke
// Created: 7/14/00
// Modification History:
//
////////////////////////////////////////////////////////////////
#ifndef __HOTKEY_H__
#define __HOTKEY_H__

#include "bdefs.h"
#include "uievent.h"
#include "genregmgr.h"

#if _MSC_VER >= 1300
#include <iostream>
#else
#include <iostream.h>
#endif

class CHotKey
{
public:

	//default constructor
	CHotKey();
	//copy constructor
	CHotKey(const CHotKey& rhs);

	~CHotKey();

	//sorts the hotkeys events so that they will always be listed in the same
	//order, and make the menus look consistant
	void		SortEvents();

	//checks to see if this hotkey and the passed in hotkey contain the
	//same starting events
	bool		HasSameStartingEvents(const CHotKey& rhs) const;

	//adds a key to the list of events needed to trigger the event
	//It also adds the opposite event to the end list (such us KEYUP,
	//would add a KEYDOWN event to the end)
	//returns true if it successful, or false if there is
	//no more room to store the event
	bool		AddEvent(const CUIEvent& Event);

	//determines if the hotkey contains the specified event in
	//the starting list
	//returns true if it does, false otherwise
	bool		ContainsEvent(const CUIEvent& Event) const;

	//gets a certain event from the starting list
	const CUIEvent&		GetEvent(uint32 nIndex) const;

	//gets the index of a specified event. It will return -1 if the event is not
	//found
	int32				GetEventIndex(const CUIEvent& Event) const;

	//gets the number of keys used by this hotkey
	//(uses the starting list)
	uint32		GetNumEvents() const;

	//clears the list of keys in both lists
	void		ResetEvents();

	//get the name of the event this hotkey is linked to
	const char*	GetEventName() const		{ return m_pszEventName; }

	//sets the name of the event, fails if it cannot set the name,
	//and returns false
	bool		SetEventName(const char* pszEventName);

	//get the description of the event this hotkey is linked to
	const char*	GetDescription() const;

	//sets the description of the event, fails if it cannot set the name,
	//and returns false
	bool		SetDescription(const char* pszDescription);

	//saves the hotkey to the specified stream
#if _MSC_VER >= 1300
	void		Save(std::ostream& OutFile) const;
#else
	void		Save(ostream& OutFile) const;
#endif

	//loads the key from the registry
	bool		LoadFromRegistry(CGenRegMgr& RegMgr, const char* pszRegDir, const char* pszName);

	//saves the key to the registry
	bool		SaveToRegistry(CGenRegMgr& RegMgr, const char* pszRegDir, const char* pszName) const;

	//sets whether or not the user can modify it
	void		SetUserChangable(bool bChangable);
	//determines whether or not the user can modify it
	bool		IsUserChangable() const;

	//loads the hotkey from the specified stream
#if _MSC_VER >= 1300
	bool		Load(std::istream& InFile);
#else
	bool		Load(istream& InFile);
#endif
	//loads the hotkey from the specified stream, except for the name
	//(this is so the hotkey database can load the name ahead of time
	//and do checks for duplicates)
#if _MSC_VER >= 1300
	bool		Load(std::istream& InFile, const char* pszName);
#else
	bool		Load(istream& InFile, const char* pszName);
#endif
	//returns the starting event list
	const CUIEventList&	GetStartEventList() const	{return m_StartEventList;}

	//returns the ending event list
	const CUIEventList&	GetEndEventList() const		{return m_EndEventList;}

	//clears out the start event list
	void ClearStartEventList();

	//clears out the end event list
	void ClearEndEventList();

	//assignment operator
	const CHotKey& operator=(const CHotKey& rhs);

	//a list of events
	CUIEventList	m_StartEventList;
	CUIEventList	m_EndEventList;

private:

	//deletes the old string and resizes the length of it,
	//ensures nothing about the memory it allocates
	bool		ResizeString(uint32 nLength, char** pszString);

	//creates an event given the type and a value that is context sensitive
	//based upon the type. Returns LTNULL if the type is not expected
	static CUIEvent*	CreateEvent(uint32 nType, uint32 nValue);

	//given a specific event, it will get the appropriate value associated
	//with the type of the event
	static uint32		GetEventValue(const CUIEvent& Event);

	//determines whether or not this can be remapped by the user
	bool		m_bUserChangable;

	//the name of the event this is linked to
	char*		m_pszEventName;

	//the description of what this hotkey does
	char*		m_pszDescription;


};


#endif


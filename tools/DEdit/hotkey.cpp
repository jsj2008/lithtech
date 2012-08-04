#include "hotkey.h"

CHotKey::CHotKey() :
	m_pszDescription(LTNULL),
	m_pszEventName(LTNULL),
	m_bUserChangable(true)
{
}

CHotKey::CHotKey(const CHotKey& rhs) :
	m_pszDescription(LTNULL),
	m_pszEventName(LTNULL),
	m_bUserChangable(true)
{
	*this = rhs;
}

CHotKey::~CHotKey()
{
	delete [] m_pszEventName;
	delete [] m_pszDescription;

	ResetEvents();
}


//adds a key to the list of keys needed to trigger the event
//returns true if it successful, or false if there is
//no more room to store the key
bool CHotKey::AddEvent(const CUIEvent& Event)
{
	//clone it and add it to the starting list
	CUIEvent* pClone = Event.Clone();

	//ensure it worked properly
	if(pClone == LTNULL)
	{
		return false;
	}

	//add it to the starting list
	m_StartEventList.Add(pClone);

	//find the inverse
	int nInverse = UIEVENT_NONE;

	switch(pClone->GetType())
	{
	case UIEVENT_KEYDOWN:	nInverse = UIEVENT_KEYUP;		break;
	case UIEVENT_KEYUP:		nInverse = UIEVENT_KEYDOWN;		break;
	case UIEVENT_MOUSEDOWN:	nInverse = UIEVENT_MOUSEUP;		break;
	case UIEVENT_MOUSEUP:	nInverse = UIEVENT_MOUSEDOWN;	break;
	default:				nInverse = UIEVENT_NONE;		break;
	}

	//if it was a valid inverse, lets add it to the end list
	if(nInverse != UIEVENT_NONE)
	{
		//add another clone
		pClone = Event.Clone();

		if(pClone)
		{
			//but switch this one to the inverse
			pClone->SetType(nInverse);

			m_EndEventList.Add(pClone);
		}
	}

	return true;
}

//checks to see if this hotkey and the passed in hotkey contain the
//same starting events
bool CHotKey::HasSameStartingEvents(const CHotKey& rhs) const
{
	//do a size check
	if(m_StartEventList.GetSize() != rhs.m_StartEventList.GetSize())
		return false;

	//now we run through and make sure we can find every starting event in
	//the other entry
	for(uint32 nCurrEvent = 0; nCurrEvent < m_StartEventList.GetSize(); nCurrEvent++)
	{
		if(rhs.ContainsEvent(*(m_StartEventList[nCurrEvent])) == false)
		{
			//not a match
			return false;
		}
	}

	return true;
}

//callback for sorting events
static int SortKeysCallback(const void* pElem1, const void* pElem2)
{
	//get the elements
	ASSERT(pElem1);
	ASSERT(pElem2);

	CUIEvent* pEvent1 = *((CUIEvent**)pElem1);
	CUIEvent* pEvent2 = *((CUIEvent**)pElem2);

	//the order of events should go: Ctrl - Shift - Keys - Mouse - Other

	//create a score for each event
	int32 nEvent1Score = 0;

	switch(pEvent1->GetType())
	{
	case UIEVENT_KEYDOWN:
	case UIEVENT_KEYUP:
		{
			CUIKeyEvent* pKey = ((CUIKeyEvent*)pEvent1);
			switch(pKey->GetCode())
			{
			case VK_CONTROL:	nEvent1Score = 0;						break;
			case VK_SHIFT:		nEvent1Score = 1;						break;
			default:			nEvent1Score = 50 + pKey->GetCode();	break;
			}
		}
		break;
	case UIEVENT_MOUSEDOWN:
	case UIEVENT_MOUSEUP:
		nEvent1Score = 10000 + ((CUIMouseEvent*)pEvent1)->GetButton();
		break;
	default:
		nEvent1Score = 100000;
		break;
	}

	//create a score for each event
	int32 nEvent2Score = 0;

	switch(pEvent2->GetType())
	{
	case UIEVENT_KEYDOWN:
	case UIEVENT_KEYUP:
		{
			CUIKeyEvent* pKey = ((CUIKeyEvent*)pEvent2);
			switch(pKey->GetCode())
			{
			case VK_CONTROL:	nEvent2Score = 0;						break;
			case VK_SHIFT:		nEvent2Score = 1;						break;
			default:			nEvent2Score = 50 + pKey->GetCode();	break;
			}
		}
		break;
	case UIEVENT_MOUSEDOWN:
	case UIEVENT_MOUSEUP:
		nEvent2Score = 10000 + ((CUIMouseEvent*)pEvent2)->GetButton();
		break;
	default:
		nEvent2Score = 100000;
		break;
	}

	return nEvent1Score - nEvent2Score;
}

//sorts the hotkeys events so that they will always be listed in the same
//order, and make the menus look consistant
void CHotKey::SortEvents()
{
	//create a buffer
	CUIEvent* pEventList[256];

	//copy all the keys into it
	uint32 nCurrKey;
	for(nCurrKey = 0; nCurrKey < GetNumEvents(); nCurrKey++)
	{
		pEventList[nCurrKey] = m_StartEventList[nCurrKey];
	}

	::qsort(&pEventList, GetNumEvents(), sizeof(CUIEvent*), SortKeysCallback);

	//now copy the list back
	for(nCurrKey = 0; nCurrKey < GetNumEvents(); nCurrKey++)
	{
		m_StartEventList[nCurrKey] = pEventList[nCurrKey];
	}

}

//gets the index of a specified event. It will return -1 if the event is not
//found
int32 CHotKey::GetEventIndex(const CUIEvent& Event) const
{
	for(uint32 nCurrEvent = 0; nCurrEvent < GetNumEvents(); nCurrEvent++)
	{
		if(*(m_StartEventList[nCurrEvent]) == Event)
		{
			return nCurrEvent;
		}
	}
	return -1;
}

//determines if the hotkey contains the specified key
//returns true if it does, false otherwise
bool CHotKey::ContainsEvent(const CUIEvent& Event) const
{
	return (GetEventIndex(Event) == -1) ? false : true;	
}

//gets a certain key
const CUIEvent& CHotKey::GetEvent(uint32 nIndex) const
{
	//note, this could cause bad problems if index is out of range
	return *(m_StartEventList[nIndex]);
}

//gets the number of keys used by this hotkey
uint32 CHotKey::GetNumEvents() const
{
	return m_StartEventList.GetSize();
}

//clears the list of keys
void CHotKey::ResetEvents()
{
	//delete all the elements
	ClearStartEventList();
	ClearEndEventList();
}

//clears out the start event list
void CHotKey::ClearStartEventList()
{
	for(UINT32 nCurrStart = 0; nCurrStart < m_StartEventList.GetSize(); nCurrStart++)
	{
		delete m_StartEventList[nCurrStart];
	}
	m_StartEventList.RemoveAll();
}

//clears out the end event list
void CHotKey::ClearEndEventList()
{
	for(UINT32 nCurrEnd = 0; nCurrEnd < m_EndEventList.GetSize(); nCurrEnd++)
	{
		delete m_EndEventList[nCurrEnd];
	}

	m_EndEventList.RemoveAll();
}

//sets the name of the event, fails if it cannot set the name,
//and returns false
bool CHotKey::SetEventName(const char* pszEventName)
{
	if(pszEventName)
	{
		if(ResizeString(strlen(pszEventName) + 1, &m_pszEventName))
		{
			strcpy(m_pszEventName, pszEventName);
			return true;
		}
		return false;
	}

	return true;
}

//get the description of the event this hotkey is linked to
const char*	CHotKey::GetDescription() const
{
	return m_pszDescription;
}

//sets the description of the event, fails if it cannot set the name,
//and returns false
bool CHotKey::SetDescription(const char* pszDescription)
{
	if(pszDescription)
	{
		if(ResizeString(strlen(pszDescription) + 1, &m_pszDescription))
		{
			strcpy(m_pszDescription, pszDescription);
			return true;
		}
		return false;
	}

	return true;
}



//saves the hotkey to the specified stream
#if _MSC_VER >= 1300
void CHotKey::Save(std::ostream& OutFile) const
#else
void CHotKey::Save(ostream& OutFile) const
#endif
{
	//write out the event name
#if _MSC_VER >= 1300
	OutFile << strlen(GetEventName()) << std::endl; 
	OutFile << GetEventName() << std::endl;

	//write out the event name
	OutFile << strlen(GetDescription()) << std::endl;
	OutFile << GetDescription() << std::endl;

	//write out if it is modifiable or not
	OutFile << ((IsUserChangable()) ? 1 : 0) << std::endl;
#else
	OutFile << strlen(GetEventName()) << endl; 
	OutFile << GetEventName() << endl;

	//write out the event name
	OutFile << strlen(GetDescription()) << endl;
	OutFile << GetDescription() << endl;

	//write out if it is modifiable or not
	OutFile << ((IsUserChangable()) ? 1 : 0) << endl;
#endif

	//lists that we need to save
	const CUIEventList* pLists[] = {&m_StartEventList, &m_EndEventList};

	for(uint32 nCurrList = 0; nCurrList < sizeof(pLists) / sizeof(pLists[0]); nCurrList++)
	{
		//write out the number of events in the current list
		uint32 nNumEvents = pLists[nCurrList]->GetSize();

#if _MSC_VER >= 1300
		OutFile << nNumEvents << std::endl;
#else
		OutFile << nNumEvents << endl;
#endif

		for(uint32 nCurrEvent = 0; nCurrEvent < nNumEvents; nCurrEvent++)
		{
			//cache the event so we don't have to use this ugly thing over and over
			CUIEvent& Event = *((*pLists[nCurrList])[nCurrEvent]);

#if _MSC_VER >= 1300
			OutFile << Event.GetType() << std::endl;
			OutFile << GetEventValue(Event) << std::endl;
#else
			OutFile << Event.GetType() << endl;
			OutFile << GetEventValue(Event) << endl;
#endif
		}
	}
}

//loads the hotkey from the specified stream
#if _MSC_VER >= 1300
bool CHotKey::Load(std::istream& InFile)
#else
bool CHotKey::Load(istream& InFile)
#endif
{
	//first read in the name

	//read in the event name
	uint32 nEventNameLen;
	InFile >> nEventNameLen;

	char* pszNameBuffer = new char[nEventNameLen + 1];

	//make sure that we could allocate the memory
	if(pszNameBuffer == NULL)
	{
		return false;
	}

	//skip past the newline
	InFile.get();

	//read in the specified number of characters
	for(uint32 nCurrChar = 0; nCurrChar < nEventNameLen; nCurrChar++)
	{
		pszNameBuffer[nCurrChar] = InFile.get();
	}
	pszNameBuffer[nEventNameLen] = '\0';

	//have the other load set up the name, and load the rest
	bool bRV =  Load(InFile, pszNameBuffer);

	//clean up
	delete [] pszNameBuffer;

	return bRV;
}

//loads the hotkey from the specified stream, except for the name
//(this is so the hotkey database can load the name ahead of time
//and do checks for duplicates)
#if _MSC_VER >= 1300
bool CHotKey::Load(std::istream& InFile, const char* pszName)
#else
bool CHotKey::Load(istream& InFile, const char* pszName)
#endif
{
	ResetEvents();

	uint32 nCurrChar;

	if(ResizeString(strlen(pszName) + 1, &m_pszEventName) == false)
	{
		return false;
	}
	strcpy(m_pszEventName, pszName);


	//read in the description
	uint32 nDescriptionLen;
	InFile >> nDescriptionLen;

	//skip past the newline
	InFile.get();

	if(ResizeString(nDescriptionLen + 1, &m_pszDescription) == false)
	{
		return false;
	}

	//read in the specified number of characters
	for(nCurrChar = 0; nCurrChar < nDescriptionLen; nCurrChar++)
	{
		m_pszDescription[nCurrChar] = InFile.get();
	}
	m_pszDescription[nDescriptionLen] = '\0';

	//read in if it is user changable
	uint32 nChangable;
	InFile >> nChangable;
	SetUserChangable((nChangable) ? true : false);

	//lists that we need to load
	CUIEventList* pLists[] = {&m_StartEventList, &m_EndEventList};

	for(uint32 nCurrList = 0; nCurrList < sizeof(pLists) / sizeof(pLists[0]); nCurrList++)
	{
		//get the number of keys
		uint32 nNumEvents;
		InFile >> nNumEvents;

		//read in all the keys
		for(uint32 nCurrEvent = 0; nCurrEvent < nNumEvents; nCurrEvent++)
		{
			uint32 nType;
			uint32 nCode;

			//read in the type and code
			InFile >> nType >> nCode;
			
			//create the event
			CUIEvent* pEvent = CreateEvent(nType, nCode);

			//add it to the list if it is valid
			if(pEvent)
			{
				pLists[nCurrList]->Add(pEvent);
			}
		}
	}

	return true;
}


//deletes the old string and resizes the length of it,
//ensures nothing about the memory it allocates
bool CHotKey::ResizeString(uint32 nLength, char** pszString)
{
	//sanity check
	if(pszString == LTNULL)
	{
		return false;
	}

	//clean up the old string
	delete [] *pszString;
	*pszString = LTNULL;

	if(nLength > 0)
	{
		*pszString = new char[nLength];
	
		if(!(*pszString))
		{
			return false;
		}
	}

	return true;
}



//assignment operator
const CHotKey& CHotKey::operator=(const CHotKey& rhs)
{
	ResetEvents();

	if(SetEventName(rhs.GetEventName()) && SetDescription(rhs.GetDescription()))
	{
		//run through both lists and create clones of each hotkey, then
		//add them into this new list
		for(UINT32 nCurrStart = 0; nCurrStart < rhs.m_StartEventList.GetSize(); nCurrStart++)
		{
			m_StartEventList.Add(rhs.m_StartEventList[nCurrStart]->Clone());
		}
		for(UINT32 nCurrEnd = 0; nCurrEnd < rhs.m_EndEventList.GetSize(); nCurrEnd++)
		{
			m_EndEventList.Add(rhs.m_EndEventList[nCurrEnd]->Clone());
		}

		SetUserChangable(rhs.IsUserChangable());
	}

	return *this;
}

//sets whether or not the user can modify it
void CHotKey::SetUserChangable(bool bChangable)
{
	m_bUserChangable = bChangable;
}

//determines whether or not the user can modify it
bool CHotKey::IsUserChangable() const
{
	return m_bUserChangable;
}

//loads the key from the registry
bool CHotKey::LoadFromRegistry(CGenRegMgr& RegMgr, const char* pszRegDir, const char* pszName)
{
	
	ResetEvents();

	CString sKeyName;


	static const uint32 nBuffSize = 512;
	char pszTextBuff[nBuffSize];

	//read in the name of this hotkey
	sKeyName.Format("%sName", pszName);
	if(RegMgr.GetValue(pszRegDir, sKeyName, pszTextBuff, nBuffSize))
	{
		SetEventName(pszTextBuff);
	}

	//read in the description
	sKeyName.Format("%sDescription", pszName);
	if(RegMgr.GetValue(pszRegDir, sKeyName, pszTextBuff, nBuffSize))
	{
		SetDescription(pszTextBuff);
	}

	//write if it is changable or not
	BOOL bChangable;
	sKeyName.Format("%sChangable", pszName);
	if(RegMgr.GetStringBoolValue(pszRegDir, sKeyName, &bChangable))
	{
		SetUserChangable((bChangable) ? true : false);
	}


	//lists that we need to load
	CUIEventList* pLists[] = {&m_StartEventList, &m_EndEventList};

	for(uint32 nCurrList = 0; nCurrList < sizeof(pLists) / sizeof(pLists[0]); nCurrList++)
	{
		//get the number of events for this hotkey
		uint32 nNumEvents;
		sKeyName.Format("%sList%dNumEvents", pszName, nCurrList);
		if(RegMgr.GetDwordValue(pszRegDir, sKeyName, &nNumEvents) == false)
		{
			return false;
		}
		
		//read in all the events
		for(uint32 nCurrEvent = 0; nCurrEvent < nNumEvents; nCurrEvent++)
		{

			//read in the type of the event
			uint32 nType;
			sKeyName.Format("%sList%dEvent%dType", pszName, nCurrList, nCurrEvent);
			if(RegMgr.GetDwordValue(pszRegDir, sKeyName, &nType) == false)
			{
				ResetEvents();
				return false;
			}

			//read in the extra value for the event
			uint32 nValue;
			sKeyName.Format("%sList%dEvent%dValue", pszName, nCurrList, nCurrEvent);
			if(RegMgr.GetDwordValue(pszRegDir, sKeyName, &nValue) == false)
			{
				ResetEvents();
				return false;
			}

			//now create the item and add it to the list
			CUIEvent* pEvent = CreateEvent(nType, nValue);
			if(pEvent)
			{
				pLists[nCurrList]->Add(pEvent);
			}			
		}
	}

	return true;
}

//saves the key to the registry
bool CHotKey::SaveToRegistry(CGenRegMgr& RegMgr, const char* pszRegDir, const char* pszName) const
{
	CString sKeyName;

	//write the name of the hotkey
	sKeyName.Format("%sName", pszName);
	RegMgr.SetStringValue(pszRegDir, sKeyName, (GetEventName()) ? GetEventName() : "");

	//write the description
	sKeyName.Format("%sDescription", pszName);
	RegMgr.SetStringValue(pszRegDir, sKeyName, (GetDescription()) ? GetDescription() : "");

	//write if it is changable or not
	sKeyName.Format("%sChangable", pszName);
	RegMgr.SetStringBoolValue(pszRegDir, sKeyName, IsUserChangable());

	//lists that we need to save
	const CUIEventList* pLists[] = {&m_StartEventList, &m_EndEventList};

	for(uint32 nCurrList = 0; nCurrList < sizeof(pLists) / sizeof(pLists[0]); nCurrList++)
	{
		//get the number of events in this list
		uint32 nNumEvents = pLists[nCurrList]->GetSize();

		//get the number of events for this hotkey
		sKeyName.Format("%sList%dNumEvents", pszName, nCurrList);
		RegMgr.SetDwordValue(pszRegDir, sKeyName, nNumEvents);

		//write all the events
		for(uint32 nCurrEvent = 0; nCurrEvent < nNumEvents; nCurrEvent++)
		{
			//cache the event so we don't have to use this ugly thing over and over
			CUIEvent& Event = *((*pLists[nCurrList])[nCurrEvent]);

			//write out the event type
			sKeyName.Format("%sList%dEvent%dType", pszName, nCurrList, nCurrEvent);
			RegMgr.SetDwordValue(pszRegDir, sKeyName, Event.GetType());

			//now write out the event value
			sKeyName.Format("%sList%dEvent%dValue", pszName, nCurrList, nCurrEvent);
			RegMgr.SetDwordValue(pszRegDir, sKeyName, GetEventValue(Event));

		}
	}

	return true;
}


//creates an event given the type and a value that is context sensitive
//based upon the type. Returns LTNULL if the type is not expected
CUIEvent* CHotKey::CreateEvent(uint32 nType, uint32 nValue)
{
	//the event we are going to add
	CUIEvent* pEvent = LTNULL;

	//add the event to the list
	switch(nType)
	{
	case UIEVENT_KEYDOWN:	
		pEvent = new CUIKeyEvent(UIEVENT_KEYDOWN, nValue);		
		break;
	case UIEVENT_KEYUP:		
		pEvent = new CUIKeyEvent(UIEVENT_KEYUP, nValue);		
		break;
	case UIEVENT_MOUSEDOWN:	
		pEvent = new CUIMouseEvent(UIEVENT_MOUSEDOWN, nValue); 
		break;
	case UIEVENT_MOUSEUP:	
		pEvent = new CUIMouseEvent(UIEVENT_MOUSEUP, nValue);	
		break;
	case UIEVENT_MOUSEMOVE:	
		pEvent = new CUIMouseEvent(UIEVENT_MOUSEMOVE, 0);		
		break;
	case UIEVENT_MOUSEWHEEL:
		pEvent = new CUIMouseEvent(UIEVENT_MOUSEWHEEL, 0);
		break;
	default: 
		//really shouldn't get here
		ASSERT(false);
		break;
	}

	return pEvent;
}

//given a specific event, it will get the appropriate value associated
//with the type of the event
uint32 CHotKey::GetEventValue(const CUIEvent& Event)
{
	uint32 nValue = 0;
	switch(Event.GetType())
	{
	case UIEVENT_KEYDOWN:
	case UIEVENT_KEYUP:
		nValue = ((CUIKeyEvent&)Event).GetCode();
		break;
	case UIEVENT_MOUSEDOWN:
	case UIEVENT_MOUSEUP:
		nValue = ((CUIMouseEvent&)Event).GetButton();
		break;
	default:
		break;
	}

	return nValue;
}

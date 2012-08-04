#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "packerproperty.h"

//------------------------------------------------------------
// Helpers

void FreeString(char*& pszStr)
{
	delete [] pszStr;
	pszStr = NULL;
}

char* CloneString(const char* pszStr, bool bStripQuotes)
{
	const char* pCurr = pszStr;

	//skip over any beginning quote
	if(bStripQuotes && pszStr[0] == '\"')
		pCurr++;

	//see if we have an ending quote
	uint32 nStrLen = strlen(pCurr);

	char* pRV = new char[nStrLen + 1];
	
	if(pRV)
	{
		strcpy(pRV, pszStr);

		//see if it ends with a quote
		if(bStripQuotes && (nStrLen > 0) && (pRV[nStrLen - 1] == '\"'))
		{
			pRV[nStrLen - 1] = '\0';
		}
	}

	return pRV;
}

		


//------------------------------------------------------------
// Bool Property
CPackerBoolProperty::CPackerBoolProperty() :
	m_bVal(false)
{
}

CPackerBoolProperty::CPackerBoolProperty(const char* pszName, bool bValue, const char* pszHelp) :
	CPackerProperty(pszName, pszHelp),
	m_bVal(bValue)
{
}

CPackerBoolProperty::CPackerBoolProperty(const CPackerBoolProperty& rhs)
{
	*this = rhs;
}

CPackerBoolProperty& CPackerBoolProperty::operator=(const CPackerBoolProperty& rhs)
{
	SetName(rhs.GetName());
	SetHelp(rhs.GetHelp());
	SetEnabled(rhs.IsEnabled());
	SetValue(rhs.GetValue());

	return *this;
}

uint32 CPackerBoolProperty::GetType() const
{
	return PROPERTY_BOOL;
}

bool CPackerBoolProperty::SaveValue(char* pszBuffer, uint32 nBufferLen) const
{
	strncpy(pszBuffer, GetValue() ? "TRUE" : "FALSE", nBufferLen);
	return true;
}

bool CPackerBoolProperty::LoadValue(char** ppszBuffer, uint32 nNumBuffers)
{
	assert(ppszBuffer);
	if(nNumBuffers == 0)
	{
		return false;
	}

	//see if this first buffer is true or some variant
	if((stricmp(ppszBuffer[0], "TRUE") == 0) || (stricmp(ppszBuffer[0], "T") == 0))
	{
		SetValue(true);
	}
	else
	{
		//default to false
		SetValue(false);
	}

	return true;
}

bool CPackerBoolProperty::GetValue() const
{
	return m_bVal;
}

void CPackerBoolProperty::SetValue(bool bVal)
{
	m_bVal = bVal;
}

//------------------------------------------------------------
// String Property
CPackerStringProperty::CPackerStringProperty() : 
	m_bFilename(false),
	m_pszFilter(NULL),
	m_pszVal(NULL),
	m_bFileLoad(false)
{
}

CPackerStringProperty::CPackerStringProperty(	const char* pszName, const char* pszValue, 
												const char* pszHelp) : 
	CPackerProperty(pszName, pszHelp),
	m_bFilename(false),
	m_pszFilter(NULL),
	m_pszVal(NULL),
	m_bFileLoad(false)
{
	SetValue(pszValue);
}

CPackerStringProperty::CPackerStringProperty(	const char* pszName, const char* pszValue, bool bLoad,
												bool bStripFilename, const char* pszFileFilter, 
												const char* pszHelp) : 
	CPackerProperty(pszName, pszHelp),
	m_bFilename(false),
	m_pszFilter(NULL),
	m_pszVal(NULL),
	m_bFileLoad(false),
	m_bStripFilename(false)
{
	SetValue(pszValue);
	SetAsFilename(bLoad, bStripFilename, pszFileFilter);
}

CPackerStringProperty::CPackerStringProperty(const CPackerStringProperty& rhs) : 
	m_pszFilter(NULL),
	m_pszVal(NULL)
{
	*this = rhs;
}

CPackerStringProperty& CPackerStringProperty::operator=(const CPackerStringProperty& rhs)
{
	SetName(rhs.GetName());
	SetHelp(rhs.GetHelp());
	SetEnabled(rhs.IsEnabled());
	SetValue(rhs.GetValue());
	
	if(rhs.IsFilename())
	{
		SetAsFilename(rhs.IsFileLoad(), rhs.ShouldStripFilename(), rhs.GetFileFilter());
	}
	else
	{
		ClearAsFilename();
	}

	return *this;
}


CPackerStringProperty::~CPackerStringProperty()
{
	FreeString(m_pszVal);
	FreeString(m_pszFilter);
}

uint32 CPackerStringProperty::GetType() const
{
	return PROPERTY_STRING;
}

bool CPackerStringProperty::SaveValue(char* pszBuffer, uint32 nBufferLen) const
{
	//safety check
	if(nBufferLen == 0)
		return true;

	//write out the file with quotes
	strncpy(pszBuffer, "\"", nBufferLen);

	//we need to copy over the string, but convert '\' to a '\\' so it will not mess up
	//any quotes
	if(GetValue())
	{
		uint32 nStrLen = strlen(GetValue());
		uint32 nOutLen = 1;
		for(uint32 nCurrChar = 0; (nCurrChar < nStrLen) && (nOutLen < nBufferLen - 1); nCurrChar++)
		{
			if(GetValue()[nCurrChar] == '\\')
			{
				strncat(pszBuffer, "\\\\", nBufferLen);
				nOutLen += 2;
			}
			else
			{
				strncat(pszBuffer, GetValue() + nCurrChar, 1);
				nOutLen += 1;
			}
		}
	}

	strncat(pszBuffer, "\"", nBufferLen);

	//make sure the buffer is capped
	pszBuffer[nBufferLen - 1] = '\0';

	return true;
}

bool CPackerStringProperty::LoadValue(char** ppszBuffer, uint32 nNumBuffers)
{
	assert(ppszBuffer);
	if(nNumBuffers == 0)
	{
		return false;
	}

	//we need to grab the first buffer
	const char* pszBuff = ppszBuffer[0];

	FreeString(m_pszVal);
	m_pszVal = CloneString(pszBuff, true);

	return true;
}

const char*	CPackerStringProperty::GetValue() const
{
	return m_pszVal;
}

void CPackerStringProperty::SetValue(const char* pszVal)
{
	assert(pszVal);

	FreeString(m_pszVal);
	m_pszVal = CloneString(pszVal, false);
}

bool CPackerStringProperty::IsFileLoad() const
{
	return m_bFileLoad;
}

bool CPackerStringProperty::ShouldStripFilename() const
{
	return m_bStripFilename;
}

//file information
const char* CPackerStringProperty::GetFileFilter() const
{
	return m_pszFilter;
}

bool CPackerStringProperty::IsFilename() const
{
	return m_bFilename;
}

bool CPackerStringProperty::SetAsFilename(bool bLoad, bool bStripFilename, const char* pszFilter)
{
	m_bFileLoad			= bLoad;
	m_bStripFilename	= bStripFilename;

	assert(pszFilter);
	FreeString(m_pszFilter);
	m_pszFilter = CloneString(pszFilter, false);
	m_bFilename = true;

	return m_pszFilter ? true : false;
}


bool CPackerStringProperty::ClearAsFilename()
{
	FreeString(m_pszFilter);
	m_bFilename = false;

	return true;
}

//------------------------------------------------------------
// Enum Property
CPackerEnumProperty::CPackerEnumProperty() :
	m_ppszItems(NULL),
	m_nSelected(0),
	m_nNumItems(0)
{
}

CPackerEnumProperty::CPackerEnumProperty(	const char* pszName, const char* pszItems, uint32 nSel, 
											const char* pszHelp) :
	CPackerProperty(pszName, pszHelp),
	m_ppszItems(NULL),
	m_nSelected(0),
	m_nNumItems(0)
{
	SetItems(pszItems);
	SetSelection(nSel);
}

CPackerEnumProperty::CPackerEnumProperty(const CPackerEnumProperty& rhs) :
	m_ppszItems(NULL),
	m_nNumItems(0)
{
	*this = rhs;
}

CPackerEnumProperty& CPackerEnumProperty::operator=(const CPackerEnumProperty& rhs)
{
	SetName(rhs.GetName());
	SetHelp(rhs.GetHelp());
	SetEnabled(rhs.IsEnabled());
	SetItems(rhs.m_ppszItems, rhs.GetNumItems());
	SetSelection(rhs.GetSelection());	

	return *this;
}


CPackerEnumProperty::~CPackerEnumProperty()
{
	Free();
}

void CPackerEnumProperty::Free()
{
	for(uint32 nCurrSel = 0; nCurrSel < GetNumItems(); nCurrSel++)
	{
		FreeString(m_ppszItems[nCurrSel]);
	}
	delete [] m_ppszItems;

	m_ppszItems = NULL;
	m_nNumItems = 0;
}

uint32 CPackerEnumProperty::GetType() const
{
	return PROPERTY_ENUM;
}

bool CPackerEnumProperty::SaveValue(char* pszBuffer, uint32 nBufferLen) const
{
	//write out the item with quotes
	strncpy(pszBuffer, "\"", nBufferLen);

	const char* pszSelection = GetItem(GetSelection());

	if(pszSelection)
		strncat(pszBuffer, pszSelection, nBufferLen);

	strncat(pszBuffer, "\"", nBufferLen);

	return true;
}

bool CPackerEnumProperty::LoadValue(char** ppszBuffer, uint32 nNumBuffers)
{
	assert(ppszBuffer);
	if(nNumBuffers == 0)
	{
		return false;
	}

	//copy over the value in the buffer
	const char* pszBuffer = ppszBuffer[0];

	char* pszVal = CloneString(pszBuffer, true);

	//match it
	int32 nSelection = FindItem(pszVal);

	//done with this buffer, clean it up
	delete [] pszVal;
	pszVal = NULL;

	if(nSelection == -1)
		SetSelection(0);
	else
		SetSelection(nSelection);
	
	return true;
}

//sets up the items. Should be \n delimited
bool CPackerEnumProperty::SetItems(const char* pszItems)
{
	Free();

	uint32 nStrLen		= strlen(pszItems);

	//default it to one since the last item won't have a trailing newline
	uint32 nNumItems	= 1;

	//count the newlines
	uint32 nCurrChar;
	for(nCurrChar = 0; nCurrChar < nStrLen; nCurrChar++)
	{
		if(pszItems[nCurrChar] == '\n')
			nNumItems++;
	}

	//allocate our array
	m_ppszItems = new char*	[nNumItems];

	if(m_ppszItems == NULL)
		return false;

	//now we run through and build up each option string
	char pszCurrItem[MAX_ITEM_LEN + 1];
	pszCurrItem[0] = '\0';
	uint32 nBufferOff = 0;

	uint32 nCurrItem = 0;

	for(nCurrChar = 0; nCurrChar < nStrLen; nCurrChar++)
	{
		//if this is character is a newline, we need to add this item and move on
		if(pszItems[nCurrChar] == '\n')
		{
			m_ppszItems[nCurrItem] = CloneString(pszCurrItem, false);

			//move onto the next string
			nCurrItem++;
			nBufferOff = 0;

			//clear out this string
			pszCurrItem[0] = '\0';
		}
		else
		{
			//add this character and move on
			if(nBufferOff < MAX_ITEM_LEN)
			{
				pszCurrItem[nBufferOff] = pszItems[nCurrChar];
				pszCurrItem[nBufferOff + 1] = '\0';
				nBufferOff++;
			}
		}
	}

	//add the final string
	m_ppszItems[nCurrItem] = CloneString(pszCurrItem, false);
	m_nNumItems = nNumItems;

	return true;	
}


//sets up the items based upon a given list
bool CPackerEnumProperty::SetItems(char*const * pszList, uint32 nNumItems)
{
	Free();

	//make the list
	m_ppszItems = new char* [nNumItems];

	//check for validity
	if(m_ppszItems == NULL)
		return false;

	//copy over the strings
	for(uint32 nCurrString = 0; nCurrString < nNumItems; nCurrString++)
	{
		m_ppszItems[nCurrString] = CloneString(pszList[nCurrString], false);
	}

	//set the number of items
	m_nNumItems = nNumItems;

	//success
	return true;
}

//finds the ID of a string. Returns -1 if not found
int32 CPackerEnumProperty::FindItem(const char* pszVal) const
{
	for(uint32 nCurr = 0; nCurr < GetNumItems(); nCurr++)
	{
		if(stricmp(GetItem(nCurr), pszVal) == 0)
		{
			return nCurr;
		}
	}
	return -1;
}

//gets the number of items
uint32 CPackerEnumProperty::GetNumItems() const
{
	return m_nNumItems;
}

//gets a specific item
const char*	CPackerEnumProperty::GetItem(uint32 nItem) const
{
	assert(nItem < GetNumItems());
	if(nItem < GetNumItems())
	{
		return m_ppszItems[nItem];
	}

	return NULL;
}	

//get selected item
uint32 CPackerEnumProperty::GetSelection() const
{
	return m_nSelected;
}

//get the text of the selected item
const char*	CPackerEnumProperty::GetSelectionText() const
{
	return GetItem(GetSelection());
}

//set the selected item
void CPackerEnumProperty::SetSelection(uint32 nItem)
{
	assert(nItem < GetNumItems());
	m_nSelected = (nItem < GetNumItems()) ? nItem : GetNumItems();
}

//------------------------------------------------------------
// Real Property

CPackerRealProperty::CPackerRealProperty() :
	m_fVal(0.0f),
	m_bRanged(false)
{
}

CPackerRealProperty::CPackerRealProperty(const char* pszName, float fVal, bool bInteger, const char* pszHelp) :
	CPackerProperty(pszName, pszHelp),
	m_fVal(fVal),
	m_bRanged(false),
	m_bInteger(bInteger)
{
}

CPackerRealProperty::CPackerRealProperty(const char* pszName, float fVal, bool bInteger, float fMin, float fMax,
										 const char* pszHelp) :
	CPackerProperty(pszName, pszHelp),
	m_fVal(fVal),
	m_bRanged(false),
	m_bInteger(bInteger)
{
	SetRanged(fMin, fMax);
}

CPackerRealProperty::CPackerRealProperty(const CPackerRealProperty& rhs)
{
	*this = rhs;
}

CPackerRealProperty& CPackerRealProperty::operator=(const CPackerRealProperty& rhs)
{
	SetName(rhs.GetName());
	SetHelp(rhs.GetHelp());
	SetEnabled(rhs.IsEnabled());

	m_bRanged	= rhs.m_bRanged;
	m_fMin		= rhs.m_fMin;
	m_fMax		= rhs.m_fMax;

	SetValue(rhs.GetValue());
	SetInteger(rhs.IsInteger());


	return *this;
}

uint32 CPackerRealProperty::GetType() const
{
	return PROPERTY_REAL;
}

bool CPackerRealProperty::SaveValue(char* pszBuffer, uint32 nBufferLen) const
{
	sprintf(pszBuffer, "%.4f", GetValue());
	return true;
}

bool CPackerRealProperty::LoadValue(char** ppszBuffer, uint32 nNumBuffers)
{
	assert(ppszBuffer);
	if(nNumBuffers == 0)
	{
		return false;
	}

	SetValue((float)atof(ppszBuffer[0]));

	return true;
}

//gets the rounded integer version
int32 CPackerRealProperty::GetIntValue() const
{
	return (int32)(GetValue() + 0.5f);
}

//get the floating point version
float CPackerRealProperty::GetValue() const
{
	return m_fVal;
}

void CPackerRealProperty::SetValue(float fVal)
{
	//make sure it is in range
	if(IsRanged())
	{
		if(fVal < GetMin())
			m_fVal = GetMin();
		else if(fVal > GetMax())
			m_fVal = GetMax();
		else
			m_fVal = fVal;
	}
	else
	{
		m_fVal = fVal;
	}
}

//range information
bool CPackerRealProperty::IsRanged() const
{
	return m_bRanged;
}

void CPackerRealProperty::SetRanged(float fMin, float fMax)
{
	m_bRanged = true;
	m_fMin = fMin;
	m_fMax = fMax;
}

void CPackerRealProperty::ClearRanged()
{
	m_bRanged = false;
}

float CPackerRealProperty::GetMin() const
{
	return m_fMin;
}

float CPackerRealProperty::GetMax() const
{
	return m_fMax;
}

bool CPackerRealProperty::IsInteger() const
{
	return m_bInteger;
}

void CPackerRealProperty::SetInteger(bool bInteger)
{
	m_bInteger = bInteger;
}

//------------------------------------------------------------
// Interface Property
// The interface property simply allows for better control over
// the look and feel in graphical UI's, and provides a means
// for breaking apart groups, and other things

//constructors
CPackerInterfaceProperty::CPackerInterfaceProperty() :
	m_nInterfaceType(BLANK)
{
}

CPackerInterfaceProperty::CPackerInterfaceProperty(const char* pszName, uint32 nType, const char* pszHelp) :
	CPackerProperty(pszName, pszHelp),
	m_nInterfaceType(nType)
{
}

CPackerInterfaceProperty::CPackerInterfaceProperty(const CPackerInterfaceProperty& rhs)
{
	*this = rhs;
}

CPackerInterfaceProperty& CPackerInterfaceProperty::operator=(const CPackerInterfaceProperty& rhs)
{
	SetName(rhs.GetName());
	SetHelp(rhs.GetHelp());
	SetEnabled(rhs.IsEnabled());
	
	SetInterfaceType(rhs.GetInterfaceType());

	return *this;
}

uint32 CPackerInterfaceProperty::GetType() const
{
	return PROPERTY_INTERFACE;
}

bool CPackerInterfaceProperty::SaveValue(char* pszBuffer, uint32 nBufferLen) const
{
	return true;
}

bool CPackerInterfaceProperty::LoadValue(char** ppszBuffer, uint32 nNumBuffers)
{
	return true;
}

//type information
uint32 CPackerInterfaceProperty::GetInterfaceType() const
{
	return m_nInterfaceType;
}

void CPackerInterfaceProperty::SetInterfaceType(uint32 nType)
{
	m_nInterfaceType = nType;
}

//------------------------------------------------------------
// Base Property
CPackerProperty::CPackerProperty(const char* pszName, const char* pszHelp) :
	m_pszName(NULL),
	m_pszHelp(NULL),
	m_bEnabled(true)
{
	if(pszName)
		SetName(pszName);
	if(pszHelp)
		SetHelp(pszHelp);
}

CPackerProperty::~CPackerProperty()
{
	FreeString(m_pszName);
	FreeString(m_pszHelp);
}

//sets the name of this property
bool CPackerProperty::SetName(const char* pszName)
{
	FreeString(m_pszName);
	m_pszName = CloneString(pszName, false);

	return true;
}

//gets the name of this property
const char* CPackerProperty::GetName() const
{
	return m_pszName;
}

//sets the help string associated with this property
bool CPackerProperty::SetHelp(const char* pszHelp)
{
	FreeString(m_pszHelp);
	
	if(pszHelp)
	{
		m_pszHelp = CloneString(pszHelp, false);
	}

	return true;
}

//gets the help string associated with this property
const char* CPackerProperty::GetHelp() const
{
	return m_pszHelp;
}

//determines if this property is enabled or not
bool CPackerProperty::IsEnabled() const
{
	return m_bEnabled;
}

//enables/disables this property
void CPackerProperty::SetEnabled(bool bEnable)
{
	m_bEnabled = bEnable;
}


#include "packerproplist.h"
#include "packerproperty.h"
#include "string.h"
#include "assert.h"

CPackerPropList::CPackerPropList() :
	m_ppPropList(NULL),
	m_nNumProps(0)
{
}

CPackerPropList::~CPackerPropList()
{
	FreeList();
}

//adds a property onto the list. This turns ownership of the property over to the
//list. It assumes all properties are allocated with new
bool CPackerPropList::AppendProperty(CPackerProperty* pProp)
{
	//allocate a new block with one more element
	CPackerProperty** ppNewList = new CPackerProperty*[GetNumProperties() + 1];

	//if this failed, we have problems
	if(ppNewList == NULL)
		return false;

	//ok it succeeded, so copy over the old list
	for(uint32 nCurrItem = 0; nCurrItem < GetNumProperties(); nCurrItem++)
	{
		ppNewList[nCurrItem] = m_ppPropList[nCurrItem];
	}

	//now add on the new property
	ppNewList[GetNumProperties()] = pProp;

	//free the old list
	delete [] m_ppPropList;

	m_ppPropList = ppNewList;

	//add the item count
	m_nNumProps++;

	return true;
}

//frees the associated list of items
void CPackerPropList::FreeList()
{
	//run through and delete all the items
	for(uint32 nCurrItem = 0; nCurrItem < GetNumProperties(); nCurrItem++)
	{
		delete m_ppPropList[nCurrItem];
	}
	delete [] m_ppPropList;
	m_ppPropList = NULL;

	m_nNumProps = 0;
}

//determines the number of properties in the list
uint32 CPackerPropList::GetNumProperties() const
{
	return m_nNumProps;
}

//gets a specified property
CPackerProperty* CPackerPropList::GetProperty(uint32 nIndex)
{
	if(nIndex < GetNumProperties())
	{
		return m_ppPropList[nIndex];
	}
	assert(false);
	return NULL;
}

//determines if the list contains the specified property
bool CPackerPropList::ContainsProperty(const char* pszPropName)
{
	return GetProperty(pszPropName) ? true : false;
}

//gets the specified property
CPackerProperty* CPackerPropList::GetProperty(const char* pszPropName)
{
	for(uint32 nCurrProp = 0; nCurrProp < GetNumProperties(); nCurrProp++)
	{
		if(stricmp(m_ppPropList[nCurrProp]->GetName(), pszPropName) == 0)
		{
			return m_ppPropList[nCurrProp];
		}
	}
	return NULL;
}

//handles the enabling of an item
bool CPackerPropList::EnableProperty(const char* pszName, bool bEnable)
{
	CPackerProperty* pProp = GetProperty(pszName);

	if(pProp)
	{
		pProp->SetEnabled(bEnable);
		return true;
	}

	return false;
}

//utility functions to get specific values out of properties. Each will look for
//the appropriate property, and if it can't find it, or it is of the wrong type
//it will return the default that is specified
const char* CPackerPropList::GetString(const char* pszPropName, const char* pszDefault)
{
	CPackerProperty* pProp = GetProperty(pszPropName);
	if((pProp == NULL) || (pProp->GetType() != PROPERTY_STRING))
		return pszDefault;

	return ((CPackerStringProperty*)pProp)->GetValue();
}

bool CPackerPropList::GetBool(const char* pszPropName, bool bDefault)
{
	CPackerProperty* pProp = GetProperty(pszPropName);
	if((pProp == NULL) || (pProp->GetType() != PROPERTY_BOOL))
		return bDefault;

	return ((CPackerBoolProperty*)pProp)->GetValue();
}

float CPackerPropList::GetReal(const char* pszPropName, float fDefault)
{
	CPackerProperty* pProp = GetProperty(pszPropName);
	if((pProp == NULL) || (pProp->GetType() != PROPERTY_REAL))
		return fDefault;

	return ((CPackerRealProperty*)pProp)->GetValue();
}

uint32 CPackerPropList::GetEnum(const char* pszPropName, uint32 nDefault)
{
	CPackerProperty* pProp = GetProperty(pszPropName);
	if((pProp == NULL) || (pProp->GetType() != PROPERTY_ENUM))
		return nDefault;

	return ((CPackerEnumProperty*)pProp)->GetSelection();
}

const char* CPackerPropList::GetEnumText(const char* pszPropName, const char* pszDefault)
{
	CPackerProperty* pProp = GetProperty(pszPropName);
	if((pProp == NULL) || (pProp->GetType() != PROPERTY_ENUM))
		return pszDefault;

	return ((CPackerEnumProperty*)pProp)->GetSelectionText();
}

void CPackerPropList::SetString(const char* pszPropName, const char* pszString)
{
	CPackerProperty* pProp = GetProperty(pszPropName);
	if((pProp == NULL) || (pProp->GetType() != PROPERTY_STRING))
		return;

	((CPackerStringProperty*)pProp)->SetValue(pszString);
}

void CPackerPropList::SetBool(const char* pszPropName, bool bVal)
{
	CPackerProperty* pProp = GetProperty(pszPropName);
	if((pProp == NULL) || (pProp->GetType() != PROPERTY_BOOL))
		return;

	((CPackerBoolProperty*)pProp)->SetValue(bVal);
}

void CPackerPropList::SetReal(const char* pszPropName, float fVal)
{
	CPackerProperty* pProp = GetProperty(pszPropName);
	if((pProp == NULL) || (pProp->GetType() != PROPERTY_REAL))
		return;

	((CPackerRealProperty*)pProp)->SetValue(fVal);
}

void CPackerPropList::SetEnum(const char* pszPropName, uint32 nVal)
{
	CPackerProperty* pProp = GetProperty(pszPropName);
	if((pProp == NULL) || (pProp->GetType() != PROPERTY_ENUM))
		return;

	((CPackerEnumProperty*)pProp)->SetSelection(nVal);
}

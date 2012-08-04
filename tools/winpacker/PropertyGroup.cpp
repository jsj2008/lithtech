#include "propertygroup.h"
#include <string.h>
#include <assert.h>

CPropertyGroup::CPropertyGroup(const char* pszName) :
	m_nNumProperties(0),
	m_ppPropList(NULL)
{
	//copy over the name
	assert(pszName);

	m_pszName = new char [strlen(pszName) + 1];

	if(m_pszName)
		strcpy(m_pszName, pszName);
}

CPropertyGroup::~CPropertyGroup()
{
	delete [] m_pszName;
	delete [] m_ppPropList;
}

//gets the number of properties in this group
uint32 CPropertyGroup::GetNumProperties() const
{
	return m_nNumProperties;
}

//gets a specified property
CPackerProperty* CPropertyGroup::GetProperty(uint32 nIndex)
{
	if(nIndex < GetNumProperties())
	{
		return m_ppPropList[nIndex];
	}
	return NULL;
}

//adds a property to this list. Note that it does not
//take ownership of the property and is meant to work in 
//conjunction with the property list so the list holds
//onto the property and this refers to it
bool CPropertyGroup::AddProperty(CPackerProperty* pProp)
{
	//resize the array
	CPackerProperty** ppNewList = new CPackerProperty* [GetNumProperties() + 1];

	//copy over the old list
	if(ppNewList)
	{
		for(uint32 nCurrProp = 0; nCurrProp < GetNumProperties(); nCurrProp++)
		{
			ppNewList[nCurrProp] = m_ppPropList[nCurrProp];
		}
		
		//add our new property
		ppNewList[GetNumProperties()] = pProp;

		//clean up the old list
		delete [] m_ppPropList;

		//now set this as our list
		m_ppPropList = ppNewList;
		m_nNumProperties++;

		return true;
	}

	return false;
}

//gets the name of this group
const char* CPropertyGroup::GetName() const
{
	return m_pszName;
}
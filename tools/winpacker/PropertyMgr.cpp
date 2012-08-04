#include <stdio.h>
#include "propertymgr.h"
#include "packerproperty.h"
#include <string.h>
#include <assert.h>

static CPackerProperty* CloneProperty(const CPackerProperty& Prop)
{
	switch(Prop.GetType())
	{
	case PROPERTY_BOOL:
		return new CPackerBoolProperty((const CPackerBoolProperty&)Prop);
		break;
	case PROPERTY_STRING:
		return new CPackerStringProperty((const CPackerStringProperty&)Prop);
		break;
	case PROPERTY_ENUM:
		return new CPackerEnumProperty((const CPackerEnumProperty&)Prop);
		break;
	case PROPERTY_REAL:
		return new CPackerRealProperty((const CPackerRealProperty&)Prop);
		break;
	case PROPERTY_INTERFACE:
		return new CPackerInterfaceProperty((const CPackerInterfaceProperty&)Prop);
		break;
	default:
		//add clone for new property
		assert(false);
		break;
	}
	return NULL;
}

CPropertyMgr::CPropertyMgr() :
	m_ppGroupList(NULL),
	m_nNumGroups(0)
{
}

CPropertyMgr::~CPropertyMgr()
{
	Free();
}

//retrieves the property list
CPackerPropList& CPropertyMgr::GetPropList()
{
	return m_PropList;
}

uint32 CPropertyMgr::GetNumGroups() const
{
	return m_nNumGroups;
}

//gets a group based off of an index
CPropertyGroup* CPropertyMgr::GetGroup(uint32 nIndex)
{
	if(nIndex >= GetNumGroups())
	{
		return NULL;
	}

	return m_ppGroupList[nIndex];
}


//------------------
// IPackerUI Impl

// Allows for the creation of a property. This property will be placed underneath
// the group specified by pszGroupName. It will fail if a property of the same name
// already exists
bool CPropertyMgr::CreateProperty(const CPackerProperty& Property, const char* pszGroupName)
{
	//see if we have a property of that name
	if(GetPropList().ContainsProperty(Property.GetName()))
	{
		//already has that property
		return false;
	}

	CPackerProperty* pNewProp = CloneProperty(Property);

	GetPropList().AppendProperty(pNewProp);

	//add it to the group
	return AddPropertyToGroup(pNewProp, pszGroupName);
}

// This will create a reference to a property so that it can be contained in
// multiple groups. If the property doesn't exist, it will fail
bool CPropertyMgr::CreateReference(const char* pszPropName, const char* pszGroupName)
{
	//safety
	if(pszPropName == NULL)
	{
		return false;
	}

	//find the property
	CPackerProperty* pProp = GetPropList().GetProperty(pszPropName);

	if(pProp == NULL)
	{
		return false;
	}

	//add it to the group
	return AddPropertyToGroup(pProp, pszGroupName);	
}

// Allows for a property to be enabled or disabled. If it is being disabled, the
// pszReason string can be used for filling out a reason string on why it is
// disabled.
bool CPropertyMgr::EnableProperty(	const char* pszPropName, bool bEnable)
{
	if(pszPropName == NULL)
		return false;

	//find the property
	CPackerProperty* pProp = GetPropList().GetProperty(pszPropName);

	if(pProp == NULL)
	{
		return false;
	}

	//set the state
	pProp->SetEnabled(bEnable);

	//do something with the reason text...

	return true;
}

//frees memory associated with this object
void CPropertyMgr::Free()
{
	for(uint32 nCurrGroup = 0; nCurrGroup < GetNumGroups(); nCurrGroup++)
	{
		delete m_ppGroupList[nCurrGroup];
	}
	delete [] m_ppGroupList;

	m_ppGroupList = NULL;
	m_nNumGroups  = 0;			
}

//adds a group to the list
CPropertyGroup* CPropertyMgr::AddGroup(const char* pszName)
{
	//allocate the new group
	CPropertyGroup* pNewGroup = new CPropertyGroup(pszName);

	if(pNewGroup == NULL)
		return NULL;

	//create the new list
	CPropertyGroup** ppNewList = new CPropertyGroup* [GetNumGroups() + 1];

	if(ppNewList == NULL)
	{
		delete pNewGroup;
		return NULL;
	}

	for(uint32 nCurrGroup = 0; nCurrGroup < GetNumGroups(); nCurrGroup++)
	{
		ppNewList[nCurrGroup] = m_ppGroupList[nCurrGroup];
	}

	ppNewList[GetNumGroups()] = pNewGroup;

	//free the old list
	delete [] m_ppGroupList;
	m_ppGroupList = ppNewList;
	m_nNumGroups++;

	return pNewGroup;
}

//gets a group from the list
CPropertyGroup* CPropertyMgr::GetGroup(const char* pszName)
{
	for(uint32 nCurrGroup = 0; nCurrGroup < GetNumGroups(); nCurrGroup++)
	{
		if(stricmp(m_ppGroupList[nCurrGroup]->GetName(), pszName) == 0)
		{
			return m_ppGroupList[nCurrGroup];
		}
	}
	return NULL;
}

//will get a group if it exists, or create a new one
CPropertyGroup* CPropertyMgr::GetOrCreateGroup(const char* pszName)
{
	CPropertyGroup* pRV = GetGroup(pszName);

	if(pRV == NULL)
	{
		return AddGroup(pszName);
	}

	return pRV;
}

//adds a property to the specified group
bool CPropertyMgr::AddPropertyToGroup(CPackerProperty* pProp, const char* pszGroup)
{
	if(pProp == NULL)
		return false;

	//if the group doesn't have a name, use the word "DEFAULT" instead
	if(pszGroup == NULL)
		pszGroup = "Default";

	//get the group
	CPropertyGroup* pGroup = GetOrCreateGroup(pszGroup);

	if(pGroup == NULL)
		return false;

	//add this property to the group
	return pGroup->AddProperty(pProp);
}


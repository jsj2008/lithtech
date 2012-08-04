#ifndef __PROPERTYMGR_H__
#define __PROPERTYMGR_H__

#ifndef __IPACKERUI_H__
#	include "IPackerUI.h"
#endif

#ifndef __PACKERPROPLIST_H__
#	include "PackerPropList.h"
#endif

#ifndef __PROPERTYGROUP_H__
#	include "PropertyGroup.h"
#endif

class CPropertyMgr :
	public IPackerUI
{
public:
	
	CPropertyMgr();
	virtual ~CPropertyMgr();

	//retrieves the property list
	CPackerPropList&	GetPropList();

	//------------------
	// IPackerUI Impl

	// Allows for the creation of a property. This property will be placed underneath
	// the group specified by pszGroupName. It will fail if a property of the same name
	// already exists
	virtual bool	CreateProperty(const CPackerProperty& pProperty, const char* pszGroupName);

	// This will create a reference to a property so that it can be contained in
	// multiple groups. If the property doesn't exist, it will fail
	virtual bool	CreateReference(const char* pszPropName, const char* pszGroupName);

	// Allows for a property to be enabled or disabled. If it is being disabled, the
	// pszReason string can be used for filling out a reason string on why it is
	// disabled.
	virtual bool	EnableProperty(	const char* pszPropName, bool bEnable);

	//group information
	uint32			GetNumGroups() const;

	//gets a group based off of an index
	CPropertyGroup*	GetGroup(uint32 nIndex);

	//gets a group from the list
	CPropertyGroup*		GetGroup(const char* pszName);

private:

	//adds a property to the specified group
	bool				AddPropertyToGroup(CPackerProperty* pProp, const char* pszGroup);	

	//frees memory associated with this object
	void				Free();

	//adds a group to the list
	CPropertyGroup*		AddGroup(const char* pszName);


	//will get a group if it exists, or create a new one
	CPropertyGroup*		GetOrCreateGroup(const char* pszName);

	//the actual property list
	CPackerPropList		m_PropList;

	//the group list
	CPropertyGroup**	m_ppGroupList;
	uint32				m_nNumGroups;

};

#endif


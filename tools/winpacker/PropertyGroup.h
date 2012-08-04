#ifndef __PROPERTYGROUP_H__
#define __PROPERTYGROUP_H__

#include "ltinteger.h"

class CPackerProperty;

class CPropertyGroup
{
public:
	
	CPropertyGroup(const char* pszName);
	~CPropertyGroup();

	//gets the number of properties in this group
	uint32				GetNumProperties() const;

	//gets a specified property
	CPackerProperty*	GetProperty(uint32 nIndex);

	//adds a property to this list. Note that it does not
	//take ownership of the property and is meant to work in 
	//conjunction with the property list so the list holds
	//onto the property and this refers to it
	bool				AddProperty(CPackerProperty* pProp);

	//gets the name of this group
	const char*			GetName() const;
	
private:

	char*				m_pszName;

	uint32				m_nNumProperties;
	CPackerProperty**	m_ppPropList;
};

#endif


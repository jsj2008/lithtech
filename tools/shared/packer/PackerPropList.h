#ifndef __PACKERPROPLIST_H__
#define __PACKERPROPLIST_H__

//for the integer types
#include "ltinteger.h"


class CPackerProperty;

class CPackerPropList
{
public:

	CPackerPropList();
	~CPackerPropList();

	//adds a property onto the list. This turns ownership of the property over to the
	//list. It assumes all properties are allocated with new
	bool				AppendProperty(CPackerProperty* pProp);

	//frees the associated list of items
	void				FreeList();

	//determines the number of properties in the list
	uint32				GetNumProperties() const;

	//gets a specified property
	CPackerProperty*	GetProperty(uint32 nIndex);

	//determines if the list contains the specified property
	bool				ContainsProperty(const char* pszPropName);

	//gets the specified property
	CPackerProperty*	GetProperty(const char* pszPropName);

	//handles the enabling of an item
	bool				EnableProperty(const char* pszName, bool bEnable);

	//utility functions to get specific values out of properties. Each will look for
	//the appropriate property, and if it can't find it, or it is of the wrong type
	//it will return the default that is specified
	const char*			GetString(const char* pszPropName, const char* pszDefault = "");
	bool				GetBool(const char* pszPropName, bool bDefault = false);
	float				GetReal(const char* pszPropName, float fDefault = 0.0f);
	uint32				GetEnum(const char* pszPropName, uint32 nDefault = 0);
	const char*			GetEnumText(const char* pszPropName, const char* pszDefault = "");

	//utility functions to set specific values of properties. These should be used
	//over direct access for memory safety
	void				SetString(const char* pszPropName, const char* pszString);
	void				SetBool(const char* pszPropName, bool bVal);
	void				SetReal(const char* pszPropName, float fVal);
	void				SetEnum(const char* pszPropName, uint32 nVal);

private:
	
	CPackerProperty**	m_ppPropList;

	uint32				m_nNumProps;
};

#endif

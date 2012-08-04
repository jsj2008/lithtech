#ifndef __PROPUTILS_H__
#define __PROPUTILS_H__

//given a property name and a node, it will try and find a string property matching that
//name, and return the default string if it fails
inline const char* GetStringProp(CWorldNode* pNode, const char* pszPropName, const char* pszDefault = NULL)
{
	assert(pNode);

	CBaseProp* pProp = pNode->GetPropertyList()->GetProp(pszPropName);

	if((pProp == NULL) || (pProp->GetType() != PT_STRING))
	{
		return pszDefault;
	}

	return ((CStringProp*)pProp)->GetString();
}

//given a property name and a node, it will try and find a bool property matching that
//name, and return the default bool if it fails
inline bool GetBoolProp(CWorldNode* pNode, const char* pszPropName, bool bDefault = false)
{
	assert(pNode);

	CBaseProp* pProp = pNode->GetPropertyList()->GetProp(pszPropName);

	if((pProp == NULL) || (pProp->GetType() != PT_BOOL))
	{
		return bDefault;
	}

	return !!((CBoolProp*)pProp)->m_Value;
}


//given a property name and a node, it will try and find a real property matching that
//name, and return the default value if it fails
inline float GetRealProp(CWorldNode* pNode, const char* pszPropName, float fDefault = 0.0f)
{
	assert(pNode);

	CBaseProp* pProp = pNode->GetPropertyList()->GetProp(pszPropName);

	if((pProp == NULL) || (	(pProp->GetType() != PT_REAL) &&
							(pProp->GetType() != PT_LONGINT) &&
							(pProp->GetType() != PT_FLAGS)))
	{
		return fDefault;
	}

	return ((CRealProp*)pProp)->m_Value;
}


#endif
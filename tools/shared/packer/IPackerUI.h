//---------------------------------------------------------------
// IPackerUI.h
//
// Provides the definition for IPackerUI, which provides a
// mechanism for preprocessor implementations to communicate
// with a front end user interface for managing properties.
//
// Created: 4/16/01
// Author: John O'Rorke
// Modification History:
//
//---------------------------------------------------------------
#ifndef __IPACKERUI_H__
#define __IPACKERUI_H__

//----------------------
// Forward declarations

// property classes
class CPackerProperty;



class IPackerUI
{
public:

	//---------------------
	// Constructors

	IPackerUI()				{}
	virtual ~IPackerUI()	{}

	//---------------------
	// Property Management

	// Allows for the creation of a property. This property will be placed underneath
	// the group specified by pszGroupName. It will fail if a property of the same name
	// already exists
	virtual bool	CreateProperty(const CPackerProperty& pProperty, const char* pszGroupName)	= 0;

	// This will create a reference to a property so that it can be contained in
	// multiple groups. If the property doesn't exist, it will fail
	virtual bool	CreateReference(const char* pszPropName, const char* pszGroupName)			= 0;

private:

};

#endif

//------------------------------------------------------------
// KeyDefaultFactory.h
//
// Provides a factory for the creation of CKeyDefaultAggregate
// objects so that new ones can be easily added.
//
// Author: John O'Rorke
// Created: 4/9/01
// Modification History:
//
//------------------------------------------------------------
#ifndef __KEYDEFAULTFACTORY_H__
#define __KEYDEFAULTFACTORY_H__

class CKeyDefaultAggregate;

class CKeyDefaultFactory
{
public:

	//handle creating a key default object
	static CKeyDefaultAggregate*	CreateDefault(const char* pszDefaultName);

	//get the ID of a string to display information on a default
	static CString					GetDefaultText(const char* pszDefaultName);

	//this allows each configuration to update the global options. This should
	//really only be called when someone switches an input style
	static void						UpdateGlobalOptions(const char* pszDefaultName);

private:

	//don't allow instantiation
	CKeyDefaultFactory()							{}
	CKeyDefaultFactory(const CKeyDefaultFactory&)	{}
};

#endif


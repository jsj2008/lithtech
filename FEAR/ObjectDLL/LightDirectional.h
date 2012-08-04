// ----------------------------------------------------------------------- //
//
// MODULE  :	LightDirectional.h
//
// PURPOSE :	Provides the definition for the directional light class 
//				which represents an aligned cube of space which has light
//				orthographically projected into it
//
// CREATED :	10/29/03
//
// (c) 2003 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef __LIGHTDIRECTIONAL_H__
#define __LIGHTDIRECTIONAL_H__

LINKTO_MODULE( LightDirectional );

#ifndef __LIGHTBASE_H__
#	include "LightBase.h"
#endif

class LightDirectional :
	public LightBase
{
public:

	LightDirectional();
	~LightDirectional();
	
private:

	//virtual function that derived classes must override to handle loading in of
	//property data
	virtual void	ReadLightProperties(const GenericPropList *pProps);
	//virtual function that derived classes may override to change the creation struct before the object is created
	virtual void	PostReadProp(ObjectCreateStruct *pStruct);

	virtual uint32	OnObjectCreated(const GenericPropList* pProps, float fCreationReason);
};

class LightDirectional_Plugin: 
	public LightBase_Plugin
{
public:

	virtual LTRESULT PreHook_EditStringList(
		const char* szRezPath,				
		const char* szPropName,				
		char** aszStrings,					
		uint32* pcStrings,	
		const uint32 cMaxStrings,		
		const uint32 cMaxStringLength)		
	{
		//handle setting up any LOD properties on the light
		if(	LTStrEquals(szPropName, "VolumetricLOD") )
		{
			return CEngineLODPropUtil::AddLODStrings(aszStrings, pcStrings, cMaxStrings, cMaxStringLength);
		}
		return LightBase_Plugin::PreHook_EditStringList(szRezPath, szPropName, aszStrings, pcStrings, cMaxStrings, cMaxStringLength);
	}
};


#endif

// ----------------------------------------------------------------------- //
//
// MODULE  : Key.h
//
// PURPOSE : Key definition for Keyframer class
//
// CREATED : 10/7/97
//
// ----------------------------------------------------------------------- //

#ifndef __KEY_H__
#define __KEY_H__

#include "ltengineobjects.h"
#include "CommandMgr.h"

LINKTO_MODULE( Key );

class Key : public BaseClass
{
	public :
};

class CKeyPlugin : public IObjectPlugin 
{
	public :
		
		virtual LTRESULT PreHook_PropChanged(
				const char *szObjName,
				const char *szPropName,
				const int nPropType,
				const GenericProp &gpPropValue,
				ILTPreInterface *pInterface,
				const char *szModifiers );

		virtual LTRESULT CKeyPlugin::PreHook_EditStringList( 
				const char * szRezPath, 
				const char *szPropName,
				char **aszStrings,
				uint32 *pcStrings,
				const uint32 cMaxStrings,
				const uint32 cMaxStringLength );

	protected :

		CCommandMgrPlugin m_CommandMgrPlugin;

};

#endif // __KEY_H__

// ----------------------------------------------------------------------- //
//
// MODULE  : StringEditMgrPlugin.cpp
//
// PURPOSE : Defines the CStringEditMgrPlugin class.  This class
//           performs error checks for StringIDs. 
//
// CREATED : 09/07/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "StringEditMgrPlugin.h"
#include "StringEditMgr.h"

// checks the value to make sure it's valid
LTRESULT CStringEditMgrPlugin::PreHook_PropChanged(
				const	char		*szObjName,
				const	char		*szPropName,
				const	int			nPropType,
				const	GenericProp	&gpPropValue,
						ILTPreInterface	*pInterface,
				const	char		*szModifiers )
{
	if( !szObjName || !szPropName || !pInterface ) return LT_UNSUPPORTED;

	// make sure this is a string id
	if( nPropType != LT_PT_STRINGID )
		return LT_UNSUPPORTED;

	// make sure the string keeper database is loaded
	if( !g_pLTIStringEdit || !g_pLTDBStringEdit )
		return LT_UNSUPPORTED;

	if( gpPropValue.GetString()[0] == '\0' )
		return LT_UNSUPPORTED;

	if( !g_pLTIStringEdit->DoesIDExist(g_pLTDBStringEdit, gpPropValue.GetString()) )
	{
		pInterface->ShowDebugWindow( true );
		pInterface->CPrint( "Invalid String ID - '%s'.", gpPropValue.GetString() );
		pInterface->CPrint( "Object: %s    Property: %s    Value: '%s'", szObjName, szPropName, gpPropValue.GetString() );
		pInterface->CPrint( "" );
		return LT_UNSUPPORTED;
	}

	return LT_OK;
}

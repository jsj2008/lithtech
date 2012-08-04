// ----------------------------------------------------------------------- //
//
// MODULE  : StringEditMgrPlugin.h
//
// PURPOSE : Declares the CStringEditMgrPlugin class.  This class
//           performs error checks for StringIDs. 
//
// CREATED : 09/07/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __STRINGEDITMGRPLUGIN_H__
#define __STRINGEDITMGRPLUGIN_H__

#include "iobjectplugin.h"

class CStringEditMgrPlugin : public IObjectPlugin
{
	public:
		virtual LTRESULT PreHook_PropChanged(
				const	char		*szObjName,
				const	char		*szPropName,
				const	int			nPropType,
				const	GenericProp	&gpPropValue,
						ILTPreInterface	*pInterface,
				const	char		*szModifiers );
};

#endif  // __STRINGEDITMGRPLUGIN_H__

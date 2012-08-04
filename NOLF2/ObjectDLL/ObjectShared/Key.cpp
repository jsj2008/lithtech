// ----------------------------------------------------------------------- //
//
// MODULE  : Key.cpp
//
// PURPOSE : Key implementation for Keyframer class
//
// CREATED : 10/7/97
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "key.h"

LINKFROM_MODULE( Key );

#pragma force_active on
BEGIN_CLASS(Key)
	ADD_REALPROP(TimeStamp, 0.0f)
	ADD_STRINGPROP(SoundName, "")
	ADD_REALPROP_FLAG(SoundRadius, 0.0f,PF_RADIUS)
	ADD_STRINGPROP_FLAG(Command, "", PF_NOTIFYCHANGE)
	ADD_VECTORPROP_FLAG(BezierPrev, PF_BEZIERPREVTANGENT)
	ADD_VECTORPROP_FLAG(BezierNext, PF_BEZIERNEXTTANGENT)
END_CLASS_DEFAULT_FLAGS_PLUGIN(Key, BaseClass, NULL, NULL, CF_NORUNTIME, CKeyPlugin)
#pragma force_active off

LTRESULT CKeyPlugin::PreHook_PropChanged( const char *szObjName,
										  const char *szPropName,
										  const int nPropType,
										  const GenericProp &gpPropValue,
										  ILTPreInterface *pInterface,
										  const char *szModifiers )
{
	// Check to see if our coomad prop has changed...

	if( !_stricmp( "Command", szPropName ))
	{
		// Pass it to the command mgr to process...

		if( m_CommandMgrPlugin.PreHook_PropChanged( szObjName,
													szPropName,
													nPropType,
													gpPropValue,
													pInterface,
													szModifiers ) == LT_OK )
		{
			return LT_OK;
		}
	}

	return LT_UNSUPPORTED;
}

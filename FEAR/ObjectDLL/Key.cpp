// ----------------------------------------------------------------------- //
//
// MODULE  : Key.cpp
//
// PURPOSE : Key implementation for Keyframer class
//
// CREATED : 10/7/97
//
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "Key.h"
#include "SoundDB.h"

LINKFROM_MODULE( Key );

BEGIN_CLASS(Key)
	ADD_REALPROP(TimeStamp, 0.0f, "This is the time that it takes for the KeyFramer to get to this key from the previous key in the key path.")
	ADD_STRINGPROP_FLAG(SoundName, "", PF_STATICLIST, "The name of the sound record that will be played when the KeyFramer arrives at this Key on the key path.")
	ADD_COMMANDPROP_FLAG(Command, "", PF_NOTIFYCHANGE, "This is a Command string that that will be processed when the KeyFramer arrives at this key.")
	ADD_VECTORPROP_FLAG(BezierPrev, PF_BEZIERPREVTANGENT, "When making a key path it is possible to create Bezier Curves. While in Object mode, if you select one of the Keys in a key path, press the P key and drag the mouse while pressing the LMB, you will pull out the Bezier handle on the 'in' side of the key. This field denotes the world coordinate position of that handle.")
	ADD_VECTORPROP_FLAG(BezierNext, PF_BEZIERNEXTTANGENT, "After pulling out the first Bewzier handle, you can repeat the process to pull out the bezier handle on the 'out' side of the key. This field denotes the world coordinate position of that handle.")
END_CLASS_FLAGS_PLUGIN(Key, BaseClass, CF_NORUNTIME, CKeyPlugin, "Key objects are used in conjunction with the KeyFramer object to create KeyFramed paths for objects to follow." )

CMDMGR_BEGIN_REGISTER_CLASS( Key )
CMDMGR_END_REGISTER_CLASS( Key, BaseClass )

LTRESULT CKeyPlugin::PreHook_PropChanged( const char *szObjName,
										  const char *szPropName,
										  const int nPropType,
										  const GenericProp &gpPropValue,
										  ILTPreInterface *pInterface,
										  const char *szModifiers )
{
	// Check to see if our coomad prop has changed...
	if( LTStrIEquals( "Command", szPropName ))
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

LTRESULT CKeyPlugin::PreHook_EditStringList( const char * szRezPath, 
											const char *szPropName,
											char **aszStrings,
											uint32 *pcStrings,
											const uint32 cMaxStrings,
											const uint32 cMaxStringLength )
{
	LTASSERT(szPropName && aszStrings && pcStrings, "TODO: Add description here");

	if( LTStrIEquals( szPropName, "SoundName" ))
	{
		if( CSoundDBPlugin::Instance().PreHook_EditStringList( szRezPath,
			szPropName,
			aszStrings,
			pcStrings,
			cMaxStrings,
			cMaxStringLength ) == LT_OK )
		{
			return LT_OK;
		}
	}

	return LT_UNSUPPORTED;
}

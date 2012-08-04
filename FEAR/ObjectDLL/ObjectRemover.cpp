// ----------------------------------------------------------------------- //
//
// MODULE  : ObjectRemover.cpp
//
// PURPOSE : ObjectRemover - Implementation
//
// CREATED : 04.23.1999
//
// (c) 1999-2003 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "ObjectRemover.h"
#include "ServerUtilities.h"
#include "ObjectMsgs.h"
#include "AIUtils.h"
#include "time.h"
#include "GameModeMgr.h"

LINKFROM_MODULE( ObjectRemover );

class CObjectRemoverPlugin : public IObjectPlugin
{
public:

	virtual LTRESULT PreHook_EditStringList(
		const char* szRezPath,
		const char* szPropName,
		char** aszStrings,
		uint32* pcStrings,
		const uint32 cMaxStrings,
		const uint32 cMaxStringLength);

protected :

};


#define GameModeNone	"<none>"
#define NUM_GAME_MODE_PROPS	8

BEGIN_CLASS(ObjectRemover)

	PROP_DEFINEGROUP(Groups0and1, PF_GROUP(1), "This flag is a subset of two of the groups that the ObjectRemover can choose to keep or discard.")
		ADD_STRINGPROP_FLAG(Group00Object00, "", PF_GROUP(1)|PF_OBJECTLINK, "An object in group 0 that can be discarded or kept")
		ADD_STRINGPROP_FLAG(Group00Object01, "", PF_GROUP(1)|PF_OBJECTLINK, "An object in group 0 that can be discarded or kept")
		ADD_STRINGPROP_FLAG(Group00Object02, "", PF_GROUP(1)|PF_OBJECTLINK, "An object in group 0 that can be discarded or kept")
		ADD_STRINGPROP_FLAG(Group00Object03, "", PF_GROUP(1)|PF_OBJECTLINK, "An object in group 0 that can be discarded or kept")
		ADD_STRINGPROP_FLAG(Group00Object04, "", PF_GROUP(1)|PF_OBJECTLINK, "An object in group 0 that can be discarded or kept")
		ADD_STRINGPROP_FLAG(Group00Object05, "", PF_GROUP(1)|PF_OBJECTLINK, "An object in group 0 that can be discarded or kept")
		ADD_STRINGPROP_FLAG(Group00Object06, "", PF_GROUP(1)|PF_OBJECTLINK, "An object in group 0 that can be discarded or kept")
		ADD_STRINGPROP_FLAG(Group00Object07, "", PF_GROUP(1)|PF_OBJECTLINK, "An object in group 0 that can be discarded or kept")
		ADD_STRINGPROP_FLAG(Group00Object08, "", PF_GROUP(1)|PF_OBJECTLINK, "An object in group 0 that can be discarded or kept")
		ADD_STRINGPROP_FLAG(Group00Object09, "", PF_GROUP(1)|PF_OBJECTLINK, "An object in group 0 that can be discarded or kept")
		ADD_STRINGPROP_FLAG(Group00Object10, "", PF_GROUP(1)|PF_OBJECTLINK, "An object in group 0 that can be discarded or kept")
		ADD_STRINGPROP_FLAG(Group01Object00, "", PF_GROUP(1)|PF_OBJECTLINK, "An object in group 1 that can be discarded or kept")
		ADD_STRINGPROP_FLAG(Group01Object01, "", PF_GROUP(1)|PF_OBJECTLINK, "An object in group 1 that can be discarded or kept")
		ADD_STRINGPROP_FLAG(Group01Object02, "", PF_GROUP(1)|PF_OBJECTLINK, "An object in group 1 that can be discarded or kept")
		ADD_STRINGPROP_FLAG(Group01Object03, "", PF_GROUP(1)|PF_OBJECTLINK, "An object in group 1 that can be discarded or kept")
		ADD_STRINGPROP_FLAG(Group01Object04, "", PF_GROUP(1)|PF_OBJECTLINK, "An object in group 1 that can be discarded or kept")
		ADD_STRINGPROP_FLAG(Group01Object05, "", PF_GROUP(1)|PF_OBJECTLINK, "An object in group 1 that can be discarded or kept")
		ADD_STRINGPROP_FLAG(Group01Object06, "", PF_GROUP(1)|PF_OBJECTLINK, "An object in group 1 that can be discarded or kept")
		ADD_STRINGPROP_FLAG(Group01Object07, "", PF_GROUP(1)|PF_OBJECTLINK, "An object in group 1 that can be discarded or kept")
		ADD_STRINGPROP_FLAG(Group01Object08, "", PF_GROUP(1)|PF_OBJECTLINK, "An object in group 1 that can be discarded or kept")
		ADD_STRINGPROP_FLAG(Group01Object09, "", PF_GROUP(1)|PF_OBJECTLINK, "An object in group 1 that can be discarded or kept")
		ADD_STRINGPROP_FLAG(Group01Object10, "", PF_GROUP(1)|PF_OBJECTLINK, "An object in group 1 that can be discarded or kept")

	PROP_DEFINEGROUP(Groups2and3, PF_GROUP(2), "This flag is a subset of two of the groups that the ObjectRemover can choose to keep or discard.")
		ADD_STRINGPROP_FLAG(Group02Object00, "", PF_GROUP(2)|PF_OBJECTLINK, "An object in group 2 that can be discarded or kept")
		ADD_STRINGPROP_FLAG(Group02Object01, "", PF_GROUP(2)|PF_OBJECTLINK, "An object in group 2 that can be discarded or kept")
		ADD_STRINGPROP_FLAG(Group02Object02, "", PF_GROUP(2)|PF_OBJECTLINK, "An object in group 2 that can be discarded or kept")
		ADD_STRINGPROP_FLAG(Group02Object03, "", PF_GROUP(2)|PF_OBJECTLINK, "An object in group 2 that can be discarded or kept")
		ADD_STRINGPROP_FLAG(Group02Object04, "", PF_GROUP(2)|PF_OBJECTLINK, "An object in group 2 that can be discarded or kept")
		ADD_STRINGPROP_FLAG(Group02Object05, "", PF_GROUP(2)|PF_OBJECTLINK, "An object in group 2 that can be discarded or kept")
		ADD_STRINGPROP_FLAG(Group02Object06, "", PF_GROUP(2)|PF_OBJECTLINK, "An object in group 2 that can be discarded or kept")
		ADD_STRINGPROP_FLAG(Group02Object07, "", PF_GROUP(2)|PF_OBJECTLINK, "An object in group 2 that can be discarded or kept")
		ADD_STRINGPROP_FLAG(Group02Object08, "", PF_GROUP(2)|PF_OBJECTLINK, "An object in group 2 that can be discarded or kept")
		ADD_STRINGPROP_FLAG(Group02Object09, "", PF_GROUP(2)|PF_OBJECTLINK, "An object in group 2 that can be discarded or kept")
		ADD_STRINGPROP_FLAG(Group02Object10, "", PF_GROUP(2)|PF_OBJECTLINK, "An object in group 2 that can be discarded or kept")
		ADD_STRINGPROP_FLAG(Group03Object00, "", PF_GROUP(2)|PF_OBJECTLINK, "An object in group 3 that can be discarded or kept")
		ADD_STRINGPROP_FLAG(Group03Object01, "", PF_GROUP(2)|PF_OBJECTLINK, "An object in group 3 that can be discarded or kept")
		ADD_STRINGPROP_FLAG(Group03Object02, "", PF_GROUP(2)|PF_OBJECTLINK, "An object in group 3 that can be discarded or kept")
		ADD_STRINGPROP_FLAG(Group03Object03, "", PF_GROUP(2)|PF_OBJECTLINK, "An object in group 3 that can be discarded or kept")
		ADD_STRINGPROP_FLAG(Group03Object04, "", PF_GROUP(2)|PF_OBJECTLINK, "An object in group 3 that can be discarded or kept")
		ADD_STRINGPROP_FLAG(Group03Object05, "", PF_GROUP(2)|PF_OBJECTLINK, "An object in group 3 that can be discarded or kept")
		ADD_STRINGPROP_FLAG(Group03Object06, "", PF_GROUP(2)|PF_OBJECTLINK, "An object in group 3 that can be discarded or kept")
		ADD_STRINGPROP_FLAG(Group03Object07, "", PF_GROUP(2)|PF_OBJECTLINK, "An object in group 3 that can be discarded or kept")
		ADD_STRINGPROP_FLAG(Group03Object08, "", PF_GROUP(2)|PF_OBJECTLINK, "An object in group 3 that can be discarded or kept")
		ADD_STRINGPROP_FLAG(Group03Object09, "", PF_GROUP(2)|PF_OBJECTLINK, "An object in group 3 that can be discarded or kept")
		ADD_STRINGPROP_FLAG(Group03Object10, "", PF_GROUP(2)|PF_OBJECTLINK, "An object in group 3 that can be discarded or kept")

	PROP_DEFINEGROUP(Groups4and5, PF_GROUP(3), "This flag is a subset of two of the groups that the ObjectRemover can choose to keep or discard.")
		ADD_STRINGPROP_FLAG(Group04Object00, "", PF_GROUP(3)|PF_OBJECTLINK, "An object in group 4 that can be discarded or kept")
		ADD_STRINGPROP_FLAG(Group04Object01, "", PF_GROUP(3)|PF_OBJECTLINK, "An object in group 4 that can be discarded or kept")
		ADD_STRINGPROP_FLAG(Group04Object02, "", PF_GROUP(3)|PF_OBJECTLINK, "An object in group 4 that can be discarded or kept")
		ADD_STRINGPROP_FLAG(Group04Object03, "", PF_GROUP(3)|PF_OBJECTLINK, "An object in group 4 that can be discarded or kept")
		ADD_STRINGPROP_FLAG(Group04Object04, "", PF_GROUP(3)|PF_OBJECTLINK, "An object in group 4 that can be discarded or kept")
		ADD_STRINGPROP_FLAG(Group04Object05, "", PF_GROUP(3)|PF_OBJECTLINK, "An object in group 4 that can be discarded or kept")
		ADD_STRINGPROP_FLAG(Group04Object06, "", PF_GROUP(3)|PF_OBJECTLINK, "An object in group 4 that can be discarded or kept")
		ADD_STRINGPROP_FLAG(Group04Object07, "", PF_GROUP(3)|PF_OBJECTLINK, "An object in group 4 that can be discarded or kept")
		ADD_STRINGPROP_FLAG(Group04Object08, "", PF_GROUP(3)|PF_OBJECTLINK, "An object in group 4 that can be discarded or kept")
		ADD_STRINGPROP_FLAG(Group04Object09, "", PF_GROUP(3)|PF_OBJECTLINK, "An object in group 4 that can be discarded or kept")
		ADD_STRINGPROP_FLAG(Group04Object10, "", PF_GROUP(3)|PF_OBJECTLINK, "An object in group 4 that can be discarded or kept")
		ADD_STRINGPROP_FLAG(Group05Object00, "", PF_GROUP(3)|PF_OBJECTLINK, "An object in group 5 that can be discarded or kept")
		ADD_STRINGPROP_FLAG(Group05Object01, "", PF_GROUP(3)|PF_OBJECTLINK, "An object in group 5 that can be discarded or kept")
		ADD_STRINGPROP_FLAG(Group05Object02, "", PF_GROUP(3)|PF_OBJECTLINK, "An object in group 5 that can be discarded or kept")
		ADD_STRINGPROP_FLAG(Group05Object03, "", PF_GROUP(3)|PF_OBJECTLINK, "An object in group 5 that can be discarded or kept")
		ADD_STRINGPROP_FLAG(Group05Object04, "", PF_GROUP(3)|PF_OBJECTLINK, "An object in group 5 that can be discarded or kept")
		ADD_STRINGPROP_FLAG(Group05Object05, "", PF_GROUP(3)|PF_OBJECTLINK, "An object in group 5 that can be discarded or kept")
		ADD_STRINGPROP_FLAG(Group05Object06, "", PF_GROUP(3)|PF_OBJECTLINK, "An object in group 5 that can be discarded or kept")
		ADD_STRINGPROP_FLAG(Group05Object07, "", PF_GROUP(3)|PF_OBJECTLINK, "An object in group 5 that can be discarded or kept")
		ADD_STRINGPROP_FLAG(Group05Object08, "", PF_GROUP(3)|PF_OBJECTLINK, "An object in group 5 that can be discarded or kept")
		ADD_STRINGPROP_FLAG(Group05Object09, "", PF_GROUP(3)|PF_OBJECTLINK, "An object in group 5 that can be discarded or kept")
		ADD_STRINGPROP_FLAG(Group05Object10, "", PF_GROUP(3)|PF_OBJECTLINK, "An object in group 5 that can be discarded or kept")

	PROP_DEFINEGROUP(Groups6and7, PF_GROUP(4), "This flag is a subset of two of the groups that the ObjectRemover can choose to keep or discard.")
		ADD_STRINGPROP_FLAG(Group06Object00, "", PF_GROUP(4)|PF_OBJECTLINK, "An object in group 6 that can be discarded or kept")
		ADD_STRINGPROP_FLAG(Group06Object01, "", PF_GROUP(4)|PF_OBJECTLINK, "An object in group 6 that can be discarded or kept")
		ADD_STRINGPROP_FLAG(Group06Object02, "", PF_GROUP(4)|PF_OBJECTLINK, "An object in group 6 that can be discarded or kept")
		ADD_STRINGPROP_FLAG(Group06Object03, "", PF_GROUP(4)|PF_OBJECTLINK, "An object in group 6 that can be discarded or kept")
		ADD_STRINGPROP_FLAG(Group06Object04, "", PF_GROUP(4)|PF_OBJECTLINK, "An object in group 6 that can be discarded or kept")
		ADD_STRINGPROP_FLAG(Group06Object05, "", PF_GROUP(4)|PF_OBJECTLINK, "An object in group 6 that can be discarded or kept")
		ADD_STRINGPROP_FLAG(Group06Object06, "", PF_GROUP(4)|PF_OBJECTLINK, "An object in group 6 that can be discarded or kept")
		ADD_STRINGPROP_FLAG(Group06Object07, "", PF_GROUP(4)|PF_OBJECTLINK, "An object in group 6 that can be discarded or kept")
		ADD_STRINGPROP_FLAG(Group06Object08, "", PF_GROUP(4)|PF_OBJECTLINK, "An object in group 6 that can be discarded or kept")
		ADD_STRINGPROP_FLAG(Group06Object09, "", PF_GROUP(4)|PF_OBJECTLINK, "An object in group 6 that can be discarded or kept")
		ADD_STRINGPROP_FLAG(Group06Object10, "", PF_GROUP(4)|PF_OBJECTLINK, "An object in group 6 that can be discarded or kept")
		ADD_STRINGPROP_FLAG(Group07Object00, "", PF_GROUP(4)|PF_OBJECTLINK, "An object in group 7 that can be discarded or kept")
		ADD_STRINGPROP_FLAG(Group07Object01, "", PF_GROUP(4)|PF_OBJECTLINK, "An object in group 7 that can be discarded or kept")
		ADD_STRINGPROP_FLAG(Group07Object02, "", PF_GROUP(4)|PF_OBJECTLINK, "An object in group 7 that can be discarded or kept")
		ADD_STRINGPROP_FLAG(Group07Object03, "", PF_GROUP(4)|PF_OBJECTLINK, "An object in group 7 that can be discarded or kept")
		ADD_STRINGPROP_FLAG(Group07Object04, "", PF_GROUP(4)|PF_OBJECTLINK, "An object in group 7 that can be discarded or kept")
		ADD_STRINGPROP_FLAG(Group07Object05, "", PF_GROUP(4)|PF_OBJECTLINK, "An object in group 7 that can be discarded or kept")
		ADD_STRINGPROP_FLAG(Group07Object06, "", PF_GROUP(4)|PF_OBJECTLINK, "An object in group 7 that can be discarded or kept")
		ADD_STRINGPROP_FLAG(Group07Object07, "", PF_GROUP(4)|PF_OBJECTLINK, "An object in group 7 that can be discarded or kept")
		ADD_STRINGPROP_FLAG(Group07Object08, "", PF_GROUP(4)|PF_OBJECTLINK, "An object in group 7 that can be discarded or kept")
		ADD_STRINGPROP_FLAG(Group07Object09, "", PF_GROUP(4)|PF_OBJECTLINK, "An object in group 7 that can be discarded or kept")
		ADD_STRINGPROP_FLAG(Group07Object10, "", PF_GROUP(4)|PF_OBJECTLINK, "An object in group 7 that can be discarded or kept")

	PROP_DEFINEGROUP(Groups8and9, PF_GROUP(5), "This flag is a subset of two of the groups that the ObjectRemover can choose to keep or discard.")
		ADD_STRINGPROP_FLAG(Group08Object00, "", PF_GROUP(5)|PF_OBJECTLINK, "An object in group 8 that can be discarded or kept")
		ADD_STRINGPROP_FLAG(Group08Object01, "", PF_GROUP(5)|PF_OBJECTLINK, "An object in group 8 that can be discarded or kept")
		ADD_STRINGPROP_FLAG(Group08Object02, "", PF_GROUP(5)|PF_OBJECTLINK, "An object in group 8 that can be discarded or kept")
		ADD_STRINGPROP_FLAG(Group08Object03, "", PF_GROUP(5)|PF_OBJECTLINK, "An object in group 8 that can be discarded or kept")
		ADD_STRINGPROP_FLAG(Group08Object04, "", PF_GROUP(5)|PF_OBJECTLINK, "An object in group 8 that can be discarded or kept")
		ADD_STRINGPROP_FLAG(Group08Object05, "", PF_GROUP(5)|PF_OBJECTLINK, "An object in group 8 that can be discarded or kept")
		ADD_STRINGPROP_FLAG(Group08Object06, "", PF_GROUP(5)|PF_OBJECTLINK, "An object in group 8 that can be discarded or kept")
		ADD_STRINGPROP_FLAG(Group08Object07, "", PF_GROUP(5)|PF_OBJECTLINK, "An object in group 8 that can be discarded or kept")
		ADD_STRINGPROP_FLAG(Group08Object08, "", PF_GROUP(5)|PF_OBJECTLINK, "An object in group 8 that can be discarded or kept")
		ADD_STRINGPROP_FLAG(Group08Object09, "", PF_GROUP(5)|PF_OBJECTLINK, "An object in group 8 that can be discarded or kept")
		ADD_STRINGPROP_FLAG(Group08Object10, "", PF_GROUP(5)|PF_OBJECTLINK, "An object in group 8 that can be discarded or kept")
		ADD_STRINGPROP_FLAG(Group09Object00, "", PF_GROUP(5)|PF_OBJECTLINK, "An object in group 9 that can be discarded or kept")
		ADD_STRINGPROP_FLAG(Group09Object01, "", PF_GROUP(5)|PF_OBJECTLINK, "An object in group 9 that can be discarded or kept")
		ADD_STRINGPROP_FLAG(Group09Object02, "", PF_GROUP(5)|PF_OBJECTLINK, "An object in group 9 that can be discarded or kept")
		ADD_STRINGPROP_FLAG(Group09Object03, "", PF_GROUP(5)|PF_OBJECTLINK, "An object in group 9 that can be discarded or kept")
		ADD_STRINGPROP_FLAG(Group09Object04, "", PF_GROUP(5)|PF_OBJECTLINK, "An object in group 9 that can be discarded or kept")
		ADD_STRINGPROP_FLAG(Group09Object05, "", PF_GROUP(5)|PF_OBJECTLINK, "An object in group 9 that can be discarded or kept")
		ADD_STRINGPROP_FLAG(Group09Object06, "", PF_GROUP(5)|PF_OBJECTLINK, "An object in group 9 that can be discarded or kept")
		ADD_STRINGPROP_FLAG(Group09Object07, "", PF_GROUP(5)|PF_OBJECTLINK, "An object in group 9 that can be discarded or kept")
		ADD_STRINGPROP_FLAG(Group09Object08, "", PF_GROUP(5)|PF_OBJECTLINK, "An object in group 9 that can be discarded or kept")
		ADD_STRINGPROP_FLAG(Group09Object09, "", PF_GROUP(5)|PF_OBJECTLINK, "An object in group 9 that can be discarded or kept")
		ADD_STRINGPROP_FLAG(Group09Object10, "", PF_GROUP(5)|PF_OBJECTLINK, "An object in group 9 that can be discarded or kept")

	PROP_DEFINEGROUP(Groups10and11, PF_GROUP(6), "This flag is a subset of two of the groups that the ObjectRemover can choose to keep or discard.")
		ADD_STRINGPROP_FLAG(Group10Object00, "", PF_GROUP(6)|PF_OBJECTLINK, "An object in group 10 that can be discarded or kept")
		ADD_STRINGPROP_FLAG(Group10Object01, "", PF_GROUP(6)|PF_OBJECTLINK, "An object in group 10 that can be discarded or kept")
		ADD_STRINGPROP_FLAG(Group10Object02, "", PF_GROUP(6)|PF_OBJECTLINK, "An object in group 10 that can be discarded or kept")
		ADD_STRINGPROP_FLAG(Group10Object03, "", PF_GROUP(6)|PF_OBJECTLINK, "An object in group 10 that can be discarded or kept")
		ADD_STRINGPROP_FLAG(Group10Object04, "", PF_GROUP(6)|PF_OBJECTLINK, "An object in group 10 that can be discarded or kept")
		ADD_STRINGPROP_FLAG(Group10Object05, "", PF_GROUP(6)|PF_OBJECTLINK, "An object in group 10 that can be discarded or kept")
		ADD_STRINGPROP_FLAG(Group10Object06, "", PF_GROUP(6)|PF_OBJECTLINK, "An object in group 10 that can be discarded or kept")
		ADD_STRINGPROP_FLAG(Group10Object07, "", PF_GROUP(6)|PF_OBJECTLINK, "An object in group 10 that can be discarded or kept")
		ADD_STRINGPROP_FLAG(Group10Object08, "", PF_GROUP(6)|PF_OBJECTLINK, "An object in group 10 that can be discarded or kept")
		ADD_STRINGPROP_FLAG(Group10Object09, "", PF_GROUP(6)|PF_OBJECTLINK, "An object in group 10 that can be discarded or kept")
		ADD_STRINGPROP_FLAG(Group10Object10, "", PF_GROUP(6)|PF_OBJECTLINK, "An object in group 10 that can be discarded or kept")
		ADD_STRINGPROP_FLAG(Group11Object00, "", PF_GROUP(6)|PF_OBJECTLINK, "An object in group 11 that can be discarded or kept")
		ADD_STRINGPROP_FLAG(Group11Object01, "", PF_GROUP(6)|PF_OBJECTLINK, "An object in group 11 that can be discarded or kept")
		ADD_STRINGPROP_FLAG(Group11Object02, "", PF_GROUP(6)|PF_OBJECTLINK, "An object in group 11 that can be discarded or kept")
		ADD_STRINGPROP_FLAG(Group11Object03, "", PF_GROUP(6)|PF_OBJECTLINK, "An object in group 11 that can be discarded or kept")
		ADD_STRINGPROP_FLAG(Group11Object04, "", PF_GROUP(6)|PF_OBJECTLINK, "An object in group 11 that can be discarded or kept")
		ADD_STRINGPROP_FLAG(Group11Object05, "", PF_GROUP(6)|PF_OBJECTLINK, "An object in group 11 that can be discarded or kept")
		ADD_STRINGPROP_FLAG(Group11Object06, "", PF_GROUP(6)|PF_OBJECTLINK, "An object in group 11 that can be discarded or kept")
		ADD_STRINGPROP_FLAG(Group11Object07, "", PF_GROUP(6)|PF_OBJECTLINK, "An object in group 11 that can be discarded or kept")
		ADD_STRINGPROP_FLAG(Group11Object08, "", PF_GROUP(6)|PF_OBJECTLINK, "An object in group 11 that can be discarded or kept")
		ADD_STRINGPROP_FLAG(Group11Object09, "", PF_GROUP(6)|PF_OBJECTLINK, "An object in group 11 that can be discarded or kept")
		ADD_STRINGPROP_FLAG(Group11Object10, "", PF_GROUP(6)|PF_OBJECTLINK, "An object in group 11 that can be discarded or kept")

	ADD_LONGINTPROP(GroupsToKeep, 1, "This value determines how many of the groups to keep. The object will draw from the entered groups of objects and remove all but this number of groups.")

	PROP_DEFINEGROUP(GameModes, PF_GROUP(7), "This will bring up a dialog where you can set which game modes this object will be allowed in.")
		ADD_BOOLPROP_FLAG(SinglePlayer, true, PF_GROUP(7), "Allow object for Singleplayer games.")
		ADD_STRINGPROP_FLAG(GameMode0, GameModeNone, PF_GROUP(7) | PF_STATICLIST, "Allow object for this mode.")
		ADD_STRINGPROP_FLAG(GameMode1, GameModeNone, PF_GROUP(7) | PF_STATICLIST, "Allow object for this mode.")
		ADD_STRINGPROP_FLAG(GameMode2, GameModeNone, PF_GROUP(7) | PF_STATICLIST, "Allow object for this mode.")
		ADD_STRINGPROP_FLAG(GameMode3, GameModeNone, PF_GROUP(7) | PF_STATICLIST, "Allow object for this mode.")
		ADD_STRINGPROP_FLAG(GameMode4, GameModeNone, PF_GROUP(7) | PF_STATICLIST, "Allow object for this mode.")
		ADD_STRINGPROP_FLAG(GameMode5, GameModeNone, PF_GROUP(7) | PF_STATICLIST, "Allow object for this mode.")
		ADD_STRINGPROP_FLAG(GameMode6, GameModeNone, PF_GROUP(7) | PF_STATICLIST, "Allow object for this mode.")
		ADD_STRINGPROP_FLAG(GameMode7, GameModeNone, PF_GROUP(7) | PF_STATICLIST, "Allow object for this mode.")

END_CLASS_FLAGS_PLUGIN(ObjectRemover, GameBase, 0, CObjectRemoverPlugin, "Remove groups of objects when level starts.  If the number of GroupsToKeep is less than the number of groups defined then the groups removed will be chosen and random. ")


CMDMGR_BEGIN_REGISTER_CLASS( ObjectRemover )
CMDMGR_END_REGISTER_CLASS( ObjectRemover, GameBase )


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ObjectRemover::ObjectRemover()
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

ObjectRemover::ObjectRemover() : GameBase()
{
	m_cGroupsToKeep = 1;

}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ObjectRemover::~ObjectRemover()
//
//	PURPOSE:	Destructor
//
// ----------------------------------------------------------------------- //

ObjectRemover::~ObjectRemover()
{
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ObjectRemover::EngineMessageFn
//
//	PURPOSE:	Handle engine messages
//
// ----------------------------------------------------------------------- //

uint32 ObjectRemover::EngineMessageFn(uint32 messageID, void *pData, float fData)
{
	switch(messageID)
	{
		case MID_ALLOBJECTSCREATED:
		{
			AllObjectsCreated();
			break;
		}

		case MID_PRECREATE:
		{
			if (fData == PRECREATE_WORLDFILE || fData == PRECREATE_STRINGPROP)
			{
				// If the readprop failed, then delete this whole object.
				if( !ReadProp(&((ObjectCreateStruct*)pData)->m_cProperties))
					return LT_ERROR;
			}

			break;
		}

		case MID_INITIALUPDATE:
		{
			break;
		}

		case MID_SAVEOBJECT:
		{
			Save((ILTMessage_Write*)pData);
		}
		break;

		case MID_LOADOBJECT:
		{
			Load((ILTMessage_Read*)pData);
		}
		break;

		default : break;
	}

	return GameBase::EngineMessageFn(messageID, pData, fData);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ObjectRemover::ReadProp
//
//	PURPOSE:	Set property value
//
// ----------------------------------------------------------------------- //

bool ObjectRemover::ReadProp(const GenericPropList *pProps)
{
    if( !pProps )
		return false;

	char szProp[128] = {0};
	const char *pszObject = NULL;

	// Get the name of the current mode.
	char const* pszModeName = GameModeMgr::Instance().GetGameModeName();

	// Do special case if it's sp.
	if( LTStrEquals( pszModeName, GameModeMgr::GetSinglePlayerRecordName( )))
	{
		// Check if allowed.  If not, then return false, which will delete this ObjectRemover.
		if( !pProps->GetBool( "SinglePlayer", true ))
			return false;
	}
	// Current game not SP, so check the other modes.
	else
	{
		// Assume that the current game mode is not listed in the allowed game modes.  If
		// we don't find it, then this ObjectRemover will be deleted.
		bool bAllowed = false;

		// Look through each of the gamemode properties and see if it matches the current
		// game mode.
		char szGameModeProp[256];
		for( int i = 0; i < NUM_GAME_MODE_PROPS; i++ )
		{
			LTSNPrintF( szGameModeProp, LTARRAYSIZE( szGameModeProp ), "GameMode%d", i );
			char const* pszGameModeName = pProps->GetString( szGameModeProp, "" );
			if( LTStrEquals( pszModeName, pszGameModeName ))
			{
				// The objectremover is allowed for the current gamemode.
				bAllowed = true;
				break;
			}
		}

		// If the objectremover isn't allowed for the current gamemode, then delete it by returning false.
		if( !bAllowed )
			return false;
	}

	for( int32 iGroup = 0 ; iGroup < kMaxGroups ; ++iGroup )
	{
		for( int32 iObject = 0 ; iObject < kMaxObjectsPerGroup ; ++iObject )
		{
			LTSNPrintF( szProp, ARRAY_LEN(szProp), "Group%2.2dObject%2.2d", iGroup, iObject );
			pszObject = pProps->GetString( szProp, "" );
			
			if( pszObject && pszObject[0] )
			{
				m_astrObjects[iGroup][iObject] = pszObject;
			}
		}
	}

	m_cGroupsToKeep = pProps->GetLongInt( "GroupsToKeep", m_cGroupsToKeep );

    return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ObjectRemover::AllObjectsCreated()
//
//	PURPOSE:	AllObjectsCreated
//
// ----------------------------------------------------------------------- //

bool ObjectRemover::AllObjectsCreated()
{
	HOBJECT ahObjects[kMaxGroups][kMaxObjectsPerGroup][128];
    memset(ahObjects, 0, sizeof(HOBJECT)*kMaxGroups*kMaxObjectsPerGroup*128);

	int cGroupsWithObjects = 0;

	{for ( int iGroup = 0 ; iGroup < kMaxGroups ; iGroup++ )
	{
        bool bGroupHasObjects = false;

		{for ( int iObject = 0 ; iObject < kMaxObjectsPerGroup ; iObject++ )
		{
			if ( !m_astrObjects[iGroup][iObject].empty() ) 
			{
				char szString[1024];
				strcpy(szString, m_astrObjects[iGroup][iObject].c_str());

				uint32 cTokens = 0;

				const char* szToken = strtok(szString, ";");

				while ( szToken )
				{
					HOBJECT hObject;
					if ( LT_OK == FindNamedObject(szToken, hObject) )
					{
						ahObjects[cGroupsWithObjects][iObject][cTokens++] = hObject;
						bGroupHasObjects = true;
					}

					szToken = strtok(NULL, ";");
				}
			}
		}}

		if ( bGroupHasObjects )
		{
			cGroupsWithObjects++;
		}
	}}

	// Remove the objects

    bool abRemoved[kMaxGroups];
    memset(abRemoved, false, sizeof(bool)*kMaxGroups);
	int iSafety = 50000;
	int cRemove = cGroupsWithObjects-m_cGroupsToKeep;

	while ( (cRemove > 0) && (--iSafety > 0) )
	{
		int iRemove = GetRandom(0, cGroupsWithObjects-1);
		if ( !abRemoved[iRemove] )
		{
			for ( int iObject = 0 ; iObject < kMaxObjectsPerGroup ; iObject++ )
			{
				for ( int iToken = 0 ; iToken < 128 ; iToken++ )
				{
					if ( ahObjects[iRemove][iObject][iToken] )
					{
						if ( IsAI(ahObjects[iRemove][iObject][iToken]) )
						{
							ILTBaseClass *pAI = g_pLTServer->HandleToObject( ahObjects[iRemove][iObject][iToken] );
							g_pCmdMgr->QueueMessage( this, pAI, "REMOVE" );
						}
						else
						{
							g_pLTServer->RemoveObject(ahObjects[iRemove][iObject][iToken]);
						}
					}
				}
			}

            abRemoved[iRemove] = true;
			cRemove--;
		}
	}

	// Remove ourselves...

    g_pLTServer->RemoveObject(m_hObject);

    return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ObjectRemover::Save
//
//	PURPOSE:	Save the object
//
// ----------------------------------------------------------------------- //

void ObjectRemover::Save(ILTMessage_Write *pMsg)
{
	if (!pMsg) return;

	for ( int iGroup = 0 ; iGroup < kMaxGroups ; iGroup++ )
	{
		for ( int iObject = 0 ; iObject < kMaxObjectsPerGroup ; iObject++ )
		{
			SAVE_STDSTRING(m_astrObjects[iGroup][iObject]);
		}
	}

	SAVE_INT(m_cGroupsToKeep);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ObjectRemover::Load
//
//	PURPOSE:	Load the object
//
// ----------------------------------------------------------------------- //

void ObjectRemover::Load(ILTMessage_Read *pMsg)
{
	if (!pMsg) return;

	for ( int iGroup = 0 ; iGroup < kMaxGroups ; iGroup++ )
	{
		for ( int iObject = 0 ; iObject < kMaxObjectsPerGroup ; iObject++ )
		{
			LOAD_STDSTRING(m_astrObjects[iGroup][iObject]);
		}
	}

	LOAD_INT(m_cGroupsToKeep);
}

// Initialize our search table.
static CParsedMsg::CToken mpModeTable[] = 
{
	"GameMode0",
	"GameMode1",
	"GameMode2",
	"GameMode3",
	"GameMode4",
	"GameMode5",
	"GameMode6",
	"GameMode7",
};
static uint32 nGameModeTableCount = LTARRAYSIZE( mpModeTable );

LTRESULT CObjectRemoverPlugin::PreHook_EditStringList(
	const char* szRezPath,
	const char* szPropName,
	char** aszStrings,
	uint32* pcStrings,
	const uint32 cMaxStrings,
	const uint32 cMaxStringLength)
{
	CParsedMsg::CToken cTok( szPropName );

	// Check if the property is one of the GameModeX's.
	bool bFound = false;
	for( uint32 i = 0; i < nGameModeTableCount; i++ )
	{
		if( mpModeTable[i] == cTok )
		{
			bFound = true;
			break;
		}
	}

	if( bFound )
	{
		LTStrCpy(aszStrings[(*pcStrings)++], GameModeNone, cMaxStringLength );

		uint32 nNumGameModes = DATABASE_CATEGORY( GameModes ).GetNumRecords( );
		for( uint32 nGameMode = 0; nGameMode < nNumGameModes; nGameMode++ )
		{
			// exit out early if we can't hold any more strings
			if( *pcStrings >= cMaxStrings )
				break;

			HRECORD hGameModeRecord = DATABASE_CATEGORY( GameModes ).GetRecordByIndex( nGameMode );

			// Check if this isn't a multiplayer mode.
			if( !DATABASE_CATEGORY( GameModes ).GETRECORDATTRIB( hGameModeRecord, Multiplayer ))
				continue;

			LTStrCpy( aszStrings[(*pcStrings)++], g_pLTDatabase->GetRecordName( hGameModeRecord ), cMaxStringLength );
		}

		qsort( aszStrings + 1, *pcStrings - 1, sizeof( char * ), CaseInsensitiveCompare );

		return LT_OK;
	}

	return LT_UNSUPPORTED;
}

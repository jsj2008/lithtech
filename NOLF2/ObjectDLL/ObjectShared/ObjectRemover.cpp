// ----------------------------------------------------------------------- //
//
// MODULE  : ObjectRemover.cpp
//
// PURPOSE : ObjectRemover - Implementation
//
// CREATED : 04.23.1999
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "ObjectRemover.h"
#include "ServerUtilities.h"
#include "ObjectMsgs.h"
#include "AIUtils.h"
#include "time.h"

LINKFROM_MODULE( ObjectRemover );

#pragma force_active on
BEGIN_CLASS(ObjectRemover)

	PROP_DEFINEGROUP(Groups0and1, PF_GROUP(1))
		ADD_STRINGPROP_FLAG(Group00Object00, "", PF_GROUP(1)|PF_OBJECTLINK)
		ADD_STRINGPROP_FLAG(Group00Object01, "", PF_GROUP(1)|PF_OBJECTLINK)
		ADD_STRINGPROP_FLAG(Group00Object02, "", PF_GROUP(1)|PF_OBJECTLINK)
		ADD_STRINGPROP_FLAG(Group00Object03, "", PF_GROUP(1)|PF_OBJECTLINK)
		ADD_STRINGPROP_FLAG(Group00Object04, "", PF_GROUP(1)|PF_OBJECTLINK)
		ADD_STRINGPROP_FLAG(Group00Object05, "", PF_GROUP(1)|PF_OBJECTLINK)
		ADD_STRINGPROP_FLAG(Group00Object06, "", PF_GROUP(1)|PF_OBJECTLINK)
		ADD_STRINGPROP_FLAG(Group00Object07, "", PF_GROUP(1)|PF_OBJECTLINK)
		ADD_STRINGPROP_FLAG(Group00Object08, "", PF_GROUP(1)|PF_OBJECTLINK)
		ADD_STRINGPROP_FLAG(Group00Object09, "", PF_GROUP(1)|PF_OBJECTLINK)
		ADD_STRINGPROP_FLAG(Group00Object10, "", PF_GROUP(1)|PF_OBJECTLINK)
		ADD_STRINGPROP_FLAG(Group01Object00, "", PF_GROUP(1)|PF_OBJECTLINK)
		ADD_STRINGPROP_FLAG(Group01Object01, "", PF_GROUP(1)|PF_OBJECTLINK)
		ADD_STRINGPROP_FLAG(Group01Object02, "", PF_GROUP(1)|PF_OBJECTLINK)
		ADD_STRINGPROP_FLAG(Group01Object03, "", PF_GROUP(1)|PF_OBJECTLINK)
		ADD_STRINGPROP_FLAG(Group01Object04, "", PF_GROUP(1)|PF_OBJECTLINK)
		ADD_STRINGPROP_FLAG(Group01Object05, "", PF_GROUP(1)|PF_OBJECTLINK)
		ADD_STRINGPROP_FLAG(Group01Object06, "", PF_GROUP(1)|PF_OBJECTLINK)
		ADD_STRINGPROP_FLAG(Group01Object07, "", PF_GROUP(1)|PF_OBJECTLINK)
		ADD_STRINGPROP_FLAG(Group01Object08, "", PF_GROUP(1)|PF_OBJECTLINK)
		ADD_STRINGPROP_FLAG(Group01Object09, "", PF_GROUP(1)|PF_OBJECTLINK)
		ADD_STRINGPROP_FLAG(Group01Object10, "", PF_GROUP(1)|PF_OBJECTLINK)

	PROP_DEFINEGROUP(Groups2and3, PF_GROUP(2))
		ADD_STRINGPROP_FLAG(Group02Object00, "", PF_GROUP(2)|PF_OBJECTLINK)
		ADD_STRINGPROP_FLAG(Group02Object01, "", PF_GROUP(2)|PF_OBJECTLINK)
		ADD_STRINGPROP_FLAG(Group02Object02, "", PF_GROUP(2)|PF_OBJECTLINK)
		ADD_STRINGPROP_FLAG(Group02Object03, "", PF_GROUP(2)|PF_OBJECTLINK)
		ADD_STRINGPROP_FLAG(Group02Object04, "", PF_GROUP(2)|PF_OBJECTLINK)
		ADD_STRINGPROP_FLAG(Group02Object05, "", PF_GROUP(2)|PF_OBJECTLINK)
		ADD_STRINGPROP_FLAG(Group02Object06, "", PF_GROUP(2)|PF_OBJECTLINK)
		ADD_STRINGPROP_FLAG(Group02Object07, "", PF_GROUP(2)|PF_OBJECTLINK)
		ADD_STRINGPROP_FLAG(Group02Object08, "", PF_GROUP(2)|PF_OBJECTLINK)
		ADD_STRINGPROP_FLAG(Group02Object09, "", PF_GROUP(2)|PF_OBJECTLINK)
		ADD_STRINGPROP_FLAG(Group02Object10, "", PF_GROUP(2)|PF_OBJECTLINK)
		ADD_STRINGPROP_FLAG(Group03Object00, "", PF_GROUP(2)|PF_OBJECTLINK)
		ADD_STRINGPROP_FLAG(Group03Object01, "", PF_GROUP(2)|PF_OBJECTLINK)
		ADD_STRINGPROP_FLAG(Group03Object02, "", PF_GROUP(2)|PF_OBJECTLINK)
		ADD_STRINGPROP_FLAG(Group03Object03, "", PF_GROUP(2)|PF_OBJECTLINK)
		ADD_STRINGPROP_FLAG(Group03Object04, "", PF_GROUP(2)|PF_OBJECTLINK)
		ADD_STRINGPROP_FLAG(Group03Object05, "", PF_GROUP(2)|PF_OBJECTLINK)
		ADD_STRINGPROP_FLAG(Group03Object06, "", PF_GROUP(2)|PF_OBJECTLINK)
		ADD_STRINGPROP_FLAG(Group03Object07, "", PF_GROUP(2)|PF_OBJECTLINK)
		ADD_STRINGPROP_FLAG(Group03Object08, "", PF_GROUP(2)|PF_OBJECTLINK)
		ADD_STRINGPROP_FLAG(Group03Object09, "", PF_GROUP(2)|PF_OBJECTLINK)
		ADD_STRINGPROP_FLAG(Group03Object10, "", PF_GROUP(2)|PF_OBJECTLINK)

	PROP_DEFINEGROUP(Groups4and5, PF_GROUP(3))
		ADD_STRINGPROP_FLAG(Group04Object00, "", PF_GROUP(3)|PF_OBJECTLINK)
		ADD_STRINGPROP_FLAG(Group04Object01, "", PF_GROUP(3)|PF_OBJECTLINK)
		ADD_STRINGPROP_FLAG(Group04Object02, "", PF_GROUP(3)|PF_OBJECTLINK)
		ADD_STRINGPROP_FLAG(Group04Object03, "", PF_GROUP(3)|PF_OBJECTLINK)
		ADD_STRINGPROP_FLAG(Group04Object04, "", PF_GROUP(3)|PF_OBJECTLINK)
		ADD_STRINGPROP_FLAG(Group04Object05, "", PF_GROUP(3)|PF_OBJECTLINK)
		ADD_STRINGPROP_FLAG(Group04Object06, "", PF_GROUP(3)|PF_OBJECTLINK)
		ADD_STRINGPROP_FLAG(Group04Object07, "", PF_GROUP(3)|PF_OBJECTLINK)
		ADD_STRINGPROP_FLAG(Group04Object08, "", PF_GROUP(3)|PF_OBJECTLINK)
		ADD_STRINGPROP_FLAG(Group04Object09, "", PF_GROUP(3)|PF_OBJECTLINK)
		ADD_STRINGPROP_FLAG(Group04Object10, "", PF_GROUP(3)|PF_OBJECTLINK)
		ADD_STRINGPROP_FLAG(Group05Object00, "", PF_GROUP(3)|PF_OBJECTLINK)
		ADD_STRINGPROP_FLAG(Group05Object01, "", PF_GROUP(3)|PF_OBJECTLINK)
		ADD_STRINGPROP_FLAG(Group05Object02, "", PF_GROUP(3)|PF_OBJECTLINK)
		ADD_STRINGPROP_FLAG(Group05Object03, "", PF_GROUP(3)|PF_OBJECTLINK)
		ADD_STRINGPROP_FLAG(Group05Object04, "", PF_GROUP(3)|PF_OBJECTLINK)
		ADD_STRINGPROP_FLAG(Group05Object05, "", PF_GROUP(3)|PF_OBJECTLINK)
		ADD_STRINGPROP_FLAG(Group05Object06, "", PF_GROUP(3)|PF_OBJECTLINK)
		ADD_STRINGPROP_FLAG(Group05Object07, "", PF_GROUP(3)|PF_OBJECTLINK)
		ADD_STRINGPROP_FLAG(Group05Object08, "", PF_GROUP(3)|PF_OBJECTLINK)
		ADD_STRINGPROP_FLAG(Group05Object09, "", PF_GROUP(3)|PF_OBJECTLINK)
		ADD_STRINGPROP_FLAG(Group05Object10, "", PF_GROUP(3)|PF_OBJECTLINK)

	PROP_DEFINEGROUP(Groups6and7, PF_GROUP(4))
		ADD_STRINGPROP_FLAG(Group06Object00, "", PF_GROUP(4)|PF_OBJECTLINK)
		ADD_STRINGPROP_FLAG(Group06Object01, "", PF_GROUP(4)|PF_OBJECTLINK)
		ADD_STRINGPROP_FLAG(Group06Object02, "", PF_GROUP(4)|PF_OBJECTLINK)
		ADD_STRINGPROP_FLAG(Group06Object03, "", PF_GROUP(4)|PF_OBJECTLINK)
		ADD_STRINGPROP_FLAG(Group06Object04, "", PF_GROUP(4)|PF_OBJECTLINK)
		ADD_STRINGPROP_FLAG(Group06Object05, "", PF_GROUP(4)|PF_OBJECTLINK)
		ADD_STRINGPROP_FLAG(Group06Object06, "", PF_GROUP(4)|PF_OBJECTLINK)
		ADD_STRINGPROP_FLAG(Group06Object07, "", PF_GROUP(4)|PF_OBJECTLINK)
		ADD_STRINGPROP_FLAG(Group06Object08, "", PF_GROUP(4)|PF_OBJECTLINK)
		ADD_STRINGPROP_FLAG(Group06Object09, "", PF_GROUP(4)|PF_OBJECTLINK)
		ADD_STRINGPROP_FLAG(Group06Object10, "", PF_GROUP(4)|PF_OBJECTLINK)
		ADD_STRINGPROP_FLAG(Group07Object00, "", PF_GROUP(4)|PF_OBJECTLINK)
		ADD_STRINGPROP_FLAG(Group07Object01, "", PF_GROUP(4)|PF_OBJECTLINK)
		ADD_STRINGPROP_FLAG(Group07Object02, "", PF_GROUP(4)|PF_OBJECTLINK)
		ADD_STRINGPROP_FLAG(Group07Object03, "", PF_GROUP(4)|PF_OBJECTLINK)
		ADD_STRINGPROP_FLAG(Group07Object04, "", PF_GROUP(4)|PF_OBJECTLINK)
		ADD_STRINGPROP_FLAG(Group07Object05, "", PF_GROUP(4)|PF_OBJECTLINK)
		ADD_STRINGPROP_FLAG(Group07Object06, "", PF_GROUP(4)|PF_OBJECTLINK)
		ADD_STRINGPROP_FLAG(Group07Object07, "", PF_GROUP(4)|PF_OBJECTLINK)
		ADD_STRINGPROP_FLAG(Group07Object08, "", PF_GROUP(4)|PF_OBJECTLINK)
		ADD_STRINGPROP_FLAG(Group07Object09, "", PF_GROUP(4)|PF_OBJECTLINK)
		ADD_STRINGPROP_FLAG(Group07Object10, "", PF_GROUP(4)|PF_OBJECTLINK)

	PROP_DEFINEGROUP(Groups8and9, PF_GROUP(5))
		ADD_STRINGPROP_FLAG(Group08Object00, "", PF_GROUP(5)|PF_OBJECTLINK)
		ADD_STRINGPROP_FLAG(Group08Object01, "", PF_GROUP(5)|PF_OBJECTLINK)
		ADD_STRINGPROP_FLAG(Group08Object02, "", PF_GROUP(5)|PF_OBJECTLINK)
		ADD_STRINGPROP_FLAG(Group08Object03, "", PF_GROUP(5)|PF_OBJECTLINK)
		ADD_STRINGPROP_FLAG(Group08Object04, "", PF_GROUP(5)|PF_OBJECTLINK)
		ADD_STRINGPROP_FLAG(Group08Object05, "", PF_GROUP(5)|PF_OBJECTLINK)
		ADD_STRINGPROP_FLAG(Group08Object06, "", PF_GROUP(5)|PF_OBJECTLINK)
		ADD_STRINGPROP_FLAG(Group08Object07, "", PF_GROUP(5)|PF_OBJECTLINK)
		ADD_STRINGPROP_FLAG(Group08Object08, "", PF_GROUP(5)|PF_OBJECTLINK)
		ADD_STRINGPROP_FLAG(Group08Object09, "", PF_GROUP(5)|PF_OBJECTLINK)
		ADD_STRINGPROP_FLAG(Group08Object10, "", PF_GROUP(5)|PF_OBJECTLINK)
		ADD_STRINGPROP_FLAG(Group09Object00, "", PF_GROUP(5)|PF_OBJECTLINK)
		ADD_STRINGPROP_FLAG(Group09Object01, "", PF_GROUP(5)|PF_OBJECTLINK)
		ADD_STRINGPROP_FLAG(Group09Object02, "", PF_GROUP(5)|PF_OBJECTLINK)
		ADD_STRINGPROP_FLAG(Group09Object03, "", PF_GROUP(5)|PF_OBJECTLINK)
		ADD_STRINGPROP_FLAG(Group09Object04, "", PF_GROUP(5)|PF_OBJECTLINK)
		ADD_STRINGPROP_FLAG(Group09Object05, "", PF_GROUP(5)|PF_OBJECTLINK)
		ADD_STRINGPROP_FLAG(Group09Object06, "", PF_GROUP(5)|PF_OBJECTLINK)
		ADD_STRINGPROP_FLAG(Group09Object07, "", PF_GROUP(5)|PF_OBJECTLINK)
		ADD_STRINGPROP_FLAG(Group09Object08, "", PF_GROUP(5)|PF_OBJECTLINK)
		ADD_STRINGPROP_FLAG(Group09Object09, "", PF_GROUP(5)|PF_OBJECTLINK)
		ADD_STRINGPROP_FLAG(Group09Object10, "", PF_GROUP(5)|PF_OBJECTLINK)

	PROP_DEFINEGROUP(Groups10and11, PF_GROUP(6))
		ADD_STRINGPROP_FLAG(Group10Object00, "", PF_GROUP(6)|PF_OBJECTLINK)
		ADD_STRINGPROP_FLAG(Group10Object01, "", PF_GROUP(6)|PF_OBJECTLINK)
		ADD_STRINGPROP_FLAG(Group10Object02, "", PF_GROUP(6)|PF_OBJECTLINK)
		ADD_STRINGPROP_FLAG(Group10Object03, "", PF_GROUP(6)|PF_OBJECTLINK)
		ADD_STRINGPROP_FLAG(Group10Object04, "", PF_GROUP(6)|PF_OBJECTLINK)
		ADD_STRINGPROP_FLAG(Group10Object05, "", PF_GROUP(6)|PF_OBJECTLINK)
		ADD_STRINGPROP_FLAG(Group10Object06, "", PF_GROUP(6)|PF_OBJECTLINK)
		ADD_STRINGPROP_FLAG(Group10Object07, "", PF_GROUP(6)|PF_OBJECTLINK)
		ADD_STRINGPROP_FLAG(Group10Object08, "", PF_GROUP(6)|PF_OBJECTLINK)
		ADD_STRINGPROP_FLAG(Group10Object09, "", PF_GROUP(6)|PF_OBJECTLINK)
		ADD_STRINGPROP_FLAG(Group10Object10, "", PF_GROUP(6)|PF_OBJECTLINK)
		ADD_STRINGPROP_FLAG(Group11Object00, "", PF_GROUP(6)|PF_OBJECTLINK)
		ADD_STRINGPROP_FLAG(Group11Object01, "", PF_GROUP(6)|PF_OBJECTLINK)
		ADD_STRINGPROP_FLAG(Group11Object02, "", PF_GROUP(6)|PF_OBJECTLINK)
		ADD_STRINGPROP_FLAG(Group11Object03, "", PF_GROUP(6)|PF_OBJECTLINK)
		ADD_STRINGPROP_FLAG(Group11Object04, "", PF_GROUP(6)|PF_OBJECTLINK)
		ADD_STRINGPROP_FLAG(Group11Object05, "", PF_GROUP(6)|PF_OBJECTLINK)
		ADD_STRINGPROP_FLAG(Group11Object06, "", PF_GROUP(6)|PF_OBJECTLINK)
		ADD_STRINGPROP_FLAG(Group11Object07, "", PF_GROUP(6)|PF_OBJECTLINK)
		ADD_STRINGPROP_FLAG(Group11Object08, "", PF_GROUP(6)|PF_OBJECTLINK)
		ADD_STRINGPROP_FLAG(Group11Object09, "", PF_GROUP(6)|PF_OBJECTLINK)
		ADD_STRINGPROP_FLAG(Group11Object10, "", PF_GROUP(6)|PF_OBJECTLINK)

	ADD_LONGINTPROP(GroupsToKeep, 1)

END_CLASS_DEFAULT(ObjectRemover, BaseClass, NULL, NULL)
#pragma force_active off

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ObjectRemover::ObjectRemover()
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

ObjectRemover::ObjectRemover() : BaseClass()
{
	m_cGroupsToKeep = 1;

	for ( int iGroup = 0 ; iGroup < kMaxGroups ; iGroup++ )
	{
		for ( int iObject = 0 ; iObject < kMaxObjectsPerGroup ; iObject++ )
		{
            m_ahstrObjects[iGroup][iObject] = LTNULL;
		}
	}
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
	for ( int iGroup = 0 ; iGroup < kMaxGroups ; iGroup++ )
	{
		for ( int iObject = 0 ; iObject < kMaxObjectsPerGroup ; iObject++ )
		{
			FREE_HSTRING(m_ahstrObjects[iGroup][iObject]);
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ObjectRemover::EngineMessageFn
//
//	PURPOSE:	Handle engine messages
//
// ----------------------------------------------------------------------- //

uint32 ObjectRemover::EngineMessageFn(uint32 messageID, void *pData, LTFLOAT fData)
{
	switch(messageID)
	{
		case MID_UPDATE:
		{
			Update();
			break;
		}

		case MID_PRECREATE:
		{
			if (fData == PRECREATE_WORLDFILE || fData == PRECREATE_STRINGPROP)
			{
				ReadProp((ObjectCreateStruct*)pData);
			}

			break;
		}

		case MID_INITIALUPDATE:
		{
			if (fData != INITIALUPDATE_SAVEGAME)
			{
                SetNextUpdate(m_hObject, UPDATE_NEXT_FRAME);
			}

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

	return BaseClass::EngineMessageFn(messageID, pData, fData);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ObjectRemover::ReadProp
//
//	PURPOSE:	Set property value
//
// ----------------------------------------------------------------------- //

LTBOOL ObjectRemover::ReadProp(ObjectCreateStruct *pInfo)
{
    if (!pInfo) return LTFALSE;

	GenericProp genProp;

	for ( int iGroup = 0 ; iGroup < kMaxGroups ; iGroup++ )
	{
		for ( int iObject = 0 ; iObject < kMaxObjectsPerGroup ; iObject++ )
		{
			char szProp[128];
			sprintf(szProp, "Group%2.2dObject%2.2d", iGroup, iObject);
            if ( g_pLTServer->GetPropGeneric( szProp, &genProp ) == LT_OK )
				if ( genProp.m_String[0] )
                    m_ahstrObjects[iGroup][iObject] = g_pLTServer->CreateString( genProp.m_String );
		}
	}

    if ( g_pLTServer->GetPropGeneric( "GroupsToKeep", &genProp ) == LT_OK )
		m_cGroupsToKeep = genProp.m_Long;

    return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ObjectRemover::Update()
//
//	PURPOSE:	Update
//
// ----------------------------------------------------------------------- //

LTBOOL ObjectRemover::Update()
{
	HOBJECT ahObjects[kMaxGroups][kMaxObjectsPerGroup][128];
    memset(ahObjects, LTNULL, sizeof(HOBJECT)*kMaxGroups*kMaxObjectsPerGroup*128);

	int cGroupsWithObjects = 0;

	{for ( int iGroup = 0 ; iGroup < kMaxGroups ; iGroup++ )
	{
        LTBOOL bGroupHasObjects = LTFALSE;

		{for ( int iObject = 0 ; iObject < kMaxObjectsPerGroup ; iObject++ )
		{
			if ( m_ahstrObjects[iGroup][iObject] ) 
			{
				char szString[1024];
				strcpy(szString, g_pLTServer->GetStringData(m_ahstrObjects[iGroup][iObject]));

				uint32 cTokens = 0;

				const char* szToken = strtok(szString, ";");

				while ( szToken )
				{
					HOBJECT hObject;
					if ( LT_OK == FindNamedObject(szToken, hObject) )
					{
						ahObjects[cGroupsWithObjects][iObject][cTokens++] = hObject;
						bGroupHasObjects = LTTRUE;
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

    LTBOOL abRemoved[kMaxGroups];
    memset(abRemoved, LTFALSE, sizeof(LTBOOL)*kMaxGroups);
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
							SendTriggerMsgToObject(this, ahObjects[iRemove][iObject][iToken], LTFALSE, "REMOVE");
						}
						else
						{
							g_pLTServer->RemoveObject(ahObjects[iRemove][iObject][iToken]);
						}
					}
				}
			}

            abRemoved[iRemove] = LTTRUE;
			cRemove--;
		}
	}

	// Remove ourselves...

    g_pLTServer->RemoveObject(m_hObject);

    return LTTRUE;
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
			SAVE_HSTRING(m_ahstrObjects[iGroup][iObject]);
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
			LOAD_HSTRING(m_ahstrObjects[iGroup][iObject]);
		}
	}

	LOAD_INT(m_cGroupsToKeep);
}
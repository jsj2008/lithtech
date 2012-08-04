// ----------------------------------------------------------------------- //
//
// MODULE  : PhysicsImpulseDirectional.cpp
//
// PURPOSE : PhysicsImpulseDirectional - Implementation
//
// CREATED : 04/16/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "PhysicsImpulseDirectional.h"
#include "PhysicsUtilities.h"

#define MAX_IMPULSE_TARGETS		50

LINKFROM_MODULE( PhysicsImpulseDirectional );

BEGIN_CLASS(PhysicsImpulseDirectional)

	ADD_REALPROP_FLAG(ImpulseForce, PhysicsUtilities::DEFAULT_IMPULSE_FORCE, 0, "The amount of force the PhysicsImpulseDirectional object will apply to the objects specified below.  The direction of the force is applied in the direction along the forward vector of the PhysicsImpulseDirectional object.  This value is measured in game unit newton seconds.")
	ADD_BOOLPROP_FLAG(RemoveWhenDone, true, 0, "This flag toggles whether or not the PhysicsImpulseDirectional object is removed after being triggered. Sometimes you may want to trigger an PhysicsImpulseDirectional object repeatedly. In which case you would set this flag to false.")
	ADD_STRINGPROP_FLAG(Object1, "", PF_OBJECTLINK, "This is the name of an object that will receive ImpulseForce amount of force applied in the direction specified by the foward vector of this PhysicsImpulseDirectional object.")
	ADD_STRINGPROP_FLAG(Object2, "", PF_OBJECTLINK, "This is the name of an object that will receive ImpulseForce amount of force applied in the direction specified by the foward vector of this PhysicsImpulseDirectional object.")
	ADD_STRINGPROP_FLAG(Object3, "", PF_OBJECTLINK, "This is the name of an object that will receive ImpulseForce amount of force applied in the direction specified by the foward vector of this PhysicsImpulseDirectional object.")
	ADD_STRINGPROP_FLAG(Object4, "", PF_OBJECTLINK, "This is the name of an object that will receive ImpulseForce amount of force applied in the direction specified by the foward vector of this PhysicsImpulseDirectional object.")
	ADD_STRINGPROP_FLAG(Object5, "", PF_OBJECTLINK, "This is the name of an object that will receive ImpulseForce amount of force applied in the direction specified by the foward vector of this PhysicsImpulseDirectional object.")
	ADD_STRINGPROP_FLAG(Object6, "", PF_OBJECTLINK, "This is the name of an object that will receive ImpulseForce amount of force applied in the direction specified by the foward vector of this PhysicsImpulseDirectional object.")
	ADD_STRINGPROP_FLAG(Object7, "", PF_OBJECTLINK, "This is the name of an object that will receive ImpulseForce amount of force applied in the direction specified by the foward vector of this PhysicsImpulseDirectional object.")
	ADD_STRINGPROP_FLAG(Object8, "", PF_OBJECTLINK, "This is the name of an object that will receive ImpulseForce amount of force applied in the direction specified by the foward vector of this PhysicsImpulseDirectional object.")
	ADD_STRINGPROP_FLAG(Object9, "", PF_OBJECTLINK, "This is the name of an object that will receive ImpulseForce amount of force applied in the direction specified by the foward vector of this PhysicsImpulseDirectional object.")
	ADD_STRINGPROP_FLAG(Object10, "", PF_OBJECTLINK, "This is the name of an object that will receive ImpulseForce amount of force applied in the direction specified by the foward vector of this PhysicsImpulseDirectional object.")
	ADD_STRINGPROP_FLAG(Object11, "", PF_OBJECTLINK, "This is the name of an object that will receive ImpulseForce amount of force applied in the direction specified by the foward vector of this PhysicsImpulseDirectional object.")
	ADD_STRINGPROP_FLAG(Object12, "", PF_OBJECTLINK, "This is the name of an object that will receive ImpulseForce amount of force applied in the direction specified by the foward vector of this PhysicsImpulseDirectional object.")
	ADD_STRINGPROP_FLAG(Object13, "", PF_OBJECTLINK, "This is the name of an object that will receive ImpulseForce amount of force applied in the direction specified by the foward vector of this PhysicsImpulseDirectional object.")
	ADD_STRINGPROP_FLAG(Object14, "", PF_OBJECTLINK, "This is the name of an object that will receive ImpulseForce amount of force applied in the direction specified by the foward vector of this PhysicsImpulseDirectional object.")
	ADD_STRINGPROP_FLAG(Object15, "", PF_OBJECTLINK, "This is the name of an object that will receive ImpulseForce amount of force applied in the direction specified by the foward vector of this PhysicsImpulseDirectional object.")
	ADD_STRINGPROP_FLAG(Object16, "", PF_OBJECTLINK, "This is the name of an object that will receive ImpulseForce amount of force applied in the direction specified by the foward vector of this PhysicsImpulseDirectional object.")
	ADD_STRINGPROP_FLAG(Object17, "", PF_OBJECTLINK, "This is the name of an object that will receive ImpulseForce amount of force applied in the direction specified by the foward vector of this PhysicsImpulseDirectional object.")
	ADD_STRINGPROP_FLAG(Object18, "", PF_OBJECTLINK, "This is the name of an object that will receive ImpulseForce amount of force applied in the direction specified by the foward vector of this PhysicsImpulseDirectional object.")
	ADD_STRINGPROP_FLAG(Object19, "", PF_OBJECTLINK, "This is the name of an object that will receive ImpulseForce amount of force applied in the direction specified by the foward vector of this PhysicsImpulseDirectional object.")
	ADD_STRINGPROP_FLAG(Object20, "", PF_OBJECTLINK, "This is the name of an object that will receive ImpulseForce amount of force applied in the direction specified by the foward vector of this PhysicsImpulseDirectional object.")
	ADD_STRINGPROP_FLAG(Object21, "", PF_OBJECTLINK, "This is the name of an object that will receive ImpulseForce amount of force applied in the direction specified by the foward vector of this PhysicsImpulseDirectional object.")
	ADD_STRINGPROP_FLAG(Object22, "", PF_OBJECTLINK, "This is the name of an object that will receive ImpulseForce amount of force applied in the direction specified by the foward vector of this PhysicsImpulseDirectional object.")
	ADD_STRINGPROP_FLAG(Object23, "", PF_OBJECTLINK, "This is the name of an object that will receive ImpulseForce amount of force applied in the direction specified by the foward vector of this PhysicsImpulseDirectional object.")
	ADD_STRINGPROP_FLAG(Object24, "", PF_OBJECTLINK, "This is the name of an object that will receive ImpulseForce amount of force applied in the direction specified by the foward vector of this PhysicsImpulseDirectional object.")
	ADD_STRINGPROP_FLAG(Object25, "", PF_OBJECTLINK, "This is the name of an object that will receive ImpulseForce amount of force applied in the direction specified by the foward vector of this PhysicsImpulseDirectional object.")
	ADD_STRINGPROP_FLAG(Object26, "", PF_OBJECTLINK, "This is the name of an object that will receive ImpulseForce amount of force applied in the direction specified by the foward vector of this PhysicsImpulseDirectional object.")
	ADD_STRINGPROP_FLAG(Object27, "", PF_OBJECTLINK, "This is the name of an object that will receive ImpulseForce amount of force applied in the direction specified by the foward vector of this PhysicsImpulseDirectional object.")
	ADD_STRINGPROP_FLAG(Object28, "", PF_OBJECTLINK, "This is the name of an object that will receive ImpulseForce amount of force applied in the direction specified by the foward vector of this PhysicsImpulseDirectional object.")
	ADD_STRINGPROP_FLAG(Object29, "", PF_OBJECTLINK, "This is the name of an object that will receive ImpulseForce amount of force applied in the direction specified by the foward vector of this PhysicsImpulseDirectional object.")
	ADD_STRINGPROP_FLAG(Object30, "", PF_OBJECTLINK, "This is the name of an object that will receive ImpulseForce amount of force applied in the direction specified by the foward vector of this PhysicsImpulseDirectional object.")
	ADD_STRINGPROP_FLAG(Object31, "", PF_OBJECTLINK, "This is the name of an object that will receive ImpulseForce amount of force applied in the direction specified by the foward vector of this PhysicsImpulseDirectional object.")
	ADD_STRINGPROP_FLAG(Object32, "", PF_OBJECTLINK, "This is the name of an object that will receive ImpulseForce amount of force applied in the direction specified by the foward vector of this PhysicsImpulseDirectional object.")
	ADD_STRINGPROP_FLAG(Object33, "", PF_OBJECTLINK, "This is the name of an object that will receive ImpulseForce amount of force applied in the direction specified by the foward vector of this PhysicsImpulseDirectional object.")
	ADD_STRINGPROP_FLAG(Object34, "", PF_OBJECTLINK, "This is the name of an object that will receive ImpulseForce amount of force applied in the direction specified by the foward vector of this PhysicsImpulseDirectional object.")
	ADD_STRINGPROP_FLAG(Object35, "", PF_OBJECTLINK, "This is the name of an object that will receive ImpulseForce amount of force applied in the direction specified by the foward vector of this PhysicsImpulseDirectional object.")
	ADD_STRINGPROP_FLAG(Object36, "", PF_OBJECTLINK, "This is the name of an object that will receive ImpulseForce amount of force applied in the direction specified by the foward vector of this PhysicsImpulseDirectional object.")
	ADD_STRINGPROP_FLAG(Object37, "", PF_OBJECTLINK, "This is the name of an object that will receive ImpulseForce amount of force applied in the direction specified by the foward vector of this PhysicsImpulseDirectional object.")
	ADD_STRINGPROP_FLAG(Object38, "", PF_OBJECTLINK, "This is the name of an object that will receive ImpulseForce amount of force applied in the direction specified by the foward vector of this PhysicsImpulseDirectional object.")
	ADD_STRINGPROP_FLAG(Object39, "", PF_OBJECTLINK, "This is the name of an object that will receive ImpulseForce amount of force applied in the direction specified by the foward vector of this PhysicsImpulseDirectional object.")
	ADD_STRINGPROP_FLAG(Object40, "", PF_OBJECTLINK, "This is the name of an object that will receive ImpulseForce amount of force applied in the direction specified by the foward vector of this PhysicsImpulseDirectional object.")
	ADD_STRINGPROP_FLAG(Object41, "", PF_OBJECTLINK, "This is the name of an object that will receive ImpulseForce amount of force applied in the direction specified by the foward vector of this PhysicsImpulseDirectional object.")
	ADD_STRINGPROP_FLAG(Object42, "", PF_OBJECTLINK, "This is the name of an object that will receive ImpulseForce amount of force applied in the direction specified by the foward vector of this PhysicsImpulseDirectional object.")
	ADD_STRINGPROP_FLAG(Object43, "", PF_OBJECTLINK, "This is the name of an object that will receive ImpulseForce amount of force applied in the direction specified by the foward vector of this PhysicsImpulseDirectional object.")
	ADD_STRINGPROP_FLAG(Object44, "", PF_OBJECTLINK, "This is the name of an object that will receive ImpulseForce amount of force applied in the direction specified by the foward vector of this PhysicsImpulseDirectional object.")
	ADD_STRINGPROP_FLAG(Object45, "", PF_OBJECTLINK, "This is the name of an object that will receive ImpulseForce amount of force applied in the direction specified by the foward vector of this PhysicsImpulseDirectional object.")
	ADD_STRINGPROP_FLAG(Object46, "", PF_OBJECTLINK, "This is the name of an object that will receive ImpulseForce amount of force applied in the direction specified by the foward vector of this PhysicsImpulseDirectional object.")
	ADD_STRINGPROP_FLAG(Object47, "", PF_OBJECTLINK, "This is the name of an object that will receive ImpulseForce amount of force applied in the direction specified by the foward vector of this PhysicsImpulseDirectional object.")
	ADD_STRINGPROP_FLAG(Object48, "", PF_OBJECTLINK, "This is the name of an object that will receive ImpulseForce amount of force applied in the direction specified by the foward vector of this PhysicsImpulseDirectional object.")
	ADD_STRINGPROP_FLAG(Object49, "", PF_OBJECTLINK, "This is the name of an object that will receive ImpulseForce amount of force applied in the direction specified by the foward vector of this PhysicsImpulseDirectional object.")
	ADD_STRINGPROP_FLAG(Object50, "", PF_OBJECTLINK, "This is the name of an object that will receive ImpulseForce amount of force applied in the direction specified by the foward vector of this PhysicsImpulseDirectional object.")

END_CLASS_FLAGS(PhysicsImpulseDirectional, GameBase, 0, "The PhysicsImpulseDirectional object should be used to apply a physical force the physically simulated objects specifed.  These objects will receive ImpulseForce amount of force applied in the direction specified by the foward vector of this PhysicsImpulseDirectional object.  The force value is measured in game unit newton seconds.")


CMDMGR_BEGIN_REGISTER_CLASS( PhysicsImpulseDirectional )

	ADD_MESSAGE( ON,	1,	NULL,	MSG_HANDLER( PhysicsImpulseDirectional, HandleOnMsg ),	"ON", "Trigger the object to apply the specified impulse force.", "msg PhysicsImpulseDirectional ON" )

CMDMGR_END_REGISTER_CLASS( PhysicsImpulseDirectional, GameBase )

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	PhysicsImpulseDirectional::PhysicsImpulseDirectional()
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

PhysicsImpulseDirectional::PhysicsImpulseDirectional() : GameBase(), 
	m_saObjectNames ( ),
	m_fImpulse ( PhysicsUtilities::DEFAULT_IMPULSE_FORCE ),
	m_bRemoveWhenDone ( true )
{
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	PhysicsImpulseDirectional::EngineMessageFn
//
//	PURPOSE:	Handle engine messages
//
// ----------------------------------------------------------------------- //

uint32 PhysicsImpulseDirectional::EngineMessageFn(uint32 messageID, void *pData, float fData)
{
	switch(messageID)
	{
		case MID_INITIALUPDATE:
		{
			SetNextUpdate(UPDATE_NEVER);
		}
		break;

		case MID_PRECREATE :
		{
			if (fData == PRECREATE_WORLDFILE || fData == PRECREATE_STRINGPROP)
			{
				ReadProp(&((ObjectCreateStruct*)pData)->m_cProperties);
			}
		}
		break;

		case MID_SAVEOBJECT:
		{
            Save((ILTMessage_Write*)pData, (uint32)fData);
		}
		break;

		case MID_LOADOBJECT:
		{
            Load((ILTMessage_Read*)pData, (uint32)fData);
		}
		break;

		default : break;
	}

	return GameBase::EngineMessageFn(messageID, pData, fData);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	PhysicsImpulseDirectional::HandleOnMsg
//
//	PURPOSE:	Handle ON message...
//
// ----------------------------------------------------------------------- //

void PhysicsImpulseDirectional::HandleOnMsg( HOBJECT /*hSender*/, const CParsedMsg& /*crParsedMsg*/ )
{
	ApplyForceToObjects();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	PhysicsImpulseDirectional::ReadProp
//
//	PURPOSE:	Read object properties
//
// ----------------------------------------------------------------------- //

void PhysicsImpulseDirectional::ReadProp(const GenericPropList *pProps)
{
	m_fImpulse			= pProps->GetReal( "ImpulseForce", m_fImpulse );
	m_bRemoveWhenDone	= pProps->GetBool( "RemoveWhenDone", m_bRemoveWhenDone );

	char szObject[128] = {0};
	m_saObjectNames.reserve( MAX_IMPULSE_TARGETS );

	for( uint32 nTarget = 0; nTarget < MAX_IMPULSE_TARGETS; ++nTarget )
	{
		LTSNPrintF( szObject, ARRAY_LEN(szObject), "Object%d", nTarget + 1 );

		const char *pszTargetName = pProps->GetString( szObject, "" );	
		if( pszTargetName && pszTargetName[0] )
		{
			m_saObjectNames.push_back( pszTargetName );
		}
	}

	// Shrink-to-fit...
	StringArray( m_saObjectNames ).swap( m_saObjectNames );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	PhysicsImpulseDirectional::ApplyForceToObjects()
//
//	PURPOSE:	Apply force to all of the specified objects
//
// ----------------------------------------------------------------------- //

void PhysicsImpulseDirectional::ApplyForceToObjects()
{
	if (!m_hObject || m_fImpulse < 0.0f) return;

	// Use forward vector of the object for the force direction...

	LTRotation rRot;
	g_pLTServer->GetObjectRotation(m_hObject, &rRot);
	LTVector vDir = rRot.Forward();
	vDir.Normalize();


	// Apply the force to the specified objects...

	ObjArray <HOBJECT, MAX_OBJECT_ARRAY_SIZE> objArray;
	uint32 nTotalFound = 0;

	StringArray::iterator iter;
	for( iter = m_saObjectNames.begin(); iter != m_saObjectNames.end(); ++iter )
	{
		if( !(*iter).empty() )
		{
			g_pLTServer->FindNamedObjects((*iter).c_str(), objArray, &nTotalFound);

			for (uint32 i=0; i < nTotalFound; i++)
			{
				PhysicsUtilities::ApplyPhysicsImpulseForce(objArray.GetObject(i), m_fImpulse, vDir, LTVector(0, 0, 0), true);
			}
		}
	}

	if (m_bRemoveWhenDone)
	{
		g_pLTServer->RemoveObject(m_hObject);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	PhysicsImpulseDirectional::Save
//
//	PURPOSE:	Save the object
//
// ----------------------------------------------------------------------- //

void PhysicsImpulseDirectional::Save(ILTMessage_Write *pMsg, uint32 /*dwSaveFlags*/)
{
	if (!pMsg) return;

	SAVE_FLOAT(m_fImpulse);
    SAVE_BOOL(m_bRemoveWhenDone);

	SAVE_DWORD( m_saObjectNames.size() );
	StringArray::iterator iter;
	for( iter = m_saObjectNames.begin(); iter != m_saObjectNames.end(); ++iter )
	{
		SAVE_STDSTRING( (*iter) );
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	PhysicsImpulseDirectional::Load
//
//	PURPOSE:	Load the object
//
// ----------------------------------------------------------------------- //

void PhysicsImpulseDirectional::Load(ILTMessage_Read *pMsg, uint32 /*dwLoadFlags*/)
{
	if (!pMsg) return;

	LOAD_FLOAT(m_fImpulse);
	LOAD_BOOL(m_bRemoveWhenDone);

	uint32 nSize;
	LOAD_DWORD( nSize );

	m_saObjectNames.clear();
	m_saObjectNames.resize( nSize );

	for( uint32 i = 0; i < nSize; ++i )
	{
		LOAD_STDSTRING( m_saObjectNames[i] );
	}
}

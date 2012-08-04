// ----------------------------------------------------------------------- //
//
// MODULE  : PhysicsCollisionSystem.cpp
//
// PURPOSE : PhysicsCollisionSystem - Implementation
//
// CREATED : 1/13/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "PhysicsCollisionSystem.h"
#include "iltphysicssim.h"

LINKFROM_MODULE( PhysicsCollisionSystem );

BEGIN_CLASS(PhysicsCollisionSystem)
	ADD_STRINGPROP_FLAG(Object1, "", PF_OBJECTLINK, "This is the name of an object that will be a part of the collision group and not collide with other objects of the same group.")
	ADD_STRINGPROP_FLAG(Object2, "", PF_OBJECTLINK, "This is the name of an object that will be a part of the collision group and not collide with other objects of the same group.")
	ADD_STRINGPROP_FLAG(Object3, "", PF_OBJECTLINK, "This is the name of an object that will be a part of the collision group and not collide with other objects of the same group.")
	ADD_STRINGPROP_FLAG(Object4, "", PF_OBJECTLINK, "This is the name of an object that will be a part of the collision group and not collide with other objects of the same group.")
	ADD_STRINGPROP_FLAG(Object5, "", PF_OBJECTLINK, "This is the name of an object that will be a part of the collision group and not collide with other objects of the same group.")
	ADD_STRINGPROP_FLAG(Object6, "", PF_OBJECTLINK, "This is the name of an object that will be a part of the collision group and not collide with other objects of the same group.")
	ADD_STRINGPROP_FLAG(Object7, "", PF_OBJECTLINK, "This is the name of an object that will be a part of the collision group and not collide with other objects of the same group.")
	ADD_STRINGPROP_FLAG(Object8, "", PF_OBJECTLINK, "This is the name of an object that will be a part of the collision group and not collide with other objects of the same group.")
	ADD_STRINGPROP_FLAG(Object9, "", PF_OBJECTLINK, "This is the name of an object that will be a part of the collision group and not collide with other objects of the same group.")
	ADD_STRINGPROP_FLAG(Object10, "", PF_OBJECTLINK, "This is the name of an object that will be a part of the collision group and not collide with other objects of the same group.")
	ADD_STRINGPROP_FLAG(Object11, "", PF_OBJECTLINK, "This is the name of an object that will be a part of the collision group and not collide with other objects of the same group.")
	ADD_STRINGPROP_FLAG(Object12, "", PF_OBJECTLINK, "This is the name of an object that will be a part of the collision group and not collide with other objects of the same group.")
	ADD_STRINGPROP_FLAG(Object13, "", PF_OBJECTLINK, "This is the name of an object that will be a part of the collision group and not collide with other objects of the same group.")
	ADD_STRINGPROP_FLAG(Object14, "", PF_OBJECTLINK, "This is the name of an object that will be a part of the collision group and not collide with other objects of the same group.")
	ADD_STRINGPROP_FLAG(Object15, "", PF_OBJECTLINK, "This is the name of an object that will be a part of the collision group and not collide with other objects of the same group.")
	ADD_STRINGPROP_FLAG(Object16, "", PF_OBJECTLINK, "This is the name of an object that will be a part of the collision group and not collide with other objects of the same group.")
	ADD_STRINGPROP_FLAG(Object17, "", PF_OBJECTLINK, "This is the name of an object that will be a part of the collision group and not collide with other objects of the same group.")
	ADD_STRINGPROP_FLAG(Object18, "", PF_OBJECTLINK, "This is the name of an object that will be a part of the collision group and not collide with other objects of the same group.")
	ADD_STRINGPROP_FLAG(Object19, "", PF_OBJECTLINK, "This is the name of an object that will be a part of the collision group and not collide with other objects of the same group.")
	ADD_STRINGPROP_FLAG(Object20, "", PF_OBJECTLINK, "This is the name of an object that will be a part of the collision group and not collide with other objects of the same group.")
	
	ADD_BOOLPROP( MPClientOnly, true, "In multiplayer games the collision systems need to be created on the client.  This flag is ignored in singleplayer games." )

END_CLASS_FLAGS(PhysicsCollisionSystem, GameBase, 0, "This class will group together all world models listed into a single collision system so that way they will not collide with each other.")


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	PhysicsCollisionSystem::PhysicsCollisionSystem
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

PhysicsCollisionSystem::PhysicsCollisionSystem() :
	GameBase		( OT_NORMAL ),
	m_saObjectNames	( ),
	m_bMPClientOnly	( true )
{

}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	PhysicsCollisionSystem::~PhysicsCollisionSystem
//
//	PURPOSE:	Destructor
//
// ----------------------------------------------------------------------- //

PhysicsCollisionSystem::~PhysicsCollisionSystem()
{

}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	PhysicsCollisionSystem::EngineMessageFn
//
//  PURPOSE:	Handle messages from the engine...
//
// ----------------------------------------------------------------------- //

uint32 PhysicsCollisionSystem::EngineMessageFn( uint32 messageID, void *pvData, float fData )
{
	switch( messageID )
	{
		case MID_PRECREATE:
		{
			if( fData == PRECREATE_WORLDFILE )
			{
				ObjectCreateStruct *pOCS = (ObjectCreateStruct*)pvData;
				ReadProp( &pOCS->m_cProperties );

				// Ensure the object will never be sent to the client.
				if( pOCS )
					pOCS->m_Flags |= FLAG_NOTINWORLDTREE;

				// The clients must know about the collision system object to create an associated
				// physics constraint in their simulation...
				if( IsMultiplayerGameServer( ) && m_bMPClientOnly )
					pOCS->m_Flags = FLAG_FORCECLIENTUPDATE;
			}
		}
		break;

		case MID_ALLOBJECTSCREATED:
		{
			InitialUpdate( );
		}
		break;
	};

	return GameBase::EngineMessageFn( messageID, pvData, fData );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	PhysicsCollisionSystem::PreCreate
//
//	PURPOSE:	Handle pre create
//
// ----------------------------------------------------------------------- //

bool PhysicsCollisionSystem::ReadProp(const GenericPropList *pProps)
{
	static const uint32 knNumObjectProperties = 20;
	m_saObjectNames.reserve( knNumObjectProperties );

	for( uint32 nTarget = 0; nTarget < knNumObjectProperties; ++nTarget )
	{
		char szObject[128];
		LTSNPrintF( szObject, LTARRAYSIZE(szObject), "Object%d", nTarget + 1 );
		
		const char *pszTargetName = pProps->GetString( szObject, "" );	
		if( !LTStrEmpty(pszTargetName) )
		{
			m_saObjectNames.push_back( pszTargetName );
		}
	}

	// Shrink-to-fit...
	StringArray( m_saObjectNames ).swap( m_saObjectNames );

	m_bMPClientOnly	= pProps->GetBool( "MPClientOnly", true );

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	PhysicsCollisionSystem::OnAllObjectsCreated
//
//	PURPOSE:	Handle installing the collision system on the listed objects
//
// ----------------------------------------------------------------------- //

void PhysicsCollisionSystem::InitialUpdate( )
{
	//determine what our collision system ID will be so we can
	//install that in the objects
	uint32 nNewSystem = 0;
	g_pLTServer->PhysicsSim()->GetUniqueSystemGroup(nNewSystem);

	ObjArray<HOBJECT, 20> aCollisionSystemObjects;

	//run through our listed objects and install it
	for(StringArray::const_iterator it = m_saObjectNames.begin(); it != m_saObjectNames.end(); ++it )
	{
		//get the object that this is referring to
		ObjArray<HOBJECT, 1> objArray;
		if(g_pLTServer->FindNamedObjects( it->c_str(), objArray ) != LT_OK)
			continue;

		//make sure that we found the object
		if(objArray.NumObjects() != 1)
			continue;

		//we can now get the world model rigid body from the object
		HPHYSICSRIGIDBODY hRigidBody;
		if(g_pLTServer->PhysicsSim()->GetWorldModelRigidBody(objArray.GetObject(0), hRigidBody) != LT_OK)
			continue;

		//we now want to set this physics system as the physics system for the rigid body
		g_pLTServer->PhysicsSim()->SetRigidBodyCollisionSystem(hRigidBody, nNewSystem);

		//and release our rigid body to avoid reference leaking
		g_pLTServer->PhysicsSim()->ReleaseRigidBody(hRigidBody);

		// Add to the collision system objects list so they may be sent to the clients...
		aCollisionSystemObjects.AddObject( objArray.GetObject(0) );
	}

	
	if( IsMultiplayerGameServer( ) && m_bMPClientOnly )
	{
		// Need to send the objects to the clients so they can create their own collision system...

		CAutoMessage cMsg;
		cMsg.Writeuint8( SFX_PHYSICS_COLLISION_SYSTEM_ID );

		uint32 nNumObjects = aCollisionSystemObjects.NumObjects( );
		cMsg.Writeuint32( nNumObjects );

		for( uint32 nObject = 0; nObject < nNumObjects; ++nObject )
		{
			cMsg.WriteObject( aCollisionSystemObjects.GetObject( nObject ));
		}

		g_pLTServer->SetObjectSFXMessage( m_hObject, cMsg.Read( ));

		// The object can't be removed but we don't want to ever update it...
		SetNextUpdate( UPDATE_NEVER );
	}
	else
	{
		//we are done with this object, so we now need to remove this object so it will no longer
		//consume memory/time
		g_pLTServer->RemoveObject( m_hObject );
	}
}


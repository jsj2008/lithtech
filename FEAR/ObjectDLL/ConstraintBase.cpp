// ----------------------------------------------------------------------- //
//
// MODULE  :	ConstraintBase.cpp
//
// PURPOSE :	Provides the base class for all of the constraint game object types. This
//				handles common operations and data shared by all of the constraints, and
//				also the logic for creating objects and saving and loading to ensure that
//				all constraints handle events in the same manner.
//
// CREATED :	11/18/03
//
// (c) 2003 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "ConstraintBase.h"
#include "ParsedMsg.h"

LINKFROM_MODULE( ConstraintBase );

//---------------------------------------------------------------------------------------------
// Command manager plugin support
//---------------------------------------------------------------------------------------------

CMDMGR_BEGIN_REGISTER_CLASS( ConstraintBase )

//					Message			Num Params	Validation FnPtr		Syntax
	ADD_MESSAGE( BROKEN,			2,		NULL,	MSG_HANDLER( ConstraintBase, HandleBrokenMsg ),		"Broken [0/1]", "TODO:CMDDESC", "TODO:CMDEXP" )

CMDMGR_END_REGISTER_CLASS( ConstraintBase, GameBase )

//---------------------------------------------------------------------------------------------
// ConstraintBase
//---------------------------------------------------------------------------------------------

BEGIN_CLASS(ConstraintBase)

	ADD_STRINGPROP_FLAG(Object1, "", PF_OBJECTLINK, "The name of the first object that this constraint will link. This should be left blank if it is intended to link to the world.")
	ADD_STRINGPROP(ModelNode1, "", "The name of the node within the first object that the constraint will link to. This should be left blank if the object is not a model.")

	ADD_STRINGPROP_FLAG(Object2, "", PF_OBJECTLINK, "The name of the second object that this constraint will link. This should be left blank if it is intended to link to the world.")
	ADD_STRINGPROP(ModelNode2, "", "The name of the node within the second object that the constraint will link to. This should be left blank if the object is not a model.")

	ADD_BOOLPROP(Breakable, false, "Determines if this constraint can be broken at runtime when a force that exceeds the threshold is applied")
	ADD_REALPROP(BreakForce, 100.0f, "Indicates the force that when exceeded, the constraint will be broken. This number is largely determined through experimentation, but higher numbers mean more force is required to break the constraint")
	ADD_BOOLPROP(StartBroken, false, "Determines if the constraint should start broken or not. Only applicable to breakable constraints")
	ADD_COMMANDPROP(BreakCmd, "", "Command that will be triggered when this breakable constraint is broken.")

	ADD_BOOLPROP( MPClientOnly, true, "In multiplayer games the constraints need to be created on the client.  This flag is ignored in singleplayer games." )

END_CLASS_FLAGS(ConstraintBase, GameBase, CF_HIDDEN, "Provides a base class for all the constraint types")


ConstraintBase::ConstraintBase() :
	GameBase(OT_NORMAL),
	m_hConstraint(INVALID_PHYSICS_CONSTRAINT),
	m_hBreakable(INVALID_PHYSICS_BREAKABLE),
	m_bFailedCreation(false),
	m_bWasLoaded(true),
	m_bBreakable(false),
	m_fBreakForce(100.0f),
	m_bIsBroken(false),
	m_bMPClientOnly(true)
{
}

ConstraintBase::~ConstraintBase()
{
	TermPhysicsObjects();
}

//virtual function that derived classes must override to handle loading in of
//property data
void ConstraintBase::ReadConstraintProperties(const GenericPropList *pProps)
{
	//read in the objects that this connects
	m_sObject1	= pProps->GetString("Object1", "");
	m_sNode1	= pProps->GetString("ModelNode1", "");

	m_sObject2	= pProps->GetString("Object2", "");
	m_sNode2	= pProps->GetString("ModelNode2", "");

	//read in the breakable properties
	m_bBreakable	= pProps->GetBool("Breakable", false);
	m_fBreakForce	= pProps->GetReal("BreakForce", 100.0f);
	m_bIsBroken		= pProps->GetBool("StartBroken", false);
	m_sBrokenCmd	= pProps->GetCommand("BreakCmd", "");

	m_bMPClientOnly	= pProps->GetBool( "MPClientOnly", true );
}

//handles events sent from the engine. These are primarily messages
//associated with saving and loading
uint32 ConstraintBase::EngineMessageFn(uint32 messageID, void *pData, float fData)
{
	switch(messageID)
	{
		case MID_PRECREATE:
		{
			if (fData == PRECREATE_WORLDFILE ||
				fData == PRECREATE_STRINGPROP ||
				fData == PRECREATE_NORMAL)
			{
				ObjectCreateStruct *pOCS = reinterpret_cast<ObjectCreateStruct*>(pData);

				//let the child object handle loading in the necessary properties
				ReadConstraintProperties(&pOCS->m_cProperties);
				m_bWasLoaded = false;

				// The clients must know about the constraint object to create an associated
				// physics constraint in their simulation...
				if( IsMultiplayerGameServer( ) && m_bMPClientOnly )
					pOCS->m_Flags = FLAG_FORCECLIENTUPDATE;
			}
			break;
		}

		case MID_INITIALUPDATE:
		{
			SetNextUpdate(UPDATE_NEVER);
			break;
		}

		case MID_LOADOBJECT:
		{
			Load((ILTMessage_Read*)pData, (uint32)fData);
			break;
		}

		case MID_SAVEOBJECT:
		{
			Save((ILTMessage_Write*)pData, (uint32)fData);			
			break;
		}

		case MID_ALLOBJECTSCREATED:
		{
			//only try and create our constraint if we didn't fail in the past
			if(!m_bFailedCreation)
			{
				if(!InternalCreateConstraint(!m_bWasLoaded))
					m_bFailedCreation = true;
			}
			break;
		}

		default : 
			break;
	}

	return GameBase::EngineMessageFn(messageID, pData, fData);
}

void ConstraintBase::Save(ILTMessage_Write *pMsg, uint32 dwSaveFlags)
{
	if (!pMsg) 
		return;

	ILTMessage_Write& cMsg = *pMsg;

	cMsg.WriteString(m_sObject1.c_str());
	cMsg.WriteString(m_sNode1.c_str());
	cMsg.WriteString(m_sObject2.c_str());
	cMsg.WriteString(m_sNode2.c_str());

	cMsg.Writebool(m_bFailedCreation);

	cMsg.Writebool(m_bBreakable);
	cMsg.Writefloat(m_fBreakForce);
	cMsg.Writebool(m_bIsBroken);
	cMsg.WriteString(m_sBrokenCmd.c_str());
}

void ConstraintBase::Load(ILTMessage_Read *pMsg, uint32 dwLoadFlags)
{
	char pszBuffer[512];

	if (!pMsg) 
		return;

	ILTMessage_Read& cMsg = *pMsg;

	cMsg.ReadString(pszBuffer, LTARRAYSIZE(pszBuffer));
	m_sObject1 = pszBuffer;
	cMsg.ReadString(pszBuffer, LTARRAYSIZE(pszBuffer));
	m_sNode1 = pszBuffer;

	cMsg.ReadString(pszBuffer, LTARRAYSIZE(pszBuffer));
	m_sObject2 = pszBuffer;
	cMsg.ReadString(pszBuffer, LTARRAYSIZE(pszBuffer));
	m_sNode2 = pszBuffer;

	m_bFailedCreation = cMsg.Readbool();

	m_bBreakable	= cMsg.Readbool();
	m_fBreakForce	= cMsg.Readfloat();
	m_bIsBroken		= cMsg.Readbool();

	cMsg.ReadString(pszBuffer, LTARRAYSIZE(pszBuffer));
	m_sBrokenCmd = pszBuffer;
}

//called to free the physics objects associated with this object
void ConstraintBase::TermPhysicsObjects()
{
	//see if we have a breakable, if so we need to clean that up first since the constraint
	//won't be in the environment
	if(m_hBreakable != INVALID_PHYSICS_BREAKABLE)
	{
		g_pLTServer->PhysicsSim()->RemoveBreakableFromSimulation(m_hBreakable);
		g_pLTServer->PhysicsSim()->ReleaseBreakable(m_hBreakable);
		g_pLTServer->PhysicsSim()->ReleaseConstraint(m_hConstraint);
	}
	else if(m_hConstraint != INVALID_PHYSICS_CONSTRAINT)
	{
		//we have a constraint, so just remove that from the simulation and release it
		g_pLTServer->PhysicsSim()->RemoveConstraintFromSimulation(m_hConstraint);
		g_pLTServer->PhysicsSim()->ReleaseConstraint(m_hConstraint);
	}

	//and clear out our data
	m_hBreakable = INVALID_PHYSICS_BREAKABLE;
	m_hConstraint = INVALID_PHYSICS_CONSTRAINT;
}

//given the textual name of an object and optionally the model node within the object, this will
//convert the strings to an appropriate rigid body, and return false if the operation cannot be done.
//The returned rigid body must have release called on it to be properly freed
bool ConstraintBase::ConvertToRigidBody(const char* pszObjectName, const char* pszNodeName, HPHYSICSRIGIDBODY& hRigidBody)
{
	//first off, default the handles to valid values
	hRigidBody = INVALID_PHYSICS_RIGID_BODY;
	
	//if no name is provided, then they just mean the world, so that is ok
	if(LTStrEmpty(pszObjectName))
		return true;
	
	//now find the object in the world
	ObjArray<HOBJECT, 1> objArray;
	g_pLTServer->FindNamedObjects( pszObjectName, objArray );

	//make sure an object was found
	if( objArray.NumObjects() == 0 )
		return false;
		
	HOBJECT hObject = objArray.GetObject(0);

	//now determine the type of the object, we can only handle models and world models
	uint32 nObjType = OT_NORMAL;
	g_pLTServer->Common()->GetObjectType(hObject, &nObjType);

	//now if it is a model, we have to find a model node
	if(nObjType == OT_MODEL)
	{

		HMODELNODE hModelNode;
		g_pLTServer->GetModelLT()->GetNode(hObject, pszNodeName, hModelNode);

		//there must be a valid node
		if(hModelNode == INVALID_MODEL_NODE)
			return false;

		//and that node must have a rigid body associated with it
		if(g_pLTServer->PhysicsSim()->GetModelRigidBody(hObject, hModelNode, hRigidBody) != LT_OK)
			return false;

		if(hRigidBody == INVALID_PHYSICS_RIGID_BODY)
			return false;
	}
	else if(nObjType == OT_WORLDMODEL)
	{
		//get the rigid body associated with the world model
		if(g_pLTServer->PhysicsSim()->GetWorldModelRigidBody(hObject, hRigidBody) != LT_OK)
			return false;

		if(hRigidBody == INVALID_PHYSICS_RIGID_BODY)
			return false;
	}
	else
	{
		//all other object types are not supported
		return false;
	}

	//we have a valid rigid body, success
	return true;
}

//called internally when the constraint needs to be created
bool ConstraintBase::InternalCreateConstraint(bool bSetupBodySpaceData)
{
	//make sure to clean up any existing constraints
	TermPhysicsObjects();

	//cache the physics simulation interface just to make the code a bit more readable
	ILTPhysicsSim* pILTPhysicsSim = g_pLTServer->PhysicsSim();

	//first off determine the two rigid bodies
	HPHYSICSRIGIDBODY hBody1, hBody2;

	if(!ConvertToRigidBody(m_sObject1.c_str(), m_sNode1.c_str(), hBody1))
		return false;

	if(!ConvertToRigidBody(m_sObject2.c_str(), m_sNode2.c_str(), hBody2))
	{
		pILTPhysicsSim->ReleaseRigidBody(hBody1);
		return false;
	}

	//see if we need to setup the rigid body space first
	if(bSetupBodySpaceData)
	{
		//get the transforms for each rigid body
		LTRigidTransform tBody1, tBody2;
		pILTPhysicsSim->GetRigidBodyTransform(hBody1, tBody1);
		pILTPhysicsSim->GetRigidBodyTransform(hBody2, tBody2);

		//invert each
		tBody1.Inverse();
		tBody2.Inverse();

		//and allow the object to initialize itself
		if(!SetupBodySpaceData(tBody1, tBody2))
			return false;
	}

	//now that we have the two rigid bodies, convert them over to an actual appropriate
	//constraint
	m_hConstraint = CreateConstraint(hBody1, hBody2);

	//free up our rigid body references
	pILTPhysicsSim->ReleaseRigidBody(hBody1);
	pILTPhysicsSim->ReleaseRigidBody(hBody2);

	//check to see if the constraint was properly created
	if(m_hConstraint == INVALID_PHYSICS_CONSTRAINT)
		return false;

	//it is valid, we either need to wrap it in a breakable object now, or add it to the world
	if(m_bBreakable)
	{
		//it is breakable, so create a breakable constraint that wraps it
		m_hBreakable = pILTPhysicsSim->CreateBreakable(m_hConstraint, m_fBreakForce);

		//see if it is valid
		if(m_hBreakable == INVALID_PHYSICS_BREAKABLE)
		{
			//it isn't
			pILTPhysicsSim->ReleaseConstraint(m_hConstraint);
			return false;
		}

		//setup the current state of our breakable constraint, and ensure all of the operations worked
		//correctly

		//make sure the breakable reflects our current state
		LTRESULT hrSetBroken = pILTPhysicsSim->SetBreakableBroken(m_hBreakable, m_bIsBroken);

		//register our callback for this breakable object
		LTRESULT hrSetCallback = pILTPhysicsSim->RegisterBreakableCallback(m_hBreakable, ConstraintBrokenCB, (void*)this);

		//and add it to the simulation
		LTRESULT hrAddToSim = pILTPhysicsSim->AddBreakableToSimulation(m_hBreakable);

		//now if any one of those operations failed, we need to cleanup and bail
		if((hrSetBroken != LT_OK) || (hrSetCallback != LT_OK) || (hrAddToSim != LT_OK))
		{
			pILTPhysicsSim->ReleaseConstraint(m_hConstraint);
			pILTPhysicsSim->ReleaseBreakable(m_hBreakable);
			return false;
		}
	}
	else
	{
		//not a breakable constraint, so just add it into the world
		if(pILTPhysicsSim->AddConstraintToSimulation(m_hConstraint) != LT_OK)
		{
			pILTPhysicsSim->ReleaseConstraint(m_hConstraint);
			return false;
		}
	}

	if( IsMultiplayerGameServer( ) && m_bMPClientOnly )
	{
		// Need to create the SpecialFX message on the first update to guarantee the the
		// dependent objects are available on the client...
		CreateClientSpecialFX( );
	}

	//everything was created successfully, return success
	return true;
}

//callback function that is triggered when the breakable constraint is broken. This will simply
//call into the OnConstraintBroken function of the ConstraintBase object provided through the
//user data
void ConstraintBase::ConstraintBrokenCB(HPHYSICSBREAKABLE hBreakable, void* pUserData)
{
	//we should always have valid user data!
	LTASSERT(pUserData, "Error: Invalid user data provided to the constraint broken callback");

	if(pUserData)
		((ConstraintBase*)pUserData)->OnConstraintBroken(hBreakable);
}

//called to actually handle the constraint being broken
void ConstraintBase::OnConstraintBroken(HPHYSICSBREAKABLE hBreakable)
{
	//just a quick sanity check
	LTASSERT(hBreakable == m_hBreakable, "Error: Invalid breakable provide into OnConstraintBroken");

	//update the broken status of the constraint
	m_bIsBroken = true;

	//and now trigger the command
	if( !m_sBrokenCmd.empty() )
	{
		g_pCmdMgr->QueueCommand( m_sBrokenCmd.c_str(), m_hObject, m_hObject );
	}
}

// Setup the SpecialFX message to create the client constraint...
void ConstraintBase::CreateClientSpecialFX( )
{
	// Find the rigidbodies to constrain.  If an object is not specified then the world is used...
	HOBJECT hObject1 = INVALID_HOBJECT;
	if( !m_sObject1.empty( ))
	{
		//now find the object in the world
		ObjArray<HOBJECT, 1> objArray;
		g_pLTServer->FindNamedObjects( m_sObject1.c_str( ), objArray );

		//make sure an object was found
		if( objArray.NumObjects() == 0 )
			return;

		hObject1 = objArray.GetObject(0);
	}

	HOBJECT hObject2 = INVALID_HOBJECT;
	if( !m_sObject2.empty( ))
	{
		//now find the object in the world
		ObjArray<HOBJECT, 1> objArray;
		g_pLTServer->FindNamedObjects( m_sObject2.c_str( ), objArray );

		//make sure an object was found
		if( objArray.NumObjects() == 0 )
			return;

		hObject2 = objArray.GetObject(0);
	}

	// Set SFX message for clients who will join in the future...
	
	CAutoMessage cMsg;
	cMsg.Writeuint8( SFX_PHYSICS_CONSTRAINT_ID );
	
	PHYSICSCONSTRAINTCREATESTRUCT cs;
	
	if( hObject1 )
	{
		cs.m_bHasObject1 = true;
		cs.m_hObject1 = hObject1;
	}
	
	if( hObject2 )
	{
		cs.m_bHasObject2 = true;
		cs.m_hObject2 = hObject2;
	}

	// Let each constraint type write data specific for their needs...
	SetupConstraintCreateStruct( cs );
	cs.Write( cMsg );
	g_pLTServer->SetObjectSFXMessage( m_hObject, cMsg.Read( ));
}

//---------------------------------------------------------------------------------------------
// ConstraintBaseBase message handlers
//---------------------------------------------------------------------------------------------

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	ConstraintBase::HandleBrokenMsg
//
//  PURPOSE:	Handle a BROKEN message...
//
// ----------------------------------------------------------------------- //

void ConstraintBase::HandleBrokenMsg( HOBJECT hSender, const CParsedMsg &crParsedMsg )
{
	if(crParsedMsg.GetArgCount() == 2)
	{
		//determine the state that they want
		bool bNewState = (atoi(crParsedMsg.GetArg(1)) != 0);

		//now determine if it is different from our current state
		if(bNewState == m_bIsBroken)
			return;

		//update our state
		m_bIsBroken = bNewState;

		//and if it was broken, trigger the broken command
		if(m_bIsBroken && !m_sBrokenCmd.empty())
		{
			g_pCmdMgr->QueueCommand( m_sBrokenCmd.c_str(), m_hObject, m_hObject );
		}

		//if we have a breakable object, make sure that the state reflects this message
		if(m_hBreakable != INVALID_PHYSICS_BREAKABLE)
		{
			g_pLTServer->PhysicsSim()->SetBreakableBroken(m_hBreakable, m_bIsBroken);
		}
	}
}


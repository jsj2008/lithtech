//---------------------------------------------------------------------------
// ConstraintBase.h
//
// Provides the base class for all of the constraint game object types. This
// handles common operations and data shared by all of the constraints, and
// also the logic for creating objects and saving and loading to ensure that
// all constraints handle events in the same manner.
//
//---------------------------------------------------------------------------
#ifndef __CONSTRAINTBASE_H__
#define __CONSTRAINTBASE_H__

#ifndef __GAMEBASE_H__
#	include "GameBase.h"
#endif

#ifndef __ILTPHYSICSSIM_H__
#	include "iltphysicssim.h"
#endif

LINKTO_MODULE( ConstraintBase );

class ConstraintBase : 
	public GameBase
{
public:

	ConstraintBase();
	virtual ~ConstraintBase();

protected:

	//virtual function that derived classes must override to handle loading in of
	//property data. Derived classes should call this for the parent first.
	virtual void	ReadConstraintProperties(const GenericPropList *pProps);

	//this function is called only once when the object is created (NOT when it is loaded), and provides
	//the rigid bodies that the constraint will be linked between. This allows for rigid bodies to handle
	//setting up any data that must be specified in the space of the rigid body. This data should then
	//be saved and loaded and used. This is guaranteed to be called before CreateConstraint, and after
	//all other objects have been created. This should return false if it cannot setup the data
	//and therefore should not create the constraint.
	virtual bool	SetupBodySpaceData(const LTRigidTransform& tInvBody1, const LTRigidTransform& tInvBody2)
	{
		return false;
	}

	//called to create the actual constraint object from the properties that have been read in
	//or loaded
	virtual HPHYSICSCONSTRAINT	CreateConstraint(HPHYSICSRIGIDBODY hBody1, HPHYSICSRIGIDBODY hBody2) 
	{ 
		return INVALID_PHYSICS_CONSTRAINT; 
	}

	//handles events sent from the engine. 
	virtual uint32	EngineMessageFn(uint32 messageID, void *pData, float fData);

	//handles saving and loading all data to a message stream. Derived classes should
	//call this first in order to work properly
	virtual void	Save(ILTMessage_Write *pMsg, uint32 dwSaveFlags);
	virtual void	Load(ILTMessage_Read *pMsg, uint32 dwLoadFlags);

	// Let the derived constraint objects write their data to the message...
	virtual void SetupConstraintCreateStruct( PHYSICSCONSTRAINTCREATESTRUCT &rCS ) { };

	//handles parsing of various messages
	bool		HandleBrokenMsg(const CParsedMsg& cMsg);

	// Message Handlers...
	DECLARE_MSG_HANDLER( ConstraintBase, HandleBrokenMsg );

private:

	//callback function that is triggered when the breakable constraint is broken. This will simply
	//call into the OnConstraintBroken function of the ConstraintBase object provided through the
	//user data
	static void	ConstraintBrokenCB(HPHYSICSBREAKABLE hBreakable, void* pUserData);

	//called to actually handle the constraint being broken
	void		OnConstraintBroken(HPHYSICSBREAKABLE hBreakable);

	//called to free the physics objects associated with this object
	void		TermPhysicsObjects();

	//given the textual name of an object and optionally the model node within the object, this will
	//convert the strings to an appropriate rigid body, and return false if the operation cannot be done.
	//The returned rigid body must have release called on it to be properly freed
	bool		ConvertToRigidBody(const char* pszObjectName, const char* pszNodeName, HPHYSICSRIGIDBODY& hRigidBody);

	//called internally when the constraint needs to be created. If true is passed in, it will make sure
	//to call SetupBodySpaceData prior to creating the constraint
	bool		InternalCreateConstraint(bool bSetupBodySpaceData);

	// Setup the SpecialFX message to create the client constraint...
	void		CreateClientSpecialFX( );

	//determines if this was created from a save file
	bool					m_bWasLoaded;

	//Holds data about the first object that this constraint applies to. If the object name is empty
	//this refers to the main world rigid body. If it is a model, the node string contains the name
	//of the node
	std::string				m_sObject1;
	std::string				m_sNode1;

	//the same as the above, but instead is for the second object that the constraint applies to
	std::string				m_sObject2;
	std::string				m_sNode2;

	//the handle to the actual constraint object
	HPHYSICSCONSTRAINT		m_hConstraint;

	//the handle to the breakable constraint object. This can be invalid if the constraint
	//isn't breakable
	HPHYSICSBREAKABLE		m_hBreakable;

	//flag indicating whether or not the object failed creation. If it did, we don't want to ever
	//try to recreate it when we load again
	bool					m_bFailedCreation;

	//breakable properties

	//flag indicating whether or not this constraint is even breakable
	bool					m_bBreakable;

	//the amount of force it takes to break this constraint. No real units, just an
	//approximate value, larger values are harder to break
	float					m_fBreakForce;

	//the current state: is this constraint broken or not?
	bool					m_bIsBroken;

	//the command to send when the constraint breaks
	std::string				m_sBrokenCmd;

	// In multiplayer games the constraints need to be created on the client...
	bool					m_bMPClientOnly;

};

#endif

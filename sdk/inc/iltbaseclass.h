/*!

 MODULE  : ltbaseclass.h.

 PURPOSE : C++ LithTech game base class interface

 CREATED : 9/17/97

*/

#ifndef __ILTBASECLASS_H_
#define __ILTBASECLASS_H_

#ifndef __LTBASEDEFS_H__
#include "ltbasedefs.h"
#endif

#ifndef __ILTMESSAGE_H__
#include "iltmessage.h"
#endif

struct ContainerPhysics;
struct ObjectCreateStruct;

/*!
Defines an interface for notifications from the engine to simulation or
game objects.  \b LTGameObject, \b BaseClass, and \b BaseClassClient all 
derive from this class.

This interface will replace \b EngineMessageFn; for each \b MID_* message
that the engine used to send to \b EngineMessageFn, there is an equivalent
pure virtual function in \b ILTBaseClass that the engine now calls instead.

The default implementation of each pure virtual function inherited from 
\b ILTBaseClass in \b LTGameObject, \b BaseClass, and \b BaseClassClient 
is to call \b EngineMessageFn with the equivalent \b MID_* message.  Note 
that this makes \b EngineMessageFn a purely game-side function that 
the engine does not directly call.

See the Object chapter of the LithTech Programming Guide for more information.
*/

class ILTBaseClass
{
public:
	virtual ~ILTBaseClass();

/*!
\param messageID One of the \b MID_* define values.
\param pData Optional. A void pointer to data appropriate to the message type.
\param lData Optional. A float value appropriate to the message type.
\return Returns 1.

Receives event notifications from the engine.  Note that the engine actually calls
one of the \b "On*" notification functions, where the default implementation is to call
\b EngineMessageFn() with the appropriate \b MID_*.

The meaning of \b pData and \b lData vary by message type. See the Object chapter of the
LithTech Programming Guide for more information.

Before returning, this function should call the \b EngineMessageFn() of the object's parent
class to allow for inherited behavior and to allow aggregates to process the message.

\b EngineMessageFn will be deprecated in a future release in favor of the virtual function
interface inherited from \b ILTBaseClass.

Used For: Obsolete.
*/
    virtual uint32 EngineMessageFn(uint32 messageID, void *pData, float lData);

/*!
\param hSender A handle the object that sent the message (might be NULL).
\param messageID The user defined message-type identifier for this message.
\param hRead Pointer to the message class containg the data of this message.
\return Returns 1.

All classes deriving from BaseClass must define this function to receive and handle messages
from other engine objects. \b hRead is a message object whose contents are determined by the
message type.

If the hSender is NULL, this usually indicates a message from the server, since there is no
other method of sending a message from the server to an object.

Before returning, this function should call the \b ObjectMessageFn() of the object's parent
class to allow for inherited behavior and to allow aggregates to process the message.

Used For: Object.
*/
    virtual uint32 ObjectMessageFn(HOBJECT hSender, ILTMessage_Read *pMsg);

/*!
\return One of the \b OT_* defines.

Returns the type (one of the OT_* defines) of the object.

Used For: Object.
*/
    uint8 GetType() const { return m_nType; }

/*!
\param type One of the \b OT_* defines.

Sets the type (one of the OT_* defines) of the object.

Used For: Object.
*/
    void SetType(uint8 type) { m_nType = type; }

/*!
\param pOCS Pointer to the \b ObjectCreateStruct that will be used to create the object.
\param precreateType a \b PRECREATE_* define value.
\return Returns 1 to indicate that the object should be created, or 0 to indicate that it should not.

Called by the engine to notify the game object instance that its associated engine object is about to be created.

Called by the engine after the instance of the game object class derived from \b BaseClass or \b BaseClassClient
is instantiated, but before its associated engine object (\b LTObject) is created.

The \b OnPrecreate notification is commonly used to:
 
1) Load properties from a .LTB (formerly .DAT) file into
   the instance of the class derived from \b BaseClass,

2) Manipulate the \b ObjectCreateStruct that will be used
   to instantiate the object.

Before returning, this function should call the \b OnPrecreate() of the object's parent
class to allow for inherited behavior and to allow aggregates to process the message.

Replaces \b EngineMessageFn and \b MID_PRECREATE.

Used For: Object.
*/
	virtual uint32 OnPrecreate(ObjectCreateStruct* pOCS, float precreateType );

/*!
\param createType an \b OBJECTCREATED_* define value.
\return Returns 1 (ignored).

Called by the engine to notify the game object instance that its associated engine object has just been instantiated.

\b OnObjectCreated is called after:

1) The engine instantiates an engine object, and

3) The engine sets the \b m_hObject of the
   instance of the game object class derived from \b BaseClass
   to point to the engine object.

The \b OnObjectCreated notification is frequently used to manipulate the \b LTObject
that is associated with the instance of the game object class derived from \b BaseClass.

Before returning, this function should call the \b OnObjectCreated() of the object's parent
class to allow for inherited behavior and to allow aggregates to process the message.

Replaces \b EngineMessageFn and \b MID_OBJECTCREATED.

Used For: Object.
*/
	virtual uint32 OnObjectCreated				( float createType );

/*!
\return Returns 1 (ignored).

Called by the engine to give the game object instance a chance to update itself.

\b OnUpdate is called after the update time set in through \b ILTServer::SetNextUpdate() for this object elapses.

The update time must be set in each MID_UPDATE notification, otherwise no more
updates will be received.  Call SetNextUpdate to set the NextUpdate.

Game code can use this notification to frequently update game objects.

Before returning, this function should call the \b OnUpdate() of the object's parent
class to allow for inherited behavior and to allow aggregates to process the message.

Replaces \b EngineMessageFn and \b MID_UPDATE.

\see SetNextUpdate()
Used For: Object.
*/
	virtual uint32 OnUpdate						();

	virtual uint32 OnTouch						( HOBJECT object, 
														float force );
/*!
\param linkObj the \b HOBJECT of the linked object
\return Returns 1 (ignored).

Called by the engine to notify the object that one of its links to another object is about to be broken.

Before returning, this function should call the \b OnLinkBroken() of the object's parent
class to allow for inherited behavior and to allow aggregates to process the message.

Replaces \b EngineMessageFn and \b MID_LINKBROKEN.

\see LTContactInfo
Used For: Object.
*/
	virtual uint32 OnLinkBroken					( HOBJECT linkObj );

	/*!
\param crusherObj the \b HOBJECT of the object crushing you.
\return Returns 1 (ignored).

Called by the engine when another object is pushing this object into a wall.  It won't
move any further unless you make yourself nonsolid (ie: a player would
take damage from each crush notification, then die).

\note Not applicable to LT 3.0 physics!

Before returning, this function should call the \b OnCrush() of the object's parent
class to allow for inherited behavior and to allow aggregates to process the message.

Replaces \b EngineMessageFn and \b MID_CRUSH.

Used For: Obsolete.
*/
	virtual uint32 OnCrush						( HOBJECT crusherObj );

	/*!
\param readMsg the \b HMESSAGEREAD containing the saved game state.
\param dwParam user parameters (passed to ILTServer::RestoreObjects)
\return Returns 1 (ignored).

Called by the engine when the object should load in its serialized state.

Before returning, this function should call the \b OnLoad() of the object's parent
class to allow for inherited behavior and to allow aggregates to process the message.

Replaces \b EngineMessageFn and \b MID_LOADOBJECT.

\see ILTServer::RestoreObjects
Used For: Object.
*/
	virtual uint32 OnLoad						( ILTMessage_Read *pMsg,	 
													  float dwParam );

													  /*!
\param writeMsg the \b HMESSAGEWRITE to contain the saved game state.
\param dwParam user parameters (passed to ILTServer::SaveObjects)
\return Returns 1 (ignored).

Called by the engine when the object should serialize out its current state.

Before returning, this function should call the \b OnSave() of the object's parent
class to allow for inherited behavior and to allow aggregates to process the message.

Replaces \b EngineMessageFn and \b MID_SAVEOBJECT.

\see ILTServer::SaveObjects
Used For: Object.
*/
	virtual uint32 OnSave						( ILTMessage_Write *pMsg,
													  float dwParam );

/*!
\param pCP the \b ContainerPhysics containing physics information
\return Returns 1 (ignored).

Called by the engine each frame that the object is inside a container.

This gives you a chance to modify the physics applied to an object \em without actually modifying its
velocity or acceleration (a great way to dampen velocity).

\note Not applicable to LT 3.0 physics!

Before returning, this function should call the \b OnAffectPhysics() of the object's parent
class to allow for inherited behavior and to allow aggregates to process the message.

Replaces \b EngineMessageFn and \b MID_AFFECTPHYSICS.

Used For: Obsolete.
*/
	virtual uint32 OnAffectPhysics				( ContainerPhysics* pCP );

/*!
\return Returns 1 (ignored).

Called by the engine for an attachment object whose parent is being removed.

Before returning, this function should call the \b OnParentAttachmentRemoved() of the object's parent
class to allow for inherited behavior and to allow aggregates to process the message.

Replaces \b EngineMessageFn and \b MID_PARENTATTACHMENTREMOVED.

Used For: Object.
*/
	virtual uint32 OnParentAttachmentRemoved	();

/*!
\return Returns 1 (ignored).

Called when an object is activated.

Before returning, this function should call the \b OnActivate() of the object's parent
class to allow for inherited behavior and to allow aggregates to process the message.

Replaces \b EngineMessageFn and \b MID_ACTIVATING.

Used For: Object.
*/	
	virtual uint32 OnActivate					();

/*!
\return Returns 1 (ignored).

Called when an object is deactivated.

Before returning, this function should call the \b OnDeactivate() of the object's parent
class to allow for inherited behavior and to allow aggregates to process the message.

Replaces \b EngineMessageFn and \b MID_DEACTIVATING.

Used For: Object.
*/
	virtual uint32 OnDeactivate					();

	
/*!
\return Returns 1 (ignored).

Called for every object after all objects are loaded from the world file. It will arrive after \b OnObjectCreated, but before
the first \b OnUpdate.

This is a handy place to set up any inter-object references, as all objects are guaranteed to have been created.

Before returning, this function should call the \b OnAllObjectsCreated() of the object's parent
class to allow for inherited behavior and to allow aggregates to process the message.

Replaces \b EngineMessageFn and \b MID_ALLOBJECTSCREATED.

Used For: Object.
*/	
	virtual uint32 OnAllObjectsCreated			();
	
	//
	// Accessors
	//
    HOBJECT GetHOBJECT() const { return m_hObject; }

	//
	// Modifiers
	//
    void SetHOBJECT(HOBJECT h) { m_hObject = h; }

/*!
\param pAggregate A pointer to the \b IAggregate instance to associate with this \b BaseClass.

Associates an \b IAggregate-derived class with this BaseClass.

Associated aggregates receive all event notifications received by the BaseClass.
This is useful for adding generic behavior (such as damage) without the need to derive a new class from BaseClass.

\see RemoveAggregate()

Used For: Object.
*/
    void AddAggregate(LPAGGREGATE pAggregate);

/*!
\param pAggregate A pointer to the \b IAggregate instance to disassociate from this \b BaseClass.

Disassociates an \b IAggregate-derived class from this \b BaseClass.

After an aggregate is disassociated, it will no longer receive forwarded event notifications from the \b BaseClass.

\see AddAggregate()

Used For: Object.
*/
    bool RemoveAggregate(LPAGGREGATE pAggregate);

protected:
    ILTBaseClass(uint8 nType = OT_NORMAL) 
       : m_pFirstAggregate(0),
		 m_hObject(INVALID_HOBJECT),
         m_nType(nType) {}

// Data members
public:

/*!
It is \em very important that these data members stay in this order.
This version and the C version must be the same!
*/

/*!
A pointer to the object's first associated \b IAggregate. Each \b IAggregate includes
a pointer to the next \b IAggregate also associated with the object.

\see AddAggregate()
*/
    LPAGGREGATE m_pFirstAggregate;

/*!
A handle to this object. This variable is set by the engine upon instantiation of the associated engine object
(following MID_PRECREATE).
You can use this to pass in an
\b HOBJECT to the functions that require one instead of calling
ObjectToHandle() every time.
*/
    HOBJECT m_hObject;

/*!
The type (one of the \b OT_* defines) of the object.

\see SetType()
\see GetType()

C++ only data.
*/
    uint8           m_nType;
};



//! Object is being created at runtime.
#define PRECREATE_NORMAL        0.0f

//! Object is being loaded from a world file.  Read props in.
#define PRECREATE_WORLDFILE     1.0f

//! Object is created from CreateObjectProps.  Use GetPropGeneric to read props.
#define PRECREATE_STRINGPROP    2.0f

//! Object comes from a savegame.
#define PRECREATE_SAVEGAME      3.0f

//! Object being created on client side (client-only object).
#define PRECREATE_CLIENTOBJ		4.0f

//! Deprecated alias for \b OBJECTCREATED_NORMAL
#define INITIALUPDATE_NORMAL        0.0f

//! Deprecated alias for \b OBJECTCREATED_WORLDFILE
#define INITIALUPDATE_WORLDFILE     1.0f

//! Deprecated alias for \b OBJECTCREATED_STRINGPROP
#define INITIALUPDATE_STRINGPROP    2.0f

//! Deprecated alias for \b OBJECTCREATED_SAVEGAME
#define INITIALUPDATE_SAVEGAME      3.0f

//! Normal creation.
#define OBJECTCREATED_NORMAL        0.0f

//! Being created from a world file.
#define OBJECTCREATED_WORLDFILE     1.0f

//! Object is created from CreateObjectProps.  Use GetPropGeneric to read props.
#define OBJECTCREATED_STRINGPROP    2.0f

//! Created from a savegame.
#define OBJECTCREATED_SAVEGAME      3.0f

//! Object created on client side (client-only object).
#define OBJECTCREATED_CLIENTOBJ		4.0f

/*!
Here are all the message IDs and structures that LithTech uses for object notifications.
*/

/*!
These are the engine message identifications ("MID")
and structures that LithTech sends to an object's \b EngineMessageFn.

\b EngineMessageFn and these messages will be deprecated in a future release of LithTech,
in favor of the notification interface provided by \b ILTBaseClass.
*/
enum
{
    
/*!
A message identifier sent from the engine to the \b EngineMessageFn of a \b BaseClass to notify the instance that an engine object is about to be created.

The \b MID_PRECREATE value is passed to the messageID parameter of the \b BaseClass::EngineMessageFn after:

1) The instance of the game object class derived from \b BaseClass
   is instantiated.

The \b MID_PRECREATE notification is commonly used to:
 
1) Load properties from a .LTB (formerly .DAT) file into
   the instance of the class derived from \b BaseClass,

2) Manipulate the \b ObjectCreateStruct that will be used
   to instantiate the object.

In a \b MID_PRECEREATE notification, the \b EngineMessageFn's pData parameter is a pointer to an \b ObjectCreateStruct that will be
used to instantiate the engine object. The \b EngineMessageFn's lData parameter is a \b PRECREATE_* define value.
*/
    MID_PRECREATE =       0,

/*!
A message identifier sent from the engine to the \b EngineMessageFn of a \b BaseClass to notify the instance that an engine object has just been instantiated.

The \b MID_OBJECTCREATED is passed to the \b messageID parameter of the \b EngineMessageFn after:

1) The engine instantiates an engine object, and

3) The engine sets the \b m_hObject of the
   instance of the game object class derived from \b BaseClass
   to point to the engine object.

The \b MID_OBJECTCREATED notification is frequently used to manipulate the \b LTObject
that is associated with the instance of the game object class derived from \b BaseClass.

In a \b MID_OBJECTCREATED notification, the \b pData parameter of the
\b EngineMessageFn is not used. The \b lData parameter is an \b MID_OBJECTCREATED define value.
*/
    MID_OBJECTCREATED =   1,

/*!
Deprecated alias for \b MID_OBJECTCREATED
*/
    MID_INITIALUPDATE =   1,
    
/*!
A message identifier sent from the engine to the \b EngineMessageFn of a 
\b BaseClass to update the game object instance.

The \b MID_UPDATE is passed to the \b messageID parameter of the 
\b EngineMessageFn after:

1) The update time set in the \b ILTServer::SetNextUpdate() for this object elapses.

The update time must be set in each MID_UPDATE notification, otherwise no more
updates will be received.

Game code can use this notification to frequently update game objects.

Call SetNextUpdate to set the NextUpdate.

In a \b MID_UPDATE notification, neither the \b pData nor \b lData parameters of the
\b EngineMessageFn are used.
*/
    MID_UPDATE =          2,

/*!
This message is received by an object's EngineMessageFn() when a collision
occurs between it and another object or static world geometry.  The
\b pData argument points to the \b LTContactInfo for the collision.
*/
    MID_TOUCHNOTIFY =     3,

/*!
This is notification when a link to an object is about to be broken.
\b pData is an \b HOBJECT to the link's object.
*/
    MID_LINKBROKEN =      4,

/*!
This is notification when a model string key is crossed.
(You only get it if your \b FLAG_MODELKEYS flag is set
and an animation with Stringkeys set on certain keyframes is played).
\b pData is an \b ArgList* containing the Stringkey parameters.
*/
    MID_MODELSTRINGKEY =  5,

/*!
Called when an object pushes you into a wall.  It won't
move any further unless you make yourself nonsolid (ie: a player would
take damage from each crush notification, then die).
pData is the \b HOBJECT of the object crushing you.

Not applicable to LT 3.0 physics (if _LT_PHYS_3_ is defined).

*/
    MID_CRUSH =           6,

/*!
Load and save yourself for a serialization.
\b pData is an \b HMESSAGEREAD or \b HMESSAGEWRITE.
\b fData is the \b dwParam passed to ServerDE::SaveObjects or ServerDE::RestoreObjects.
*/
    MID_LOADOBJECT =      7,
    MID_SAVEOBJECT =      8,

/*!
Called for a container for objects inside it each frame.  This gives you a chance
to modify the physics applied to an object \em without actually modifying its
velocity or acceleration (a great way to dampen velocity).
\b pData is a \b ContainerPhysics*.

Not applicable to LT 3.0 physics (if _LT_PHYS_3_ is defined).

*/
    MID_AFFECTPHYSICS =   9,

/*!
The parent of an attachment between you and it is being removed.
*/
    MID_PARENTATTACHMENTREMOVED = 10,

/*!
Sent when an object is becoming active
*/
    MID_ACTIVATING =      12,

/*!
Sent when an object is becoming inactive
*/
    MID_DEACTIVATING =    13,

/*!
Sent to each object after all objects are loaded. It will arrive after \b MID_OBJECTCREATED, but before
the first \b MID_UPDATE.

In a \b MID_ALLOBJECTSCREATED notification, neither the \b pData nor \b lData parameters of the
\b EngineMessageFn are used.
*/
    MID_ALLOBJECTSCREATED = 14,

/*!
Sent when the engine is providing the movement encoding transform hint for a node
*/
  	MID_TRANSFORMHINT = 15,
};




#endif //__ILTBASECLASS_H_


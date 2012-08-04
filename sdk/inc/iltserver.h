/*!
server_de defines the ILTServer interface, which consists of all the
server-side DirectEngine functionality.
*/

#ifndef __ILTSERVER_H__
#define __ILTSERVER_H__

#ifndef __LTBASEDEFS_H__
#include "ltbasedefs.h"
#endif

#ifndef __LTSERVEROBJ_H__
#include "ltserverobj.h"
#endif

#ifndef __ILTCSBASE_H__
#include "iltcsbase.h"
#endif


/*!
Typedef to deal with the fact that there were two different versions of a
console variable handle
*/
typedef HCONSOLEVAR HCONVAR;
DEFINE_HANDLE_TYPE(HCLIENTREF);

/*!  
Load world flags.  
*/

//!  Load objects from the world file?
#define LOADWORLD_LOADWORLDOBJECTS (1<<0)
//! Start running world immediately?
#define LOADWORLD_RUNWORLD (1<<1)

/*!  
Don't reload geometry if the world filename is the same as the
currently loaded one.  
*/
#define LOADWORLD_NORELOADGEOMETRY (1<<2)

/*!  
Load/save objects flags.  
*/

#define RESTOREOBJECTS_RESTORETIME  1   //! Restore time information?

enum
{
    SAVEOBJECTS_SAVEGAMECONSOLE  = 1   //! Save the game console state?
};


/*!
Client info flags.

\b CIF_LOCAL and \b CIF_PLAYBACK are not settable with SetClientInfoFlags().
*/

enum
{
//! Client is local (non-network) connection.
    CIF_LOCAL                 = (1<<0),
//! Virtual client from a demo playback.
    CIF_PLAYBACK              = (1<<1),
//! Client object is sent with full position resolution.
    CIF_FULLRES               = (1<<2),
//! Client object rotations are sent to client.
    CIF_SENDCOBJROTATION      = (1<<3)
};

/*!
Object states.

\b OBJSTATE_INACTIVE and \b OBJSTATE_INACTIVE_TOUCH override the effect of
PingObjects().  \b OBJSTATE_ACTIVE clears the effect of \b OBJSTATE_INACTIVE
or \b OBJSTATE_INACTIVE_TOUCH, but the object can be auto-deactivated.
*/

enum
{
//! Normal healthy object.
    OBJSTATE_ACTIVE         = 0,

//! Inactive (no updates, physics, or touch notifications).
    OBJSTATE_INACTIVE       = 1,

//! Inactive, but receives touch notifications
    OBJSTATE_INACTIVE_TOUCH = 2,
};



/*!  
Server state flags.  
*/

enum
{
//! Server is paused.
    SS_PAUSED      = (1<<0),

//! Server can pre-cache files.
    SS_CACHING      = (1<<2)
};

/*!  
Object lists.  
*/

struct ObjectLink
{
    HOBJECT     m_hObject;
    ObjectLink  *m_pNext;
};

struct ObjectList
{
    ObjectLink  *m_pFirstLink;
    int     m_nInList;
};


#define MAX_CLASSPROPINFONAME_LEN   128

/*!  
Used by ILTServer::GetClassProp.  
*/

class ClassPropInfo
{
public:
    char    m_PropName[MAX_CLASSPROPINFONAME_LEN];
    short       m_PropType;     //! PT_ define from serverobj_de.h.
};


class ILTPhysics;
class ILTModel;
class ILTTransform;
class ILTLightAnim;
class ILTSoundMgr;
struct GenericProp;
struct ObjectCreateStruct;

/*!  
ILTServer interface. This is the interface to most server-side LithTech
functionality.  
*/

class ILTServer : public ILTCSBase {
public:
    interface_version_derived(ILTServer, ILTCSBase, 4);
    
    

/*!  
World control.  
*/

#ifndef DOXYGEN_SHOULD_SKIP_THIS
 
		bool (*IntersectSegment)(IntersectQuery *pQuery, IntersectInfo *pInfo);
		bool (*CastRay)(IntersectQuery *pQuery, IntersectInfo *pInfo);
		virtual LTRESULT	GetWorldBox(LTVector &min, LTVector &max)=0;
		ObjectList*	(*GetPointAreas)(const LTVector *pPoint);
		ObjectList*	(*GetBoxIntersecters)(const LTVector *pMin, const LTVector *pMax);
		virtual LTRESULT FindWorldModelObjectIntersections(
			HOBJECT hWorldModel,
			const LTVector &vNewPos, const LTRotation &rNewRot,
			BaseObjArray<HOBJECT> &objArray)=0;
		ObjectList* (*FindObjectsTouchingSphere)(const LTVector *pPosition,float radius);
		LTRESULT	(*SetObjectRotation)(HOBJECT hObj, const LTRotation *pRotation);
		virtual LTRESULT GetObjectPos(HLOCALOBJ hObj, LTVector *pPos) = 0;
		virtual LTRESULT GetObjectRotation(HLOCALOBJ hObj, LTRotation *pRotation) = 0;

		void	(*SetObjectPos)(HOBJECT hObj, const LTVector *pos);  //!
		LTRESULT	(*RotateObject)(HOBJECT hObj, const LTRotation *pRotation);
		void	(*TiltToPlane)(HOBJECT hObj, const LTVector *pNormal);
		virtual LTRESULT MoveObject(HOBJECT hObj, const LTVector *pNewPos)=0;
		LTRESULT	(*TeleportObject)(HOBJECT hObj, const LTVector *pNewPos);
		virtual LTRESULT	GetStandingOn(HOBJECT hObj, CollisionInfo *pInfo)=0;
		LTRESULT	(*GetLastCollision)(CollisionInfo *pInfo);

#endif//doxygen

/*!
\param pPoint The point in world coordinates.

\param pColor (return) The color value at pPoint.

\return \b DFALSE if the point is outside the world.

Calculate the lighting shade (RGB, range 0-255) at the point you specify.

Used for: Misc.  
*/
    bool (*GetPointShade)(const LTVector *pPoint, LTVector *pColor);

/*!
\param pName Name to search for.

\param objArray (return) An array of objects.

\param nTotalFound (return) Number of objects in array.

Get all objects with the given name.

Used for: Object.  
*/
    virtual LTRESULT FindNamedObjects(const char *pName,
                     BaseObjArray<HOBJECT> &objArray,
                     uint32 *nTotalFound=NULL)=0;

/*!
\param  pList       Object list to relinquish.

Any time the engine gives you a list, give the list back with
this function, or else you will have significant memory leaks.

Used for: Misc.  
*/
    void (*RelinquishList)(ObjectList *pList);

/*!  
Object list management.  
*/

/*!
\return A pointer to the allocated list.

Allocate a list.

\note Free the list with RelinquishList().

\see RelinquishList()

Used for: Object.
*/
    ObjectList* (*CreateObjectList)();

/*!
\param pList Object list to modify.

\param hObj Object to add.

\return a pointer to the new ObjectLink

Add objects to a list.

Used for: Object.  
*/
    ObjectLink* (*AddObjectToList)(ObjectList *pList, HOBJECT hObj);

/*!
\param pList Object list to modify.

\param hObj Object to remove.

Remove an object from the list given the handle.

\note \b Caution: LithTech will do a search over all objects in the list.

Used for: Object.  
*/
    void (*RemoveObjectFromList)(ObjectList *pList, HOBJECT hObj);

/*!  
New message functions. The main difference between these and the
functions above is that these don't free the message, so you
can send it multiple times.  
*/

/*!
\param  pMsg     \b Message to send to the object.
\param  hSender     Source object (can be \b NULL).
\param  hSendTo     Destination object.
\param  flags       Message flags.
\return \b LT_INVALIDPARAMS if the message is invalid; otherwise, returns \b LT_OK.

\see ObjectMessageFn()
\see ILTCommon::CreateMessage()
\see ILTMessage_Write::Read()

Send a message to an object. The object will receive the message through its
ObjectMessageFn function.

Used for: Messaging.  
*/
    virtual LTRESULT SendToObject(ILTMessage_Read *pMsg,
        HOBJECT hSender, HOBJECT hSendTo, uint32 flags)=0;

/*!
\param  pMsg     \b Message to send to the server.
\param  hSender     Source object (cannot be \b NULL).
\param  flags       Message flags.
\return \b LT_INVALIDPARAMS if the message is invalid, or the server shell is invalid; otherwise, returns \b LT_OK.

\see OnObjectMessage()
\see ILTCommon::CreateMessage()
\see ILTMessage_Write::Read()

Send the message to the server shell. The server shell will receive the message
through its OnObjectMessage() function.

Used for: Messaging.  
*/
    virtual LTRESULT SendToServer(ILTMessage_Read *pMsg,
        HOBJECT hSender, uint32 flags)=0;

/*!
\param  hObject     Object to modify.
\param  pMsg     \b Message to associate with this object.
\return \b LT_INVALIDPARAMS if the message is invalid; otherwise, returns \b LT_OK.

Sets up a special effect message.  If your object has a
special effect message, the SpecialEffectNotify() of the client shell
will be called.

\note An object can have only \b one associated special effect
message.  If the object already has a special effect message, then
calling this function will replace it.

\see SpecialEffectNotify()
\see ILTCommon::CreateMessage()
\see ILTMessage_Write::Read()

Used for: Messaging.  
*/
    virtual LTRESULT SetObjectSFXMessage(HOBJECT hObject,
        ILTMessage_Read *pMsg)=0;

/*!
\param  pMsg     \b Message to send to the client.
\param  hSendTo     Destination client.  Use \b NULL to send to all clients.
\param  flags       Message flags.
\return \b LT_INVALIDPARAMS if the message is invalid; otherwise, returns \b LT_OK.

\see OnMessage()
\see ILTCommon::CreateMessage()
\see ILTMessage_Write::Read()

Send the message to the client.  The client shell will receive the message
through its OnMessage() function.

Used for: Messaging.  
*/
    virtual LTRESULT SendToClient(ILTMessage_Read *pMsg,
        HCLIENT hSendTo, uint32 flags)=0;

/*!
\param  pMsg     \b SFX Message to send
\param  pos     Position of message reference (in world coordinates).
\param  flags       Message flags.
\return \b LT_INVALIDPARAMS if the message is invalid; otherwise, returns \b LT_OK.

\see SpecialEffectNotify()
\see ILTCommon::CreateMessage()
\see ILTMessage_Write::Read()

Send an "instant" special effect message to all the clients who can see
\b pos. The clients will receive the message through the IClientShell::SpecialEffectNotify()
function.

Used for: Messaging.  
*/
    virtual LTRESULT SendSFXMessage(ILTMessage_Read *pMsg, const LTVector &pos,
        uint32 flags)=0;

/*!  
Client Functions.  
*/

/*!
\param  hParent     Parent client.
\param  hChild      Child client.
\return LT_INVALIDPARAMS if hParent == hChild, otherwise LT_OK

Attach one client to another. Messages sent to \b hParent will also go
to \b hChild, and the client object of \b hChild will be reported as
the object of \b hParent.  This is useful for demo playback when you
want to watch another player's view.

Used for: Server Management.  
*/
    LTRESULT (*AttachClient)(HCLIENT hParent, HCLIENT hChild);

/*!
\param  hClient     Child client to detach.
\return LT_OK

Detach a child client from its parent attachment.

Used for: Server Management.
*/
    LTRESULT (*DetachClient)(HCLIENT hClient);

/*!
\param hPrev (input/return) The client to start from.  Use \b LTNULL to
get the first client.

\return Handle to the next client after \b hPrev.  \b LTNULL when there are
no more.

Get next client.

Used for: Server Management.  
*/
    HCLIENT (*GetNextClient)(HCLIENT hPrev);

/*!
\param hRef (input/return) The client reference to start from.  Use
\b LTNULL to get the first client reference.

\return Handle to the next client reference after \b hRef, or \b LTNULL when
there are no more.

Get next saved client reference. (These are created when a world is
saved.)

Used for: Server Management.  
*/
    HCLIENTREF (*GetNextClientRef)(HCLIENTREF hRef);

/*!
\param hClient Client to query.

\return Client reference info flags.

\b HCLIENTREF objects are used for comparison to \b HCLIENT objects.

Used for: Server Management.  
*/
    uint32 (*GetClientRefInfoFlags)(HCLIENTREF hClient);

/*!
\param hClient Client to query.

\param pName (return) Buffer to hold client reference name.

\param maxLen Buffer length.

\return false if the client ref is invalid. Otherwise, true.

An \b HCLIENTREF is used for comparison to an \b HCLIENT.

Used for: Server Management.  
*/
    bool (*GetClientRefName)(HCLIENTREF hClient, char *pName,
        int maxLen);

/*!
\param hClient Client to query.

\return Handle to client reference object.

An \b HCLIENTREF is used for comparison to an \b HCLIENT.

Used for: Server Management.  
*/
    HOBJECT (*GetClientRefObject)(HCLIENTREF hClient);

/*!
\param clientID Client to query.

\return Client handle.

Get the handle of the client identified by clientID.  GetClientHandle() will return NULL if clientID is not valid.

Used for: Server Management.  
*/
    HCLIENT (*GetClientHandle)(uint32 clientID);

/*!
\param hClient Client to query.

\return Client ID.

Get the unique ID of a client.

Used for: Server Management.  
*/
    uint32 (*GetClientID)(HCLIENT hClient);

/*!
\param hClient Client to query.

\param ping (return) Ping time.

\return LT_NOTINITIALIZED if client is not connected. Otherwise, LT_OK.

Get the ping time of a client.

Used for: Server Management.  
*/
    virtual LTRESULT GetClientPing(HCLIENT hClient, float &ping)=0;

/*!
\param hClient Client to query.

\param pName (return) Buffer to hold client name.

\param maxLen Buffer length.

\return \b true if successful; otherwise, returns \b false.

Get the name of a client.

Used for: Server Management.  
*/
    bool (*GetClientName)(HCLIENT hClient, char *pName, int maxLen);

/*!
\param hClient Client to query.

\param pName Buffer holding desired name

\param maxLen Buffer length.

\return \b true if successful; otherwise, returns \b false.

Set the name of a client.

Used for: Server Management.  
*/
		bool	(*SetClientName)(HCLIENT hClient, const char *pName, int MaxLen);

/*!
\param hClient Client to query.

\param pData (out) Buffer to hold client data.  Pass NULL for length returned in maxLen.

\param maxLen (in,out) Buffer length.  Sets bytes copied.

\return \b true if successful; otherwise, returns \b false.

Get the name of a client.

Used for: Server Management.  
*/
    virtual bool GetClientData(HCLIENT hClient, uint8* pData, int& maxLen) = 0;

/*!
\param hClient Client to query.

\param pData Buffer holding desired data

\param len Buffer length.

\return \b true if successful; otherwise, returns \b false.

Set the data of a client.

Used for: Server Management.  
*/
	virtual bool SetClientData(HCLIENT hClient, uint8 const* pData, int len ) = 0;

/*!
\param hClient Client to query

\param pAddr Array for holding the client's address

\param pPort uint16 for holding the client's port

\return \b LT_ERROR if an invalid parameter is provided; otherwise, returns \b LT_OK

Retrieve the TCP/IP communications address and port for the given client

Note: The local client will return 0.0.0.0:0

Used for: Client Management
*/
		virtual LTRESULT GetClientAddr(HCLIENT hClient, uint8 pAddr[4], uint16 *pPort) = 0;

/*!
\param hClient Client to modify.

\param dwClientFlags Client info flags (a combination of the \b CIF_
flags above).

Set client info flags.

Used for: Server Management.  
*/
    void (*SetClientInfoFlags)(HCLIENT hClient, uint32 dwClientFlags);

/*!
\param hClient Client to query.

\return Client info flags (a combination of the \b CIF_ flags above).

Get client info flags.

Used for: Server Management.  
*/
    uint32 (*GetClientInfoFlags)(HCLIENT hClient);

/*!
\param hClient Client to modify.

\param pData User data.

Set user data for \b HCLIENT objects.

Used for: Server Management.  
*/
    void (*SetClientUserData)(HCLIENT hClient, void *pData);

/*!
\param hClient Client to query.

\return Pointer to user data.

Get user data for \b HCLIENT objects.

Used for: Server Management.  
*/
    void* (*GetClientUserData)(HCLIENT hClient);

/*!
\param hClient Client to kick.

Kick a client off the server. OnRemoveClient() will be called.

\see OnRemoveClient()

Used for: Server Management.  
*/
    LTRESULT (*KickClient)(HCLIENT hClient);

/*!
\param hClient Client to modify.

\param pPos Position (in world coordinates).

\return LT_INVALIDPARAMS if hClient is not valid. Otherwise, LT_OK.

Set where the client is seeing and hearing from.  This function
controls what the server sends the client.  You should set this every
frame.

Used for: Server Management.  
*/
    LTRESULT (*SetClientViewPos)(HCLIENT hClient, const LTVector *pPos);

/*!  
The "game console state".  This is a set of console variables and
functions used internally by the application DLLs.  The user cannot
access this at all.  It is very useful for tracking global game
variables and triggering them through game objects.  
*/

/*!
\param pString Console command to execute.

Run a string through the game console.  The syntax is the same as any
other console string (semicolons separate commands, and new variable
names create new \b HCONVAR handles).

Used for: Misc.  
*/
    void (*RunGameConString)(const char *pString);

/*!
\param pName Console variable name.

\param pVal Console variable value.

Set a console variable.  Creates a new one if it doesn't exist.

Used for: Misc.  
*/
    void (*SetGameConVar)(const char *pName, const char *pVal);

/*!
\param pName Console variable name.

\return Handle to console variable object.

Get a handle to a console variable.

Used for: Misc.  
*/
    HCONVAR (*GetGameConVar)(const char *pName);

/*!
Helpers.
*/

/*!
\param pPropName Property name (case-sensitive).

\param pRet (return) Buffer to hold string property.

\param maxLen Buffer length.

\return \b LT_NOTFOUND if property doesn't exist.

Get a string property value from the world file.

Used for: Object.  
*/
    LTRESULT (*GetPropString)(const char *pPropName, char *pRet, int maxLen);

/*!
\param pPropName Property name (case-sensitive).

\param pRet (return) Vector property to retrieve.

\return \b LT_NOTFOUND if property doesn't exist.

Get a vector property value from the world file.

Used for: Object.  
*/
    LTRESULT (*GetPropVector)(const char *pPropName, LTVector *pRet);

/*!
\param pPropName Property name (case-sensitive).

\param pRet (return) Color property to retrieve.

\return \b LT_NOTFOUND if property doesn't exist.

Get a color property value from the world file.

Used for: Object.  
*/
    LTRESULT (*GetPropColor)(const char *pPropName, LTVector *pRet);

/*!
\param pPropName Property name (case-sensitive).

\param pRet (return) Real or float property to retrieve.

\return \b LT_NOTFOUND if property doesn't exist.

Get a real or float property value from the world file.

Used for: Object.  
*/
    LTRESULT (*GetPropReal)(const char *pPropName, float *pRet);

/*!
\param pPropName Property name (case-sensitive).

\param pRet (return) Flags property to retrieve.

\return \b LT_NOTFOUND if property doesn't exist.

Get a flags property value from the world file.

Used for: Object.  
*/
    LTRESULT (*GetPropFlags)(const char *pPropName, uint32 *pRet);

/*!
\param pPropName Property name (case-sensitive).

\param pRet (return) Bool property to retrieve.

\return \b LT_NOTFOUND if property doesn't exist.

Get a boolean property value from the world file.

Used for: Object.  
*/
    LTRESULT (*GetPropBool)(const char *pPropName, bool *pRet);

/*!
\param pPropName Property name (case-sensitive).

\param pRet (return) Long integer property to retrieve.

\return \b LT_NOTFOUND if property doesn't exist.

Get a long integer property value from the world file.

Used for: Object.  
*/
    LTRESULT (*GetPropLongInt)(const char *pPropName, int32 *pRet);

/*!
\param pPropName Property name (case-sensitive).

\param pRet (return) Quaternion rotation property to retrieve.

\return \b LT_NOTFOUND if property doesn't exist.

Get a quaternion rotation property value from the world file.

Used for: Object.  
*/
    LTRESULT (*GetPropRotation)(const char *pPropName, LTRotation *pRet);

/*!
\param pPropName Property name (case-sensitive).

\param pAngles (return) Euler rotation property to retrieve.

\return \b LT_NOTFOUND if property doesn't exist.

Get an Euler rotation property value from the world file.

Used for: Object.  
*/
    LTRESULT (*GetPropRotationEuler)(const char *pPropName, LTVector *pAngles);

/*!
\param pPropName Property name (case-sensitive).

\param pProp (return) Generic property to retrieve.

\return LT_NOTFOUND if the named property is not found, else LT_OK


Fills in the \b GenericProp for the different data types.  For a list
of which property types map to which \b GenericProp variable, see the
\b GenericProp structure.

\note If the property exists, it always initializes the members in the
prop first, so if the \b GenericProp variable doesn't support that
property type, it will be zero

Used for: Object.  
*/
    LTRESULT (*GetPropGeneric)(const char *pPropName, GenericProp *pProp);

/*!
\param pPropName Property name (case-sensitive).

\param pPropType (return) Property type to retrieve (one of the \b PT_
defines).

\return Returns \b LT_NOTFOUND if it doesn't exist.

See if a property exists.

Used for: Object.  
*/
    LTRESULT (*DoesPropExist)(const char *pPropName, int32 *pPropType);

/*!
\param pObject Object to query.

\return Handle to \b pObject.

Get objects from handles.

Used for: Object.  
*/
    HOBJECT (*ObjectToHandle)(LPBASECLASS pObject);

/*!
\param pObject Handle of object to query.

\return Object.

Get handles from objects.

\note HandleToObject() will return \b NULL if the given object does not
reside on this server (i.e. if it is controlled remotely).

Used for: Server Management.  
*/
    LPBASECLASS (*HandleToObject)(HOBJECT hObject);

/*!
\param pStr1 String 1 for comparison.

\param pStr2 String 2 for comparison.

\return \b true if the strings are the same, \b false otherwise.

Does a fast upper-case string comparison of the two strings.

Used for: Misc.  
*/
    bool (*UpperStrcmp)(const char *pStr1, const char *pStr2);

/*!
\param pCommand String to parse.

\param pNewCommandPos (return) New ending position (after semicolon or
end of string).

\param argBuffer Memory for scratchpad.

\param argPointers (return) Array of pointers to arguments found.

\param nArgs (return) Number of arguments found (size of array).

\return \b true if a semicolon is found and there are more arguments in
the string, or \b false if there are no more arguments.

Parse a string for arguments.  This function works like a command line
parser.  Arguments within quotation marks are considered one argument.

Used for: Misc.  
*/
    int (*Parse)(const char *pCommand, const char **pNewCommandPos,
        char *argBuffer, char **argPointers, int *nArgs);

/*!  
Inter-object link manipulation.
*/

/*!
\param hOwner Owner object.

\param hLinked Linked object.

\return \b LT_ERROR if trying to make a link to itself.

If you want to hold on to an \b HOBJECT and receive notification when
the object is removed from the world, you can create an inter-object
link between the two.  The owner will get \b MID_LINKBROKEN
notification when the link is being broken (by either the owner or
linked object being removed).  If a link between the two objects
already exists, the function will not create another link and return
\b LT_OK.

Used for: Object.
*/
    LTRESULT (*CreateInterObjectLink)(HOBJECT hOwner, HOBJECT hLinked);

/*!
\param hOwner Owner object.

\param hLinked Linked object.

Break an inter-object link between the owner and the linked object, if
one exists.  You can only break a link from the owner, since the
linked object doesn't know it is linked to the owner.

\note \b MID_LINKBROKEN will \em not be called.

Used for: Object.
*/
    virtual void BreakInterObjectLink(HOBJECT hOwner, HOBJECT hLinked)=0;

/*!
\param hObj Object that links will be broken from.

\param hLinked Linked object.

\param bNotify Should the linked object be notified?

Break all inter-object links between the object and the linked object, if
one exists.  If hLinked is \b LTNULL all links to hObj are broken.

\note \b MID_LINKBROKEN will \em not be called if bNotify is \b false.

Used for: Object.
*/
	virtual void BreakInterObjectLink( HOBJECT hObj, HOBJECT hLinked, bool bNotify )=0;	

/*!
Object manipulation.
*/

/*!
\param hParent Parent object.

\param hChild Child object.

\param pSocketName Socket name (on parent).

\param pOffset Position offset (in local parent coordinates).

\param pRotationOffset Rotation offset (in local parent coordinates).

\param pAttachment (return) Attachment handle.

\return \b LT_OK, \b LT_NODENOTFOUND, or \b LT_INVALIDPARAMS.

Attaches \b hChild to \b hParent.  If \b pSocketName is set, this
function will attach \b hChild to a specific node on \b
hParent. LithTech will automatically detach if you remove \b hParent,
but it will \em not automatically detach if you remove \b hChild.  If
you remove the child without removing the attachment, the results are
undefined.

Used for: Object.
*/
    LTRESULT (*CreateAttachment)(HOBJECT hParent, HOBJECT hChild,
								 const char  *pSocketName, 
								 LTVector    *pOffset, 
								 LTRotation  *pRotationOffset,
								 HATTACHMENT *pAttachment);

/*!
\param hAttachment Attachment to remove.

\return \b LT_INVALIDPARAMS if hAttachment is NULL, \b LT_ERROR if hAttachment
has no parent, LT_NOTFOUND if the parent is not found, else \b LT_OK

Removes an attachment.

\note An attachment becomes invalid when you remove the parent so this
function will crash if you call it with an attachment with a parent
you've removed.

Used for: Object.
*/
    LTRESULT (*RemoveAttachment)(HATTACHMENT hAttachment);

/*!
\param hParent Parent object.

\param hChild Child object.

\param hAttachment (return) Attachment handle, or \b NULL if there is an
error.

\return \b LT_ERROR, \b LT_NOTFOUND, or \b LT_OK if it found it.

Look for an attachment on the parent.

Used for: Object.
*/
    LTRESULT (*FindAttachment)(HOBJECT hParent, HOBJECT hChild,
        HATTACHMENT *hAttachment);

/*!
\param hObject Object to query.

\param r (return) Red value (range: 0.0-1.0).

\param g (return) Green value (range: 0.0-1.0).

\param b (return) Blue value (range: 0.0-1.0).

\param a (return) Alpha value (range: 0.0-1.0).

\return \b false if hObject is NULL, else \b true

Get object color. All objects default to (0,0,0,1).

Used for: Object.
*/
    bool (*GetObjectColor)(HOBJECT hObject, float *r, float *g,
        float *b, float *a);

/*!
\param hObject Object to modify.

\param r Red value (range: 0.0-1.0).

\param g Green value (range: 0.0-1.0).

\param b Blue value (range: 0.0-1.0).

\param a Alpha value (range: 0.0-1.0).

\return \b false if hObject is NULL, else \b true

Set object color.  All objects default to (0,0,0,1) For lights, this
changes the color of the light.  For models, this brightens the
shading of a model.

\note This function overrides a model's render style's material diffuse color.

Used for: Object.
*/
    bool (*SetObjectColor)(HOBJECT hObject, float r, float g,
        float b, float a);

/*!
\param hObj Object to start from.  Use \b NULL to get first object.

\return \b NULL if there are no more objects; otherwise, returns the next
object.

Iterate through all the objects in the world.  This is generally a bad
idea for speed reasons, but sometimes it is necessary.

\see GetNextInactiveObject()

Used for: Object.
*/
    HOBJECT (*GetNextObject)(HOBJECT hObj);

/*!
\param hObj Object to start from.  Use \b NULL to get the first inactive
object.

\return \b NULL if there are no more inactive objects; otherwise, returns
the next inactive object.

Same as GetNextObject(), but this iterates over all the inactive objects.

\see GetNextObject()

Used for: Object.
*/
    HOBJECT (*GetNextInactiveObject)(HOBJECT hObj);

/*!
\param hObject Object to query.

\param pName (return) Buffer to hold name.

\param nameBufSize Buffer size.

\return LT_INVALIDPARAMS if the buffer is not large enough, else LT_OK

Get the name of an object.

Used for: Object.
*/
    virtual LTRESULT GetObjectName(HOBJECT hObject, char *pName,
        uint32 nameBufSize)=0;

/*!
\param hObj Object to modify.

\param nextUpdate Next update time.

This is a counter that controls when each object has its Update()
function called. It is set to 0.0001 when an object is created, so
Update() is called right away by default.

Used for: Object.
*/
    void (*SetNextUpdate)(HOBJECT hObj, LTFLOAT nextUpdate);

/*!
\param hObj Object to query.

\param pScale (return) Scale.

\return \b LT_ERROR if jObj or pScale is invalid, else \b LT_OK

Get the scale of the object (only works on models and sprites).

Used for: Object.
*/
    LTRESULT (*GetObjectScale)(HOBJECT hObj, LTVector *pScale);

/*!
\param hObj Object to modify.

\param pNewScale New scale to set.

Scale the object (only works on models and sprites).

Used for: Object.
*/
    void (*ScaleObject)(HOBJECT hObj, const LTVector *pNewScale);

/*!
\param hObj The object to query.

\param flags (return) Flags (a combination of \b NETFLAG_ defines).

\return \b LT_INVALIDPARAMS if hObj is not found, else \b LT_OK

Get the net flags of an object.

Used for: Object.
*/
    virtual LTRESULT GetNetFlags(HOBJECT hObj, uint32 &flags)=0;

/*!
\param hObj The object to modify.

\param flags Flags (a combination of \b NETFLAG_ defines).

\return \b LT_INVALIDPARAMS if hObj is not found, else \b LT_OK

Set the net flags of an object.

Used for: Object.
*/
    virtual LTRESULT SetNetFlags(HOBJECT hObj, uint32 flags)=0;

/*!
\param hObj The object to modify.

\param state The object state (a combination of \b OBJSTATE_ defines).

Set the state of an object.

Used for: Object.
*/
    void (*SetObjectState)(HOBJECT hObj, int state);

/*!
\param hObj The object to query.

\return The object state (a combination of \b OBJSTATE_ defines).

Get the state of an object.

Used for: Object.
*/
    int (*GetObjectState)(HOBJECT hObj);

/*!
\param hObj The object to modify.

\param pri The blocking priority.

Set the blocking priority (defaults to 0).  See the FAQ for a description
of how this works.

Used for: Object.
*/
    void (*SetBlockingPriority)(HOBJECT hObj, uint8 pri);

/*!
\param hObj The object to query.

\return The blocking priority.

Get the blocking priority (defaults to 0).  See the FAQ for a description
of how this works.

Used for: Object.
*/
    uint8 (*GetBlockingPriority)(HOBJECT hObj);

/*!
Sprite manipulation.
*/

/*!
\param  hObj    Sprite to modify.
\param  hPoly       Polygon to clip to.  Use INVALID_POLY to un-clip the sprite.

\return \b LT_ERROR - \em hObj is invalid (NULL, or not an \b OT_SPRITE).
\return \b LT_OK - Successful.

Clips the sprite onto a polygon.

Used for: Special FX.
*/
    LTRESULT (*ClipSprite)(HOBJECT hObj, HPOLY hPoly);

/*!
Light manipulation.
*/

/*!  The color of a light is snapped to 256 different values, so if
you want to do fancy interpolation, you will need to store your own,
higher precision, color values.
*/

/*!
\param  hObj    Light to query.
\param  r (return) Red value (0-1).
\param  g (return) Green value (0-1).
\param  b (return) Blue value (0-1).

Get the color of a light. When you create a light, its color defaults to (0,0,0).

\note This merely calls GetObjectColor().

\see SetLightColor()
\see GetObjectColor()

Used for: Special FX.
*/
    void (*GetLightColor)(HOBJECT hObj, float *r, float *g, float *b);

/*!
\param  hObj    Light to modify.
\param  r       Red value (0-1).
\param  g       Green value (0-1).
\param  b       Blue value (0-1).

Set the color of a light.

\note This merely calls SetObjectColor().

\see GetLightColor()
\see SetObjectColor()

Used for: Special FX.
*/
    void (*SetLightColor)(HOBJECT hObj, float r, float g, float b);

/*!
\param  hObj    Light to query.

\return If \em hObj is an \b OT_LIGHT, returns the radius of the light.
    Otherwise returns 1.0.

Get the radius of a light. When you create a light, its radius defaults to 100.

\see SetLightRadius()

Used for: Special FX.
*/
    float (*GetLightRadius)(HOBJECT hObj);

/*!
\param  hObj    Light to modify.
\param  radius  Radius to set.

Set the radius of a light.

\see GetLightRadius()

Used for: Special FX.
*/
    void (*SetLightRadius)(HOBJECT hObj, float radius);


/*!
Network session manipulation 
*/

/*!
\param sName    New session name to use.
\return \b LT_NOTINITIALIZED if the main network driver is uninitialized, or \b LT_ERROR if the session name cannot be set; otherwise, returns \b LT_OK.

Update the session name.

Used for: Networking.
*/
    LTRESULT (*UpdateSessionName)(const char* sName);

/*!
\param sName (return) String to fill with session name.
\param dwBufferSize Size of sName parameter.
\return LT_NOTINITIALIZED if the main network driver is uninitialized, or LT_ERROR if session is invalid or the buffer is too small to contain the session name; otherwise, returns LT_OK.

Get the name of the session.

Used for: Networking.
*/
    LTRESULT (*GetSessionName)(char* sName, uint32 dwBufferSize);

/*!
Model manipulation.
*/

/*!
\param hObj Model to modify.

\param pNodeName Node to modify.

\param bHidden \b TRUE means to hide; \b FALSE means to unhide.

\return \b LT_OK, \b LT_ERROR, \b LT_NOCHANGE, or \b LT_NODENOTFOUND.

Hide or unhide a node on the model (they are all unhidden by default).

Used for: Models and Animation.
*/
    LTRESULT (*SetModelNodeHideStatus)(HOBJECT hObj, char *pNodeName,
        bool bHidden);

/*!
\param hObj The model to query.

\param pNodeName The node to query.

\param bHidden (return) \b true for hidden, \b false for unhidden.

\return \b LT_OK, \b LT_ERROR, \b LT_NOCHANGE, or \b LT_NODENOTFOUND.

Get the hide/unhide status of a node on the model (they are all
unhidden by default).

Used for: Models and Animation.
*/
    LTRESULT (*GetModelNodeHideStatus)(HOBJECT hObj, char *pNodeName,
        bool *bHidden);

/*!
\param hObject The model to query.

\param hAnim The animation to query.

\return Pointer to the name of the animation, or \b LTNULL if the handle is
invalid.

Get the animation name from an animation.

Used for: Models and Animation.
*/
    const char *(*GetAnimName)(HOBJECT hObject, HMODELANIM hAnim);

/*!
\param hObj Model to modify.

\param bPlaying \b true for playing, or \b false for not playing.

Set the Playing state of the model.  The default state is \b false.

Used for: Models and Animation.
*/
    void (*SetModelPlaying)(HOBJECT hObj, bool bPlaying);

/*!
\param hObj Model to query.

\return \b true if the model is playing; otherwise, returns \b false.

Get the playing state of the model. The default state is \b false.

Used for: Models and Animation.
*/
    bool (*GetModelPlaying)(HOBJECT hObj);

/*!
\param hObj Model to query.

\param pFilename (return) Buffer for model filename (can use \b LTNULL).

\param fileBufLen The \b pFilename buffer length.

\param pSkinName (return) Buffer for skin filename (can use LTNULL).

\param skinBufLen The \b pSkinName buffer length.

\return \b false if the object is not a model.

Get the model filenames.  Initializes \b pFilename or \b pSkinName to zero
length if the model doesn't have a filename or skin.

Used for: Models and Animation.
*/
    bool (*GetModelFilenames)(HOBJECT hObj, char *pFilename, int fileBufLen, char *pSkinName, int skinBufLen);

/*!
Save game 
*/

/*!
\param pszSaveFileName The save filename.

\param pList The list of objects to save.

\param dwParam User parameters (\b dwParam gets passed into
\b MID_SAVEOBJECT as \b fData).

\param flags The save flags (\b SAVEOBJECTS_ defines).

\return \b LT_ERROR if file could not be opened.  \b LT_OK on success.

Save objects to a file.

Used for: Misc.
*/
    LTRESULT (*SaveObjects)(const char *pszSaveFileName, ObjectList *pList,
        uint32 dwParam, uint32 flags);

/*!
\param pszRestoreFileName The restore filename.

\param dwParam User parameters (\b dwParam gets passed into
\b MID_LOADOBJECT as \b fData).

\param flags The save flags (\b RESTOREOBJECTS_ defines).

\return \b LT_OK on success.

Restore objects from a file.

Used for: Misc.
*/
    LTRESULT (*RestoreObjects)(const char *pszRestoreFileName, uint32 dwParam,
        uint32 flags);

/*!
\param nSaveFileVersion - The save file version number.

\return \b LT_OK on success.

Access the engine's save file version.

Used for: Misc.
*/
    virtual LTRESULT GetSaveFileVersion( uint32& nSaveFileVersion ) = 0;

/*!
Load world
*/

/*!
\param pszWorldFileName Name and path in the rez of the world file to load.

\param flags A combination of the \b LOADWORLD_ flags.

\return LT_MISSINGWORLDFILE is the world file cannot be found. Otherwise, LT_OK.

Used for: Server Management.
*/
    LTRESULT (*LoadWorld)(const char *pszWorldFileName, uint32 flags);

/*!
\return LT_NOTINWORLD if world is not loaded. Otherwise, LT_OK.

Run the loaded world.

Used for: Server Management.
*/
    LTRESULT (*RunWorld)();

/*!
\param msg		Message to send.
\return \b LT_NOTFOUND if there is no server app or if it isn't set up to receive the message; otherwise, returns \b LT_OK.

Send a message to the standalone server app.  It will receive it through the ServerAppHandler::ShellMessageFn() function.

Used for: Networking.
*/
		LTRESULT (*SendToServerApp)( ILTMessage_Read& msg );

/*!
\param nMaxConnections (return) Filled in with maximum number of remote connections allowed.
\return LT_NOTINITIALIZED if the main network driver is uninitialized; otherwise, returns LT_OK.

Get the maximum number of remote clients allowed for the session.

Used for: Networking.
*/
    LTRESULT (*GetMaxConnections)(uint32 &nMaxConnections);


/*!
\param pDirName The directory path.

\return A pointer to a non-circular linked list of filenames terminated by \b DNULL.

Get a list of files and directories in a directory (pass in "" to start with).
Use FreeFileList() to avoid memory leaks.

\see FreeFileList()

Used for: Misc.
*/
    FileEntry* (*GetFileList)(const char *pDirName);

/*!
\param pHead Head of file list.

Use when you're done with each list you get from GetFileList().

\see GetFileList()

Used for: Misc.
*/
    void (*FreeFileList)(FileEntry *pHead);


/*!  
Main functions.  
*/

/*!
Grants access to the \b m_pGameInfo from the \b StartGameRequest structure.

\param ppData Filled with a pointer to the GameInfo data.
\param pLen Filled with the length of the GameInfo data.
\return LT_OK

\note This is \em not copy of the data. The pointer filled in can be NULL if
there is no game info data.

Used for: Server Management.  
*/
    LTRESULT (*GetGameInfo)(void **ppData, uint32 *pLen);

/*!
\param hClass Class to query.
\param count (return) Number of properties.
\return LT_OK

Get the number of properties defined by the given class.

Used for: Server Management.
*/
    virtual LTRESULT GetNumClassProps(const HCLASS hClass,
        uint32 &count)=0;

/*!
\param pName Name of class to retrieve.

\return \b LTNULL if the class doesn't exist; otherwise, returns the
handle of the requested class.

Used for: Server Management.  
*/
    HCLASS (*GetClass)(const char *pName);

/*!
\param  hClass      Class to query.
\param  iProp       Property to query.
\param  info (return) Property info.
\return LT_OK

Get the \b ClassDef info from a class. This is useful if you want to
access the property names of the class.

Used for: Server Management.  
*/
    virtual LTRESULT GetClassProp(const HCLASS hClass,
        const uint32 iProp, ClassPropInfo &info)=0;

/*!
\param  hClass Class to query.
\param  pName (return) Buffer for name.
\param  maxNameBytes (return) Size of buffer.
\return LT_OK

Get the name of a class.

Used for: Server Management.
*/
    virtual LTRESULT GetClassName(const HCLASS hClass,
        char *pName, uint32 maxNameBytes)=0;

/*!
\param  hObject     Object to query.
\return The class of the object.

Get the class of an object.

Used for: Object.
*/
    HCLASS (*GetObjectClass)(HOBJECT hObject);

/*!
\param  hClass      Class to query.
\param  obj (return) Static object.
\return \b LT_NOTFOUND if the server doesn't have a static object of this class.

Get the static object of a class.

Used for: Server Management.
*/
    LTRESULT (*GetStaticObject)(HCLASS hClass, HOBJECT *obj);

/*!
\param  hClass      Class to query.
\param  hTest       Test base class.

\return \b TRUE if \b hClass is an \b hTest, or is derived from \b hTest.
Returns \b FALSE if \b hClass is not an \b hTest, if \b hClass is not
derived from \b hTest, or if \b hTest is \b LTNULL.

Tells if \b hClass is the class of \b hTest (or is derived from \b hTest).  If
you pass in \b LTNULL for \b hTest (for example, if GetClass() returns \b LTNULL
in IsKindOf(hClass, GetClass("TestClass"))), then IsKindOf() will return \b FALSE.

Used for: Object.  
*/
    bool (*IsKindOf)(HCLASS hClass, HCLASS hTest);

/*!
\param  hClass      Class to use for object creation.
\param  pStruct     Description of the object.
\return Handle to object.

Create an object using the default type of the object.  If you pass in \b LTNULL for
pStruct, OnPreCreate will not be called, and the object will not be added to the world.
(This is only valid for creating CF_CLASSONLY objects.)

Used for: Object.
*/
    LPBASECLASS (*CreateObject)(HCLASS hClass, ObjectCreateStruct *pStruct);

/*!
\param  hClass      Class to use for object creation.
\param  pStruct     Description of object.
\param  pszProps    Properties string.
\return Handle to object.

Create an object, using a string to specify the properties of the object.

Used for: Object.
*/
    LPBASECLASS (*CreateObjectProps)(HCLASS hClass,
        ObjectCreateStruct *pStruct, const char *pszProps);

/*!
Re-declaration of ILTCSBase::RemoveObject(HOBJECT)
*/
    virtual LTRESULT RemoveObject(HOBJECT hObj) = 0;
/*!
\param	hClass		Class of the object
\param	pObject		Class-only object to remove
\return LT_OK for valid objects, LT_ERROR otherwise

Delete a class-only object

Used for: Class-only Objects
*/
    virtual LTRESULT RemoveObject(const HCLASS hClass, LPBASECLASS pObject) = 0;

/*!
\return Current state flags (a combination of the \b SS_ flags above).

Get server state flags.

Used for: Server Management.
*/
    uint32 (*GetServerFlags)();

/*!
\param flags Flags to set (a combination of the \b SS_ flags above).
\return New flags (some may not be accepted).

Set server state flags.

Used for: Server Management.
*/
    uint32 (*SetServerFlags)(uint32 flags);

/*!
\param  fileType    Type of file (one of the \b FT_ defines in ltcodes.h).
\param  pFilename   File to cache.
\return \b LT_NOTFOUND if it can't find the file.

Cache the given file.  Only call this from ServerShell::CacheFiles.

Used for: Misc.
*/
    LTRESULT (*CacheFile)(uint32 fileType, const char *pFilename);

/*!
\param  pFilename   File to load.

\param type File type (one of the \b FT_ defines in ltcodes.h).  Types
currently supported are \b FT_MODEL and \b FT_TEXTURE. (Textures are not
loaded on the server.  A message is sent to the clients to load the
texture in their thread.)

\return \b LT_UNSUPPORTED if you pass an invalid type, \b
LT_MISSINGFILE if the file is missing, \b LT_ALREADYEXISTS if the file
is already in memory, or \b LT_INPROGRESS if it is already loading in
the thread. (\b Note: Many of these returns don't signify errors.)

Load the specified file in the loader thread.  Also send a message to
clients saying to do the same.  When the operation is completed, this
function calls IServerShell::FileLoadNotify with a result (which can
be an error if the file is missing).  Objects can be created using the
file but they will not be visible until all files they need are
loaded.

Used for: Misc.  
*/
    virtual LTRESULT ThreadLoadFile(const char *pFilename, uint32 type)=0;

/*!
\param  pFilename   File to unload.
\param  type    File type (one of the FT_ defines in ltcodes.h).

\return \b LT_ERROR if the file is being used and can't be unloaded, or
\b LT_NOTFOUND if the file isn't currently loaded.

Unload the data.

Used for: Misc.  
*/
    virtual LTRESULT UnloadFile(const char *pFilename, uint32 type)=0;

/*!
Helpers.
*/

/*!
\param  hPoly       Polygon to query.
\param  hObject (return) Object containing hPoly.

Get the world object that a particular \b HPOLY comes from.

Used for: Misc.  
*/
    virtual LTRESULT GetHPolyObject(
        const HPOLY hPoly, HOBJECT &hObject)=0;

/*!
\param  pCounter    The counter that the engine will update.

Start a counter.  Use this to time sections of code.  Timing is done in
microseconds (millionths of a second).

Used for: Misc.
*/
    void (*StartCounter)(LTCounter *pCounter);

/*!
\param  pCounter    The counter that the engine will stop.
\return Microseconds since StartCounter was called.

Stop a counter.  Use this to time sections of code.  Timing is done in
microseconds (millionths of a second).

Used for: Misc.
*/
    uint32 (*EndCounter)(LTCounter *pCounter);

/*!
\param  min     Minimum extent of range.
\param  max     Maximum extent of range.
\return A random floating-point number in the given range.

Used for: Misc.
*/
    LTFLOAT (*Random)(LTFLOAT min, LTFLOAT max);

/*!
\param  min     Minimum extent of range.
\param  max     Maximum extent of range.
\return A random integer number in the given range.

Used for: Misc.
*/
    int (*IntRandom)(int min, int max);

/*!
\param  scale   Maximum extent of range.
\return A floating-point number from 0.0 to \b scale.

Used for: Misc.
*/
    LTFLOAT (*RandomScale)(LTFLOAT scale);

/*!
\param pMsg The printf-styled format string to display.  This
parameter can be followed by the usual format parameters.

Only use this for debugging.  This sends an \b STC_BPRINT message
with the string in it.

Used for: Misc.  
*/
    void (*BPrint)(const char *pMsg, ...);

/*!
\param pMsg The printf-styled format string to display.  This
parameter can be followed by the usual format parameters.

Output a TRACE message to the Debug Output window.  Newlines
must be explicitly used.

Used for: Misc.  
*/
    void (*DebugOut)(const char *pMsg, ...);

/*!
\param  pDef (return) Sky definition structure (filled in by function).

\return Always returns \b LT_OK.

Get the sky definition.

Used for: Special FX.
*/
    LTRESULT (*GetSkyDef)(SkyDef *pDef);

/*!
\param  pDef    Sky definition structure.

\return Always returns \b LT_OK.

Set the sky definition.

Used for: Special FX.
*/
    LTRESULT (*SetSkyDef)(SkyDef *pDef);

/*!
\param  hObj    Object to add.
\param  index   Index of sky object. Each object should have a unique index.

\return \b LT_ERROR - Either the server object is NULL, or the index is beyond
        the maximum range allowed (i.e. \em index >= \b MAX_SKYOBJECTS).
\return \b LT_OK - Successful.

Add an object to the sky list.

Used for: Special FX.
*/
    LTRESULT (*AddObjectToSky)(HOBJECT hObj, uint32 index);

/*!
\param  hObj    Object to remove.

\return \b LT_ERROR - The server object is NULL.
\return \b LT_OK - Successful.

Remove an object from the sky list.

Used for: Special FX.
*/
    LTRESULT (*RemoveObjectFromSky)(HOBJECT hObj);

/*!
\param  hObj (return) Global light object to query.

\return \b LT_ERROR - \em hObj is NULL.
\return \b LT_OK - Successful.

Get the global light object.

Used for: Special FX.
*/
    LTRESULT (*GetGlobalLightObject)(HOBJECT *hObj);

/*!
\param  hObj    Light object to set as the global light.

\return Always returns \b LT_OK.

Set the global light object.

Used for: Special FX.
*/
    LTRESULT (*SetGlobalLightObject)(HOBJECT hObj);

};

#endif  //! __ILTSERVER_H__







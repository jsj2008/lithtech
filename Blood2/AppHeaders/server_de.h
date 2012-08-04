
// server_de defines the ServerDE structure, which consists of all the
// server-side DirectEngine functionality.

#ifndef __SERVER_DE_H__
#define __SERVER_DE_H__

	
	#include "basedefs_de.h"
	#include "serverobj_de.h"
	#include "common_de.h"
	#include "CSBase.h"


	DEFINE_HANDLE_TYPE(HCONVAR);
	DEFINE_HANDLE_TYPE(HCLIENTREF);


	// Load world flags.
	#define LOADWORLD_LOADWORLDOBJECTS	(1<<0)	// Load objects from the world file?
	#define LOADWORLD_RUNWORLD			(1<<1)	// Start running world immediately?
	#define LOADWORLD_NORELOADGEOMETRY	(1<<2)	// Don't reload geometry if the world filename
												// is the same as the currently loaded one.

	// ---------------------------------------------------------------------- //
	// Load/Save objects flags.
	// ---------------------------------------------------------------------- //

	#define RESTOREOBJECTS_RESTORETIME	1	// Restore time information?

	#define SAVEOBJECTS_SAVEGAMECONSOLE	1	// Save the game console state?
	#define SAVEOBJECTS_SAVEPORTALS		2	// Save the portal states?


	// ---------------------------------------------------------------------- //
	// Client info flags.
	//
	// CIF_LOCAL and CIF_PLAYBACK are not settable with SetClientInfoFlags.
	// ---------------------------------------------------------------------- //

	#define CIF_LOCAL				(1<<0)	// Client is local (non-network) connection.
	#define CIF_PLAYBACK			(1<<1)	// Virtual client from a demo playback.
	#define CIF_FULLRES				(1<<2)	// Client object is sent with full position resolution.
	#define CIF_SENDCOBJROTATION	(1<<3)	// Client object rotations are sent to client.
	#define CIF_FORCENEXTUPDATE		(1<<4)	// Force the server to update the client on the state
											// of the world as soon as it can.  In multiplayer 
											// games, it doesn't update the clients every frame
											// and this can be used if you send a message to the client
											// that needs to arrive in sync with object changes (like
											// animation changes).
								

	// ---------------------------------------------------------------------- //
	// Object states.
	//
	// OBJSTATE_INACTIVE and OBJSTATE_INACTIVE_TOUCH override the effect of 
	// PingObjects.  OBJSTATE_ACTIVE clears the effect of OBJSTATE_INACTIVE
	// or OBJSTATE_INACTIVE_TOUCH, but the object can be autodeactivated.
	// ---------------------------------------------------------------------- //
							

	#define OBJSTATE_ACTIVE					0	// Normal healthy object.
	#define OBJSTATE_INACTIVE				1	// Inactive (no updates, physics, or touch notifies).
	#define OBJSTATE_INACTIVE_TOUCH			2	// Inactive, but gets touch notifies.
	#define	OBJSTATE_AUTODEACTIVATE_NOW		3	// Autodeactivate now, can reactivate thru PingObjects call.



	// ---------------------------------------------------------------------- //
	// Server state flags.
	// ---------------------------------------------------------------------- //
	
	#define SS_PAUSED		(1<<0)	// Server is paused.
	#define SS_DEMOPLAYBACK	(1<<1)	// We're running in 'demo playback' mode (read only).
	#define SS_CACHING		(1<<2)	// Server can pre-cache files.
	#define SS_LASTFLAG		SS_DEMOPLAYBACK

	
	// ---------------------------------------------------------------------- //
	// Object lists.
	// ---------------------------------------------------------------------- //
	
	typedef struct ObjectLink_t
	{
		HOBJECT		m_hObject;
		struct ObjectLink_t	*m_pNext;
	} ObjectLink;
	
	typedef struct ObjectList_t
	{
		ObjectLink	*m_pFirstLink;
		int			m_nInList;
	} ObjectList;




	// ---------------------------------------------------------------------- //
	// GenericProp -
	// Here's a list of which property types map to which variables:
	// PT_STRING   - m_Vec and m_Color if it's formatted like '1 2 3', m_String, m_Long, m_Float, m_Bool.
	//             - if this is 'true' or 'false', it'll set m_Long, m_Float, and m_Bool to 0 or 1.

	// PT_VECTOR   - m_Vec, m_String ('1 2 3')
	// PT_COLOR    - m_Color, m_String ('1 2 3')
	// PT_REAL     - m_String, m_Long, m_Float, m_Bool
	// PT_FLAGS    - m_String, m_Long, m_Float, m_Bool
	// PT_BOOL     - m_String, m_Long, m_Float, m_Bool
	// PT_LONGINT  - m_String, m_Long, m_Float, m_Bool
	// PT_ROTATION - m_Rotation and m_Vec (with euler rotation).
	// ---------------------------------------------------------------------- //

	#define MAX_GP_STRING_LEN 200

	typedef struct GenericProp_t
	{
		DVector		m_Vec;
		DVector		m_Color;

		char		m_String[MAX_GP_STRING_LEN+1];
		DRotation	m_Rotation;
		long		m_Long;
		float		m_Float;
		DBOOL		m_Bool;
	} GenericProp;




	class PhysicsLT;


	// ---------------------------------------------------------------------- //
	// CServerDE interface. This is your interface to most DirectEngine
	// functionality. 
	// ---------------------------------------------------------------------- //

	class ServerDE : public CSBase
	{	
	friend class CServerMgr;

	protected:

			virtual		~ServerDE() {}

	
	public:		
		// Access to the smaller interfaces.

			CommonLT*	Common()	{return m_pCommonLT;}
			PhysicsLT*	Physics()	{return m_pPhysicsLT;}


		// Main functions.

			// Gives you access to the m_pGameInfo from the StartGameRequest structure.
			// Note: this is a copy of the data and the pointer filled in can be NULL
			// if there is no game info data.
			DRESULT		(*GetGameInfo)(void **ppData, DDWORD *pLen);

			// Returns DNULL if the class doesn't exist.
			HCLASS		(*GetClass)(char *pName);

			// Get a class's static object.  Returns DE_NOTFOUND if the server
			// doesn't have a static object of this class.
			DRESULT		(*GetStaticObject)(HCLASS hClass, HOBJECT *obj);

			// OBSOLETE.  Use PhysicsLT.
			virtual HOBJECT GetWorldObject()=0;

			// Get an object's class.
			HCLASS		(*GetObjectClass)(HOBJECT hObject);

			// Tells if hClass is the class of hTest (or is derived from hTest).
			// If you pass in DNULL for hTest (like if GetClass returns DNULL in 
			// IsKindOf(hClass, GetClass("TestClass"))), it will return FALSE.
			DBOOL		(*IsKindOf)(HCLASS hClass, HCLASS hTest);
			

			// Creates an object using the object's default object type.
			LPBASECLASS	(*CreateObject)(HCLASS hClass, struct ObjectCreateStruct_t *pStruct);

			// Creates an object using string to specify properties.
			LPBASECLASS	(*CreateObjectProps)(HCLASS hClass, struct ObjectCreateStruct_t *pStruct, char *pszProps );

			// Remove the object from the world.  Note: the object won't be removed
			// or deleted until the end of the frame.
			void		(*RemoveObject)(HOBJECT hObject);


			// Returns the current time, in seconds, since the server started up.
			DFLOAT		(*GetTime)();
			
			// Returns the time since the last server shell update.
			DFLOAT		(*GetFrameTime)();


			// OBSOLETE.  Use PhysicsLT functions.
			virtual DRESULT GetGlobalForce(DVector *pVec)=0;
			virtual DRESULT SetGlobalForce(DVector *pVec)=0;

			
			// Server state flags.  (Combination of the SS_ flags above).
			// SetServerFlags returns the new flags (some may not be accepted).
			DDWORD		(*GetServerFlags)();
			DDWORD		(*SetServerFlags)(DDWORD flags);

			// Cache in the given file.  Only call this from ServerShell::CacheFiles.
			// fileType is one of the FT_ defines in de_codes.
			// Returns DE_NOTFOUND if it can't find the file.
			DRESULT		(*CacheFile)(DDWORD fileType, char *pFilename);
			
			
		// Helpers..

			// Use this to iterate over all the polies in the world.  
			// Returns DE_FINISHED when there are no more polies to look at.
			// Returns LT_NOTINITIALIZED if there is no world loaded.
			// Iterate like this:
			// HPOLY hPoly = INVALID_HPOLY;
			// while(pServerDE->GetNextPoly(&hPoly) == LT_OK)
			// {
			//     ... do something with hPoly ...
			// }
			DRESULT		(*GetNextPoly)(HPOLY *hPoly);

			// Use these to time sections of code.  Timing is done in microseconds
			// (1,000,000 counts per second).
			void		(*StartCounter)(struct DCounter_t *pCounter);
			DDWORD		(*EndCounter)(struct DCounter_t *pCounter);
			
			// Returns a random number in the given range.
			DFLOAT		(*Random)(DFLOAT min, DFLOAT max);

			// Returns an integer number in the given range.
			int			(*IntRandom)(int min, int max);

			// Returns a number from 0 to scale.
			DFLOAT		(*RandomScale)(DFLOAT scale);

			// Only use this for debugging.. sends an STC_BPRINT message
			// with the string in it.
			void		(*BPrint)(char *pMsg, ...);

			// A better BPrint.. prints right to the console (doesn't do any network stuff) and 
			// doesn't matter if you're connected yet.
			void		(*CPrint)(char *pMsg, ...);

			// Used to output a TRACE message to the Debug Output window.  Newlines must be explicitly used.
			void	(*DebugOut)( char *pMsg, ... );

			// Get/Set the sky definition.
			DRESULT		(*GetSkyDef)(struct SkyDef_t *pDef);
			DRESULT		(*SetSkyDef)(struct SkyDef_t *pDef);

			// Add/Remove objects from the sky list.
			// Each object should have a unique index.
			DRESULT		(*AddObjectToSky)(HOBJECT hObj, DDWORD index);
			DRESULT		(*RemoveObjectFromSky)(HOBJECT hObj);


		// String functions.  Strings are reference counted objects that cannot
		// be manipulated.  When you create one with FormatString or CreateString,
		// the reference count is 1.  When you copy a string with CopyString, the
		// reference count is incremented.  When you free one with FreeString,
		// it decrements the reference count.. when the reference count goes to
		// zero, it deletes the string.  If you forget to free up any strings, 
		// LithTech will spit out a message telling you about it..

			// In Windows, messageCode comes from your resource DLL.  The messages
			// need to be formatted with %1!s! %2!s! (the number is the argument 
			// number and the !s! says its a string).  You can also use !f! and !d!
			// for floating point and whole number.
			HSTRING		(*FormatString)(int messageCode, ...);
			
			// Copy a string.. much more efficient than CreateString().
			HSTRING		(*CopyString)(HSTRING hString);
			HSTRING		(*CreateString)(char *pString);
			void		(*FreeString)(HSTRING hString);

			DBOOL		(*CompareStrings)(HSTRING hString1, HSTRING hString2);
			DBOOL		(*CompareStringsUpper)(HSTRING hString1, HSTRING hString2);

			// Get the string's data.. you really should only use this for strings
			// that you stored off and want to pass to the engine for a filename 
			// or something..  Most strings could be in some format other than ANSI.
			char*		(*GetStringData)(HSTRING hString);


		// World control.

			// Portal flags are a combination of PORTAL_ flags in de_codes.
			// Returns DE_OK, DE_NOTFOUND, or DE_NOTINWORLD.  This is
			// case sensitive.
			DRESULT		(*GetPortalFlags)(char *pPortalName, DDWORD *pFlags);
			DRESULT		(*SetPortalFlags)(char *pPortalName, DDWORD flags);

			// Intersect a line segment.. (used to be raycast, but line segments are WAY faster).
			// Returns TRUE and fills in pInfo if it hit something.
			DBOOL		(*IntersectSegment)(IntersectQuery *pQuery, IntersectInfo *pInfo);

			// Same as IntersectSegment, except for it casts a ray from pQuery->m_From
			// in the direction of pQuery->m_Dir.
			DBOOL		(*CastRay)(IntersectQuery *pQuery, IntersectInfo *pInfo);
			
			///////////////// NOT IMPLEMENTED YET /////////////////
			// Find out what's at a given point (is it inside the world, outside, what
			// area brushes is it inside..)  You must give back the list to the engine
			// with RelinquishList()!
			ObjectList*	(*GetPointAreas)(DVector *pPoint);

			///////////////// NOT IMPLEMENTED YET /////////////////
			// Find the objects intersecting this box.
			ObjectList*	(*GetBoxIntersecters)(DVector *pMin, DVector *pMax);

			// Get the shade (RGB, 0-255) at the point you specify.
			// Returns DFALSE if the point is outside the world.
			DBOOL		(*GetPointShade)(DVector *pPoint, DVector *pColor);
			
			// Gets all the objects with the given name.
			// Don't forget to relinquish the list you get!
			ObjectList*	(*FindNamedObjects)(char *pName);

			// Find all the objects touching the given sphere.
			// Don't forget to relinquish the list you get!
			// Be VERY careful of using a large radius.  This function can take
			// tons of cycles if the radius is too large.
			ObjectList* (*FindObjectsTouchingSphere)(DVector *pPosition, float radius);

			// Finds objects touching the cone you specify.  This will provide a slight
			// overestimation of the cone you specify.  This can be much faster than
			// FindObjectsTouchingSphere, depending on how small the angle is.
			ObjectList* (*FindObjectsTouchingCone)(DVector *pPos, DVector *pDir, float angle, float dist);
			
			// Any time the engine gives you a list, you must 'give it back' with this,
			// or else you'll have tons of memory leaks.
			void		(*RelinquishList)(ObjectList *pList);

			// Get the bounding box for the current world.
			virtual DRESULT	GetWorldBox(DVector &min, DVector &max)=0;


		// Object list management.

			// Allocate a list (free it with RelinquishList).
			ObjectList*	(*CreateObjectList)();
			
			// Add objects to a list.
			ObjectLink*	(*AddObjectToList)(ObjectList *pList, HOBJECT hObj);
			
			// Remove an object from the list given the handle. 
			// Note: DirectEngine will just do a search over all the objects in the
			//       list for this, so be warned.
			void		(*RemoveObjectFromList)(ObjectList *pList, HOBJECT hObj);



		// OBSOLETE messaging functions.  Use CommonLT::CreateMessage, LMessage::Release, 
		// and ServerDE::SendToX.

			virtual HMESSAGEWRITE	StartSpecialEffectMessage(LPBASECLASS pObject)=0;
			virtual HMESSAGEWRITE	StartInstantSpecialEffectMessage(DVector *pPos)=0;
			virtual HMESSAGEWRITE	StartMessageToObject(LPBASECLASS pSender, HOBJECT hSendTo, DDWORD messageID)=0;
			virtual DRESULT			StartMessageToServer(LPBASECLASS pSender, DDWORD messageID, HMESSAGEWRITE *hWrite)=0;
			virtual HMESSAGEWRITE	StartMessage(HCLIENT hSendTo, DBYTE messageID)=0;
			virtual DRESULT			EndMessage2(HMESSAGEWRITE hMessage, DDWORD flags)=0;
			virtual DRESULT			EndMessage(HMESSAGEWRITE hMessage)=0;	// Just calls EndMessage2 with MESSAGE_GUARANTEED.
			

		// NEW message functions.  The main difference between these and the functions above is
		// that these don't free the message for you so you can send it multiple times.  

			// Use this to setup a special effect message.  If your object has
			// a special effect message, the client shell's SpecialEffectNotify() 
			// will be called.  An object can only have ONE special effect message.
			// If the object already has a special effect message, then it
			// clears out the current one.
			virtual DRESULT	SetObjectSFXMessage(HOBJECT hObject, LMessage &msg)=0;

			// Send a message to an object.  hSender can be NULL.
			virtual DRESULT SendToObject(LMessage &msg, DDWORD msgID, HOBJECT hSender, HOBJECT hSendTo, DDWORD flags)=0;

			// Send the message to the server shell.  hSender can be NULL.
			virtual DRESULT SendToServer(LMessage &msg, DDWORD msgID, HOBJECT hSender, DDWORD flags)=0;

			// Send the message to the client.  If hSendTo is NULL, it sends to them all.
			virtual DRESULT SendToClient(LMessage &msg, DBYTE msgID, HCLIENT hSendTo, DDWORD flags)=0;
		
			// Sends the sfx message to all the clients who can see pos.
			virtual DRESULT	SendSFXMessage(LMessage &msg, DVector &pos, DDWORD flags)=0;

		
		// Client Functions.

			// Attach one client to another.  This means that messages sent to hParent
			// will also go to hClient, and hClient's client object will be reported
			// as hParent's object.  This is useful for demo playback when you want
			// to watch another player's view.
			DRESULT	(*AttachClient)(HCLIENT hParent, HCLIENT hChild);

			// Detach a client from its parent attachment.
			DRESULT	(*DetachClient)(HCLIENT hClient);

			// Iterate over the clients.  Pass in NULL to start.  Returns NULL
			// when there are no more.
			HCLIENT	(*GetNextClient)(HCLIENT hPrev);

			// Iterate over the saved client references.  These come from when
			// a world is saved.  Pass in DNULL to start.  See the Client Functions
			// section for the functions to look at these.
			HCLIENTREF	(*GetNextClientRef)(HCLIENTREF hRef);

			// HCLIENTREFs are used for comparison to HCLIENTs.
			DDWORD	(*GetClientRefInfoFlags)(HCLIENTREF hClient);
			DBOOL	(*GetClientRefName)(HCLIENTREF hClient, char *pName, int maxLen);
			HOBJECT	(*GetClientRefObject)(HCLIENTREF hClient);
			
			// Get a client's (unique) ID.
			DDWORD	(*GetClientID)(HCLIENT hClient);
			
			// Get a client's ping time.
			virtual DRESULT GetClientPing(HCLIENT hClient, float &ping)=0;
			
			// Get a client's name.
			DBOOL	(*GetClientName)(HCLIENT hClient, char *pName, int maxLen);
			
			// A combination of the CIF_ flags above.
			void	(*SetClientInfoFlags)(HCLIENT hClient, DDWORD dwClientFlags );
			DDWORD	(*GetClientInfoFlags)(HCLIENT hClient);

			// User data for HCLIENTs.
			void	(*SetClientUserData)(HCLIENT hClient, void *pData);
			void*	(*GetClientUserData)(HCLIENT hClient);

			// Kick a client off the server.
			// OnRemoveClient will be called.
			void	(*KickClient)(HCLIENT hClient);

			// Set where the client is seeing/hearing from.  This controls what the 
			// server sends the client.  You should set this every frame.
			DRESULT	(*SetClientViewPos)(HCLIENT hClient, DVector *pPos);


		// The 'game console state'.  This is a set of console variables and functions
		// that is used internally by the application DLLs.  The user can't access this
		// at all.  It's very useful for tracking global game variables and triggering
		// them through game objects.

			// Run a string through the game console.  The syntax is the same as any
			// other console string (semicolons separate commands, and new variable names
			// create new HCONVARs).
			void	(*RunGameConString)(char *pString);

			// Sets a console variable.  Creates a new one if it doesn't exist.
			void	(*SetGameConVar)(char *pName, char *pVal);
			
			// Get a handle to a console variable.
			HCONVAR	(*GetGameConVar)(char *pName);
			
			// Get the floating point value of a console variable.  If the hVar is
			// NULL, then it returns 0.
			float	(*GetVarValueFloat)(HCONVAR hVar);

			// Get the string value of a console variable.  If the hVar is
			// NULL, then it returns NULL.
			char*	(*GetVarValueString)(HCONVAR hVar);


		// Helpers.

			// These are used to get the property values from the world file.
			// The property names are case sensitive.  If the property doesn't exist,
			// it will return DE_NOTFOUND.
			DRESULT		(*GetPropString)(char *pPropName, char *pRet, int maxLen);
			DRESULT		(*GetPropVector)(char *pPropName, DVector *pRet);
			DRESULT		(*GetPropColor)(char *pPropName, DVector *pRet);
			DRESULT		(*GetPropReal)(char *pPropName, float *pRet);
			DRESULT		(*GetPropFlags)(char *pPropName, DDWORD *pRet);
			DRESULT		(*GetPropBool)(char *pPropName, DBOOL *pRet);
			DRESULT		(*GetPropLongInt)(char *pPropName, long *pRet);
			DRESULT		(*GetPropRotation)(char *pPropName, DRotation *pRet);
			DRESULT		(*GetPropRotationEuler)(char *pPropName, DVector *pAngles); // pAngles = (pitch, yaw, roll)
			
			// Fills in the GenericProp for the different data types.  For a list
			// of which property types map to which GenericProp variable, see 
			// the GenericProp structure.
			// Note: if the property exists, it always initializes the members in the prop first,
			// so if the GenericProp variable doesn't support that property type, 
			// it'll be zero (or ROT_INIT'd).
			DRESULT		(*GetPropGeneric)(char *pPropName, GenericProp *pProp);


			// See if a property exists.  If so, pPropType is filled in (if it's non-NULL) with
			// on of the PT_ defines.  Returns DE_NOTFOUND if it doesn't exist.
			DRESULT		(*DoesPropExist)(char *pPropName, int *pPropType);

			// Get handles from objects and objects from handles. Note: HandleToObject will return
			// NULL if the given object doesn't reside on this server (it's controlled remotely).
			HOBJECT		(*ObjectToHandle)(LPBASECLASS pObject);
			LPBASECLASS	(*HandleToObject)(HOBJECT hObject);

			// Tests if a command is on for this client.
			DBOOL	(*IsCommandOn)(HCLIENT hClient, int command);

			// OBSOLETE.  Use CommonLT::GetRotationVectors.
			virtual DRESULT GetRotationVectors(DRotation *pRotation, 
				DVector *pUp, DVector *pRight, DVector *pForward)=0;

			// Gets orientation vectors, given an angle vector (x, y, and z rotations).
			void	(*GetRotationVectorsFromMatrix)(DMatrix *pMat, 
				DVector *pUp, DVector *pRight, DVector *pForward);

			// Does a (fast) upper-case string comparison of the 2 strings.
			DBOOL	(*UpperStrcmp)(char *pStr1, char *pStr2);

			// Rotate this rotation around the given axis by a certain amount.
			void	(*RotateAroundAxis)(DRotation *pRotation, DVector *pAxis, float amount);

			// Treat the rotation like Euler angles...
			void	(*EulerRotateX)(DRotation *pRotation, float amount);
			void	(*EulerRotateY)(DRotation *pRotation, float amount);
			void	(*EulerRotateZ)(DRotation *pRotation, float amount);

			// Align a rotation to a normal and spin it.
			// Use pUp to set the frame of reference.
			// If pUp is NULL or the same direction as pVector, then DirectEngine 
			// will kludge an up vector.
			void	(*AlignRotation)(DRotation *pRotation, DVector *pVector, DVector *pUp);

			// OBSOLETE: use CommonLT::SetupEuler.
			virtual DRESULT	SetupEuler(DRotation *pRotation, float pitch, float yaw, float roll)=0;

			// Interpolate between two rotations (with quaternions).
			DRESULT	(*InterpolateRotation)(DRotation *pDest, DRotation *pRot1, DRotation *pRot2, float t);
			
			// Create a transformation matrix from a translation, rotation.
			void	(*SetupTransformationMatrix)( DMatrix *pMat, DVector *pTranslation, DRotation *pRotation );
			// Create a translation matrix for the given translation.
			void	(*SetupTranslationMatrix)( DMatrix *pMat, DVector *pTranslation );
			// Create a rotation matrix for the given rotation.
			void	(*SetupRotationMatrix)(DMatrix *pMat, DRotation *pRot);
			// Create a translation vector from a tranformation matrix.
			void	(*SetupTranslationFromMatrix)( DVector *pTranslation, DMatrix *pMat );
			// Create a rotation for the given tranformation matrix.
			void	(*SetupRotationFromMatrix)(DRotation *pRot, DMatrix *pMat);


			// Rotate around a point.
			void	(*SetupRotationAroundPoint)(DMatrix *pMat, DRotation *pRot, DVector *pPoint);

			// Parse a string for arguments.  Works similar to command line parser.
			// args within quotation marks are considered one arg.
			// Returns the all args after pCommand and before end of string or semi-colon.
			//		pCommand - Beginning of string.
			//		pNewCommandPos - Parse fills this in with the ending position
			//		argBuffer - memory Parse can use as scratchpad
			//		argPointers - Parse fills these pointers in to point to args found.
			//		nArgs - Parse fills in the number of arguments found.
			//		Return - DTRUE, semicolon found, more args in string.  DFALSE, no more args.
			int			(*Parse)(char *pCommand, char **pNewCommandPos, char *argBuffer, char **argPointers, int *nArgs);

		// Sound functions.

			// Play a sound with full control
			// Arguments:
			//		pPlaySoundInfo - sound control structure
			// Returns:
			//		DE_OK if successful.
			//		DE_ERROR on error.
			DRESULT	(*PlaySound)( PlaySoundInfo *pPlaySoundInfo );

			// Get total length in seconds of sound.
			// Arguments:
			//		hSound - Handle to sound.
			//		fDuration - Duration of sound.
			// Returns:
			//		DE_OK if successful.
			//		DE_INVALIDPARAMS if hSound not available or not tracking time.
			DRESULT	(*GetSoundDuration)( HSOUNDDE hSound, DFLOAT *fDuration );

			// IsSoundDone
			// Arguments:
			//		hSound - Handle to sound.
			//		bDone - Indicates sound is completed.
			// Returns:
			//		DE_OK if successful.
			//		DE_INVALIDPARAMS if hSound not available or not tracking time.
			DRESULT	(*IsSoundDone)( HSOUNDDE hSound, DBOOL *bDone );

			// Kill a sound.
			// Arguments:
			//		hSoundHandle - Handle to sound.
			// Returns:
			//		DE_OK if successful.
			//		DE_ERROR on error.
			DRESULT	(*KillSound)( HSOUNDDE hSound );


		// Inter-object link manipulation.

			// If you want to hold on to an HOBJECT and get notification when the
			// object is removed from the world, you can create an inter-object link
			// between the two.  The owner will get MID_LINKBROKEN notification when 
			// the link is being broken (by either the owner or linked object being removed).
			// If a link between the two objects already exists, the function will not
			// create another link and return DE_OK.
			// An object cannot make a link to itself.  This will return DE_ERROR.
			DRESULT	(*CreateInterObjectLink)(HOBJECT hOwner, HOBJECT hLinked);

			// Breaks an inter-object link between the owner and linked object
			// (if one exists..)  You can only break a link from the owner, since
			// the linked object doesn't even know it's linked to the owner..
			// Note: MID_LINKBROKEN will NOT be called.
			void	(*BreakInterObjectLink)(HOBJECT hOwner, HOBJECT hLinked);


		// Object manipulation.

			// Attaches hChild to hParent.  If pNodeName is set, it'll attach hChild to a 
			// specific node on hParent.
			// DE will automatically detach if you remove hParent,
			// but it will NOT automatically detach if you remove hChild.  If you remove the 
			// child without removing the attachment, the results are undefined.
			// Returns DE_OK, DE_NODENOTFOUND, or DE_INVALIDPARAMS.
			DRESULT	(*CreateAttachment)(HOBJECT hParent, HOBJECT hChild, char *pNodeName, 
				DVector *pOffset, DRotation *pRotationOffset, HATTACHMENT *pAttachment);

			// Removes an attachment.  Note: an attachment becomes invalid when you remove the parent
			// so it'll crash if you call it with an attachment with a parent you've removed.
			DRESULT	(*RemoveAttachment)(HATTACHMENT hAttachment);

			// Look for an attachment on the parent.  Returns LT_ERROR, DE_NOTFOUND,
			// or LT_OK if it found it.  hAttachment is set to NULL if there's an error.
			DRESULT	(*FindAttachment)(HOBJECT hParent, HOBJECT hChild, HATTACHMENT *hAttachment);


			// Get/Set object color info (RGBA 0-1).
			// All objects default to (0,0,0,1)
			// For lights, this changes the light's color.
			// For models, this brightens a model's shading.
			DBOOL	(*GetObjectColor)(HOBJECT hObject, float *r, float *g, float *b, float *a);
			DBOOL	(*SetObjectColor)(HOBJECT hObject, float r, float g, float b, float a);

			// Get/Set an object's user-defined flags.
			DDWORD	(*GetObjectUserFlags)(HOBJECT hObj);
			DRESULT	(*SetObjectUserFlags)(HOBJECT hObj, DDWORD flags);

			// OBSOLETE: use the CommonLT ones.
			virtual float	GetObjectMass(HOBJECT hObj)=0;
			virtual void	SetObjectMass(HOBJECT hObj, float mass)=0;

			// OBSOLETE: Use PhysicsLT.
			virtual float	GetForceIgnoreLimit(HOBJECT hObj, float &limit)=0;
			virtual void	SetForceIgnoreLimit(HOBJECT hObj, float limit)=0;
			
			// Use this to iterate thru all the objects in the world.
			// Pass in NULL to start off with.  It'll return NULL when
			// you have iterated through all the objects.
			// This is generally a bad idea, but sometimes you have to.
			HOBJECT	(*GetNextObject)(HOBJECT hObj);

			// Same as GetNextObject, but this iterates over all the inactive objects.
			HOBJECT	(*GetNextInactiveObject)(HOBJECT hObj);
			
			// OBSOLETE: Use CommonLT version.
			short	GetObjectType(HOBJECT hObj)		{DDWORD temp; Common()->GetObjectType(hObj, &temp); return (short)temp;}

			// Get the object's name.
			char*	(*GetObjectName)(HOBJECT hObject);

			// Set an object's friction coefficient.
			virtual DRESULT	SetFrictionCoefficient(HOBJECT hObj, float coeff)=0;
			
			// This is a counter that controls when each object gets its Update()
			// function called. This is set to 0.0001 when an object is created 
			// so by default, Update() gets called right away.
			void	(*SetNextUpdate)(HOBJECT hObj, DFLOAT nextUpdate);

			// Sets the time which the engine will automatically deactivate an object.  If object
			// is currently autodeactivated, then this call will activate the object
			void	(*SetDeactivationTime)(HOBJECT hObj, DFLOAT fDeactivationTime);

			// Activates any objects seen by this object
			void	(*PingObjects)(HOBJECT hObj);

			// Object data accessors.
			void	(*GetObjectPos)(HOBJECT hObj, DVector *pos);
			void	(*SetObjectPos)(HOBJECT hObj, DVector *pos);  // Exactly the same as calling Teleport().

			// Scale the object (only works on models and sprites).
			void	(*ScaleObject)(HOBJECT hObj, DVector *pNewScale);
			DRESULT	(*GetObjectScale)(HOBJECT hObj, DVector *pScale);

			// OBSOLETE.  Use PhysicsLT::MoveObject.
			virtual DRESULT MoveObject(HOBJECT hObj, DVector *pNewPos)=0;

			// Teleports the object directly to the position.
			DRESULT	(*TeleportObject)(HOBJECT hObj, DVector *pNewPos);

			// OBSOLETE.  Use PhysicsLT::GetStandingOn.
			virtual DRESULT	GetStandingOn(HOBJECT hObj, CollisionInfo *pInfo)=0;

			// Get information about the last collision.  Only valid during MID_TOUCHNOTIFY or MID_CRUSH messages.
			DRESULT	(*GetLastCollision)(CollisionInfo *pInfo);

			// Get/Set the object's rotation.
			DRESULT	(*GetObjectRotation)(HOBJECT hObj, DRotation *pRotation);
			DRESULT	(*SetObjectRotation)(HOBJECT hObj, DRotation *pRotation);

			// This rotates the object to the new rotation with possible client side interpolation
			DRESULT	(*RotateObject)(HOBJECT hObj, DRotation *pRotation);

			// Tilt their acceleration to be along the plane they're standing on.
			void	(*TiltToPlane)(HOBJECT hObj, DVector *pNormal);

			// Get/Set the object's flags.
			DDWORD	(*GetObjectFlags)(HOBJECT hObj);
			void	(*SetObjectFlags)(HOBJECT hObj, DDWORD flags);

			// Get/set an object's net flags.  Net flags are a combination of NETFLAG_ defines.
			virtual DRESULT GetNetFlags(HOBJECT hObj, DDWORD &flags)=0;
			virtual DRESULT SetNetFlags(HOBJECT hObj, DDWORD flags)=0;

			// Get/Set the object's state.  State is one of the OBJSTATE_ defines above.
			void	(*SetObjectState)(HOBJECT hObj, int state);
			int		(*GetObjectState)(HOBJECT hObj);

			// OBSOLETE.  Use PhysicsLT.
			virtual void GetObjectDims(HOBJECT hObj, DVector *pNewDims)=0;
			virtual DRESULT	SetObjectDims(HOBJECT hObj, DVector *pNewDims)=0;
			virtual DRESULT	SetObjectDims2(HOBJECT hObj, DVector *pNewDims)=0;

			// OBSOLETE: use CommonLT functions.
			virtual DRESULT	GetVelocity(HOBJECT hObj, DVector *pVel)=0;
			virtual DRESULT	SetVelocity(HOBJECT hObj, DVector *pVel)=0;
			virtual DRESULT	GetAcceleration(HOBJECT hObj, DVector *pAccel)=0;
			virtual DRESULT	SetAcceleration(HOBJECT hObj, DVector *pAccel)=0;

			// Get/Set blocking priority (defaults to 0).
			// See FAQ for a description of how this works.
			void	(*SetBlockingPriority)(HOBJECT hObj, DBYTE pri);
			DBYTE	(*GetBlockingPriority)(HOBJECT hObj);


		// Sprite manipulation.

			// This clips the sprite on the poly.
			// Returns DE_OK or DE_ERROR if not a sprite.
			// Pass in INVALID_HPOLY to un-clip the sprite.
			DRESULT	(*ClipSprite)(HOBJECT hObj, HPOLY hPoly);

		
		// Light manipulation.

			// Get/Set a light's color (RGB, 0.0f to 1.0f).
			// When you create a light, its color defaults to (0,0,0).
			
			// Note: a light's color is snapped to 256 different values, so if
			// you want to do any fancy interpolation or anything, you'll need
			// to store your own, higher precision, color values.
			
			// Note: this just calls GetObjectColor/SetObjectColor.
			void	(*GetLightColor)(HOBJECT hObj, float *r, float *g, float *b);
			void	(*SetLightColor)(HOBJECT hObj, float r, float g, float b);

			// Get/Set a light's radius.
			// When you create a light, its radius defaults to 100.
			float	(*GetLightRadius)(HOBJECT hObj);
			void	(*SetLightRadius)(HOBJECT hObj, float radius);

		
		// Model manipulation.

			// Iterate through the model's nodes.  Returns DE_FINISHED when there are no more.
			// hCurNode = INVALID_MODEL_NODE;
			// while(interface->GetNextModelNode(hModel, hCurNode, &hCurNode) == DE_OK)
			// { ... }
			DRESULT	(*GetNextModelNode)(HOBJECT hObject, HMODELNODE hNode, HMODELNODE *pNext);

			// Get a model node's name.
			DRESULT	(*GetModelNodeName)(HOBJECT hObject, HMODELNODE hNode, char *pName, DDWORD maxLen);

			// Get a model's command string.
			DRESULT	(*GetModelCommandString)(HOBJECT hObj, char *pStr, DDWORD maxLen);

			// Hide/unhide a node on the model (they're all unhidden by default).
			// Returns DE_OK, DE_ERROR, LT_NOCHANGE, or DE_NODENOTFOUND.
			DRESULT	(*SetModelNodeHideStatus)(HOBJECT hObj, char *pNodeName, DBOOL bHidden);
			DRESULT	(*GetModelNodeHideStatus)(HOBJECT hObj, char *pNodeName, /* out */DBOOL *bHidden);
			
			// Get the current (global) transformation for a model node.
			// Returns DFALSE if the node does not exist or if the object
			// you pass in is not a model.
			DBOOL	(*GetModelNodeTransform)(HOBJECT hObj, char *pNodeName,	
				DVector *pPos, DRotation *pRot);

			// Get the absolute world position and rotation of a node on an attached model.
			// Inputs:
			//		hAttachment - Attachment created with CreateAttachment
			//		pChildNodeName - name of node on attached child model
			// Outputs:
			//		pPos - absolute world position of node.
			//		pRot - absolute world rotation of node.
			DRESULT	(*GetAttachedModelNodeTransform)( HATTACHMENT hAttachment, char *pChildNodeName, DVector *pPos, DRotation *pRot);


			// Get an animation index from a model.
			// Returns -1 if the animation doesn't exist (or if the object isn't a model).
			HMODELANIM	(*GetAnimIndex)(HOBJECT hObj, char *pAnimName);

			// If the object is a model, this sets its current animation.
			void	(*SetModelAnimation)(HOBJECT hObj, HMODELANIM hAnim);

			// Returns the animation the model is currently on.  (HMODELANIM)-1 if none.
			HMODELANIM	(*GetModelAnimation)(HOBJECT hObj);

			// Starts the current animation over.
			DRESULT	(*ResetModelAnimation)(HOBJECT hObj);

			// Tells what the playback state of the model is (a combination of the
			// MS_ bits defined in basedefs_de.h).
			DDWORD	(*GetModelPlaybackState)(HOBJECT hObj);

			// Get/Set the looping state of the model.  The default state is TRUE.
			void	(*SetModelLooping)(HOBJECT hObj, DBOOL bLoop);
			DBOOL	(*GetModelLooping)(HOBJECT hObj);

			// Get the model filenames.  You can pass in DNULL for pFilename and pSkinName.
			// Returns DFALSE if the object is not a model.
			// Initializes pFilename or pSkinName to zero length if the model doesn't have 
			// a filename or skin.
			DBOOL	(*GetModelFilenames)(HOBJECT hObj, char *pFilename, int fileBufLen, char *pSkinName, int skinBufLen);

			// Change the object filenames (works on models and sprites.. SetModelFilenames
			// is the same as SetObjectFilenames).  Even if it fails on the server and
			// returns an error, it'll still send a message to the clients.
			DRESULT	(*SetModelFilenames)(HOBJECT hObj, char *pFilename, char *pSkinName);
			DRESULT	(*SetObjectFilenames)(HOBJECT hObj, char *pFilename, char *pSkinName);

			// Get the endpoints of the model's extents for its current animation frame.
			// This is in world space so if you just want the dimensions, take (pMax - pMin) / 2.
			// Returns DE_ERROR if it's not a model.
			DRESULT	(*GetModelFrameBox)(HOBJECT hObj, DVector *pMin, DVector *pMax);

			// OBSOLETE: use CommonLT.
			virtual DRESULT	GetModelAnimUserDims(HOBJECT hObj, DVector *pDims, HMODELANIM hAnim)=0;


		// Container manipulation.
		
			// Get the container's container code (can only be set in the ObjectCreateStruct during creation).
			// Returns DFALSE if the object isn't a container.
			DBOOL	(*GetContainerCode)(HOBJECT hObj, D_WORD *pCode);

			// Gets the list of containers the object is inside.
			// pFlagList must be the same size as pContainerList.
			// Returns the number of elements in pList filled in.
			DDWORD	(*GetObjectContainers)(HOBJECT hObj, 
				HOBJECT *pContainerList, DDWORD *pFlagList, DDWORD maxListSize);

			// Gets the list of objects inside the container.
			// Returns the number of elements in pList filled in.
			DDWORD	(*GetContainedObjects)(HOBJECT hContainer, 
				HOBJECT *pObjectList, DDWORD *pFlagList, DDWORD maxListSize);

			DDWORD (*GetPointContainers)(DVector *pPoint, HOBJECT *pList, DDWORD maxListSize);

		
		// Surface manipulation.

			// OBSOLETE: use CommonLT.
			virtual DRESULT GetPolyTextureFlags(HPOLY hPoly, DDWORD *pFlags)=0;


		// Save game

			// dwParam gets passed into MID_SAVEOBJECT as fData.
			// Flags is a SAVEOBJECTS_ define.
			DRESULT (*SaveObjects)(char *pszSaveFileName, ObjectList *pList, DDWORD dwParam, DDWORD flags);

			// dwParam gets passed into MID_LOADOBJECT as fData.
			// Flags is a combination of RESTOREOBJECTS_ defines.
			DRESULT (*RestoreObjects)( char *pszRestoreFileName, DDWORD dwParam, DDWORD flags );


		// Load world

			// Flags is a combination of the LOADWORLD_ flags.
			DRESULT (*LoadWorld)(char *pszWorldFileName, DDWORD flags);
			DRESULT (*RunWorld)();


		// Network session manipulation

			// Updates the sessions' name.
			DRESULT (*UpdateSessionName)(const char* sName);

			// Gets the sessions' name.
			DRESULT (*GetSessionName)(char* sName, DDWORD dwBufferSize);

			// Send a message to the standalone server app.  Returns LT_NOTFOUND
			// if there is no server app or if it isn't setup to receive the message.
			DRESULT	(*SendToServerApp)(char *pMsg);

			// Gets the tcp/ip address of the main driver if available.
			// If you're hosting a game, hostPort is filled in with the port you're hosting on.
			// If not, it's set to 0xFFFF.
			DRESULT (*GetTcpIpAddress)(char* sAddress, DDWORD dwBufferSize, D_WORD &hostPort);

			// Get a list of files/directories in a directory (pass in "" to start with).
			// The list is a noncircular linked list with DNULL terminating it.
			FileEntry*	(*GetFileList)(char *pDirName);
			
			// Use FreeFileList when you're done with each list you get.
			void		(*FreeFileList)(FileEntry *pHead);

	
	protected:

			CommonLT	*m_pCommonLT;
			PhysicsLT	*m_pPhysicsLT;
	};


	// For backward compatibility.. never use this.
	#define CServerDE ServerDE


#endif  // __SERVER_DE_H__




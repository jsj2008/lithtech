/*!
Base class for ILTClient and ILTServer.  
This is just here for b/w compatibility.
*/

#ifndef __ILTCSBASE_H__
#define __ILTCSBASE_H__


#ifndef __LTBASEDEFS_H__
#include "ltbasedefs.h"
#endif

#ifndef __LTMODULE_H__
#include "ltmodule.h"
#endif

/*!  Forward declarations of base interface classes */

class ILTPhysics;
class ILTModel;
class ILTTransform;
class ILTLightAnim;
class ILTSoundMgr;
class ILTCommon;
class LTObjRef;
class ILTMessage_Read;
class ILTMath;

class ILTCSBase : public IBase {
public:
    interface_version(ILTCSBase, 4);

/*!
\return Pointer to the \b ILTCommon interface.

Accesses the ILTCommon interface.

Used for:   Interface.
*/
    virtual ILTCommon *Common() = 0;

/*!
\return Pointer to the \b ILTPhysics interface.

Accesses the ILTPhysics interface.

Used for:   Interface.
*/
    virtual ILTPhysics *Physics() = 0;

/*!
\return Pointer to the \b ILTTransform interface.

Accesses the ILTTransform interface.

Used for:   Interface.
*/
    virtual ILTTransform *GetTransformLT() = 0;

/*!
\return Pointer to the \b ILTModel interface.

Accesses the ILTModel interface.

Used for:   Interface.
*/
    virtual ILTModel *GetModelLT() = 0;

/*!
\return Pointer to the \b ILTSoundMgr interface.

Accesses the ILTSoundMgr interface.

Used for:   Interface.
*/
    virtual ILTSoundMgr *SoundMgr() = 0;

/*!
Messaging functions.  All of these functions are obsolete.  Use
the LMessage functions. */

	// FIXME: implement for Shogo and Blood2
	//virtual ILTMath *Math() = 0;

public:
                
/*!
\param pMsg \b printf style format string and variable parameter list.

A better BPrint. It prints right to the console (doesn't do any network 
stuff) and doesn't care if you're connected yet.

Used for: Misc.
*/
    virtual void    CPrint(const char *pMsg, ...) = 0;

/*!
\param hObj Handle to container object.
\param pCode Address of variable that gets set.
\return \b false when the object isn't a container.

Get the container's container code (can only be set in the 
\b ObjectCreateStruct during creation).  

Used for: Misc.
*/
    virtual bool  GetContainerCode(HOBJECT hObj, uint16 *pCode) = 0;

/*!
\param hObj The object to query.

\param pContainerList (return) The container list array/buffer.

\param pFlagList (return) The flag list array/buffer.

\param maxListSize The number of elements in list arrays.

\return The number of elements in \b pContainerList filled in.

Get the list of containers the object is inside.  \b pFlagList must
be the same size as \b pContainerList.

Used for: Object.
*/
    virtual uint32 GetObjectContainers(HOBJECT hObj,
        HOBJECT *pContainerList, uint32 maxListSize) = 0;

/*!
\param hContainer The container to query.

\param pObjectList (return) The object list array/buffer.

\param pFlagList (return) The flag list array/buffer.

\param maxListSize The number of elements in list arrays.

\return The number of elements in \b pContainerList filled in.

Get the list of objects inside the container.
Returns the number of elements in \b pObjectList filled in.

Used for: Object.  
*/
    virtual uint32 GetContainedObjects(HOBJECT hContainer,
        HOBJECT *pObjectList, uint32 maxListSize) = 0;

/*!
\param pFilename Name of the file to open.
\param pStream Address of pointer to an \b ILTStream that gets set.

Open a file up.  Pass in the relative filename.
Free the file by calling DStream::Release().

Used for: Misc.
*/
    virtual LTRESULT OpenFile(const char *pFilename, ILTStream **pStream) = 0;


/*!
\param pszSourceFile Name of the file to open.
\param pszDestFile Where to copy the file to.

Copies a file.  This function is useful for copying files out of the
rez file to a temporary file, so you can do special operations on it like
LoadLibrary.

Used for: Misc.
*/
	virtual LTRESULT CopyFile(const char *pszSourceFile, const char *pszDestFile) = 0;


/*!
\param pStream Address of pointer to an \b ILTStream that gets set.
\param nCacheSize Stream memory cache block size

Opens a memory stream.
Free the file by calling DStream::Release().

Used for: Misc.
*/
        virtual LTRESULT OpenMemoryStream(ILTStream **pStream, uint32 nCacheSize = 256) = 0;



/*!
\param nNum Index of blind data.

\param nId ID of blind data (will fail if doesn't match).

\param pData A pointer to the blind data is placed here.

\param nSize Size of blind data is placed here.

\return Returns LTTRUE on success.

Retrieve a pointer to a section of blind data
Get a pointer to a section of blind data.  Blind data
should be freed with FreeBlindObjectData after it has been used.

Used for: Misc.
*/
	virtual LTRESULT GetBlindObjectData(uint32 nNum, uint32 nId, uint8*& pData, uint32& nSize) = 0;

/*!
\param nNum Index of blind data.

\param nId ID of blind data (will fail if doesn't match).

\return Returns LTTRUE on success.

Free the memory used by blind data.  Once all blind data has been freed,
support structures will also be freed.

Used for: Misc.
*/
	virtual LTRESULT FreeBlindObjectData(uint32 nNum, uint32 nId) = 0;


/*!  String functions.  Strings are reference-counted objects that
cannot be manipulated.  When you create one with FormatString() or
CreateString(), the reference count is 1.  When you copy a string with
CopyString(), the reference count is incremented.  When you free one
with FreeString(), it decrements the reference count. When the
reference count goes to zero, it deletes the string.  If you forget to
free up any strings, LithTech will spit out a message telling you
about it.  */



/*!
\param messageCode Index of format string to use.

In Windows, messageCode comes from your resource DLL.  The messages
need to be formatted with %1!s! %2!s! (the number is the argument 
number and the !s! says its a string).  You can also use !f! and !d!
for floating point and whole number.

Used for: Misc.
*/
    virtual HSTRING FormatString(int messageCode, ...) = 0;
        


/*!
\param hString Handle to the string to copy.

Copy a string. This is much more efficient than CreateString().

Used for: Misc.
*/
    virtual HSTRING CopyString(HSTRING hString) = 0;



/*!
\param pString Null terminated character string.

Create a new \b HSTRING from the given ASCII-terminated \b char*.

Used for: Misc.
*/
    virtual HSTRING CreateString(const char *pString) = 0;


/*!
\param hString Handle to the string to free.

Destroy an \b HSTRING.

Used for: Misc.
*/
    virtual void    FreeString(HSTRING hString) = 0;




/*!
\param hString1 Handle of string one.
\param hString2 Handle of string two.

Compare two \b HSTRING strings for equality.

Used for: Misc.
*/
    virtual bool  CompareStrings(HSTRING hString1, HSTRING hString2) = 0;



/*!
\param hString1 Handle of string one.
\param hString2 Handle of string two.

Compare two \b HSTRING strings for equality.

Used for: Misc.
*/
    virtual bool  CompareStringsUpper(HSTRING hString1, 
        HSTRING hString2) = 0;



/*!
\param hString Handle of string.

Get the string's data. You should only use this for strings
that you stored off and want to pass to the engine for a filename 
or something.  Most strings could be in some format other than ANSI.

Used for: Misc.
*/
    virtual const char* GetStringData(HSTRING hString) = 0;




/*!
\param hVar Handle of console variable.
\return The floating point value of a console variable, 
or 0 if the \b hVar is \b NULL. 

Used for: Misc.
*/
    virtual float GetVarValueFloat(HCONSOLEVAR hVar) = 0;



/*!
\param hVar Handle of console variable.
\return The string value of a console variable, or \b NULL if the \b hVar is \b NULL.

Used for: Misc.
*/
    virtual const char* GetVarValueString(HCONSOLEVAR hVar) = 0;



/*!
\return The current time, in seconds, since the shell started.

Used for: Misc.
*/
    virtual LTFLOAT GetTime() = 0;
        


/*!
\return The time since the last shell update.

Used for: Misc.
*/
    virtual LTFLOAT GetFrameTime() = 0;


/*!

  The worlds are offset at processing time to be centered around the origin. This offset
  can be found by calling this function. This is useful for mapping position values back
  to the original level values for debugging and error reporting

\param vVector The vector offset from the original level. To get the orginal value use the equation orignal = position + offset
\return Always returns LT_OK-

  Used for: Misc.
*/

	virtual LTRESULT GetSourceWorldOffset(LTVector& vVector) = 0;

/*!
\param hObj ???

Remove the object from the world.  

\note The object will not be removed or deleted until the end of the frame.

Used for: Object.
*/
    virtual LTRESULT RemoveObject(HOBJECT hObj) = 0;

/*!
\param pName Name of the lightgroup

Function to generate the appropriate lightgroup ID given the name of the lightgroup.
\note	Lightgroup names are case-sensitive.

Used for: LightGroup control
*/
	virtual LTRESULT GetLightGroupID(const char *pName, uint32 *pResult) const = 0;

/*!
\param pName Name of the occluder

Function to generate the appropriate occluder ID given the name of the occluder.
\note	Occluder names are case-insensitive.

Used for: Occluder control
*/
	virtual LTRESULT GetOccluderID(const char *pName, uint32 *pResult) const = 0;

/*!
\param pName Name of the texture effect
\param nStage The stage that is desired

Function to generate the appropriate ID for a texture effect given the name of the
effect and the stage that the parameter belongs to.
\note	Lightgroup names are case-sensitive.

Used for: Texture Effect Variable manipulation
*/
	virtual LTRESULT GetTextureEffectVarID(const char *pName, uint32 nStage, uint32 *pResult) const = 0;

/*!
\param hObj Object to link to
\param pRef LTObjRef object to add to the object's reference list

This function adds a reference link to the object's list, to be called when the
object gets deleted.
\note	This function is called internally by the LTObjRef class.  Do not call this function externally!
*/
	virtual LTRESULT LinkObjRef(HOBJECT hObj, LTObjRef *pRef) = 0;


/*!
\param sAddress (return) buffer to receive local IP address string in dot notation format (NNN.NNN.NNN.NNN) followed by a null byte.
\param dwBufferSize  size of sAddress buffer (this should be at least 17 bytes to ensure room for the dot notation string and a null byte)  if there is not enough room the string will be truncated to (dwBufferSize - 1) bytes.
\param hostPort (return) filled in with host port
\return \b LT_INVALIDPARAMS if \b sAddress is null, or \b LT_NOTINITIALIZED if there is no main driver; otherwise returns \b LT_OK.

Get the TCP/IP address of the main driver, if it is available.  If you
are hosting a game, \b hostPort is filled in with the port you are
hosting on.  If you are not hosting, it is set to 0xFFFF.

\note  dwBufferSize must be greater than zero bytes and less than or equal to the size of the space allocated for sAddress.   This is not checked by the current code in the engine, and it will fault if you pass in zero bytes.

Used for: Networking.
*/
    LTRESULT (*GetTcpIpAddress)(char* sAddress, uint32 dwBufferSize,
        uint16 &hostPort);

/*!
\param  pMsg	user data to send
\param  pAddr   IP address to target; can be numeric or host name.
\param  nPort   Port to use
\return \b LT_NOTINITIALIZED if there is no TCP/IP driver or the driver's socket is uninitialized, or \b LT_ERROR if the address cannot be built; otherwise, returns \b LT_OK.

Send a message using the TCP/IP protocol.

\b sAddr is a string containing an IP addresses to check.
If the first character of the address is a numeral (between '0' and '9') it is treated
as a numeric IP address; otherwise it's treated as a host name.

Used for: Networking. 
*/
    virtual LTRESULT SendTo(ILTMessage_Read *pMsg, const char *pAddr, uint16 nPort) = 0;

/*!
\param  pAddr   IP address to target; can be numeric or host name.
\param  nPort   Port to use
\param	pPingID Resulting Ping ID
\return \b LT_NOTINITIALIZED if there is no TCP/IP driver or the driver's socket is uninitialized, or \b LT_ERROR if the address cannot be built; otherwise, returns \b LT_OK.

Start a ping request to a TCP/IP address

Used for: Networking
*/
    virtual LTRESULT StartPing(const char *pAddr, uint16 nPort, uint32 *pPingID) = 0;

/*!
\param	nPingID		Ping ID to query
\param	pStatus		Ping status result
\param  pLatency	Latency result if *pStatus == PING_STATUS_SUCCESS

\return \b LT_NOTFOUND if that ping ID is unknown, LT_OK otherwise.
*/
    virtual LTRESULT GetPingStatus(uint32 nPingID, uint32 *pStatus, uint32 *pLatency) = 0;

/*!
\param	nPingID	Ping ID to remove

\return \b LT_NOTFOUND if that ping ID is unknown, LT_OK otherwise.
*/
    virtual LTRESULT RemovePing(uint32 nPingID) = 0;

/*!
Gets the rotation of the specified object

Used for: Object.
*/
    virtual LTRESULT GetObjectRotation(HLOCALOBJ hObj, LTRotation *pRotation) = 0;

/*!
Gets the position of the specified object

Used for: Object.
*/
    virtual LTRESULT GetObjectPos(HLOCALOBJ hObj, LTVector *pPos) = 0;

/*!
\param hObj Model instance
\param pAnimName name of animation
\return -1 if the animation doesn't exist or if the object isn't a
model.

Get an animation index from a model.
The animation index is a handle to an animation associated with a model.

Used for: Models and Animation.  */
    virtual HMODELANIM  GetAnimIndex(HOBJECT hObj, const char *pAnimName) = 0;



/*!
\param hObj The model whose animation you want to set
\param hAnim The animation you want to play

Set the current animation of the object, if it is a model.

Used for: Models and Animation.
*/
    virtual void    SetModelAnimation(HOBJECT hObj, HMODELANIM hAnim) = 0;


/*!
\param hObj ???
\return The animation the model is currently on, or (\b HMODELANIM)-1 if
none.

Used for: Models and Animation.  */
    virtual HMODELANIM  GetModelAnimation(HOBJECT hObj) = 0;



/*!
\param hObj ???
\param bLoop ???

Set the looping state of the model.  The default state is \b TRUE.

Used for: Models and Animation.
*/
    virtual void    SetModelLooping(HOBJECT hObj, bool bLoop) = 0;


/*!
\param hObj ???
\return ???

Get the looping state of the model.  The default state is \b TRUE.

Used for: Models and Animation.
*/
    virtual bool  GetModelLooping(HOBJECT hObj) = 0;



/*!
\param hObj ???
\return ???

Start the current animation over.

Used for: Models and Animation.
*/
    virtual LTRESULT    ResetModelAnimation(HOBJECT hObj) = 0;



/*!
\param hObj ???
\return ???

Tell what the playback state of the model is (a combination of the
\b MS_ bits defined in basedefs_de.h).

Used for: Models and Animation.
*/
    virtual uint32  GetModelPlaybackState(HOBJECT hObj) = 0;

/*!
\param pPoint Location to test.
\param pList Array of \b HOBJECT
\param maxListSize Length of the pList array.
\return The number of containers filled in.

Find out what containers contain the given point.

Used for: Misc.
*/
    virtual uint32  GetPointContainers(const LTVector *pPoint, HOBJECT *pList, 
        uint32 maxListSize) = 0;

};


#endif //! __ILTCSBASE_H__




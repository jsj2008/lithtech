
#ifndef __ILTCOMMON_H__
#define __ILTCOMMON_H__


#ifndef __LTBASEDEFS_H__
#include "ltbasedefs.h"
#endif

#ifndef __LTMODULE_H__
#include "ltmodule.h"
#endif

struct CompVector;
class CompRot;

class ILTTransform;
class ILTModel;
class CLTClient;
struct ObjectCreateStruct;
class ILTMessage_Write;

/*!
Define a holder to get this interface like this:
\code
define_holder_to_instance(ILTCommon, your_var, Server);
define_holder_to_instance(ILTCommon, your_var, Client);
\endcode
*/

class ILTCommon : public IBase {
public:
    interface_version(ILTCommon, 4);

/*!  Helpers.  */

public:

/*!
Tell what type of object this is.

Used for: Object.
*/
    virtual LTRESULT GetObjectType(HOBJECT hObj, uint32 *type) = 0;

/*!
\param vec The vector to compress.
\param CompV (return) The CompVector struct to pack vec into.

\return \b LT_OK.

Compress an \b LTVector down to 9 bytes.
\see UncompressVector()
 
Used for: Misc.  
*/
    virtual LTRESULT CompressVector(const LTVector& vec, CompVector& CompV)=0;

/*!
\param CompV The CompVector struct to be unpacked.
\param vec (return) The \b LTVector to hold the uncompressed data.

\return \b LT_OK.

Uncompress an \b LTVector which was compressed using \b CompressVector.
\see CompressVector()
 
Used for: Misc.  
*/
    virtual LTRESULT UncompressVector(const CompVector& CompV, LTVector& vec)=0;

/*!
\param rot The rotation to compress.
\param CompR (return) The CompRot struct to pack rot into.

\return \b LT_OK.

Compress an \b LTRotation down to 6 (or sometimes 3) bytes.
\see UncompressRotation()
 
Used for: Misc.  
*/
    virtual LTRESULT	CompressRotation(const LTRotation& rot, CompRot& CompR)=0;

/*!
\param CompV The CompRot struct to be unpacked.
\param rot (return) The LTRotation to hold the uncompressed data.

\return \b LT_OK.

Uncompress an \b LTRotation which was compressed using \b CompressRotation.
\see CompressRotation()

Used for: Misc.  
*/
 
/*!
\return \b LT_FINISHED when done. 

Initialize the \b ConParse and call.  Fills in \b m_Args and \b
m_nArgs with each set of tokens.

\code       
Sample loop:
    parse.Init(pStr);
    while (pEngine->Parse2(&parse) == LT_OK)
    {
        ... process args ...
    }
\endcode

Used for: Misc.  
*/
    virtual LTRESULT Parse(ConParse *pParse) = 0;

/*!
Get the user dimensions of a model animation (the
dimensions set in ModelEdit).

Used for: Models and Animation.  
*/
    virtual LTRESULT GetModelAnimUserDims(HOBJECT hObject, 
        LTVector *pDims, HMODELANIM hAnim) = 0;


    virtual LTRESULT GetRotationVectors(LTRotation &rot, LTVector &up, 
        LTVector &right, LTVector &forward) = 0;
    virtual LTRESULT SetupEuler(LTRotation &rot, float pitch, float yaw, 
        float roll) = 0;

/*!
Start a message.  Make sure to properly deal with this object as a reference counted object.

Used for: Messaging.
*/
    virtual LTRESULT CreateMessage(ILTMessage_Write* &pMsg) = 0;

/*!  Geometry stuff. */

public:

/*!
\param pPoint The location to test.
\return \b LT_NOTINWORLD when no world is loaded, \b LT_INSIDE when the point is
in the world, or \b LT_OUTSIDE when the point is not in the world.

Used for: Misc.  
*/
    virtual LTRESULT GetPointStatus(const LTVector *pPoint) = 0;

/*!
\param pPoint The location to test.
\param pColor The lighting at the given point.
\return \b LT_NOTINWORLD when the point is outside the world.

Get the shade (RGB, 0-255) at the point you specify.

Used for: Misc.
*/
    virtual LTRESULT GetPointShade(const LTVector *pPoint, LTVector *pColor) = 0;

/*!
\param hPoly The handle of the polygon.
\param pFlags Pointer to dest where flags are written.

\return \b LT_ERROR when no world is loaded or \b hPoly is
invalid; otherwise, returns \b LT_OK.

Get the texture flags from a poly.
 
Used for: Misc.  */
    virtual LTRESULT GetPolyTextureFlags(HPOLY hPoly, uint32 *pFlags)=0;

/*!
\param hPoly The handle of the polygon.
\param pPlane Pointer to dest where the polygon's plane is written.

\return \b LT_ERROR when no world is loaded or \b hPoly is
invalid; otherwise, returns \b LT_OK.

Get the plane for a poly.
 
Used for: Misc.  */
    virtual LTRESULT GetPolyPlane(HPOLY hPoly, LTPlane *pPlane)=0;

    virtual LTRESULT	UncompressRotation(const CompRot& CompR, LTRotation& rot)=0;
/*!
Change the filenames of a model or a sprite.  
\param hObj The object to change the resource of
\param eType The type of resource that is being changed
\param nIndex The index of the resource being changed
\param pszResource The name of the resource that it should be changed to

Used for: Object.  
*/
    virtual LTRESULT SetObjectResource(	HOBJECT hObj, EObjectResource eType, 
										uint32 nIndex, const char* pszResource) = 0;
/*!
Changes all the appropriate files to match found in the create struct
\param hObj The object to change the filenames of
\param pStruct The create struct which holds the appropriate names

Used for: Object.  
*/
    virtual LTRESULT SetObjectFilenames(	HOBJECT hObj, ObjectCreateStruct* pStruct) = 0;


/*!
Get the object's flag parameters.  Specify what flag type you're
accessing and the value.
 
Used for: Object.  
*/
    virtual LTRESULT GetObjectFlags(
        const HOBJECT hObj,
        const ObjFlagType flagType,
        uint32 &dwFlags) = 0;

/*!
Set the object's flag parameters.  Specify what flag type you're
accessing, the value, and the mask of flags to modify.  Use FLAGMASK_ALL
to modify all flags.
 
Used for: Object.  
*/
    virtual LTRESULT SetObjectFlags(
        HOBJECT hObj,
        const ObjFlagType flagType,
        uint32 dwFlags,
		uint32 dwMask) = 0;

/*!  Attachments.  */

public:

/*!
\return \b LT_NOTINITIALIZED if either of the objects doesn't exist.

Get the parent and child of an attachment.

Used for: Object.  
*/
    virtual LTRESULT GetAttachmentObjects(HATTACHMENT hAttachment, 
        HOBJECT &hParent, HOBJECT &hChild) = 0;

/*!
\param  inList The list to be filled in.
\param  inListSize How large \b inList is.
\param  outListSize How many elements in \b inList were filled in.
\param outNumAttachments How many attachments \b hObj has.
This number can be larger that \b outListSize if \b inList can't hold all
the attachments.

Gets the objects attached to this object.  

Used for: Object.  
*/
    virtual LTRESULT GetAttachments(HLOCALOBJ hObj, HLOCALOBJ *inList, 
        uint32 inListSize, uint32 &outListSize, 
        uint32 &outNumAttachments) = 0;

/*!
Basically ILTCommon::GetAttachmentTransform(WORLD) *
ModelLT::GetNodeTransform(LOCAL).

Used for: Object.  
*/
    virtual LTRESULT GetAttachedModelNodeTransform(
        HATTACHMENT hAttachment, HMODELNODE hNode, 
        LTransform &transform) = 0;

/*!
Get the attachment transform (based off the parent object position,
model socket, and attachment offset).  If \b bWorldSpace is \b DTRUE,
then the transform is in world space.

Used for: Object.  
*/
    virtual LTRESULT GetAttachmentTransform(HATTACHMENT hAttachment, 
        LTransform &transform, bool bWorldSpace) = 0;

/*!
Basically ILTCommon::GetAttachmentTransform(WORLD) *
ModelLT::GetSocketTransform(LOCAL).

Used for: Object.  
*/
    virtual LTRESULT GetAttachedModelSocketTransform(
        HATTACHMENT hAttachment, HMODELSOCKET hSocket, 
        LTransform &transform) = 0;

/*!
\param hObject Object for which attachments should be enumerated.
\param typeFilter \b AttachmentType enum describing whether to count all, server-only, or client-only attachments.
\param dwAttachCount [return] Number of attachments to \b hObject.
\return \b LT_INVALIDPARAMS if \b hObject is NULL.

Fetch the number of attachments that a given object has associated with it.

\note The \b typeFilter parameter is ignored when the server version of this function is called.
The server only knows about server-made attachments so it always counts those.

Used for: Object.  
*/
    virtual LTRESULT NumAttachments(HLOCALOBJ hObject, uint32 &dwAttachCount) = 0;

};


/*! 
Class to store compressed rotation.
Includes an overriden equality operator.
\see CompressRotation()
\see UncompressRotation().
*/
class CompRot
{
public:
#ifndef DOXYGEN_SHOULD_SKIP_THIS
	bool	operator==(CompRot &testRot)
	{
		if(m_Bytes[0] != testRot.m_Bytes[0] || 
			m_Bytes[1] != testRot.m_Bytes[1] || 
			m_Bytes[2] != testRot.m_Bytes[2])
		{
			return false;
		}

		if(m_Bytes[0] >= 0)
		{
			if(m_Bytes[3] != testRot.m_Bytes[3] || 
				m_Bytes[4] != testRot.m_Bytes[4] || 
				m_Bytes[5] != testRot.m_Bytes[5])
			{
				return false;
			}
		}

		return true;
	}
#endif /* DOXYGEN_SHOULD_SKIP_THIS */

	char	m_Bytes[6];
};

/*! 
Class to store compressed vector.

\see CompressRotation()
\see UncompressRotation()
\note Only the lower 16 bits of \b dwB and \b dwC are packed by engine networking code.
*/
struct CompVector
{
	float	fA;
	uint32	dwB;
	uint32	dwC;
	uint8	order;
};

/*! 
Class to store compressed position vector.
*/
class CompWorldPos
{
public:
	bool	operator==(CompWorldPos &testPos)
	{
		return m_Pos[0]==testPos.m_Pos[0] && m_Pos[1]==testPos.m_Pos[1] && 
			m_Pos[2]==testPos.m_Pos[2] && m_Extra==testPos.m_Extra;
	}

	static uint32 DataSize() { return sizeof(uint16) * 3 + sizeof(char); }
	
	uint16	m_Pos[3];
	char	m_Extra;
};


#endif //! __ILTCOMMON_H__

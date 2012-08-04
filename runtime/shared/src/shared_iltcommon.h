#ifndef __SHARED_ILTCOMMON_H__
#define __SHARED_ILTCOMMON_H__

#ifndef __ILTCOMMON_H__
#include "iltcommon.h"
#endif


//
//CSharedLTCommon implements some of the ILTCommon functions which 
//are the same on client and server.
//

class CLTCommonShared : public ILTCommon {
// Internal functions (not exposed in API).
public:

    // Used by GetAttachedModelNodeTransform and GetAttachedModelSocketTransform since
    // the code in them is nearly identical.
    LTRESULT GetAttachedNodeOrSocketTransform(HATTACHMENT hAttachment,
        uint32 iObject, LTransform &tFinal, bool bNode);

public:
    //internal use function for use in the shared implementation.
    virtual ILTModel *GetILTModel() = 0;

protected:
	// Call this function to get a pointer to the object's flags specified by nFlagType
	virtual const uint32 *GetObjectFlagPointer(const HOBJECT hObj, ObjFlagType nFlagType);

public:
    //ILTCommon functions that have a shared implementation.
    virtual LTRESULT GetObjectFlags(const HOBJECT hObj, const ObjFlagType flagType, uint32 &dwFlags);
    virtual LTRESULT SetObjectFlags(HOBJECT hObj, const ObjFlagType flagType, uint32 dwFlags, uint32 dwMask);
    virtual LTRESULT Parse(ConParse *pParse);
    virtual LTRESULT GetObjectType(HOBJECT hObj, uint32 *type);
    virtual LTRESULT GetModelAnimUserDims(HOBJECT hObject, LTVector *pDims, HMODELANIM hAnim);
    virtual LTRESULT GetRotationVectors(LTRotation &rot, LTVector &up, LTVector &right, LTVector &forward);
    virtual LTRESULT SetupEuler(LTRotation &rot, float pitch, float yaw, float roll);
    virtual LTRESULT GetAttachmentTransform(HATTACHMENT hAttachment, LTransform &transform, bool bWorldSpace);
    virtual LTRESULT GetAttachedModelNodeTransform(HATTACHMENT hAttachment, HMODELNODE hNode, LTransform &transform);
    virtual LTRESULT GetAttachedModelSocketTransform(HATTACHMENT hAttachment, HMODELSOCKET hSocket, LTransform &transform);
    virtual LTRESULT GetAttachments(HLOCALOBJ pObj, HLOCALOBJ *inList, uint32 inListSize, uint32 &outListSize, uint32 &outNumAttachments);

    virtual LTRESULT CompressVector(const LTVector& vec, CompVector& CompV);
    virtual LTRESULT UncompressVector(const CompVector& CompV, LTVector& vec);
    virtual LTRESULT CompressRotation(const LTRotation& rot, CompRot& CompR);
    virtual LTRESULT UncompressRotation(const CompRot& CompR, LTRotation& rot);
};



#endif


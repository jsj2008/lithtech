#include "bdefs.h"

#include "shared_iltcommon.h"
#include "de_objects.h"
#include "impl_common.h"
#include "objectmgr.h"
#include "iltmodel.h"
#include "geomroutines.h"



//------------------------------------------------------------------
//------------------------------------------------------------------
// Holders and their headers.
//------------------------------------------------------------------
//------------------------------------------------------------------

//ILTTransform game interface.
#include "ilttransform.h"
static ILTTransform *ilt_transform;
define_holder(ILTTransform, ilt_transform);

//the ICompress interface.
#include "compress.h"
static ICompress *compress;
define_holder(ICompress, compress);

// Helper function for Get/SetObjectFlags
const uint32 *CLTCommonShared::GetObjectFlagPointer(const HOBJECT hObj, ObjFlagType nFlagType)
{
	switch (nFlagType)
	{
		case OFT_Flags :
			return &hObj->m_Flags;
		case OFT_Flags2 :
			return &hObj->m_Flags2;
		case OFT_User :
			return &hObj->m_UserFlags;
		case OFT_Client :
			return &hObj->cd.m_ClientFlags;
		default :
			return 0;
	}
}

LTRESULT CLTCommonShared::GetObjectFlags(const HOBJECT hObj, const ObjFlagType flagType, uint32 &dwFlags) {
    FN_NAME(CLTCommonShared::GetObjectFlags);
    
    dwFlags = 0;

    CHECK_PARAMS2(hObj);

	const uint32 *pFlags = GetObjectFlagPointer(hObj, flagType);

	if (!pFlags)
		return LT_ERROR;

	dwFlags = *pFlags;

    return LT_OK;
}

LTRESULT CLTCommonShared::SetObjectFlags(HOBJECT hObj, const ObjFlagType flagType, uint32 dwFlags, uint32 nMask) {
    FN_NAME(CLTCommonShared::SetObjectFlags);

    CHECK_PARAMS2(hObj);

	uint32 *pFlags = const_cast<uint32*>(GetObjectFlagPointer(hObj, flagType));

	if (!pFlags)
		return LT_ERROR;

	*pFlags = (*pFlags & ~nMask) | (dwFlags & nMask);

    return LT_OK;
}

LTRESULT CLTCommonShared::Parse(ConParse *pParse) {
    return pParse->Parse() ? LT_OK : LT_FINISHED;
}

LTRESULT CLTCommonShared::GetObjectType(HOBJECT hObj, uint32 *type) {
    CHECK_PARAMS(hObj, CommonLT::GetObjectType);
    *type = HObjToLTObj(hObj)->m_ObjectType;
    return LT_OK;
}

LTRESULT CLTCommonShared::GetModelAnimUserDims(HOBJECT pObject, LTVector *pDims, HMODELANIM hAnim) 
{
    ModelInstance *pInstance;

    if (!pObject || pObject->m_ObjectType != OT_MODEL) {
        RETURN_ERROR(1, CommonLT::GetModelAnimUserDims, LT_INVALIDPARAMS);
    }

    pInstance = ToModel(pObject);

	Model *pModelDB = pInstance->GetModelDB();

	if (!pModelDB) {
		RETURN_ERROR_PARAM(1, CommonLT::GetModelAnimUserDims, LT_MISSINGMODELFILE, "invalid model db");
	}

    if (hAnim >= pModelDB->NumAnims()) {
        RETURN_ERROR_PARAM(1, CommonLT::GetModelAnimUserDims, LT_INVALIDPARAMS, "invalid hAnim");
    }

    *pDims = pModelDB->GetAnimInfo(hAnim)->m_vDims;
    return LT_OK;
}


LTRESULT CLTCommonShared::GetRotationVectors(LTRotation &rot, LTVector &up, LTVector &right, LTVector &forward) 
{
    gr_GetRotationVectors(&rot, &right, &up, &forward);
	return LT_OK;
}

LTRESULT CLTCommonShared::SetupEuler(LTRotation &rot, float pitch, float yaw, float roll) 
{
    gr_EulerToRotation(pitch, yaw, roll, &rot);
    return LT_OK;
}

float quat_length( float q[4] )
{

	float dot =  (q[QX] * q[QX] + q[QY] * q[QY]  + q[QZ] * q[QZ]) ;
	float  a  = (q[QW] * q[QW] );

	a = a * a ;
	dot = dot * dot ;
	return sqrtf( ( dot + a ) ) ;
}




    

LTRESULT CLTCommonShared::GetAttachmentTransform(HATTACHMENT hAttachment, 
												 LTransform &transform, /* out */
												 bool bWorldSpace) 
{
    FN_NAME(CLTCommonShared::GetAttachmentTransform);

    LTObject *pParent, *pChild;
    LTRESULT dResult;
    Attachment *pAttachment;
    LTransform tBase;

    pAttachment = (Attachment*)hAttachment;
    if (!pAttachment) {
        ERR(1, LT_INVALIDPARAMS);
    }

    dResult = GetAttachmentObjects(hAttachment, pParent, pChild);
    if (dResult != LT_OK) {
        return dResult;
    }
 
    // Setup the base transform.
    if (pParent->m_ObjectType == OT_MODEL && pAttachment->m_iSocket != -1) 
	{
        dResult = GetILTModel()->GetSocketTransform(pParent, pAttachment->m_iSocket, tBase, bWorldSpace == LTTRUE);
        if (dResult != LT_OK)
            return dResult;	
		
		transform.m_Scale = tBase.m_Scale;
    }
    else // attachment is connected to a world model.
	{
		tBase.m_Scale.Init(1.0f, 1.0f, 1.0f);
        if (bWorldSpace) {
            tBase.m_Pos = pParent->GetPos();
            tBase.m_Rot = pParent->m_Rotation;
        }
        else {
            tBase.m_Pos.Init();
            tBase.m_Rot.Init();
        }

		transform.m_Scale =  pChild->m_Scale ;
    }

	// same as ilt_transform->multiply but without that pesky virtual call overhead.
	// basic formula : R2 R1, R2 t1 + t2)
 	// (R', T', S' ) = (( R1 * R2 ) , ( R1 * P2 ) + P1 , S1 * S2
	transform.m_Rot = tBase.m_Rot * pAttachment->m_Offset.m_Rot ;
	quat_RotVec( &transform.m_Pos.x, tBase.m_Rot.m_Quat, &pAttachment->m_Offset.m_Pos.x);
	transform.m_Pos += tBase.m_Pos ;

    return LT_OK;
}

LTRESULT CLTCommonShared::GetAttachedNodeOrSocketTransform(HATTACHMENT hAttachment, uint32 iObject, LTransform &tFinal, bool bNode) {
    LTRESULT dResult;
    LTransform tAttachment, tChild;
    LTObject *pParent, *pChild;

    dResult = GetAttachmentObjects(hAttachment, pParent, pChild);
    if (dResult != LT_OK) {
        return dResult;
    }

    dResult = GetAttachmentTransform(hAttachment, tAttachment, LTTRUE);
    if (dResult != LT_OK) {
        return dResult;
    }

    if (bNode) {
        dResult = GetILTModel()->GetNodeTransform(pChild, iObject, tChild, LTFALSE);
    }
    else {
        dResult = GetILTModel()->GetSocketTransform(pChild, iObject, tChild, LTFALSE);
    }

    if (dResult != LT_OK) {
        return dResult;
    }

    // Now multiply them..
    ilt_transform->Multiply(tFinal, tAttachment, tChild);    
    return LT_OK;
}


LTRESULT CLTCommonShared::GetAttachedModelNodeTransform(HATTACHMENT hAttachment, HMODELNODE hNode, LTransform &tFinal) {
    return GetAttachedNodeOrSocketTransform(hAttachment, hNode, tFinal, LTTRUE);
}

LTRESULT CLTCommonShared::GetAttachedModelSocketTransform(HATTACHMENT hAttachment, HMODELSOCKET hSocket, LTransform &tFinal) {
    return GetAttachedNodeOrSocketTransform(hAttachment, hSocket, tFinal, LTFALSE);
}

LTRESULT CLTCommonShared::GetAttachments(HLOCALOBJ pObj, HLOCALOBJ *inList, uint32 inListSize, 
    uint32 &outListSize, uint32 &outNumAttachments)
{
    FN_NAME(CLTCommonShared::GetAttachments);
    Attachment *pCur;
    LTObject *pParent, *pChild;
    LTRESULT dResult;

    outListSize = outNumAttachments = 0;
    
    if (!pObj || !inList) {
        ERR(1, LT_INVALIDPARAMS);
    }

    pCur = pObj->m_Attachments;
    while (pCur) {
        dResult = GetAttachmentObjects((HATTACHMENT)pCur, pParent, pChild);
        if (dResult == LT_OK) {
            if (outListSize < inListSize) {
                inList[outListSize] = pChild;
                outListSize++;
            }

            outNumAttachments++;
        }

        pCur = pCur->m_pNext;
    }

    return LT_OK;
}

LTRESULT CLTCommonShared::CompressVector(const LTVector& vec, CompVector& CompV)
{
	compress->EncodeCompressVector(&CompV, &vec);
	return LT_OK;
}

LTRESULT CLTCommonShared::UncompressVector(const CompVector& CompV, LTVector& vec)
{
	compress->DecodeCompressVector(&vec, &CompV);
	return LT_OK;
}

LTRESULT CLTCommonShared::CompressRotation(const LTRotation& rot, CompRot& CompR)
{
	compress->EncodeCompressRotation(&rot, &CompR);
	return LT_OK;
}

LTRESULT CLTCommonShared::UncompressRotation(const CompRot& CompR, LTRotation& rot)
{
	// copy CompR's bytes into local array because
	// ic_UncompressRotation may overwrite it
	char	bytes[6];
	memcpy(bytes, CompR.m_Bytes, sizeof(bytes));
	compress->UncompressRotation(bytes, &rot);
	return LT_OK;
}



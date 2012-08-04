// ----------------------------------------------------------------------- //
//
// MODULE  : CServerMark.cpp
//
// PURPOSE : CServerMark implementation
//
// CREATED : 1/15/99
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "ServerMark.h"
#include "WeaponMgr.h"
#include "ilttransform.h"

BEGIN_CLASS(CServerMark)
END_CLASS_DEFAULT_FLAGS(CServerMark, BaseClass, NULL, NULL, CF_HIDDEN)

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CServerMark::CServerMark()
//
//	PURPOSE:	Initialize object
//
// ----------------------------------------------------------------------- //

CServerMark::CServerMark() : BaseClass(OT_SPRITE)
{
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CServerMark::~CServerMark()
//
//	PURPOSE:	Destructor
//
// ----------------------------------------------------------------------- //

CServerMark::~CServerMark()
{
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CServerMark::Setup()
//
//	PURPOSE:	Attach the mark to its parent
//
// ----------------------------------------------------------------------- //

void CServerMark::Setup(CLIENTWEAPONFX & theStruct)
{
	LTransform globalTransform, parentTransform, localTransform;
    ILTTransform *pTransformLT;
    LTVector vParentPos, vOffset;
    LTRotation rParentRot, rRot;
    LTRotation rOffset;

	if (!g_pWeaponMgr ||
        !g_pLTServer ||
		!theStruct.hObj ||
        !(pTransformLT = g_pLTServer->GetTransformLT()))
	{
		return;
	}

	// Attach the mark to the parent object...

	// Figure out what the rotation we want is.
    rOffset.Init();
    g_pLTServer->AlignRotation(&rRot, &(theStruct.vSurfaceNormal), LTNULL);


	// MD
	// Ok, now we have the transform in global space but attachments are specified in
	// local space (so they can move as the object moves and rotates).

	// Set the global LTransform.
	pTransformLT->Set(globalTransform, theStruct.vPos, rRot);

	// Get the object's transform.
    g_pLTServer->GetObjectPos(theStruct.hObj, &vParentPos);
    g_pLTServer->GetObjectRotation(theStruct.hObj, &rParentRot);
	pTransformLT->Set(parentTransform, vParentPos, rParentRot);

	// Get the offset.
	pTransformLT->Difference(localTransform, globalTransform, parentTransform);
	pTransformLT->Get(localTransform, vOffset, rOffset);


	HATTACHMENT hAttachment;
    LTRESULT dRes = g_pLTServer->CreateAttachment(theStruct.hObj, m_hObject, LTNULL,
											     &vOffset, &rOffset, &hAttachment);
    if (dRes != LT_OK)
	{
        g_pLTServer->RemoveObject(m_hObject);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CServerMark::EngineMessageFn
//
//	PURPOSE:	Handle engine messages
//
// ----------------------------------------------------------------------- //

uint32 CServerMark::EngineMessageFn(uint32 messageID, void *pData, LTFLOAT fData)
{
	switch(messageID)
	{
		case MID_PARENTATTACHMENTREMOVED :
		{
            g_pLTServer->RemoveObject(m_hObject);
		}
		break;

		default : break;
	}

	return BaseClass::EngineMessageFn(messageID, pData, fData);
}
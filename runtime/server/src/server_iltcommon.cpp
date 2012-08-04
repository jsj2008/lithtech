#include "bdefs.h"

#include "shared_iltcommon.h"
#include "de_objects.h"
#include "moveobject.h"
#include "s_object.h"
#include "packetdefs.h"
#include "objectmgr.h"
#include "s_client.h"
#include "server_extradata.h"
#include "s_net.h"
#include "servermgr.h"
#include "impl_common.h"
#include "serverde_impl.h"
#include "ltobjectcreate.h"

#include "ltmessage_server.h"

//------------------------------------------------------------------
//------------------------------------------------------------------
// Holders and their headers.
//------------------------------------------------------------------
//------------------------------------------------------------------

//ILTModel game interface server instance.
#include "iltmodel.h"
static ILTModel *ilt_model_server;
define_holder_to_instance(ILTModel, ilt_model_server, Server);

//ILTTransform game interface.
#include "ilttransform.h"
static ILTTransform *ilt_transform;
define_holder(ILTTransform, ilt_transform);

//IWorld holder
#include "world_server_bsp.h"
static IWorldServerBSP *world_bsp_server;
define_holder(IWorldServerBSP, world_bsp_server);



//
//Server side implementation class for ILTCommon.
//
class CLTCommonServer : public CLTCommonShared {
public:
    declare_interface(CLTCommonServer);

    //internal use function for use in the shared implementation.
    virtual ILTModel *GetILTModel();

    //We have overwritten the shared implementations of these functions.
protected:
	virtual const uint32 *GetObjectFlagPointer(const HOBJECT hObj, ObjFlagType nFlagType);

public:
    LTRESULT SetObjectFlags(HOBJECT pObj, const ObjFlagType nFlagType, uint32 nFlags, uint32 nMask);
	LTRESULT SetObjectResource(	HOBJECT pObj, EObjectResource eType, uint32 nIndex, const char* pszResource);
	LTRESULT SetObjectFilenames(HOBJECT pObj, ObjectCreateStruct* pStruct);
    LTRESULT GetPointStatus(const LTVector *pPoint);
    LTRESULT GetPointShade(const LTVector *pPoint, LTVector *pColor);
    LTRESULT GetPolyTextureFlags(HPOLY hPoly, uint32 *pFlags);
    LTRESULT GetPolyPlane(HPOLY hPoly, LTPlane *pPlane);
    LTRESULT CreateMessage(ILTMessage_Write* &pMsg);
    LTRESULT GetAttachmentObjects(HATTACHMENT hAttachment, HOBJECT &hParent, HOBJECT &hChild);
	LTRESULT NumAttachments(HLOCALOBJ hObject, uint32 &dwAttachCount);
};

//instantiate our implementation class
instantiate_interface(CLTCommonServer, ILTCommon, Server);


ILTModel *CLTCommonServer::GetILTModel() {
    return ilt_model_server;
}

const uint32 *CLTCommonServer::GetObjectFlagPointer(const HOBJECT hObj, ObjFlagType nFlagType)
{
	// Thou shalt not modify client flags on the server
	if (nFlagType == OFT_Client)
		return 0;

	return CLTCommonShared::GetObjectFlagPointer(hObj, nFlagType);
}

LTRESULT CLTCommonServer::SetObjectFlags(HOBJECT hObj, const ObjFlagType nFlagType, uint32 nFlags, uint32 nMask) {
    FN_NAME(CLTCommonServer::SetObjectFlags);

    CHECK_PARAMS2(hObj);

	// Get the flags pointer
	uint32 *pFlags = const_cast<uint32*>(GetObjectFlagPointer(hObj, nFlagType));
	if (!pFlags)
		return LT_ERROR;

	// Jump out if nothing's going to change
	if ((*pFlags & nMask) == (nFlags & nMask))
		return LT_OK;

	// Find out what's going to change
	uint32 nChangingFlags = (nMask & (*pFlags ^ nFlags));

	// Take care of anything that has to happen before the flags get modified...
	switch (nFlagType)
	{
		case OFT_Flags :
        {
            // They changed a FLAGS_.
            hObj->m_InternalFlags |= IFLAG_APPLYPHYSICS;
            // If we're going to nonsolid, get rid of anything standing on us.
            if ((nChangingFlags & FLAG_SOLID) && (nFlags & FLAG_SOLID)) 
			{
                DetachObjectStanding(hObj);
            }
		}
		break;
	}

	// Change the flags
	*pFlags = (*pFlags & ~nMask) | (nFlags & nMask);

	// Take care of anything that has to happen after the flags get modified...
	switch (nFlagType)
	{
		case OFT_Flags :
        {
            // Only tell clients if it changes a flag relevant to them.
            if (nChangingFlags & CLIENT_FLAGMASK)
            {
                SetObjectChangeFlags(hObj, CF_FLAGS);
            }

            // If they turned on real world model physics, retransform the world model.
            if (HasWorldModel(hObj) && (nChangingFlags & FLAG_BOXPHYSICS) && !(nFlags & FLAG_BOXPHYSICS))
            {
                RetransformWorldModel(ToWorldModel(hObj));
            }

            sm_UpdateInBspStatus(hObj);
        }
		break;
		case OFT_Flags2 :
		case OFT_User :
		{
            SetObjectChangeFlags(hObj, CF_FLAGS);
		}
		break;
    }

    return LT_OK;
}

LTRESULT CLTCommonServer::SetObjectResource(HOBJECT pObj, EObjectResource eType, uint32 nIndex, const char* pszResource)
{
    FN_NAME(CLTCommonServer::SetObjectResource);
    CHECK_PARAMS2(pObj && (pObj->m_ObjectType == OT_MODEL || pObj->m_ObjectType == OT_SPRITE));

	ObjectCreateStruct ocs;
	ocs.Clear();

	if(pObj->m_ObjectType == OT_MODEL)
	{
		if(eType == eObjectResource_ObjectFile)
		{
			LTStrCpy(ocs.m_Filename, pszResource, MAX_CS_FILENAME_LEN);
		}
		else if(eType == eObjectResource_Texture)
		{
			assert(nIndex < MAX_MODEL_TEXTURES);
			LTStrCpy(ocs.m_SkinNames[nIndex], pszResource, MAX_CS_FILENAME_LEN);
		}
		else if(eType == eObjectResource_RenderStyle)
		{
			assert(nIndex  < MAX_MODEL_RENDERSTYLES);
			LTStrCpy(ocs.m_RenderStyleNames[nIndex], pszResource, MAX_CS_FILENAME_LEN);
		}
		else if(eType == eObjectResource_ChildModel )
		{
			assert(nIndex < MAX_CHILD_MODELS );
			LTStrCpy(ocs.m_Filenames[nIndex+1], pszResource, MAX_CS_FILENAME_LEN);
		}

		return SetObjectFilenames(pObj, &ocs);
	}
	else
	{
		assert(eType == eObjectResource_ObjectFile);
		LTStrCpy(ocs.m_Filename, pszResource, MAX_CS_FILENAME_LEN);

		return SetObjectFilenames(pObj, &ocs);
	}

	//return the error code
	return LT_OK;
}

LTRESULT CLTCommonServer::SetObjectFilenames(HOBJECT pObj, ObjectCreateStruct *pStruct) 
{
    FN_NAME(CLTCommonServer::SetObjectFilenames);

    LTRESULT dResult;
    Model *pOldModel, *pNewModel;
    Attachment *pAttachment;
    uint32 newSocketIndex;
    LTBOOL bNodesChanged;
    ExtraDataBackup backup;


    CHECK_PARAMS2(pStruct && pObj &&
        (pObj->m_ObjectType == OT_MODEL || pObj->m_ObjectType == OT_SPRITE));

    pOldModel = LTNULL;

    if (pObj->m_ObjectType == OT_MODEL)
	{
        pOldModel = ToModel(pObj)->GetModelDB();
		// we do this so as to keep the file around for a bit
		// because this function binds and unbinds the model to the ltobject... 
		if(pOldModel) 
			pOldModel->AddRef(); 
	}

	//setup the model now, we don't need to terminate, since it will properly clean up any resources
	//that are changed
    dResult = sm_InitExtraData(pObj, pStruct);
    if (dResult != LT_OK)
    {
		// Remove reference added above.
		if( pOldModel )
			pOldModel->Release( );

        return dResult;
    }

	// Flag that we are changing the filenames.
	SetObjectChangeFlags(pObj, CF_FILENAMES);

/*
	// Tell all the clients about it.
	CPacket_Write cChangePacket;
	cChangePacket.Writeuint8(SMSG_CHANGEOBJECTFILENAMES);
	cChangePacket.Writeuint16(pObj->m_ObjectID);

    if (pObj->m_ObjectType == OT_MODEL)
    {
        sm_WriteChangedModelFiles(pObj, cChangePacket, pStruct);
    }
    else if (pObj->m_ObjectType == OT_SPRITE)
    {
		//write out the header info
		cChangePacket.Writeuint8((uint8)eObjectResource_ObjectFile);
		cChangePacket.Writeuint8((uint8)0);

	    if (pObj->sd->m_pFile)
		{
			cChangePacket.Writeuint16((uint16)pObj->sd->m_pFile->m_FileID);
		}
		else
		{
			cChangePacket.Writeuint16(0xFFFF);
		}
		cChangePacket.Writeuint8(0xFF);
    }

	CPacket_Read cChangePacket_Send(cChangePacket);

	// Send a message to all clients that know about this object.
	LTLink *pListHead, *pCur;
	pListHead = &g_pServerMgr->m_Clients.m_Head;
	for(pCur=pListHead->m_pNext; pCur != pListHead; pCur=pCur->m_pNext)
	{
		Client *pClient = (Client*)pCur->m_pData;

		if(pClient->m_ObjInfos[pObj->m_ObjectID].m_ChangeFlags & CF_SENTINFO)
		{
			SendToClient(pClient, cChangePacket_Send, LTFALSE);
		}
	}
*/
    // If we've changed models, then reorder the attachment socket node bindings.
    if (pOldModel) 
	{
        bNodesChanged = LTFALSE;
        pNewModel = ToModel(pObj)->GetModelDB();
	
		// if we have the same model... skip re-arranging attachements.
		if( pOldModel == pNewModel )
		{
			pOldModel->Release();	
			return dResult ;
		}
		
		// rearrange...
        pAttachment = pObj->m_Attachments;
        while (pAttachment) 
		{
            if (pAttachment->m_iSocket < pOldModel->NumSockets()) 
			{
                if (pNewModel->FindSocket( pOldModel->GetSocket(pAttachment->m_iSocket)->GetName(), 
										   &newSocketIndex))
                {
                    pAttachment->m_iSocket = newSocketIndex;
                }
            }
            // Look for it in the node list if it's not in the socket list
            else if (pAttachment->m_iSocket < (pOldModel->NumSockets() + pOldModel->NumNodes())) 
			{
                if (pNewModel->FindNode(
								pOldModel->GetNode(pAttachment->m_iSocket - pOldModel->NumSockets())->GetName(), 
								&newSocketIndex))
                {
                    pAttachment->m_iSocket = newSocketIndex + pNewModel->NumSockets();
                }
            }

            pAttachment = pAttachment->m_pNext;
        }

        if (bNodesChanged) 
		{
			SetObjectChangeFlags(pObj, CF_ATTACHMENTS);
        }

		// we're done with the old-model.
		pOldModel->Release();
    }
	
    return dResult;
}

LTRESULT CLTCommonServer::GetPointStatus(const LTVector *pPoint) {
    if (ic_IsPointInsideWorld(world_bsp_server->ServerTree(), pPoint))
    {
        return LT_INSIDE;
    }
    else
    {
        return LT_OUTSIDE;
    }
}

LTRESULT CLTCommonServer::GetPointShade(const LTVector *pPoint, LTVector *pColor) {
    return si_GetPointShade(pPoint, pColor);
}


LTRESULT CLTCommonServer::GetPolyTextureFlags(HPOLY hPoly, uint32 *pFlags) {
    WorldPoly *pPoly;

    *pFlags = 0;


    pPoly = world_bsp_server->GetPolyFromHPoly(hPoly);
    if (pPoly)
    {
        *pFlags = pPoly->GetSurface()->m_TextureFlags;
        return LT_OK;
    }
    else
    {
        RETURN_ERROR(2, CommonLT::GetPolyTextureFlags, LT_ERROR);
    }
}

LTRESULT CLTCommonServer::GetPolyPlane(HPOLY hPoly, LTPlane *pPlane) {
    WorldPoly *pPoly;

    pPoly = world_bsp_server->GetPolyFromHPoly(hPoly);
    if (pPoly)
    {
        *pPlane = *pPoly->GetPlane();
        return LT_OK;
    }
    else
    {
        RETURN_ERROR(2, CommonLT::GetPolyPlane, LT_ERROR);
    }
}

LTRESULT CLTCommonServer::CreateMessage(ILTMessage_Write* &pMsg) {
	pMsg = static_cast<ILTMessage_Write*>(CLTMessage_Write_Server::Allocate_Server());
    return LT_OK;
}

LTRESULT CLTCommonServer::GetAttachmentObjects(HATTACHMENT hAttachment, HOBJECT &hParent, HOBJECT &hChild) {
    FN_NAME(CLTCommonServer::GetAttachmentObjects);
    Attachment *pAttachment;


    pAttachment = (Attachment*)hAttachment;
    if (!pAttachment)
    {
        ERR(1, LT_INVALIDPARAMS);
    }

    hParent = sm_FindObject(pAttachment->m_nParentID);
    hChild	= sm_FindObject(pAttachment->m_nChildID);

    if (!hParent || !hChild)
    {
        ERR(1, LT_NOTINITIALIZED);
    }

    return LT_OK;
}

LTRESULT CLTCommonServer::NumAttachments(HLOCALOBJ hObject, uint32 &dwAttachCount) 
{
	FN_NAME(CLTCommonServer::NumAttachments);
	dwAttachCount = 0;

	if (!hObject)
	{
		ERR(1, LT_INVALIDPARAMS);
	}

	Attachment * pAttachment = hObject->m_Attachments;
	while (pAttachment)
	{
		++dwAttachCount;
		pAttachment = pAttachment->m_pNext;
	}

	return LT_OK;
}


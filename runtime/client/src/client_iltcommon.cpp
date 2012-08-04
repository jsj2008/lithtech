#include "bdefs.h"

#include "shared_iltcommon.h"
#include "setupobject.h"
#include "de_objects.h"
#include "clientmgr.h"
#include "impl_common.h"
#include "ltobjectcreate.h"

#include "ltmessage_client.h"

//------------------------------------------------------------------
//------------------------------------------------------------------
// Holders and their headers.
//------------------------------------------------------------------
//------------------------------------------------------------------

//IWorldClientBSP holder
#include "world_client_bsp.h"
static IWorldClientBSP *world_bsp_client;
define_holder(IWorldClientBSP, world_bsp_client);

//IWorldSharedBSP holder
#include "world_shared_bsp.h"
static IWorldSharedBSP *world_bsp_shared;
define_holder(IWorldSharedBSP, world_bsp_shared);

//ILTModel game interface client instance.
#include "iltmodel.h"
static ILTModel *ilt_model_client;
define_holder_to_instance(ILTModel, ilt_model_client, Client);

//ILTTransform game interface.
#include "ilttransform.h"
static ILTTransform *ilt_transform;
define_holder(ILTTransform, ilt_transform);

//
//Our client side implementation class for ILTCommon.
//

class CLTCommonClient : public CLTCommonShared {
public:
    declare_interface(CLTCommonClient);

    //internal use function for use in the shared implementation.
    virtual ILTModel *GetILTModel();

    //overrides from ILTCommon
    LTRESULT SetObjectResource(HOBJECT pObj, EObjectResource eType, uint32 nIndex, const char* pszResource);
	LTRESULT SetObjectFilenames(HOBJECT pObj, ObjectCreateStruct *pStruct);
	LTRESULT GetPolyTextureFlags(HPOLY hPoly, uint32 *pFlags);
	LTRESULT GetPolyPlane(HPOLY hPoly, LTPlane *pPlane);
    LTRESULT GetPointStatus(const LTVector *pPoint);
    LTRESULT GetPointShade(const LTVector *pPoint, LTVector *pColor);
    LTRESULT GetAttachmentObjects(HATTACHMENT hAttachment, HOBJECT &hParent, HOBJECT &hChild);
    LTRESULT CreateMessage(ILTMessage_Write* &pMsg);
	LTRESULT NumAttachments(HLOCALOBJ hObject, uint32 &dwAttachCount) { dwAttachCount = 0; return LT_OK; }
};

//instantiate our implemenation class with the name 'Client'
instantiate_interface(CLTCommonClient, ILTCommon, Client);

ILTModel *CLTCommonClient::GetILTModel() {
    return ilt_model_client;
}

LTRESULT CLTCommonClient::SetObjectFilenames(HOBJECT pObj, ObjectCreateStruct *pStruct) 
{
    FN_NAME(CLTCommonClient::SetObjectFilenames);
    InternalObjectSetup objectSetup;
    uint32 i;
    


    CHECK_PARAMS2(pStruct && pObj &&
        (pObj->m_ObjectType == OT_MODEL || pObj->m_ObjectType == OT_SPRITE));

    // Unload un-used model textures..
    if (pObj->m_ObjectType == OT_MODEL)
    {
		//This function is currently not being called because it would constantly remove textures
		//and ruin caching. This leads to some wasted texture memory during the course of a level,
		//but does help prevent hitching
        //g_pClientMgr->FreeUnusedModelTextures(pObj);
    }

    // Setup the InternalObjectSetup.
    objectSetup.m_pSetup = pStruct;
    objectSetup.m_Filename[0].m_pFilename = pStruct->m_Filename;
    objectSetup.m_Filename[0].m_FileType = FILE_ANYFILE;
    for (i=0; i < MAX_MODEL_TEXTURES; i++)
    {
        objectSetup.m_SkinNames[i].m_pFilename = pStruct->m_SkinNames[i];
        objectSetup.m_SkinNames[i].m_FileType = FILE_ANYFILE;
    }

    for(i=0; i < MAX_MODEL_RENDERSTYLES; i++)
    {
        objectSetup.m_RenderStyleNames[i].m_pFilename = pStruct->m_RenderStyleNames[i];
        objectSetup.m_RenderStyleNames[i].m_FileType = FILE_ANYFILE;
    }

    // Init 
    return so_ExtraInit(pObj, &objectSetup, LTFALSE);
}

LTRESULT CLTCommonClient::SetObjectResource(HOBJECT pObj, EObjectResource eType, uint32 nIndex, const char* pszResource)
{
    FN_NAME(CLTCommonClient::SetObjectResource);
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


LTRESULT CLTCommonClient::GetPolyTextureFlags(HPOLY hPoly, uint32 *pFlags) {
    WorldPoly *pPoly;

    if (!g_pClientMgr->m_pCurShell)
    {
        RETURN_ERROR(2, CommonLT::GetPolyTextureFlags, LT_NOTINITIALIZED);
    }

    *pFlags = 0;

    pPoly = world_bsp_client->GetPolyFromHPoly(hPoly);
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

LTRESULT CLTCommonClient::GetPolyPlane(HPOLY hPoly, LTPlane *pPlane) {
    WorldPoly *pPoly;

    if (!g_pClientMgr->m_pCurShell)
    {
        RETURN_ERROR(2, CommonLT::GetPolyPlane, LT_NOTINITIALIZED);
    }

    pPoly = world_bsp_client->GetPolyFromHPoly(hPoly);
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

LTRESULT CLTCommonClient::GetPointStatus(const LTVector *pPoint) {
    WorldTree *pWorldTree = world_bsp_client->ClientTree();

    if (!pWorldTree)
        return LT_NOTINWORLD;

    return ic_IsPointInsideWorld(pWorldTree, pPoint) ? LT_INSIDE : LT_OUTSIDE;
}

LTRESULT CLTCommonClient::GetPointShade(const LTVector *pPoint, LTVector *pColor) {
    LTRGBColor rgb;

    if (!pPoint || !pColor)
        RETURN_ERROR(1, CLTClient::GetPointShade, LT_INVALIDPARAMS);

    if (!g_pClientMgr->m_pCurShell)
        return LT_NOTINWORLD;

    w_DoLightLookup(world_bsp_shared->LightTable(), pPoint, &rgb);

    pColor->x = rgb.rgb.r;
    pColor->y = rgb.rgb.g;
    pColor->z = rgb.rgb.b;
    return LT_OK;
}

LTRESULT CLTCommonClient::GetAttachmentObjects(HATTACHMENT hAttachment, HOBJECT &hParent, HOBJECT &hChild) {
    FN_NAME(CLTCommonClient::GetAttachmentObjects);
    Attachment *pAttachment;

    pAttachment = (Attachment *)hAttachment;
    if (!pAttachment)
    {
        ERR(1, LT_INVALIDPARAMS);
    }

	hChild	= g_pClientMgr->FindObject(pAttachment->m_nChildID);
    hParent = g_pClientMgr->FindObject(pAttachment->m_nParentID);
    
    if (!hParent || !hChild)
    {
        ERR(1, LT_NOTINITIALIZED);
    }

    return LT_OK;
}

LTRESULT CLTCommonClient::CreateMessage(ILTMessage_Write* &pMsg) {
	pMsg = static_cast<ILTMessage_Write*>(CLTMessage_Write_Client::Allocate_Client());

    return LT_OK;
}



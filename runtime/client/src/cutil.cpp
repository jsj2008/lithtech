//------------------------------------------------------------------
//
//  FILE      : C_Util.cpp
//
//  PURPOSE   : Implements the CClientMgr utility functions.
//
//  CREATED   : January 15, 1997
//
//  COPYRIGHT : LithTech Inc,. 1996-2000
//
//------------------------------------------------------------------

// Includes....
#include "bdefs.h"

#include "clientmgr.h"
#include "moveobject.h"
#include "cmoveabstract.h"
#include "animtracker.h"
#include "clientshell.h"
#include "render.h"
#include "sprite.h"


//------------------------------------------------------------------
//------------------------------------------------------------------
// Holders and their headers.
//------------------------------------------------------------------
//------------------------------------------------------------------

//IWorldClientBSP holder
#include "world_client_bsp.h"
static IWorldClientBSP *world_bsp_client;
define_holder(IWorldClientBSP, world_bsp_client);

//IClientFileMgr
#include "client_filemgr.h"
static IClientFileMgr *client_file_mgr;
define_holder(IClientFileMgr, client_file_mgr);

//IClientShell game client shell object.
#include "iclientshell.h"
static IClientShell *i_client_shell;
define_holder(IClientShell, i_client_shell);





// ------------------------------------------------------------------ //

/*
Sound* CClientMgr::GetSound(sound_type soundType, FileRef *pFileRef, uint32 dwSoundBufferFlags)
{
    Sound *pSound;
    FileIdentifier *pIdent;
    char *pFileName;

    if (!GetSoundMgr()->m_bValid || !GetSoundMgr()->m_bEnabled)
        return LTNULL;

    // See if the file even exists.
    pIdent = client_file_mgr->GetFileIdentifier(pFileRef, TYPECODE_SOUND);
    if (!pIdent)
    {
        pFileName = client_file_mgr->GetFilename(pFileRef);

        if (g_DebugLevel >= 2)
        {
            dsi_PrintToConsole("Missing sound file %s", pFileName);
        }

        return LTNULL;
    }

    // If the file ident already points to a sound, it has already been created.  Streaming sounds don't
    // save their pointers into the ident, so a buffer is created for each instance...
    if (!(dwSoundBufferFlags & SOUNDBUFFER_STREAM) && pIdent->m_pData)
    {
        ((Sound *)pIdent->m_pData)->m_dwSoundBufferFlags |= SOUNDBUFFER_TAG;
        return (Sound*)pIdent->m_pData;
    }

    // Ok, load it up.
    pSound = sound_Create(soundType, pIdent, dwSoundBufferFlags);

    // Add it to the sound buffer lists, if it isn't a streaming file.  Since the streaming file can't duplicate
    // one buffer, it must have a sound buffer per instance.  So there's no reason to put it in the sound buffer
    // list, since it's in the instance list.  When the streaming sound instance is destroyed, it destroys
    // the sound buffer...
    if (pSound && !(dwSoundBufferFlags & SOUNDBUFFER_STREAM))
    {
        // Add it to the list of sounds...
        dl_AddTail(&m_Sounds, &pSound->m_Node, pSound);
    }

    return pSound;
}
*/

LTRESULT CClientMgr::SetupError(LTRESULT theError, ...) {
    va_list marker;
    LTRESULT dResult;

    va_start(marker, theError);
    dResult = dsi_SetupMessage(m_ErrorString, sizeof(m_ErrorString)-1, theError, marker);
    va_end(marker);

    return dResult;
}


LTRESULT CClientMgr::ProcessError(LTRESULT theError) {
    if (theError & ERROR_DISCONNECT)
    {
        if (i_client_shell != NULL) {
            i_client_shell->OnEvent(LTEVENT_DISCONNECT, theError & ~(ERROR_DISCONNECT | ERROR_SHUTDOWN));
        }

        if (m_pCurShell)
        {
            delete m_pCurShell;
            m_pCurShell = LTNULL;
        }

        dsi_DoErrorMessage(m_ErrorString);
        //dsi_SetConsoleUp(LTTRUE);

        return LT_OK;
    }
    else if (theError & ERROR_SHUTDOWN)
    {
        if (i_client_shell != NULL) {
            i_client_shell->OnEvent(LTEVENT_DISCONNECT, theError & ~(ERROR_DISCONNECT | ERROR_SHUTDOWN));
        }

        r_TermRender(2, false);
 
        dsi_OnClientShutdown(m_ErrorString);

        return LT_ERROR;
    }
    else
    {
        return LT_OK;
    }
}


void CClientMgr::ForwardMessagesToScript()
{
    int i;

    if (i_client_shell != NULL) {
        for (i=0; i < dsi_NumKeyDowns(); i++) {
            i_client_shell->OnKeyDown(dsi_GetKeyDown(i), dsi_GetKeyDownRep(i));
        }

        for (i=0; i < dsi_NumKeyUps(); i++) {
            i_client_shell->OnKeyUp(dsi_GetKeyUp(i));
        }

        dsi_ClearKeyDowns();
        dsi_ClearKeyUps();
    }
}


void CClientMgr::ForwardCommandChanges(int32 *pChanges, int32 nChanges)
{
    int32 i;

    if (i_client_shell != NULL) {
        for (i=0; i < nChanges; i++) {
            if (m_Commands[m_iCurInputSlot][pChanges[i]]) {
                i_client_shell->OnCommandOn(pChanges[i]);
            }
            else {
                i_client_shell->OnCommandOff(pChanges[i]);
            }
        }
    }
}


void CClientMgr::UpdateFrameRate()
{
    m_FramerateTracker.Add(1.0f);
    m_FramerateTracker.Update(m_FrameTime);
}


LTRESULT CClientMgr::AddSharedTexture3(FileIdentifier *pIdent, SharedTexture* &pTexture) {
    pTexture = LTNULL;

    if (pIdent->m_pData) {
        pTexture = (SharedTexture*)pIdent->m_pData; }
    else {
        pTexture = m_SharedTextureBank.Allocate();
        memset(pTexture, 0, sizeof(*pTexture));
        dl_AddHead(&m_SharedTextures, &pTexture->m_Link, pTexture);
        pTexture->m_pFile = pIdent;
        pIdent->m_pData = pTexture; }

    // Bind it to the renderer.
    if (!pTexture->m_pRenderData) 
	{
        r_BindTexture(pTexture, LTFALSE);
	}

    return LT_OK;
}


LTRESULT CClientMgr::AddSharedTexture2(FileRef *pRef, SharedTexture* &pTexture) {

    FileIdentifier *pIdent;

    pTexture = LTNULL;

    pIdent = client_file_mgr->GetFileIdentifier(pRef, TYPECODE_TEXTURE);
    if (pIdent)
    {
        return AddSharedTexture3(pIdent, pTexture);
    }
    else
    {
        return LT_MISSINGFILE;
    }
}


SharedTexture* CClientMgr::AddSharedTexture(FileRef *pRef) {
    SharedTexture *pSharedTexture;

    if (AddSharedTexture2(pRef, pSharedTexture) == LT_OK)
    {
        return pSharedTexture;
    }
    else
    {
        return LTNULL;
    }
}


void CClientMgr::FreeSharedTexture(SharedTexture *pTexture) 
{
    dl_RemoveAt(&m_SharedTextures, &pTexture->m_Link);

    if (pTexture->m_pFile)
    {
        pTexture->m_pFile->m_pData = LTNULL;
    }

    r_UnbindTexture(pTexture,true);
    m_SharedTextureBank.Free(pTexture);
}


void CClientMgr::FreeSharedTextures() {
    LTLink *pCur, *pNext, *pListHead;

    pListHead = &m_SharedTextures.m_Head;
    pCur = pListHead->m_pNext;
    while (pCur != pListHead)
    {
        pNext = pCur->m_pNext;
        FreeSharedTexture((SharedTexture*)pCur->m_pData);
        pCur = pNext;
    }

    ASSERT(m_SharedTextures.m_nElements == 0);
}


int TagTexture(SharedTexture *pTexture)
   {
       if (pTexture)
       {
	        bool bAlreadyTagged = bool (!!(pTexture->GetFlags() & ST_TAGGED));
            pTexture->SetFlags(pTexture->GetFlags() | ST_TAGGED);
	        return (bAlreadyTagged ? 0 : 1);
       }

 	return 0;
   }


int _TagSpriteTextures(Sprite *pSprite)
   {
       uint32 i, j;
   
	int nCount = 0;
    for (i=0; i < pSprite->m_nAnims; i++)
    {
        for (j=0; j < pSprite->m_Anims[i].m_nFrames; j++)
        {
            nCount += TagTexture(pSprite->m_Anims[i].m_Frames[j].m_pTex);
        }
    }

	return nCount;
}


static int _TagWorldBspTextures(const WorldBsp *bsp)
{
    uint32 i;
    const Surface *pSurface;
   
	int nCount = 0;
    for (i=0; i < bsp->m_nSurfaces; i++)
    {
         pSurface = &bsp->m_Surfaces[i];
        nCount += TagTexture(pSurface->GetSharedTexture());
    }

	return nCount;
}


void CClientMgr::UntagAllTextures() {
    LTLink *pListHead, *pCur;
    SharedTexture *pTexture;

    pListHead = &m_SharedTextures.m_Head;
    for (pCur=pListHead->m_pNext; pCur != pListHead; pCur=pCur->m_pNext)
    {
        pTexture = ((SharedTexture*)pCur->m_pData);
        pTexture->SetFlags(pTexture->GetFlags() & ~ST_TAGGED);
    }
}


void CClientMgr::TagUsedTextures() {
    LTLink *pCur, *pListHead;
    ModelInstance *pModelInst;
    LTParticleSystem *pSystem;
    uint32 i;
    SharedTexture *pTexture;

    uint32 nTagCount = 0;

    // Tag textures that the game code has references to.
    pListHead = &m_SharedTextures.m_Head;
    for (pCur=pListHead->m_pNext; pCur != pListHead; pCur=pCur->m_pNext)
    {
        pTexture = ((SharedTexture*)pCur->m_pData);
        if (pTexture->GetRefCount() > 0)
        {
            nTagCount += TagTexture(pTexture);
        }
    }

    // Tag all linked textures
    pListHead = &m_SharedTextures.m_Head;
    for (pCur=pListHead->m_pNext; pCur != pListHead; pCur=pCur->m_pNext)
    {
        pTexture = (SharedTexture*)pCur->m_pData;
        nTagCount += TagTexture(pTexture->GetLinkedTexture(eLinkedTex_Detail));
		nTagCount += TagTexture(pTexture->GetLinkedTexture(eLinkedTex_EnvMap));
		nTagCount += TagTexture(pTexture->GetLinkedTexture(eLinkedTex_BumpMap));
		nTagCount += TagTexture(pTexture->GetLinkedTexture(eLinkedTex_EffectTexture1));
		nTagCount += TagTexture(pTexture->GetLinkedTexture(eLinkedTex_EffectTexture2));
		nTagCount += TagTexture(pTexture->GetLinkedTexture(eLinkedTex_EffectTexture3));
		nTagCount += TagTexture(pTexture->GetLinkedTexture(eLinkedTex_EffectTexture4));
    }

    // Tag all textures we can't unload.
    if (m_pCurShell) {
        for (i=0; i < world_bsp_client->NumWorldModels(); i++)
        {
            const WorldData *pWorldData = world_bsp_client->GetWorldModel(i);

            nTagCount += _TagWorldBspTextures(pWorldData->OriginalBSP());
            if (pWorldData->m_pWorldBsp)
                nTagCount += _TagWorldBspTextures(pWorldData->m_pWorldBsp);
        }
    }

    // Go thru objects..
    pListHead = &m_ObjectMgr.m_ObjectLists[OT_MODEL].m_Head;
    for (pCur=pListHead->m_pNext; pCur != pListHead; pCur=pCur->m_pNext)
    {
        pModelInst = (ModelInstance*)pCur->m_pData;
        for (i=0; i < MAX_MODEL_TEXTURES; i++)
        {
            nTagCount += TagTexture(pModelInst->m_pSkins[i]);
        }
    }

    pListHead = &m_Sprites.m_Head;
    for (pCur=pListHead->m_pNext; pCur != pListHead; pCur=pCur->m_pNext)
    {
        nTagCount += _TagSpriteTextures((Sprite*)pCur->m_pData);
    }

    pListHead = &m_ObjectMgr.m_ObjectLists[OT_PARTICLESYSTEM].m_Head;
    for (pCur=pListHead->m_pNext; pCur != pListHead; pCur=pCur->m_pNext)
    {
        pSystem = (LTParticleSystem*)pCur->m_pData;
        nTagCount += TagTexture(pSystem->m_pCurTexture);
    }
}


void CClientMgr::TagAndFreeTextures() 
{
    uint32 i;

    // NOTE: It runs through here twice so detail textures get freed (first pass
    // frees their owner texture and the second pass frees the detail texture).
    for (i=0; i < 2; i++)
    {
        UntagAllTextures();
        TagUsedTextures();
        FreeUnusedSharedTextures();
    }
}

void CClientMgr::TagAndFreeSprites() 
{
	// Untag All Sprites

	LTLink *pCur, *pListHead;
	pListHead = &m_Sprites.m_Head;
    for (pCur=pListHead->m_pNext; pCur != pListHead; pCur=pCur->m_pNext)
    {
        ((Sprite*)pCur->m_pData)->m_bTagged = false;
    }


	// Tag Used Sprites in ParticleSystems

	pListHead = &m_ObjectMgr.m_ObjectLists[OT_PARTICLESYSTEM].m_Head;
    for (pCur=pListHead->m_pNext; pCur != pListHead; pCur=pCur->m_pNext)
    {
        LTParticleSystem *pSystem = (LTParticleSystem*)pCur->m_pData;
		if (pSystem->m_pSprite)
		{
			pSystem->m_pSprite->m_bTagged = true;
		}
    }

	// Tag Used Sprites
	
	pListHead = &m_ObjectMgr.m_ObjectLists[OT_SPRITE].m_Head;
    for (pCur=pListHead->m_pNext; pCur != pListHead; pCur=pCur->m_pNext)
    {
        SpriteInstance* pSpriteInstance = (SpriteInstance*)pCur->m_pData;
		Sprite* pSprite = pSpriteInstance->GetSprite();
		if (pSprite)
		{
			pSprite->m_bTagged = true;
		}
    }

	// Tag Used Sprites in Models
	
    // Go thru objects..
    pListHead = &m_ObjectMgr.m_ObjectLists[OT_MODEL].m_Head;
    for (pCur=pListHead->m_pNext; pCur != pListHead; pCur=pCur->m_pNext)
    {
        ModelInstance* pModelInst = (ModelInstance*)pCur->m_pData;
        for (int i=0; i < MAX_MODEL_TEXTURES; i++)
        {
			if (pModelInst->m_pSprites[i])
			{
				pModelInst->m_pSprites[i]->m_bTagged = true;
			}
        }
    }

	// Tag Used Sprites in Polygrids

	pListHead = &m_ObjectMgr.m_ObjectLists[OT_POLYGRID].m_Head;
    for (pCur=pListHead->m_pNext; pCur != pListHead; pCur=pCur->m_pNext)
    {
        LTPolyGrid* pPolyGrid = (LTPolyGrid*)pCur->m_pData;
		if (pPolyGrid->m_pSprite)
		{
			pPolyGrid->m_pSprite->m_bTagged = true;
		}
    }

	// Free all Unused Shared Sprites

	pListHead = &m_Sprites.m_Head;
	pCur=pListHead->m_pNext;
	while (pCur != pListHead)
    {
		LTLink* pNext = pCur->m_pNext;

		Sprite* pSprite = ((Sprite*)pCur->m_pData);
		if (false == pSprite->m_bTagged)
		{
			spr_Destroy(pSprite);
        }

		pCur = pNext;
    }



}



void CClientMgr::BindUnboundTextures() {
    LTLink *pCur;
    SharedTexture *pTexture;

    for (pCur = m_SharedTextures.m_Head.m_pNext; pCur != &m_SharedTextures.m_Head; pCur=pCur->m_pNext)
    {
        pTexture = (SharedTexture*)pCur->m_pData;

        if (!pTexture->m_pRenderData)
        {
            r_BindTexture(pTexture, LTFALSE);
        }
    }
}


void CClientMgr::UnbindUnusedSharedTextures() {
    LTLink *pCur, *pNext, *pListHead;
    SharedTexture *pTexture;

    // Get rid of untagged ones.
    pListHead = &m_SharedTextures.m_Head;
    pCur = pListHead->m_pNext;
    while (pCur != pListHead)
    {
        pNext = pCur->m_pNext;

        pTexture = (SharedTexture*)pCur->m_pData;

        if (!(pTexture->GetFlags() & ST_TAGGED))
        {
            r_UnbindTexture(pTexture,true);
        }

        pCur = pNext;
    }
}


// Free the textures associated with a model if nobody else is using them
// Note : This assumes that the model textures aren't being used by
// anything other than models!
void CClientMgr::FreeUnusedModelTextures(LTObject *pObject) {
    ModelInstance *pInstance = pObject->ToModel();

    LTLink *pCur, *pListHead;
    SharedTexture *pTexture;
    uint32 nTextureLoop;

    // Handle the case where there are duplicate entries in the skins..
    for (nTextureLoop = 0; nTextureLoop < MAX_MODEL_TEXTURES; ++nTextureLoop)
    {
        for (uint32 nDupeLoop = nTextureLoop + 1; nDupeLoop < MAX_MODEL_TEXTURES; ++nDupeLoop)
        {
            if ((pInstance->m_pSkins[nTextureLoop] == pInstance->m_pSkins[nDupeLoop]) && (pInstance->m_pSkins[nTextureLoop]))
            {
                pInstance->m_pSkins[nDupeLoop] = LTNULL;
            }
        }
    }

    // Untag the textures used by this model
    for (nTextureLoop = 0; nTextureLoop < MAX_MODEL_TEXTURES; ++nTextureLoop)
    {
        pTexture = pInstance->m_pSkins[nTextureLoop];
        if (!pTexture)
            continue;
        // Make sure sprites don't get unloaded
        if (pInstance->m_pSprites[nTextureLoop])
            pTexture->SetFlags(pTexture->GetFlags() | ST_TAGGED);
        else
            pTexture->SetFlags(pTexture->GetFlags() & ~ST_TAGGED);
    }

    // Mark all client side model object textures as touched
    pListHead = &m_ObjectMgr.m_ObjectLists[OT_MODEL].m_Head;
    for (pCur=pListHead->m_pNext; pCur != pListHead; pCur=pCur->m_pNext)
    {
        ModelInstance *pFlagInstance = (ModelInstance*)pCur->m_pData;

        if (pFlagInstance == pInstance)
            continue;

        for (nTextureLoop = 0; nTextureLoop < MAX_MODEL_TEXTURES; ++nTextureLoop)
        {
            TagTexture(pFlagInstance->m_pSkins[nTextureLoop]);
        }
    }

    // Get rid of the model's textures if they haven't been tagged
    for (nTextureLoop = 0; nTextureLoop < MAX_MODEL_TEXTURES; ++nTextureLoop)
    {
        pTexture = pInstance->m_pSkins[nTextureLoop];
        if (!pTexture)
            continue;
        if ((pTexture->GetFlags() & ST_TAGGED) == 0)
        {
            FreeSharedTexture(pTexture);
            pInstance->m_pSkins[nTextureLoop] = LTNULL;
        }
    }
}


void CClientMgr::FreeUnusedSharedTextures() {
    LTLink *pCur, *pNext, *pListHead;
    SharedTexture *pTexture;

    // Get rid of untagged ones.
    pListHead = &m_SharedTextures.m_Head;
    pCur = pListHead->m_pNext;
    while (pCur != pListHead)
    {
        pNext = pCur->m_pNext;

        pTexture = (SharedTexture*)pCur->m_pData;

        if (!(pTexture->GetFlags() & ST_TAGGED))
        {
            FreeSharedTexture(pTexture);
        }

        pCur = pNext;
    }
}


void CClientMgr::AddToObjectMap(uint16 id) {
    LTRecord *newMap;
    uint32 newSize;

    // Make sure the object map is large enough.
    if (m_ObjectMapSize <= id)
    {
        newSize = id + 100;
        LT_MEM_TRACK_ALLOC(newMap = (LTRecord*)dalloc(sizeof(LTRecord) * newSize),LT_MEM_TYPE_OBJECT);
        memset(newMap, 0, sizeof(LTRecord) * newSize);

        memcpy(newMap, m_ObjectMap, sizeof(LTRecord) * m_ObjectMapSize);
        dfree(m_ObjectMap);
        m_ObjectMap = newMap;
        m_ObjectMapSize = newSize;
    }
}


void CClientMgr::ClearObjectMapEntry(uint16 id) {
    if (id < m_ObjectMapSize)
    {
        m_ObjectMap[id].m_nRecordType = 0;
        m_ObjectMap[id].m_pRecordData = LTNULL;
    }
}


LTObject* CClientMgr::FindObject(uint16 id) {
    if (id < m_ObjectMapSize && m_ObjectMap[id].m_nRecordType == RECORDTYPE_LTOBJECT)
        return (LTObject *)m_ObjectMap[id].m_pRecordData;

    return LTNULL;
}


LTRecord* CClientMgr::FindRecord(uint16 id) {
    if (id < m_ObjectMapSize)
        return &m_ObjectMap[id];

    return LTNULL;
}


void CClientMgr::ScaleObject( LTObject *pObject, const LTVector *pNewScale )
{
	LTBOOL bDifferent;

    bDifferent = !pNewScale->NearlyEquals(pObject->m_Scale);

    pObject->m_Scale = *pNewScale;
    world_bsp_client->ClientTree()->InsertObject(pObject);

    // Redo their dims if it's a model.
    if (bDifferent && pObject->m_ObjectType == OT_MODEL)
    {
        UpdateModelDims( ToModel(pObject) );
    }
}


// t.f move this to modelinstance.
void CClientMgr::UpdateModelDims( ModelInstance *pInstance )
{
    MoveState moveState;
    LTAnimTracker *pTracker;
    AnimInfo *pAnim;
    LTVector theDims;

    // Don't do it if they don't want us to.
    if ((pInstance->cd.m_ClientFlags & CF_DONTSETDIMS) || (pInstance->m_Flags2 & FLAG2_SERVERDIMS))
        return;

    pTracker = &pInstance->m_AnimTracker;
    if (pTracker->IsValid() && m_pCurShell)
    {
        pAnim = pTracker->GetCurAnimInfo();

        LTObject *obj = (LTObject *)pInstance;

        moveState.Setup(world_bsp_client->ClientTree(), m_MoveAbstract, obj, obj->m_BPriority);

        theDims = pAnim->m_vDims;
        theDims.x *= pInstance->m_Scale.x;
        theDims.y *= pInstance->m_Scale.y;
        theDims.z *= pInstance->m_Scale.z;

        ChangeObjectDimensions(&moveState, theDims, LTFALSE);
    }
}


void CClientMgr::MoveObject(LTObject *pObject, const LTVector *pNewPos, bool bForce)
{
    LTVector vDiff;

    if (!bForce)
    {
        vDiff = pObject->GetPos() - *pNewPos;
        if (vDiff.MagSqr() < 0.001f)
            return;
    }

    pObject->SetPos(*pNewPos);

    // Do special stuff if it's a WorldModel.
    if (pObject->HasWorldModel())
    {
        RetransformWorldModel(pObject->ToWorldModel());
    }

    world_bsp_client->ClientTree()->InsertObject(pObject);
}


void CClientMgr::RelocateObject(LTObject *pObject)
{
    MoveObject( pObject, &pObject->GetPos(), LTTRUE );
}


void CClientMgr::RotateObject(LTObject *pObject, const LTRotation *pNewRot)
{
	//set rotation
    pObject->m_Rotation = *pNewRot;

    // Do special stuff if it's a WorldModel.
    if (pObject->HasWorldModel())
	{
	    WorldModelInstance *pInst = pObject->ToWorldModel();
		MoveState theState;

        // Call RotateWorldModel to setup its dims and relocate it in the WorldTree
        // and stuff.
        theState.Setup(world_bsp_client->ClientTree(), m_MoveAbstract, pInst, pInst->m_BPriority);

        RotateWorldModel(&theState, *pNewRot, LTFALSE);
    }

	//insert into BSP tree
    world_bsp_client->ClientTree()->InsertObject(pObject);
}


void CClientMgr::MoveAndRotateObject(LTObject *pObject, const LTVector *pNewPos, const LTRotation *pNewRot)
{
    pObject->SetPos(*pNewPos);
    pObject->m_Rotation = *pNewRot;

    // Do special stuff for a WorldModel.
    if (pObject->HasWorldModel())
	{
        //obj_SetupWorldModelTransform(pObject->ToWorldModel());
        RetransformWorldModel(pObject->ToWorldModel());
    }

    world_bsp_client->ClientTree()->InsertObject(pObject);
}

//EOF

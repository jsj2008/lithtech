// ----------------------------------------------------------------
//  client loading and setting up of objects.
// ----------------------------------------------------------------
#include "bdefs.h"

#include "de_sprite.h"
#include "clientmgr.h"
#include "sprite.h"
#include "dutil.h"
#include "setupobject.h"
#include "animtracker.h"
#include "ltobjectcreate.h"

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
static IClientFileMgr* client_file_mgr;
define_holder(IClientFileMgr, client_file_mgr);

//ILTRenderStyles
#include "iltrenderstyles.h"
static ILTRenderStyles* renderstyles;
define_holder(ILTRenderStyles, renderstyles);

//ILTModel
#include "iltmodel.h"
static ILTModel *model;
define_holder_to_instance(ILTModel, model, Client);



LTRESULT LoadSprite(FileRef *pFilename, Sprite **ppSprite)
{
    Sprite *pSprite;
    ILTStream *pStream;
    FileIdentifier *pFileRef;
    const char *pFilenameString;

    
    *ppSprite = LTNULL;
    
    pFilenameString = client_file_mgr->GetFilename(pFilename);
    

    // Get the sprite.
    pFileRef = client_file_mgr->GetFileIdentifier(pFilename, TYPECODE_SPRITE);
    if (pFileRef)
    {
        if (pFileRef->m_pData)
        {
            pSprite = (Sprite*)pFileRef->m_pData;
        }
        else
        {
            // Create the sprite.
            pStream = client_file_mgr->OpenFile(pFilename);
            if (!pStream)
            {
                g_pClientMgr->SetupError(LT_MISSINGSPRITEFILE, pFilenameString);
                RETURN_ERROR_PARAM(1, LoadSprite, LT_MISSINGSPRITEFILE, pFilenameString);
            }
            
            if (g_DebugLevel >= 2)
            {
                dsi_ConsolePrint("Loading sprite: %s\n", client_file_mgr->GetFilename(pFilename));
            }
            
            pSprite = spr_Create(pStream);
            if (!pSprite)
            {
                g_pClientMgr->SetupError(LT_INVALIDSPRITEFILE, pFilenameString);
                RETURN_ERROR_PARAM(1, LoadSprite, LT_INVALIDSPRITEFILE, pFilenameString);
            }

            // Successfully created...
            pSprite->m_pFileIdent = pFileRef;
            pFileRef->m_pData = pSprite;
            dl_AddHead(&g_pClientMgr->m_Sprites, &pSprite->m_Link, pSprite);

            pStream->Release();
        }
    }
    else
    {
        g_pClientMgr->SetupError(LT_MISSINGSPRITEFILE, pFilenameString);
        RETURN_ERROR_PARAM(1, LoadSprite, LT_MISSINGSPRITEFILE, pFilenameString);
    }

    *ppSprite = pSprite;
    return LT_OK;
}



// ------------------------------------------------------------------ //
// Static functions to implement the table functions.
// ------------------------------------------------------------------ //

LTRESULT ModelSetTexture(FileRef *pRef, ModelInstance* pInstance, uint32 index)
{
    int len;
    char endChars[4];
    const char *pName;
    LTRESULT dResult;

	//see if we had a sprite setup on this slot
	if(pInstance->m_pSprites[index])
	{
		pInstance->m_pSprites[index] = LTNULL;

		assert(pInstance->m_nNumSprites > 0);
		pInstance->m_nNumSprites--;
	}
    

    pName = client_file_mgr->GetFilename(pRef);
    len = strlen(pName);

    if (len > 3)
    {
        endChars[0] = pName[len-3];
        endChars[1] = pName[len-2];
        endChars[2] = pName[len-1];
        endChars[3] = 0;

        if (du_UpperStrcmp(endChars, "DTX"))
        {
            return g_pClientMgr->AddSharedTexture2(pRef, pInstance->m_pSkins[index]);
        }
        else if (du_UpperStrcmp(endChars, "SPR"))
        {
            dResult = LoadSprite(pRef, &pInstance->m_pSprites[index]);
            if (dResult != LT_OK)
                return dResult;

			//we have one more sprite on our model
			pInstance->m_nNumSprites++;

            spr_InitTracker(&pInstance->m_SpriteTrackers[index], pInstance->m_pSprites[index]);
            if (pInstance->m_SpriteTrackers[index].m_pCurFrame)
            {
                pRef->m_pFilename = pInstance->m_SpriteTrackers[index].m_pCurFrame->m_pTex->m_pFile->m_Filename;
                return g_pClientMgr->AddSharedTexture2(pRef, pInstance->m_pSkins[index]);
            }
        }
    }

    return LT_ERROR;
}

// Called on model create - loads up a render style into the index list...
LTRESULT ModelSetRenderStyle(FileRef *pRef, ModelInstance* pInstance, uint32 index)
{
	const char* pName = client_file_mgr->GetFilename(pRef);
	uint32 len = strlen(pName);
	if (len > 3)
	{
		char endChars[4];
		endChars[0] = pName[len-3];
		endChars[1] = pName[len-2];
		endChars[2] = pName[len-1];
		endChars[3] = 0;

		if (du_UpperStrcmp(endChars, "LTB"))
		{
			if (pInstance->m_pRenderStyles[index]) 
				renderstyles->FreeRenderStyle(pInstance->m_pRenderStyles[index]);
			pInstance->m_pRenderStyles[index] = renderstyles->LoadRenderStyle(pName);
			if (!pInstance->m_pRenderStyles[index]) 
				return LT_ERROR;
			return LT_OK;
		}
	}

    return LT_ERROR;
}

static bool IsFileValid(const FileRef& File)
{
	if(File.m_pFilename && File.m_pFilename[0] != '\0')
		return true;

	if((File.m_FileID != 0xFFFF)) 
		return true;

	return false;
}

// client init model object.
static LTRESULT ModelExtraInit(	LTObject			*pObject, 
								InternalObjectSetup *pSetup, 
								bool				 bLocalFromServer)
{
    ModelInstance *pModelInstance;
    Model *pModel,*pChildModel;
    LTRESULT dResult;

    uint32 i;

    pModelInstance = ToModel(pObject);
    
	if(!bLocalFromServer && IsFileValid(pSetup->m_Filename[0]))
	{
		// try and load the model.
		dResult = g_pClientMgr->LoadModel( pSetup->m_Filename[0], pModel );
		
		if( dResult == LT_OK )
		{
			// if the new model is different then the interned model
			// install new model.
			if( pModel != pModelInstance->GetModelDB() ) 
			{
			pModelInstance->BindModelDB(pModel);
			pModelInstance->InitAnimTrackers();
			}// else go on our merry way.
		}
		else // 
		{
			// well heck we got ourselves an invalid model here !
			dResult = g_pClientMgr->LoadModel("models\\default.ltb", pModel );
			if( dResult == LT_OK )
			{
				pModelInstance->BindModelDB(pModel);
				pModelInstance->InitAnimTrackers();
			}
			// If were're here its bad.
			DEBUG_MODEL_REZ(("model-rez: client ModelExtraInit could not load  %s (%d)", 
								pSetup->m_Filename[0].m_pFilename, 
								pSetup->m_Filename[0].m_FileID));
			return dResult ;
		}
	}
	else
	{
		pModel = pModelInstance->GetModelDB();
		if( !pModel )
		{
			// well heck we got ourselves an invalid model here !
			dResult = g_pClientMgr->LoadModel("models\\default.ltb", pModel );
			if( dResult == LT_OK )
			{
				pModelInstance->BindModelDB(pModel);
				pModelInstance->InitAnimTrackers();
			}

			// If were're here its bad.
			DEBUG_MODEL_REZ(("model-rez: client ModelExtraInit no model file specified"));
			return dResult;
		}
	}
	
	// add child models if any.
	for( i = 1 ; i < MAX_CHILD_MODELS ; i++ )
	{	
		if(IsFileValid(pSetup->m_Filename[i]) ) // if we have a filename load it.
		{
			pSetup->m_Filename[i].m_FileType= FILE_SERVERFILE;
			if( g_pClientMgr->LoadModel( pSetup->m_Filename[i], pChildModel ) == LT_OK )
			{
				char err_str[256];
				if (pModel)
					pModel->AddChildModel(pChildModel,pChildModel->GetFilename(),err_str, sizeof(err_str) );
			}
		}
	}

    for (i=0; i < MAX_MODEL_TEXTURES; i++)
    {
		if(IsFileValid(pSetup->m_SkinNames[i]))
		{
			pModelInstance->m_pSkins[i] = LTNULL;
	        dResult = ModelSetTexture(&pSetup->m_SkinNames[i], pModelInstance, i);
		}
    }

    for (i=0; i < MAX_MODEL_RENDERSTYLES; i++)
	{
		if(IsFileValid(pSetup->m_RenderStyleNames[i]))
		{
			if (pModelInstance->m_pRenderStyles[i])
			{
				renderstyles->FreeRenderStyle(pModelInstance->m_pRenderStyles[i]);
 				pModelInstance->m_pRenderStyles[i] = NULL;
			}
			dResult = ModelSetRenderStyle(&pSetup->m_RenderStyleNames[i], pModelInstance, i);
		}
	}

	g_pClientMgr->UpdateModelDims(pModelInstance);
  
   
    return LT_OK;
}

static LTRESULT ModelExtraTerm(LTObject *pObject)
{
    // Just decrement the reference count when terminating a model object
    ModelInstance *pInstance = ToModel(pObject);

	// Remove trackers
	ASSERT( pInstance->m_AnimTrackers );
	pInstance->UnbindModelDB();

    return LT_OK;
}

static LTRESULT WorldModelExtraInit(LTObject *pObject, 
    InternalObjectSetup *pSetup, bool bLocalFromServer)
{
    WorldModelInstance *pInst;
    WorldBsp *pWorldBsp;


    if (bLocalFromServer)
        return LT_OK;

    pInst = pObject->ToWorldModel();
    if (world_bsp_client->InitWorldModel(pInst, pSetup->m_pSetup->m_Filename))
    {
        pWorldBsp = (WorldBsp*)pInst->m_pOriginalBsp;
        pInst->SetDims((pWorldBsp->m_MaxBox - pWorldBsp->m_MinBox) * 0.5f);
    }
    else
    {
        RETURN_ERROR_PARAM(1, WorldModelExtraInit, 
							  LT_MISSINGWORLDMODEL, 
							  pSetup->m_pSetup->m_Filename[0]);
    }

    return LT_OK;
}


static LTRESULT SpriteExtraInit(LTObject *pObject, InternalObjectSetup *pSetup, bool bLocalFromServer)
{
    SpriteInstance *pSpriteInstance;
    Sprite *pSprite;
    LTRESULT dResult;

    dResult = LoadSprite(&pSetup->m_Filename[0], &pSprite);
    if (dResult != LT_OK)
        return dResult;

    // Create the sprite instance.
    pSpriteInstance = ToSprite(pObject);
    spr_InitTracker(&pSpriteInstance->m_SpriteTracker, pSprite);
    return LT_OK;
}


static LTRESULT ContainerExtraInit(LTObject *pObject, InternalObjectSetup *pSetup, bool bLocalFromServer)
{
    LTRESULT dResult;

    if (bLocalFromServer)
        return LT_OK;

    dResult = WorldModelExtraInit(pObject, pSetup, bLocalFromServer);
    if (dResult != LT_OK)
        return dResult;

    ToContainer(pObject)->m_ContainerCode = pSetup->m_pSetup->m_ContainerCode;
    return LT_OK;
}


// ------------------------------------------------------------------ //
// Interface functions.
// ------------------------------------------------------------------ //

struct ExtraInitStruct
{
    LTRESULT (*Init)(LTObject *pObject, 
        InternalObjectSetup *pSetup, bool bLocalFromServer);
    LTRESULT (*Term)(LTObject *pObject);
};

ExtraInitStruct g_ExtraInitStructs[NUM_OBJECTTYPES] =
{
    LTNULL, LTNULL,             // OT_NORMAL
    ModelExtraInit, ModelExtraTerm,     // OT_MODEL
    WorldModelExtraInit, LTNULL,// OT_WORLDMODEL
    SpriteExtraInit, LTNULL,    // OT_SPRITE
    LTNULL, LTNULL,             // OT_LIGHT
    LTNULL, LTNULL,             // OT_CAMERA
    LTNULL, LTNULL,             // OT_PARTICLESYSTEM
    LTNULL, LTNULL,             // OT_POLYGRID
    LTNULL, LTNULL,             // OT_LINESYSTEM
    ContainerExtraInit, LTNULL, // OT_CONTAINER
    LTNULL, LTNULL,             // OT_CANVAS
    LTNULL, LTNULL              // OT_VOLUMEEFFECT
};


LTRESULT so_ExtraInit(LTObject *pObject, 
    InternalObjectSetup *pSetup, bool bFromLocalServer)
{
    if (g_ExtraInitStructs[pObject->m_ObjectType].Init)
        return g_ExtraInitStructs[pObject->m_ObjectType].Init(pObject, pSetup, bFromLocalServer);
    else
        return LT_OK;
}


LTRESULT so_ExtraTerm(LTObject *pObject)
{
    if (g_ExtraInitStructs[pObject->m_ObjectType].Term)
        return g_ExtraInitStructs[pObject->m_ObjectType].Term(pObject);
    else
        return LT_OK;
}















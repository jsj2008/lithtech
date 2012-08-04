

#include "bdefs.h"

#include "serverobj.h"
#include "servermgr.h"
#include "model_ops.h"
#include "s_net.h"
#include "moveobject.h"
#include "server_extradata.h"
#include "smoveabstract.h"
#include "animtracker.h"
#include "syslthread.h"
#include "dhashtable.h"
#include "ftserv.h"
#include "ltobjectcreate.h"

//------------------------------------------------------------------
//------------------------------------------------------------------
// Holders and their headers.
//------------------------------------------------------------------
//------------------------------------------------------------------

//server file mgr.
#include "server_filemgr.h"
static IServerFileMgr *server_filemgr;
define_holder(IServerFileMgr, server_filemgr);

//IWorld holder
#include "world_server_bsp.h"
#include "de_mainworld.h"
static IWorldServerBSP *world_bsp_server;
define_holder(IWorldServerBSP, world_bsp_server);

//Model interface
#include "iltmodel.h"
static ILTModel *model;
define_holder_to_instance(ILTModel, model, Server);


LTRESULT se_ChangeSpriteFile(LTObject *pObject, const char* pszFile)
{
	SpriteInstance *pSpriteInstance = (SpriteInstance*)pObject;

	if(server_filemgr->AddUsedFile(pszFile, 0, &pObject->sd->m_pFile) == 0)
	{
		DEBUG_PRINT(1, ("Couldn't find sprite file %s, using sprites/default.spr.", pszFile));

		const char* pTestFilename = "sprites\\default.spr";
		if(server_filemgr->AddUsedFile(pTestFilename, 0, &pObject->sd->m_pFile) == 0)
		{
			RETURN_ERROR_PARAM(1, se_InitSprite, LT_MISSINGFILE, pTestFilename);
		}
	}

	return LT_OK;
}

// ------------------------------------------------------------------------
// se_LoadAdditionalChildModels( model-instance, filename-array )
// create struct may have instructions to add child models. do it here.
// ------------------------------------------------------------------------
static void se_LoadAdditionalChildModels( LTObject *pObject, const ObjectCreateStruct & OCS )
{
	ModelInstance *pModelInstance = pObject->ToModel() ;

	Model *pModel = NULL ;
	Model *pChildModel ;

	pModel = pModelInstance->GetModelDB();

	if( pModel )
	{
		for(uint32 cm_cnt = 1 ; cm_cnt < MAX_CHILD_MODELS ; cm_cnt ++ )
		{
			if( OCS.m_Filenames[cm_cnt][0] != '\0' )
			if( g_pServerMgr->LoadModel( OCS.m_Filenames[cm_cnt], pChildModel ) == LT_OK )
			{
				char err_str[256];
				pModel->AddChildModel( pChildModel, pChildModel->GetFilename(), err_str, sizeof(err_str) );
			}
		}
	}
}

// ------------------------------------------------------------------------
// se_ChangeModelFile( model-instance, filename )
// if the pszFile is valid, change pObject's model with new file.
// ------------------------------------------------------------------------
LTRESULT se_ChangeModelFile(LTObject *pObject, const char* pszFile)
{
	ModelInstance *pModelInstance = pObject->ToModel();
		
	Model *pModel = NULL ;
	
	LTRESULT Result;
	if (!pszFile || !*pszFile)
	{
		// Handle a null filename on a model that's already loaded as a noop
		if (pModelInstance->GetModelDB())
			return LT_OK;
		else
			Result = LT_MISSINGFILE;
	}
	else
	{
	
		Result = g_pServerMgr->LoadModel( pszFile, pModel ); 
	}
	if(Result == LT_OK)
	{
		//see if the models match up, if so, we don't need to reset it
		pModelInstance->BindModelDB(pModel);

		// add do callback to trakers.
		pModelInstance->InitAnimTrackers(AT_DOCALLBACKS);
	}
	else
	{
		Result = g_pServerMgr->LoadModel( "models\\default.ltb", pModel );
		if( Result == LT_OK )
		{
			pModelInstance->BindModelDB(pModel);

			// add do callback to trakers.
			pModelInstance->InitAnimTrackers(AT_DOCALLBACKS);
		}
		else 
		{
			RETURN_ERROR_PARAM(1, se_InitModelObject, LT_MISSINGFILE, pszFile);	
		}
	}

	return LT_OK;
}

LTRESULT se_ChangeModelTexture(LTObject *pObject, const char* pszFile, uint32 nTex)
{
    char pszTexName[256];

	CHelpers::FormatFilename( pszFile, pszTexName, sizeof( pszTexName ));

	if(pszTexName[0] != 0)
    {
		if(server_filemgr->AddUsedFile(pszTexName, 0, &pObject->sd->m_pSkins[nTex]) == 0)
		{
			dsi_ConsolePrint("Couldn't find model skin %s.", pszTexName);

			const char* pTestFilename = "skins\\default.dtx";
			server_filemgr->AddUsedFile(pTestFilename, 0, &pObject->sd->m_pSkins[nTex]);
		}
	}

	return LT_OK;
}

LTRESULT se_ChangeModelRenderStyle(LTObject *pObject, const char* pszFile, uint32 nRS)
{
    char pszRenderStyle[256];

	//pRenderStyleName = pStruct->m_RenderStyleNames[i];
	CHelpers::FormatFilename( pszFile, pszRenderStyle, sizeof( pszRenderStyle ));

	if(pszRenderStyle[0] != 0)
    {
		if(server_filemgr->AddUsedFile(pszRenderStyle, 0, &pObject->sd->m_pRenderStyles[nRS]) == 0)
		{
			dsi_ConsolePrint("Couldn't find model renderstyle %s.", pszRenderStyle);

			const char* pTestFilename = "RenderStyles\\default.ltb";
			server_filemgr->AddUsedFile(pTestFilename, 0, &pObject->sd->m_pRenderStyles[nRS]);
		}
	}

	return LT_OK;
}



// Initialize a model instance object
static LTRESULT se_InitModelObject(LTObject *pObject, ObjectCreateStruct *pStruct)
{
	uint32 i;

	// Setup the skins.
	for(i=0; i < MAX_MODEL_TEXTURES; i++)
	{
		if(pStruct->m_SkinNames[i][0] != '\0')
		se_ChangeModelTexture(pObject, pStruct->m_SkinNames[i], i);
	}

	// Setup the renderstyles.
	for(i=0; i < MAX_MODEL_RENDERSTYLES; i++)
	{
		if(pStruct->m_RenderStyleNames[i][0] != '\0')
		se_ChangeModelRenderStyle(pObject, pStruct->m_RenderStyleNames[i], i);
	}

	LTRESULT Result = se_ChangeModelFile(pObject, pStruct->m_Filename);
	if(Result != LT_OK)
		return Result;

	se_LoadAdditionalChildModels( pObject, *pStruct );

	return LT_OK;
}

static LTRESULT se_TermModelObject(LTObject *pObject)
{
	// Get the model instance
	ModelInstance *pModelInstance = ToModel(pObject);
	
	ASSERT(pModelInstance);
	if (!pModelInstance)
		return LT_ERROR;

	pModelInstance->UnbindModelDB();
	
	return LT_OK;
}

static LTRESULT se_InitSprite(LTObject *pObject, ObjectCreateStruct *pStruct)
{
	if(!pStruct || !pObject)
		return LT_INVALIDPARAMS;

	SpriteInstance *pSpriteInstance = (SpriteInstance*)pObject;
	pSpriteInstance->m_ClipperPoly  = INVALID_HPOLY;

	return se_ChangeSpriteFile(pObject, pStruct->m_Filename);
}


static LTRESULT se_InitWorldModel(LTObject *pObject, ObjectCreateStruct *pStruct)
{
	WorldModelInstance *pInstance;
	WorldBsp *pModelBsp;
	LTVector newDims;
	MoveState moveState;

	pInstance = (WorldModelInstance*)pObject;

	if(world_bsp_server->InitWorldModel(pInstance, pStruct->m_Filename))
	{
		pModelBsp = (WorldBsp*)pInstance->m_pOriginalBsp;
		newDims = (pModelBsp->m_MaxBox - pModelBsp->m_MinBox) * 0.5f;

		// Move the OBJECT to the center of the world model geometry.
		pObject->SetPos(pModelBsp->m_WorldTranslation);

		// Set its dims for it.
		moveState.Setup(world_bsp_server->ServerTree(), g_pServerMgr->m_MoveAbstract, pObject, pObject->m_BPriority);
		ChangeObjectDimensions(&moveState, newDims, false);

		return LT_OK;
	}
	else
	{
		RETURN_ERROR(1, se_InitWorldModel, LT_MISSINGWORLDMODEL);
	}
}


struct ExtraDataStruct
{
	LTRESULT (*Init)(LTObject *pObject, ObjectCreateStruct *pStruct);
	LTRESULT (*Term)(LTObject *pObject);
};

ExtraDataStruct g_ExtraDataStructs[NUM_OBJECTTYPES] =
{
	LTNULL, LTNULL,				// OT_NORMAL
	se_InitModelObject, se_TermModelObject,	// OT_MODEL
	se_InitWorldModel, LTNULL,	// OT_WORLDMODEL
	se_InitSprite, LTNULL,		// OT_INITSPRITE
	LTNULL, LTNULL,				// OT_LIGHT
	LTNULL, LTNULL,				// OT_CAMERA
	LTNULL, LTNULL,				// OT_PARTICLESYSTEM
	LTNULL, LTNULL,				// OT_POLYGRID
	LTNULL, LTNULL,				// OT_LINESYSTEM
	se_InitWorldModel, LTNULL,	// OT_CONTAINER
	LTNULL, LTNULL,				// OT_CANVAS
	LTNULL, LTNULL				// OT_VOLUMEEFFECT
};





LTRESULT sm_InitExtraData(LTObject *pObject, ObjectCreateStruct *pStruct)
{
	char formattedFilename[512];

	// before we pass this off we need to redo the file name
	CHelpers::FormatFilename(pStruct->m_Filename, 
							 formattedFilename,
							 sizeof(formattedFilename));

	strcpy (pStruct->m_Filename,formattedFilename);

	if(g_ExtraDataStructs[pObject->m_ObjectType].Init)
	{
		return g_ExtraDataStructs[pObject->m_ObjectType].Init(pObject, pStruct);
	}
	else
	{
		return LT_OK;
	}
}


LTRESULT sm_TermExtraData(LTObject *pObject)
{
	if(g_ExtraDataStructs[pObject->m_ObjectType].Term)
	{
		return g_ExtraDataStructs[pObject->m_ObjectType].Term(pObject);
	}
	else
	{
		return LT_OK;
	}
}


LTRESULT BackupExtraData(LTObject *pObject, ExtraDataBackup *pBackup)
{
	if(!pObject || !pObject->sd)
	{
		RETURN_ERROR(1, BackupExtraData, LT_INVALIDPARAMS);
	}

	pBackup->m_pFile = pObject->sd->m_pFile;
	pBackup->m_pSkin = pObject->sd->m_pSkin;
	if(pObject->m_ObjectType == OT_MODEL)
	{
		pBackup->m_AnimTracker = ToModel(pObject)->m_AnimTracker;

	}
	return LT_OK;
}



LTRESULT RestoreExtraData(LTObject *pObject, ExtraDataBackup *pBackup)
{
	if(!pObject || !pObject->sd)
	{
		RETURN_ERROR(1, RestoreExtraData, LT_INVALIDPARAMS);
	}

	pObject->sd->m_pFile = pBackup->m_pFile;
	pObject->sd->m_pSkin = pBackup->m_pSkin;

	if(pObject->m_ObjectType == OT_MODEL)
	{
		ToModel(pObject)->m_AnimTracker = pBackup->m_AnimTracker;
	}
	return LT_OK;
}







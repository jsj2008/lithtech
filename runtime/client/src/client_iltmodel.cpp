#include "bdefs.h"

#include "iltmodel.h"
#include "clientmgr.h"



//IClientFileMgr
#include "client_filemgr.h"
static IClientFileMgr* client_file_mgr;
define_holder(IClientFileMgr, client_file_mgr);

//
//Client side implementation class for ILTModel
//

class CLTModelClient : public ILTModelClient {
public:
    declare_interface(CLTModelClient);

    //ILTModelClient functions.
    LTRESULT SetRenderStyle(HOBJECT pObj, uint32 iIndex, CRenderStyle* pRenderStyle);
    LTRESULT GetRenderStyle(HOBJECT pObj, uint32 iIndex, CRenderStyle** ppRenderStyle);

    //ILTModel overrides.
    LTRESULT SetCurAnim(HOBJECT hModel, ANIMTRACKERID TrackerID, HMODELANIM hAnim);

	// getfilesnames 
	LTRESULT GetFilenames( HOBJECT hModel, 
								   char *pFilename, uint32 file_name_buf_size,
								   char *pSkinname, uint32 skin_name_buf_size);

    LTRESULT GetModelDBFilename( HOBJECT hModel, 
										 char *pRetFilename, uint32 fileBufLen );
	LTRESULT GetSkinFilename(   HOBJECT hModel, uint32 skin_index,
										char *pRetFilenames, uint32 fileBufLen );

	// db manip
	LTRESULT CacheModelDB( const char *filename, HMODELDB & hModelDB ); //client side implementation
	LTRESULT UncacheModelDB( HMODELDB &hModelDB );
	LTRESULT UncacheModelDB( const char *filaname );
	LTRESULT IsModelDBLoaded( HMODELDB hModelDB);

};

//instantiate our implementation class.
instantiate_interface(CLTModelClient, ILTModel, Client);
implements_also(CLTModelClient, Client, ILTModelClient, Default);


// ------------------------------------------------------------------------
// 
// ------------------------------------------------------------------------
LTRESULT CLTModelClient::GetFilenames(HOBJECT hModel, char *pFilename, uint32 fileBufLen, 
										char *pSkinName, uint32 skinBufLen)
{
	FN_NAME(CLTModelClient::GetFilenames);
    ModelInstance *pInst;
    
    CHECK_PARAMS2(hModel && hModel->m_ObjectType == OT_MODEL);

	pInst = hModel->ToModel();

    if (pInst->GetModelDB() && pInst->GetModelDB()->GetFilename() ) 
	{
        LTStrCpy(pFilename, pInst->GetModelDB()->GetFilename(), fileBufLen);
    }
    else if(fileBufLen)
	{
        pFilename[0] = 0;
    }
	
	if( pInst->m_pSkins[0])
	{
        LTStrCpy(pSkinName, pInst->m_pSkins[0]->m_pFile->m_Filename, skinBufLen);
    }
    else if(skinBufLen)
	{
        pSkinName[0] = 0;
    }

    return LT_OK;
}

// ------------------------------------------------------------------------
// 
// ------------------------------------------------------------------------
LTRESULT CLTModelClient::GetModelDBFilename( HOBJECT hModel, char *pRetFilename, uint32 fileBufLen )
{
		FN_NAME(CLTModelClient::GetFilenames);
    ModelInstance *pInst;
    
    CHECK_PARAMS2(hModel && hModel->m_ObjectType == OT_MODEL);

	pInst = hModel->ToModel();

    if (pInst->GetModelDB() && pInst->GetModelDB()->GetFilename() ) 
	{
        LTStrCpy(pRetFilename, pInst->GetModelDB()->GetFilename(), fileBufLen);
    }
    else if(fileBufLen)
	{
        pRetFilename[0] = 0;
    }

	return LT_OK ;
}


// ------------------------------------------------------------------------
// 
// ------------------------------------------------------------------------
LTRESULT CLTModelClient::GetSkinFilename( HOBJECT hModel, uint32 skin_index, char *pRetFilenames, uint32 fileBufLen )
{
	FN_NAME(CLTModelClient::GetFilenames);
    ModelInstance *pInst;
    
    CHECK_PARAMS2(hModel && hModel->m_ObjectType == OT_MODEL);

	pInst = hModel->ToModel();
	
	if( skin_index < MAX_MODEL_TEXTURES)
	{
		if( pInst->m_pSkins[skin_index])
		{
			LTStrCpy(pRetFilenames, pInst->m_pSkins[skin_index]->m_pFile->m_Filename, fileBufLen);
		}
		else if(fileBufLen)
		{
			pRetFilenames[0] = '\0';
		}
    }

	return LT_OK ;
}



LTRESULT CLTModelClient::SetRenderStyle(HOBJECT pObj, uint32 iIndex, CRenderStyle* pRenderStyle)
{
    FN_NAME(CLTModelClient::SetRenderStyle);
    ModelInstance *pInst; //uint32 iPiece;

    CHECK_PARAMS2(pObj && pObj->m_ObjectType == OT_MODEL);
    pInst = ToModel(pObj);

    if (pInst->SetRenderStyle(iIndex,pRenderStyle)) return LT_OK;
    return LT_ERROR;
}

LTRESULT CLTModelClient::GetRenderStyle(HOBJECT pObj, uint32 iIndex, CRenderStyle** ppRenderStyle)
{
    FN_NAME(CLTModelClient::GetRenderStyle);
    ModelInstance *pInst; 

    CHECK_PARAMS2(pObj && pObj->m_ObjectType == OT_MODEL);
    pInst = ToModel(pObj);

    if (pInst->GetRenderStyle(iIndex,ppRenderStyle)) return LT_OK;
    return LT_ERROR;
}

LTRESULT CLTModelClient::SetCurAnim(HOBJECT hModel, ANIMTRACKERID TrackerID, HMODELANIM hAnim) 
{
    FN_NAME(CLTModelClient::SetCurAnim);
    LTRESULT dResult;
    ModelInstance *pInst;

    pInst = hModel->ToModel();
    CHECK_PARAMS2(pInst && pInst->GetTracker(TrackerID));

    dResult = ILTModel::SetCurAnim(hModel, TrackerID, hAnim);
    if (dResult == LT_OK)
    {
        g_pClientMgr->UpdateModelDims(pInst);
    }

    return dResult;
}



// loads a model, its kept by the client mgr, but its not installed.
LTRESULT CLTModelClient::CacheModelDB( const char *filename, HMODELDB & hModelDB )
{	
	LTRESULT dResult ;
	hModelDB = 0;
	
	if( filename )
	{
		dResult = g_pClientMgr->CacheModelFile( filename );
		if( dResult == LT_OK )
		{
			hModelDB = g_ModelMgr.Find( filename )->m_FileID;
			
		}
	}
	return dResult ;
}

// flushes the model requested
LTRESULT CLTModelClient::UncacheModelDB( HMODELDB &hModelDB )
{
	// find the model in client mgr that matches the file ident .
	if( hModelDB )
	{
		Model *pModel = g_ModelMgr.Find( hModelDB );
		if( pModel )
		{
			g_pClientMgr->UncacheModelFile( pModel );
			hModelDB = 0;
			return LT_OK ;
		}
	}
	return LT_ERROR ;
}

// flushes the model requested
LTRESULT CLTModelClient::UncacheModelDB( const char *pkFilename )
{
	char fixed_filename[256];
	CHelpers::FormatFilename( pkFilename, fixed_filename, 256);
	// find the model in client mgr that matches the file ident .
	Model *pModel = g_ModelMgr.Find( fixed_filename );
	if( pModel )
	{
		g_pClientMgr->UncacheModelFile( pModel );
		return LT_OK ;
	}
	
	return LT_ERROR ;
}


// check the client for the model in its lists.
LTRESULT CLTModelClient::IsModelDBLoaded( HMODELDB hModelDB)
{
	if( hModelDB )
	{
		if( g_ModelMgr.Find( hModelDB ) != NULL )
		{
			return LTTRUE ;
		}
	}

	return LTFALSE ;
} 


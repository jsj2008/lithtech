//
// Monolith 
// Server side model interface implementation
// server_iltmodel.cpp
// 
#include "bdefs.h"

#include "iltmodel.h"
#include "s_object.h"
#include "packetdefs.h"
#include "de_objects.h"
#include "animtracker.h"
#include "dhashtable.h"
#include "servermgr.h"
#include "server_extradata.h"
#include "s_net.h"

//server file mgr.
#include "server_filemgr.h"
static IServerFileMgr *server_filemgr;
define_holder(IServerFileMgr, server_filemgr);

#include "iltserver.h"
static ILTServer *iltserver ;
define_holder(ILTServer, iltserver );

// ------------------------------------------------------------------------
// Server side model interface implementation
// ------------------------------------------------------------------------
class CLTModelServer : public ILTModel {
public:
    declare_interface(CLTModelServer);

    // Function implementations.
	virtual LTRESULT SetPieceHideStatus(HOBJECT pObj, HMODELPIECE hPiece, bool bHidden);
    virtual LTRESULT AddTracker(HOBJECT hModel, ANIMTRACKERID TrackerID);
    virtual LTRESULT RemoveTracker(HOBJECT hModel, ANIMTRACKERID TrackerID);
    virtual LTRESULT SetLooping(HOBJECT hModel, ANIMTRACKERID TrackerID, bool bLooping);
    virtual LTRESULT SetPlaying(HOBJECT hModel, ANIMTRACKERID TrackerID, bool bPlaying);
    virtual LTRESULT SetCurAnimTime(HOBJECT hModel, ANIMTRACKERID TrackerID, uint32 curTime);
    virtual LTRESULT SetAnimRate(HOBJECT hModel, ANIMTRACKERID TrackerID, float fRate);
    virtual LTRESULT SetCurAnim(HOBJECT hModel, ANIMTRACKERID TrackerID, HMODELANIM hAnim);
    virtual LTRESULT ResetAnim(HOBJECT hModel, ANIMTRACKERID TrackerID);
    virtual LTRESULT SetWeightSet(HOBJECT hModel, ANIMTRACKERID TrackerID, HMODELWEIGHTSET hSet);

	// getfilesnames 
	LTRESULT GetFilenames( HOBJECT hModel, 
								   char *pFilename, uint32 file_name_buf_size,
								   char *pSkinname, uint32 skin_name_buf_size);

	LTRESULT GetModelDBFilename( HOBJECT hModel, 
										 char *pRetFilename, uint32 fileBufLen );
	LTRESULT GetSkinFilename(   HOBJECT hModel, uint32 skin_index ,
										char *pRetFilenames, uint32 fileBufLen );


	LTRESULT CacheModelDB( const char *filename, HMODELDB & hModelDB ); //client side implementation
	LTRESULT UncacheModelDB( HMODELDB &hModelDB );
	LTRESULT IsModelDBLoaded( HMODELDB hModelDB);
	LTRESULT AddChildModelDB( HOBJECT hModel, HMODELDB hModelDB );

};

//instantiate our server class and put it into the interface mgr.
instantiate_interface(CLTModelServer, ILTModel, Server);



LTRESULT CLTModelServer::GetFilenames(HOBJECT hModel, 
								   char *pFilename, uint32 fileBufLen,
								   char *pSkinName, uint32 skinBufLen)
{

	FN_NAME(CLTModelServer::GetFilenames);
        
    CHECK_PARAMS2(hModel && hModel->m_ObjectType == OT_MODEL);
    
	GetModelDBFilename(hModel,pFilename,fileBufLen);
    
    if (hModel->sd->m_pSkin) 
	{
        LTStrCpy(pSkinName, server_filemgr->GetUsedFilename(hModel->sd->m_pSkin), skinBufLen);
    }
    else if(skinBufLen)
	{
        pSkinName[0] = '\0';
    }

    return LT_OK;
}

// ------------------------------------------------------------------------
// GetModelDBFilename
// get the filename of the model database associated with this object.
// ------------------------------------------------------------------------
LTRESULT CLTModelServer::GetModelDBFilename(HOBJECT hModel,  char *pFilename, uint32 fileBufLen)
{

	FN_NAME(CLTModelServer::GetModelDBFilename);
    ModelInstance *pInst;
    
    CHECK_PARAMS2(hModel && hModel->m_ObjectType == OT_MODEL);

	pInst = hModel->ToModel();

    if (pInst->GetModelDB() && pInst->GetModelDB()->GetFilename() ) 
	{
        LTStrCpy(pFilename, pInst->GetModelDB()->GetFilename(), fileBufLen);
    }
    else if(fileBufLen)
	{
        pFilename[0] = '\0';
    }

    
    return LT_OK;
}

// ------------------------------------------------------------------------
// GetSkinFilenames
// ------------------------------------------------------------------------
LTRESULT CLTModelServer::GetSkinFilename(HOBJECT hModel, uint32 skin_index, char *pRetFilename, uint32 fileBufLen)
{
   
	FN_NAME(CLTModelServer::GetSkinFilename);
    ModelInstance *pInst;
    
    CHECK_PARAMS2(hModel && hModel->m_ObjectType == OT_MODEL);

	pInst = hModel->ToModel();

	if( skin_index < MAX_MODEL_TEXTURES )
	{
		if (pInst->sd->m_pSkins[skin_index]) 
		{
			char * filename = server_filemgr->GetUsedFilename(pInst->sd->m_pSkins[skin_index]);

			if( filename != NULL )
				LTStrCpy(pRetFilename, filename, fileBufLen);
		}
		else if(fileBufLen)
		{
			pRetFilename[0] = '\0';
		}
    }
    

    return LT_OK;
}

LTRESULT CLTModelServer::SetPieceHideStatus(HOBJECT pObj, HMODELPIECE hPiece, bool bHidden)
{
    LTRESULT dResult;

    // ILTModel does most of the work.
    dResult = ILTModel::SetPieceHideStatus(pObj, hPiece, bHidden);
    return dResult;
}

LTRESULT CLTModelServer::AddTracker(HOBJECT hModel, ANIMTRACKERID TrackerID) {
    LTRESULT dResult;

    dResult = ILTModel::AddTracker(hModel, TrackerID);
    if (dResult == LT_OK)
    {
		SetObjectChangeFlags(hModel, CF_MODELINFO);
    }

    return dResult;
}

LTRESULT CLTModelServer::RemoveTracker(HOBJECT hModel, ANIMTRACKERID TrackerID) {
    LTRESULT dResult;

    dResult = ILTModel::RemoveTracker(hModel, TrackerID);
    if (dResult == LT_OK)
    {
		SetObjectChangeFlags(hModel, CF_MODELINFO);
    }

    return dResult;
}

LTRESULT CLTModelServer::SetLooping(HOBJECT hModel, ANIMTRACKERID TrackerID, bool bLooping) {
    FN_NAME(CLTModelServer::SetLooping);
    LTRESULT dResult;
    ModelInstance *pInst;

    pInst = hModel->ToModel();
    CHECK_PARAMS2(pInst && pInst->GetTracker(TrackerID));

    dResult = ILTModel::SetLooping(hModel, TrackerID, bLooping);
    if (dResult == LT_OK)
    {
		SetObjectChangeFlags(pInst, CF_MODELINFO);
    }

    return dResult;
}

LTRESULT CLTModelServer::SetPlaying(HOBJECT hModel, ANIMTRACKERID TrackerID, bool bPlaying) {
    FN_NAME(CLTModelServer::SetPlaying);
    LTRESULT dResult;
    ModelInstance *pInst;

    pInst = hModel->ToModel();
    CHECK_PARAMS2(pInst && pInst->GetTracker(TrackerID));

    dResult = ILTModel::SetPlaying(hModel, TrackerID, bPlaying);
    if (dResult == LT_OK)
    {
		SetObjectChangeFlags(pInst, CF_MODELINFO);
    }

    return dResult;
}

LTRESULT CLTModelServer::SetCurAnimTime(HOBJECT hModel, ANIMTRACKERID TrackerID, uint32 curTime) {
    FN_NAME(CLTModelServer::SetCurAnimTime);
    LTRESULT dResult;
    ModelInstance *pInst;

    CHECK_PARAMS2(hModel && hModel->m_ObjectType == OT_MODEL);

    pInst = hModel->ToModel();
    CHECK_PARAMS2(pInst && pInst->GetTracker(TrackerID));

    dResult = ILTModel::SetCurAnimTime(hModel, TrackerID, curTime);
    if (dResult == LT_OK)
    {
        // [2/9/2000] BL: we call trk_SetAtKeyFrame to jump the animation to the specified time
        // in the case that the tracker might not have the AT_PLAYING flag set, and thus may
        // never actually advance itself to this specified time. the client will also make
        // this call when it gets this change info.
//        trk_SetAtKeyFrame(pTracker, pTracker->m_TimeRef.m_Cur.m_Time);
		SetObjectChangeFlags(pInst, CF_FORCEMODELINFO);
    }

    return dResult;
}


// set the animation rate on the ptracker
LTRESULT CLTModelServer::SetAnimRate(HOBJECT hModel, ANIMTRACKERID TrackerID, float fRate) { 
    FN_NAME(CLTModelServer::SetAnimRate);
    LTRESULT dResult ;
    ModelInstance *pInst ;

    CHECK_PARAMS2(hModel && hModel->m_ObjectType == OT_MODEL);

    pInst = hModel->ToModel();

    CHECK_PARAMS2(pInst && pInst->GetTracker(TrackerID));


    dResult = ILTModel::SetAnimRate(hModel, TrackerID, fRate);

    if (dResult == LT_OK)
    {
		SetObjectChangeFlags(pInst, CF_MODELINFO);
    }

    return dResult ;
}

LTRESULT CLTModelServer::SetCurAnim(HOBJECT hModel, ANIMTRACKERID TrackerID, HMODELANIM hAnim) {
    FN_NAME(CLTModelServer::SetCurAnim);
    LTRESULT dResult;
    ModelInstance *pInst;

    CHECK_PARAMS2(hModel && hModel->m_ObjectType == OT_MODEL);

    pInst = hModel->ToModel();
    CHECK_PARAMS2(pInst && pInst->GetTracker(TrackerID));

    dResult = ILTModel::SetCurAnim(hModel, TrackerID, hAnim);
    if (dResult == LT_OK)
    {
		SetObjectChangeFlags(pInst, CF_MODELINFO);
    }

    return dResult;
}

LTRESULT CLTModelServer::ResetAnim(HOBJECT hModel, ANIMTRACKERID TrackerID) {
    FN_NAME(CLTModelServer::ResetAnim);
    LTRESULT dResult;
    ModelInstance *pInst;

    CHECK_PARAMS2(hModel && hModel->m_ObjectType == OT_MODEL);

    pInst = hModel->ToModel();
    CHECK_PARAMS2(pInst && pInst->GetTracker(TrackerID));

    dResult = ILTModel::ResetAnim(hModel, TrackerID);
    if (dResult == LT_OK)
    {
		SetObjectChangeFlags(pInst, CF_FORCEMODELINFO);
    }

    return dResult;
}

LTRESULT CLTModelServer::SetWeightSet(HOBJECT hModel, ANIMTRACKERID TrackerID, HMODELWEIGHTSET hSet) {
    FN_NAME(CLTModelServer::SetWeightSet);
    LTRESULT dResult;
    ModelInstance *pInst;

    CHECK_PARAMS2(hModel && hModel->m_ObjectType == OT_MODEL);

    pInst = hModel->ToModel();
    CHECK_PARAMS2(pInst && pInst->GetTracker(TrackerID));

    dResult = ILTModel::SetWeightSet(hModel, TrackerID, hSet);
    if (dResult == LT_OK)
    {
		SetObjectChangeFlags(pInst, CF_MODELINFO);
    }

    return dResult;
}




// ------------------------------------------------------------------------
// CacheModelDB( filename, out-handle-on-model-database )
// returns LT_OK on success, LT_ERROR on fail.
// 
// ------------------------------------------------------------------------
LTRESULT CLTModelServer::CacheModelDB( const char *filename, HMODELDB & hModelDB )
{
	LTRESULT dResult = LT_ERROR ;
	hModelDB = 0;
	
	dResult = g_pServerMgr->CacheModelFile( filename) ;
	if( dResult == LT_OK )
	{
		Model *pModel = g_ModelMgr.Find( filename );
		if(pModel)
			hModelDB = pModel->m_FileID ;
		else 
			hModelDB = 0;
	}

	return dResult ;
}


// ------------------------------------------------------------------------
// UncacheModelDB( handle-on-model-database )
// returns LT_OK on sucess, LT_ERROR on fail.
// ------------------------------------------------------------------------
LTRESULT CLTModelServer::UncacheModelDB( HMODELDB &hModelDB )
{
	if( hModelDB )
	{
		// find the model 
		Model *pModel = g_ModelMgr.Find( hModelDB );
		if( pModel )
		{
			hModelDB = 0;
			return g_pServerMgr->UncacheModelFile( pModel );
		}
	}

	return LT_ERROR;// nothing done return error.
}

// check the server manager for the model in its lists.
LTRESULT CLTModelServer::IsModelDBLoaded( HMODELDB hModelDB)
{
	if( hModelDB )
	{
		if( g_ModelMgr.Find( hModelDB ) != NULL )
			return LTTRUE ;
	}

	return LTFALSE ;
}


LTRESULT CLTModelServer::AddChildModelDB( HOBJECT hModel, HMODELDB hModelDB )
{
	FN_NAME(CLTModelServer::AddChildModelDB);
	CHECK_PARAMS2(hModel);
	ModelInstance* pModelInstance = hModel->ToModel();
	Model*		   pModelDB;

	CHECK_PARAMS2(pModelInstance);

	// convert modeldb handle into an actual model
	if( hModelDB )
	{
		pModelDB = g_ModelMgr.Find( hModelDB );

		if( pModelDB != NULL ) 
		{
			pModelInstance->AddChildModelDB( pModelDB );
			{
			uint16 parent_id = pModelInstance->GetModelDB()->m_FileID;
			g_pServerMgr->SendChangeChildModel( parent_id, hModelDB );
			return LT_OK;
			}
		}
	}
	
	return LT_ERROR ;  // invalid hmodeldb handle... quit.
}

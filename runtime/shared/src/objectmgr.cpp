#include "bdefs.h"

#include "ltobjectcreate.h"

#include "objectmgr.h"
#include "geomroutines.h"
#include "de_mainworld.h"
#include "animtracker.h"
#include "transformmaker.h"

#include "serverobj.h"
#include "ltengineobjects.h"

#include "ltobjref.h"
#include "polygrid.h"

//ILTRenderStyles
#include "iltrenderstyles.h"
static ILTRenderStyles* renderstyles;
define_holder(ILTRenderStyles, renderstyles);

//ILTModel game interface server instance.
#include "iltmodel.h"
static ILTModel *ilt_model_server;
define_holder_to_instance(ILTModel, ilt_model_server, Server);


#include "ilttransform.h"
static ILTTransform *ilt_transform;
define_holder(ILTTransform, ilt_transform);


// Used for the global frame code.. when it wraps around, all object managers
// have their objects reset.
LTLink g_ObjectMgrs(LTLink_Init);

#define QUAT_EXPAND( q ) q[0], q[1], q[2], q[3] 

// ------------------------------------------------------------------------- //
// ObjectMgr.
// ------------------------------------------------------------------------- //


ObjectMgr::ObjectMgr() :
    m_ObjectBankNormal(32, OBJECT_PREALLOCATIONS),
    m_ObjectBankModel(32, MODEL_PREALLOCATIONS),
    m_ObjectBankWorldModel(32, OBJECT_PREALLOCATIONS),
    m_ObjectBankSprite(32, SPRITE_PREALLOCATIONS),
    m_ObjectBankLight(32, OBJECT_PREALLOCATIONS),
    m_ObjectBankCamera(32, OBJECT_PREALLOCATIONS),
    m_ObjectBankParticleSystem(32, OBJECT_PREALLOCATIONS),
    m_ObjectBankPolyGrid(32, OBJECT_PREALLOCATIONS),
    m_ObjectBankLineSystem(32, OBJECT_PREALLOCATIONS),
    m_ObjectBankContainer(32, OBJECT_PREALLOCATIONS),
    m_ObjectBankCanvas(32, OBJECT_PREALLOCATIONS),
	m_ObjectBankVolumeEffect(32, OBJECT_PREALLOCATIONS)

{

	m_CurFrameCode = 0;
	m_InternalLink.Init();
}


uint32 ObjectMgr::GetFrameCode()
{
    return m_CurFrameCode;
}


uint32 ObjectMgr::IncFrameCode()
{
    LTLink *pListHead, *pCur;
    uint32 i;

    if (m_CurFrameCode == 0xFFFFFFFF)
    {
        m_CurFrameCode = 1;

        // Reset the frame code on all the objects.
        for (i=0; i < NUM_OBJECTTYPES; i++)
        {
            pListHead = &m_ObjectLists[i].m_Head;
            for (pCur=pListHead->m_pNext; pCur != pListHead; pCur=pCur->m_pNext)
            {
                ((LTObject*)pCur->m_pData)->m_WTFrameCode = 0;
            }
        }
    }
    else
    {
        ++m_CurFrameCode;
    }

    return m_CurFrameCode;
}




// ------------------------------------------------------------------------- //
// LTObject.
// ------------------------------------------------------------------------- //

LTObject::LTObject() : WorldTreeObj(WTObj_DObject)
{
    Clear();
    m_ObjectType = OT_NORMAL;
}

LTObject::LTObject(char objectType) : WorldTreeObj(WTObj_DObject)
{
    Clear();
    m_ObjectType = objectType;
}

LTObject::~LTObject()
{
	ASSERT(m_RefList.IsTiedOff());
	
    if (m_pObjectMgr)
    {
        om_RemoveAttachments(m_pObjectMgr, this);
    }

    // Remove it from the moving object lists
	dl_Remove(&cd.m_MovingLink);
	dl_Remove(&cd.m_RotatingLink);

    if (cd.m_hLineSystem)
    {
        // Note : I couldn't find a way to delete this "properly"
        // delete cd.m_hLineSystem;
        cd.m_hLineSystem = LTNULL;
    }

    RemoveFromWorldTree();
}

void LTObject::Init(ObjectMgr *pMgr, ObjectCreateStruct *pStruct)
{
	//sanity checks
	assert(pMgr);
	assert(pStruct);

    // Get stuff out of the struct.
    m_Flags					= pStruct->m_Flags;
    m_Flags2				= pStruct->m_Flags2;
    m_Pos					= pStruct->m_Pos;
    m_Rotation				= pStruct->m_Rotation;
    m_Scale					= pStruct->m_Scale;
	m_GlobalForceOverride	= pStruct->m_GlobalForceOverride;
    m_ObjectType			= (uint8)pStruct->m_ObjectType;
    m_pObjectMgr			= pMgr;
	m_nRenderGroup			= pStruct->m_nRenderGroup;
}

void LTObject::Clear()
{
    // Initialize all its data.
	dl_TieOff(&m_RefList);
    m_Flags = 0;
    m_Flags2 = 0;
    m_UserFlags = 0;
    m_ColorR = m_ColorG = m_ColorB = 255;
    m_ColorA = 255;
    m_Attachments = LTNULL;
    m_Pos.Init();
    m_Rotation.Init();
    m_Scale.Init(1.0f, 1.0f, 1.0f);
	m_GlobalForceOverride.Init();
    m_Radius = 0.0f;
    m_ObjectID = INVALID_OBJECTID;
    m_ObjectType = OT_NORMAL;
    m_pUserData = LTNULL;
    m_Velocity.Init();
    m_Mass = 30; // Default mass is 30..
    m_Dims.Init(1,1,1);

    m_FrictionCoefficient = 5; // Default friction coefficient.
    m_Acceleration.Init();
    m_pStandingOn = LTNULL;
    m_pNodeStandingOn = LTNULL;
    m_MinBox.Init();
    m_MaxBox.Init();
    m_ForceIgnoreLimitSqr = 1000.0f * 1000.0f; // ie: don't tell them unless they change this!
    dl_TieOff(&m_ObjectsStandingOn);
    m_StandingOnLink.m_pData = this;
    sd = LTNULL;
    m_BPriority = 0;
    m_InternalFlags = 0;
	m_nRenderGroup = 0;


    cd.m_ClientFlags = 0;

	// Init the prediction info.
	cd.m_fLastUpdatePosTime = 0.0f;
	cd.m_fLastUpdatePosTime = 0.0f;
	cd.m_LastUpdatePosServer.Init();
	cd.m_LastUpdateVelServer.Init();
	cd.m_MovingLink.m_pData = this;
	dl_TieOff(&cd.m_MovingLink);

    cd.m_fRotAccumulatedTime = 0.0f;
    cd.m_RotatingLink.m_pData = this;
    dl_TieOff(&cd.m_RotatingLink);
    cd.m_fLastUpdatePosTime = 0.0f;
    cd.m_LastUpdatePosServer.Init();
    cd.m_LastUpdateVelServer.Init();

    cd.m_hLineSystem = LTNULL;

}


void LTObject::SetupTransform(LTMatrix &mat) {
    gr_SetupTransformation(&m_Pos, &m_Rotation, &m_Scale, &mat);
}

bool LTObject::InsertSpecial(WorldTree *pTree) {
    // Objects which are camera-relative don't get inserted into the world tree
    if ((m_Flags & FLAG_REALLYCLOSE) != 0) {
        // Insert them into the constant visibility list
        pTree->InsertAlwaysVisObject(this);
        return LTTRUE;
    }
    else {
        return LTFALSE;
    }

}

void LTObject::NotifyObjRefList_Delete()
{
	while (!m_RefList.IsTiedOff())
	{
		LTObjRef *pRef = (LTObjRef *)m_RefList.m_pNext;
		if (pRef->IsTiedOff())
		{
			ASSERT(!"Infinite loop found in object notify list");
			m_RefList.TieOff();
		}
		
		// Remove the link from the list.
		pRef->Remove();

		// Get the hobject this ref holds.
		HOBJECT hObj = *pRef;

		// Get the receiver for this ref.
		ILTObjRefReceiver* pReceiver = pRef->GetReceiver( );

		// Tell the ref that its object is gone.  This will clear the HOBJECT.
		pRef->OnObjDelete();

		// If the ref had a receiver, notify the receiver that the object is gone.
		if( pReceiver )
		{
			// Pass the pointer to the ref and the original hobject.  Since the ref
			// has already been cleared, it will not have information about
			// the hobject by itself.
			pReceiver->OnLinkBroken( reinterpret_cast< LTObjRefNotifier* >( pRef ), hObj );
		}
	}
}

// ------------------------------------------------------------------------- //
// WorldModelInstance.
// ------------------------------------------------------------------------- //

WorldModelInstance::WorldModelInstance()
    : LTObject(OT_WORLDMODEL)
{
    Clear();
}

WorldModelInstance::WorldModelInstance(char objectType)
    : LTObject(objectType)
{
    Clear();
}

WorldModelInstance::~WorldModelInstance()
{
}

void WorldModelInstance::Init(ObjectMgr *pMgr, ObjectCreateStruct *pStruct)
{
    LTObject::Init(pMgr, pStruct);
}

void WorldModelInstance::Clear()
{
    m_ColorR = m_ColorG = m_ColorB = 255;

    m_pOriginalBsp = LTNULL;
    m_pWorldBsp = LTNULL;
    m_pValidBsp = LTNULL;

    m_Transform.Identity();
    m_BackTransform.Identity();
}

bool WorldModelInstance::InsertSpecial(WorldTree *pTree) 
{
    // Physics BSP and vis BSPs go on the root node.
    if (m_pOriginalBsp)
    {
        ASSERT(m_Links[0].m_Link.IsTiedOff());
        ASSERT(m_Links[1].m_Link.IsTiedOff());

        if (m_pOriginalBsp->GetWorldInfoFlags() & WIF_PHYSICSBSP)
        {
            pTree->GetRootNode()->AddObjectToList(&m_Links[0], NOA_Objects);
            return LTTRUE;
        }
        else if (m_pOriginalBsp->GetWorldInfoFlags() & WIF_VISBSP)
        {
			//we don't actually need to insert this object into the world tree at all
            return LTTRUE;
        }
    }
    return LTFALSE;
}

void WorldModelInstance::InitWorldData(const WorldBsp *pOriginalBsp, const WorldBsp *pWorldBsp)
{
	//store the specified BSP's
    m_pOriginalBsp = pOriginalBsp;
    m_pWorldBsp = pWorldBsp;

	//if this is a moveable world model we will have two, the source and current, otherwise
	//just the source, so we need a quick pointer to point to the valid one for using
    m_pValidBsp = m_pWorldBsp ? m_pWorldBsp : m_pOriginalBsp;

	//we also need to setup some flags based upon our world information
	static const uint32 k_nWorldModelFlags = IFLAG_NOTMOVEABLE | IFLAG_MAINWORLDMODEL;

	//clear out any of the old flag values that could be set
	m_InternalFlags &= ~k_nWorldModelFlags;

	//and now setup the new ones

	//see if this world model is the main world model
	if(m_pOriginalBsp->GetWorldInfoFlags() & WIF_MAINWORLD)
		m_InternalFlags |= IFLAG_MAINWORLDMODEL;

	//see if this world model is moveable or not
	if(!m_pWorldBsp)
		m_InternalFlags |= IFLAG_NOTMOVEABLE;
}

HPOLY WorldModelInstance::MakeHPoly(const Node *pNode) 
{
    HPOLY hRet;

    if (m_pWorldBsp)
    {
        hRet = m_pWorldBsp->MakeHPoly(pNode);
        if (hRet != INVALID_HPOLY)
            return hRet;
    }

    // Ok.. try the other BSP.
    return m_pOriginalBsp->MakeHPoly(pNode);
}

// ------------------------------------------------------------------------- //
// ModelInstance.
// ------------------------------------------------------------------------- //

// ------------------------------------------------------------------------
// ModelInstance Constructor
// ------------------------------------------------------------------------
ModelInstance::ModelInstance()
    : LTObject(OT_MODEL)
{
    uint32 i;
	
    trk_Init(&m_AnimTracker, NULL, 0);

    m_pNodeInfo		= NULL;
	m_nNumNodeInfos	= 0;

	//note that the hidden pieces bit field relies upon the maximum number of pieces being
	//divisible by the size of a hidden pieces component.
	ASSERT(MAX_PIECES_PER_MODEL % 32 == 0);

	for(uint32 nCurrPiece = 0; nCurrPiece < MAX_PIECES_PER_MODEL / 32; nCurrPiece++)
	{
		m_HiddenPieces[nCurrPiece] = 0;
	}

	// animations
    m_AnimTrackers = &m_AnimTracker;
    m_AnimTracker.m_Link.m_pNext = NULL;
    m_AnimTracker.SetModelInstance(this);
    m_AnimTracker.m_ID      = MAIN_TRACKER;
	m_AnimTracker.m_Flags  |= AT_ALLOWINVALID; // until model is bound ... 

	// textures & renderstyles
    for (i = 0; i < MAX_MODEL_TEXTURES; i++)
	{
        m_pSprites[i] = LTNULL;
		m_pSkins[i] = LTNULL;
	}
	m_nNumSprites = 0;
    
    for (i = 0; i < MAX_MODEL_RENDERSTYLES; i++)
        m_pRenderStyles[i] = NULL;

	// Cached Transforms 
	m_CachedTransforms			= NULL;
	m_CachedTransformInfo		= NULL;
	m_RenderingTransforms		= NULL;

    m_LastDirLightAmount		= -1.0f;
	m_nRenderInfoIndex			= INVALID_MODEL_INFO_INDEX;
	m_nRenderInfoParentIndex	= INVALID_MODEL_INFO_INDEX;

	m_StringKeyCallBack			= NULL ;

#if(MODEL_OBB)
		// Oriented bounding box stuff from modeldb
 	m_NumOBBs =0; 
 	m_ModelOBBs=NULL; // base/cached transformed 
#endif // MODEL_OBB

}

ModelInstance::~ModelInstance()
{
	//clear out any node controls installed on the model
	FreeNodeInfo();
	
	// get rid of Cached Transforms 
	DisableTransformCache();
}


void ModelInstance::InitAnimTrackers( uint16 at_flags )
{
	// set up anim trackers 
	for ( LTAnimTracker *pCurTracker=m_AnimTrackers; pCurTracker; pCurTracker=pCurTracker->GetNext())
    {
		trk_Init(pCurTracker, GetModelDB(), 0);
		pCurTracker->m_Flags |= at_flags;
	}
}

// ------------------------------------------------------------------------
// BindModelDB()
// associate a model db with this engine object.
// ------------------------------------------------------------------------
void ModelInstance::BindModelDB( Model *pNewModel )
{
	// if we are binding a different model
	if( pNewModel != GetModelDB() )
	{
		// get rid of the old
		UnbindModelDB();
		pNewModel->AddRef(); // increment usage.
		
		DEBUG_MODEL_REZ(("model-rez: model-instance(%s) bind %s", 
							(GetCSType()?"s":"c"),
							pNewModel->GetFilename() ));
		
		// setup trackers
		m_AnimTracker.SetModel(pNewModel);
		// set up anim trackers 
		InitAnimTrackers();
		SetupNodeInfo();
		EnableTransformCache();
#if(MODEL_OBB)
		EnableCollisionObjects();
#endif
	}
}

// ------------------------------------------------------------------------
// UnbindeModelDB
// unbind the current model from this engine object.
// ------------------------------------------------------------------------
void ModelInstance::UnbindModelDB()
{
	LTAnimTracker* pTracker = m_AnimTrackers->GetNext();

	while( pTracker )
	{
		LTAnimTracker* pTemp = pTracker;
		pTracker = pTracker->GetNext();
		sb_Free(&m_pObjectMgr->m_TrackerBank, pTemp);
	}

	// Get the model
	Model *pModel = GetModelDB();
	// Stop pointing at it
	m_AnimTrackers->SetNext(LTNULL);
	m_AnimTracker.m_TimeRef.m_pModel = LTNULL;
	
	// delete if necessary.
	if (pModel)
	{
		DEBUG_MODEL_REZ(("model-rez: model-instance(%s) unbind %s",
						(GetCSType()?"s":"c"), 
						pModel->GetFilename() )) ;

		pModel->Release(); // modelinstance is not using it any more.
		pModel = NULL ;
	}

	//free up all the node control data
	FreeNodeInfo();
	
	// get rid of Cached Transforms 
	DisableTransformCache();

#if(MODEL_OBB)
	DisableCollisionObjects();
#endif
}


// ------------------------------------------------------------------------
// Update this object.
// ------------------------------------------------------------------------
void ModelInstance::ClientUpdate( uint32 msFrameTime )
{
	LTAnimTracker *pTracker;

	// Update model animations.
    for (pTracker = m_AnimTrackers; pTracker; pTracker=pTracker->GetNext())
    {
        pTracker->m_StringKeyCallback = m_StringKeyCallBack;
        trk_Update(pTracker, msFrameTime);
    }

	//we need to reset all the transforms so they will be re-evaluated
	ResetCachedTransformNodeStates();
}


void ModelInstance::ServerUpdate( uint32 msFrameTime )
{
	LTAnimTracker *pTracker;

	// Do MovementEncoding.  Should do hint before tracker update so that updating
	// the iframe is the last thing we do in the frame, since everything else up to this
	// point has been using the previous iframe.
	DoMoveHint(m_AnimTrackers);

    // Update model animations.
    for (pTracker=m_AnimTrackers; pTracker; pTracker=pTracker->GetNext())
    {
	   pTracker->m_StringKeyCallback = m_StringKeyCallBack;
     
       trk_Update(pTracker, msFrameTime);
    }

	//we need to reset all the transforms so they will be re-evaluated
	ResetCachedTransformNodeStates();
}



bool ModelInstance::GetNodeTransform( uint32 iNode, LTransform &transform, bool bInWorldSpace )
{
	if( iNode > GetModelDB()->NumNodes() )
		return false ;

	LTMatrix res ;

	if( !GetCachedTransform( iNode, res  ) )
		return false ;
	
	// if we want it in local space, undo the root node.
	if( !bInWorldSpace ) 
	{
		LTMatrix selfpos ;
		LTMatrix tmp;
		SetupTransform(selfpos);
		selfpos.Inverse();
		tmp = selfpos * res ;
		res = tmp;		
	}
	
	// [RP] 11/7/02 - In order to preserve propper orientation on a scaled model we need to
	//		normalize the basis vectors of the transform before converting it to a quaternion.
	
	LTVector r0, r1, r2;
	res.GetBasisVectors( &r0, &r1, &r2 );
	res.SetBasisVectors2( &r0.Normalize(), &r1.Normalize(), &r2.Normalize() );
	
	// convert to an "ltransform"
	Mat_GetTranslation(res, transform.m_Pos);
    quat_ConvertFromMatrix((float*)&transform.m_Rot.m_Quat, res.m);
	transform.m_Scale.Init(1.0f,1.0f,1.0f);

	return true; 
}
	
bool ModelInstance::GetNodeTransform( uint32 iNode, LTMatrix &mat , bool bInWorldSpace )
{
	if( iNode > GetModelDB()->NumNodes() )
		return false ;

	if( !GetCachedTransform( iNode, mat ) )
		return false ;

	// if we want it in local space, undo the root node.
	if( !bInWorldSpace ) 
	{
		LTMatrix selfpos ;
		SetupTransform(selfpos);
		selfpos.Inverse();

		// res = root * res ;
		mat = selfpos * mat ;
	}

	return true ;
}


bool ModelInstance::GetSocketTransform( uint32 iSock, LTransform &tf, bool bInWorldSpace )
{
	if( iSock >= GetModelDB()->NumSockets() )
	{
		// GetSocketTransform is called but we don't really have a socket. 
		// Perhaps we've encoded the inode by adding num sockets to it. (see si_CreateAttachment )
		return GetNodeTransform( iSock - GetModelDB()->NumSockets(), tf, bInWorldSpace );
	}

	// GetSocket transform
	ModelSocket *pSocket = GetModelDB()->GetSocket(iSock);
	LTransform tfNode;
	
	// Get socket's node transform (checking that the node is valid)
	if(!GetNodeTransform( pSocket->m_iNode, tfNode, bInWorldSpace ))
		return false ;


	// concatenate the node transform with the socket's
	// tf =  tfNode * pSocket 
	tf.m_Rot = tfNode.m_Rot * pSocket->m_Rot ;
 	quat_RotVec( &tf.m_Pos.x, tfNode.m_Rot.m_Quat, &pSocket->m_Pos.x);
	
	// [RP] 11/7/02 - In order to preserve propper positioning on a scaled model we need to
	//		scale the sockets offset by the scale of the model. 

	tf.m_Pos = tf.m_Pos * m_Scale;

	tf.m_Pos += tfNode.m_Pos ;
	tf.m_Scale = pSocket->m_Scale ;

	return true ;
}

bool ModelInstance::GetSocketTransform( uint32 iSock, LTMatrix *mat, bool bInWorldSpace )
{
	if( iSock >= GetModelDB()->NumSockets() )
	{
		// GetSocketTransform is called but we don't really have a socket. 
		// Perhaps we've encoded the inode by adding num sockets to it. (see si_CreateAttachment )
		return GetNodeTransform( iSock - GetModelDB()->NumSockets(), *mat, bInWorldSpace) ;
	}

	// GetSocket transform
	ModelSocket *pSocket = GetModelDB()->GetSocket(iSock);
	LTMatrix mNode, mSock;

	// Get socket's node transform (checking that the node is valid)
	if(!GetNodeTransform( pSocket->m_iNode, mNode, bInWorldSpace ))
		return false ;

	pSocket->m_Rot.ConvertToMatrix(mSock);
	mSock.SetTranslation(pSocket->m_Pos);


	MatMul(mat, &mNode, &mSock );

	return true ;
}


// Movement encoding hints.
void ModelInstance::DoMoveHint( LTAnimTracker* pTracker)
{
	if ( pTracker->m_hHintNode != INVALID_MODEL_NODE )
	{
		LTransform tf;

		if( GetNodeTransform(pTracker->m_hHintNode, tf, LTFALSE ) )
		{
			if ( (pTracker->m_hLastHintAnim == pTracker->m_TimeRef.m_Cur.m_iAnim) &&
				 (pTracker->m_TimeRef.m_Cur.m_iFrame >= pTracker->m_dwLastHintTime) )
			{

				// jeffo
				// LastPercent == 0 if we just wrapped a looping animation.
				// Do not calculate the difference after looping, because that
				// will result in a large bogus difference.

				LTransform tfDiff;
				ilt_transform->Difference(tfDiff, tf, pTracker->m_tfLastHint);

				LTMatrix mMat;
				RotationToMatrix(&m_Rotation, &mMat);
				mMat.Apply3x3(tfDiff.m_Pos);

				ServerObjToBaseClass(this)->EngineMessageFn(MID_TRANSFORMHINT, &tfDiff, (float)pTracker->m_hHintNode);
			}
			else // we are between frames or animation. still we need some hint.
			{
				LTransform tfDiff;
				tfDiff.m_Rot = tf.m_Rot;

				LTMatrix mMat;
				RotationToMatrix(&m_Rotation, &mMat);
				mMat.Apply3x3(tf.m_Pos, tfDiff.m_Pos);

				ServerObjToBaseClass(this)->EngineMessageFn(MID_TRANSFORMHINT, &tfDiff, (float)pTracker->m_hHintNode);
			}

			pTracker->m_dwLastHintTime = pTracker->m_TimeRef.m_Cur.m_iFrame;
			pTracker->m_hLastHintAnim = pTracker->m_TimeRef.m_Cur.m_iAnim;
			pTracker->m_tfLastHint = tf;
		}
	}
}


// ------------------------------------------------------------------------
// HasNodeControlFn
// Returns whether or not a node control function is associated with the
// specified node
// ------------------------------------------------------------------------
bool ModelInstance::HasNodeControlFn( HMODELNODE hNode )
{
	assert(m_pNodeInfo);
	assert(hNode < m_nNumNodeInfos);
	return (m_pNodeInfo[hNode].m_pNodeControls != NULL);
}

// ------------------------------------------------------------------------
//adds a node control function for a single node
// ------------------------------------------------------------------------
void ModelInstance::AddNodeControlFn( HMODELNODE hNode, NodeControlFn pFn, void *pUserData)
{
	assert(m_pNodeInfo);
	assert(hNode < m_nNumNodeInfos);

	//allocate the data
	SNodeControlInfo* pInfo;
	LT_MEM_TRACK_ALLOC(pInfo = new SNodeControlInfo,LT_MEM_TYPE_OBJECT);

	if(!pInfo)
		return;

	//setup the information
	pInfo->m_ControlFn = pFn;
	pInfo->m_pUserData = pUserData;
	pInfo->m_pNext		= m_pNodeInfo[hNode].m_pNodeControls;

	//add it into the list
	m_pNodeInfo[hNode].m_pNodeControls = pInfo;
}

// ------------------------------------------------------------------------
//adds a node control function for the entire model (all nodes will have this function called)
// ------------------------------------------------------------------------
void ModelInstance::AddNodeControlFn( NodeControlFn pFn, void *pUserData)
{
	//we need to add a control function to every node
	for(uint32 nCurrNode = 0; nCurrNode < m_nNumNodeInfos; nCurrNode++)
	{
		AddNodeControlFn(nCurrNode, pFn, pUserData);
	}
}

// ------------------------------------------------------------------------
//removes a node control function from the specified node. If the user data is NULL, it will
//remove all that match the function and will not check the user data
// ------------------------------------------------------------------------
void ModelInstance::RemoveNodeControlFn( HMODELNODE hNode, NodeControlFn pFn, void* pUserData)
{
	// Make sure we have nodecontrols to remove.
	if( !m_pNodeInfo || hNode >= m_nNumNodeInfos )
	{
		ASSERT( !"ModelInstance::RemoveNodeControlFn: No nodecontrols to remove." );
		return;
	}

	//run through all the registered control functions and remove any that
	//fit the criteria
	SNodeControlInfo* pCurr = m_pNodeInfo[hNode].m_pNodeControls;
	SNodeControlInfo* pPrev = NULL;

	//apply it in sequence
	while(pCurr)
	{
		SNodeControlInfo* pNext = pCurr->m_pNext;

		//see if the functions matches and thenwe need to check the data,
		//and if the incoming data is null, it acts as a wildcard so any
		//data will be accepted
		if((pFn == pCurr->m_ControlFn) && (!pUserData || (pUserData == pCurr->m_pUserData)))
		{
			//we have a complete match, we need to remove it

			//correct the pointers in the previous element (or the head if none)
			if(pPrev)
				pPrev->m_pNext = pNext;
			else
				m_pNodeInfo[hNode].m_pNodeControls = pNext;

			//free the memory
			delete pCurr;
		}
		else
		{
			//valid function, update our previous
			pPrev = pCurr;
		}

		//onto the next..
		pCurr = pNext;
	}
}

// ------------------------------------------------------------------------
//removes a node control function from all nodes. If the user data is NULL, it will
//remove all that match the function and will not check the user data
// ------------------------------------------------------------------------
void ModelInstance::RemoveNodeControlFn( NodeControlFn pFn, void* pUserData)
{
	//we need to remove a control function to every node
	for(uint32 nCurrNode = 0; nCurrNode < m_nNumNodeInfos; nCurrNode++)
	{
		RemoveNodeControlFn(nCurrNode, pFn, pUserData);
	}
}


// ------------------------------------------------------------------------
//applies the series of node control functions to the specified matrix
// ------------------------------------------------------------------------
void ModelInstance::ApplyNodeControl( const NodeControlData& Data )
{
	assert(m_pNodeInfo);
	assert(Data.m_hNode < m_nNumNodeInfos);

	//get the current transform
	SNodeControlInfo* pCurr = m_pNodeInfo[Data.m_hNode].m_pNodeControls;

	//apply it in sequence
	while(pCurr)
	{
		//apply the transform
		pCurr->m_ControlFn(Data, pCurr->m_pUserData);

		//and onto the next one...
		pCurr = pCurr->m_pNext;
	}
}

// ------------------------------------------------------------------------
// More node query stuff
// returns parent node index or INVALIDNODE
// ------------------------------------------------------------------------
uint32 ModelInstance::NodeGetParent( uint32 child_index )
{
	// child_index >= 0  and < numnodes
	uint32 ret = GetModelDB()->GetNode(child_index)->GetParentNodeIndex();
	return ret;
}

uint32 ModelInstance::NodeGetChild( uint32 iNode, uint32 child_index )
{
	// if inode >=0 and < numnodes 
	// and child_index >= 0 and < iNode's numchildren
	return GetModelDB()->GetNode(iNode)->GetChild(child_index)->m_NodeIndex ;
}

uint32 ModelInstance::NodeGetNumChildren( uint32 iNode )
{
	// if inode >= 0 and < numnodes
	return GetModelDB()->GetNode(iNode)->NumChildren();
}

uint32 ModelInstance::NodeGetRootIndex()
{
	return GetModelDB()->GetRootNode()->m_NodeIndex ;
}

// ------------------------------------------------------------------------
// AddChildModel( pNewChildModel )
// ------------------------------------------------------------------------
bool ModelInstance::AddChildModelDB(  Model *pNewChildModel )
{
	Model *pParentModel = GetModelDB();
	
	if( pParentModel )
	{
		char err_str[256];
		const char *par_filename = pParentModel->GetFilename();
		const char *filename = pNewChildModel->GetFilename();
		
		if (pParentModel->AddChildModel(pNewChildModel, 
										const_cast<char*>(filename),
										err_str, sizeof(err_str)))
		{
			DEBUG_MODEL_REZ(("model-rez: add-childmodel %s -> %s ", pNewChildModel->GetFilename(), pParentModel->GetFilename()));
			return true;
		}else
		{
			DEBUG_MODEL_REZ(("model-rez: add-childmodel warning: %s %s -> %s" ,err_str, pNewChildModel->GetFilename(), pParentModel->GetFilename()));
			// we failed, send some report.
			return false ; 	
		}
	}

	return false ;
}

// ------------------------------------------------------------------------
//counts the number of anim trackers associated with this object
// ------------------------------------------------------------------------
uint32 ModelInstance::GetNumAnimTrackers() const
{
	//just run through the list and count
	uint32 nTotal = 0;
	for(LTAnimTracker* pCurr = m_AnimTrackers; pCurr; pCurr = pCurr->GetNext())
		nTotal++;

	return nTotal;
}


// ------------------------------------------------------------------------
// EnableTransformCache()
// create space for animation data.
// ------------------------------------------------------------------------
void ModelInstance::EnableTransformCache()
{
	//make sure to clear out any previous transform caches
	DisableTransformCache();

	// allocate as much space as there are m_FlatNodeLists
	// take the current node control and cache it. replace the 
	// original node control with our own that handles
	// 
	uint32 nNumNodes = NumNodes();

	// allocate some space for transforms and needed associated info.
	LT_MEM_TRACK_ALLOC( m_CachedTransforms = new LTMatrix [ nNumNodes ], 
						LT_MEM_TYPE_OBJECT);

	LT_MEM_TRACK_ALLOC( m_CachedTransformInfo= new SCachedTransformInfo [ nNumNodes ], 
						LT_MEM_TYPE_OBJECT);


	// only allocate rendering transforms if on the client.
	if( GetCSType() == ClientType )
	{
		LT_MEM_TRACK_ALLOC( m_RenderingTransforms= new DDMatrix [ nNumNodes ], 
							LT_MEM_TYPE_OBJECT);
	}

	ResetCachedTransformNodeStates();
}


// ------------------------------------------------------------------------
// DisableTransformCache()
// delete the transform cache
// ------------------------------------------------------------------------
void ModelInstance::DisableTransformCache()
{
	// remove the cached transforms.
	delete [] m_CachedTransforms;
	m_CachedTransforms = NULL ;

	delete [] m_CachedTransformInfo;
	m_CachedTransformInfo= NULL ;

	delete [] m_RenderingTransforms;
	m_RenderingTransforms = NULL ;
}

// ------------------------------------------------------------------------
//Allocates all the node control data after a model has been setup
// ------------------------------------------------------------------------
void ModelInstance::SetupNodeInfo()
{
	//clear out any old stuff...
	FreeNodeInfo();

	//make sure we have a model
	assert(GetModelDB() && "No model associated with instance in call SetupNodeControlData");

	uint32 nNumNodes = GetModelDB()->NumNodes();

	//allocate an info structure for each node
	LT_MEM_TRACK_ALLOC(m_pNodeInfo = new SNodeInfo [nNumNodes],LT_MEM_TYPE_OBJECT);

	//now clear out the entire list
	memset(m_pNodeInfo, 0, sizeof(SNodeInfo) * nNumNodes);	

	//success
	m_nNumNodeInfos = nNumNodes;
}

// --------------------------------------- ---------------------------------
//Frees up all allocated data for the node control functions
// ------------------------------------------------------------------------
void ModelInstance::FreeNodeInfo()
{
	//run through the list and remove all elements
	for(uint32 nCurrNode = 0; nCurrNode < m_nNumNodeInfos; nCurrNode++)
	{
		//delete the chain
		SNodeControlInfo* pCurr = m_pNodeInfo[nCurrNode].m_pNodeControls;

		while(pCurr)
		{
			SNodeControlInfo* pNext = pCurr->m_pNext;
			delete pCurr;
			pCurr = pNext;
		}
	}

	//just clean up the old data
	delete [] m_pNodeInfo;

	//clear everything out
	m_pNodeInfo		= NULL;
	m_nNumNodeInfos	= 0;
}

//  ----------------------------------------------------------------
//  GetLODValFromDist
// given a distance value get the lod value.
//  ----------------------------------------------------------------
bool ModelInstance::GetLODValFromDist( uint32 piece_index, float dist, uint32 &lod )
{
    if (!m_AnimTrackers)
        return false;
    lod = 0;
    
    if( piece_index < GetModelDB()->NumPieces() )
    {
        ModelPiece *pPiece = GetModelDB()->GetPiece( piece_index );
        lod = pPiece->GetLODIndexFromDist(dist);
    }
    // bad piece index value;
    return false;
}

// ----------------------------------------------------------------
// get the number of lods on this piece.
// ----------------------------------------------------------------
bool ModelInstance::GetNumLOD( uint32 piece_index, uint32 & num_lods ) 
{
    num_lods = 0;
    
    if (!m_AnimTrackers)
        return false;

    if( piece_index < GetModelDB()->NumPieces() )
    {
        ModelPiece *pPiece = GetModelDB()->GetPiece(piece_index );
        num_lods =  pPiece->NumLODs() ;
        return true ;
    }
    
    return false ;
}


// ------------------------------------------------------------------------
// ForceUpdateCachedTransforms()
// This udpates the xforms no matter what.
// ------------------------------------------------------------------------
bool ModelInstance::ForceUpdateCachedTransforms()
{
	TransformMaker tMaker ;
	LTMatrix	   mToWorld;
    LTAnimTracker *pCur;

	SetupTransform(mToWorld);

    tMaker.m_nAnims = 0;
    for (pCur=m_AnimTrackers; pCur; pCur=(LTAnimTracker*)pCur->m_Link.m_pNext)
    {
        if (tMaker.m_nAnims >= MAX_GVP_ANIMS)
            return false;

        tMaker.m_Anims[ tMaker.m_nAnims ] = pCur->m_TimeRef;
        tMaker.m_nAnims++;
    }

	tMaker.m_iMoveHintNode	= m_AnimTrackers->m_hHintNode ;
	tMaker.m_pInstance		= this;
	tMaker.m_pStartMat		= &mToWorld; 
	tMaker.m_pOutput		= m_CachedTransforms;
	
	
	if (!tMaker.SetupTransforms()) 
	{
		dsi_ConsolePrint("ModelInstance::ForceUpdateCachedTransforms failed for %s.", GetModelDB()->GetFilename());
		return false; 
	}

	// since we have just recalculated all the nodes, set them all to evaluated and not needing
	//evaluation
	for( uint32 iNode = 0 ; iNode < GetModelDB()->NumNodes() ; iNode++ )
	{
		SetNodeEvaluated(iNode, true);
		SetShouldEvaluateNode(iNode, false);
	}

	return true ;
}


bool ModelInstance::GetCachedTransform( uint32 iNode, LTMatrix &transform )	
{ 
	if( iNode < GetModelDB()->NumNodes() ) 
	{ 
		// first check if its already evaluated.
		if( !IsNodeEvaluated(iNode) ) 
		{
			//indicate that we need to evaluate this node
			SetupNodePath( iNode );

			//and evaluate the tree
			UpdateCachedTransformsWithPath();	
		}

		//copy out the transform
		transform = m_CachedTransforms[ iNode ]; 
		return true; 
	}
	else 
	{
		//invlaid node ID, just give them the identity
		transform.Identity();
		return false; 
	}
}

// ------------------------------------------------------------------------
// GetRenderingTransforms()
// get the matrix for use in rendering of the current animation state.
// ------------------------------------------------------------------------
DDMatrix*	ModelInstance::GetRenderingTransforms()
{ 
	UpdateCachedTransformsWithPath();	
	
	// only do this if we're the client
	if( GetCSType() == ClientType )
	{
		// Apply the inverse global transform of the bind pos to create 
		// a difference transform between bind pose and animated pose. 
		// This is what d3d needs.
		for (uint32 i=0; i < GetModelDB()->NumNodes(); ++i) 
		{
			// if we need to update rendering transform do it.
			// if not already done and eval'd by transform maker.
			if( !IsNodeEvaluatedRendering(i) && IsNodeEvaluated(i) )
			{
				// render trans = anim-global-result * inverse of base pose ; 
				// thus getting the diff
				Convert_DItoDD( m_CachedTransforms[i] * GetModelDB()->GetNode(i)->GetInvGlobalTransform(),
					            m_RenderingTransforms[i]); 

				SetNodeEvaluatedRendering(i, true);
			}
		}
	}

	return m_RenderingTransforms ; 
}
	
// ------------------------------------------------------------------------
// ResetCachedTransformNodeStates()
// mark all nodes as unevaluated & off the eval-path for the current frame
// code. 
// ------------------------------------------------------------------------
void ModelInstance::ResetCachedTransformNodeStates()
{
	uint32 nNumNodes = NumNodes();

	// reset every node to ignore/not-on-path
	for( uint32 nCurrNode = 0 ; nCurrNode < nNumNodes ; nCurrNode++ )
	{
		SetNodeEvaluated(nCurrNode, false);
		SetNodeEvaluatedRendering(nCurrNode, false);
	}
}


// ------------------------------------------------------------------------
// UpdateCachedTransformsWithPath() 
// evaluate a path through hierarchy evaluating nodes till terminus is reached.
// ------------------------------------------------------------------------
bool ModelInstance::UpdateCachedTransformsWithPath()
{	
	TransformMaker		tMaker;
	LTAnimTracker		*pCur;
	LTMatrix			mStartTransform;

	// create the transform that's the current pos/orient
	SetupTransform(mStartTransform);

	tMaker.m_nAnims = 0;
	// animations to update.
	for (pCur=m_AnimTrackers; pCur; pCur=(LTAnimTracker*)pCur->m_Link.m_pNext)
	{
		ASSERT( tMaker.m_nAnims <= MAX_GVP_ANIMS );
			
		tMaker.m_Anims[ tMaker.m_nAnims ] = pCur->m_TimeRef;
		tMaker.m_nAnims++;
	}

	// setup tmaker with hintnode, this, root of xform hier.
	// where to put concatenation results.
	tMaker.m_iMoveHintNode = m_AnimTrackers->m_hHintNode ;
	tMaker.m_pInstance	   = this;
	tMaker.m_pStartMat     = &mStartTransform; 
	tMaker.m_pOutput       = m_CachedTransforms;

	// evaluate the hierarchy with paths.
	if (!tMaker.SetupTransformsWithPath()) 
	{
		dsi_ConsolePrint("ModelInstance::UpdateCachedTransforms failed for %s. Possible missing weightset(s)", GetModelDB()->GetFilename());
		return false; 
	}
	
	return true;
}


// ------------------------------------------------------------------------
// SetupNodePath( target-node )
// sets up the node path from taget-node to the root.
// notes :
// if this node has never been evaluated, do so.
// if the parent is on the path, and has already been evaluated put it back 
// on the path. Its possible that this func will be called serveral times in 
// a frame, but we want to restablish the path regardless of eval state of nodes,
// unless target-node has already been evaluated.
// Since this func does not reset the cached transform states, make sure they 
// are correct before calling this func.
// ------------------------------------------------------------------------
void ModelInstance::SetupNodePath( uint32 iNode )
{
	//make sure that we have a valid model database
	assert(GetModelDB());

	//make sure that the node is within range
	if(iNode >= NumNodes())
		return;

	//has this node already been evaluated already?
	if(IsNodeEvaluated(iNode))
	{
		//this node is already setup for evaluation, so we don't need to proceed
		return;
	}

	//we now need to go up the node tree and tag all elements as needing to be evaluated
	//even if they are already evaluated (so that the recursion algorithm knows which branches
	//to go down)
	while(iNode != NODEPARENT_NONE)
	{
		//if this node is already set to be evaluated we can just bail
		if(ShouldEvaluateNode(iNode))
			break;

		//set this node so it will be checked by the evaluator
		SetShouldEvaluateNode(iNode, true);

		//recurse up the tree
		iNode = GetModelDB()->GetNode(iNode)->GetParentNodeIndex();
	}
}


// ------------------------------------------------------------------------
// SetPieceLODNodePath( CDIModelDrawable *pLOD)
// sets up the node path for a model piece's lod.
// this should be a very small number of nodes, since it is just the most
// distal nodes of the mesh that are being asked to be put on the path.
// ------------------------------------------------------------------------
void ModelInstance::SetupLODNodePath( CDIModelDrawable *pLOD )
{
	// put every node used by this mesh on the the evaluation path.
	for( uint32 iNodeCnt = 0 ; iNodeCnt < pLOD->m_UsedNodeListSize ; iNodeCnt++ )
	{
		SetupNodePath( pLOD->m_pUsedNodeList[ iNodeCnt ] );
	}
}

LTAnimTracker*  ModelInstance::GetTracker(ANIMTRACKERID TrackerID)
{
    if (TrackerID == MAIN_TRACKER)
    {
        return &m_AnimTracker;
    }

    LTAnimTracker *pCur;

    if (!m_AnimTrackers)
        return NULL;

    for (pCur=m_AnimTrackers; pCur; pCur=pCur->GetNext())
    {
        if (pCur->m_ID == TrackerID)
        {
            return pCur;
        }
    }

    return NULL;
}

LTAnimTracker*  ModelInstance::GetTracker(ANIMTRACKERID TrackerID, LTAnimTracker** &pPrev)
{
    pPrev = NULL;
    if (TrackerID == MAIN_TRACKER)
    {
        return &m_AnimTracker;
    }

    LTAnimTracker *pCur;
    pPrev = &m_AnimTrackers;

    if (!m_AnimTrackers)
        return NULL;

    for (pCur=m_AnimTrackers; pCur; pCur=pCur->GetNext())
    {
        if (pCur->m_ID == TrackerID)
        {
            return pCur;
        }
        pPrev = (LTAnimTracker**)&pCur->m_Link.m_pNext;
    }

    return NULL;
}

bool ModelInstance::SetRenderStyle(uint32 iIndex, CRenderStyle* pRenderStyle) 
{
    assert(iIndex < MAX_MODEL_RENDERSTYLES);

    pRenderStyle->IncRefCount();                // Keep our ref counts up to date (the old one can get free'd this way if no-one is using it)...
    if (m_pRenderStyles[iIndex]) renderstyles->FreeRenderStyle(m_pRenderStyles[iIndex]); 

    m_pRenderStyles[iIndex] = pRenderStyle;
    return true;
}

bool ModelInstance::GetRenderStyle(uint32 iIndex, CRenderStyle** ppRenderStyle) 
{
    assert(iIndex < MAX_MODEL_RENDERSTYLES);

    *ppRenderStyle = m_pRenderStyles[iIndex];
    return true;
}


 // --------------------------------------------------------
 // Collision Objects 
 // --------------------------------------------------------
 
#if(MODEL_OBB)
 // create instance of obb data from model-database
 void ModelInstance::EnableCollisionObjects()
 {
 	// get the obbs from the model-db
 	// allocate double that one for original and one for transformed
 
 	if( m_NumOBBs ) // free it if we have it.
	{
		DisableCollisionObjects();
	}
 
 	Model *pModel = GetModelDB();
 	m_NumOBBs = pModel->GetNumOBB();
 	
	if(m_NumOBBs > 0)
	{
		LT_MEM_TRACK_ALLOC(m_ModelOBBs = new ModelOBB [m_NumOBBs], LT_MEM_TYPE_OBJECT); 	
		pModel->GetCopyOfOBBSet( m_ModelOBBs);
	}
 }

// ------------------------------------------------------------------------
// 
// ------------------------------------------------------------------------
void ModelInstance::DisableCollisionObjects()
{
	// free all the data.
	if( m_NumOBBs > 0)
	{
		delete [] m_ModelOBBs;
		m_ModelOBBs = NULL;
		m_NumOBBs = 0;
	}
}
 
// ------------------------------------------------------------------------
// Update()
// transform the obbs using the current animation state.
// ------------------------------------------------------------------------
void ModelInstance::UpdateCollisionObjects( ModelOBB *user_obbs )
{
	// from either the cached or new transforms, transform the obbs according to 
	// the current transform state.
	LTMatrix obb_mat, tmp_mat ;
	ModelOBB *pOBB ;
	
	uint32    num_obb        = m_NumOBBs;
	uint32    obb_cnt ; 
	// transform source obb to result obb using current
	// current global transforms.

	// first update the transforms 
	for( obb_cnt = 0 ; obb_cnt < num_obb ; obb_cnt++ )
	{
		SetupNodePath( m_ModelOBBs[ obb_cnt ].m_iNode );
	}

	UpdateCachedTransformsWithPath();

	for( obb_cnt = 0 ; obb_cnt < num_obb ; obb_cnt++ )
	{
	// get obb xform (basis+pos) from modeldb
		pOBB = &	m_ModelOBBs[ obb_cnt ];
		LTMatrix &xform_mat = m_CachedTransforms[ pOBB->m_iNode ];
		obb_mat.Identity();
		obb_mat.SetBasisVectors( &pOBB->m_Basis[0],
								 &pOBB->m_Basis[1],
								 &pOBB->m_Basis[2]);
		obb_mat.SetTranslation( pOBB->m_Pos );

		// original obb xform * mat = new transform ;
		tmp_mat = xform_mat * obb_mat ;
		tmp_mat.GetBasisVectors( &user_obbs[obb_cnt].m_Basis[0], 
								 &user_obbs[obb_cnt].m_Basis[1],
								 &user_obbs[obb_cnt].m_Basis[2]);
		tmp_mat.GetTranslation( user_obbs[obb_cnt].m_Pos );
		user_obbs[obb_cnt].m_Size = pOBB->m_Size ;
		user_obbs[obb_cnt].m_iNode= pOBB->m_iNode ;
	}
}

void ModelInstance::UpdateCollisionObject( uint32 obb_index , ModelOBB &user_obb)
{
	// ditch if the index is wrong...
	if( obb_index >= m_NumOBBs || obb_index < 0 ){
		ASSERT(0) ; // wrong index value
		return ;
	}
	
	ModelOBB *pOBB ;
	LTMatrix obb_mat, tmp_mat ;
	
	SetupNodePath( m_ModelOBBs[ obb_index ].m_iNode );
	UpdateCachedTransformsWithPath();
	
	// get obb xform (basis+pos) from modeldb
	pOBB = &	m_ModelOBBs[ obb_index ];
	LTMatrix &xform_mat = m_CachedTransforms[ pOBB->m_iNode ];
	obb_mat.Identity();
 	obb_mat.SetBasisVectors( &pOBB->m_Basis[0], &pOBB->m_Basis[1], &pOBB->m_Basis[2]);
	obb_mat.SetTranslation( pOBB->m_Pos );
	
	// original obb xform * mat = new transform ;
	tmp_mat = xform_mat * obb_mat ;
	tmp_mat.GetBasisVectors( &user_obb.m_Basis[0],
	 							 &user_obb.m_Basis[1],
	 							 &user_obb.m_Basis[2]);
	tmp_mat.GetTranslation( user_obb.m_Pos );
	 
	user_obb.m_Size = pOBB->m_Size ;
	user_obb.m_iNode= pOBB->m_iNode;
}

// ------------------------------------------------------------------------
// Gives unto caller a copy of the collision objects.
// ------------------------------------------------------------------------
void ModelInstance::GetCollisionObjects( ModelOBB *pco)
{
 	for(uint32 obb_cnt = 0 ; obb_cnt < m_NumOBBs ; obb_cnt ++ )
 	{
		pco[ obb_cnt ] = m_ModelOBBs[ obb_cnt ];	
	}
}
#endif // MODEL_OBB

// ------------------------------------------------------------------------- //
// SpriteInstance.
//------------------------------------------------------------------------- //

SpriteInstance::SpriteInstance()
    : LTObject(OT_SPRITE)
{
    // Sprites default to RGB 255.
    m_ColorR = m_ColorG = m_ColorB = 255;

    memset(&m_SpriteTracker, 0, sizeof(m_SpriteTracker));
    m_ClipperPoly = INVALID_HPOLY;
    m_SCImpl.m_pSprite = this;
	m_nEffectShaderID = -1;
}

SpriteInstance::~SpriteInstance()
{
}


// ------------------------------------------------------------------------- //
// DynamicLight.
// ------------------------------------------------------------------------- //

DynamicLight::DynamicLight()
    : LTObject(OT_LIGHT)
{
    m_LightRadius = 100.0f;
}

DynamicLight::~DynamicLight()
{
}


// ------------------------------------------------------------------------- //
// CameraInstance.
// ------------------------------------------------------------------------- //

CameraInstance::CameraInstance()
    : LTObject(OT_CAMERA)
{
    m_bFullScreen = LTTRUE;
    m_xFov = m_yFov = MATH_HALFPI;
    m_LightAdd.Init();

}

CameraInstance::~CameraInstance()
{
}


// ------------------------------------------------------------------------- //
// LTParticleSystem.
// ------------------------------------------------------------------------- //

LTParticleSystem::LTParticleSystem()
    : LTObject(OT_PARTICLESYSTEM)
{
    m_pCurTexture = LTNULL;

    VEC_INIT(m_MinPos);
    VEC_INIT(m_MaxPos);

    VEC_INIT(m_SystemCenter);
    m_SystemRadius = 1.0f;

    VEC_INIT(m_OldCenter);
    m_OldRadius = 1.0f;

    m_ParticleRadius = 300;
    m_GravityAccel = -500.0f;
    m_psFlags = 0;
    m_pParticleBank = LTNULL;
    m_pSprite = LTNULL;

    // Particles default to RGB 255.
    m_ColorR = m_ColorG = m_ColorB = 255;
    m_SoftwareR = m_SoftwareG = m_SoftwareB = 255;

    m_ParticleHead.m_pNext = m_ParticleHead.m_pPrev = &m_ParticleHead;

    m_nParticles = 0;
    m_nChangedParticles = 0;

    // No blend mode set
    m_nSrcBlend = m_nDestBlend = -1;
	m_nEffectShaderID = -1;
}

LTParticleSystem::~LTParticleSystem()
{
    PSParticle *pCur, *pNext;

    m_MinPos.Init((float)MAX_CREAL, (float)MAX_CREAL, (float)MAX_CREAL);
    m_MaxPos.Init((float)-MAX_CREAL, (float)-MAX_CREAL, (float)-MAX_CREAL);
    m_pCurTexture = LTNULL;

    // Move all the particles to the free list.
    if (m_pObjectMgr)
    {
        pCur = m_ParticleHead.m_pNext;
        while (pCur != &m_ParticleHead)
        {
            pNext = pCur->m_pNext;
            sb_Free(&m_pObjectMgr->m_ParticleBank, pCur);
            pCur = pNext;
        }
    }

    m_ParticleHead.m_pNext = m_ParticleHead.m_pPrev = &m_ParticleHead;
    m_nParticles = 0;
}

void LTParticleSystem::Init(ObjectMgr *pMgr, ObjectCreateStruct *pStruct)
{
    LTObject::Init(pMgr, pStruct);
    m_pParticleBank = &pMgr->m_ParticleBank;
}


// ------------------------------------------------------------------------- //
// LTPolyGrid.
// ------------------------------------------------------------------------- //

LTPolyGrid::LTPolyGrid()
    : LTObject(OT_POLYGRID)
{
    dl_TieOff(&m_LeafLinks);
    m_Data = LTNULL;
    m_Indices = LTNULL;
    m_pSprite = LTNULL;
    m_xPan = m_yPan = 0.0f;
    m_xScale = m_yScale = 1.0f;
	m_fBaseReflection = 0.5f;
	m_fFresnelVolumeIOR = 1.33f;
	m_nEffectShaderID = -1;
    m_pEnvMap = LTNULL;
    m_nTris = 0;
    m_nIndices = 0;
    m_Width = m_Height = 0;
	m_nPGFlags = 0;
	m_pValidMask = NULL;
	m_nValidMaskWidth = 0;
}

LTPolyGrid::~LTPolyGrid()
{
	if(m_Data)
	{
		dfree(m_Data);
		m_Data = NULL;
	}

	if(m_Indices)
	{
		dfree(m_Indices);
		m_Indices = NULL;
	}

	if(m_pValidMask)
	{
		dfree(m_pValidMask);
		m_pValidMask = NULL;
	}
}


// ------------------------------------------------------------------------- //
// LineSystem..
// ------------------------------------------------------------------------- //

LineSystem::LineSystem()
    : LTObject(OT_LINESYSTEM)
{
    m_LineHead.m_pNext = m_LineHead.m_pPrev = &m_LineHead;
    m_pLineBank = LTNULL;

    m_SystemRadius = 0.0f;
    m_bChanged = LTFALSE;


    m_SystemCenter.Init();
    m_MinPos.Init();
    m_MaxPos.Init();
}


LineSystem::~LineSystem()
{
    LSLine *pCur, *pNext;

    pCur = m_LineHead.m_pNext;
    while (pCur != &m_LineHead)
    {
        pNext = pCur->m_pNext;
        sb_Free(m_pLineBank, pCur);
        pCur = pNext;
    }

    m_LineHead.m_pNext = m_LineHead.m_pPrev = &m_LineHead;
    m_pLineBank = LTNULL;
}


void LineSystem::Init(ObjectMgr *pMgr, ObjectCreateStruct *pStruct)
{
    LTObject::Init(pMgr, pStruct);
    m_pLineBank = &pMgr->m_LineBank;
}



// ------------------------------------------------------------------------- //
// ContainerInstance.
// ------------------------------------------------------------------------- //

ContainerInstance::ContainerInstance()
    : WorldModelInstance(OT_CONTAINER)
{
    m_ContainerCode = 0xFFFF;
}

ContainerInstance::~ContainerInstance()
{
}

void ContainerInstance::Init(ObjectMgr *pMgr, ObjectCreateStruct *pStruct)
{
    WorldModelInstance::Init(pMgr, pStruct);
    m_ContainerCode = pStruct->m_ContainerCode;
}



// ------------------------------------------------------------------------- //
// Canvas.
// ------------------------------------------------------------------------- //

Canvas::Canvas()
    : LTObject(OT_CANVAS)
{
    m_CanvasRadius = 1.0f;
    m_Fn = LTNULL;
    m_pFnUserData = LTNULL;
}


// ------------------------------------------------------------------------- //
// LTVolumeEffect.
// ------------------------------------------------------------------------- //

LTVolumeEffect::LTVolumeEffect()
	: LTObject(OT_VOLUMEEFFECT)
{
	m_EffectType = VolumeEffectInfo::kUninitialized;
	m_Dims = LTVector( 16.0f, 16.0f, 16.0f );
	m_nEffectShaderID = -1;
}

LTVolumeEffect::~LTVolumeEffect()
{
}

void LTVolumeEffect::Init( ObjectMgr* pMgr, ObjectCreateStruct* pStruct )
{
	LTObject::Init( pMgr, pStruct );
}





// ------------------------------------------------------------------------- //
// External functions.
// ------------------------------------------------------------------------- //
LTRESULT om_Init(ObjectMgr *pMgr, LTBOOL bClient)
{
    int i;

    memset(pMgr->m_ObjectBankPointers, 0, sizeof(pMgr->m_ObjectBankPointers));
    pMgr->m_ObjectBankPointers[OT_NORMAL] = &pMgr->m_ObjectBankNormal;
    pMgr->m_ObjectBankPointers[OT_MODEL] = &pMgr->m_ObjectBankModel;
    pMgr->m_ObjectBankPointers[OT_WORLDMODEL] = &pMgr->m_ObjectBankWorldModel;
    pMgr->m_ObjectBankPointers[OT_SPRITE] = &pMgr->m_ObjectBankSprite;
    pMgr->m_ObjectBankPointers[OT_LIGHT] = &pMgr->m_ObjectBankLight;
    pMgr->m_ObjectBankPointers[OT_CAMERA] = &pMgr->m_ObjectBankCamera;
    pMgr->m_ObjectBankPointers[OT_PARTICLESYSTEM] = &pMgr->m_ObjectBankParticleSystem;
    pMgr->m_ObjectBankPointers[OT_POLYGRID] = &pMgr->m_ObjectBankPolyGrid;
    pMgr->m_ObjectBankPointers[OT_LINESYSTEM] = &pMgr->m_ObjectBankLineSystem;
    pMgr->m_ObjectBankPointers[OT_CONTAINER] = &pMgr->m_ObjectBankContainer;
    pMgr->m_ObjectBankPointers[OT_CANVAS] = &pMgr->m_ObjectBankCanvas;
	pMgr->m_ObjectBankPointers[OT_VOLUMEEFFECT] = &pMgr->m_ObjectBankVolumeEffect;

    // Just a debug check for when new object types are added.. they MUST have a bank in here.
    for (i=0; i < NUM_OBJECTTYPES; i++)
    {
        ASSERT(pMgr->m_ObjectBankPointers[i] != LTNULL);
    }

    if (bClient)
    {
        LT_MEM_TRACK_ALLOC(sb_Init2(&pMgr->m_ParticleBank, sizeof(PSParticle), 1024, 1024), LT_MEM_TYPE_MISC);
        LT_MEM_TRACK_ALLOC(sb_Init2(&pMgr->m_LineBank, sizeof(LSLine), 64, 64), LT_MEM_TYPE_MISC);
    }
    else
    {
        LT_MEM_TRACK_ALLOC(sb_Init(&pMgr->m_ParticleBank, sizeof(PSParticle), 64), LT_MEM_TYPE_MISC);
        LT_MEM_TRACK_ALLOC(sb_Init(&pMgr->m_LineBank, sizeof(LSLine), 64), LT_MEM_TYPE_MISC);
    }

    LT_MEM_TRACK_ALLOC(sb_Init2(&pMgr->m_TrackerBank, sizeof(LTAnimTracker), TRACKER_CACHE, TRACKER_START_SIZE), LT_MEM_TYPE_MISC);
    LT_MEM_TRACK_ALLOC(sb_Init2(&pMgr->m_AttachmentBank, sizeof(Attachment), 16, 32), LT_MEM_TYPE_MISC);

    for (i=0; i < NUM_OBJECTTYPES; i++)
    {
        dl_InitList(&pMgr->m_ObjectLists[i]);
    }

    pMgr->m_InternalLink.m_pData = pMgr;
    dl_Insert(&g_ObjectMgrs, &pMgr->m_InternalLink);
    return LT_OK;
}


LTRESULT om_Term(ObjectMgr *pMgr)
{
    int i;
    LTLink *pCur;

    for (i=0; i < NUM_OBJECTTYPES; i++)
    {
        // Make sure they've removed all the objects.
        ASSERT(pMgr->m_ObjectLists[i].m_Head.m_pNext == &pMgr->m_ObjectLists[i].m_Head);
        pMgr->m_ObjectBankPointers[i]->Term();
    }

    sb_Term(&pMgr->m_ParticleBank);
    sb_Term(&pMgr->m_LineBank);
    sb_Term(&pMgr->m_AttachmentBank);
    sb_Term(&pMgr->m_TrackerBank);

    // Get it out of the global list if it's in there.
    for (pCur=g_ObjectMgrs.m_pNext; pCur != &g_ObjectMgrs; pCur=pCur->m_pNext)
    {
        if (pCur->m_pData == pMgr)
        {
            dl_Remove(pCur);
            break;
        }
    }

    return LT_OK;
}


LTRESULT om_CreateObject(ObjectMgr *pMgr, ObjectCreateStruct *pStruct, LTObject **ppObject)
{
    LTObject *pObject;
    unsigned short objectType;


    *ppObject = LTNULL;

    objectType = pStruct->m_ObjectType;
    if (objectType >= NUM_OBJECTTYPES)
    {
        RETURN_ERROR_PARAM(1, om_CreateObject, LT_INVALIDPARAMS, pStruct->m_Filename);
    }

    // Do the 'extra' initialization for the object type.
    LT_MEM_TRACK_ALLOC(pObject = (LTObject*)pMgr->m_ObjectBankPointers[objectType]->AllocVoid(), LT_MEM_TYPE_MISC);
    if (!pObject)
    {
        RETURN_ERROR_PARAM(1, om_CreateObject, LT_OUTOFMEMORY, pStruct->m_Filename);
    }

    pObject->Init(pMgr, pStruct);

    // Add to the appropriate list.
    dl_AddHead(&pMgr->m_ObjectLists[objectType], &pObject->m_Link, pObject);

    *ppObject = pObject;
    return LT_OK;
}


LTRESULT om_DestroyObject(ObjectMgr *pMgr, LTObject *pObject)
{
    dl_RemoveAt(&pMgr->m_ObjectLists[pObject->m_ObjectType], &pObject->m_Link);
    if (pObject->m_ObjectType < NUM_OBJECTTYPES)
    {
        pMgr->m_ObjectBankPointers[pObject->m_ObjectType]->FreeVoid(pObject);
    }

    return LT_OK;
}


LTRESULT om_CreateAttachment(ObjectMgr *pMgr, LTObject *pParent, uint16 nChildID, int iSocket,
    LTVector *pOffset, LTRotation *pRotationOffset, Attachment **ppAttachment)
{
	//check the sanity of our parameters
	if(!pParent || (nChildID == pParent->m_ObjectID))
	{
		//one of the parameters is invalid
		assert(!"Invalid parameters for creating an attachment");
		return LT_ERROR;
	}

    // Setup the attachment.
	Attachment *pAttachment;
    LT_MEM_TRACK_ALLOC(pAttachment = (Attachment*)sb_Allocate(&pMgr->m_AttachmentBank), LT_MEM_TYPE_OBJECT);
    if (!pAttachment)
    {
        return LT_ERROR;
    }

	//now actually setup the attachment
    pAttachment->m_pNext		= pParent->m_Attachments;
	pAttachment->m_nChildID		= nChildID;
	pAttachment->m_nParentID	= pParent->m_ObjectID;
	pAttachment->m_iSocket		= iSocket;

	//add it into our attachment list
    pParent->m_Attachments = pAttachment;

    if (pOffset)
    {
        pAttachment->m_Offset.m_Pos = *pOffset;
    }

    if (pRotationOffset)
    {
        pAttachment->m_Offset.m_Rot = *pRotationOffset;
    }
    else
    {
        pAttachment->m_Offset.m_Rot.Init();
    }

    if (ppAttachment)
    {
        *ppAttachment = pAttachment;
    }

    return LT_OK;
}

LTRESULT om_RemoveAttachment(ObjectMgr *pMgr, LTObject *pParent, uint16 nChildID)
{
    // Find it and remove it.
    Attachment **ppPrev = &pParent->m_Attachments;
    Attachment *pCur	= pParent->m_Attachments;

    while (pCur)
    {
        if (pCur->m_nChildID == nChildID)
        {
            *ppPrev = pCur->m_pNext;
            sb_Free(&pMgr->m_AttachmentBank, pCur);
            return LT_OK;
        }

        ppPrev = &pCur->m_pNext;
        pCur = pCur->m_pNext;
    }

    RETURN_ERROR(1, ILTServer::RemoveAttachment, LT_ERROR);

}


void om_ClearSerializeIDs(ObjectMgr *pMgr)
{
    int i;
    LTLink *pListHead, *pCur;

    for (i=0; i < NUM_OBJECTTYPES; i++)
    {
        pListHead = &pMgr->m_ObjectLists[i].m_Head;
        for (pCur=pListHead->m_pNext; pCur != pListHead; pCur=pCur->m_pNext)
        {
            ((LTObject*)pCur->m_pData)->m_SerializeID = INVALID_SERIALIZEID;
        }
    }
}

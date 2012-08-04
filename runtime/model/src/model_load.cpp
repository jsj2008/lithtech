// ----------------------------------------------------------------
//  model_load.cpp
// lithtech (c) 2000
// ----------------------------------------------------------------

#include "bdefs.h"
#include "model.h"
#include "model_ops.h"
#include "iltstream.h"
#include "conparse.h"
#include "ltb.h"

#include "modelallocations.h"
#include "ltt_buffer.h"

#include "syscounter.h"
#include "systimer.h"

#define LTB_D3D_VERSION 2;

extern uint32 g_ModelMemory;

// ------------------------------------------------------------------------
// COMRPESSION 

enum MODEL_ANIM_COMPRESSION_TYPES { 
	ANCMPRS_NONE			= 0, // no compression
	ANCMPRS_REL				= 1, // relevant data only
	ANCMPRS_REL_16			= 2, // rel + 16 bit compress on pos/ 6byte compress on quat.
	ANCMPRS_REL_16_ROT_ONLY = 3  // no compression on position, but 16 bits for rotation
};

// pos values are stored by loping off precision.
const float  NKF_TRANS_SCALE_1_11_4	  = 	(16.0f);		// 2^4
const float  NKF_TRANS_OOSCALE_1_11_4 = 	(1.0f/NKF_TRANS_SCALE_1_11_4);


static inline 
float UnpackFromInt16( int16 intval ) {
	return (float)(intval) * NKF_TRANS_OOSCALE_1_11_4;
}

//utility function to verify that we aren't off in the binary file
void VerifyFileMarker(ILTStream& File)
{
	uint32 nMarker;
	File >> nMarker;

	assert(nMarker == 0xFEEEFEEE);
}

// ------------------------------------------------------------------------
// UnpackLTVectorFromInt16Vec( in_compressed_vec, out_uncompressed_vec )
// 
// Convert a 16bit encoded vector to a float. The loss is float precision.
// ------------------------------------------------------------------------
static inline 
void UnpackLTVectorFromInt16Vec( int16 vec[3], LTVector &out_vec )
{
	out_vec.x = UnpackFromInt16( vec[0]);
	out_vec.y = UnpackFromInt16( vec[1]);
	out_vec.z = UnpackFromInt16( vec[2]);
}

static inline 
void UnpackLTVectorFromInt16Vec( int16 vec[3], float out_vec[3] )
{
	out_vec[0] = UnpackFromInt16( vec[0]);
	out_vec[1] = UnpackFromInt16( vec[1]);
	out_vec[2] = UnpackFromInt16( vec[2]);
}

// ------------------------------------------------------------------------
// CheapUncompressRotation(  in_compressed, out_uncompressed )
//
// converts a quat encoded into int16 in the range -32767 to 32767 in to 
// a quat with float -1.0f to 1.0f.
// ------------------------------------------------------------------------
static inline
void CheapUncompressRotation( int16 compressed_rot[4], LTRotation & out_rot )
{
	out_rot.m_Quat[0] = (float)compressed_rot[0] / 0x7fff ;
	out_rot.m_Quat[1] = (float)compressed_rot[1] / 0x7fff ;
	out_rot.m_Quat[2] = (float)compressed_rot[2] / 0x7fff ;
	out_rot.m_Quat[3] = (float)compressed_rot[3] / 0x7fff ;
}

static inline
void CheapUncompressRotation( int16 compressed_rot[4], float out_rot[4] )
{
	out_rot[0] = (float)compressed_rot[0] / 0x7fff ;
	out_rot[1] = (float)compressed_rot[1] / 0x7fff ;
	out_rot[2] = (float)compressed_rot[2] / 0x7fff ;
	out_rot[3] = (float)compressed_rot[3] / 0x7fff ;
}

// END COMPRESSION 
// ------------------------------------------------------------------------


//------------------------------------------------------------------ //
// feed back on errors
//------------------------------------------------------------------ //
const uint32 kErrMsgSize = 256 ;
char g_szLastLoadError[kErrMsgSize] = "error ";


static void setLoadErrorMsg(const char * msg )
{
	LTStrCpy( g_szLastLoadError, msg, sizeof(g_szLastLoadError) );
}

// load error messages 
static char const * const g_szErrorMsgMap[31] = {
	"00 Invalid Header",
	"01 could not load command string, check format",
	"02 could not find pieces section",
	"03 too many pieces to load",
	"04 failed to allocated memory for pieces",
	"05 ran out of memeory allocating piece.",
	"06 failed to load piece",
	"07 could not find nodes section.",
	"08 could not load nodes",
	"09 error in PrecalcNodeList, invalid nodes.",
	"10 invalid animation weight sets",
	"11 could not find Childmodel file section.",
	"12 no child models, or too many child models",
	"13 could not initialize self as child model.",
	"14 could not read child model name",
	"15 child model has child models, that not good.",
	"16 could not allocate memory for child models...",
	"17 .. ",
	"18 could not initialize (add to self), child model.",
	"19 failed to load child model",
	"20 failed to get child model extra data",
	"21 could not find animation section.",
	"22 could not allocate space for animations.",
	"23 could not allocate memory for new anim.",
	"24 failed to load animation.",
	"25 .. ",
	"26 could not load animation bindings.",
	"27 could not load sockets.",
	"28 no sockets, file corrupted.",
	"29 could not create flat node list.",
	"30 ltb file type is not model type."
};


// ------------------------------------------------------------------------
// CDefVertexLst *NewDefVertexLstFromFile( ILTStream *, IAlloc *)
// returns a new CDefVertexLst from a file. returns null if nothing was read.
// Part of reading NodeKeyFrame data from a file
// ------------------------------------------------------------------------
static void LoadVertexListFromFile( CDefVertexLst& List, ILTStream &file, LAlloc *alloc )
{
	uint32 size ;
	file.Read( &size , sizeof(uint32) ) ;

	if( size )
	{
		List.setSize( size, alloc );

		// slurp up the data ..
		file.Read( List.getArray(), sizeof(float) * 3 * size );
	}
}


//utility function that given a file and a type of channel and the amount of data
//it will setup the appropriate channel data, read it into the buffer, and increment
//the offset
void AnimNode::SetupPosChannel(const IAnimPosChannel* pInterpreter, ILTStream& file, uint32 nElements, uint8*& pData)
{
	assert(pInterpreter);
	m_pPosChannel = pInterpreter;

	uint32 nDataSize = nElements * pInterpreter->GetDataSize();

	if(nDataSize)
	{
		//now read in the data
		file.Read(pData, nDataSize);

		m_pPosData = pData;

		pData += nDataSize;
	}
	else
	{
		//no data
		m_pPosData = NULL;
	}
}

void AnimNode::SetupQuatChannel(const IAnimQuatChannel* pInterpreter, ILTStream& file, uint32 nElements, uint8*& pData)
{
	assert(pInterpreter);
	m_pQuatChannel = pInterpreter;

	uint32 nDataSize = nElements * pInterpreter->GetDataSize();

	if(nDataSize)
	{
		//now read in the data
		file.Read(pData, nDataSize);

		m_pQuatData = pData;

		pData += nDataSize;
	}
	else
	{
		//no data
		m_pQuatData = NULL;
	}
}

//Utility function that given several parameters will pick the appropriate channel
//interpreter
static const IAnimPosChannel* GetPosInterpreter(bool bCompressed, uint32 nNumItems)
{
	//if we have no items, use the null position
	if(nNumItems == 0)
		return NULLPOSChannel::GetSingleton();

	//now pick whether or not we are compressed
	if(bCompressed)
	{
		if(nNumItems == 1)
			return SinglePOS16Channel::GetSingleton();
		else
			return POS16Channel::GetSingleton();
	}
	else
	{
		if(nNumItems == 1)
			return SinglePOSChannel::GetSingleton();
		else
			return POSChannel::GetSingleton();
	}
}

//Utility function that given several parameters will pick the appropriate channel
//interpreter
static const IAnimQuatChannel* GetQuatInterpreter(bool bCompressed, uint32 nNumItems)
{
	//if we have no items, use the null position
	if(nNumItems == 0)
		return NULLQUATChannel::GetSingleton();

	//now pick whether or not we are compressed
	if(bCompressed)
	{
		if(nNumItems == 1)
			return SingleQUAT16Channel::GetSingleton();
		else
			return QUAT16Channel::GetSingleton();
	}
	else
	{
		if(nNumItems == 1)
			return SingleQUATChannel::GetSingleton();
		else
			return QUATChannel::GetSingleton();
	}
}

// ------------------------------------------------------------------------
// Loads the animation data for this node for this animation from disk. As parameters it
//takes the file, the compression method, the node that the animation node correlates to,
//and also the block of memory that has been allocated to store animation data in. It is
//free to store a pointer to the block for reading from, and is in charge of moving the pointer
//past its animation data.
// ------------------------------------------------------------------------
bool AnimNode::Load(ILTStream &file, ModelAnim* pAnim, uint32 compression_type, uint8*& pAnimData)
{
	if(!compression_type) 
	{	
		uint8 is_vertex_anim ;
		file.Read( &is_vertex_anim , sizeof(int8) );

		if( is_vertex_anim ) 
		{
			SetupPosChannel(NULLPOSChannel::GetSingleton(), file, 0, pAnimData);
			SetupQuatChannel(NULLQUATChannel::GetSingleton(), file, 0, pAnimData);
		
			LT_MEM_TRACK_ALLOC(m_pVertexChannel= new CDefVertexLst [ pAnim->NumKeyFrames() ], LT_MEM_TYPE_MODEL);

			if(!m_pVertexChannel)
			{
				Term();
				return false;
			}

			for( uint32 i = 0 ; i < pAnim->NumKeyFrames() ; i++ )
			{
				//load the vertices from the file
				LoadVertexListFromFile(m_pVertexChannel[i], file , pAnim->GetAlloc() );				
			}
		}
		else
		{
			SetupPosChannel(POSChannel::GetSingleton(), file, pAnim->NumKeyFrames(), pAnimData);
			SetupQuatChannel(QUATChannel::GetSingleton(), file, pAnim->NumKeyFrames(), pAnimData);
		}
	}
	else if( compression_type == ANCMPRS_REL )
	{
		// compression is basically run length encoded, the number of repeats
		// is implicit.
		// note that the keyframes array values is set to identity on creation.
		uint32 num_pos, num_quat;
		
		file >> num_pos ;
		SetupPosChannel(GetPosInterpreter(false, num_pos), file, num_pos, pAnimData);
		
		file >> num_quat ; 
		SetupQuatChannel(GetQuatInterpreter(false, num_quat), file, num_quat, pAnimData);
	}
	else if( compression_type == ANCMPRS_REL_16 )
	{
		// note: 
		// pos values are 16 bit encoded, losing some precision.
		// quaternions are naively compressed, per member 16 bit compressed.
		// but instead of precision loss, the int16 range is mapped to -1 to 1
		// float range.
	
		uint32 num_pos, num_quat;
				
		file >> num_pos ;
		SetupPosChannel(GetPosInterpreter(true, num_pos), file, num_pos, pAnimData);
		
		file >> num_quat ; 
		SetupQuatChannel(GetQuatInterpreter(true, num_quat), file, num_quat, pAnimData);

	}
	else if( compression_type == ANCMPRS_REL_16_ROT_ONLY )
	{
		uint32 num_pos, num_quat;
		
		file >> num_pos ;
		SetupPosChannel(GetPosInterpreter(false, num_pos), file, num_pos, pAnimData);
		
		file >> num_quat ; 
		SetupQuatChannel(GetQuatInterpreter(true, num_quat), file, num_quat, pAnimData);
	}

	return true;
}

//this will recursively load animation nodes, traversing the model node heirarchy
bool ModelAnim::LoadAnimNodesRecurse(ILTStream& file, uint32 nCompressionType, const ModelNode* pNode, uint8*& pAnimData)
{
	//figure out which node we are loading
	uint32 nAnimNode = pNode->GetNodeIndex();

	//alright, now tell that node to load
	if(!m_pAnimNodes[nAnimNode].Load(file, this, nCompressionType, pAnimData))
	{
		//failed to load, bail
		Term();
		return false;
	}

	//we now need to recursively load the children
	for(uint32 nCurrChild = 0; nCurrChild < pNode->NumChildren(); nCurrChild++)
	{
		if(!LoadAnimNodesRecurse(file, nCompressionType, pNode->m_Children[nCurrChild], pAnimData))
		{
			return false;
		}
	}

	//success
	return true;
}

bool ModelAnim::Load(ILTStream &file, uint8*& pAnimData)
{
	uint32 i, nKeyFrames;
	uint32 nCompressionType = 0 ;// no compression
	AnimKeyFrame *pKeyFrame;
	
	Term();

	// Save misc info.
	if(!m_pModel->LoadString(file, m_pName))
	{
		return false;
	}

	file >> nCompressionType ;
	file >> m_InterpolationMS;
	file >> nKeyFrames;

	if(!m_KeyFrames.SetSize2(nKeyFrames, GetAlloc()))
	{
		Term();
		return false;
	}

	for(i=0; i < nKeyFrames; i++)
	{
		pKeyFrame = &m_KeyFrames[i];

		file >> pKeyFrame->m_Time;

		if(!m_pModel->LoadString(file, pKeyFrame->m_pString))
		{
			Term();
			return false;
		}
	}


	//allocate our animation node list.

	//we shouldn't have the list already allocated
	assert(!m_pAnimNodes);

	//allocate our new list
	m_pAnimNodes = LNew_Array(GetAlloc(), AnimNode, m_pModel->NumNodes(), false);

	//check the allocation
	if(!m_pAnimNodes)
	{
		Term();
		return false;
	}

	if(!LoadAnimNodesRecurse(file, nCompressionType, m_pModel->GetRootNode(), pAnimData))
	{
		Term();
		return false;
	}

	return true;
}

bool ModelNode::Load(ILTStream &file, const LTMatrix& mInvParentMat)
{
	uint32 i, j, nChildren;
	ModelNode *pChild;
	LTMatrix mGlobalTransform;


	if(!m_pModel->LoadString(file, m_pName))
		return false;

	file >> m_NodeIndex;
	file >> m_Flags;


	// Load the global transform.
	for(i=0; i < 4; i++)
		for(j=0; j < 4; j++)
			file >> mGlobalTransform.m[i][j];

	SetGlobalTransform(mGlobalTransform, mInvParentMat);

	// Save the child nodes.
	file >> nChildren;

	if(!m_Children.SetSizeInit4(nChildren, LTNULL, GetAlloc()))
		return false;

	for(i=0; i < nChildren; i++)
	{
		pChild = LNew_1P(GetAlloc(), ModelNode, m_pModel);
		if(!pChild)
			return false;

		if(!pChild->Load(file, ~mGlobalTransform))
		{
			LDelete(GetAlloc(), pChild);
			DeleteAndClearArray2(m_Children, GetAlloc());
			return false;
		}

		m_Children[i] = pChild;
	}

	return true;
}


bool AnimInfo::Load(ILTStream &file, uint8*& pAnimData)
{
	file >> m_vDims;

	if( m_pAnim )
	{
		m_pAnim->Load(file, pAnimData);
	}
	return true ;
}

// ----------------------------------------------------------------
//  Load up the piece.
// ----------------------------------------------------------------
bool ModelPiece::Load(ILTStream &file, LTB_Header& LTBHeader, uint32 nFileVersion)
{
	uint32 i;
	
	// read in name of piece
	if(!m_pModel->LoadString(file, m_pName))
		return false;

	// read in the amnt of lods in this piece.
	file >> m_nLODs ;

	// load the distance values for every render object 
	m_pLODDists = LNew_Array(m_pModel->GetAlloc(), float, m_nLODs, false);
	
	if(!m_pLODDists)
	{
		Term();
		return false;
	}

	// read the distance values .
	file.Read(m_pLODDists, m_nLODs * sizeof(float));

	// read min/max lod offsets, no longer used and should be removed next file version
	uint32 tmp ;
	file.Read(&tmp,sizeof(uint32));
	file.Read(&tmp,sizeof(uint32));

    // ALLOCATING PIECES's  MESHES.
	m_pRenderObjects = LNew_Array(m_pModel->GetAlloc(), CDIModelDrawable*, m_nLODs, false);
	
	if (!m_pRenderObjects)
	{
		Term();
		return false;
	}

    uint32 render_object_type;

	// load up the lods
    for (i = 0; i < m_nLODs; ++i) 
	{		
		file >> m_nNumTextures;
		for (uint32 j = 0; j < MAX_PIECE_TEXTURES; ++j)
		{
			file >> m_iTextures[j];
		}

		file >> m_iRenderStyle;
		file >> m_nRenderPriority;

        file >> render_object_type ;

        m_pRenderObjects[i] = CreateModelRenderObject( render_object_type );

		if (!m_pRenderObjects[i]) 
		{ 
			setLoadErrorMsg(" Unable to load ltb model file. Possibly out of date format - try recompiling it."); 
			return false; 
		}
        
		if (!m_pRenderObjects[i]->Load(file, LTBHeader))
			return false;

		// load used node list.
		m_pRenderObjects[i]->m_UsedNodeListSize = 0;
		
		file >> m_pRenderObjects[i]->m_UsedNodeListSize;
			
		if( m_pRenderObjects[i]->m_UsedNodeListSize != 0 )
		{
			m_pRenderObjects[i]->CreateUsedNodeList(m_pRenderObjects[i]->m_UsedNodeListSize);
			file.Read( m_pRenderObjects[i]->m_pUsedNodeList, sizeof(uint8) * m_pRenderObjects[i]->m_UsedNodeListSize) ;
		}	
	}

	return true;
}

bool WeightSet::Load(ILTStream &file)
{
	uint32 i, nWeights;

	if(!m_pModel->LoadString(file, m_pName))
		return false;

	file >> nWeights;
	if(!m_Weights.SetSize2(nWeights, GetAlloc()))
		return false;

	for(i=0; i < nWeights; i++)
	{
		file >> m_Weights[i];
	}

	return true;
}

bool Model::LoadString(ILTStream &file, const char* &pStr)
{
	char tempStr[4096];

	if(file.ReadString(tempStr, sizeof(tempStr)) != LT_OK)
		return false;

	pStr = AddString(tempStr);
	return true;
}


bool Model::LoadAnimBindings(ModelLoadRequest *pRequest, ILTStream &file, bool bAllowUpdates)
{
	uint32 i, j, nAnimBindings, testIndex;
	ChildInfo *pInfo;
	AnimInfo *pAnimInfo;
	char animName[256];
	LTVector dims, trans;

	for(i=0; i < NumChildModels(); i++)
	{
		pInfo = GetChildModel(i);

		file >> nAnimBindings;

		// If the save index is wrong or the number of anims is wrong then the
		// bounding radii need to be marked as out of date.
		if(pInfo->m_pModel)
		{
			if( nAnimBindings != pInfo->m_pModel->CalcNumParentAnims() )
				
			{
				ASSERT(pInfo->m_pModel != this);
			}
		}

		for(j=0; j < nAnimBindings; j++)
		{
			if(file.ReadString(animName, sizeof(animName)) != LT_OK)
				return false;

			file >> dims;
			file >> trans;

			// The model didn't exist..
			if(!pInfo->m_pModel)
				continue;

			pAnimInfo = FindAnimInfo(animName, pInfo->m_pModel, &testIndex);

			// If it's missing or out of order from what we expected, radii are invaild.
			if(!pAnimInfo || testIndex != (pInfo->m_AnimOffset + j))
			{
				// check for duped animation names. 
				if( pAnimInfo ) dsi_ConsolePrint("Error: animation name duped in model db %s ", animName);
				ASSERT(pInfo->m_pModel != this);
			}

			if(!pAnimInfo)
			{
				continue;
			}

			pAnimInfo->m_vDims = dims;
			pAnimInfo->m_vTranslation = trans;
		}
	}

	return true;
}


bool Model::LoadSockets(ILTStream &file)
{
	uint32 i, nSockets;
	ModelSocket *pSocket;

	file >> nSockets;
	if(!m_Sockets.SetSizeInit4(nSockets, LTNULL, GetAlloc()))
		return false;

	for(i=0; i < nSockets; i++)
	{
		pSocket = LNew(GetAlloc(), ModelSocket);
		if(!pSocket)
		{
			return false;
		}

		m_Sockets[i] = pSocket;

		file >> pSocket->m_iNode;
		if(pSocket->m_iNode >= NumNodes())
			return false;

		if(!LoadString(file, pSocket->m_pName))
			return false;

		file >> pSocket->m_Rot;
		file >> pSocket->m_Pos;
		file >> pSocket->m_Scale ;
	}

	return true;
}


bool Model::LoadWeightSets(ILTStream &file)
{
	uint32 i, nSets;
	WeightSet *pSet;

	DeleteAndClearArray2(m_WeightSets, GetAlloc());

	file >> nSets;
	if(!m_WeightSets.SetSizeInit4(nSets, LTNULL, GetAlloc()))
		return false;

	for(i=0; i < m_WeightSets; i++)
	{
		pSet = m_WeightSets[i] = LNew_1P(GetAlloc(), WeightSet, this);
		if(!pSet || !pSet->Load(file))
		{
			DeleteAndClearArray2(m_WeightSets, GetAlloc());
			return false;
		}

		// validate weight set. The weight set must have the same number of
		// weights as there are nodes in the model heierarchy.
		if( pSet->m_Weights.GetSize() != NumNodes() )
		{
			return false ;
		}
	}

	return true;
}



// ----------------------------------------------------------------
//  This is really puff right now, b/c it has to support
// both ltb and abc.
// ----------------------------------------------------------------
LTRESULT Model::Load(ModelLoadRequest *pRequest, const char* pFilename)
{
	LTRESULT dRet, dResult;

	uint32 i, j, curOffset, nChildModels;
	uint32 nAnims, totalNumAnims;
	uint32 dwMemAllocSize;
	uint32 error_val = 0;
	uint32 nPieces;

	Model *pModel;
	ModelAnim *pAnim;
	ModelPiece *pPiece;
	ModelLoadRequest myLoadRequest;
	ModelAllocations allocs;

	ChildInfo *pChildModel;

	uint8* pCurrAnimData;

	// Make sure the request is valid.
	if(!pRequest->m_pFile ||
		(pRequest->m_bLoadChildModels && !pRequest->m_LoadChildFn))
	{
		ASSERT(false);
		return LT_INVALIDPARAMS;
	}

	Term();

	ILTStream &file = *pRequest->m_pFile;
	dRet = LT_ERROR;

	// get the ltb header
	LTB_Header LTBHeader;
    if (pFilename)  
	{    
		const char* pExt = strrchr(pFilename,'.');
		if (pExt) {
			// check that file name has "ltb" extention. 
			if (stricmp(pExt,".ltb")==0 ) 
			{
				file.Read(&LTBHeader,sizeof(LTBHeader));
				
				if( LTBHeader.m_iFileType != LTB_D3D_MODEL_FILE )
				{
					error_val = 30;
					buffer256 error_string("Model::Load Error Incorrect LTB file type (%d)!!! \n", LTBHeader.m_iFileType);
					dsi_ConsolePrint(error_string);
					
#if !defined(__LINUX)
				    OutputDebugString(error_string);
#endif
					goto Error ;
				}
			}
            else {
				setLoadErrorMsg(" model file is not of .ltb type.\n");
				goto Error; 
			} 
		}
		else goto Error; }
	else goto Error;

	file >> m_FileVersion;

	// make sure we can support the file format.
	if( m_FileVersion < (MIN_MODEL_FILE_VERSION))
	{
		char msg[ kErrMsgSize ];
		LTSNPrintF(msg, sizeof(msg), "fileversion %d is not supported.", m_FileVersion );
		setLoadErrorMsg(msg);

		dRet = LT_ERROR ;
		goto Error ;

	}

	// Load up data size information.
	if(!allocs.Load(file, m_FileVersion))
	{
		setLoadErrorMsg(" Model alloc failed in Model::ValidateHeader() ");
		dRet = LT_ERROR ;
		goto Error ;
	}

	//Setup Block Allocator
	dwMemAllocSize = allocs.CalcAllocationSize();
	
	LTBOOL bBlockAllocSuccess;
	LT_MEM_TRACK_ALLOC(bBlockAllocSuccess = m_BlockAlloc.Init(GetAlloc(), dwMemAllocSize), LT_MEM_TYPE_MODEL);
	if(!bBlockAllocSuccess)
	{
		// yep out of memory, 
		dRet =  LT_OUTOFMEMORY;
		goto Error ;
	}


	m_pAlloc = &m_BlockAlloc;
	m_StringList.SetAlloc(&m_BlockAlloc);

	// Memory tracking
	g_ModelMemory += dwMemAllocSize;

	SetFilename(pFilename);

	if(!LoadString(file, m_CommandString)){
		 error_val = 1;
		goto Error;
	}

	file >> m_VisRadius;
	
	// load Oriented Bounding Box DATA
	uint32 num_obb ;
	file >> num_obb  ; // s/b zero.

#if(MODEL_OBB)
	m_NumOBBs = num_obb;
	if(m_NumOBBs>0)
 	{
		if(m_FileVersion >= MIN_MODELOBB_FILE_VERSION)
		{
 			LT_MEM_TRACK_ALLOC(m_ModelOBBs = new ModelOBB [m_NumOBBs],LT_MEM_TYPE_MODEL);
			file.Read(m_ModelOBBs,sizeof(ModelOBB)*m_NumOBBs);
		}
		else
		{
			// If our model format doesn't support the new OOB format,
			// read in the data in the old format, then throw it away.
			ModelOBB_Depricated *tmp = new ModelOBB_Depricated [ num_obb ] ;
			file.Read( tmp, sizeof(ModelOBB_Depricated) * num_obb );
			delete [] tmp;
		}
 	}
#else 
	// if there's obb info just slurp it up into nothing.
	if( num_obb != 0)
	{
		DEBUG_MODEL_REZ(("model-rez: warning obbs defined for model, this feature is not supported in this version of exe"));
		ModelOBB *tmp = new ModelOBB [ num_obb ] ;
		file.Read( tmp, sizeof(ModelOBB) * num_obb );
		delete [] tmp;
	}
#endif

	
	// go to pieces section of bin.
	file >> nPieces;

	if(nPieces > MAX_PIECES_PER_MODEL)
	{
		char szBuf[256];
		sprintf(szBuf, "Error loading model %s: model pieces exceeds %d\n", pFilename, MAX_PIECES_PER_MODEL);
		dsi_ConsolePrint(szBuf);

		error_val = 3;
		goto Error;
	}

	if(!m_Pieces.SetSizeInit4(nPieces, LTNULL, GetAlloc())) {
		error_val = 4 ; goto Error; }

	//uint32 iCurVertexWeight = 0;
	for (i=0; i < nPieces; ++i) {
		pPiece = LNew_1P(GetAlloc(), ModelPiece, this);
		if (!pPiece) {
			Term(); error_val = 5; goto Error; }

		m_Pieces[i] = pPiece;
		if (!pPiece->Load(file,  LTBHeader, m_FileVersion)) {
			error_val = 6; goto Error; }
	}

	// Load Nodes

	//setup a dummy parent matrix that is nothing but the identity, this makes it so that the root node's
	//transform will be from the origin of the model object
	LTMatrix mInvParentMat;
	mInvParentMat.Identity();

	if (!GetRootNode()->Load(file, mInvParentMat)) 
	{	
		// Load the node tree.
		error_val = 8 ; 
		goto Error;
	}

	// (need the node lists for the name searches it does in here).
	if(!PrecalcNodeLists())
			{error_val = 9 ; goto Error;}

	if (!LoadWeightSets(file)) {
		error_val = 10 ; goto Error;
	}

	for( i = 0 ; i < nPieces ; i++ )
	{
		for( uint32 iLODCnt = 0 ; iLODCnt < m_Pieces[i]->NumLODs() ; iLODCnt++)
			m_Pieces[i]->GetLOD(iLODCnt)->CalcUsedNodes(this);
	}

	// Load the child models.

	file >> nChildModels;

	// Must have at least one (for the model itself).
	if(nChildModels == 0 || nChildModels > MAX_CHILD_MODELS)
			{error_val = 12 ; goto Error;}

	for(i=0; i < nChildModels; i++)
	{
		if(i == 0)
		{
			// Add ourselves as the first child model.
			pChildModel = &m_SelfChildModel;
			if(!InitChildInfo(0, pChildModel, this, "SELF"))
					{error_val = 13 ; goto Error;}
		}
		else 
		{ 
			// load our child model. 
			if( !LoadString(file, myLoadRequest.m_pFilename ) )
			{
				error_val = 14 ; 
				goto Error;
			}

			// don't load sub models.
			myLoadRequest.m_LoadChildFn = pRequest->m_LoadChildFn;
			myLoadRequest.m_pFile = LTNULL;
			myLoadRequest.m_pLoadFnUserData = pRequest->m_pLoadFnUserData;
			myLoadRequest.m_bLoadChildModels = false;  //  Recursive loading is NOT supported.

			pModel = LTNULL;
	
			// do it.
			if(pRequest->m_bLoadChildModels)
				dResult = pRequest->m_LoadChildFn(&myLoadRequest, &pModel);
			else
				dResult = LT_NOCHANGE; // (don't load it)

			if(dResult == LT_OK || dResult == LT_NOCHANGE)
			{
				if(!pModel)
				{
					pRequest->m_bAllChildrenLoaded = false;
				}

				// Make sure the child model doesn't have any child models.
				if(pModel && pModel->NumChildModels() > 1)
				{
					dRet = LT_INVALIDFILE;
						{error_val = 15 ; goto Error;}
				}

				LT_MEM_TRACK_ALLOC(pChildModel = new ChildInfo, LT_MEM_TYPE_MODEL);
				if(!pChildModel)
						{error_val = 16 ; goto Error;}

				if(!InitChildInfo(i, pChildModel, pModel, myLoadRequest.m_pFilename))
						{error_val = 18; goto Error;}

				ASSERT(i == m_nChildModels);
				m_ChildModels[i] = pChildModel;
				m_nChildModels++;

				//make sure that our visible radius will encompass this child model
				m_VisRadius = LTMAX(m_VisRadius, pChildModel->m_pModel->m_VisRadius);
			}
			else
			{
				dRet = dResult;
					{error_val = 19 ; goto Error;}
			}
		}
	}

	// Load animations.
	file >> nAnims;

	totalNumAnims = nAnims + CalcNumChildModelAnims();
	LTBOOL bExpandResult;
	LT_MEM_TRACK_ALLOC(bExpandResult = m_Anims.SetSize2(totalNumAnims, GetDefAlloc()), LT_MEM_TYPE_MODEL);
	if(!bExpandResult)
			{error_val = 22 ; goto Error;}

	//allocate our block of memory  for our animations
	m_pAnimData = LNew_Array(GetAlloc(), uint8, allocs.m_nAnimData, false);
	
	//a pointer that points to where memory should be taken from for the node animations
	pCurrAnimData = m_pAnimData;

	//now load up our animations
	for(i=0; i < nAnims; i++)
	{
		pAnim = LNew_1P(GetAlloc(), ModelAnim, this);
		if(!pAnim)
				{error_val = 23 ; goto Error;}

		m_Anims[i].m_pAnim = pAnim;

		if(!m_Anims[i].Load(file, pCurrAnimData))
		{
			LDelete(GetAlloc(), pAnim);
				{error_val = 24 ; goto Error;}
		}
	}

	//make sure that our animation data ended at the right spot
	assert(pCurrAnimData == (m_pAnimData + allocs.m_nAnimData));

	// Add the anims from child models to our list.
	curOffset = nAnims;
	for(i=0; i < NumChildModels(); i++)
	{
		pChildModel = m_ChildModels[i];

		if(pChildModel->m_pModel && pChildModel->m_pModel != this)
		{
			pChildModel->m_AnimOffset = curOffset;

			for(j=0; j < pChildModel->m_pModel->m_Anims; j++)
			{
				m_Anims[curOffset] = pChildModel->m_pModel->m_Anims[j];

				curOffset++;
			}
		}
	}
	
	// Load sockets.
	if(!LoadSockets(file))
			{error_val = 27 ; goto Error;}

	// Load the animation bindings that exist and setup new ones for the ones that don't exist.
	if(!LoadAnimBindings(pRequest, file, true))
			{error_val = 26 ; goto Error;}


	if(file.ErrorStatus() != LT_OK)
			{error_val = 28 ; goto Error;}

	ParseCommandString();
	SetNodeParentOffsets();
	
	return LT_OK;

// Error conditions go here.
Error:

    buffer256 error_string("Error loading model: %s %d %s\n", pFilename,error_val, g_szErrorMsgMap[ error_val]);
	dsi_ConsolePrint(error_string);
#if !defined(__LINUX)
    OutputDebugString(error_string);
#endif

    //printLoadErrorMsg();

	Term();
	return dRet;
}



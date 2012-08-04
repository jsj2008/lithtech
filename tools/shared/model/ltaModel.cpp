// --------------------------------------------------------------------
// LTA Packer for ABC style models.
// Convert a read in LTA file to the memory format of lt-models.
// ltaModel.cpp
// lithtech.(c) 2000
//

// get rid of pesky stl warnings
#pragma warning (disable:4786)

#include "precompile.h"

// LT
#include "model.h"

// lta
#include "ltamgr.h"
#include "ltaModel.h"

// packer
#include "newgenlod.h"

// c++
#include <iostream>
#include <fstream>
#include <vector>
#include <map>
#include <list>
#include <string>
#pragma warning (disable:4786)

using namespace std;


// ------------------------------------------------------------------------
// the version of the lt model file for this process.
// ------------------------------------------------------------------------
const uint32 LTA_MODEL_VERSION = 13;
const uint32 LTA_DEFAULT_ANIM_INTERP_TIME = 200 ; // default interp time.

static bool   s_bGenerateNormalsForModelPlease = false ;
static string s_CurBasePath ; // the current process path.




///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
// Meta format to lt-model format
///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////

// ------------------------------------------------------------------------
// convert a shape to a piece...
// this is really basic, it does not take vertex weight into account.
// note : if there are more normals than 
// ------------------------------------------------------------------------
static void 
ShapeToPiece(	MetaModel &metaModel, 
				Shape *shape, PieceLOD *pPiece, 
				CLTATranslStatus & status )
{

	char *tmp = (char*)shape->GetName().data(); // use shape's name as piece name
	pPiece->SetName( tmp );
	
	// get tris / get the face set info
	int i ;
	int nTris = shape->GetMesh().tri_FaceSet.size()/3;
	int bHasTCoords = shape->GetMesh().tcoords.size() > 0  ;

	if(!pPiece->m_Tris.SetSize2(nTris, pPiece->GetAlloc()))
	{
		// error outta room
		status.m_status = status.OUT_OF_MEM ;
		return ;

	}

	// collect the u-v texture coords.

	int tcnt = 0;
	std::vector< int > & uv_fs = shape->GetMesh().tri_UVSet ;
	std::vector< LTVector > & tcoords = shape->GetMesh().tcoords ;
	
	for(  i = 0 ; i < nTris ; i++ )
	{
		for( int j  = 0 ; j < 3 ; j++ )
		{
			pPiece->m_Tris[i].m_Indices[j] = shape->GetMesh().tri_FaceSet[tcnt];

			if( bHasTCoords )
			{
				pPiece->m_Tris[i].m_UVs[j].tu = tcoords[ uv_fs[tcnt] ].x ;
				pPiece->m_Tris[i].m_UVs[j].tv = tcoords[ uv_fs[tcnt] ].y ;
			}
			tcnt++ ;
		}
	}

	// get the vecs
	int nVerts = shape->GetMesh().vertex.size() ;
	int nNorms = shape->GetMesh().normals.size(); ;
	bool DoNormsPerVerts = false ;

	if ( nNorms == nVerts )
		DoNormsPerVerts = true ;
	

	if(!pPiece->m_Verts.SetSize2(nVerts, pPiece->GetAlloc()))
	{
		status.m_status = status.OUT_OF_MEM ;
		return ;
	}
	
	// Get VERTEX 

	ModelVert *pVert ;
	for( i = 0 ; i < nVerts ; i++ )
	{
		pVert = &pPiece->m_Verts[i] ;
		
		pVert->m_nWeights = 0;      // mesh deform
		pVert->m_iReplacement = 0 ; // lod

		pVert->m_Vec = shape->GetMesh().vertex[i] ;
	
		
		if( DoNormsPerVerts )
			pVert->m_Normal = shape->GetMesh().normals[i] ;

		/*
		reading weighted deform data is done later.
		*/
	}

	// if we don't have one normal per vertex fudge it here.
	if( !DoNormsPerVerts && nNorms )
	{
		s_bGenerateNormalsForModelPlease = true ;
			
			// A ET! there are more norms than verts, we need to average them
			// this algoritm ain't cutting it, don't have time. let importer recalc norms.
			// which in the abc case if fine.
		vector< int >  &nrm_fs = shape->GetMesh().tri_NormSet ;

		if( nrm_fs.size() > 0)
		{
			ModelVert *pVert ;
			for( i = 0 ; i < nVerts ; i++ )
			{			
				pVert = &pPiece->m_Verts[i] ;
				pVert->m_Normal += shape->GetMesh().normals[ nrm_fs[i ] ] ;

			}
		}
	}

	if( nNorms == 0 )
		s_bGenerateNormalsForModelPlease = true ;

	// Add code here for normals and color verts
	CLTARenderStyle *pRS = (CLTARenderStyle*)shape->GetAppearance();
	
	pPiece->m_nNumTextures = pRS->m_nNumTextures;
	for (i = 0; i < MAX_PIECE_TEXTURES; i++) {
		pPiece->m_iTextures[i] = pRS->m_iTextures[i]; }

	pPiece->m_iRenderStyle		= pRS->m_iRenderStyle;
	pPiece->m_nRenderPriority	= pRS->m_nRenderPriority;

	status.m_status = status.OK ;
}

// ------------------------------------------------------------------------
// NodeToModelNode( Node* , ModelNode , status )
// recure down Node hier filling up ModelNode hier as we go.
// ------------------------------------------------------------------------
static void
NodeToModelNode( Node* root , ModelNode *mroot , CLTATranslStatus & stat )
{
	ASSERT(root);
	ASSERT(mroot);

	mroot->m_NodeIndex = root->index ;
	// if the node is named "null" then mark it as removable
	mroot->m_Flags = root->GetName() == "null" ? MNODE_REMOVABLE : 0 ;
	mroot->SetGlobalTransform ( root->mat );
	mroot->SetName( (char*) root->GetName().c_str() );
	
	// allocate space for children of this node
	if( ! (mroot->m_Children.SetSizeInit4(root->children.size(), 
										  LTNULL, 
										   mroot->GetAlloc()) ))
	{
		stat.m_status = stat.OUT_OF_MEM ;
		return ;
	}

	for( int i = 0 ;i < (int)root->children.size() ; i++ )
	{
		ModelNode *pChild ;
		pChild = LNew_1P( mroot->GetAlloc(), ModelNode, mroot->GetModel()) ;
		if( !pChild )
		{
			stat.m_status = stat.OUT_OF_MEM ;
			return ;
		}
		pChild->m_iParentNode  = mroot->m_NodeIndex ;
		mroot->m_Children[i] = pChild ;
		NodeToModelNode( root->children[i], mroot->m_Children[i], stat );

		if( stat.m_status != stat.OK )
		{
			return ;
		}
	}

}

// ------------------------------------------------------------------------
// VtxAnim2DefVertexLst( CVtxAnim *, frame-number, CDefVertexLst )
//
// go through all the values for frame x in CVtxAnim and stuff it into
// pDefVert list 
// ------------------------------------------------------------------------
static void 
VtxAnim2DefVertexLst( CVtxAnim *pVAnim, int frameNum,  CDefVertexLst *pDefVert )
{
	int vSize = pVAnim->NumVertexPerFrame () ;
	LTVector vec ;
	for( int i = 0 ;i < vSize ; i++ )
	{
		vec = pVAnim->Get(frameNum, i );
		pDefVert->append(vec.x,vec.y,vec.z);
	}
	// pDefVert->Compress()

}

// ------------------------------------------------------------------------
// VtxAnim2DefVertexLst( CVtxAnim *, frame-number, CDefVertexLst, mat )
//
// go through all the values for frame x in CVtxAnim and stuff it into
// pDefVert list after transforming it by mat
// ------------------------------------------------------------------------
static void 
VtxAnim2DefVertexLstXForm( CVtxAnim *pVAnim , int frameNum, CDefVertexLst *pDefVert, LTMatrix &mat )
{
	int vSize = pVAnim->NumVertexPerFrame () ;
	LTVector vec, srcVec ;
	for( int i = 0 ;i < vSize ; i++ )
	{
		srcVec = pVAnim->Get(frameNum, i );
		mat.Apply( srcVec, vec );
		pDefVert->append(vec.x,vec.y,vec.z);
	}
}



// ------------------------------------------------------------------------
// PosQuatInterp( result-pos, result-quat, pos-quat1, pos-quat2, percent )
// interpolate frames
// ------------------------------------------------------------------------
static inline void
PosQuatInterp(	LTVector & resPos, LTRotation & resQuat, 
				LTVector & pos1, LTRotation & quat1, 
				LTVector & pos2, LTRotation & quat2, 
				float percent )
{
	resQuat.Slerp(quat1, quat2, percent);
	resPos = pos1 + (pos2 - pos1) * percent;
}

// ------------------------------------------------------------------------
// GetAnimValueAtTime( pos-quat-anim, time, ret-pos, ret-quat )
// get interpolated value from PosQuat type Animation.
// ------------------------------------------------------------------------
static inline int
GetAnimValueAtTime( CPosQuatAnim *pPQAnim, int time, LTVector &pos, LTRotation & quat )
{
	int start, end ;
	float percent ;

	int res =  pPQAnim->GetKeyFrameIndexFromTime( time, start, end, percent );

	if( res )
	{
		LTVector pos1,pos2;
		LTRotation quat1,quat2;

		pPQAnim->Get( start, pos1, quat1.m_Quat );
		pPQAnim->Get( end  , pos2, quat2.m_Quat );

		PosQuatInterp(pos, quat, pos1, quat1, pos2, quat2, percent);
	}


	return res;

}

// ------------------------------------------------------------------------
// ConvertToMatrix( result-matrix, pos, quat )
// create matrix from position quat pair.
// ------------------------------------------------------------------------
static 
void ConvertToMatrix(LTMatrix &mat, LTVector & Translation, LTRotation & Quaternion)
{
	Quaternion.ConvertToMatrix(mat);
	mat.m[0][3] = Translation.x;
	mat.m[1][3] = Translation.y;
	mat.m[2][3] = Translation.z;
}

// ------------------------------------------------------------------------
// ModelNodeToAnimNodeSkelMerge( parent-anim-node, cur-model-node, cur-anim-node, v-a-anim, p-q-anim )
//
// Create AnimNode data set from two lta anim sets, one a V-A anim, the other Pos Quat.
//  Create AnimNodes from ModelNodes. 
//  Convert animation data from the AnimSet to animnode's keyframe data.
//  Merge v-a anim and pos-quat anim into same data set. Since there's more
//  v-a data, interp new data of pos-quat anim.
// 
// ------------------------------------------------------------------------
static void
ModelNodeToAnimNodeSkelMerge(	AnimNode  * pParentAnimNode,
								ModelNode * pModelNode, 
								AnimNode  * pAnimNode, 
								CAnimSet  * pAnimSet,
								CAnimSet  * pSkelAnimSet,
								CLTATranslStatus &status )
{
	int i ;
	int numKF = pAnimNode->GetAnim()->NumKeyFrames();
	CMoArray<AnimKeyFrame, NoCache>	&vAnimKeyFrames = pAnimNode->GetAnim()->m_KeyFrames;
	LAlloc* alloc  = pAnimNode->GetAlloc() ;

	ASSERT(!pAnimNode->m_KeyFrames);
	if(!pAnimNode->m_KeyFrames.SetSize2( numKF, alloc) )
	{
		status = status.OUT_OF_MEM ;
		status.m_msg = "m_KeyFrames.SetSize2 failed ...  ";

		return ;
	}

	// anim hierarchy linkage
	pAnimNode->m_pNode = pModelNode;
	pAnimNode->m_pParentNode = pParentAnimNode;


	if( pAnimSet != NULL )
	{
		CAnim *pltaAnim  = pAnimSet->FindAnimByTargetName( string(pModelNode->GetName()) );
		CPosQuatAnim *psklAnim  = (CPosQuatAnim*)pSkelAnimSet->FindAnimByTargetName( string(pModelNode->GetName()));

		// if we have a vertex animation set stuff the keyframe data
		if( pltaAnim != NULL && pltaAnim->GetType() == CAnim::VERTEX )
		{
			CVtxAnim *pVAnim = (CVtxAnim*)pltaAnim ; // upgrade the pointers
			Shape    *pShape = pVAnim->GetTarget() ;
			uint32    iPiece ;

			// find the associated peice in the model and assign it to the cur anim node
			if(pAnimNode->GetModel()->FindPiece( pShape->GetName().c_str(), &iPiece ) == NULL )
			{
				status = status.FAIL ;
				string msg = " could not find piece for anim node. ";
				msg += pShape->GetMeshName().c_str() ;
				
				status.OnError("ModelNodeToAnimNode");
				return ;
			}

			pAnimNode->SetVertexAnimTarget(iPiece) ;
		
			// get a handle on the kf 
			CMoArray<NodeKeyFrame,NoCache> &animKF = pAnimNode->m_KeyFrames ;     

			LAlloc *alloc = pAnimNode->GetAlloc();
			
			int iTime = 0 ;
			LTVector pos,pos1,pos2;
			LTRotation quat,quat1,quat2 ;
				
			pos1.Init(0,0,0);
			quat1.Init(0,0,0,1);
			// for every frame in pVAnim 			
			for( int iframe = 0 ;iframe < numKF ; iframe++ )
			{	
				animKF[iframe].m_pDefVertexLst = new CDefVertexLst(alloc) ;
				// bake the nodes transform 
				VtxAnim2DefVertexLst( pVAnim, iframe, animKF[iframe].m_pDefVertexLst);

			//	iTime = vAnimKeyFrames[iframe].m_Time ;

			//	GetAnimValueAtTime( psklAnim, iTime, pos, quat );
				// put deformed nodes at id.
				pAnimNode->m_KeyFrames[iframe].m_Translation = pos1 ;
				pAnimNode->m_KeyFrames[iframe].m_Quaternion  = quat1 ;
			}
		}
		// if there is no vertex animation at this node use only the skeletal anim vals.
		else if(psklAnim != NULL )
		{
			int iTime = 0 ;
			LTVector pos ;
			LTRotation quat ;
			
			// for every frame in pVAnim 			
			for( int iframe = 0 ;iframe < numKF ; iframe++ )
			{	
				iTime = vAnimKeyFrames[iframe].m_Time ;

				if(!GetAnimValueAtTime( psklAnim, iTime, pos, quat ))
				{
					char buf[256];
					CLTATranslStatus status ;
			
					status.m_msg = "bad key frame ";
					status.m_msg += string(itoa(iTime,buf,256)) ;
					status.m_msg += string(pModelNode->GetName()) ;
					status.m_msg += " in AnimSet : " ;
					status.m_msg += string(pAnimSet->GetName() );
					status.OnError("in ModelNodeToAnimNode", LT_FALSE);
				}

				pAnimNode->m_KeyFrames[iframe].m_Translation = pos ;
				pAnimNode->m_KeyFrames[iframe].m_Quaternion  = quat ;
			}
		}
		else // we have an 'empty animation' set the values same as in hierarchy.
		{
			ModelNode *pNode = pAnimNode->m_pNode ;
			LTMatrix mat = pNode->GetLocalTransform();
			LTVector pos;		
			mat.GetTranslation(pos);
			LTRotation quat;
			quat_ConvertFromMatrix(quat.m_Quat , mat.m );
			pos.Init(0,-10,0);
			for( int iframe =0; iframe < numKF ; iframe++ )
			{
				pAnimNode->m_KeyFrames[iframe].m_Translation = pos;
				pAnimNode->m_KeyFrames[iframe].m_Quaternion  = quat;
			}
		}
	}
		
	// allocate children
	if(!pAnimNode->m_Children.SetSizeInit4(pModelNode->NumChildren(), LTNULL, alloc))
	{
		status = status.OUT_OF_MEM ;
		status.m_msg = "m_Children.SetSizeInit4 failed ...  ";
		status.OnError("ModelNodeToAnimNode");
		return ;
	}

	// Load child nodes.
	for( i=0; i < (int)pModelNode->NumChildren(); i++)
	{
		AnimNode *pChild = LNew_2P(alloc, AnimNode, pAnimNode->GetAnim(), pAnimNode);
		if(!pChild)
		{
			status = status.OUT_OF_MEM ;
			status.m_msg = " LNew_2P failed Out of memory" ;
			status.OnError("ModelNodeToAnimNode");
			return ;
		}
			
		pChild->m_pNode = pModelNode->m_Children[i];
		ModelNodeToAnimNodeSkelMerge(	pAnimNode, 
										pModelNode->m_Children[i], 
										pChild, 
										pAnimSet,
										pSkelAnimSet,
										status );
		
		if( status != status.OK )
		{
			// should free pChild 
			return ;
		}

		pAnimNode->m_Children[i] = pChild;
	}
}

// ------------------------------------------------------------------------
// ModelNodeToAnimNode( AnimNode *Parent, ModelNode *Cur, AnimNode*Cur )
// 1. Create AnimNodes from ModelNodes. 
// 2. Convert animation data from the AnimSet to animnode's keyframe data.
// 
// ------------------------------------------------------------------------
static void
ModelNodeToAnimNode(  AnimNode  * pParentAnimNode,
					  ModelNode * pModelNode, 
					  AnimNode  * pAnimNode, 
					  CAnimSet  * pAnimSet,
					  CLTATranslStatus &status )
{
	int i ;
	int numKF = pAnimNode->GetAnim()->NumKeyFrames();	
	LAlloc* alloc  = pAnimNode->GetAlloc() ;

	ASSERT(!pAnimNode->m_KeyFrames);
	if(!pAnimNode->m_KeyFrames.SetSize2( numKF, alloc) )
	{
		status = status.OUT_OF_MEM ;
		status.m_msg = "m_KeyFrames.SetSize2 failed ...  ";

		return ;
	}

	// setup hierarchical linkage
	pAnimNode->m_pNode = pModelNode;
	pAnimNode->m_pParentNode = pParentAnimNode;
	
	// set the kf
	LTMatrix id_mat ;
	id_mat.Identity();

	// note: you should really fill this if  apropriate
	if( pAnimSet != NULL )
	{
		CAnim *pltaAnim  = pAnimSet->FindAnimByTargetName( string(pModelNode->GetName()) );
		
		if( pltaAnim == NULL )
		{
			
		}
		// if we have a vertex animation set stuff the keyframe data
		if( pltaAnim != NULL && pltaAnim->GetType() == CAnim::VERTEX )
		{
			CVtxAnim *pVAnim = (CVtxAnim*)pltaAnim ; // upgrade the pointers
			Shape    *pShape = pVAnim->GetTarget() ;
			uint32    iPiece ;

			// find the associated peice in the model and assign it to the cur anim node
			if(pAnimNode->GetModel()->FindPiece( pShape->GetName().c_str(), &iPiece ) == NULL )
			{
				status = status.FAIL ;
				string msg = " could not find piece for anim node. ";
				msg += pShape->GetMeshName().c_str() ;
				
				status.OnError("ModelNodeToAnimNode");
				return ;
			}

			pAnimNode->SetVertexAnimTarget(iPiece) ;
		
			// get a handle on the kf 
			CMoArray<NodeKeyFrame,NoCache> &animKF = pAnimNode->m_KeyFrames ;     

			LAlloc *alloc = pAnimNode->GetAlloc();
			
			// for every frame in pVAnim 			
			for( int iframe = 0 ;iframe < numKF ; iframe++ )
			{
				animKF[iframe].m_pDefVertexLst = new CDefVertexLst(alloc) ;
				
				VtxAnim2DefVertexLst( pVAnim, iframe, animKF[iframe].m_pDefVertexLst); 
				// set keyframe to id-mat
				pAnimNode->m_KeyFrames[iframe].ConvertFromMatrix( id_mat );				
			}
		}
		else  if ( pltaAnim != NULL && pltaAnim->GetType() == CAnim::POSQUAT )
		{
			LTVector pos(0,10,0);
			float    quat[4];

			CPosQuatAnim *pPQAnim = (CPosQuatAnim*)pltaAnim ;
			// for now fill all the kfs with first frame data.	
			for( int iframe = 0 ; iframe< numKF ; iframe++ ){

				pPQAnim->Get( iframe, pos, quat );
				//pAnimNode->m_KeyFrames[iframe].m_Translation = pos ;
				//pAnimNode->m_KeyFrames[iframe].m_Quaternion.Init(quat[0],quat[1],quat[2],quat[3]);
				
				pPQAnim->Get( iframe, pAnimNode->m_KeyFrames[iframe].m_Translation ,
									  pAnimNode->m_KeyFrames[iframe].m_Quaternion.m_Quat);					
			}
		} 
		else 
		{
			// either there is no animation for this node or one that's not
			// recognized by this lta reader.
			// we'll just generate pos-quat values for this animation from
			// the original node value.

			CLTATranslStatus status ;
			if( pltaAnim == NULL ){
				status.m_msg = "could not find anim for ";
				status.m_msg += string(pModelNode->GetName()) ;
				status.m_msg += " in AnimSet : " ;
				status.m_msg += string(pAnimSet->GetName() );
			}else { 
				status.m_msg = "invalid animation type serious error";
			}
			status.OnError("in ModelNodeToAnimNode", LT_FALSE);

			ModelNode *pNode = pAnimNode->m_pNode ;
			LTMatrix mat = pNode->GetLocalTransform();
			LTVector pos;		
			mat.GetTranslation(pos);
			LTRotation quat;
			quat_ConvertFromMatrix(quat.m_Quat , mat.m );
			
			for( int iframe =0; iframe < numKF ; iframe++ )
			{
				pAnimNode->m_KeyFrames[iframe].m_Translation = pos;
				pAnimNode->m_KeyFrames[iframe].m_Quaternion  = quat;
			}
		}
	}
		

	// Load child nodes.
	if(!pAnimNode->m_Children.SetSizeInit4(pModelNode->NumChildren(), LTNULL, alloc))
	{
		status = status.OUT_OF_MEM ;
		status.m_msg = "m_Children.SetSizeInit4 failed ...  ";
		status.OnError("ModelNodeToAnimNode");
		return ;
	}

	for( i=0; i < (int)pModelNode->NumChildren(); i++)
	{
		AnimNode *pChild = LNew_2P(alloc, AnimNode, pAnimNode->GetAnim(), pAnimNode);
		if(!pChild)
		{
			status = status.OUT_OF_MEM ;
			status.m_msg = " LNew_2P failed Out of memory" ;
			status.OnError("ModelNodeToAnimNode");
			return ;
		}
			
		pChild->m_pNode = pModelNode->m_Children[i];
		ModelNodeToAnimNode(  pAnimNode, pModelNode->m_Children[i], pChild, pAnimSet, status );
		
		if( status != status.OK )
		{
			// should free pChild 
			return ;
		}

		pAnimNode->m_Children[i] = pChild;
	}
}


// ------------------------------------------------------------------------
// AnimSetToModelAnim( AnimSet , ModelAnim )
// convert from CAnimSet to ModelAnim
// ------------------------------------------------------------------------
static 
void AnimSetToModelAnim( MetaModel & metaModel, int cur_set,  ModelAnim *pModelAnim )
{
	CAnimSet *pAnimSet = metaModel.GetAnimSet(cur_set);

	const CKeyFrame &KeyFrame = *pAnimSet->GetKeyFrame() ;

	 //set manim->m_pName ;
	pModelAnim->SetName( pAnimSet->GetName().c_str() );
	// model weight sets ?
	// m_InterpolationMS  // attack time
	// int nKeyFrames = aset->
	int nKeyFrames =KeyFrame.Size();
	int res = pModelAnim->m_KeyFrames.SetSize2(nKeyFrames, pModelAnim->GetAlloc() ) ;
	if( !res )
	{
		CLTATranslStatus( "could not alloc keys ", "AnimSetToModelAnim" );
		return ;
	}
	
	for( int i = 0 ;i < nKeyFrames ; i++ )
	{
		pModelAnim->m_KeyFrames[i].m_Time = KeyFrame.GetTime( i ) ;

		// if there's a string dup it, other wise ... 
		string str = KeyFrame.GetVal( i );
		if( str.size() > 0 )		
		//	pModelAnim->m_KeyFrames[i].m_pString = strdup(str.c_str());
			pModelAnim->m_KeyFrames[i].m_pString = 
				pModelAnim->GetModel()->AddString(str.c_str());
		else
			pModelAnim->m_KeyFrames[i].m_pString = pModelAnim->GetModel()->AddString("");

	}

	AnimNode *pAnimNodeRoot = pModelAnim->GetRootNode();
	
	CLTATranslStatus ModelNodeToAnimNodeConv_status ;
	// create anin data from model data.
	ModelNodeToAnimNode( pAnimNodeRoot, 
						 pModelAnim->GetRootNode()->m_pNode, 
						 pAnimNodeRoot , 
						 pAnimSet,
						 ModelNodeToAnimNodeConv_status );

	if(ModelNodeToAnimNodeConv_status != ModelNodeToAnimNodeConv_status.OK )
	{
		ModelNodeToAnimNodeConv_status.OnError("AnimSetToModelAnim");
		return ; 
	}

	//// now that we've set up the animnode hierarchy....

}

// ------------------------------------------------------------------------
// Converts from an lta-anim set to an LT ModelAnim.
// If there are two animations with the same name except for a trailing "VA"
// on one of them, then merge them together, taking the vertex anim from the one
// with the VA postfix and merging it with the skeletal anim.
// ------------------------------------------------------------------------
static void
AnimSetToModelAnimSkelMerge( MetaModel & metaModel, int va_set, int sk_set, ModelAnim *pModelAnim )
{
	CAnimSet *pAnimSet		= metaModel.GetAnimSet(va_set);
	CAnimSet *pSkelAnimSet	= metaModel.GetAnimSet(sk_set);

	const CKeyFrame &KeyFrame = *pAnimSet->GetKeyFrame() ;

	// even though the v-a anim is the host for the p-q anim data, 
	// we lose the "VA" postfix on the name.
	pModelAnim->SetName( pSkelAnimSet->GetName().c_str() );

	// model weight sets ?
	// m_InterpolationMS  // attack time
	// int nKeyFrames = aset->
	int nKeyFrames =KeyFrame.Size();
	int res = pModelAnim->m_KeyFrames.SetSize2(nKeyFrames, pModelAnim->GetAlloc() ) ;
	if( !res )
	{
		CLTATranslStatus( "could not alloc keys ", "AnimSetToModelAnim" );
		return ;
	}
	
	// copy over the key times from the anim-set
	for( int i = 0 ;i < nKeyFrames ; i++ )
	{
		pModelAnim->m_KeyFrames[i].m_Time = KeyFrame.GetTime( i ) ;
		//pModelAnim->m_KeyFrames[i].m_pString = "";

			// if there's a string dup it, other wise ... 
		string str = KeyFrame.GetVal( i );
		if( str.size() > 0 )		
		{
			//pModelAnim->m_KeyFrames[i].m_pString = strdup(str.c_str());
			pModelAnim->m_KeyFrames[i].m_pString = 
				pModelAnim->GetModel()->AddString(str.c_str());
		}
		else {
			pModelAnim->m_KeyFrames[i].m_pString = pModelAnim->GetModel()->AddString("");
		}
	}

	AnimNode *root = pModelAnim->GetRootNode();
	
	CLTATranslStatus mntan_status ;

	// create anim data from model data.
	ModelNodeToAnimNodeSkelMerge( root, 
						 pModelAnim->GetRootNode()->m_pNode, 
						 root , 
						 pAnimSet,
						 pSkelAnimSet,
						 mntan_status );

	if(mntan_status != mntan_status.OK )
	{
		mntan_status.OnError("AnimSetToModelAnim");
		return ; 
	}

}

///////////////////////////////////////////////////////////////////////////


// ------------------------------------------------------------------------
// container for keeping track of skeletal deformation info comming in from lta.
// ------------------------------------------------------------------------
struct Deformer {
	
	// weight item = index into influence and weight of vertex 
	typedef std::pair< int, float > weight_item_t ;

	typedef std::vector< weight_item_t > weightset_t ;
	
	std::vector< std::string > influences ;
	// a list of lists of weight pairs
	std::vector< weightset_t > weightsets ;

	std::string target_name  ;

};




// ------------------------------------------------------------------------
// Parse The data out 
/*
	(add-deformer (skel-deformer "name" (target "name" ) (infuences ( "" ))
								 (weightsets ( ( i w i w ) ( i w i w ) ) )
								 */
// ------------------------------------------------------------------------
static 
void CreateDeformersFromPN( std::vector< Deformer* > &deformers, 
						    std::vector< CLTANode *> &add_deformers_cmds,							
							int & numWeights )
{

	numWeights = 0;
	CLTANode *cur_pn ;
	int WtCnt   =0;
	CLTATranslStatus errors ;

	for( int i = 0 ;i < (int)add_deformers_cmds.size() ; i++ )
	{
		cur_pn = add_deformers_cmds[i] ; // get the lsit (add-deformer ... 

		
		if( cur_pn->GetNumElements() > 1 )  // we have something in there ... 
		{
			// get the skel-deformer node 
			cur_pn = cur_pn->GetElement( 1 ) ;
			CLTANode *target, *influences, *weightsets ;


			target =     CLTAUtil::ShallowFindList( cur_pn, "target");
			influences = CLTAUtil::ShallowFindList( cur_pn, "influences");
			weightsets = CLTAUtil::ShallowFindList( cur_pn, "weightsets" ); // (ws ( ( i w i w  ) ... )

			if( target == NULL )
			{
				errors << " found deformer a null target! syntax error in file ??" ;
				errors.OnError(" CrateDeformersFromPN " , 0 );
				continue ;
			}

			// get the current ;
			deformers.push_back( new Deformer() );
			Deformer *curDeformer = deformers.back();

			// set target name
			curDeformer->target_name = target->GetElement(1)->GetValue();
			
	
			influences = influences->GetElement(1) ;// pop

			curDeformer->influences.reserve( influences->GetNumElements() ) ;
			for( int iInfls = 0 ; iInfls < (int)influences->GetNumElements() ; iInfls ++ )
			{
				curDeformer->influences.push_back( influences->GetElement(iInfls)->GetValue());		
			}
		
			// pop (weightsets 
			weightsets = weightsets->GetElement(1) ;
			// alloc weight sets array
			curDeformer->weightsets.reserve( weightsets->GetNumElements() );
			CLTANode *weights ;
			int nVerts = weightsets->GetNumElements() ;

			// for every 
			for( int iWS = 0 ; iWS < nVerts ; iWS ++ )
			{
				// weights = ( i w i w i w )
				weights = weightsets->GetElement( iWS );
				int ws_size = weights->GetNumElements() /2 ; // remember list of  i w sets
				
				WtCnt += ws_size ;
	
				// add a new vector of weight pairs
			
				curDeformer->weightsets.push_back( Deformer::weightset_t(ws_size) );
				
				float wt_sum = 0 ;
				int   weight_list_size = weights->GetNumElements() ;

				// iNumweights index into the weights index for the vertex 
				for( int iWtElem = 0, cnt=0 ; iWtElem < weight_list_size ; iWtElem+=2 , cnt++)
				{
					numWeights++ ;
					int   NodeIndex = atoi( weights->GetElement( iWtElem ) ->GetValue() ) ;
					float NodeWeight= (float)atof( weights->GetElement( iWtElem+1) ->GetValue() ) ;
					wt_sum += NodeWeight ;
	
					Deformer::weight_item_t WeightItem( NodeIndex, NodeWeight );

					curDeformer->weightsets[iWS][cnt] =  WeightItem ;
				}
			} 
		}
	}
}

// ------------------------------------------------------------------------
// create localized vertex from global.
// ------------------------------------------------------------------------
static
void LocalizeVertex ( Model *pModel, PieceLOD *pModelPiece, int node_index, 
					 int Vertex,  float Weight, float *result_vec )
{
	//ModelPiece *pModelPiece = pModel->GetPiece(cur_piece_index);

	LTMatrix mat = pModel->m_FlatNodeList[ node_index ]->GetInvGlobalTransform();

	LTVector vec =  pModelPiece->m_Verts[ Vertex ].m_Vec;
	LTVector new_vec ;

	mat.Apply( vec, new_vec ); 

	result_vec[0] = new_vec.x  * Weight ;
	result_vec[1] = new_vec.y  * Weight ;
	result_vec[2] = new_vec.z  * Weight ;
	result_vec[3] = Weight ;

}



// ------------------------------------------------------------------------
// cmd_ApplyDeformers( parse-node-root, target-model )
// apply the lta deformers onto the current lt-model
// ------------------------------------------------------------------------
static 
void cmd_ApplyDeformers( CLTANode *pnroot, 
						 Model *pModel, 
						 CMoArray<PieceLOD*,NoCache> &PieceList,
						 CLTATranslStatus &status  )
{	
	std::vector< CLTANode * > add_deformer_cmds ;
	std::map< string, uint32, less<string> > NodeNameToIndexMap ;

	CLTAUtil::FindAll( add_deformer_cmds , pnroot, "add-deformer");

	if( add_deformer_cmds.size() )
	{
		int numWeights=0;
		int weight_vert_cnt = 0;
		std::vector< Deformer *> deformers ;
		
		CreateDeformersFromPN( deformers, add_deformer_cmds , numWeights);
			
		// generate name->index map from nodes 
		for( int nNode = 0 ; nNode < (int)pModel->m_FlatNodeList.GetSize() ; nNode++ )
		{
			char *pctmp = pModel->GetNode( nNode )->GetName() ;
			NodeNameToIndexMap[ pctmp ] = nNode ;
		}
		
		// allocate on numWeights for the model
		if(!pModel->m_VertexWeights.SetSize2(numWeights, pModel->GetAlloc()))
		{
			// error out of mem, crash/burn
			status = status.OUT_OF_MEM ;
			status.m_msg = "out of mem allocating weights ... " ;
			return ;
		}

		// iterate through deformers setting the pieces up
		for( int DefCnt = 0; DefCnt < (int)deformers.size() ; DefCnt ++ )
		{
			 uint32 cur_piece_index ;
			// find the piece 
			ModelVert *cur_vert ;
			PieceLOD *cur_piece = NULL ;
		
			// Find the piece that corresponds to the deformer 
			for( uint32 piece_search = 0 ; piece_search < PieceList.GetSize() ; ++piece_search)
			{
				if( strcmp(deformers[DefCnt]->target_name.c_str(), 
						   PieceList[piece_search]->GetName()) == 0)
				{
					cur_piece = PieceList[piece_search] ;
					cur_piece_index = piece_search;
					break;
				}
			}

			if(cur_piece == NULL )
			{
				//this deformer's target does not exist!!
				status.m_msg = "deformer found without target shape... " ;
				continue;
			}

			Deformer * cur_def = deformers[DefCnt];

			for( int VertCnt = 0 ; VertCnt < (int)cur_piece->m_Verts.GetSize() ; VertCnt ++ ) 
			{
				cur_vert = &cur_piece->m_Verts[ VertCnt ];

				cur_vert->m_nWeights = cur_def->weightsets[ VertCnt ].size();

				if(cur_vert->m_nWeights > 0)
				{
					cur_vert->m_Weights = &pModel->m_VertexWeights[ weight_vert_cnt ];
					weight_vert_cnt += cur_vert->m_nWeights;

					for( int j=0; j < cur_vert->m_nWeights; j++)
					{	
						float weight ;
						// find the node index from the map 
						uint32 wt_idx = cur_def->weightsets[ VertCnt ][ j ].first ;
						uint32 node_index = NodeNameToIndexMap[ cur_def->influences[ wt_idx ].c_str() ];

						//cur_vert->m_Weights[j].m_iNode  = cur_def->weightsets[ VertCnt ][ j ].first ;
						cur_vert->m_Weights[j].m_iNode = node_index;

						weight = cur_def->weightsets[ VertCnt ][ j ].second ;
						// localize the vertex, lt expects this.
						LocalizeVertex( pModel, cur_piece, cur_vert->m_Weights[j].m_iNode,
										VertCnt, weight,   cur_vert->m_Weights[j].m_Vec  );
					}
				}
			}
			// for transform caches.
			cur_piece->CalcUsedNodeList();
		}

		// Free Deformers here.
		{ 
			int num_defs = deformers.size() ;
			for( int i = 0 ;i < num_defs ; i++ )
			{
				delete deformers[i] ;
			}
			// let scope fall out deal with the rest 
		}
	}
	else  // rats no deformers create default weights set ?.
	{
		status = status.FAIL;
		return ;
	}
}

// types used in cmd_BuildLOD 
struct distTriPair        { float triPercent ; float dist; };
struct pieceNamePriorPair { const char* nameRef; int priority ; };

// ------------------------------------------------------------------------
// BuildLODS
// evaluate the set-repl-lod-original for the current model

/*
	(set-repl-lod-original
		 (piece-priorities ( ("name" prior) ("name" prior) ("name" prior) ))
		 (dists ( 0 1 2 3 ))
		 (tri-% ( 100 90 50 40 ))
	)
	*/
// ------------------------------------------------------------------------
static
void cmd_BuildLOD( CLTANode *add_lod_cmd, Model *pModel, CLTATranslStatus & status  ) 
{
	BuildLODRequest request;
	LODRequestInfo info;
	int  i;
	CLTANode *lod_cmd ;
	std::vector< pieceNamePriorPair > piece_weights;
	std::vector< distTriPair  > dist_tri_prcnt_lst ;
	float NumTris ;

	if (!pModel)
		return;

	// find the lod command in the parse tree
	lod_cmd = CLTAUtil::FindList( add_lod_cmd ,"set-repl-lod-original");

	// quit if it ain't there.
	if( lod_cmd == NULL )
		return ;

	status.AddToLog("Building LODS\r\n");

	// get the number of triangles the model has now.
	NumTris = (float)pModel->CalcNumTris();
	
	// get peice-weights from node
	CLTANode *piece_weightsPN = CLTAUtil::FindList( lod_cmd, "piece-priorities");

	if( piece_weightsPN == NULL )
	{
		// bad error incomplete node 
		return ;
	}

	piece_weightsPN = piece_weightsPN->GetElement(1) ;// (cdr (piece-weights ( ... ))
	
	// piece_weights = ( ( name value ) ( name value ) ... )
	for( i = 0 ;i < (int)piece_weightsPN->GetNumElements() ; i++ )
	{
		pieceNamePriorPair item ;
		CLTANode *name_val_pair = piece_weightsPN->GetElement(i);

		item.nameRef   = name_val_pair->GetElement(0)->GetValue () ;
		item.priority= atoi( name_val_pair->GetElement(1)->GetValue() );

		piece_weights.push_back( item );
	}
	
	
	CLTANode *tri_prcnt = CLTAUtil::FindList( lod_cmd, "tri-%"); // 
	CLTANode *dist_lst  = CLTAUtil::FindList( lod_cmd, "dists");

	if( tri_prcnt == NULL || dist_lst == NULL )
	{
		status = status.FAIL ;
		status.m_msg = "set-repl-lod-original nodes tri- or dists are missing, incomplete node";
		// bad error incomplete node ...
		return ;
	}
	
	tri_prcnt = tri_prcnt->GetElement(1) ; // cdr on (tri-% ( ... ) )
	dist_lst  = dist_lst ->GetElement(1) ; // cdr on (dists ( ... ) )

	for( i = 0 ; i < (int)tri_prcnt->GetNumElements() ; i++ )
	{
		distTriPair item ;

		item.triPercent = (float)atof( tri_prcnt->GetElement(i)->GetValue ()  ) ;
		item.dist       = (float)atof( dist_lst ->GetElement(i)->GetValue ()  ) ;
		dist_tri_prcnt_lst.push_back( item ) ;
	}

	// we have nothing to lod, let's move on.
	if( tri_prcnt->GetNumElements() == 0 )
		return ;

	// ------------------------------------------------------------------------
	// build up the request from the pn data 
	// ------------------------------------------------------------------------
	
	request.m_pModel = pModel;


	// Init the piece weights.
	request.m_PieceWeights.SetSizeInit2(pModel->NumPieces(), 1.0f);

	// for now we are assuming that the model pieces are in the same order
	// as the exporters... Normally we should search the names.
	for( i = 0 ;i < (int)pModel->NumPieces() ; i++ )
	{
		request.m_PieceWeights[i]= (float)piece_weights[i].priority ;
	}
	

	// create the lod infos so we can build those levels.
	for( i = 0 ;i < (int)dist_tri_prcnt_lst.size() ; i++ )
	{
		info.m_Dist = dist_tri_prcnt_lst[i].dist ;
		info.m_nTris= (uint)(dist_tri_prcnt_lst[i].triPercent  * NumTris) ; 
		request.m_LODInfos.Append(info);
	}
		
	if (!BuildLODs(&request))
	{
		status.m_msg = "cmd_BuildLOD Error: BuildLODs failed " ;
		status       = status.FAIL;
		status.OnError("cmd_BuildLOD");
		return ;
	}
	
	// ok we have generated lods, now lets push down the material definitions 
	// to all lods.
	uint32 num_lods = dist_tri_prcnt_lst.size();

	for ( uint32 pieces = 0 ; pieces < pModel->NumPieces() ; pieces++ )
	{
		for( uint32 piece_lod = 1 ; piece_lod < num_lods ; piece_lod++ )
		{
			PieceLOD *pLOD = pModel->GetPiece(pieces)->GetLOD( piece_lod );
            for( int num_tex = 0 ; num_tex < MAX_PIECE_TEXTURES ; ++num_tex )
            {
			    pLOD->m_iTextures[num_tex] = pModel->GetPiece(pieces)->GetLOD(uint32(0))->m_iTextures[num_tex] ;
            }
		}
	}
}


// ------------------------------------------------------------------------
//  (socket "sock_name" 
	     //(parent "node-name")
	     //(pos ( x y z )) 
	     //(quat ( a b c d ))))
//
//		 ==> ModelSocket
// ------------------------------------------------------------------------
static 
void ParseSocketPN( CLTANode *socketPN, Model *pModel, ModelSocket *pSocket )
{
	CLTANode *parent ;
	const char *name = socketPN->GetElement(1)->GetValue() ;
	const char *parent_name ;

	if( name != NULL )
	{
		// set the name 
		strcpy( pSocket->m_Name , name );

		// get the parent node 
		parent = CLTAUtil::ShallowFindList( socketPN, "parent");
		parent_name = parent->GetElement(1)->GetValue() ; // ( parent "name " )

		pModel->FindNode( parent_name , &pSocket->m_iNode );
	//	pSocket->m_iNode = node_index ;

		// get the pos / quat 
		CLTANode *ParseNode = CLTAUtil::ShallowFindList( socketPN, "pos");
		LTA::ParseVector( ParseNode->GetElement(1), &(pSocket->m_Pos.x) );

		ParseNode = CLTAUtil::ShallowFindList( socketPN, "quat") ;
		LTA::ParseVector( ParseNode->GetElement(1), pSocket->m_Rot.m_Quat );
		
		ParseNode = CLTAUtil::ShallowFindList( socketPN, "scale");
		if(ParseNode!=NULL)
			LTA::ParseVector( ParseNode->GetElement(1), &(pSocket->m_Scale.x) );

	}else 
	{
		// error socket requires a name 
		CLTATranslStatus error ;
		error << " error socket requires a name " ; 
		error.OnError("parse socket nodes" ,0);
	}
	
}


// ------------------------------------------------------------------------
// read :
/*
(add-sockets (
 (socket (socket "sock_name" 
	     (parent "node-name")
	     (pos ( x y z )) 
	     (quat ( a b c d ))))
	)
)
*/
// ------------------------------------------------------------------------
static 
void cmd_ApplyAddSockets( CLTANode *pnroot, Model *pModel, CLTATranslStatus & status )
{
	CLTANode *cmd ;
	std::vector< CLTANode *> add_sock_cmds ;
	
	cmd = CLTAUtil::FindList( pnroot, "add-sockets");

	if( cmd == NULL )
		return ; // no add-sockets commands .

	CLTAUtil::FindAll( add_sock_cmds, cmd, "socket" );

	int numSocks = add_sock_cmds.size() ;

	// no socks no go
	if( numSocks == 0 )
		return ;

	// allocate model's sockets 
	if(!pModel->m_Sockets.SetSizeInit4(numSocks, LTNULL, pModel->GetAlloc()))
	{
		status = status.OUT_OF_MEM ;
		status.m_msg = "if(!pModel->m_Sockets.SetSizeInit4(numSocks, LTNULL, pModel->GetAlloc())) FAILED";
		return ;
	}
	
	// go through the add-socket commands; build the sockets
	for( int i = 0 ;i < (int)add_sock_cmds.size() ; i++ )
	{
		CLTANode *add_sock = add_sock_cmds[i] ; 
		ModelSocket *pSocket = LNew(pModel->GetAlloc(), ModelSocket);
		
		if(!pSocket)
		{
			status = status.OUT_OF_MEM ;
			status.m_msg = " LNew on ModelSocket FAILED " ;
			return ;
		}

		ParseSocketPN( add_sock, pModel, pSocket );
		pModel->m_Sockets[i] = pSocket;
	}
}


// ------------------------------------------------------------------------
// child model info structure used in loading child models
// ------------------------------------------------------------------------
struct ChildModel {

	ChildModel():file_name(NULL), scale_skel(0) {}
	~ChildModel() {
		if( file_name != NULL ) free(file_name); //delete [] file_name ;
	}

	char * file_name ;
	int save_index ;
	int scale_skel;
	
	struct relation { 
		relation() { pos[0]=pos[1]=pos[2]=0.0f; quat[0]=quat[1]=quat[2]=0.0f ;quat[3]=1.0f;}
		float pos[3]; float quat[4] ; 
	};

	vector<relation> node_relations ;
};

// ------------------------------------------------------------------------
// 
// ------------------------------------------------------------------------
static
void ChildModelFromPN( ChildModel & cm, CLTANode *pn_cm )
{
	CLTANode *pn_list ;
	pn_list = CLTAUtil::ShallowFindList( pn_cm, "filename");
			
	if( pn_list == NULL )
	{
		CLTATranslStatus::AddToLog("Warning: child model node missing filename ... skipping\r\n");
		return ;
	}
				
	cm.file_name = strdup( pn_list->GetElement(1)->GetValue() );

	pn_list = CLTAUtil::ShallowFindList( pn_cm,"save-index");
	if(pn_list)
		cm.save_index = atoi( pn_list->GetElement(1)->GetValue());

	pn_list = CLTAUtil::ShallowFindList( pn_cm,"scale-skel");
	if( pn_list )
		cm.scale_skel = 1;

	pn_list = CLTAUtil::ShallowFindList( pn_cm, "node-relations" );

	if( pn_list )
	{
		pn_list = pn_list->GetElement(1) ; // pop list
		CLTANode *relationpn, *tmp ;
		for( uint32 i = 0 ; i < pn_list->GetNumElements() ; i++ )
		{
			relationpn = pn_list->GetElement(i);
			ChildModel::relation relation;
			
			tmp = CLTAUtil::ShallowFindList(relationpn, "pos");
			if( tmp != NULL )
				LTA::ParseVector(tmp->GetElement(1), relation.pos);

			tmp = CLTAUtil::ShallowFindList(relationpn, "quat");
			if( tmp != NULL )
				LTA::ParseVector(tmp->GetElement(1), relation.quat);
			
			cm.node_relations.push_back(relation);

		}
	}
}


// ------------------------------------------------------------------------
// apply the add-child-models cmd
// ------------------------------------------------------------------------
static
void cmd_ApplyAddChildModels(	CLTANode *pnroot, Model *pModel, 
								CLTATranslStatus & status )
{
	CLTANode *add_childmodels ;
	int			list_size ;
	int			i ;
	char		errString[1024];

	ChildInfo *pChildInfo = pModel->GetSelfChildModel();

	add_childmodels = CLTAUtil::FindList( pnroot, "add-childmodels");

	// no childmodels? ok move along.. 
	if( add_childmodels == NULL )
	{
		return ;
	}
		
	add_childmodels = add_childmodels->GetElement(1) ; // cdr on (add-childmodels 
	
	list_size = add_childmodels->GetNumElements() ;
	
	pModel->InitChildInfo( 0, pChildInfo, pModel, "SELF");

	for(  i = 0 ;i < list_size ; i++ )
	{
		string tmp_filename;
		Model *child_model = NULL;
		ChildModel cm;

		ChildModelFromPN( cm, add_childmodels->GetElement(i));

		string filename (cm.file_name) ;
		int save_index       = cm.save_index ;
		int scale_skel  = cm.scale_skel ;

		CLTATranslStatus::AddToLog( "\tLoading Child Model :" );
		CLTATranslStatus::AddToLog( filename );
		CLTATranslStatus::AddToLog( "\r\n" );

		// check for lta and ltc.  warn if both found, but still load the lta
		bool ltaExists = false;
		bool ltcExists = false;

		string ltaFilename = s_CurBasePath + filename + ".lta";
		string ltcFilename = s_CurBasePath + filename + ".ltc";

		FILE* testFile;

		if( testFile = fopen( ltaFilename.c_str(), "rb" ) )
		{
			ltaExists = true;
			fclose( testFile );
		}
		
		if( testFile = fopen( ltcFilename.c_str(), "rb" ) )
		{
			ltcExists = true;
			fclose( testFile );
		}

		// both files found, issue a warning
		if( ltaExists && ltcExists )
		{
			CLTATranslStatus::AddToLog( "Warning: found .lta and .ltc files.  Using .lta.\r\n" );
		}

		// load the child model
		if( ltaExists )			// load the lta file
		{
			filename += ".lta";
			child_model = ltaLoadChildModel( ltaFilename.c_str(), pModel->GetAlloc() );

		}
		else if( ltcExists )	// load the ltc file
		{
			filename += ".ltc";
			child_model = ltaLoadChildModel( ltcFilename.c_str(), pModel->GetAlloc() );
		}
		
		if( child_model == NULL )
		{
			CLTATranslStatus::AddToLog( "Error: ");
			CLTATranslStatus::AddToLog( filename.c_str());
			CLTATranslStatus::AddToLog( " ChildModel Load Failed...\r\n" );
			continue ;
		}

		child_model->SetSaveIndex( save_index );
		
		ChildInfo *pChildInfo = pModel->AddChildModel( child_model, 
													  (char*)filename.c_str(), 
														errString, 
														scale_skel == 1);

		// add the child model to 
		if( !pChildInfo )
		{
			CLTATranslStatus::AddToLog( "Error : " );
			CLTATranslStatus::AddToLog( errString );
			CLTATranslStatus::AddToLog( "\r\n" );
		}
		else
		{
			float *tmp; // holder.
			// add relations 
			for( uint32 rel_cnt = 0; rel_cnt < cm.node_relations.size() ; ++rel_cnt )
			{
				tmp = cm.node_relations[rel_cnt].pos;
				pChildInfo->m_Relation[rel_cnt].m_Pos.Init(tmp[0],tmp[1],tmp[2]);
				tmp = cm.node_relations[rel_cnt].quat;
				pChildInfo->m_Relation[rel_cnt].m_Rot.Init(tmp[0],tmp[1],tmp[2],tmp[3]);
			}
		}
	} // for create new ChildInfos
}


// ------------------------------------------------------------------------
// Apply addchildmodels as references
// add the child models to the model db as references not as real child model data.
// Normally this would break anything trying to get animation information out of the 
// parent model db, but we are only going to load up the model to save it out again, and
// we don't need the child info data.
// ------------------------------------------------------------------------
static
void cmd_ApplyAddChildModelsAsReferences(	
								CLTANode *pnroot, Model *pModel, 
								CLTATranslStatus & status )
{
	CLTANode *add_childmodels ;
	int			list_size ;
	int			i ;
	char		errString[1024];

	ChildInfo *pChildInfo = pModel->GetSelfChildModel();

	add_childmodels = CLTAUtil::FindList( pnroot, "add-childmodels");

	// no childmodels? ok move along.. 
	if( add_childmodels == NULL )
	{
		return ;
	}
		
	add_childmodels = add_childmodels->GetElement(1) ; // cdr on (add-childmodels 
	
	list_size = add_childmodels->GetNumElements() ;
	
	pModel->InitChildInfo( 0, pChildInfo, pModel, "SELF");

	for(  i = 0 ;i < list_size ; i++ )
	{
		string tmp_filename ;
		ChildModel cm ;

		ChildModelFromPN( cm, add_childmodels->GetElement(i));

		string filename (cm.file_name) ;
		int save_index  = cm.save_index ;
		int scale_skel  = cm.scale_skel ;

		CLTATranslStatus::AddToLog( "\tAdding Child Model Reference:" );
		CLTATranslStatus::AddToLog( filename );
		CLTATranslStatus::AddToLog( "\r\n" );

		pModel->AddChildModelRef( (char*)filename.c_str(), errString );
		
	} // for create new ChildInfos
}

// ------------------------------------------------------------------------
// set the weight set from the lta to the model
// read in :
// (anim-weightsets (
//   (anim-weightset (name "name" ) (weights ( a b c ... ))
//   .. 
// ))	
// ------------------------------------------------------------------------
static
void cmd_SetWeightSets( CLTANode * pnroot, Model *pModel, CLTATranslStatus &status )
{

	CLTANode *anim_ws = CLTAUtil::FindList(pnroot, "anim-weightsets");
	
	// if anim-weightsets does not exists check for add-animweightsets ... 
	// if that's not there either move on.
	if( anim_ws == NULL)
	{
		anim_ws = CLTAUtil::FindList( pnroot, "add-animweightsets" );
		if( anim_ws == NULL )
			return ;
	}
	

	anim_ws = anim_ws->GetElement(1) ; // cdr on list 

	 
//	pModel->m_WeightSets.SetSize2(anim_ws->GetNumElements(), pModel->GetAlloc() );

	for( int iWsCnt = 0 ; iWsCnt < (int)anim_ws->GetNumElements() ; iWsCnt++ )
	{
		const char *name ;
		CLTANode *wspn ;
		CLTANode *pn ;

		wspn = anim_ws->GetElement( iWsCnt );

		pn = CLTAUtil::FindList(wspn, "name");
		if( pn )
			name = pn->GetElement(1)->GetValue() ;
		else 
			name = NULL ;

		pn = CLTAUtil::FindList( wspn, "weights" );

		if( pn ) 
			pn = pn ->GetElement(1) ; // cdr the weights lst

		WeightSet* pWtSet = new WeightSet( pModel ) ;

		pWtSet->SetName( name );
		int nWeights = pn->GetNumElements();

		pWtSet->m_Weights.SetSize2(nWeights, pModel->GetAlloc());


		for( int iwsvals = 0 ; iwsvals < (int)pn->GetNumElements() ; iwsvals++ )
		{
			float val = (float)atof( pn->GetElement(iwsvals)->GetValue() );
			pWtSet->m_Weights[ iwsvals ] = val ;
		}
		
		if( pModel->NumNodes() != pWtSet->m_Weights.GetSize() )
		{
			
			status.AddToLog( string("Error adding Weight set.") + "\r\n");
			char err_str[256];
			sprintf(err_str,"Weight set \"%s\" node mismatch  ", pWtSet->GetName() );
			status.AddToLog( string("\t") + string(err_str) + "\r\n");
		}

		pModel->AddWeightSet( pWtSet );
	}

}


// ------------------------------------------------------------------------
// for every anim-binding in the lta set the model- anim data to it
// (anim-bindings  (
//    (anim-binding (name "name" ) (dims ( x y z ) ) (translation ( x y z )) (interp-time x))
// )
// ------------------------------------------------------------------------
static
void cmd_SetAnimBindings( CLTANode *pnroot, Model *pModel, CLTATranslStatus &status )
{
	CLTANode *anim_bindings = CLTAUtil::FindList(pnroot, "anim-bindings" );
	
	if( anim_bindings )
	{
		anim_bindings = anim_bindings->GetElement(1) ; // cdr the node

		if( anim_bindings )
		{
			for( int iCurBind = 0 ; iCurBind < (int)anim_bindings->GetNumElements() ; iCurBind++ )
			{
				const char	*model_anim_name  = NULL ;
				const char  *weight_set_name  = NULL ;

				LTVector dim( 0.0f , 0.0f , 0.0f);
				LTVector trans( 0.0f , 0.0f , 0.0f);
				uint32   interp_time = LTA_DEFAULT_ANIM_INTERP_TIME  ; 

				CLTANode *abPN = anim_bindings->GetElement(iCurBind);
				CLTANode *tmp ;
				
				// slurp up data 
				// get (name "name")
				model_anim_name = NULL ;
				tmp = CLTAUtil::ShallowFindList( abPN, "name"); 
				if( tmp  && tmp->GetElement(1)->IsString() )
				{
					model_anim_name = tmp->GetElement(1)->GetValue();
				}
				else {
					continue ; // why bother, bad name 
				}

				// get (dims ( x y z ))
				tmp = CLTAUtil::ShallowFindList( abPN, "dims" ); 
				if( tmp )
				{
					tmp = tmp->GetElement(1);
					LTA::ParseVector( tmp, &dim.x );				
				}

				tmp = CLTAUtil::ShallowFindList( abPN, "translation" );
				if( tmp )
				{
					tmp = tmp->GetElement(1);
					LTA::ParseVector( tmp, &trans.x );				
				}

				tmp = CLTAUtil::ShallowFindList( abPN, "interp-time" );
				if( tmp )
				{
					interp_time = atoi( tmp->GetElement(1)->GetValue() );
				}

				// get associated weight set if there's one
				tmp = CLTAUtil::ShallowFindList( abPN, "weight-set");
				if( tmp )
				{
					weight_set_name = tmp->GetElement(1)->GetValue() ;
				}


				//- spit 
				uint32 index ;
				AnimInfo *anim_info = pModel->FindAnimInfo( (char*)model_anim_name, pModel, &index );

				if( anim_info == NULL )
				{
					// woops... no model anim
					continue ;
				}

				if( anim_info->m_pAnim )
				{
					anim_info->m_pAnim->m_InterpolationMS = interp_time ;
				}

				anim_info->			m_vDims = dim ;
				anim_info->	m_vTranslation = trans ; 

				if( weight_set_name != NULL ) 
				{
					uint32 index ;
					WeightSet *WtSet = pModel->FindWeightSet( (char*)weight_set_name, &index );
					
					if(WtSet && anim_info->m_pAnim)
					{
						anim_info->m_pAnim->m_ModelEditWeightSet = index ;
					}
				}
				// ok, next one
			}// for each binding

		}// if there's a bindings list
	}// if there's a binding node
}

// ------------------------------------------------------------------------
// : (set-node-flags ( ("node-name" flag) ("node-name" flag ) ))
// ------------------------------------------------------------------------
static
void cmd_SetNodeFlags( CLTANode *pnroot, Model *pModel, CLTATranslStatus &status )
{

	CLTANode *node_flags = CLTAUtil::FindList(pnroot, "set-node-flags" );
	
	if( node_flags == NULL )
		return ;

	node_flags = node_flags->GetElement(1) ;

	if( node_flags->GetNumElements() > 0)
	{
		for( int iNode = 0 ;iNode < (int)node_flags->GetNumElements() ; iNode++ )
		{
			ModelNode *pNode ;
			CLTANode *n_f_pair = node_flags->GetElement( iNode ) ;
			const char *str = n_f_pair->GetElement(0)->GetValue();
			int value = atoi(n_f_pair->GetElement(1)->GetValue());

			pNode = pModel->FindNode( str );
			pNode->m_Flags = value ;
		}
	}
}

// ------------------------------------------------------------------------
// parse into model
// : (set-command-string "a string" )
// ------------------------------------------------------------------------
static
void cmd_GetCommandString( CLTANode *pnroot, Model *pModel, CLTATranslStatus &status )
{

	CLTANode *cmd_str = CLTAUtil::FindList(pnroot, "set-command-string" );
	
	if( cmd_str && cmd_str->GetNumElements() > 1)
	{
		const char *str = cmd_str->GetElement(1)->GetValue();

		pModel->m_CommandString = pModel->AddString( str );
	}
	
}

// ------------------------------------------------------------------------
// (set-global-radius float)
// ------------------------------------------------------------------------
static
void cmd_SetGlobalRadius( CLTANode *pnroot, Model* pModel, CLTATranslStatus &status )
{
	CLTANode *cmd_str = CLTAUtil::FindList(pnroot, "set-global-radius" );
	
	if( cmd_str && cmd_str->GetNumElements() > 1)
	{
		float val = (float)atof(cmd_str->GetElement(1)->GetValue());

		pModel->m_GlobalRadius = val ;
	}
}


// ------------------------------------------------------------------------
// cmd_ApplyCreateLodGroup. 
// apply the create-lod-group command from the lta file to the model db.
// returns 0 on no-work.
//         1 on ok.
// ------------------------------------------------------------------------
static uint32
cmd_ApplyCreateLodGroup(CLTANode *cmd_root, 
						Model    *pModel, 
						CMoArray<PieceLOD*,NoCache> & PieceList,
						CLTATranslStatus & status )
{
	CLTANode *clg = CLTAUtil::FindList(cmd_root, "lod-groups");
	uint32 i, model_piece_cnt = 0 ;
	
	// no lod groups jump out 
	if( clg == NULL ) 
		return 0;

	status.AddToLog("Assembling custom LODS\r\n");

	clg = clg->GetElement(1); // get the list 
	
	pModel->m_Pieces.SetCacheSize( clg->GetNumElements() );

	// for every lod group
	for( uint32 list_elem = 0 ; list_elem < clg->GetNumElements() ; list_elem++)
	{
		string grp_name ;
		vector<float> dists ;
		vector<string> names ;
		CLTANode *create_lod_grouppn, *distspn, *shapespn;
		
		// ------------------------------------------------------------------------
		// parse the create-lod-group node.
		// ------------------------------------------------------------------------

		// get (create-lod-group) node
		create_lod_grouppn = clg->GetElement(list_elem);
		// get the lod group name
		grp_name = create_lod_grouppn->GetElement(1)->GetValue();
		status.AddToLog(grp_name + "\r\n");

		// get the lod-dist node
		distspn				= CLTAUtil::ShallowFindList(create_lod_grouppn, "lod-dists");
		shapespn			= CLTAUtil::ShallowFindList(create_lod_grouppn, "shapes");
	
		if( distspn && shapespn )
		{
			distspn = distspn->GetElement(1); // pop the data list 
			shapespn = shapespn->GetElement(1); // pop the data list 

			// get the dist&shape values
			for( i = 0 ; i <distspn->GetNumElements() ; i++ )
			{
				dists.push_back( (float)atof( distspn->GetElement(i)->GetValue() ));
				names.push_back( shapespn->GetElement(i)->GetValue() );
			}
		}
		else 
		{
			// error ; need both dists and shapes 
		}

		int32 min,max ;
		CLTANode *minmaxpn= CLTAUtil::ShallowFindList( create_lod_grouppn, "min-max-offset");

		
		// ------------------------------------------------------------------------
		// Transfer the parsed data to model data-base.
		// Go through the pieces. Find the piece-lod that matches the lod group 
		// name. Then go through all the names in the lod group, getting
		// the pieces out of the master list and adding them to the model-piece.
		// ------------------------------------------------------------------------
		
		ModelPiece *model_piece = LNew_1P(pModel->GetAlloc(), ModelPiece, pModel );
		model_piece->SetName( (char*)grp_name.c_str());

		// get min-max offsets
		if( minmaxpn && minmaxpn->GetNumElements() >= 3)
		{
			min = atoi(minmaxpn->GetElement(1)->GetValue());
			max = atoi(minmaxpn->GetElement(2)->GetValue());

			model_piece->SetMinMaxLODOffset( min,max );
		}

		// warn cmoarray of immanent arrival of more data.
		model_piece->m_LODs.SetCacheSize(names.size());

		// find the lod geoms that belong to this piece, addit it the model-piece
		for( uint32 ilod = 0 ; ilod < names.size() ; ilod++ )
		{
			//see if we have a NULL LOD, in which case we can just add our own 0 vert LOD
			if(stricmp(names[ilod].c_str(), "NULL") == 0)
			{
				//it is a null LOD piece, so just add a dummy one
				model_piece->AddNullLOD(dists[ilod]);
				continue;
			}

			PieceLOD *pLOD =NULL ;
		
			// search for the lod-geom by name in the list of all already read-in shapes.
			// if a piece is null, then its been used.
			for( i = 0 ; i < PieceList.GetSize() ; i++ )
			{
				if( PieceList[i] != NULL &&
					strcmp(names[ilod].c_str(), PieceList[i]->GetName()) == 0)
				{
					pLOD = PieceList[i];
					// we've transfered the piece over to the model-piece db. 
					// Remove it from the list.
					PieceList[i] = NULL ;
					break;
				}
			}
			if( pLOD == NULL )
			{
				// we have no lods... 
				status.AddToLog("WARNING: Could not find SHAPE " + names[ilod] + " for group" + grp_name);
				continue;
			}
			
			model_piece->AddLOD(*pLOD,dists[ilod]);

			delete pLOD;
		}

		// add the new model piece to the model data-base.
		pModel->m_Pieces.Append( model_piece );	
	}// endfor

	status.AddToLog(" done ");
	return 1; 
}

// ------------------------------------------------------------------------
// apply modifiers
// apply stuff from the on_load_cmds node.
// ------------------------------------------------------------------------
static 
void ApplyModifiers( CLTANode *pnroot, Model *pModel, bool bChildRefs=false )
{
	CLTATranslStatus status ;
	
	status.reset();
//	CLTATranslStatus::AddToLog( "Building LOD\r\n");

	// if there is the old style build lod command, process it here.
	cmd_BuildLOD( pnroot, pModel, status );

	if( status!= status.OK ) 
	{
		status.OnError("BuildLOD Failed");
		//return ;
	}
	
	status.reset();
	CLTATranslStatus::AddToLog( "Apply Sockets\r\n");
	cmd_ApplyAddSockets( pnroot, pModel, status );

	if( status != status.OK )
	{
		status.OnError("ApplyModifiers Failed");
		//return ;
	}

	status.reset();
	CLTATranslStatus::AddToLog( "ChildModels:\r\n------------------------\r\n");

	// if we are loading in the child models as references do so, otherwise
	// really load them up.
	if( bChildRefs )
		cmd_ApplyAddChildModelsAsReferences( pnroot, pModel, status );
	else
		cmd_ApplyAddChildModels( pnroot, pModel, status );
	
	if( status != status.OK )
	{
		status.OnError("Failed To Load ChildModels");
		// This is really ok, so we failed to load childmodels...
		// they probably weren't there.
		//return ;
	}

	CLTATranslStatus::AddToLog( "\r\n------------------------\r\n");

	status.reset();
	CLTATranslStatus::AddToLog( "Setting AnimBindings\r\n");
	cmd_SetAnimBindings( pnroot, pModel, status  );

	if( status != status.OK )
	{
		status.OnError( "ApplyModifiers Failed");
		//return ;
	}

	status.reset();
	CLTATranslStatus::AddToLog( "Setting Weights Sets\r\n");
	cmd_SetWeightSets( pnroot, pModel, status );
	 
	if( status != status.OK )
	{
		status.OnError( "cmd_SetWeightSets Failed");
		//return ;
	}

	// command string
	status.reset();
	CLTATranslStatus::AddToLog( "Setting Command String.\r\n" );

	cmd_GetCommandString( pnroot, pModel, status );

	if( status != status.OK )
	{
		status.OnError( "cmd_GetCommandString Failed");
		return ;
	}

	status.reset();
	CLTATranslStatus::AddToLog( "Setting Model Node Flags.\r\n" );

	cmd_SetNodeFlags( pnroot, pModel, status );

	if( status != status.OK )
	{
		status.OnError( "cmd_SetNodeFlags Failed");
		return ;
	}

	cmd_SetGlobalRadius( pnroot, pModel, status );

	if( status != status.OK )
	{
		status.OnError("cmd SetGlobalRadius Failed");
		return ;
	}
}



// ------------------------------------------------------------------------
// apply modifiers
// apply stuff from the on_load_cmds node that's needed. Don't 
// bother with things that are not going to be seen/used.
// ------------------------------------------------------------------------
static 
void ApplyChildModelModifiers( CLTANode *pnroot, Model *pModel )
{
	CLTATranslStatus status ;

	//cmd_ApplyDeformers( pnroot, pModel, status) ;
	//status.AddToLog( "deformers\r\n");
	//if( status != status.OK )
	//{
		//status.OnError("ApplyModifiers Failed");
		//return ;
	//}
	
	//status.reset();
	//CLTATranslStatus::AddToLog( "Building LOD\r\n");
//
	//cmd_BuildLOD( pnroot, pModel, status );
//
	//if( status!= status.OK ) 
	//{
		//status.OnError("ApplyModifiers Failed");
		//return ;
	//}
	
	status.reset();
	CLTATranslStatus::AddToLog( "Apply Sockets\r\n");
	cmd_ApplyAddSockets( pnroot, pModel, status );

	if( status != status.OK )
	{
		status.OnError("ApplyModifiers Failed");
		return ;
	}

	//status.reset();
	//CLTATranslStatus::AddToLog( "ChildModels\r\n");
//
	//cmd_ApplyAddChildModels( pnroot, pModel, status );
//
	//if( status != status.OK )
	//{
		//status.OnError("ApplyModifiers Failed");
		//return ;
	//}


	status.reset();
	CLTATranslStatus::AddToLog( "AnimBindings\r\n");
	cmd_SetAnimBindings( pnroot, pModel, status  );

	if( status != status.OK )
	{
		status.OnError( "ApplyModifiers Failed");
		return ;
	}

	status.reset();
	CLTATranslStatus::AddToLog( "Set Weights Sets\r\n");
	cmd_SetWeightSets( pnroot, pModel, status );
	 
	if( status != status.OK )
	{
		status.OnError( "ApplyModifiers Failed");
		return ;
	}


	status.reset();
	CLTATranslStatus::AddToLog( "command string\r\n" );

	cmd_GetCommandString( pnroot, pModel, status );

	if( status != status.OK )
	{
		status.OnError( "ApplyModifiers Failed");
		return ;
	}
}


// ------------------------------------------------------------------------
// ParseTreeToModel
// notes :
// 1. convert the parse tree to an lt model 
// The hierarchy node is not a node in itself, its the model. 
// 2. we're doing something funky here, if the hierarchy has more than 
// one child, then the siblings of the first node in the list becomes
// its children. 
// ------------------------------------------------------------------------
static
void ParseTreeToModel(	MetaModel & metaModel, 
						CLTANode *pnroot, CLTANode *load_modifiers,
						Model      *pModel,
						CLTATranslStatus &status )
{
	int i ;
	
	CLTATranslStatus::AddToLog( "Creating Model \r\n" );
	
	// ------------------------------------------------------------------------
	// build the ltmodel node hierarchy
	// ------------------------------------------------------------------------
	// shapes to pieces 
	int nPieces = metaModel.shapes.size();
	PieceLOD *pLOD;

	// shapes read in, used to assemble explict lod groups.
	// T.F ModelPiece (this does not need to be a pointer)
	CMoArray<PieceLOD*,NoCache> PieceList ;

	PieceList.SetSizeInit4(nPieces, LTNULL, pModel->GetAlloc() );

	CLTATranslStatus shp2pce_stat ;
	CLTATranslStatus::AddToLog( "Creating Pieces \r\n" );
	// convert each shape into a piece.
	for( i = 0 ; i < nPieces ; i++ )
	{
		pLOD = new PieceLOD;
//		pLOD = LNew_1P(pModel->GetAlloc(), PieceLOD, pModel );
		if( !pLOD )
		{
			status.m_status = status.FAIL ;
			status.m_msg    = " out of mem ... " ;
			status.OnError("CRITICAL ERROR");
			return ;
		}
		pLOD->Init(pModel);
		PieceList[i] = pLOD ;
		ShapeToPiece(metaModel, metaModel.shapes[i], pLOD ,shp2pce_stat);
		
		if( shp2pce_stat.m_status != CLTATranslStatus::OK )
		{
			shp2pce_stat.OnError("error converting shape to piece " );
			
			return ;// how to handle it here? the caller will bomb out...
		}
	}
	
	// Convert hierarchy nodes from lta in memory format to lt-model 
	CLTATranslStatus nstat ;
	NodeToModelNode( metaModel.rootNode , pModel->GetRootNode(), nstat);

	if( nstat.m_status != nstat.OK )
	{
		nstat.OnError( "->NodeToModelNode error " );
		return ;
	}

	// required for later processing, sets up flatlists and such.
	if(!pModel->PrecalcNodeLists())
	{	
	}

	// get the deformers for each peice from the on load commands.
	cmd_ApplyDeformers( load_modifiers, pModel, PieceList, status) ;
	status.AddToLog( "Applying Deformers\r\n");

	if( status != status.OK )
	{
		status.OnError("ApplyDeformers Failed",false);
		status.reset();
		//return ;
	}

	// reshuffle the pieces based on the create-lod-group load command.
	if(load_modifiers == NULL || 
		cmd_ApplyCreateLodGroup( load_modifiers, pModel, PieceList, status ) == 0)
	{
		// if we didn't do this, then set up parser to manage the procedural build-lod.
		//allocate space for pieces
		if(!pModel->m_Pieces.SetSizeInit4(nPieces, LTNULL, pModel->GetAlloc() ) )
		{
			status.m_status = status.FAIL ;
			status.m_msg    = " out of mem ... " ;
			status.OnError("CRITICAL ERROR");
			return ;
		}
		// turn the shapes into model piece with the shape as the first lod
		for( i = 0 ; i < nPieces ; i++ )
		{
			pModel->m_Pieces[i] = LNew_1P(pModel->GetAlloc(), ModelPiece, pModel );
			pModel->m_Pieces[i]->SetName( (char*)PieceList[i]->GetName() );
				//T.F ModelPiece (Fix use of pointer)
			pModel->m_Pieces[i]->AddLOD( *PieceList[i], 0.0f );
			delete PieceList[i];
		}
	}

	ChildInfo *pChildInfo = pModel->GetSelfChildModel();

	// ------------------------------------------------------------------------
	//  Process animation sets into ModelAnims, stick them into new LT model.
	// ------------------------------------------------------------------------
	
	// get animation data from the parse tree
	int nAnims = metaModel.NumAnimSets() ;
	
	// AnimsToAdd is a collection of model anims to add lt-model container 
	// after we have processed everything. (we may not want all anims loaded etc...)
	vector<ModelAnim*> vAnimsToAdd ;

	// Read in anims

	// -----------------------------------------------------------------------
	// Get animations from lta, convert to mem format possibly merging data.
	// ------------------------------------------------------------------------
	LTA::CClassifiedAnimLst anPairLst ;
 	
	// Anim pairs are animations that are the same but one's a V-A anim and the
	// other is node anim.
	LTA::CollectAndClassifyAnims(metaModel, anPairLst );
	CLTATranslStatus::AddToLog( "Creating AnimNodes \r\n" );

	// Process through the anim pairs
	for( int iCurAnimPair = 0 ;iCurAnimPair < (int)anPairLst.size() ; iCurAnimPair++ )
	{
		pair<int,int> &AnimPair = anPairLst[iCurAnimPair];

		ModelAnim *pAnim = LNew_1P( pModel->GetAlloc() , ModelAnim, pModel);
		
		// store new model-anims without adding them to lt-model.
		// we want to manipulate them before putting them in lt-model
		vAnimsToAdd.push_back( pAnim );

		pAnim->GetRootNode()->m_pNode = pModel->GetRootNode() ;

		// Convert the lta-animation set to lt-modelanim and merge pairs.

		if( LTA::ShouldMerge( anPairLst, iCurAnimPair ) )
		{
			AnimSetToModelAnimSkelMerge( metaModel, AnimPair.first, AnimPair.second, pAnim );
		}else 
		{
			AnimSetToModelAnim( metaModel, AnimPair.first, pAnim );
		}
	}

	// Add the collected 
	// after collecting anims add it to model 
	for( i = 0 ; i < (int)vAnimsToAdd.size() ; i++ )
	{
		ModelAnim *pAnim = vAnimsToAdd[i] ;
		if(	pModel->AddAnimToSelf(pAnim) == NULL )
		{
			MessageBox(NULL, "BIG FAT ERROR ADDING ANIM", "in ParseTreeToModel",MB_OK);//m_Anims[i].m_pChildInfo= & pModel->m_SelfChildModel ;
		}
	}



	// must do this flatten (create index arrays) for anims
	if(!pModel->PrecalcNodeLists())
	{	
	//	MessageBox(NULL,"foo" , "bar" , MB_OK );
	}

	
	// ok we've read in the base model, 

	// VertexAnim hook up.
	// Vertex animated geometry should know how to index into the animations that's changing them.
	//* using vAnimsToAdd if there's any anims get the first one.
	//* look for the first one. traverse the first one looking for nodes that have va data
	//* note the piece lookup, get the associated piece assign to piece the anim node index
	//* move on.
	if( vAnimsToAdd.size() > 0 )
	{
		ModelAnim *pAnimSet = vAnimsToAdd[0] ;
		// using pModel 

		if( pAnimSet != NULL && pAnimSet->m_AnimNodes != NULL )
		{
			for( int i= 0 ; i < (int)pModel->NumNodes() ; i++ )
			{
				AnimNode *animNode = pAnimSet->m_AnimNodes[i];

				if( animNode->isVertexAnim() )
				{
					ModelPiece *pMP = pModel->GetPiece( animNode->GetVertexAnimTarget() );
					pMP->m_vaAnimNodeIdx = i ;
					pMP->m_isVA = 1 ;
				}
			}
		}
	}

	// setup the texture-binding list .
	pModel->m_TextureIndexMap = metaModel.m_ToolsInfo.m_TextureBindingMap ;
	pModel->m_bExcludeGeom    = metaModel.m_ToolsInfo.m_bExcludeGeom ;
	pModel->m_CompressionType = metaModel.m_ToolsInfo.m_CompressionType ;
}


// ------------------------------------------------------------------------
// Model = ltaModelLoad( instream , allocation )
// creates a new model from an lta file.
// ------------------------------------------------------------------------
Model*  ltaModelLoad( CLTAReader & InFile , LAllocCount & allocCount , bool bChildRefs )
{
	BOOL gn_BuildModelVertexNormals(Model *pModel);
	s_bGenerateNormalsForModelPlease = false ;

	MetaModel  *metaModel = new MetaModel;
	CLTATranslStatus status ;	
	
	if( !InFile.IsValid() )
	{
		cerr << " error opening file " << endl;
		delete metaModel;
		return NULL ;
	}

	CLTATranslStatus::AddToLog("loading lta file\r\n");

	//load the file into a tree
	CLTALoadOnlyAlloc Allocator(1024 * 256);

	CLTANode ModelFile;
	if(CLTANodeReader::LoadEntireFile( InFile, &ModelFile, &Allocator) == false)
	{
		delete metaModel;
		return NULL;
	}


	// NOTE: Assumes first element is the type world renderstyle lt-model-0 - get the first node name ( parent )

	CLTANode * check = &ModelFile;

	const char* szString = NULL;

	if (check->IsList() && (check->GetNumElements() > 0))
	{
		CLTANode* pFirstChild = check->GetElement(0);

		if (pFirstChild->IsList() && (pFirstChild->GetNumElements() > 0) )
		{
			CLTANode* pHeader = pFirstChild->GetElement(0);

			szString = pHeader->GetValue();
		}
	}


	if ( szString )
	{

		if ( !strcmp(szString, "world" ) )
		{

			status = status.FAIL;
			status.m_msg = "This is a World file";
			status.OnError("Invalid lta/ltc" );

			delete metaModel;
			ModelFile.Free(&Allocator);
			Allocator.FreeAllMemory();

			return NULL;
		}



		if ( !strcmp(szString, "renderstyle" ) )
		{
			status = status.FAIL;
			status.m_msg = "This is a RenderStyle file";
			status.OnError("Invalid lta/ltc" );

			delete metaModel;
			ModelFile.Free(&Allocator);
			Allocator.FreeAllMemory();
			return NULL;
		}
	}
	

	Model *pModel = new Model( &allocCount ) ;
	
	if( pModel == NULL ) 
	{
		delete metaModel;
		ModelFile.Free(&Allocator);
		Allocator.FreeAllMemory();
		return NULL ;
	}
	
	pModel->SetFileVersion( LTA_MODEL_VERSION );

	// Create The meta model from the parse tree
	LTA::InitMetaModelFromPN( *metaModel, &ModelFile, status );

	// get the on load commands from parse tree.
	CLTANode *on_load_cmds = CLTAUtil::FindList( &ModelFile, "on-load-cmds");

	// converter the meta model into the lt internal format.
	ParseTreeToModel( *metaModel, &ModelFile,on_load_cmds, pModel , status );


	// apply modifiers.
	if( on_load_cmds != NULL )
		ApplyModifiers( on_load_cmds, pModel, bChildRefs );
	

	ChildInfo *pChildInfo = pModel->GetSelfChildModel();

	pModel->SetupChildModelRelation( pChildInfo, pModel, FALSE );

	pModel->ParseCommandString();
	pModel->SetNodeParentOffsets();

	if(metaModel->m_obbset.size())
		status.AddToLog("Setting oriented bounding boxes.\r\n");

	// move the obbs from metamodel to model.
	for( uint32 inode = 0 ; inode < metaModel->m_obbset.size() ; inode++)
	{
		SOBB tmp_obb ;
		ModelNode *pModelNode ;
		CLTAOBB &lta_obb = metaModel->m_obbset[inode];

		tmp_obb.m_Orientation.Init(	lta_obb.m_Orientation[0],
									lta_obb.m_Orientation[1],
									lta_obb.m_Orientation[2],
									lta_obb.m_Orientation[3]);
		
		tmp_obb.m_Pos.Init(lta_obb.m_Pos[0],lta_obb.m_Pos[1],lta_obb.m_Pos[2]);
		tmp_obb.m_Size.Init(lta_obb.m_Size[0],lta_obb.m_Size[1],lta_obb.m_Size[2]);
		
		pModelNode = pModel->FindNode( lta_obb.m_ParentNodeName.c_str() );
		pModelNode->SetOBB(tmp_obb);
		
		if( lta_obb.m_Enabled )
			pModelNode->EnableOBB();
		else
			pModelNode->DisableOBB();

	}

	if( status != status.OK  )
	{
		status.OnError(" error Reading in LTA file " );
		delete metaModel;
		delete pModel;
		ModelFile.Free(&Allocator);
		Allocator.FreeAllMemory();
		return NULL;
	}

	// if the model has no normals, generate some.
	if(s_bGenerateNormalsForModelPlease)
	{
		CLTATranslStatus::AddToLog("generating normals.\r\n");
		gn_BuildModelVertexNormals( pModel );
	}


	CLTATranslStatus::AddToLog( "--- done --- \r\n" );
	

	delete metaModel;

	//clean up the LTA 
	ModelFile.Free(&Allocator);
	Allocator.FreeAllMemory();


	//void model_CreateDefaultOBB(Model*);
	//model_CreateDefaultOBB( pModel );

	return pModel;
}


// ------------------------------------------------------------------------
// Model = ltaModelLoad( instream , allocation )
// creates a new model from an lta file.
// ------------------------------------------------------------------------
Model*  ltaModelLoad( const char * filename, LAllocCount & allocCount, bool bChildRefs  )
{
	BOOL gn_BuildModelVertexNormals(Model *pModel);
	s_bGenerateNormalsForModelPlease = false ;

	CLTAReader InFile;
	InFile.Open(filename, CLTAUtil::IsFileCompressed(filename));

	if( !InFile.IsValid() )
	{
		CLTATranslStatus status ;
		status << " could not open model file : " ;
		status << filename ;
		status.OnError( "ltaModelLoad",0);
		return NULL ;
	}

	return ltaModelLoad(InFile, allocCount, bChildRefs);

}


// ------------------------------------------------------------------------
// ltaLoadChildModel
// Child models don't need to be as complete as main models.
// so all we're going to load is the base model and some of the anim bindings.
// returns NULL if some sort of error has occured
// this is a little redundant but what is programming really but the unwinding of the great mystery which curls uppon
// itself like a snake in its lair, 
// ------------------------------------------------------------------------
Model *ltaLoadChildModel( const char* filename, LAlloc * allocCount  )
{
	BOOL gn_BuildModelVertexNormals(Model *pModel);
	s_bGenerateNormalsForModelPlease = false ;

	MetaModel  *metaModel = new MetaModel;
	CLTATranslStatus status ;	

	//the allocator for the child model
	CLTALoadOnlyAlloc Allocator(1024 * 256);

	CLTANode ModelFile;
	
	if( CLTANodeReader::LoadEntireFile(filename, CLTAUtil::IsFileCompressed(filename), &ModelFile, &Allocator) == false)
	{
		status.m_msg = "could not open file " ;
		status.m_msg += filename ;
		status.OnError("Opening ChildModel",0);
		delete metaModel ;
		return NULL ;
	}
		
	Model *pModel = new Model( allocCount ) ;
	
	if( pModel == NULL )
	{
		status.m_msg ="Error : Out of memory, failed new in ltaLoadChildModel";
		status.OnError("ltaLoadChildModel",0);
		delete metaModel ;
		return NULL ;
	}
	
	pModel->SetFileVersion( LTA_MODEL_VERSION );

	LTA::InitMetaModelFromPN(*metaModel, &ModelFile, status);


	
	// on load commands 
	// important ...
	CLTANode *on_load_cmds = CLTAUtil::FindList( &ModelFile, "on-load-cmds");

	ParseTreeToModel( *metaModel, &ModelFile,on_load_cmds, pModel , status );

	if( status != status.OK  )
	{
		//OutputDebugString(" error in ParseTreeToModel : \n" );
		//OutputDebugString( status.m_msg.c_str());
		status.OnError(" error Reading in LTA file " );
		
		delete metaModel ;
		return NULL;
	}

	// on load commands 
	// important ...
	if( on_load_cmds )
		ApplyChildModelModifiers( on_load_cmds, pModel );
	else { status.AddToLog("WARNING: lta file contains no on-load-cmds node.\n\r");}

	ChildInfo *pChildInfo = pModel->GetSelfChildModel();

	pModel->SetupChildModelRelation( pChildInfo, pModel, FALSE );

	pModel->ParseCommandString();
	pModel->SetNodeParentOffsets();


	// if the model has no normals, generate some.
	if(s_bGenerateNormalsForModelPlease)
			gn_BuildModelVertexNormals( pModel );

	delete metaModel ;

	ModelFile.Free(&Allocator);
	Allocator.FreeAllMemory();

	return pModel;

}
////////////////////////////////////////////////////////////////////////////////////////////////

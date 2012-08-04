
#ifndef __PS2
#include <windows.h>
#endif
#include "bdefs.h"
#include "model.h"
#include "model_ops.h"
#include "conparse.h"
#include "geomroutines.h"

#include <math.h>
#include <float.h>


static int g_iCreateTransformKeys[2];
static float g_CreateTransformPercent;
static bool g_bRecurseFlipZ;
static Model *g_pRecurseModel;
static Model *g_pRecurseChildModel;
static LTMatrix *g_pRecurseTransforms;
static ModelAnim *g_pRecurseAnim;
static CMoArray<LTMatrix> *g_pNodeRelation;
static NodeControlFn g_pNC;
static void *g_pNCUserData;

extern void* dalloc(unsigned long size);
extern void* dalloc_z(unsigned long size);
extern void dfree(void *ptr);


#include <vector> // stl.
#include <set>

void model_SetAnimTrans(Model *pModel, uint32 iAnim, LTVector *pNewTrans)
{
	uint32 i;
	ModelAnim *pAnim = pModel->GetAnimCheck(iAnim);
	if(!pAnim)
		return;

	// Translate it.
	for(i=0; i < pAnim->m_KeyFrames; i++)
	{
		pAnim->GetRootNode()->m_KeyFrames[i].m_Translation = *pNewTrans;
	}
}


void model_OffsetAnimTrans(Model *pModel, uint32 iAnim, LTVector *pOffset)
{
	uint32 i;

	ModelAnim *pAnim = pModel->GetAnimCheck(iAnim);
	if(!pAnim)
		return;

	// Translate it.
	for(i=0; i < pAnim->m_KeyFrames; i++)
	{
		pAnim->GetRootNode()->m_KeyFrames[i].m_Translation += *pOffset;
	}
}


void model_RotateAnim(Model *pModel, uint32 iAnim, LTVector *pRot)
{
	uint32 i;
	LTMatrix mat[3];
	NodeKeyFrame *pKey;
	ModelAnim *pAnim;

	pAnim = pModel->GetAnimCheck(iAnim);
	if(!pAnim)
		return;

	gr_SetupMatrixEuler(mat[0].m, VEC_EXPAND(*pRot));

	// Translate it.
	for(i=0; i < pAnim->m_KeyFrames; i++)
	{
		pKey = &pAnim->GetRootNode()->m_KeyFrames[i];
		pKey->m_Quaternion.ConvertToMatrix(mat[1]);
		mat[2] = mat[0] * mat[1];
		pKey->m_Quaternion.ConvertFromMatrix(mat[2]);
	}
}


void model_SetAnimFramerate(Model *pModel, unsigned long iAnim, float framerate, std::vector<bool>* taggedKeys)
{
	unsigned long i, curMarkerTime, deltaTime, curTagNum;
	ModelAnim *pAnim;
	AnimKeyFrame *pKeyFrame;


	deltaTime = (unsigned long)(framerate * 1000.0f);
	if(deltaTime == 0)
		return;

	pAnim = pModel->GetAnimCheck(iAnim);
	if(!pAnim)
		return;

	if(pAnim->m_KeyFrames > 0)
	{
		curTagNum = 1;
		curMarkerTime = pAnim->m_KeyFrames[0].m_Time;

		// Remove all keyframes until we encounter one with delta time > deltaTime.
		for(i=1; i < (pAnim->m_KeyFrames-1); i++, curTagNum++)
		{
			pKeyFrame = &pAnim->m_KeyFrames[i];

			if(pKeyFrame->m_Time < (curMarkerTime + deltaTime))
			{
				bool hasKeyString = (pKeyFrame->m_pString[0] != '\0');
				bool isTagged = (taggedKeys && (*taggedKeys)[curTagNum]);

				// Get rid of this keyframe if it doesn't have a keystring and isn't tagged
				if( !(hasKeyString || isTagged) )
				{
					pAnim->RemoveKeyFrame(i);
					--i;
				}
			}
			else
			{
				curMarkerTime = pKeyFrame->m_Time;
			}
		}
	}
}


// {BP 1/20/98}
// Moved this code to this function because it was used in a couple places...
void model_GetDimsFromBounding( LTVector *pvBoundMax, LTVector *pvBoundMin, LTVector *pvDims )
{
	LTVector vDimsMax;

	// Set the model dims from the first frame of the first animation bounding box...
	// Not sure if BoundMin is always negative and BoundMax is always positive,
	// so got to do some absolute values...
	pvDims->x = ( pvBoundMin->x > 0.0f ) ? pvBoundMin->x : -pvBoundMin->x;
	pvDims->y = ( pvBoundMin->y > 0.0f ) ? pvBoundMin->y : -pvBoundMin->y;
	pvDims->z = ( pvBoundMin->z > 0.0f ) ? pvBoundMin->z : -pvBoundMin->z;
	vDimsMax.x = ( pvBoundMax->x > 0.0f ) ? pvBoundMax->x : -pvBoundMax->x;
	vDimsMax.y = ( pvBoundMax->y > 0.0f ) ? pvBoundMax->y : -pvBoundMax->y;
	vDimsMax.z = ( pvBoundMax->z > 0.0f ) ? pvBoundMax->z : -pvBoundMax->z;
	if( pvDims->x < vDimsMax.x )
		pvDims->x = vDimsMax.x;
	if( pvDims->y < vDimsMax.y )
		pvDims->y = vDimsMax.y;
	if( pvDims->z < vDimsMax.z )
		pvDims->z = vDimsMax.z;
}

///////////////////////////////////////////////////////////////////////////////
/// O B B  G E N 
///////////////////////////////////////////////////////////////////////////////

// ------------------------------------------------------------------------
// ExtentUtil
// utility to help generate obb pos and size from collection of points.
// ------------------------------------------------------------------------
struct ExtentUtil
{ 
	ExtentUtil(LTVector m, LTVector mx) :m_min(m),m_max(mx) {} 
	ExtentUtil():m_min(FLT_MAX,FLT_MAX,FLT_MAX),m_max(-FLT_MAX,-FLT_MAX,-FLT_MAX) {}

	void test_set( float *fval ) {
			LTVector val(fval[0],fval[1],fval[2]);
			
			if( m_min.x > val.x )	m_min.x = val.x ;
			if( m_min.y > val.y )	m_min.y = val.y ;
			if( m_min.z > val.z )	m_min.z = val.z ;
			if( m_max.x < val.x )	m_max.x = val.x ;
			if( m_max.y < val.y )	m_max.y = val.y ;
			if( m_max.z < val.z )	m_max.z = val.z ;
	}

	SOBB to_obb(  )
	{
		SOBB obb;
		obb.m_Pos = ((m_max  - m_min) / 2.0f) + m_min ;
		obb.m_Size = ( m_max - m_min ) ;
		return obb;
	}

	LTVector m_min, m_max ;
	
	// float link_length ; // average link length
};

// ---------------------------------------------------------------------------
// GetOBBDimsFromGeom
// ---------------------------------------------------------------------------
static void GenerateOBBFromGeom(Model *pModel, 	const std::vector<bool> &selected_node,
						float min_weight,
						float min_linkdist)
 				
				
{
	uint32 i, j, k;
	ModelPiece *pPiece;
	ModelVert *pVert;
	NewVertexWeight *pWeight;
	PieceLOD*pLOD;
	float	cur_lod_dist = 0;
	uint32	num_nodes = pModel->NumNodes();
	
	std::vector< ExtentUtil > obbext(num_nodes) ;
	std::vector< bool >		  enable_obb(num_nodes, false) ;

	// find the extents of the obb from the node associated geometry
	for(i=0; i < pModel->NumPieces(); i++)
	{
		pPiece = pModel->GetPiece(i);
		pLOD   = pPiece->GetLOD(cur_lod_dist);
  
		for(j=0; j < pLOD->m_Verts; j++)
		{
			pVert = &pLOD->m_Verts[j];

			//  this isn't quite accurate but close enough.         
			if(pVert->m_nWeights > 0 )
			{
				for(k=0; k < pVert->m_nWeights; k++)
				{
					pWeight = &pVert->m_Weights[k];
					
					if( pWeight->m_Vec[3] > min_weight ){
					
						obbext[pWeight->m_iNode].test_set( pWeight->m_Vec);
						enable_obb[pWeight->m_iNode] = true ;
					}
				}
			}
		}// for every vertex
	}

	// add valid obbs to model's node.
	for( i = 0 ; i < pModel->m_FlatNodeList ; i++ )
	{
		if( enable_obb[i] )
		{
			pModel->m_FlatNodeList[i]->SetOBB( obbext[i].to_obb());
			pModel->m_FlatNodeList[i]->EnableOBB();
		}
	}
}


// ---------------------------------------------------------------------------
// model_CreateDefaultOBB( model, selected nodes, min_weight, min-link-length)
// 
// create an obb for every node using the geometry for that node.
// ---------------------------------------------------------------------------
void model_CreateDefaultOBB( Model *pModel, 
							const std::vector<bool> &selected_nodes,
							float min_weight,
							float min_link_length							)
{
	ASSERT(pModel);
	GenerateOBBFromGeom(pModel, selected_nodes, min_weight, min_link_length);
}


// ------------------------------------------------------------------------
// model_CreateDefaultOBB( model )
// 
// given a model, create obbs based on the default hierarchy of the model.
// ------------------------------------------------------------------------
void model_CreateDefaultOBB( Model *pModel )
{
	ASSERT(pModel);

	std::vector<bool> selected_nodes(pModel->NumNodes(), true);
	float min_link_length = 1000.0f ;
	float min_weight      = 0.0f ;

	model_CreateDefaultOBB(pModel,selected_nodes,min_weight,min_link_length);

}

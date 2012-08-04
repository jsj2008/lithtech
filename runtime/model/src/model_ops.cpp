
#if !defined(__LINUX) && !defined(__XBOX)
#include <windows.h>
#endif
#include "bdefs.h"
#include "model.h"
#include "model_ops.h"
#include "conparse.h"
#include "geomroutines.h"


static int g_iCreateTransformKeys[2];
static float g_CreateTransformPercent;
static LTBOOL g_bRecurseFlipZ;
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

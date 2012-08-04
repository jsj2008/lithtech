#include "precompile.h"

#include "3d_ops.h"
#include "renderstruct.h"
#include "common_draw.h"
#include "common_stuff.h"
#include "FixedPoint.h"
#include "de_mainworld.h"
#include "rendererframestats.h"
#include "rendertargetmgr.h"
#include "rendertarget.h"

//------------------------------------------------------------------
//------------------------------------------------------------------
// Holders and their headers.
//------------------------------------------------------------------
//------------------------------------------------------------------

//IWorldClientBSP holder
#include "world_client_bsp.h"
static IWorldClientBSP *world_bsp_client;
define_holder(IWorldClientBSP, world_bsp_client);

//IWorldSharedBSP
#include "world_shared_bsp.h"
static IWorldSharedBSP *world_bsp_shared;
define_holder(IWorldSharedBSP, world_bsp_shared);

//True if we have a world to use this frame.
bool g_have_world;

SceneDesc *g_pSceneDesc;
uint16 g_CurFrameCode;
uint32 g_CurObjectFrameCode;

// The list of lights that will be used to light the world
DynamicLight *g_WorldDynamicLights[MAX_VISIBLE_LIGHTS];
uint32 g_nNumWorldDynamicLights;

// The list of dynamic lights that will be used to light objects
DynamicLight *g_ObjectDynamicLights[MAX_VISIBLE_LIGHTS];
uint32 g_nNumObjectDynamicLights;

ViewParams g_ViewParams;

// Increments the frame code and resets all the world structures' frame
// codes if it wraps around.
void d3d_IncrementFrameCode(RenderContext *pContext)
{
	// Increment the point code.
	if(pContext->m_CurFrameCode == 0xFFFF)
	{
		pContext->m_CurFrameCode = 1;
	}

	++pContext->m_CurFrameCode;
}

void d3d_SetupPerspectiveMatrix(LTMatrix *pMatrix, float nearZ, float farZ)
{
	pMatrix->Identity();
	pMatrix->m[2][2] = farZ / (farZ - nearZ);
	pMatrix->m[2][3] = (-farZ * nearZ) / (farZ - nearZ);
	pMatrix->m[3][2] = 1.0f;
	pMatrix->m[3][3] = 0.0f;
}


static void d3d_SetupSkyStuff(const SkyDef& Def, ViewParams* pParams)
{
	LTVector percents;
	
	if(g_pSceneDesc->m_DrawMode == DRAWMODE_NORMAL)
	{
        const LTVector &min = world_bsp_shared->ExtentsMin();
        const LTVector &max = world_bsp_shared->ExtentsMax();
		percents.x = (pParams->m_Pos.x - min.x) / (max.x - min.x);
		percents.y = (pParams->m_Pos.y - min.y) / (max.y - min.y);
		percents.z = (pParams->m_Pos.z - min.z) / (max.z - min.z);
	}
	else
	{
		percents.Init(0.5f, 0.5f, 0.5f);
	}

	LTVector v = Def.m_ViewMax - Def.m_ViewMin;

	v.x *= percents.x;
	v.y *= percents.y;
	v.z *= percents.z;

	pParams->m_SkyViewPos = Def.m_ViewMin + v;
}


void d3d_InitViewBox(ViewBoxDef *pDef,
	float nearZ, float farZ, float xFov, float yFov)
{
	float xTemp, yTemp;
	
	pDef->m_NearZ = nearZ;
	pDef->m_FarZ = farZ;

	xTemp = MATH_HALFPI - (xFov * 0.5f);
	yTemp = MATH_HALFPI - (yFov * 0.5f);

	pDef->m_WindowSize[0] = (float)(cos(xTemp) / sin(xTemp));
	pDef->m_WindowSize[1] = (float)(cos(yTemp) / sin(yTemp));

	pDef->m_COP.Init(0.0f, 0.0f, 1.0f);
}


void d3d_InitViewBox2(ViewBoxDef *pDef, 
	float nearZ, float farZ,
	const ViewParams&  PrevParams,
	float screenMinX, float screenMinY, 
	float screenMaxX, float screenMaxY)
{
	// Get the extents of the previous view frustum's view window.
	float xMinPrev = PrevParams.m_ViewBox.m_COP.x - PrevParams.m_ViewBox.m_WindowSize[0];
	float yMinPrev = -PrevParams.m_ViewBox.m_COP.y - PrevParams.m_ViewBox.m_WindowSize[1];
	float xMaxPrev = PrevParams.m_ViewBox.m_COP.x + PrevParams.m_ViewBox.m_WindowSize[0];
	float yMaxPrev = -PrevParams.m_ViewBox.m_COP.y + PrevParams.m_ViewBox.m_WindowSize[1];
	
	// Get the percents.
	float xMin = (screenMinX - (float)PrevParams.m_Rect.left) / (float)(PrevParams.m_Rect.right - PrevParams.m_Rect.left);
	float yMin = (screenMinY - (float)PrevParams.m_Rect.top) / (float)(PrevParams.m_Rect.bottom - PrevParams.m_Rect.top);
	float xMax = (screenMaxX - (float)PrevParams.m_Rect.left) / (float)(PrevParams.m_Rect.right - PrevParams.m_Rect.left);
	float yMax = (screenMaxY - (float)PrevParams.m_Rect.top) / (float)(PrevParams.m_Rect.bottom - PrevParams.m_Rect.top);

	// Note: Y is negated here because we're going from screen to world space.

	// Translate them into our COP space.
	xMin = LTLERP(xMinPrev, xMaxPrev, xMin);
	yMin = -LTLERP(yMinPrev, yMaxPrev, yMin);
	xMax = LTLERP(xMinPrev, xMaxPrev, xMax);
	yMax = -LTLERP(yMinPrev, yMaxPrev, yMax);
	
	pDef->m_NearZ			= nearZ;
	pDef->m_FarZ			= farZ;
	pDef->m_COP.z			= 1.0f;			 
	pDef->m_COP.x			= (xMin + xMax) * 0.5f;
	pDef->m_COP.y			= (yMin + yMax) * 0.5f;
	pDef->m_WindowSize[0]	= (xMax - xMin) * 0.5f;
	pDef->m_WindowSize[1]	= -(yMax - yMin) * 0.5f;
}


bool d3d_InitFrustum(ViewParams *pParams, 
	float xFov, float yFov, float nearZ, float farZ, 
	float screenMinX, float screenMinY, float screenMaxX, float screenMaxY,
	const LTVector *pPos, const LTRotation *pRotation, ViewParams::ERenderMode eMode)
{
	LTMatrix mat;

	quat_ConvertToMatrix((float*)pRotation, mat.m);
	mat.SetTranslation(*pPos);

	ViewBoxDef viewBox;
	d3d_InitViewBox(&viewBox, nearZ, farZ, xFov, yFov);

	// Setup initial states and stuff.
	return d3d_InitFrustum2(pParams, &viewBox, 
		screenMinX, screenMinY, screenMaxX, screenMaxY, &mat, LTVector(1.0f, 1.0f, 1.0f), eMode);
}

// Sets up pParams.
// pPos and pRotation are the view position and orientation.
// pRect is the rectangle on the screen that the viewing window maps into.
// pScale is an extra scale that scales all the coordinates up.
bool d3d_InitFrustum2(ViewParams *pParams, 
	ViewBoxDef *pViewBox, 
	float screenMinX, float screenMinY, float screenMaxX, float screenMaxY,
	const LTMatrix *pMat, const LTVector& vScale, ViewParams::ERenderMode eMode)
{
	LTMatrix mTempWorld, mRotation, mScale;
	LTMatrix mFOVScale, mBackTransform;
	LTMatrix mDevice, mBackTranslate;
	float leftX, rightX, topY, bottomY, normalZ;
	uint32 i;
	LTVector forwardVec, zPlanePos, vTrans;
	LTMatrix mProjectionTransform; //mUnit, mPerspective;
	
	pParams->m_mIdentity.Identity();
	pParams->m_mInvView = *pMat;


	// Here's how the viewing works:
	// The world to camera transformation rotates and translates
	// the world into camera space.
	// The camera to clip transformation just scales the sides so the
	// field of view is 90 degrees (faster to clip in).
	// Clipping takes place on NEARZ and g_ViewParams.m_FarZ.
	// In terms of Z buffer calculations, the maximum Z is MAX_FARZ (that way,
	// when the farZ clipping plane is changed, sz and rhw stay the same).



	/////// Copy stuff in and setup view limits.

	memcpy(&pParams->m_ViewBox, pViewBox, sizeof(pParams->m_ViewBox));
	pMat->GetTranslation(pParams->m_Pos);

	pParams->m_FarZ = pViewBox->m_FarZ;
	if(pParams->m_FarZ < 3.0f) pParams->m_FarZ = 3.0f;
	if(pParams->m_FarZ > MAX_FARZ) pParams->m_FarZ = MAX_FARZ;
	pParams->m_NearZ = pViewBox->m_NearZ;
	
	pParams->m_Rect.left = (int)RoundFloatToInt(screenMinX);
	pParams->m_Rect.top = (int)RoundFloatToInt(screenMinY);
	pParams->m_Rect.right = (int)RoundFloatToInt(screenMaxX);
	pParams->m_Rect.bottom = (int)RoundFloatToInt(screenMaxY);

	/////// Setup all the matrices.

	// Setup the rotation and translation transforms.
	mRotation = *pMat;
	mRotation.SetTranslation(0.0f, 0.0f, 0.0f);
	Mat_GetBasisVectors(&mRotation, &pParams->m_Right, &pParams->m_Up, &pParams->m_Forward);

	// We want to transpose (ie: when we're looking left, rotate the world to the right..)
	MatTranspose3x3(&mRotation);
	mBackTranslate.Init(
		1, 0, 0, -pParams->m_Pos.x,
		0, 1, 0, -pParams->m_Pos.y,
		0, 0, 1, -pParams->m_Pos.z,
		0, 0, 0, 1);
	MatMul(&mTempWorld, &mRotation, &mBackTranslate);
	
	// Scale it to get the full world transform.
	mScale.Init(
		vScale.x, 0, 0, 0,
		0, vScale.y, 0, 0,
		0, 0, vScale.z, 0,
		0, 0, 0, 1);
	MatMul(&pParams->m_mView, &mScale, &mTempWorld);

	// Shear so the center of projection is (0,0,COP.z)
	LTMatrix mShear;
	mShear.Init(
		1.0f, 0.0f, -pViewBox->m_COP.x/pViewBox->m_COP.z, 0.0f,
		0.0f, 1.0f, -pViewBox->m_COP.y/pViewBox->m_COP.z, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f);

	// Figure out X and Y scale to get frustum into unit slopes.
	float fFovXScale = pViewBox->m_COP.z / pViewBox->m_WindowSize[0];
	float fFovYScale = pViewBox->m_COP.z / pViewBox->m_WindowSize[1];

	// Squash the sides to 45 degree angles.
	mFOVScale.Init(
		fFovXScale, 0.0f, 0.0f, 0.0f,
		0.0f, fFovYScale, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f);

	// Setup the projection transform.
	d3d_SetupPerspectiveMatrix(&mProjectionTransform, pViewBox->m_NearZ, pParams->m_FarZ);

	// Setup the projection space (-1<x<1) to device space transformation.
	pParams->m_fScreenWidth = (screenMaxX - screenMinX);
	pParams->m_fScreenHeight = (screenMaxY - screenMinY);
	
	// Setup the device transform.  It subtracts 0.4 to account for the FP's tendency
	// to slip above and below 0.5.
	Mat_Identity(&mDevice);
	mDevice.m[0][0] = pParams->m_fScreenWidth * 0.5f - 0.0001f;
	mDevice.m[0][3] = screenMinX + pParams->m_fScreenWidth * 0.5f;
	mDevice.m[1][1] = -(pParams->m_fScreenHeight * 0.5f - 0.0001f);
	mDevice.m[1][3] = screenMinY + pParams->m_fScreenHeight * 0.5f;


	// Precalculate useful matrices.
	pParams->m_DeviceTimesProjection = mDevice * mProjectionTransform;
	pParams->m_FullTransform = pParams->m_DeviceTimesProjection * mFOVScale * mShear * pParams->m_mView;

	pParams->m_mProjection = mProjectionTransform * mFOVScale * mShear;
	
	/////// Setup the view frustum points in camera space.
	
	float xNearZ, yNearZ, xFarZ, yFarZ;
	xNearZ = (pParams->m_NearZ * pViewBox->m_WindowSize[0]) / pViewBox->m_COP.z;
	yNearZ = (pParams->m_NearZ * pViewBox->m_WindowSize[1]) / pViewBox->m_COP.z;
	xFarZ = (pParams->m_FarZ * pViewBox->m_WindowSize[0]) / pViewBox->m_COP.z;
	yFarZ = (pParams->m_FarZ * pViewBox->m_WindowSize[1]) / pViewBox->m_COP.z;

	pParams->m_ViewPoints[0].Init(-xNearZ, yNearZ,  pParams->m_NearZ);	// Near Top Left
	pParams->m_ViewPoints[1].Init(xNearZ,  yNearZ,  pParams->m_NearZ);	// Near Top Right
	pParams->m_ViewPoints[2].Init(-xNearZ, -yNearZ, pParams->m_NearZ);	// Near Bottom Left
	pParams->m_ViewPoints[3].Init(xNearZ,  -yNearZ, pParams->m_NearZ);	// Near Bottom Right

	pParams->m_ViewPoints[4].Init(-xFarZ, yFarZ,  pParams->m_FarZ);	// Far Top Left
	pParams->m_ViewPoints[5].Init(xFarZ,  yFarZ,  pParams->m_FarZ);	// Far Top Right
	pParams->m_ViewPoints[6].Init(-xFarZ, -yFarZ, pParams->m_FarZ);	// Far Bottom Left
	pParams->m_ViewPoints[7].Init(xFarZ,  -yFarZ, pParams->m_FarZ);	// Far Bottom Right

	// Transform them into world space.
	for(i=0; i < 8; i++)
	{
		MatVMul_InPlace_Transposed3x3(&pParams->m_mView, &pParams->m_ViewPoints[i]);
		pParams->m_ViewPoints[i] += pParams->m_Pos;
	}

	// Get the AABB of the view frustum
	pParams->m_ViewAABBMin = pParams->m_ViewPoints[0];
	pParams->m_ViewAABBMax = pParams->m_ViewPoints[0];
	for(i=1; i < 8; i++)
	{
		VEC_MIN(pParams->m_ViewAABBMin, pParams->m_ViewAABBMin, pParams->m_ViewPoints[i]);
		VEC_MAX(pParams->m_ViewAABBMax, pParams->m_ViewAABBMax, pParams->m_ViewPoints[i]);
	}

	/////// Setup the camera-space clipping planes.

	leftX = pViewBox->m_COP.x - pViewBox->m_WindowSize[0];
	rightX = pViewBox->m_COP.x + pViewBox->m_WindowSize[0];
	topY = pViewBox->m_COP.y + pViewBox->m_WindowSize[1];
	bottomY = pViewBox->m_COP.y - pViewBox->m_WindowSize[1];
	normalZ = pViewBox->m_COP.z;
	
	LTPlane CSClipPlanes[NUM_CLIPPLANES];
	CSClipPlanes[CPLANE_NEAR_INDEX].m_Normal.Init(0.0f, 0.0f, 1.0f);		// Near Z
	CSClipPlanes[CPLANE_NEAR_INDEX].m_Dist = 1.0f;

	CSClipPlanes[CPLANE_FAR_INDEX].m_Normal.Init(0.0f, 0.0f, -1.0f);			// Far Z
	CSClipPlanes[CPLANE_FAR_INDEX].m_Dist = -pParams->m_FarZ;
	
	CSClipPlanes[CPLANE_LEFT_INDEX].m_Normal.Init(normalZ,  0.0f, -leftX);		// Left
	CSClipPlanes[CPLANE_RIGHT_INDEX].m_Normal.Init(-normalZ, 0.0f, rightX);		// Right
	CSClipPlanes[CPLANE_TOP_INDEX].m_Normal.Init(0.0f,  -normalZ, topY);		// Top
	CSClipPlanes[CPLANE_BOTTOM_INDEX].m_Normal.Init(0.0f,  normalZ, -bottomY);	// Bottom

	CSClipPlanes[CPLANE_LEFT_INDEX].m_Normal.Norm();
	CSClipPlanes[CPLANE_TOP_INDEX].m_Normal.Norm();
	CSClipPlanes[CPLANE_RIGHT_INDEX].m_Normal.Norm();
	CSClipPlanes[CPLANE_BOTTOM_INDEX].m_Normal.Norm();

	CSClipPlanes[CPLANE_LEFT_INDEX].m_Dist = CSClipPlanes[CPLANE_RIGHT_INDEX].m_Dist = 0.0f;
	CSClipPlanes[CPLANE_TOP_INDEX].m_Dist = CSClipPlanes[CPLANE_BOTTOM_INDEX].m_Dist = 0.0f;

	// Now setup the world space clipping planes.
	mBackTransform = pParams->m_mView;
	MatTranspose3x3(&mBackTransform);
	for(i=0; i < NUM_CLIPPLANES; i++)
	{
		if(i != CPLANE_NEAR_INDEX && i != CPLANE_FAR_INDEX)
		{
			MatVMul_3x3(&pParams->m_ClipPlanes[i].m_Normal, &mBackTransform, &CSClipPlanes[i].m_Normal);
			pParams->m_ClipPlanes[i].m_Dist = pParams->m_ClipPlanes[i].m_Normal.Dot(pParams->m_Pos);
		}
	}

	// The Z planes need to be handled a little differently.
	forwardVec.Init(mRotation.m[2][0], mRotation.m[2][1], mRotation.m[2][2]);

	zPlanePos = forwardVec * pViewBox->m_NearZ;
	zPlanePos += pParams->m_Pos;

	MatVMul_3x3(&pParams->m_ClipPlanes[CPLANE_NEAR_INDEX].m_Normal, 
		&mBackTransform, &CSClipPlanes[CPLANE_NEAR_INDEX].m_Normal);

	pParams->m_ClipPlanes[CPLANE_NEAR_INDEX].m_Dist = 
		pParams->m_ClipPlanes[CPLANE_NEAR_INDEX].m_Normal.Dot(zPlanePos);

	zPlanePos = forwardVec * pParams->m_FarZ;
	zPlanePos += pParams->m_Pos;
	
	MatVMul_3x3(&pParams->m_ClipPlanes[CPLANE_FAR_INDEX].m_Normal,
		&mBackTransform, &CSClipPlanes[CPLANE_FAR_INDEX].m_Normal);

	pParams->m_ClipPlanes[CPLANE_FAR_INDEX].m_Dist = 
		pParams->m_ClipPlanes[CPLANE_FAR_INDEX].m_Normal.Dot(zPlanePos);

	// Remember AABB Corners the planes are pointing at
	for (uint32 nPlaneLoop = 0; nPlaneLoop < NUM_CLIPPLANES; ++nPlaneLoop)
	{
		pParams->m_AABBPlaneCorner[nPlaneLoop] = 
			GetAABBPlaneCorner(pParams->m_ClipPlanes[nPlaneLoop].m_Normal);
	}

	// Default the world transform to identity
	pParams->m_mInvWorld.Identity();

	//setup the environment mapping info
	pParams->m_mWorldEnvMap.SetBasisVectors(&pParams->m_Right, &pParams->m_Up, &pParams->m_Forward);

	//turn off glowing by default
	pParams->m_eRenderMode = eMode;

	d3d_SetupSkyStuff(g_pSceneDesc->m_SkyDef, pParams);
	return true;
}


bool d3d_InitFrame(ViewParams* pParams, SceneDesc *pDesc)
{
	RenderContext *pContext;


	// Lower the floating point precision to speed up multiplies and divides.
	d3d_SetFPState();

	// Possibly read in the new options.
	d3d_ReadConsoleVariables();

	g_pSceneDesc = pDesc;

	// Get stuff out of the context (if it exists).
	pContext = (RenderContext*)pDesc->m_hRenderContext;
	if(pDesc->m_DrawMode == DRAWMODE_NORMAL)
	{
		if(!pContext)
			return 0;

		g_have_world = true;

		d3d_IncrementFrameCode(pContext);
		g_CurFrameCode = pContext->m_CurFrameCode;
	}
	else
	{
        g_have_world = false;
	}					  

	//clear out all the lights we have
	g_nNumObjectDynamicLights = 0;
	g_nNumWorldDynamicLights = 0;

	g_CurObjectFrameCode = g_pStruct->IncObjectFrameCode();

	//we need to save the draw prim texture memory frame state since the sets come before this frame
	uint32	nFS_DrawPrimTexMemory = FrameStat(eFS_DrawPrimTexMemory);

	//initialize the frame statistics for this new frame
	InitFrameStats();

	//now restore it
	SetFrameStat(eFS_DrawPrimTexMemory, nFS_DrawPrimTexMemory);

	g_Device.SetDefaultRenderStates();

	HRENDERTARGET hRenderTarget = CRenderTargetMgr::GetSingleton().GetCurrentRenderTargetHandle();
	if(hRenderTarget != INVALID_RENDER_TARGET)
	{
		CRenderTarget *pRenderTarget = CRenderTargetMgr::GetSingleton().GetRenderTarget(hRenderTarget);

		if(pRenderTarget)
		{
			uint32 nWidth = 0;
			uint32 nHeight = 0;
			RenderTargetParams params = pRenderTarget->GetRenderTargetParams();
		
			int x1 = 0;
			int x2 = params.Width;
			int y1 = 0;
			int y2 = params.Height;

			// Setup the viewport...
			g_Device.SetupViewport(x1, x2, y1, y2, 0.1f, 1.0f);

			// Note: since the numbers are truncated when converted to integers, it takes a little
			// off the right and bottom to make sure it never exceeds those.
			return d3d_InitFrustum(pParams, 
				pDesc->m_xFov, pDesc->m_yFov, 
				NEARZ, g_CV_FarZ, 
				(float)x1, (float)y1, 
				(float)x2 - 0.1f, (float)y2 - 0.1f, 
				&pDesc->m_Pos, &pDesc->m_Rotation, ViewParams::eRenderMode_Normal);
		}
	}

	// Setup the viewport...
	g_Device.SetupViewport(pDesc->m_Rect.left,pDesc->m_Rect.right,pDesc->m_Rect.top,pDesc->m_Rect.bottom,0.1f,1.0f);

	// Note: since the numbers are truncated when converted to integers, it takes a little
	// off the right and bottom to make sure it never exceeds those.
	return d3d_InitFrustum(pParams, 
		pDesc->m_xFov, pDesc->m_yFov, 
		NEARZ, g_CV_FarZ, 
		(float)pDesc->m_Rect.left, (float)pDesc->m_Rect.top, 
		(float)pDesc->m_Rect.right - 0.1f, (float)pDesc->m_Rect.bottom - 0.1f, 
		&pDesc->m_Pos, &pDesc->m_Rotation, ViewParams::eRenderMode_Normal);


}


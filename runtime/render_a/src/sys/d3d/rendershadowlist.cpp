#include "precompile.h"
#include "rendershadowlist.h"
#include "relevantlightlist.h"
#include "d3d_renderstatemgr.h"
#include "setupmodel.h"
#include "d3dmeshrendobj_skel.h"
#include "d3dmeshrendobj_rigid.h"
#include "d3dmeshrendobj_vertanim.h"
#include "modelshadowshader.h"
#include "rendererframestats.h"
#include "..\shadows\d3dshadowtexture.h"
#include "ltpixelshadermgr.h"
#include <algorithm>

//------------------------------------------------------------------
// Constants and defines
//------------------------------------------------------------------

#define MAX_SHADOW_TEXTURES 8

//------------------------------------------------------------------
// Holders and their headers.
//------------------------------------------------------------------

//ILTCommon client instance.
#include "iltcommon.h"
static ILTCommon *ilt_common_client;
define_holder_to_instance(ILTCommon, ilt_common_client, Client);

//Interface for the client file manager
#include "client_filemgr.h"
static IClientFileMgr* g_pIClientFileMgr;
define_holder(IClientFileMgr, g_pIClientFileMgr);



//----------------------------------------------------------------------------
// BlurTexture Vertex format
//   Uses: No HW TnL, 4 texture channels

//the pixel offsets we use to offset the source textures
#define PIXEL_OFFSET			1.0f

#define BLURTEXTUREVERTEX_FORMAT (D3DFVF_XYZRHW | D3DFVF_TEX4)

class CBlurTextureVertex
{
public:

	void Init(float fXPosScale, float fYPosScale, float fWidth, float fHeight)
	{
		m_RHW = 1.0f;

		//setup the position
		m_Vec.x = fWidth * fXPosScale;
		m_Vec.y = fHeight * fYPosScale;
		m_Vec.z = 0.0f;

		float fXOffset = g_CV_ModelShadow_Proj_BlurPixelSpacing.m_Val / fWidth;
		float fYOffset = g_CV_ModelShadow_Proj_BlurPixelSpacing.m_Val / fHeight;

		//now handle the UV sets
		m_UVs[0].m_fU = fXPosScale - fXOffset;
		m_UVs[0].m_fV = fYPosScale - fYOffset;
		m_UVs[1].m_fU = fXPosScale + fXOffset;
		m_UVs[1].m_fV = fYPosScale - fYOffset;
		m_UVs[2].m_fU = fXPosScale + fXOffset;
		m_UVs[2].m_fV = fYPosScale + fYOffset;
		m_UVs[3].m_fU = fXPosScale - fXOffset;
		m_UVs[3].m_fV = fYPosScale + fYOffset;
	}

	struct SUVSet
	{
		float	m_fU, m_fV;
	};

	//position
	LTVector	m_Vec;
	float		m_RHW;

	SUVSet		m_UVs[4];
};

//----------------------------------------------------------------------------
// BlurTexture2Pass Vertex format
//   Uses: No HW TnL, 2 texture channels

#define BLURTEXTURE2PASSVERTEX_FORMAT (D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_TEX2)

class CBlurTexture2PassVertex
{
public:

	void Init(float fXPosScale, float fYPosScale, float fWidth, float fHeight, float fSign)
	{
		m_RHW = 1.0f;

		//setup the position
		m_Vec.x = fWidth * fXPosScale;
		m_Vec.y = fHeight * fYPosScale;
		m_Vec.z = 0.0f;

		float fXOffset = g_CV_ModelShadow_Proj_BlurPixelSpacing.m_Val / fWidth;
		float fYOffset = g_CV_ModelShadow_Proj_BlurPixelSpacing.m_Val / fHeight;

		//now handle the UV sets
		m_UVs[0].m_fU = fXPosScale - fXOffset * fSign;
		m_UVs[0].m_fV = fYPosScale - fYOffset * fSign;
		m_UVs[1].m_fU = fXPosScale + fXOffset * fSign;
		m_UVs[1].m_fV = fYPosScale - fYOffset * fSign;

		//the diffuse needs to have 25% put into the color channels and the alpha
		m_nDiffuse = D3DRGBA(0.25f, 0.25f, 0.25f, 0.25f);
	}

	struct SUVSet
	{
		float	m_fU, m_fV;
	};

	//position
	LTVector	m_Vec;
	float		m_RHW;

	uint32		m_nDiffuse;

	SUVSet		m_UVs[2];
};


//------------------------------------------------------------------------------------------
// Helper functions
//------------------------------------------------------------------------------------------

//this function will draw a line from each light in the list to the specified position
static void DrawLinesFromLights(const CRelevantLightList& LightList, const LTVector& vToPos)
{
	for (uint32 nCurrLight = 0; nCurrLight < LightList.GetNumLights(); ++nCurrLight)
	{
		LTVector vLightPos = LightList.GetLight(nCurrLight).GetPos();

		BASIC_VERTEX vVert[2];
		vVert[0].x = vLightPos.x;
		vVert[0].y = vLightPos.y;
		vVert[0].z = vLightPos.z;
		vVert[0].color = 0x000000FF;

		vVert[1].x = vToPos.x;
		vVert[1].y = vToPos.y;
		vVert[1].z = vToPos.z;
		vVert[1].color = 0x00FF0000;

		D3D_CALL(PD3DDEVICE->SetVertexShader(NULL));
		D3D_CALL(PD3DDEVICE->SetFVF(BASIC_VERTEX_FLAGS));
		D3D_CALL(PD3DDEVICE->DrawPrimitiveUP(D3DPT_LINELIST, 1, vVert, sizeof(BASIC_VERTEX)));
	}
}

//this renders the plane of the specified light for debugging purposes
static void DrawLightPlanes(const LTVector& modelPos, const LTVector& lightPos, const LTVector& lightUp, const LTVector& lightRight, float fSize)
{
	LTVector vProjPlane_TopLeft		= modelPos - lightUp * fSize * 0.5f - lightRight * fSize * 0.5f;
	LTVector vProjPlane_TopRight	= modelPos + lightUp * fSize * 0.5f - lightRight * fSize * 0.5f;
	LTVector vProjPlane_BottomLeft	= modelPos - lightUp * fSize * 0.5f + lightRight * fSize * 0.5f;
	LTVector vProjPlane_BottomRight	= modelPos + lightUp * fSize * 0.5f + lightRight * fSize * 0.5f;

	BASIC_VERTEX vVert[16];
	vVert[0].x = vProjPlane_TopLeft.x; vVert[0].y = vProjPlane_TopLeft.y; vVert[0].z = vProjPlane_TopLeft.z; vVert[0].color = 0x0000FFFF;
	vVert[1].x = vProjPlane_TopRight.x; vVert[1].y = vProjPlane_TopRight.y; vVert[1].z = vProjPlane_TopRight.z; vVert[1].color = 0x0000FF00;
	vVert[2].x = vProjPlane_TopRight.x; vVert[2].y = vProjPlane_TopRight.y; vVert[2].z = vProjPlane_TopRight.z; vVert[2].color = 0x0000FF00;
	vVert[3].x = vProjPlane_BottomRight.x; vVert[3].y = vProjPlane_BottomRight.y; vVert[3].z = vProjPlane_BottomRight.z; vVert[3].color = 0x000000FF;
	vVert[4].x = vProjPlane_BottomRight.x; vVert[4].y = vProjPlane_BottomRight.y; vVert[4].z = vProjPlane_BottomRight.z; vVert[4].color = 0x000000FF;
	vVert[5].x = vProjPlane_BottomLeft.x; vVert[5].y = vProjPlane_BottomLeft.y; vVert[5].z = vProjPlane_BottomLeft.z; vVert[5].color = 0x00FFFF00;
	vVert[6].x = vProjPlane_BottomLeft.x; vVert[6].y = vProjPlane_BottomLeft.y; vVert[6].z = vProjPlane_BottomLeft.z; vVert[6].color = 0x00FFFF00;
	vVert[7].x = vProjPlane_TopLeft.x; vVert[7].y = vProjPlane_TopLeft.y; vVert[7].z = vProjPlane_TopLeft.z; vVert[7].color = 0x0000FFFF;
	vVert[8].x = lightPos.x; vVert[8].y = lightPos.y; vVert[8].z = lightPos.z; vVert[8].color = 0x000000FF;
	vVert[9].x = vProjPlane_TopLeft.x; vVert[9].y = vProjPlane_TopLeft.y; vVert[9].z = vProjPlane_TopLeft.z; vVert[9].color = 0x0000FFFF;
	vVert[10].x = lightPos.x; vVert[10].y = lightPos.y; vVert[10].z = lightPos.z; vVert[10].color = 0x000000FF;
	vVert[11].x = vProjPlane_TopRight.x; vVert[11].y = vProjPlane_TopRight.y; vVert[11].z = vProjPlane_TopRight.z; vVert[11].color = 0x0000FF00;
	vVert[12].x = lightPos.x; vVert[12].y = lightPos.y; vVert[12].z = lightPos.z; vVert[12].color = 0x000000FF;
	vVert[13].x = vProjPlane_BottomRight.x; vVert[13].y = vProjPlane_BottomRight.y; vVert[13].z = vProjPlane_BottomRight.z; vVert[13].color = 0x000000FF;
	vVert[14].x = lightPos.x; vVert[14].y = lightPos.y; vVert[14].z = lightPos.z; vVert[14].color = 0x000000FF;
	vVert[15].x = vProjPlane_BottomLeft.x; vVert[15].y = vProjPlane_BottomLeft.y; vVert[15].z = vProjPlane_BottomLeft.z; vVert[15].color = 0x00FFFF00;

	D3D_CALL(PD3DDEVICE->SetVertexShader(NULL));
	D3D_CALL(PD3DDEVICE->SetFVF(BASIC_VERTEX_FLAGS));
	D3D_CALL(PD3DDEVICE->DrawPrimitiveUP(D3DPT_LINELIST, 8, &vVert[0], sizeof(BASIC_VERTEX)));
}


//given a vector it will find the maximum component and return that value
static float GetMaxVecComponent(const LTVector& vVec)
{
	if((vVec.x > vVec.y) && (vVec.x > vVec.z))
		return vVec.x;
	if(vVec.y > vVec.z)
		return vVec.y;

	return vVec.z;
}

//builds up a frame of reference for the light
static void BuildLightRefFrame(LTVector& lightDir, LTVector& lightUp, LTVector& lightRight)
{
	LTVector u(0.0f, 1.0f, 0.0f);
	float dot = (float )fabs(VEC_DOT(u,lightDir));
	if (fabs(dot) > 0.9999f)
	{
		u.Init(1.0f, 0.0f, 0.0f);
	}

	lightRight = lightDir.Cross(u);
	lightRight.Norm();

	lightUp = lightRight.Cross(lightDir);
	lightUp.Norm();
}


//finds the bounding sphere for the associated model and its attachments
static void FindBoundingInfo(ModelInstance* pModel, const LTVector& vModelPos, const LTVector& vLightPos, LTVector& vFinalDir, LTVector& vFinalPos, float& fFinalRadius)
{
	//try and get the position of the model from the translation node, this fixes cinematics
	//where models go clear outside of their bounding box.
	LTVector vSrcCenter	= vModelPos;

	//get the base bounding sphere from the model
	float fSrcRadius	= pModel->GetDims().Mag();

	//now we need to run through all the attachments and extend the bounds
	Attachment* pCurr = pModel->m_Attachments;

	//now extend our bounds

	LTVector vMin(vSrcCenter.x - fSrcRadius, vSrcCenter.y - fSrcRadius, vSrcCenter.z - fSrcRadius);
	LTVector vMax(vSrcCenter.x + fSrcRadius, vSrcCenter.y + fSrcRadius, vSrcCenter.z + fSrcRadius);

	while(pCurr)
	{
		//get this object
		HOBJECT hParent;
		HOBJECT hChild;
		ilt_common_client->GetAttachmentObjects((HATTACHMENT)pCurr, hParent, hChild);
		LTObject* pCurrObj = (LTObject*)hChild;

		//extend our bounds to include this (if it is a model)
		if(pCurrObj && (pCurrObj->m_ObjectType == OT_MODEL))
		{
			//we have our model
			ModelInstance* pInstance = pCurrObj->ToModel();

			if(pInstance != pModel)
			{
				//update our bounding box extents
				LTVector vModelCenter = pInstance->GetPos();
				float    fModelRadius = pInstance->GetDims().Mag();

				VEC_MIN(vMin, vMin, LTVector(vModelCenter.x - fModelRadius, vModelCenter.y - fModelRadius, vModelCenter.z - fModelRadius));
				VEC_MAX(vMax, vMax, LTVector(vModelCenter.x + fModelRadius, vModelCenter.y + fModelRadius, vModelCenter.z + fModelRadius));
			}
		}

		//move onto the next attachment
		pCurr = pCurr->m_pNext;
	}

	//now we have the point that we want to look at
	LTVector vCenter = (vMin + vMax) * 0.5f;
	LTVector vDir = (vCenter - vLightPos);
	vDir.Normalize();

	//build up the vector basis
	LTVector vUp;
	LTVector vRight;
	BuildLightRefFrame(vDir, vUp, vRight);

	//initialize our bounding sphere
	float fSphereX	= vSrcCenter.Dot(vRight);
	float fSphereY	= vSrcCenter.Dot(vUp);
	float fRadius	= fSrcRadius;


	//ok, now we need to run through all of our attachments again and this time build up the actual bounding sphere
	pCurr = pModel->m_Attachments;
	while(pCurr)
	{
		//get this object
		HOBJECT hParent;
		HOBJECT hChild;
		ilt_common_client->GetAttachmentObjects((HATTACHMENT)pCurr, hParent, hChild);
		LTObject* pCurrObj = (LTObject*)hChild;

		//extend our bounds to include this (if it is a model)
		if(pCurrObj && (pCurrObj->m_ObjectType == OT_MODEL))
		{
			//we have our model
			ModelInstance* pInstance = pCurrObj->ToModel();

			if(pInstance != pModel)
			{
				LTVector vModelCenter = pInstance->GetPos();
				float    fModelRadius = pInstance->GetDims().Mag();

				//project this onto the plane
				float fX = vModelCenter.Dot(vRight);
				float fY = vModelCenter.Dot(vUp);

				float fDist = (float)sqrt((fX - fSphereX) * (fX - fSphereX) + (fY - fSphereY) * (fY - fSphereY));

				//see if position is the same
				if(fDist < 0.01f)
				{
					fRadius = LTMAX(fRadius, fModelRadius);
				}
				else
				{
					//ok, now what we need to do is solve this problem in 1d, which
					//is the line that runs through the center points of the
					//spheres, and is scaled so that the distance between them is
					//1 unit
					float fSphereMin = LTMIN(1.0f - fRadius / fDist, -fModelRadius / fDist);
					float fSphereMax = LTMAX(1.0f + fRadius / fDist, fModelRadius / fDist);

					//now that we know the min and max, we can position the sphere's center
					//and find the radius
					fRadius = (fSphereMax - fSphereMin) * 0.5f;

					float fMidPos = fSphereMin + fRadius;
					fSphereX = (1.0f - fMidPos) * fX + fMidPos * fSphereX;
					fSphereY = (1.0f - fMidPos) * fY + fMidPos * fSphereY;

					//scale radius
					fRadius *= fDist;
				}
			}
		}

		//move onto the next attachment
		pCurr = pCurr->m_pNext;
	}

	//ok, we are done
	vFinalDir		= vDir;
	vFinalPos		= vDir * (vDir.Dot(vCenter)) + fSphereX * vRight + fSphereY * vUp;
	fFinalRadius	= fRadius * g_CV_ModelShadow_Proj_ProjAreaRadiusScale.m_Val;
}

//Renders the model's pieces, assumes everything is already set up
static void RenderModelPieces( ModelInstance* pInstance )
{
	//find the distance to this model
	float fDist = (pInstance->GetPos() - g_ViewParams.m_Pos).Mag();

	int32 iShadowLODOffset			= g_CV_ModelShadow_Proj_LOD.m_Val;

	//we need the shadows to run through all of the pieces and tag which transforms it is using
	//to ensure that they are valid
	uint32 nNumPieces = pInstance->GetModelDB()->NumPieces();

	for (uint32 nCurrPiece = 0; nCurrPiece < nNumPieces; nCurrPiece++)
	{
		ModelPiece* pPiece		= pInstance->GetModelDB()->GetPiece(nCurrPiece);
		CDIModelDrawable* pLOD  = pPiece->GetLODFromDist( iShadowLODOffset, fDist );

		if (!pLOD || (pLOD->GetPolyCount() == 0))
			continue;

		// setup the nodes we need to render this mesh.
		pInstance->SetupLODNodePath(pLOD);
	}

	//now get our rendering transforms
	D3DMATRIX *pD3DTransforms = pInstance->GetRenderingTransforms();

	//get the render style that we will be using on all of the models
	CD3DRenderStyle* pRenderStyle	= g_RenderStateMgr.GetBackupRenderStyle();

	//clear out the texture in the first stage, otherwise it looks very odd...
	d3d_DisableTexture(0);

	// Draw all the Pieces...
	for(uint32 nCurrPass = 0; nCurrPass < pRenderStyle->GetRenderPassCount(); nCurrPass++)
	{
		g_RenderStateMgr.SetRenderStyleStates(pRenderStyle, nCurrPass);

		//make sure that we don't do any Z tests though
		StateSet ssZFunc(D3DRS_ZFUNC, D3DCMP_ALWAYS);

		//and also make sure that we don't write to Z. Yes we need both. One fixes a bug
		//on NVidia, another on ATI
		StateSet ssZRead(D3DRS_ZENABLE, D3DZB_FALSE);
		StateSet ssZWrite(D3DRS_ZWRITEENABLE, FALSE);

		// Disable fog for shadows.
		StateSet ssFogEnable(D3DRS_FOGENABLE, FALSE);

		for (uint32 nCurrPiece = 0; nCurrPiece < nNumPieces; nCurrPiece++)
		{
			ModelPiece* pPiece		= pInstance->GetModelDB()->GetPiece(nCurrPiece);
			CDIModelDrawable* pLOD  = pPiece->GetLODFromDist( iShadowLODOffset, fDist );

			if (!pLOD || (pLOD->GetPolyCount() == 0))
				continue;

			if (!pInstance->IsPieceHidden(nCurrPiece))
			{
				switch (pLOD->GetType())
				{
				case CRenderObject::eSkelMesh	 :
					{
						((CD3DSkelMesh*)pLOD)->BeginRender(pD3DTransforms, pRenderStyle, nCurrPass);
						((CD3DSkelMesh*)pLOD)->Render(pInstance, pD3DTransforms, pRenderStyle, nCurrPass);
						((CD3DSkelMesh*)pLOD)->EndRender();
						IncFrameStat(eFS_ModelRender_NumSkeletalPieces, 1);
					}
					break;
				case CRenderObject::eRigidMesh	 :
					{
						((CD3DRigidMesh*)pLOD)->BeginRender(pD3DTransforms[((CD3DRigidMesh*)pLOD)->GetBoneEffector()], pRenderStyle, nCurrPass);
						((CD3DRigidMesh*)pLOD)->Render(pInstance, pD3DTransforms[((CD3DRigidMesh*)pLOD)->GetBoneEffector()], pRenderStyle, nCurrPass);
						((CD3DRigidMesh*)pLOD)->EndRender();
						IncFrameStat(eFS_ModelRender_NumRigidPieces, 1);
					}
					break;
				case CRenderObject::eVAMesh		 :
					{
						((CD3DVAMesh*)pLOD)->UpdateVA(pInstance->GetModelDB(), &pInstance->m_AnimTracker.m_TimeRef);
						// needs global pos.
						LTMatrix mTrans;
						if(pInstance->GetCachedTransform(((CD3DVAMesh*)pLOD)->GetBoneEffector(), mTrans))
						{
							DDMatrix mD3DTrans;
							Convert_DItoDD(mTrans, mD3DTrans);
							((CD3DVAMesh*)pLOD)->Render(pInstance, mD3DTrans, pRenderStyle, nCurrPass);
							IncFrameStat(eFS_ModelRender_NumVertexAnimatedPieces, 1);
						}
					}
					break;
				}

				//add to our polygon counts
				IncFrameStat(eFS_ModelTriangles, pLOD->GetPolyCount());
			}
		}
	}
}


//This will actually render the model shadow texture assuming that the render target is valid and
//that the parameters have been set up correctly
static void RenderModelShadow( ModelInstance* pInstance, const LTVector& vModelPos, const LTVector& vLightPos, const LTVector& vLightDir, const LTVector& vLightUp,
								const LTVector& vLightColor, uint32 nShadTexSize, float fProjSizeX, float fProjSizeY, bool bPerspective  )
{
	for(uint32 nCurrLight = 0; nCurrLight < 8; nCurrLight++)
		g_RenderStateMgr.LightEnable(nCurrLight, false);

	//clear the current viewport to an opaque white color
	uint32 nClearColor = g_CV_ModelShadow_Proj_TintFill.m_Val ? D3DRGBA_255(0, 255, 0, 0) : D3DRGBA_255(255, 255, 255, 0);
	PD3DDEVICE->Clear(0, NULL, D3DCLEAR_TARGET, nClearColor, 1.0f, 0);

	//now get our old viewport for saving
	D3DVIEWPORT9 OldViewport;
	PD3DDEVICE->GetViewport(&OldViewport);

	//setup our new viewport so that we won't render to the outer edge
	D3DVIEWPORT9 NewViewport;
	NewViewport.X		= 1;
	NewViewport.Y		= 1;
	NewViewport.Width	= nShadTexSize - 2;
	NewViewport.Height	= nShadTexSize - 2;
	NewViewport.MinZ	= 0.0f;			// the Z values are irrelevant since we don't have a depth buffer
	NewViewport.MaxZ	= 1.0f;
	PD3DDEVICE->SetViewport(&NewViewport);

	//	Fetch model draw model parameters
	Model* pModel				= pInstance->GetModelDB();

	float fDistLightToModel = (vLightPos - vModelPos).Mag();

	// Set the view matrix from the point of view of the light source...
	D3DXMATRIX  MyViewMatrix;

	D3DXVECTOR3 vEye(vLightPos.x, vLightPos.y, vLightPos.z);
	D3DXVECTOR3 vLookAt(vLightPos.x + vLightDir.x, vLightPos.y + vLightDir.y, vLightPos.z + vLightDir.z);
	D3DXVECTOR3 vUp(vLightUp.x, vLightUp.y, vLightUp.z);

	D3DXMatrixLookAtLH(&MyViewMatrix,&vEye,&vLookAt,&vUp);
	g_RenderStateMgr.SetTransform(D3DTS_VIEW,&MyViewMatrix);

	//calculate the projection matrix for rendering this model
	D3DXMATRIX  MyProjMatrix;
	float fNearZ			= 1.0f;
	float fFarZ				= 20000.0f;

	if (bPerspective)
	{
		float fDistFrmProjPlane	= fDistLightToModel;
		float wAtNearZ			= fNearZ * (fProjSizeX / fDistFrmProjPlane);
		float hAtNearZ			= fNearZ * (fProjSizeY / fDistFrmProjPlane);

		D3DXMatrixPerspectiveLH(&MyProjMatrix,wAtNearZ,hAtNearZ,fNearZ,fFarZ);
	}
	else
	{
		D3DXMatrixOrthoLH(&MyProjMatrix,fProjSizeX,fProjSizeY,fNearZ,fFarZ);
	}
	//install the matrix
	g_RenderStateMgr.SetTransform(D3DTS_PROJECTION,&MyProjMatrix);

	//set the ambient color so that it will represent the color and distance from the light, and also
	//the higher the pass number, the less alpha it should have
	LTVector vAmbient = vLightColor;

	VEC_MAX(vAmbient, vAmbient, LTVector(0, 0, 0));
	VEC_MIN(vAmbient, vAmbient, LTVector(255.0f, 255.0f, 255.0f));

	g_RenderStateMgr.SetAmbientLight(255.0f - vAmbient.x, 255.0f - vAmbient.y, 255.0f - vAmbient.z);

	RenderModelPieces(pInstance);

	//now we need to render all attachments
	Attachment* pCurr = pInstance->m_Attachments;

	while(pCurr)
	{
		//get this object
		HOBJECT hParent;
		HOBJECT hChild;
		ilt_common_client->GetAttachmentObjects((HATTACHMENT)pCurr, hParent, hChild);
		LTObject* pCurrObj = (LTObject*)hChild;

		//extend our bounds to include this (if it is a model)
		if(pCurrObj && pCurrObj->m_ObjectType == OT_MODEL)
		{
			//we have our model
			ModelInstance* pAttachInstance = pCurrObj->ToModel();

			if(pAttachInstance != pInstance)
			{
				//we want to render this model
				RenderModelPieces(pAttachInstance);
			}
		}

		pCurr = pCurr->m_pNext;
	}
}

//given some information it will fill in the information structure with the appropriate planes
//and other information needed to render
static void BuildShadowLightInfo( ShadowLightInfo& info, float fMaxShadowDist, bool bPerspective )
{
	// Setup the frame of reference (up is (0,1,0) and right is generated).
	info.m_ProjectionPlane.m_Dist = info.m_ProjectionPlane.m_Normal.Dot( info.m_vProjectionCenter );

	LTVector vWindowLeft	= info.m_vProjectionCenter - info.m_Vecs[0] * info.m_fSizeX * 0.5f;
	LTVector vWindowRight	= info.m_vProjectionCenter + info.m_Vecs[0] * info.m_fSizeX * 0.5f;
	LTVector vWindowTop		= info.m_vProjectionCenter + info.m_Vecs[1] * info.m_fSizeY * 0.5f;
	LTVector vWindowBottom	= info.m_vProjectionCenter - info.m_Vecs[1] * info.m_fSizeY * 0.5f;
	info.m_vWindowTopLeft	= info.m_vProjectionCenter - info.m_Vecs[0] * info.m_fSizeX * 0.5f - info.m_Vecs[1] * info.m_fSizeY * 0.5f;

	// Setup the clipping planes.
	LTPlane *pPlane = NULL;

	pPlane					= &info.m_FrustumPlanes[ CPLANE_NEAR_INDEX ];
	pPlane->m_Normal		= info.m_Vecs[2];
	pPlane->m_Dist			= pPlane->m_Normal.Dot( info.m_vProjectionCenter );

	pPlane					= &info.m_FrustumPlanes[ CPLANE_FAR_INDEX ];
	pPlane->m_Normal		= - info.m_Vecs[2];
	pPlane->m_Dist			= pPlane->m_Normal.Dot( info.m_vProjectionCenter + ( info.m_Vecs[2] * fMaxShadowDist ) );

	if (bPerspective)
	{
		pPlane					= &info.m_FrustumPlanes[ CPLANE_LEFT_INDEX ];
		pPlane->m_Normal		= ( vWindowLeft - info.m_vLightOrigin ).Cross( info.m_Vecs[1] );
		pPlane->m_Normal.Norm();
		pPlane->m_Dist			= pPlane->m_Normal.Dot( info.m_vLightOrigin );

		pPlane					= &info.m_FrustumPlanes[ CPLANE_RIGHT_INDEX ];
		pPlane->m_Normal		= info.m_Vecs[1].Cross( vWindowRight - info.m_vLightOrigin );
		pPlane->m_Normal.Norm();
		pPlane->m_Dist			= pPlane->m_Normal.Dot( info.m_vLightOrigin );

		pPlane					= &info.m_FrustumPlanes[ CPLANE_TOP_INDEX ];
		pPlane->m_Normal		= ( vWindowTop - info.m_vLightOrigin ).Cross( info.m_Vecs[0] );
		pPlane->m_Normal.Norm();
		pPlane->m_Dist			= pPlane->m_Normal.Dot( info.m_vLightOrigin );

		pPlane					= &info.m_FrustumPlanes[ CPLANE_BOTTOM_INDEX ];
		pPlane->m_Normal		= info.m_Vecs[0].Cross( vWindowBottom - info.m_vLightOrigin );
		pPlane->m_Normal.Norm();
		pPlane->m_Dist			= pPlane->m_Normal.Dot( info.m_vLightOrigin );
	}
	else
	{
		// Ortho projection...
		pPlane					= &info.m_FrustumPlanes[ CPLANE_LEFT_INDEX ];
		pPlane->m_Normal		= info.m_Vecs[0];
		pPlane->m_Dist			= pPlane->m_Normal.Dot(info.m_vLightOrigin - info.m_Vecs[0] * info.m_fSizeX * 0.5f);

		pPlane					= &info.m_FrustumPlanes[ CPLANE_RIGHT_INDEX ];
		pPlane->m_Normal		= - info.m_Vecs[0];
		pPlane->m_Dist			= pPlane->m_Normal.Dot(info.m_vLightOrigin + info.m_Vecs[0] * info.m_fSizeX * 0.5f);

		pPlane					= &info.m_FrustumPlanes[ CPLANE_TOP_INDEX ];
		pPlane->m_Normal		= - info.m_Vecs[1];
		pPlane->m_Dist			= pPlane->m_Normal.Dot(info.m_vLightOrigin + info.m_Vecs[1] * info.m_fSizeY * 0.5f);

		pPlane					= &info.m_FrustumPlanes[ CPLANE_BOTTOM_INDEX ];
		pPlane->m_Normal		= info.m_Vecs[1];
		pPlane->m_Dist			= pPlane->m_Normal.Dot(info.m_vLightOrigin - info.m_Vecs[1] * info.m_fSizeY * 0.5f);
	}
}


//------------------------------------------------------------------------------------------
// CRenderShadowList
//
//
//------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------
// Statics
//------------------------------------------------------------------------------------------

//flag indicating if we failed to load this pixel shader prior
bool CRenderShadowList::s_bFailedPSHandle = false;


//------------------------------------------------------------------------------------------
// Functions
//------------------------------------------------------------------------------------------


CRenderShadowList::CRenderShadowList()
{
}

CRenderShadowList::CRenderShadowList(const CRenderShadowList&)
{
}

//singleton access
CRenderShadowList& CRenderShadowList::GetSingleton()
{
	static CRenderShadowList sSingleton;
	return sSingleton;
}

//given a model instance and it's light list, this will add all appropriate shadows to the list
//of shadows to be rendered
void CRenderShadowList::QueueShadows(ModelInstance* pInstance, const CRelevantLightList& LightList)
{
	//determine the largest number of shadows that we can use
	uint32 nMaxShadows = (uint32)g_CV_ModelShadow_Proj_MaxShadows.m_Val;

	//bail if there are no shadows or no lights
	if((nMaxShadows == 0) || (LightList.GetNumLights() == 0))
		return;

	//draw the debug lights if appropriate
	if (g_CV_ModelShadow_Proj_DrawLights.m_Val)
	{
		DrawLinesFromLights(LightList, pInstance->GetPos());
	}

	//Determine where the model is, we need to track the translation node if possible for models
	//that are cinematic and extend beyond their bounding box
	LTVector vModelPos	= pInstance->GetPos();

	ModelNode* pNode = pInstance->GetModelDB()->FindNode("translation");
	if(pNode)
	{
		//we have the node, lets get its position
		LTMatrix mMat;
		pInstance->GetNodeTransform(pNode->m_NodeIndex, mMat, true);
		mMat.GetTranslation(vModelPos);
	}

	uint32 nShadows = 0;

	//determine if we will be doing tweening of the lights
	bool bTweenLights = g_CV_ModelShadow_Proj_Tween.m_Val != 0;

	//tween information
	LTVector vTweenPos;
	LTVector vTweenColor;
	bool	 bTweenOrtho;

	//determine if we can support perspective
	bool bPerspective = (!!g_CV_ModelShadow_Proj_Perspective.m_Val) && CModelShadowShader::GetSingleton()->CanSupportPerspective();

	//the index of the last light used for shadow generation in the list
	uint32 nLastUsedLight = 0;

	for (uint32 nCurrLight = 0; nCurrLight < LightList.GetNumLights(); ++nCurrLight)
	{
		const CRenderLight& Light = LightList.GetLight(nCurrLight);
		assert(Light.GetType() != CRenderLight::eLight_Invalid);

		// Skip out if the light doesn't cast shadows (and we're not overriding it)...
		if (!(Light.GetFlags() & FLAG_CASTSHADOWS) && !g_CV_DrawAllModelShadows.m_Val)
			continue;

		vTweenColor = LightList.GetLightSample(nCurrLight);

		if(Light.GetType() == CRenderLight::eLight_Dir)
		{
			vTweenPos	= pInstance->GetPos() - Light.GetDir() * pInstance->GetDims().Mag() * 2.0f;
			bTweenOrtho	= true;
		}
		else
		{
			vTweenPos	= Light.GetPos();
			bTweenOrtho	= !bPerspective;
		}

		//save this index
		nLastUsedLight = nCurrLight;
		nShadows++;

		//add it to the list
		if((nShadows < nMaxShadows) || !bTweenLights)
		{
			QueueShadow(pInstance, vModelPos, vTweenPos, vTweenColor, bTweenOrtho);
		}

		//see if we have filled up the list, if so, no need to continue on
		if(nShadows >= nMaxShadows)
			break;
	}

	//don't bother tweening if we could fit all of our shadows appropriately
	if (bTweenLights && (nShadows >= nMaxShadows ))
	{
		// Merge the furthest away light source with the next furthest so that shadows are blended
		// between the furthest
		float fLowestScore = GetMaxVecComponent(LightList.GetLightSample(nLastUsedLight));
		float fNumBlendedLightSources = 1.0f;

		for (uint32 nCurrLight = nLastUsedLight + 1; nCurrLight < LightList.GetNumLights(); ++nCurrLight)
		{
			const CRenderLight& Light = LightList.GetLight(nCurrLight);
			assert(Light.GetType() != CRenderLight::eLight_Invalid);

			// Skip out if the light doesn't cast shadows (and we're not overriding it)...
			if (!(Light.GetFlags() & FLAG_CASTSHADOWS) && !g_CV_DrawAllModelShadows.m_Val)
				continue;

			float fBlendWeight			 = GetMaxVecComponent(LightList.GetLightSample(nCurrLight)) / fLowestScore;
			vTweenColor					+= LightList.GetLightSample(nCurrLight);
			fNumBlendedLightSources		+= fBlendWeight;

			if(Light.GetType() == CRenderLight::eLight_Dir)
			{
				vTweenPos	+= fBlendWeight * (pInstance->GetPos() - Light.GetDir() * pInstance->GetDims().Mag() * 2.0f);
			}
			else
			{
				vTweenPos	+= fBlendWeight * Light.GetPos();
			}
		}

		//scale the position back to the appropriate amount
		vTweenPos /= fNumBlendedLightSources;

		//now add this shadow to the list
		QueueShadow(pInstance, vModelPos, vTweenPos, vTweenColor, bTweenOrtho);
	}

	//success
	return;
}

//------------------------------------------------------------------
// Sorting heuristic for shadow prioritization
//------------------------------------------------------------------

float CRenderShadowList::CalcScore(const ViewParams& Params, const SQueuedShadow& Shadow)
{
	//we need to do a sort based upon
	// a) the distance from the camera
	// b) the opacity of the shadow
	// c) possibly the size of the model

	//the weights for each parameter
	static const float kfDistWeight		= 1.0f;
	static const float kfColorWeight	= (1.0f - kfDistWeight);

	//figure out distance information
	float fDistanceSqr = (Params.m_Pos - Shadow.m_vModelPos).MagSqr();
	float fToFarZRatio = fDistanceSqr / (Params.m_FarZ * Params.m_FarZ);

	//color intensity information
	float fMaxColor = (GetMaxVecComponent(Shadow.m_vColor) / 255.0f);

	//composite data into a final score
	float fScore  = (1.0f - fToFarZRatio) * kfDistWeight +
					fMaxColor * kfColorWeight;

	return fScore;
}

//this will render all shadows in the queue to the world and flush it
void CRenderShadowList::RenderQueuedShadows(const ViewParams& Params)
{
	//see if we need to sort our textures so that we only draw the N most prominant ones
	uint32 nShadowsToRender;

	if(g_CV_ModelShadow_Proj_MaxShadowsPerFrame.m_Val >= 0)
	{
		//and set the limit to the smaller of the number of shadows or the amount we are allowed to draw
		nShadowsToRender = LTMIN(g_CV_ModelShadow_Proj_MaxShadowsPerFrame.m_Val, m_cShadowList.size());

		if(nShadowsToRender)
		{
			//calculate the score for all shadows
			for(uint32 nCurrShadow = 0; nCurrShadow < m_cShadowList.size(); nCurrShadow++)
			{
				m_cShadowList[nCurrShadow].m_fScore = CalcScore(Params, m_cShadowList[nCurrShadow]);
			}

			//we need to limit the shadows, so sort them
			sort(m_cShadowList.begin(), m_cShadowList.end());
		}
	}
	else
	{
		nShadowsToRender = m_cShadowList.size();
	}

	//see if we have any shadows to render
	if(nShadowsToRender == 0)
	{
		m_cShadowList.clear();
		return;
	}

	//check for console variable changes
	UpdateTextureList();

	//just bail if we don't have any shadows, otherwise we will just infinitely loop
	if(m_cTextureList.size() == 0)
	{
		m_cShadowList.clear();
		return;
	}

	//lets determine if we are going to be doing shadow blending
	bool bBlurShadows = g_CV_ModelShadow_Proj_BlurShadows.m_Val && (m_cTextureList.size() >= 2);

	//if we are trying to blend shadows, lets make sure that we have a pixel shader that can
	//handle the actual blending
	if(bBlurShadows)
	{
		// Get the pixel shader.
		LTPixelShader *pPixelShader = LTPixelShaderMgr::GetSingleton().GetPixelShader(LTPixelShader::PIXELSHADER_SHADOWBLUR);
		if (NULL == pPixelShader)
		{
			FileRef ref;
			ref.m_FileType 	= FILE_ANYFILE;
			ref.m_pFilename = "ps\\blurshadow.psh";

			// Try to load it.
			ILTStream *pStream = g_pIClientFileMgr->OpenFile(&ref);
			if (NULL != pStream)
			{
				if (LTPixelShaderMgr::GetSingleton().AddPixelShader(pStream, ref.m_pFilename,
																	LTPixelShader::PIXELSHADER_SHADOWBLUR, true))
				{
					pPixelShader = LTPixelShaderMgr::GetSingleton().GetPixelShader(LTPixelShader::PIXELSHADER_SHADOWBLUR);
				}

				// Close the file.
				pStream->Release();
			}
		}

		// See if we can continue.
		if (NULL != pPixelShader && pPixelShader->IsValidShader())
		{
			s_bFailedPSHandle = false;
		}
		else
		{
			s_bFailedPSHandle = true;
		}
	}

	//we need to make sure and save our render targets
	LPDIRECT3DSURFACE9 pDepthStencilBuffer	= NULL;
	LPDIRECT3DSURFACE9 pPrevRenderTarget	= NULL;
	D3D_CALL(PD3DDEVICE->GetDepthStencilSurface(&pDepthStencilBuffer));
	D3D_CALL(PD3DDEVICE->GetRenderTarget(&pPrevRenderTarget));

	//the current shadow tex that we want to render to
	uint32 nCurrShadowTex = 0;
	uint32 nNumShadowTex = m_cTextureList.size();

	//we also need to consider that if we are blurring we can only use an even number of textures
	if(bBlurShadows && (nNumShadowTex % 2))
		nNumShadowTex--;

	//the first shadow in the span
	uint32 nStartShadow = 0;

	//we need to make sure to preserve some transformation information so it can be properly
	//restored
	//restore our viewport
	D3DXMATRIX mIdentity;
	D3DXMatrixIdentity(&mIdentity);

	//now get our old viewport for saving
	D3DVIEWPORT9 OldViewport;
	PD3DDEVICE->GetViewport(&OldViewport);

	// Figure out our projection matrix...
	D3DMATRIX PrevViewMatrix;
	PD3DDEVICE->GetTransform(D3DTS_VIEW,&PrevViewMatrix);

	D3DMATRIX PrevProjMatrix;
	PD3DDEVICE->GetTransform(D3DTS_PROJECTION,&PrevProjMatrix);

	//we need to run through our shadows, render N textures, render those on the world, and repeat
	//until we have drawn all shadows
	for(uint32 nCurrShadow = 0; nCurrShadow < nShadowsToRender; nCurrShadow++)
	{
		//see if we need to flush out the list of shadows
		if(nCurrShadowTex >= nNumShadowTex)
		{
			//we need to flush this, restore the screen as the render target and render
			D3D_CALL(PD3DDEVICE->SetRenderTarget(pPrevRenderTarget,pDepthStencilBuffer));
			PD3DDEVICE->SetViewport(&OldViewport);
			g_RenderStateMgr.SetTransform(D3DTS_VIEW,&PrevViewMatrix);
			g_RenderStateMgr.SetTransform(D3DTS_PROJECTION,&PrevProjMatrix);
			g_RenderStateMgr.SetTransform(D3DTS_WORLDMATRIX(0),&mIdentity);

			RenderTexturesOnWorld(Params, nStartShadow, nCurrShadow, bBlurShadows);

			//reset our positions
			nStartShadow	= nCurrShadow;
			nCurrShadowTex	= 0;
		}

		//we need to render this shadow to the appropriate texture
		RenderShadowTexture(nCurrShadow, nCurrShadowTex);

		if(bBlurShadows)
		{
			BlurShadowTexture(nCurrShadowTex, nCurrShadowTex + 1);
			nCurrShadowTex += 2;
		}
		else
		{
			//and onto the next texture...
			nCurrShadowTex++;
		}
	}

	//restore back to the main screen
	D3D_CALL(PD3DDEVICE->SetRenderTarget(pPrevRenderTarget,pDepthStencilBuffer));
	PD3DDEVICE->SetViewport(&OldViewport);
	g_RenderStateMgr.SetTransform(D3DTS_VIEW,&PrevViewMatrix);
	g_RenderStateMgr.SetTransform(D3DTS_PROJECTION,&PrevProjMatrix);
	g_RenderStateMgr.SetTransform(D3DTS_WORLDMATRIX(0),&mIdentity);

	//see if we have any leftover
	if(nStartShadow < nShadowsToRender)
	{
		RenderTexturesOnWorld(Params, nStartShadow, nShadowsToRender, bBlurShadows);
	}

	//release our references to the render target
	pPrevRenderTarget->Release();	pPrevRenderTarget	  = NULL;
	pDepthStencilBuffer->Release();	pDepthStencilBuffer	  = NULL;

	//clear out the list
	m_cShadowList.clear();

	//restore our device state
	g_Device.SetDefaultRenderStates();
}

//call this to free all memory associated with the shadow list. This will require it to grow
//again, but is a good chance to clean up after some level that used a lot of models
//and caused it to grow too large
void CRenderShadowList::FreeListMemory()
{
	TShadowList cEmpty;
	m_cShadowList.swap(cEmpty);
}

//forces all the associated shadow textures to be invalidated
void CRenderShadowList::InvalidateShadowTextures()
{
	for(TTextureList::iterator it = m_cTextureList.begin(); it != m_cTextureList.end(); it++)
	{
		//clear it out
		*it = NULL;
	}
}

//frees all shadow textures
void CRenderShadowList::FreeShadowTextures()
{
	for(TTextureList::iterator it = m_cTextureList.begin(); it != m_cTextureList.end(); it++)
	{
		if(*it)
		{
			D3DShadowTextureFactory::Get()->FreeShadowTexture(*it);
			*it = NULL;
		}
	}
}

//updates the texture list to reflect any changed console variables
void CRenderShadowList::UpdateTextureList()
{
	//see if we need to resize our list
	if( ((uint32)g_CV_ModelShadow_Proj_NumTextures.m_Val != m_cTextureList.size()) ||
		((uint32)g_CV_ModelShadow_Proj_TextureRes.m_Val != m_nTextureRes))
	{
		//determine how many we should actually create
		uint32 nNumTextures = LTMIN(MAX_SHADOW_TEXTURES, (uint32)g_CV_ModelShadow_Proj_NumTextures.m_Val);
		uint32 nDesiredTextureRes = LTCLAMP(g_CV_ModelShadow_Proj_TextureRes.m_Val, MIN_SHADOW_TEXTURE_SIZE, MAX_SHADOW_TEXTURE_SIZE);

		//round it down to the nearest power of 2
		uint32 nNearestPower = MAX_SHADOW_TEXTURE_SIZE;
		while(nNearestPower > nDesiredTextureRes)
			nNearestPower >>= 1;

		if((nNumTextures != m_cTextureList.size()) || (nNearestPower != m_nTextureRes))
		{
			dsi_ConsolePrint("Recreating %d shadow textures", nNumTextures);

			//we need to recreate the textures
			FreeShadowTextures();

			//now resize the list and update our texture size
			m_cTextureList.resize(nNumTextures);
			m_nTextureRes = nNearestPower;

			//clear out the texture list
			InvalidateShadowTextures();
		}
	}
}

//after the textures have been rendered into the appropriate
//slots in the texture list, this function will render those textures on the world
//using the data
void CRenderShadowList::RenderTexturesOnWorld(const ViewParams& Params, uint32 nStart, uint32 nEnd, bool bBlurred)
{
	assert(nEnd - nStart <= m_cTextureList.size());

	for(uint32 nCurrShadow = nStart; nCurrShadow < nEnd; nCurrShadow++)
	{
		//get our shadow information
		SQueuedShadow& ShadowInfo = m_cShadowList[nCurrShadow];
		ModelInstance* pInstance = ShadowInfo.m_pInstance;
		assert(pInstance);

		LTVector vModelPos  = ShadowInfo.m_vModelPos;

		float fModelRadius	= pInstance->GetDims().Mag( );
		float fSize			= fModelRadius * 2.0f;

		//now that we have all the shadow textures ready, we must now run through and actually
		//render the additional passes on the world
		LTVector vLightPos(ShadowInfo.m_vLightPos);
		LTVector vLightColor(ShadowInfo.m_vColor);

		LTVector vLightDir = vModelPos - vLightPos;
		vLightDir.Normalize();

		FindBoundingInfo(pInstance, ShadowInfo.m_vModelPos, vLightPos, vLightDir, vModelPos, fSize);
		fSize *= 2.0f;

		// Setup the frame of reference
		LTVector vLightUp, vLightRight;
		BuildLightRefFrame(vLightDir, vLightUp, vLightRight);

		//determine the texture that we should use
		uint32 nTextureIndex = nCurrShadow - nStart;

		if(bBlurred)
			nTextureIndex = nTextureIndex * 2 + 1;

		D3DShadowTexture* pD3DShadowTexture = m_cTextureList[nTextureIndex];

		//sanity check
		if(!pD3DShadowTexture || !pD3DShadowTexture->m_pD3DTexture)
		{
			continue;
		}

		//figure out the amount that we want to offset the fade
		float fFadeOffset = pInstance->GetDims().Mag() * g_CV_ModelShadow_Proj_DimFadeOffsetScale.m_Val;

		// Setup the frame of reference (up is (0,1,0) and right is generated).
		ShadowLightInfo info;

		info.m_Vecs[0]					= vLightRight;
		info.m_Vecs[1]					= vLightUp;
		info.m_Vecs[2]					= vLightDir;

		info.m_fSizeX					= fSize;
		info.m_fSizeY					= fSize;
		info.m_vLightOrigin				= vLightPos;
		info.m_vProjectionCenter		= vModelPos;

		info.m_ProjectionPlane.Init(vLightDir, vModelPos);

		BuildShadowLightInfo(info, fModelRadius, !ShadowInfo.m_bOrtho);

		//render the world with our shadow texture
		CModelShadowShader::GetSingleton()->SetShadowInfo(	Params, pD3DShadowTexture->m_pD3DTexture, &info,
															1.0f, -1.0f,
															!ShadowInfo.m_bOrtho,
															g_CV_ModelShadow_Proj_MaxProjDist.m_Val,
															fFadeOffset);

		d3d_RenderWorldWithAggregate(NUM_CLIPPLANES, info.m_FrustumPlanes, g_ViewParams, CModelShadowShader::GetSingleton());
	}
}

//blurs the specified texture to the other texture, assumes that the blur shader is valid
bool CRenderShadowList::BlurShadowTexture(uint32 nSrcTex, uint32 nDestTex)
{
	//a few sanity checks
	assert(nSrcTex != nDestTex);

	if(!m_cTextureList[nSrcTex])
		return false;

	LPDIRECT3DTEXTURE9 pSrcTex = m_cTextureList[nSrcTex]->m_pD3DTexture;

	if(!pSrcTex)
		return false;

	//see if we have a valid destination texture
	if(!m_cTextureList[nDestTex])
	{
		//we don't, try and allocate it....
		m_cTextureList[nDestTex] = D3DShadowTextureFactory::Get()->AllocShadowTexture( m_nTextureRes, m_nTextureRes );

		//make sure we could allocate the texture safely
		if(!m_cTextureList[nDestTex])
		{
			dsi_ConsolePrint("Unable to allocate the shadow texture");
			return false;
		}
	}

	//determine if we can use the pixel shader version and do it in a single pass
	bool bUsePixelShader 		= false;
	LTPixelShader *pPixelShader = NULL;
	if (!s_bFailedPSHandle)
	{
		pPixelShader = LTPixelShaderMgr::GetSingleton().GetPixelShader(LTPixelShader::PIXELSHADER_SHADOWBLUR);
		if (NULL != pPixelShader && pPixelShader->IsValidShader())
		{
			bUsePixelShader = true;
		}
	}

	uint32	nNumTextures = (bUsePixelShader) ? 4 : 2;

	//get the dimensions of the texture
	float fWidth	= (float)m_nTextureRes;
	float fHeight	= (float)m_nTextureRes;

	//and set our render target to be our destination texture
	LPDIRECT3DSURFACE9 pTextureRenderSurface = NULL;
	m_cTextureList[nDestTex]->m_pD3DTexture->GetSurfaceLevel(0, &pTextureRenderSurface);

	if(pTextureRenderSurface)
	{
		if (PD3DDEVICE->SetRenderTarget(pTextureRenderSurface, NULL) == D3D_OK)
		{
			//clear the current viewport to an opaque white color
			uint32 nClearColor = g_CV_ModelShadow_Proj_TintFill.m_Val ? D3DRGBA_255(0, 255, 0, 0) : D3DRGBA_255(255, 255, 255, 0);
			PD3DDEVICE->Clear(0, NULL, D3DCLEAR_TARGET, nClearColor, 1.0f, 0);

			//setup our new viewport so that we won't render to the outer edge
			D3DVIEWPORT9 NewViewport;
			NewViewport.X		= 1;
			NewViewport.Y		= 1;
			NewViewport.Width	= m_nTextureRes - 2;
			NewViewport.Height	= m_nTextureRes - 2;
			NewViewport.MinZ	= 0.0f;			// the Z values are irrelevant since we don't have a depth buffer
			NewViewport.MaxZ	= 1.0f;
			PD3DDEVICE->SetViewport(&NewViewport);

			//alright, we are good to go, load up the source texture into channels
			uint32 nCurrChannel;
			for(nCurrChannel = 0; nCurrChannel < nNumTextures; nCurrChannel++)
			{
				d3d_SetTextureDirect(pSrcTex, nCurrChannel);
			}

			//setup texture clamping
			SamplerStateSet ssWrapU0(0, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);
			SamplerStateSet ssWrapV0(0, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);
			SamplerStateSet ssWrapU1(1, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);
			SamplerStateSet ssWrapV1(1, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);
			SamplerStateSet ssWrapU2(2, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);
			SamplerStateSet ssWrapV2(2, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);
			SamplerStateSet ssWrapU3(3, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);
			SamplerStateSet ssWrapV3(3, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);

			//make sure that we don't do any Z tests though
			StateSet ssZFunc(D3DRS_ZFUNC, D3DCMP_ALWAYS);

			//and also make sure that we don't write to Z. Yes we need both. One fixes a bug
			//on NVidia, another on ATI
			StateSet ssZRead(D3DRS_ZENABLE, D3DZB_FALSE);
			StateSet ssZWrite(D3DRS_ZWRITEENABLE, FALSE);

			//alright, now we setup the vertices to give info to the blend
			if(bUsePixelShader)
			{
				assert(pPixelShader->IsValidShader());
				static CBlurTextureVertex Verts[4];

				//setup the positions of the four vertices
				Verts[0].Init(0.0f, 0.0f, fWidth, fHeight);
				Verts[1].Init(1.0f, 0.0f, fWidth, fHeight);
				Verts[2].Init(1.0f, 1.0f, fWidth, fHeight);
				Verts[3].Init(0.0f, 1.0f, fWidth, fHeight);

				// Install the pixel shader.
				LTPixelShaderMgr::GetSingleton().InstallPixelShader(pPixelShader);

				//render the polygon
				PD3DDEVICE->SetVertexShader(NULL);
				PD3DDEVICE->SetFVF(BLURTEXTUREVERTEX_FORMAT);
				PD3DDEVICE->DrawPrimitiveUP(D3DPT_TRIANGLEFAN, 2, Verts, sizeof(CBlurTextureVertex));

				// Uninstall the pixel shader.
				LTPixelShaderMgr::GetSingleton().UninstallPixelShader();
			}
			else
			{
				static CBlurTexture2PassVertex Verts[4];

				//setup the blend modes for the operation so we take 25% of each sample
				StageStateSet tsargc00(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
				StageStateSet tsargc10(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
				StageStateSet tsopc0  (0, D3DTSS_COLOROP,   D3DTOP_MODULATE);

				StageStateSet tsarga00(0, D3DTSS_ALPHAARG1, D3DTA_DIFFUSE);
				StageStateSet tsopa0  (0, D3DTSS_ALPHAOP,   D3DTOP_SELECTARG1);

				StageStateSet tsargc01(1, D3DTSS_COLORARG1, D3DTA_CURRENT);
				StageStateSet tsargc11(1, D3DTSS_COLORARG2, D3DTA_TEXTURE);
				StageStateSet tsopc1  (1, D3DTSS_COLOROP,   D3DTOP_MODULATEALPHA_ADDCOLOR);

				StageStateSet tsarga01(1, D3DTSS_ALPHAARG1, D3DTA_CURRENT);
				StageStateSet tsarga11(1, D3DTSS_ALPHAARG2, D3DTA_TEXTURE);
				StageStateSet tsopa1  (1, D3DTSS_ALPHAOP,   D3DTOP_MODULATE2X);

				StageStateSet tsopc2  (2, D3DTSS_COLOROP,   D3DTOP_DISABLE);
				StageStateSet tsopa2  (2, D3DTSS_ALPHAOP,   D3DTOP_DISABLE);

				//setup the blend mode so that we do an additive blend on the second pass,
				//and an overwrite on the first
				StateSet ss0(D3DRS_SRCBLEND, D3DBLEND_ONE);
				StateSet ss1(D3DRS_DESTBLEND, D3DBLEND_ONE);

				//setup the vertex shader
				PD3DDEVICE->SetVertexShader(NULL);
				PD3DDEVICE->SetFVF(BLURTEXTURE2PASSVERTEX_FORMAT);

				float fSign = 1.0f;
				for(uint32 nPass = 0; nPass < 2; nPass++)
				{
					//setup the positions of the four vertices
					Verts[0].Init(0.0f, 0.0f, fWidth, fHeight, fSign);
					Verts[1].Init(1.0f, 0.0f, fWidth, fHeight, fSign);
					Verts[2].Init(1.0f, 1.0f, fWidth, fHeight, fSign);
					Verts[3].Init(0.0f, 1.0f, fWidth, fHeight, fSign);

					//handle setting up the blending
					StateSet ssAlphaBlend(D3DRS_ALPHABLENDENABLE, (nPass > 0) ? TRUE : FALSE);

					//now make it so that it will offset on the opposite diagonal
					fSign = -fSign;

					//render the polygon
					PD3DDEVICE->DrawPrimitiveUP(D3DPT_TRIANGLEFAN, 2, Verts, sizeof(CBlurTexture2PassVertex));
				}
			}

			//we need to make sure and clear out all the textures again so they can be used
			for(nCurrChannel = 0; nCurrChannel < nNumTextures; nCurrChannel++)
			{
				d3d_DisableTexture(nCurrChannel);
			}
		}

		//clean up our target
		pTextureRenderSurface->Release();
		pTextureRenderSurface = NULL;
	}

	return true;
}

//renders the specified shadow into the specified texture
bool CRenderShadowList::RenderShadowTexture(uint32 nShadow, uint32 nTexture)
{
	assert(nShadow < m_cShadowList.size());
	assert(nTexture < m_cTextureList.size());

	//make sure our texture is valid
	if(!m_cTextureList[nTexture])
	{
		//we need to regenerate our shadow texture
		m_cTextureList[nTexture] = D3DShadowTextureFactory::Get()->AllocShadowTexture( m_nTextureRes, m_nTextureRes );

		//make sure we could allocate the texture safely
		if(!m_cTextureList[nTexture])
		{
			dsi_ConsolePrint("Unable to allocate the shadow texture");
			return false;
		}
	}

	if(!m_cTextureList[nTexture]->m_pD3DTexture)
	{
		assert(!"Texture allocated, but internal D3D texture is invalid");
		return false;
	}

	//cache some useful information
	SQueuedShadow&		ShadowInfo = m_cShadowList[nShadow];
	ModelInstance*		pInstance = ShadowInfo.m_pInstance;

	LTVector vModelPos  = pInstance->GetPos();

	//disable lighting
	for (uint32 i = 0; i < 8; ++i)
		g_RenderStateMgr.LightEnable(i, false);

	//	COMPUTE MODEL SHADOW LOD AND SIZE
	float fModelRadius	= pInstance->GetDims().Mag();
	float fSize			= fModelRadius * 2.0f;

	LTVector vLightPos(ShadowInfo.m_vLightPos);
	LTVector vLightColor(ShadowInfo.m_vColor);

	LTVector vLightDir = vModelPos - vLightPos;
	vLightDir.Normalize();

	FindBoundingInfo(pInstance, ShadowInfo.m_vModelPos, vLightPos, vLightDir, vModelPos, fSize);
	fSize *= 2.0f;

	// Setup the frame of reference
	LTVector vLightUp, vLightRight;
	BuildLightRefFrame(vLightDir, vLightUp, vLightRight);

	// For drawing the projection frustum...
	if (g_CV_ModelShadow_Proj_DrawProjPlane.m_Val)
	{
		DrawLightPlanes(vModelPos, vLightPos, vLightUp, vLightRight, fSize);
	}

	// Set the ShadowTexture as the render target...
	LPDIRECT3DSURFACE9 pTextureRenderSurface = NULL;
	D3D_CALL(m_cTextureList[nTexture]->m_pD3DTexture->GetSurfaceLevel(0, &pTextureRenderSurface));

	if(pTextureRenderSurface)
	{
		// Render the Model onto the texture...
		if (PD3DDEVICE->SetRenderTarget(pTextureRenderSurface, NULL) == D3D_OK)
		{
			RenderModelShadow(pInstance, ShadowInfo.m_vModelPos, vLightPos, vLightDir, vLightUp, vLightColor, m_nTextureRes, fSize, fSize, !ShadowInfo.m_bOrtho);
		}

		// Reset the render target...
		pTextureRenderSurface->Release();
		pTextureRenderSurface = NULL;
	}

	return true;
}

//handles inserting a shadow into the list
void CRenderShadowList::QueueShadow(ModelInstance* pInstance, const LTVector& vModelPos, const LTVector& vPos, const LTVector& vColor, bool bOrtho)
{
	//first off, see if we can cull this shadow if it is too dim
	if(	(GetMaxVecComponent(vColor) * g_CV_ModelShadow_Proj_Alpha.m_Val) < g_CV_ModelShadow_Proj_MinColorComponent.m_Val)
	{
		return;
	}

	LTVector vFinalLightPos = vPos;

	//we can't. Make sure that the light position is outside of the model
	LTVector vToLight = vPos - vModelPos;

	float fDistToLightSqr	= vToLight.MagSqr();
	float fDimsSqr			= pInstance->GetDims().MagSqr();

	if((fDistToLightSqr > 0.001f) && (fDistToLightSqr < fDimsSqr))
	{
		//move it on out
		vFinalLightPos = vModelPos + vToLight * (float)sqrt(fDimsSqr / fDistToLightSqr);
	}

	//figure out the opacity of the model
	float fAlpha = pInstance->m_ColorA / 255.0f;

	//apply the global alpha scale on the shadow
	fAlpha *= g_CV_ModelShadow_Proj_Alpha.m_Val;

	SQueuedShadow Shadow;
	Shadow.m_pInstance	= pInstance;
	Shadow.m_vModelPos  = vModelPos;
	Shadow.m_vLightPos	= vFinalLightPos;
	Shadow.m_vColor		= vColor * fAlpha;
	Shadow.m_bOrtho		= bOrtho;

	m_cShadowList.push_back(Shadow);
}

//this will empty all shadows out of the list
void CRenderShadowList::FlushShadows()
{
	m_cShadowList.clear();
}

#include "precompile.h"

//IClientShell game client shell object.
#include "iclientshell.h"
static IClientShell *i_client_shell;
define_holder(IClientShell, i_client_shell);

#include "setupmodel.h"
#include "d3dmeshrendobj_rigid.h"
#include "d3dmeshrendobj_skel.h"
#include "d3dmeshrendobj_vertanim.h"
#include "d3d_renderstatemgr.h"
#include "iltdrawprim.h"
#include "d3d_texture.h"
#include "rendermodelpiecelist.h"
#include "devicelightlist.h"
#include "renderstylemap.h"
#include "rendererframestats.h"
#include "LTEffectImpl.h"
#include "lteffectshadermgr.h"
#include "ltshaderdevicestateimp.h"
#include "rendererconsolevars.h"
#include <algorithm>



//---------------------------------------------------------------------------------
// Utility functions
//---------------------------------------------------------------------------------

//given a renderable model piece, this will look at the type and determine the appropriate
//means of starting its rendering block
static void BeginRenderableModelPiece(CDIModelDrawable* pRPiece, DDMatrix* pTransforms, CD3DRenderStyle* pRenderStyle, uint32 nCurrPass, bool bEffect=false)
{
	//we shouldn't ever have a NULL piece in here
	assert(pRPiece);

	//see what type it is
	switch (pRPiece->GetType())
	{
	case CRenderObject::eSkelMesh :
		((CD3DSkelMesh*)pRPiece)->BeginRender(pTransforms, pRenderStyle, nCurrPass);		
		break;
	case CRenderObject::eRigidMesh :
		{		
			if(bEffect)
			{
				((CD3DRigidMesh*)pRPiece)->BeginRenderWithEffect(pTransforms[((CD3DRigidMesh*)pRPiece)->GetBoneEffector()], pRenderStyle, nCurrPass);
			}else
			{
				((CD3DRigidMesh*)pRPiece)->BeginRender(pTransforms[((CD3DRigidMesh*)pRPiece)->GetBoneEffector()], pRenderStyle, nCurrPass);
			}
		}
		break;
	case CRenderObject::eVAMesh :
		break;
	}
}

//given a renderable model piece, this will render it. This assumes that the begin has already
//been called for this piece and any other pieces prior have been ended
static void RenderModelPiece( CDIModelDrawable* pRPiece,
							  ModelInstance*    pInstance,
							  DDMatrix*			pTransforms,
							  CD3DRenderStyle*  pRenderStyle,
							  uint32            nCurrPass,
							  bool				bEffect=false)
{
	//all parameters should be valid
	assert(pRPiece);
	assert(pInstance);
	assert(pRenderStyle);

	//see what type it is
	switch (pRPiece->GetType())
	{
	case CRenderObject::eSkelMesh :
		{
			((CD3DSkelMesh*)pRPiece)->Render(pInstance, pTransforms, pRenderStyle, nCurrPass);
			IncFrameStat(eFS_ModelRender_NumSkeletalPieces, 1);
		}
		break;
	case CRenderObject::eRigidMesh :
		{
			if(bEffect)
			{
				((CD3DRigidMesh*)pRPiece)->RenderWithEffect(pInstance, pTransforms[((CD3DRigidMesh*)pRPiece)->GetBoneEffector()], pRenderStyle, nCurrPass);
			}
			else
			{
				((CD3DRigidMesh*)pRPiece)->Render(pInstance, pTransforms[((CD3DRigidMesh*)pRPiece)->GetBoneEffector()], pRenderStyle, nCurrPass);
			}
			IncFrameStat(eFS_ModelRender_NumRigidPieces, 1);
		}
		break;
	case CRenderObject::eVAMesh :
		{
			((CD3DVAMesh*)pRPiece)->UpdateVA(pInstance->GetModelDB(), &pInstance->m_AnimTracker.m_TimeRef);
			// needs global pos.
			LTMatrix mTrans;
			if(pInstance->GetCachedTransform(((CD3DVAMesh*)pRPiece)->GetBoneEffector(), mTrans))
			{
				DDMatrix mD3DTrans;
				Convert_DItoDD(mTrans, mD3DTrans);
				((CD3DVAMesh*)pRPiece)->Render(pInstance, mD3DTrans, pRenderStyle, nCurrPass);
				IncFrameStat(eFS_ModelRender_NumVertexAnimatedPieces, 1);
			}
		}
		break;
	}

	//update our model polygon counts for information reporting
	IncFrameStat(eFS_ModelTriangles, pRPiece->GetPolyCount());
}

//given a renderable model piece, this will look at the type and determine the appropriate
//means of ending its rendering block
static void EndRenderableModelPiece(CDIModelDrawable* pRPiece)
{
	//bail if it is a null piece, this is actually valid in the end call
	if(!pRPiece)
		return;

	//see what type it is
	switch (pRPiece->GetType())
	{
	case CRenderObject::eSkelMesh :
		((CD3DSkelMesh*)pRPiece)->EndRender();
		break;
	case CRenderObject::eRigidMesh :
		((CD3DRigidMesh*)pRPiece)->EndRender();
		break;
	case CRenderObject::eVAMesh :
		break;
	}
}


//handles checking the to be installed render style and determining if it should be updated to accomodate
//for environment map panning. Note that I am fully aware that this is probably the worst way possible
//to handle this, but I just pulled this from the original model rendering pipeline. Returns if it was
//modified or not
static bool HandleReallyCloseEnvMapPanning(ModelInstance* pInstance, CD3DRenderStyle* pRenderStyle, uint32 nRenderPass, bool bPrevWasSet)
{
	// Modify the texture transform based on the viewer position
	// for player view models.
	// check to see that the r/s has a second stage that's an env map.
	// if so, change the texture matrix based on the current view x-form.
	if(pRenderStyle->GetRenderPassCount() > 0 )
	{
		RenderPassOp RenderPass;

		if(pRenderStyle->GetRenderPass(nRenderPass, &RenderPass))
		{
			// set uv transform.
			if(RenderPass.TextureStages[1].UVTransform_Enable == true)
			{
				if( (pInstance->m_Flags & FLAG_REALLYCLOSE) != 0 )
				{
					//early out if it is already set
					if(bPrevWasSet)
						return true;

					LTMatrix mat;
					mat.Identity();

					const LTMatrix & view = g_ViewParams.m_mInvView;

					// remember somethings.
					static LTVector vLastPos(view.m[0][3], view.m[1][3], view.m[2][3]);
					static bool bInitCamOfs = true;
					static LTMatrix mCamOfs;
					if (bInitCamOfs)
					{
						mCamOfs.Identity();
						bInitCamOfs = false;
					}

					// transform based camera pos and orientation (forwardvec);

					// set x/z UV-rotation based on forward vector of camera.
					LTVector v1,v2,v3;
					view.GetBasisVectors(&v1,&v2,&v3);

					// change in pos
					LTVector vOffset(view.m[0][3] - vLastPos.x, view.m[1][3] - vLastPos.y, view.m[2][3] - vLastPos.z);
					vOffset /= g_CV_PVModelEnvmapVelocity.m_Val;
					LTVector vCamRelOfs(vOffset.Dot(v1), vOffset.Dot(v2), vOffset.Dot(v3));

					LTMatrix mRotateAboutY;
					mRotateAboutY.SetupRot(LTVector(0.0f, 1.0f, 0.0f), vCamRelOfs.x);
					mCamOfs = mRotateAboutY * mCamOfs;
					LTMatrix mRotateAboutX;
					mRotateAboutX.SetupRot(LTVector(1.0f, 0.0f, 0.0f), -(vCamRelOfs.y + vCamRelOfs.z));
					mCamOfs = mRotateAboutX * mCamOfs;

					mCamOfs.Normalize();

					mat = mCamOfs * g_ViewParams.m_mView;
					vLastPos.x = view.m[0][3];
					vLastPos.y = view.m[1][3];
					vLastPos.z = view.m[2][3];

					// set the matrix.
					g_RenderStateMgr.SetTransform(D3DTS_TEXTURE1, (D3DMATRIX*)&mat);

					return true;
				}
				else if(bPrevWasSet)
				{
					g_RenderStateMgr.SetTransform(D3DTS_TEXTURE1, (D3DXMATRIX*)&RenderPass.TextureStages[1].UVTransform_Matrix);
				}
			}
		}
	}
	return false;
}

//---------------------------------------------------------------------------------
// SQueuedPiece
//---------------------------------------------------------------------------------

CRenderModelPieceList::SQueuedPiece::SQueuedPiece()
{
	//nothing really needs to be initialized...so don't for performance
}

CRenderModelPieceList::SQueuedPiece::SQueuedPiece(const SQueuedPiece& rhs)
{
	//copy everything over
	*this = rhs;
}

CRenderModelPieceList::SQueuedPiece::~SQueuedPiece()
{
	//nothing to clean up
}

//singleton access
CRenderModelPieceList& CRenderModelPieceList::GetSingleton()
{
	static CRenderModelPieceList sSingleton;
	return sSingleton;
}

CRenderModelPieceList::SQueuedPiece& CRenderModelPieceList::SQueuedPiece::operator=(const SQueuedPiece& rhs)
{
	//just copy
	memcpy(this, &rhs, sizeof(rhs));
	return *this;
}

bool CRenderModelPieceList::SQueuedPiece::operator==(const SQueuedPiece& rhs) const
{
	return memcmp(this, &rhs, sizeof(rhs)) == 0;
}

bool CRenderModelPieceList::SQueuedPiece::operator<(const SQueuedPiece& rhs) const
{
	//here is where it gets tricky, we need to sort based upon the following criteria:
	//1 - Really Close
	//2 - Render Priority
	//3 - Render Style
	//4 - Textures
	//5 - Render object
	//6 - Lights
	
	//really close needs to come last
	if(m_bReallyClose != rhs.m_bReallyClose)
		return rhs.m_bReallyClose;

	//now we need to render lowest priority first, then higher, and so on
	if(m_nRenderPriority != rhs.m_nRenderPriority)
		return m_nRenderPriority < rhs.m_nRenderPriority;

	//now the render style, just sort based upon address
	if(m_pRenderStyle != rhs.m_pRenderStyle)
		return m_pRenderStyle < rhs.m_pRenderStyle;

	//now sort based upon the textures
	int nTexDiff = memcmp(m_TextureList, rhs.m_TextureList, sizeof(m_TextureList));
	if(nTexDiff)
		return nTexDiff < 0;

	//now upon the render object
	if(m_pRenderPiece != rhs.m_pRenderPiece)
		return m_pRenderPiece < rhs.m_pRenderPiece;

	//now upon the lights
	if(!m_pLightList->IsSameAs(rhs.m_pLightList))
		return m_pLightList < rhs.m_pLightList;

	//they are equal
	return false;
}


//---------------------------------------------------------------------------------
// CRenderModelPieceList
//---------------------------------------------------------------------------------

CRenderModelPieceList::CRenderModelPieceList()
{
}

CRenderModelPieceList::CRenderModelPieceList(const CRenderModelPieceList&)
{
}


//adds a piece to be rendered
void CRenderModelPieceList::QueuePiece(ModelInstance* pInstance, ModelPiece* pPiece, CDIModelDrawable* pLOD, DDMatrix* pTransforms, CDeviceLightList* pLightList, bool bTexture, const ModelHookData* pHookData)
{
	//make sure that the items they passed are valid
	assert(pInstance);
	assert(pPiece);
	assert(pLOD);
	assert(pLightList);

	//we need to extract all the relevant information, fill out a piece structure, and add it
	//to the list to be rendered
	SQueuedPiece Info;
	Info.m_bReallyClose			= (pInstance->m_Flags & FLAG_REALLYCLOSE) != 0;
	Info.m_nRenderPriority		= pPiece->m_nRenderPriority;
	Info.m_pLightList			= pLightList;
	Info.m_pRenderPiece			= pLOD;
	Info.m_pInstance			= pInstance;
	Info.m_pTransforms			= pTransforms;
	Info.m_pHookData			= pHookData;

	//get the appropriate render style (or the backup one if none exists)
	Info.m_pRenderStyle			= NULL;
	if (!pInstance->GetRenderStyle(pPiece->m_iRenderStyle,(CRenderStyle**)(&Info.m_pRenderStyle)) || !Info.m_pRenderStyle) 
	{
		//failed to get the associated render style, get the default
		Info.m_pRenderStyle = g_RenderStateMgr.GetBackupRenderStyle(); 
	}

	uint32 nCurrTexture = 0;
	if(bTexture)
	{
		for(; nCurrTexture < pPiece->m_nNumTextures; nCurrTexture++)
		{
			if (pInstance->m_pSkins[pPiece->m_iTextures[nCurrTexture]] && 
				pInstance->m_pSkins[pPiece->m_iTextures[nCurrTexture]]->m_pRenderData) 
			{
				Info.m_TextureList[nCurrTexture] = pInstance->m_pSkins[pPiece->m_iTextures[nCurrTexture]]; 
			}
			else 
			{ 
				Info.m_TextureList[nCurrTexture] = NULL; 
			}
		}
	}

	//fill up any leftover slots with NULL pointers
	for (; nCurrTexture < MAX_PIECE_TEXTURES; ++nCurrTexture) 
	{
		Info.m_TextureList[nCurrTexture] = NULL; 
	}

	//alright, the info is complete, put it on the list
	LT_MEM_TRACK_ALLOC(m_cPieceList.push_back(Info), LT_MEM_TYPE_RENDERER);
}

//Renders all the queued pieces and flushes the list
void CRenderModelPieceList::RenderPieceList(float fAlpha)
{
	// Oh my god!  STL-Port let us down!  We have to use stable_sort, because
	// sort appears to have a problem sorting this list sometimes.
	//we need to sort the list first
	//std::sort(m_cPieceList.begin(), m_cPieceList.end());
	stable_sort(m_cPieceList.begin(), m_cPieceList.end());

	//make sure some stuff is setup
	StateSet ssLightEnable(D3DRS_LIGHTING, TRUE);
	PD3DDEVICE->SetRenderState(D3DRS_NORMALIZENORMALS, FALSE);

	//now that the list is sorted, we can run through and render

	//flag to keep track of when we are in really close and not
	bool bInReallyClose = false;
	CReallyCloseData ReallyCloseData;

	//the previous object's settings
	CDIModelDrawable*		pPrevRPiece			= NULL;
	CDeviceLightList*		pPrevLightList		= NULL;
	bool					bPrevPieceScaled	= false;

	LPDIRECT3DBASETEXTURE9	PrevTextureList[MAX_PIECE_TEXTURES];

	//save the depth bias so we can restore it later
	DWORD					nOldDepthBias;
	PD3DDEVICE->GetRenderState(D3DRS_DEPTHBIAS, &nOldDepthBias);

	//clear out the texture list
	memset(PrevTextureList, 0, sizeof(PrevTextureList));

	for(uint32 nCurrPiece = 0; nCurrPiece < m_cPieceList.size();)
	{
		//we now have our starting, we now need to expand out and see where this render style ends
		//(being careful to observe the really close switch)
		uint32 nStartPiece = nCurrPiece;

		CD3DRenderStyle* pCurrRenderStyle = m_cPieceList[nStartPiece].m_pRenderStyle;

		for(; nCurrPiece < m_cPieceList.size(); nCurrPiece++)
		{
			//see if we need to bail
			if( (m_cPieceList[nCurrPiece].m_pRenderStyle != pCurrRenderStyle) ||
				(m_cPieceList[nCurrPiece].m_bReallyClose != m_cPieceList[nStartPiece].m_bReallyClose))
			{
				break;
			}
		}

		//we may need to modify the render style to apply the alpha of the piece
		LightingMaterial OriginalLightMaterial;

		// Over-ride the color if the model requests it...
		if (fAlpha < 0.99f) 
		{
			if (pCurrRenderStyle->GetLightingMaterial(&OriginalLightMaterial)) 
			{
				LightingMaterial OverrideLightMaterial	= OriginalLightMaterial; 
				OverrideLightMaterial					= OriginalLightMaterial;
				OverrideLightMaterial.Ambient.a			= fAlpha * OriginalLightMaterial.Ambient.a;
				OverrideLightMaterial.Diffuse.a			= fAlpha * OriginalLightMaterial.Diffuse.a;
				OverrideLightMaterial.Specular.a		= fAlpha * OriginalLightMaterial.Specular.a;

				pCurrRenderStyle->SetLightingMaterial(OverrideLightMaterial); 
			}
		}

		RSD3DOptions rsD3DOptions;
		pCurrRenderStyle->GetDirect3D_Options(&rsD3DOptions);
		if(rsD3DOptions.bUseEffectShader)
		{
			//TODO
			bool bModifiedReallyCloseEnvMap = false;

			//setup all the texture and render style information
			//g_RenderStateMgr.SetRenderStyleStates(pCurrRenderStyle, nCurrPass);
			//g_RenderStateMgr.SetRenderStyleTextures(pCurrRenderStyle, nCurrPass, m_cPieceList[nStartPiece].m_TextureList);

			//keep track of stats
			IncFrameStat(eFS_ModelRender_RenderStyleSets, 1);
			IncFrameStat(eFS_ModelRender_TextureSets, 1);

			//update our texture list information since the render style set them
			memcpy(PrevTextureList, m_cPieceList[nStartPiece].m_TextureList, sizeof(PrevTextureList));

			//now we need to render each piece of this pass
			for(uint32 nRSPiece = nStartPiece; nRSPiece < nCurrPiece; nRSPiece++)
			{
				//cache the current piece
				SQueuedPiece& Piece = m_cPieceList[nRSPiece];

				//handle environment mapping on the PV
				bModifiedReallyCloseEnvMap = HandleReallyCloseEnvMapPanning(Piece.m_pInstance, pCurrRenderStyle, 0, bModifiedReallyCloseEnvMap);

				//see if we have crossed any boundaries. If we have we need to change the appropriate part				
				if(memcmp(PrevTextureList, Piece.m_TextureList, sizeof(PrevTextureList)))
				{
					//need to swap textures
					g_RenderStateMgr.SetRenderStyleTextures(pCurrRenderStyle, 0, Piece.m_TextureList);
					memcpy(PrevTextureList, Piece.m_TextureList, sizeof(PrevTextureList));
					IncFrameStat(eFS_ModelRender_TextureSets, 1);
				}

				if(!pPrevLightList || !pPrevLightList->IsSameAs(Piece.m_pLightList))
				{
					Piece.m_pLightList->InstallLightList();
					pPrevLightList = Piece.m_pLightList;
					IncFrameStat(eFS_ModelRender_LightingSets, 1);
				}

				if(bInReallyClose != Piece.m_bReallyClose)
				{
					d3d_SetReallyClose(&ReallyCloseData);
					bInReallyClose = Piece.m_bReallyClose;
					IncFrameStat(eFS_ModelRender_ReallyCloseSets, 1);
				}

				if(pPrevRPiece != Piece.m_pRenderPiece)
				{
					//end the old one
					EndRenderableModelPiece(pPrevRPiece);

					for (uint32 iTexStage = 0; iTexStage < 4; ++iTexStage)
					{
						d3d_SetTexture(NULL, iTexStage, eFS_ModelTexMemory);
					}

					//begin the new one
					BeginRenderableModelPiece(Piece.m_pRenderPiece, Piece.m_pTransforms, pCurrRenderStyle, -1, true);

					//save the new one
					pPrevRPiece = Piece.m_pRenderPiece;
				}

				//we also need to determine whether or not we need to scale the normals of this piece
				bool bPieceScaled = Piece.m_pInstance->IsScaled();
				if(bPieceScaled != bPrevPieceScaled)
				{
					PD3DDEVICE->SetRenderState(D3DRS_NORMALIZENORMALS, bPieceScaled ? TRUE : FALSE); 
					bPrevPieceScaled = bPieceScaled;

					IncFrameStat(eFS_ModelRender_ScaleSets, 1);
				}

				LTEffectImpl* _pEffect = (LTEffectImpl*)LTEffectShaderMgr::GetSingleton().GetEffectShader(rsD3DOptions.EffectShaderID);
				ID3DXEffect* pEffect = _pEffect->GetEffect();

				if(pEffect)
				{	
					// Send our vertex declaration to DirectX.
					_pEffect->UploadVertexDeclaration();

					// Set our textures!
					for (uint32 iTexStage = 0; iTexStage < 4; ++iTexStage)
					{
						if(Piece.m_TextureList[iTexStage])
						{		
							d3d_SetTexture(Piece.m_TextureList[iTexStage], iTexStage, eFS_ModelTexMemory);

							RTexture* pRTexture = (RTexture*)(Piece.m_TextureList[iTexStage])->m_pRenderData;
							if(pRTexture)
							{
								char szBuf[16];
								sprintf(szBuf, "texture%d", iTexStage);
								HRESULT hr = pEffect->SetTexture(szBuf, pRTexture->m_pD3DTexture); 
								if(FAILED(hr))
								{
									dsi_ConsolePrint("Failed to set texture (%s)", szBuf);
								}
							}
						}
					}
					IncFrameStat(eFS_ModelRender_TextureSets, 1);

					//Ask the game for variables...
					i_client_shell->OnEffectShaderSetParams((LTEffectShader*)_pEffect, pCurrRenderStyle, Piece.m_pInstance, LTShaderDeviceStateImp::GetSingleton());

					UINT cPasses;
					pEffect->Begin(&cPasses, 0);
					for(UINT iPass = 0; iPass < cPasses; iPass++)
					{	
						pEffect->BeginPass(iPass);

						//					BeginRenderableModelPiece(Piece.m_pRenderPiece, Piece.m_pTransforms, pCurrRenderStyle, -1);

						//ok, we can finally render our piece
						RenderModelPiece(Piece.m_pRenderPiece, Piece.m_pInstance, Piece.m_pTransforms, pCurrRenderStyle, -1, true);

						//					EndRenderableModelPiece(Piece.m_pRenderPiece);

						pEffect->EndPass();
					}
					pEffect->End();
					//ok, we can finally render our piece
					//RenderModelPiece(Piece.m_pRenderPiece, Piece.m_pInstance, Piece.m_pTransforms, pCurrRenderStyle, nCurrPass, true);
				}
			}

			//we should end any outstanding pieces being rendered so the changing of the pass won't mess them up
			EndRenderableModelPiece(pPrevRPiece);

			for (uint32 iTexStage = 0; iTexStage < 4; ++iTexStage)
			{
				d3d_SetTexture(NULL, iTexStage, eFS_ModelTexMemory);
			}

			pPrevRPiece = NULL;	
		}
		else
		{		
			//run through each pass of the current render style
			for(uint32 nCurrPass = 0; nCurrPass < pCurrRenderStyle->GetRenderPassCount(); nCurrPass++)
			{
				/*			if(g_CV_ZBiasModelRSPasses.m_Val)
				{
				if (nCurrPass != 0)
				{
				PD3DDEVICE->SetRenderState(D3DRS_SLOPESCALEDEPTHBIAS, F2DW(1.0f));
				}
				else
				{
				PD3DDEVICE->SetRenderState(D3DRS_SLOPESCALEDEPTHBIAS, F2DW(0.0f));
				}

				PD3DDEVICE->SetRenderState(D3DRS_DEPTHBIAS, F2DW(1.0f));
				}*/

				bool bModifiedReallyCloseEnvMap = false;

				//setup all the texture and render style information
				g_RenderStateMgr.SetRenderStyleStates(pCurrRenderStyle, nCurrPass);
				g_RenderStateMgr.SetRenderStyleTextures(pCurrRenderStyle, nCurrPass, m_cPieceList[nStartPiece].m_TextureList);

				//keep track of stats
				IncFrameStat(eFS_ModelRender_RenderStyleSets, 1);
				IncFrameStat(eFS_ModelRender_TextureSets, 1);

				//update our texture list information since the render style set them
				memcpy(PrevTextureList, m_cPieceList[nStartPiece].m_TextureList, sizeof(PrevTextureList));

				//now we need to render each piece of this pass
				for(uint32 nRSPiece = nStartPiece; nRSPiece < nCurrPiece; nRSPiece++)
				{
					//cache the current piece
					SQueuedPiece& Piece = m_cPieceList[nRSPiece];

					//handle environment mapping on the PV
					bModifiedReallyCloseEnvMap = HandleReallyCloseEnvMapPanning(Piece.m_pInstance, pCurrRenderStyle, nCurrPass, bModifiedReallyCloseEnvMap);

					//see if we have crossed any boundaries. If we have we need to change the appropriate part
					if(memcmp(PrevTextureList, Piece.m_TextureList, sizeof(PrevTextureList)))
					{
						//need to swap textures
						g_RenderStateMgr.SetRenderStyleTextures(pCurrRenderStyle, nCurrPass, Piece.m_TextureList);
						memcpy(PrevTextureList, Piece.m_TextureList, sizeof(PrevTextureList));
						IncFrameStat(eFS_ModelRender_TextureSets, 1);
					}

					if(!pPrevLightList || !pPrevLightList->IsSameAs(Piece.m_pLightList))
					{
						Piece.m_pLightList->InstallLightList();
						pPrevLightList = Piece.m_pLightList;
						IncFrameStat(eFS_ModelRender_LightingSets, 1);
					}

					if(bInReallyClose != Piece.m_bReallyClose)
					{
						d3d_SetReallyClose(&ReallyCloseData);
						bInReallyClose = Piece.m_bReallyClose;
						IncFrameStat(eFS_ModelRender_ReallyCloseSets, 1);
					}

					if(pPrevRPiece != Piece.m_pRenderPiece)
					{
						//end the old one
						EndRenderableModelPiece(pPrevRPiece);
						//begin the new one
						BeginRenderableModelPiece(Piece.m_pRenderPiece, NULL, pCurrRenderStyle, nCurrPass);

						//save the new one
						pPrevRPiece = Piece.m_pRenderPiece;
					}

					//we also need to determine whether or not we need to scale the normals of this piece
					bool bPieceScaled = Piece.m_pInstance->IsScaled();
					if(bPieceScaled != bPrevPieceScaled)
					{
						PD3DDEVICE->SetRenderState(D3DRS_NORMALIZENORMALS, bPieceScaled ? TRUE : FALSE); 
						bPrevPieceScaled = bPieceScaled;

						IncFrameStat(eFS_ModelRender_ScaleSets, 1);
					}

					//ok, we can finally render our piece
					RenderModelPiece(Piece.m_pRenderPiece, Piece.m_pInstance, Piece.m_pTransforms, pCurrRenderStyle, nCurrPass);
				}

				//we should end any outstanding pieces being rendered so the changing of the pass won't mess them up
				EndRenderableModelPiece(pPrevRPiece);
				pPrevRPiece = NULL;	
			}
		}

		//cleanup any changes we made to the render style
		if (fAlpha < 0.99f) 
		{
			pCurrRenderStyle->SetLightingMaterial(OriginalLightMaterial); 
		}
		
	}

	//we need to unset really close if we entered it
	if(bInReallyClose)
		d3d_UnsetReallyClose(&ReallyCloseData);

	//restore our transform to be the identity matrix
	const D3DMATRIX mIdentity = { 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f};
	g_RenderStateMgr.SetTransform(D3DTS_WORLDMATRIX(0),&mIdentity);

	//restore our Z bias
	/*	if (nOldZBias != 0)
	{
	PD3DDEVICE->SetRenderState(D3DRS_SLOPESCALEDEPTHBIAS, F2DW(1.0f));
	}
	else
	{
	PD3DDEVICE->SetRenderState(D3DRS_SLOPESCALEDEPTHBIAS, F2DW(0.0f));
	}*/

	PD3DDEVICE->SetRenderState(D3DRS_DEPTHBIAS, nOldDepthBias);

	// Need to reset every time till the render state mgr handles things...
	g_Device.SetDefaultRenderStates();		

	//we are done, clear out the lists
	m_cPieceList.clear();
}

//call this to free all memory associated with the piece list. This will require it to grow
//again, but is a good chance to clean up after some level that used a lot of models
//and caused it to grow too large
void CRenderModelPieceList::FreeListMemory()
{
	//standard STL trick to force vector to give up memory
	TPieceList cEmptyPiece;
	cEmptyPiece.swap(m_cPieceList);
}

//remaps all the render styles using the specified map
void CRenderModelPieceList::RemapRenderStyles(const CRenderStyleMap& Map)
{
	for(TPieceList::iterator it = m_cPieceList.begin(); it != m_cPieceList.end(); it++)
	{
		//see what this pieces render style maps to
		CD3DRenderStyle* pMapTo;
		
		if(it->m_pHookData->m_HookFlags & MHF_NOGLOW)
		{
			pMapTo = (CD3DRenderStyle*)Map.GetNoGlowRenderStyle();
		}
		else
		{
			pMapTo = (CD3DRenderStyle*)Map.MapRenderStyle(it->m_pRenderStyle);
		}		

		//see if we overrode it
		if(pMapTo)
		{
			it->m_pRenderStyle = pMapTo;
		}
	}
}

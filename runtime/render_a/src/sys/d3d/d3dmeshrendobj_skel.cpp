// d3dmeshrendobj_skel.cpp
#include "precompile.h"

#include "d3dmeshrendobj_skel.h"
#include "renderstruct.h"
#include "ltb.h"
#include "d3d_device.h"
#include "d3d_texture.h"
#include "d3d_renderstatemgr.h"
#include "d3d_draw.h"
#include "ltvertexshadermgr.h"
#include "ltpixelshadermgr.h"
#include "de_objects.h"
#include "ltshaderdevicestateimp.h"
#include "rendererconsolevars.h"
#include "LTEffectImpl.h"
#include "lteffectshadermgr.h"
#include <set>
#include <vector>



//IClientShell game client shell object.
#include "iclientshell.h"
static IClientShell *i_client_shell;
define_holder(IClientShell, i_client_shell);



CD3DSkelMesh::CD3DSkelMesh()
{
	Reset();
}

CD3DSkelMesh::~CD3DSkelMesh()
{
	FreeAll();
}

// ------------------------------------------------------------------------
// CalcUsedNodes()
// specifically find the nodes that are used by the mesh AND are the most
// distal nodes in the set. We don't need interior nodes since the xform evaulation
// paths start at the leaves.
// NOTE : t.f fix
// This algorithm is actually in the packer, its here for models that are
// version 21 or less. This should be removed once all models are version 22.
// ------------------------------------------------------------------------
void CD3DSkelMesh::CalcUsedNodes( Model *pModel )
{
	std::set<uint32> node_set ;
	std::vector<uint32> node_list ;

	for( uint32 iBoneSet = 0 ; iBoneSet < m_iBoneSetCount  ; iBoneSet++ )
	{
		for( uint32 iBoneCnt = 0 ; iBoneCnt < 4 ; iBoneCnt++ )
		{
			node_set.insert( (uint32)m_pBoneSetArray[iBoneSet].BoneSetArray[iBoneCnt]);
		}
	}

	std::set<uint32>::iterator set_it = node_set.begin();
	// create set of terminal nodes for finding paths.
	for( ; set_it != node_set.end() ; set_it++ )
	{
		uint32 iNode  = *set_it ;
		if( iNode == 255 ) continue ;
		ModelNode *pModelNode = pModel->GetNode(iNode);

		// check if children are in the set.
		// if none of the children are in the set, add the node to the final list.
		uint32 nChildren = pModelNode->m_Children.GetSize();
		uint32 iChild ;
		bool   IsTerminalNode = true ;
		for( iChild = 0 ; iChild < nChildren ; iChild++ )
		{
			// if we find a child that is in the set, quit the search.
			if( node_set.find( pModelNode->m_Children[iChild]->m_NodeIndex ) != node_set.end() )
			{
				IsTerminalNode = false  ;
				continue ;
			}
		}

		// if all the children didn't have a parent in the mesh's bone list, add this node
		// as a terminal node.
		if(IsTerminalNode)
		{
			node_list.push_back(*set_it);
		}
	}

	// transfer the new information from here to the renderobject.
	CreateUsedNodeList(node_list.size());
	for( uint32 iNodeCnt =0 ; iNodeCnt < node_list.size() ; iNodeCnt++ )
	{
		m_pUsedNodeList[iNodeCnt] = node_list[iNodeCnt];
	}
}

void CD3DSkelMesh::Reset()
{
	m_VBController.Reset();
	m_iMaxBonesPerVert	= 0;
	m_iMaxBonesPerTri	= 0;
	m_iVertCount		= 0;
	m_iPolyCount		= 0;
	m_eRenderMethod		= eD3DRenderDirect;
	m_iBoneSetCount		= 0;
	m_pBoneSetArray		= NULL;
	m_VertType			= eNO_WORLD_BLENDS;
	m_bSWVertProcessing	= ((g_Device.GetDeviceCaps()->DevCaps & D3DDEVCAPS_HWTRANSFORMANDLIGHT) == 0) ? true : false;


	m_bSWVSBuffers = false;

	// Use software processing for shaders ? 
	if ( g_Device.GetDeviceCaps()->VertexShaderVersion < D3DVS_VERSION(1,1) )
		m_bSWVSBuffers = true;

	// in case software has been forced 
	if ( g_CV_ForceSWVertProcess )
		m_bSWVertProcessing = true;

	m_pIndexData		= NULL;
	m_bNonFixPipeData	= false;
	m_bReIndexedBones	= false;
	m_pReIndexedBoneList= NULL;

	for (uint32 i = 0; i < 4; ++i)
		m_pVertData[i] = NULL;
}

void CD3DSkelMesh::FreeAll()
{
	m_VBController.FreeAll();
	if (m_pBoneSetArray)
	{
		delete[] m_pBoneSetArray;
		m_pBoneSetArray = NULL;
	}
	if (m_pIndexData)
	{
		delete[] m_pIndexData;
		m_pIndexData = NULL;
	}
	if (m_pReIndexedBoneList)
	{
		delete[] m_pReIndexedBoneList;
		m_pReIndexedBoneList = NULL;
	}
	for (uint32 i = 0; i < 4; ++i)
	{
		if (m_pVertData[i])
		{
			delete[] m_pVertData[i];
			m_pVertData[i] = NULL;
		}
	}

	Reset();
}

bool CD3DSkelMesh::Load(ILTStream& File, LTB_Header& LTBHeader)
{
	if (LTBHeader.m_iFileType != LTB_D3D_MODEL_FILE)
	{
		OutputDebugString("Error: Wrong file type in CD3DSkelMesh::Load\n");
		return false;
	}
	if (LTBHeader.m_iVersion  != CD3D_LTB_LOAD_VERSION)
	{
		OutputDebugString("Error: Wrong file version in CD3DSkelMesh::Load\n");
		return false;
	}

	// Read in the basics...
	uint32 iObjSize; File.Read(&iObjSize,sizeof(iObjSize));
	File.Read(&m_iVertCount,sizeof(m_iVertCount));
	File.Read(&m_iPolyCount,sizeof(m_iPolyCount));
	File.Read(&m_iMaxBonesPerTri,sizeof(m_iMaxBonesPerTri));
	File.Read(&m_iMaxBonesPerVert,sizeof(m_iMaxBonesPerVert));
	File.Read(&m_bReIndexedBones,sizeof(m_bReIndexedBones));
	File.Read(&m_VertStreamFlags[0],sizeof(uint32)*4);

	// Are we using Matrix Palettes...
	bool bUseMatrixPalettes;
	File.Read(&bUseMatrixPalettes,sizeof(bUseMatrixPalettes));

	if (bUseMatrixPalettes)
	{
		m_eRenderMethod = eD3DRenderMatrixPalettes;
		return Load_MP(File);
	}
	else
	{
		m_eRenderMethod = eD3DRenderDirect;
		return Load_RD(File);
	}
}

bool CD3DSkelMesh::Load_RD(ILTStream& File)
{
	// What type of Vert do we need?
	switch (m_iMaxBonesPerTri)
	{
	case 1  : m_VertType = eNO_WORLD_BLENDS; break;
	case 2  : m_VertType = eNONINDEXED_B1; break;
	case 3  : m_VertType = eNONINDEXED_B2; break;
	case 4  : m_VertType = eNONINDEXED_B3; break;
	default : assert(0); return false;
	}

	// Read in our Verts...
	for (uint32 i=0;i<4;++i)
	{
		if (!m_VertStreamFlags[i]) continue;

		uint32 iVertexSize	= 0;									// Figure out the vertex size...
		uint32 iVertFlags	= 0;
		uint32 iUVSets		= 0;
		GetVertexFlags_and_Size(m_VertType,m_VertStreamFlags[i],iVertFlags,iVertexSize,iUVSets,m_bNonFixPipeData);

		uint32 iSize = iVertexSize * m_iVertCount;					// Alloc the VertData...
		LT_MEM_TRACK_ALLOC(m_pVertData[i] = new uint8[iSize],LT_MEM_TYPE_RENDERER);
		File.Read(m_pVertData[i],iSize);
	}

	// Read in pIndexList...
	LT_MEM_TRACK_ALLOC(m_pIndexData = new uint8[sizeof(uint16) * m_iPolyCount * 3],LT_MEM_TYPE_RENDERER);
	File.Read(m_pIndexData,sizeof(uint16) * m_iPolyCount * 3);

	// Allocate and read in the BoneSets...
	File.Read(&m_iBoneSetCount,sizeof(m_iBoneSetCount));
	LT_MEM_TRACK_ALLOC(m_pBoneSetArray = new BoneSetListItem[m_iBoneSetCount],LT_MEM_TYPE_RENDERER);
	if (!m_pBoneSetArray)
		return false;
	File.Read(m_pBoneSetArray,sizeof(BoneSetListItem)*m_iBoneSetCount);

	// Create the VBs and stuff...
	ReCreateObject();

	return true;
}

bool CD3DSkelMesh::Load_MP(ILTStream& File)
{
	// Read in out Min/Max Bones (effecting this guy)...
	File.Read(&m_iMinBone,sizeof(m_iMinBone));
	File.Read(&m_iMaxBone,sizeof(m_iMaxBone));

	// What type of Vert do we need?
	switch (m_iMaxBonesPerVert)
	{
	case 2  : m_VertType = eINDEXED_B1; break;
	case 3  : m_VertType = eINDEXED_B2; break;
	case 4  : m_VertType = eINDEXED_B3; break;
	default : assert(0); return false;
	}

	// If we are using re-indexed bones, read them in...
	if (m_bReIndexedBones)
	{
		uint32 iBoneCount = 0;
		File.Read(&iBoneCount,sizeof(iBoneCount));
		assert(iBoneCount < 10000 && "Crazy bone count, checked your packed model format.");
		LT_MEM_TRACK_ALLOC(m_pReIndexedBoneList = new uint32[iBoneCount],LT_MEM_TYPE_RENDERER);
		File.Read(m_pReIndexedBoneList,sizeof(uint32)*iBoneCount);
	}

	// Read in our Verts...
	for (uint32 i=0;i<4;++i)
	{
		if (!m_VertStreamFlags[i])
			continue;

		uint32 iVertexSize	= 0;									// Figure out the vertex size...
		uint32 iVertFlags	= 0;
		uint32 iUVSets		= 0;
		GetVertexFlags_and_Size(m_VertType,m_VertStreamFlags[i],iVertFlags,iVertexSize,iUVSets,m_bNonFixPipeData);

		uint32 iSize = iVertexSize * m_iVertCount;					// Alloc the VertData...
		LT_MEM_TRACK_ALLOC(m_pVertData[i] = new uint8[iSize],LT_MEM_TYPE_RENDERER);
		File.Read(m_pVertData[i],iSize);
	}

	// Read in pIndexList...
	LT_MEM_TRACK_ALLOC(m_pIndexData = new uint8[sizeof(uint16) * m_iPolyCount * 3],LT_MEM_TYPE_RENDERER);
	File.Read(m_pIndexData,sizeof(uint16) * m_iPolyCount * 3);

	// Create the VBs and stuff...
	ReCreateObject();

	return true;
}

// Create the VBs and stuff from our sys mem copies...
void CD3DSkelMesh::ReCreateObject()
{
	// Create our VB...
	for (uint32 i=0;i<4;++i)
	{
		if (!m_VertStreamFlags[i])
			continue;

		if (!m_VBController.CreateStream(i, m_iVertCount, m_VertStreamFlags[i], m_VertType, false, true, m_bSWVertProcessing))
		{
			FreeAll();
			return;
		}
	}

	if (!m_VBController.CreateIndexBuffer(m_iPolyCount*3,false,true,m_bSWVertProcessing))
	{
		FreeAll();
		return;
	}

	// Read in our Verts...
	for (int i=0;i<4;++i)
	{
		if (!m_VertStreamFlags[i])
			continue;

		m_VBController.Lock((VertexBufferController::VB_TYPE)(VertexBufferController::eVERTSTREAM0 + i),false);

		uint8* pVertData = (uint8*)m_VBController.getVertexData(i);
		uint32 iSize = m_VBController.getVertexSize(i) * m_iVertCount;
		memcpy(pVertData,m_pVertData[i],iSize);

		m_VBController.UnLock((VertexBufferController::VB_TYPE)(VertexBufferController::eVERTSTREAM0 + i));
	}

	// Read in pIndexList...
	m_VBController.Lock(VertexBufferController::eINDEX,false);
	memcpy(m_VBController.getIndexData(),m_pIndexData,sizeof(uint16) * m_iPolyCount * 3);
	m_VBController.UnLock(VertexBufferController::eINDEX);
}

// We're loosing focus, free the stuff...
void CD3DSkelMesh::FreeDeviceObjects()
{
	m_VBController.FreeAll();										// Free our VB...
}

inline int32 CD3DSkelMesh::SetTransformsToBoneSet(BoneSetListItem* pBoneSet,D3DMATRIX* pTransforms, int32 nNumMatrices)
{
	int32 iCurrBone;
	for (iCurrBone=0; iCurrBone < 4; ++iCurrBone) 
	{
		if (pBoneSet->BoneSetArray[iCurrBone] == 0xFF)
		{
			/*
			D3DXMATRIX mMat;
			//D3DXMatrixIdentity(&mMat);
			ZeroMemory(&mMat, sizeof(D3DXMATRIX));
			g_RenderStateMgr.SetTransform(D3DTS_WORLDMATRIX(iCurrBone),&mMat);
			continue;
			*/
			
			break;
		}

		g_RenderStateMgr.SetTransform(D3DTS_WORLDMATRIX(iCurrBone),&pTransforms[pBoneSet->BoneSetArray[iCurrBone]]);
	}



	if(nNumMatrices != iCurrBone)
	{
		if( g_CV_Use0WeightsForDisable )
		{
			// ATI requires 0 weights instead of disable
			switch (iCurrBone) 
			{
				case 1: 
					PD3DDEVICE->SetRenderState(D3DRS_VERTEXBLEND,D3DVBF_0WEIGHTS); 
					break;
				case 2: 
					PD3DDEVICE->SetRenderState(D3DRS_VERTEXBLEND,D3DVBF_1WEIGHTS); 
					break;
				case 3: 
					PD3DDEVICE->SetRenderState(D3DRS_VERTEXBLEND,D3DVBF_2WEIGHTS); 
					break;
				case 4: 
					PD3DDEVICE->SetRenderState(D3DRS_VERTEXBLEND,D3DVBF_3WEIGHTS); 
					break;
				default: 
					PD3DDEVICE->SetRenderState(D3DRS_VERTEXBLEND,D3DVBF_DISABLE); 
					ASSERT(0);
					break;
			}
		}
		else
		{
			// but NVIDIA uses disable instead of 0 weights (only on 440MX)
			switch (iCurrBone)
			{
				case 2: 
					PD3DDEVICE->SetRenderState(D3DRS_VERTEXBLEND,D3DVBF_1WEIGHTS); 
					break;
				case 3: 
					PD3DDEVICE->SetRenderState(D3DRS_VERTEXBLEND,D3DVBF_2WEIGHTS); 
					break;
				case 4: 
					PD3DDEVICE->SetRenderState(D3DRS_VERTEXBLEND,D3DVBF_3WEIGHTS); 
					break;
				default: 
					PD3DDEVICE->SetRenderState(D3DRS_VERTEXBLEND,D3DVBF_DISABLE);
					break;
			}
		}
	}

	return (iCurrBone-1);
}

inline uint32 CD3DSkelMesh::SetMatrixPalette(uint32 MinBone,uint32 MaxBone,D3DMATRIX* pTransforms)
{
	for (uint32 i=MinBone;i<=MaxBone;++i)
	{
		if (m_bReIndexedBones)
		{
			g_RenderStateMgr.SetTransform(D3DTS_WORLDMATRIX(i),&pTransforms[m_pReIndexedBoneList[i]]);
		}
		else
		{
			g_RenderStateMgr.SetTransform(D3DTS_WORLDMATRIX(i),&pTransforms[i]);
		}
	}

	switch (m_iMaxBonesPerVert)
	{
		case 2 : 
			{
				PD3DDEVICE->SetRenderState(D3DRS_VERTEXBLEND,D3DVBF_1WEIGHTS); 
				break;
			}
		case 3 :
			{
				PD3DDEVICE->SetRenderState(D3DRS_VERTEXBLEND,D3DVBF_2WEIGHTS); 
				break;
			}
		case 4 : 
			{
				PD3DDEVICE->SetRenderState(D3DRS_VERTEXBLEND,D3DVBF_3WEIGHTS); 
				break;
			}
		default: 
			{
				PD3DDEVICE->SetRenderState(D3DRS_VERTEXBLEND,D3DVBF_DISABLE);
			}
	}

	return (MaxBone-MinBone);
}

// THIS Function should be removed when we go to the full render object implementation - it's
//	temporary on the path to full render objects. The D3D pipe render model path call this guy to do
//	the transform and lighting stuff.
void CD3DSkelMesh::Render(ModelInstance *pInstance, D3DMATRIX* pD3DTransforms, CD3DRenderStyle* pRenderStyle, uint32 iRenderPass)
{
	switch (m_eRenderMethod)
	{
		case eD3DRenderDirect :
		{	// We need to do the bone walk, but we can render direct (they've been pre-processed into triangle group/bone group order)...
			uint32 iCurrentPolyIndex = 0;
			int32 nNumActiveBones = -1;
			for( int32 iBoneSet = 0; (iBoneSet < (int32)m_iBoneSetCount) ; ++iBoneSet )
			{
				BoneSetListItem* pBoneSet = &m_pBoneSetArray[iBoneSet];
				nNumActiveBones = SetTransformsToBoneSet(pBoneSet,pD3DTransforms, nNumActiveBones);

				// Set the vertex shader constants.
				if (m_pVertexShader != NULL)
				{
					// Let the client set some constants.
					if (NULL != i_client_shell)
					{
						i_client_shell->OnVertexShaderSetConstants(m_pVertexShader, iRenderPass, pRenderStyle, pInstance,
																   LTShaderDeviceStateImp::GetSingleton());
					}

					// Send the constants to the video card.
					LTVertexShaderMgr::GetSingleton().SetVertexShaderConstants(m_pVertexShader);
				}

				// Set the pixel shader constants.
				if (m_pPixelShader != NULL)
				{
					// Let the client set some constants.
					if (NULL != i_client_shell)
					{
						i_client_shell->OnPixelShaderSetConstants(m_pPixelShader, iRenderPass, pRenderStyle, pInstance,
																  LTShaderDeviceStateImp::GetSingleton());
					}

					// Send the constants to the video card.
					LTPixelShaderMgr::GetSingleton().SetPixelShaderConstants(m_pPixelShader);
				}

				RSD3DOptions rsD3DOptions;
				pRenderStyle->GetDirect3D_Options(&rsD3DOptions);
				if(rsD3DOptions.bUseEffectShader)
				{
					LTEffectImpl* _pEffect = (LTEffectImpl*)LTEffectShaderMgr::GetSingleton().GetEffectShader(rsD3DOptions.EffectShaderID);
					ID3DXEffect* pEffect = _pEffect->GetEffect();
				
					if(pEffect)
					{
						i_client_shell->OnEffectShaderSetParams((LTEffectShader*)_pEffect, pRenderStyle, pInstance, LTShaderDeviceStateImp::GetSingleton());
						pEffect->SetInt("BoneCount", nNumActiveBones);
						pEffect->CommitChanges();
					}
				
				}

				m_VBController.Render(	pBoneSet->iFirstVertIndex,
										iCurrentPolyIndex,
										pBoneSet->iVertCount,
										(pBoneSet->iIndexIntoIndexBuff - iCurrentPolyIndex)/3);



				iCurrentPolyIndex = pBoneSet->iIndexIntoIndexBuff;

				IncFrameStat(eFS_ModelRender_NumSkeletalRenderObjects, 1);
			}

			break;
		}
		case eD3DRenderMatrixPalettes :
		{
			uint32 nNumActiveBones = SetMatrixPalette(m_iMinBone,m_iMaxBone,pD3DTransforms);

			// Set the vertex shader constants.
			if (m_pVertexShader != NULL)
			{
				// Let the client set some constants.
				if (NULL != i_client_shell)
				{
					i_client_shell->OnVertexShaderSetConstants(m_pVertexShader, iRenderPass, pRenderStyle, pInstance,
															   LTShaderDeviceStateImp::GetSingleton());
				}

				// Send the constants to the video card.
				LTVertexShaderMgr::GetSingleton().SetVertexShaderConstants(m_pVertexShader);
			}

			// Set the pixel shader constants.
			if (m_pPixelShader != NULL)
			{
				// Let the client set some constants.
				if (NULL != i_client_shell)
				{
					i_client_shell->OnPixelShaderSetConstants(m_pPixelShader, iRenderPass, pRenderStyle, pInstance,
															  LTShaderDeviceStateImp::GetSingleton());
				}

				// Send the constants to the video card.
				LTPixelShaderMgr::GetSingleton().SetPixelShaderConstants(m_pPixelShader);
			}

			RSD3DOptions rsD3DOptions;
			pRenderStyle->GetDirect3D_Options(&rsD3DOptions);
			if(rsD3DOptions.bUseEffectShader)
			{
				LTEffectImpl* _pEffect = (LTEffectImpl*)LTEffectShaderMgr::GetSingleton().GetEffectShader(rsD3DOptions.EffectShaderID);
				ID3DXEffect* pEffect = _pEffect->GetEffect();

				if(pEffect)
				{
					i_client_shell->OnEffectShaderSetParams((LTEffectShader*)_pEffect, pRenderStyle, pInstance, LTShaderDeviceStateImp::GetSingleton());
					pEffect->SetInt("BoneCount", nNumActiveBones);
					pEffect->CommitChanges();
				}

			}

			m_VBController.Render(0,0,m_iVertCount,m_iPolyCount);

			break;
		}
	}
}

void CD3DSkelMesh::BeginRender(D3DMATRIX* pD3DTransforms, CD3DRenderStyle* pRenderStyle, uint32 iRenderPass)
{

// [dlj] remove this because DX9 doesn't have this bug

	// DX8 has bug with table fog with blended meshes...
//	PD3DDEVICE->GetRenderState(D3DRS_FOGTABLEMODE, &m_nPrevFogTableMode);
//	PD3DDEVICE->SetRenderState(D3DRS_FOGTABLEMODE, D3DFOG_NONE);
//	PD3DDEVICE->GetRenderState(D3DRS_FOGVERTEXMODE, &m_nPrevFogVertexMode);
//	PD3DDEVICE->SetRenderState(D3DRS_FOGVERTEXMODE, D3DFOG_LINEAR);

	// Do we need to do software vert processing...
	bool bSoftwareProcessing = m_bSWVertProcessing;

	// Check if we need to do software processing...
	switch (m_eRenderMethod)
	{
	case eD3DRenderDirect			:
		if (m_iMaxBonesPerTri > g_Device.GetDeviceCaps()->MaxVertexBlendMatrices)
		{
			bSoftwareProcessing = true;
		}
		break;
	case eD3DRenderMatrixPalettes	:
		// Note: I am multiplying by two because the spec sais, if you're doing normals as well, it's half of the cap sais...
		if ((m_iMaxBone-m_iMinBone)*2+1 > g_Device.GetDeviceCaps()->MaxVertexBlendMatrixIndex)
		{
			bSoftwareProcessing = true;
		}
		break;
	}

	// If not already software vertex processing then set 
	if (!m_bSWVertProcessing && bSoftwareProcessing)
	{
		m_bSWVertProcessing = true;
		FreeDeviceObjects();
		ReCreateObject();
	}


	// If this pass has a vertex shader, use it.
	RSD3DRenderPass *pPass = pRenderStyle->GetRenderPass_D3DOptions(iRenderPass);
	if (NULL != pPass &&
	    pPass->bUseVertexShader &&
		pPass->VertexShaderID != LTVertexShader::VERTEXSHADER_INVALID)
	{
		if ( m_bSWVSBuffers && !m_bSWVertProcessing )
		{
			m_bSWVertProcessing = true;
			FreeDeviceObjects();
			ReCreateObject();
		}

		// Store the pointer to the actual shader during rendering.
		m_pVertexShader = LTVertexShaderMgr::GetSingleton().GetVertexShader(pPass->VertexShaderID);
		if (m_pVertexShader != NULL)
		{
			// Install the shader.
			if (!LTVertexShaderMgr::GetSingleton().InstallVertexShader(m_pVertexShader))
			{
				m_pVertexShader = NULL;
				return;
			}
		}
	}
	else if (!m_VBController.getVertexFormat(0) || m_bNonFixPipeData)
	{

		RSD3DOptions rsD3DOptions;
		pRenderStyle->GetDirect3D_Options(&rsD3DOptions);
		if(!rsD3DOptions.bUseEffectShader)
		{
			return; // This is a non fixed function pipe VB - bail out...
		}

		//return;				// This is a non fixed function pipe VB - bail out...
	}
	else if (FAILED(g_RenderStateMgr.SetVertexShader(m_VBController.getVertexFormat(0))))
	{
		return;
	}

	// If this pass has a pixel shader, use it.
	if (NULL != pPass &&
	    pPass->bUsePixelShader &&
		pPass->PixelShaderID != LTPixelShader::PIXELSHADER_INVALID)
	{
		// Store the pointer to the actual shader during rendering.
		m_pPixelShader = LTPixelShaderMgr::GetSingleton().GetPixelShader(pPass->PixelShaderID);
		if (m_pPixelShader != NULL)
		{
			// Install the shader.
			if (!LTPixelShaderMgr::GetSingleton().InstallPixelShader(m_pPixelShader))
			{
				m_pPixelShader = NULL;
				return;
			}
		}
	}


	// We need software processing 
	if(m_bSWVertProcessing)
	{
		PD3DDEVICE->SetSoftwareVertexProcessing(TRUE);
	}


	m_VBController.SetStreamSources();

	if(m_eRenderMethod == eD3DRenderMatrixPalettes)
	{
		PD3DDEVICE->SetRenderState(D3DRS_INDEXEDVERTEXBLENDENABLE, TRUE);
	}
}

void CD3DSkelMesh::EndRender()
{
	if(m_eRenderMethod == eD3DRenderMatrixPalettes)
	{
		PD3DDEVICE->SetRenderState(D3DRS_INDEXEDVERTEXBLENDENABLE, FALSE);
	}

	if ( m_bSWVertProcessing )
	{
		// If we are running with hardware then turn back on hardware processing
		if ( (g_Device.GetDeviceCaps()->DevCaps & D3DDEVCAPS_HWTRANSFORMANDLIGHT) )
		{
		
			PD3DDEVICE->SetSoftwareVertexProcessing(FALSE);
		}
	}

	PD3DDEVICE->SetRenderState(D3DRS_VERTEXBLEND,D3DVBF_DISABLE);

// [dlj] remove this because DX9 doesn't have this bug
//	PD3DDEVICE->SetRenderState(D3DRS_FOGTABLEMODE, m_nPrevFogTableMode);
//	PD3DDEVICE->SetRenderState(D3DRS_FOGVERTEXMODE, m_nPrevFogVertexMode);

	PD3DDEVICE->SetStreamSource(0, 0, 0, 0);
	PD3DDEVICE->SetIndices(0);

	// Uninstall the vertex shader.
	if (NULL != m_pVertexShader)
	{
		LTVertexShaderMgr::GetSingleton().UninstallVertexShader();
		m_pVertexShader = NULL;
	}

	// Uninstall the pixel shader.
	if (NULL != m_pPixelShader)
	{
		LTPixelShaderMgr::GetSingleton().UninstallPixelShader();
		m_pPixelShader = NULL;
	}


}


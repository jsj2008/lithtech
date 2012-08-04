
//
//
// D3D-specific sky drawing code.
//
//

#include "precompile.h"
#include "common_stuff.h"
#include "renderstruct.h"
#include "3d_ops.h"
#include "common_draw.h"
#include "drawsky.h"

#include "d3d_renderworld.h"

extern void d3d_DrawPolyGrid(const ViewParams &Params, LTObject *pGrid);
extern void d3d_DrawSprite(const ViewParams &Params, LTObject *pObj);

void d3d_DrawSkyObjects(const ViewParams& SkyParams)
{
/* JJH - this viewport was calculated assuming a screen resolution render target.
		 second camera viewports need different settings, so this was
		 causing the skybox to draw incorrectly in alternate cameras.
		 if you uncomment this, make sure to also uncomment the setviewport at
		 the end.

	//preserve our old viewport
	D3DVIEWPORT9 cOldViewport;
	PD3DDEVICE->GetViewport(&cOldViewport);

	//setup our new viewport that matches our sky dimensions
	D3DVIEWPORT9 cViewPort;
	cViewPort.X			= SkyParams.m_Rect.left;
	cViewPort.Y			= SkyParams.m_Rect.top;
	cViewPort.Width		= SkyParams.m_Rect.right - SkyParams.m_Rect.left;
	cViewPort.Height	= SkyParams.m_Rect.bottom - SkyParams.m_Rect.top;
	cViewPort.MinZ		= 0.0f;
	cViewPort.MaxZ		= 1.0f;
	PD3DDEVICE->SetViewport(&cViewPort);*/
	
	//disable reading/writing to the Z buffer
	StateSet ssZWrite(D3DRS_ZWRITEENABLE, FALSE);
	StateSet ssZRead(D3DRS_ZENABLE, D3DZB_FALSE);

	// Set the fog distances..
	StateSet ssFogStart(D3DRS_FOGSTART, *((uint32*)&g_CV_SkyFogNearZ.m_Val));
	StateSet ssFogEnd(D3DRS_FOGEND, *((uint32*)&g_CV_SkyFogFarZ.m_Val));

	//Preserve the old fog state so it can be properly restored
	uint32 oldFogEnable;
	D3D_CALL(PD3DDEVICE->GetRenderState(D3DRS_FOGENABLE, (unsigned long *) &oldFogEnable));

	// Disable SPMT.
	StageStateSet tssColorOp1(1, D3DTSS_COLOROP, D3DTOP_DISABLE);

	for(uint32 i = 0; i < (uint32)g_pSceneDesc->m_nSkyObjects; i++)
	{
		LTObject *pSkyObject = g_pSceneDesc->m_SkyObjects[i];
		if (pSkyObject->m_Flags & FLAG_VISIBLE) 
		{
			if ((pSkyObject->m_ObjectType == OT_WORLDMODEL) && g_CV_DrawWorldModels.m_Val) 
			{
				if ((pSkyObject->m_Flags & FLAG_FOGDISABLE) && oldFogEnable) 
				{
					D3D_CALL(PD3DDEVICE->SetRenderState(D3DRS_FOGENABLE, FALSE)); 
				}
				
				// Setup translucency states for it.
				if (pSkyObject->IsTranslucent()) 
				{
					d3d_SetTranslucentObjectStates(!!(pSkyObject->m_Flags2 & FLAG2_ADDITIVE)); 
				}
				else 
				{
					d3d_UnsetTranslucentObjectStates(FALSE); 
				}

				if (g_Device.m_pRenderWorld)
				{
					WorldModelInstance *pInstance = (WorldModelInstance*)pSkyObject;
					CD3D_RenderWorld *pWorldModel = g_Device.m_pRenderWorld->FindWorldModel(pInstance->m_pOriginalBsp->m_WorldName);
					if (pWorldModel)
					{
						pWorldModel->Draw(SkyParams, true);
					}
				}

				D3D_CALL(PD3DDEVICE->SetRenderState(D3DRS_FOGENABLE, oldFogEnable));
			}
			else if((pSkyObject->m_ObjectType == OT_POLYGRID) && g_CV_DrawPolyGrids.m_Val) 
			{
				// Set the states for it.
				if (pSkyObject->IsTranslucent())  
				{
					d3d_SetTranslucentObjectStates(); 
				}
				else 
				{
					d3d_UnsetTranslucentObjectStates(FALSE); 
				}

				d3d_DrawPolyGrid(SkyParams, pSkyObject); 
			}
			else if((pSkyObject->m_ObjectType == OT_SPRITE) && g_CV_DrawSprites.m_Val) 
			{
				d3d_SetTranslucentObjectStates();
				d3d_DrawSprite(SkyParams, pSkyObject); 
			} 
		}
	}

	// Unset translucent stuff.
	d3d_UnsetTranslucentObjectStates(FALSE);

//	PD3DDEVICE->SetViewport(&cOldViewport);
}


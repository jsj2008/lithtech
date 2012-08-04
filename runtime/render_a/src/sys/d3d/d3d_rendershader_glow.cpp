#include "precompile.h"
#include "d3d_rendershader_glow.h"
#include "common_draw.h"
#include "d3d_texture.h"
#include "memstats_world.h"
#include "d3d_renderblock.h"
#include "texturescriptinstance.h"

//indicates that a new shader is being started
bool CRenderShader_Glow::BeginNewShader(uint32 nFVF, uint32 nVertSize, IDirect3DIndexBuffer9* pIB, IDirect3DVertexBuffer9* pVB)
{
	// Set up our call
	if (pIB)
		PD3DDEVICE->SetIndices(pIB);
	PD3DDEVICE->SetVertexShader(NULL);
	PD3DDEVICE->SetFVF(nFVF);
	if (pVB)
		PD3DDEVICE->SetStreamSource(0, pVB, 0, nVertSize);

	return true;
}

//draws a shader in just pitch black
void CRenderShader_Glow::DrawGlow(SharedTexture* pTexture, CTextureScriptInstance* pTexEffect, ETextureScriptChannel eChannel, uint32 nVertStart, uint32 nVertCount, uint32 nIndexStart, uint32 nTriCount)
{
	//setup our render states so that it will just be pitch black
	StateSet ss0(D3DRS_TEXTUREFACTOR, 0x00000000);

	StageStateSet tss00(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
	StageStateSet tss01(0, D3DTSS_COLORARG2, D3DTA_TFACTOR);
	StageStateSet tss02(0, D3DTSS_COLOROP, D3DTOP_BLENDTEXTUREALPHA);

	StageStateSet tss03(0, D3DTSS_ALPHAARG1, D3DTA_DIFFUSE);
	StageStateSet tss04(0, D3DTSS_ALPHAARG2, D3DTA_TEXTURE);
	StageStateSet tss05(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);

	StageStateSet tss10(1, D3DTSS_COLOROP, D3DTOP_DISABLE);
	StageStateSet tss11(1, D3DTSS_ALPHAOP, D3DTOP_DISABLE);

	//install the texture
	d3d_SetTexture(pTexture, 0, eFS_WorldBaseTexMemory);

	//setup the texture script if appropriate
	if(pTexEffect)
		pTexEffect->Install(1, eChannel);

	//now render away
	PD3DDEVICE->DrawIndexedPrimitive(	D3DPT_TRIANGLELIST, 
										0, nVertStart, nVertCount,
										nIndexStart, nTriCount);

	//remove the texture script
	if(pTexEffect)
		pTexEffect->Uninstall(1, eChannel);

	//try to not leak
	d3d_DisableTexture(0);
}


//draws a shader in just pitch black
void CRenderShader_Glow::DrawBlack(uint32 nVertStart, uint32 nVertCount, uint32 nIndexStart, uint32 nTriCount)
{
	//setup our render states so that it will just be pitch black
	StateSet ss0(D3DRS_TEXTUREFACTOR, 0x00000000);

	StageStateSet tss00(0, D3DTSS_COLORARG1, D3DTA_TFACTOR);
	StageStateSet tss01(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);

	StageStateSet tss02(0, D3DTSS_ALPHAARG1, D3DTA_DIFFUSE);
	StageStateSet tss03(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);

	StageStateSet tss10(1, D3DTSS_COLOROP, D3DTOP_DISABLE);
	StageStateSet tss11(1, D3DTSS_ALPHAOP, D3DTOP_DISABLE);

	//now render away
	PD3DDEVICE->DrawIndexedPrimitive(	D3DPT_TRIANGLELIST, 
										0, nVertStart, nVertCount,
										nIndexStart, nTriCount);
}


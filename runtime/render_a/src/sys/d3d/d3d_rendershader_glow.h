//////////////////////////////////////////////////////////////////////////////
// Dynamic glow sub-shader

#ifndef __D3D_RENDERSHADER_GLOW_H__
#define __D3D_RENDERSHADER_GLOW_H__

#ifndef __TEXTURESCRIPTINSTANCE_H__
#	include "texturescriptinstance.h"
#endif


class SharedTexture;

class CRenderShader_Glow
{
public:

	//indicates that a new shader is being started
	static bool	BeginNewShader(uint32 nFVF, uint32 nVertSize, IDirect3DIndexBuffer9* pIB, IDirect3DVertexBuffer9* pVB);

	//draw the section with black except for where the passed in texture has alpha values in which case
	//it will modulate the texture by that value
	static void DrawGlow(SharedTexture* pTexture, CTextureScriptInstance* pTexEffect, ETextureScriptChannel eChannel, uint32 nVertStart, uint32 nVertCount, uint32 nIndexStart, uint32 nTriCount);

	//draws a shader in just pitch black
	static void	DrawBlack(uint32 nVertStart, uint32 nVertCount, uint32 nIndexStart, uint32 nTriCount);

private:
	CRenderShader_Glow()	{}
};

#endif //__D3D_RENDERSHADER_GLOW_H__

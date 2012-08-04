//
//   (c) 1998-1999 Monolith Productions, Inc.  All Rights Reserved
//
// ---------------------------------------------------------------
//

// Defines the texture management routines for the d3d renderer.

#ifndef __D3D_TEXTUREMGR_H__
#define __D3D_TEXTUREMGR_H__

	//forward declarations
	class  D3DRender;
	class  CEditPoly;
	class  CPrePoly;

	//---functions for hanlding texture usage in D3D---//

	BOOL d3d_InitTextureStuff(D3DRender *pRender);
	void d3d_TermTextureStuff(D3DRender *pRender);
	
	BOOL d3d_SetupPolyTexture(D3DRender *pRender, DFileIdent_t *pTextureFile, DFileIdent_t *pDetailFile);

	void d3d_UnsetDetailTexture(D3DRender* pRender);

	//creates texture surfaces
	LPDIRECTDRAWSURFACE4 d3d_CreateTextureSurface(D3DRender *pRender, DWORD width, DWORD height, DWORD extraCaps);
	LPDIRECTDRAWSURFACE4 d3d_CreateTextureSurface2(D3DRender *pRender, DWORD width, DWORD height, DWORD extraCaps, TextureFormat *pFormat);


#endif  // __D3D_TEXTUREMGR_H__



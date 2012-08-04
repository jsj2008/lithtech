//------------------------------------------------------------------
//
//   MODULE    : D3DDRAWPRIM.H
//
//   PURPOSE   : Implements derived class CD3DDrawPrim
//
//   CREATED   : On 8/11/00 At 1:39:59 PM
//
//   COPYRIGHT : (C) 2000 LithTech Inc
//
//------------------------------------------------------------------

#ifndef __D3DDRAWPRIM_H__
#define __D3DDRAWPRIM_H__

// Make sure D3D doesn't define this guy...
#define MAKE_FOURCC

#pragma warning (disable:4530)

// Includes....
#ifndef __ILTDRAWPRIM_H__
#include "iltdrawprim.h"
#endif

#ifndef __GENDRAWPRIM_H__
#include "gendrawprim.h"
#endif

#ifndef __RENDER_H__
#include "render.h"
#endif

#ifndef __DE_OBJECTS_H__
#include "de_objects.h"
#endif

#ifndef __D3D_RENDERSTATEMGR_H__
#include "d3d_renderstatemgr.h"
#endif

#ifndef __D3D9_H__
#include <d3d9.h>
#define __D3D9_H__
#endif

#ifndef __D3DX9_H__
#include <d3dx9.h>
#define __D3DX9_H__
#endif

#ifndef __D3D9TYPES_H__
#include <d3d9types.h>
#define __D3D9TYPES_H__
#endif

// Size of the draw prim vertex buffer (used when it needs to do conversions)...
#define CD3DDRAWPRIM_BUFSIZE	128

#define DRAWPRIM_D3DTRANS_FLAGS		(D3DFVF_XYZRHW | D3DFVF_DIFFUSE)
struct  DRAWPRIM_D3DTRANS			{ float x; float y; float z; float rhw; D3DCOLOR rgba; };
#define DRAWPRIM_D3DTRANS_TEX_FLAGS	(D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_TEX1)
struct  DRAWPRIM_D3DTRANS_TEX		{ float x; float y; float z; float rhw; D3DCOLOR rgba; float u; float v; };

#ifndef RGBA_MAKE
#define RGBA_MAKE(r, g, b, a)		((D3DCOLOR) (((a) << 24) | ((r) << 16) | ((g) << 8) | (b)))
#endif

class CD3DDrawPrim : public CGenDrawPrim
{
	public :
		declare_interface(CD3DDrawPrim);

		CD3DDrawPrim();

		virtual LTRESULT BeginDrawPrim();
		virtual LTRESULT EndDrawPrim();
		
		// Sets the current camera to use (viewport, field of view etc)
		virtual LTRESULT SetCamera(const HOBJECT hCamera);

		// Sets current texture
		virtual LTRESULT SetTexture(const HTEXTURE hTexture);

		// Sets transform type
		virtual LTRESULT SetTransformType(const ELTTransformType eType);

		// Sets color operation
		virtual LTRESULT SetColorOp(const ELTColorOp eColorOp);

		// Sets source/dest alpha blending operation
		virtual LTRESULT SetAlphaBlendMode(const ELTBlendMode eBlendMode);

		// Enables/disables z buffering
		virtual LTRESULT SetZBufferMode(const ELTZBufferMode eZBufferMode);

		// Set AlphaTest Mode (on/off)
		virtual LTRESULT SetAlphaTestMode(const ELTTestMode eTestMode);

		// set the type of clipping to be done
		virtual LTRESULT SetClipMode(const ELTClipMode eClipMode);

		// set the fill mode
		virtual LTRESULT SetFillMode(ELTDPFillMode eFillMode);

		// set the cull mode
		virtual LTRESULT SetCullMode(ELTDPCullMode eCullMode);

		// set the fog enable
		virtual LTRESULT SetFogEnable(bool bFogEnable);

        //Specifiy whether or not to be in really close space for rendering
        virtual LTRESULT SetReallyClose(bool bReallyClose);

		virtual LTRESULT SetEffectShaderID(uint32 nEffectShaderID);

		virtual void SetUVWH (LT_POLYGT4 *pPrim, HTEXTURE pTex,
							 float u, float v,
							 float w, float h);

		// store off the current viewport
		virtual void SaveViewport();

		// restore the saved viewport
		virtual void RestoreViewport();

		// Draw primitives (triangles)
		virtual LTRESULT DrawPrim (LT_POLYGT3 *pPrim, 
	                           	const uint32 nCount = 1);
		virtual LTRESULT DrawPrim (LT_POLYFT3 *pPrim, 
	                           	const uint32 nCount = 1);
		virtual LTRESULT DrawPrim (LT_POLYG3 *pPrim, 
	                           	const uint32 nCount = 1);
		virtual LTRESULT DrawPrim (LT_POLYF3 *pPrim, 
	                           	const uint32 nCount = 1);
	
		// Draw primitives (quadrilaterals)
		virtual LTRESULT DrawPrim (LT_POLYGT4 *pPrim, 
	                           	const uint32 nCount = 1);

		virtual LTRESULT DrawPrim (LT_POLYGT4 **ppPrim, 
                                const uint32 nCount = 1);

		virtual LTRESULT DrawPrim (LT_POLYFT4 *pPrim, 
	                           	const uint32 nCount = 1);
		virtual LTRESULT DrawPrim (LT_POLYG4 *pPrim, 
	                           	const uint32 nCount = 1);
		virtual LTRESULT DrawPrim (LT_POLYF4 *pPrim, 
	                           	const uint32 nCount = 1);

		// Draw primitives using lines
		// Note: nCount is Line count.
		virtual LTRESULT DrawPrim (LT_LINEGT *pPrim, 
		                           const uint32 nCount = 1);
		virtual LTRESULT DrawPrim (LT_LINEFT *pPrim, 
		                           const uint32 nCount = 1);
		virtual LTRESULT DrawPrim (LT_LINEG *pPrim, 
		                           const uint32 nCount = 1);
		virtual LTRESULT DrawPrim (LT_LINEF *pPrim, 
		                           const uint32 nCount = 1);

		// Draw primitives using points
		// Note: nCount is Point count.
		virtual LTRESULT DrawPrimPoint (LT_VERTGT *pVerts, 
		                           const uint32 nCount = 1);
		virtual LTRESULT DrawPrimPoint (LT_VERTG *pVerts, 
		                           const uint32 nCount = 1);

		// Draw primitives using triangle fans
		virtual LTRESULT DrawPrimFan (LT_VERTGT *pVerts, 
	                          	  	  const uint32 nCount);
		virtual LTRESULT DrawPrimFan (LT_VERTFT *pVerts, 
	                           		  const uint32 nCount, 
							   		  LT_VERTRGBA rgba);
		virtual LTRESULT DrawPrimFan (LT_VERTG *pVerts, 
	                           		  const uint32 nCount);
		virtual LTRESULT DrawPrimFan (LT_VERTF *pVerts, 
	                           		  const uint32 nCount, 
							   		  LT_VERTRGBA rgba);
	
		// Draw primitives using triangle strips
		virtual LTRESULT DrawPrimStrip (LT_VERTGT *pVerts, 
	                              		const uint32 nCount);
		virtual LTRESULT DrawPrimStrip (LT_VERTFT *pVerts, 
	                           			const uint32 nCount, 
							   			LT_VERTRGBA rgba);
		virtual LTRESULT DrawPrimStrip (LT_VERTG *pVerts, 
	                           			const uint32 nCount);
		virtual LTRESULT DrawPrimStrip (LT_VERTF *pVerts, 
	                           			const uint32 nCount, 
							   			LT_VERTRGBA rgba);

	private:

		//saves the current D3D state into the member variables
		void		SaveStates(LPDIRECT3DDEVICE9 pDevice);

		//sets up the appropriate state for the each section
		void		SetClipMode(LPDIRECT3DDEVICE9 pDevice);
		void		SetTransformMode(LPDIRECT3DDEVICE9 pDevice);
		void		SetColorOp(LPDIRECT3DDEVICE9 pDevice);
		void		SetBlendMode(LPDIRECT3DDEVICE9 pDevice);
		void		SetZBufferMode(LPDIRECT3DDEVICE9 pDevice);
		void		SetTestMode(LPDIRECT3DDEVICE9 pDevice);
		void		SetFillMode(LPDIRECT3DDEVICE9 pDevice);
		void		SetCullMode(LPDIRECT3DDEVICE9 pDevice);
		void		SetFogEnable(LPDIRECT3DDEVICE9 pDevice);
		void		SetTexture(LPDIRECT3DDEVICE9 pDevice);
		void		SetCamera(LPDIRECT3DDEVICE9 pDevice);
		void		SetReallyClose(LPDIRECT3DDEVICE9 pDevice);

		//verifies that a render operation can take place
		bool		VerifyValid();


		// Vertex buffers for DrawPrim calls...
		LT_VERTGT	m_VertBufGT[CD3DDRAWPRIM_BUFSIZE];
		LT_VERTG	m_VertBufG[CD3DDRAWPRIM_BUFSIZE];
		DRAWPRIM_D3DTRANS		m_VertTransBuf[CD3DDRAWPRIM_BUFSIZE];
		DRAWPRIM_D3DTRANS_TEX	m_VertTransBufT[CD3DDRAWPRIM_BUFSIZE];

		// Render State Save States...
		uint32		m_PrevColorOp;
		uint32		m_PrevAlphaBlendEnable;
		uint32		m_PrevSrcBlend;
		uint32		m_PrevDstBlend;
		uint32		m_PrevZEnable;
		uint32		m_PrevZWriteEnable;
		uint32		m_PrevAlphaTestEnable;
		uint32		m_PrevAlphaTestFunc;
		uint32		m_PrevFillMode;
		uint32		m_PrevClipMode;
		uint32		m_PrevCullMode;
		uint32		m_PrevFogMode;
		D3DMATRIX	m_PrevTransView;
		D3DMATRIX	m_PrevTransProj;
		D3DVIEWPORT9 m_PrevViewport;
		D3DVIEWPORT9 m_SavedViewport;
		bool		m_bPrevReallyClose;
		bool		m_bResetViewport;

		//determines if we have changed the states
		uint32		m_nBlockCount;

		// Set the render states (and save the old ones)...
		void		PushRenderStates(LPDIRECT3DDEVICE9 pDevice);

		// Reset the old renderstates...
		void		PopRenderStates(LPDIRECT3DDEVICE9 pDevice);
};
#endif // __D3DDRAWPRIM_H__

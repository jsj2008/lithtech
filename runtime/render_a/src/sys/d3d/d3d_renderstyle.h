// d3d_renderstyle.h
//	Defines a d3d render style. The renderer supports setting render styles for render
// objects.

#ifndef __D3D_RENDERSTYLE_H__
#define __D3D_RENDERSTYLE_H__

#ifndef __LIST__
#include <list>
#define __LIST__
#endif

#ifndef __VECTOR__
#include <vector>
#define __VECTOR__
#endif

#ifndef __LTRENDERSTYLE_H__
#include "ltrenderstyle.h"
#endif

#ifndef __D3DX9_H__
#include <d3dx9.h>
#define __D3DX9_H__
#endif

using namespace std;

// PROTOTYPES
class	CD3DRenderStateMgr;
class	CD3D_Device;

// TYPEDEFS
typedef unsigned long				HD3DVERTEXSHADER;

// DEFINES
#define	RENDERSTYLE_D3D_VERSION		3

// STRUCT
class CD3DRenderPass
{
public:
	CD3DRenderPass()				{ pD3DRenderPass = NULL; }
	RenderPassOp					RenderPass;
	RSD3DRenderPass*				pD3DRenderPass;
};

class CD3DRenderStyle : public CRenderStyle{
	friend CD3DRenderStateMgr;
	friend CD3D_Device;
public:
	CD3DRenderStyle();
	virtual ~CD3DRenderStyle();

	// Lighting Material...
	virtual bool					SetLightingMaterial(LightingMaterial& LightMaterial)
	{
		m_LightingMaterial = LightMaterial;
		return true;
	}

	virtual bool					GetLightingMaterial(LightingMaterial* pLightMaterial)
	{
		*pLightMaterial = m_LightingMaterial;
		return true;
	}

	// RenderPasses...
	virtual bool					AddRenderPass(RenderPassOp& RenderPass)
	{
		if (m_RenderPasses.size() < 4)
		{
			CD3DRenderPass d3dRenderPass;
			d3dRenderPass.RenderPass = RenderPass;
			m_RenderPasses.push_back(d3dRenderPass);
			return true;
		}

		return false;
	}

	virtual bool					RemoveRenderPass(uint32 iPass)
	{
		list<CD3DRenderPass>::iterator it = m_RenderPasses.begin();
		uint32 i = 0;
		while ((i < iPass) && (it != m_RenderPasses.end()))
		{
			++it;
			++i;
		}

		if (it != m_RenderPasses.end())
		{
			m_RenderPasses.erase(it);
			return true;
		}

		return false;
	}

	virtual bool					SetRenderPass(uint32 iPass,RenderPassOp& RenderPass)
	{
		list<CD3DRenderPass>::iterator it = m_RenderPasses.begin();
		uint32 i = 0;
		while ((i < iPass) && (it != m_RenderPasses.end()))
		{
			++it;
			++i;
		}

		if (it != m_RenderPasses.end())
		{
			it->RenderPass = RenderPass;
			return true;
		}

		return false;
	}

	virtual bool					GetRenderPass(uint32 iPass,RenderPassOp* pRenderPass)
	{
		list<CD3DRenderPass>::iterator it = m_RenderPasses.begin();
		uint32 i = 0;
		while ((i < iPass) && (it != m_RenderPasses.end()))
		{
			++it;
			++i;
		}

		if (it != m_RenderPasses.end())
		{
			*pRenderPass = it->RenderPass;
			return true;
		}

		return false;
	}

	virtual uint32					GetRenderPassCount()									{ return m_RenderPasses.size(); }

	// Platform Options: Direct3D...
	virtual bool					SetDirect3D_Options(RSD3DOptions& Options)
	{
		if (!m_pD3DOptions)
		{
			LT_MEM_TRACK_ALLOC(m_pD3DOptions = new RSD3DOptions, LT_MEM_TYPE_RENDERER);
		}

		if (m_pD3DOptions)
		{
			*m_pD3DOptions = Options;
			return true;
		}
		else
		{
			return false;
		}
	}

	virtual bool					GetDirect3D_Options(RSD3DOptions* pOptions)
	{
		if (m_pD3DOptions)
		{
			*pOptions = *m_pD3DOptions;
			return true;
		}

		return false;
	}

	virtual bool					SetRenderPass_D3DOptions(uint32 iPass,RSD3DRenderPass* pD3DRenderPass);

	virtual bool					GetRenderPass_D3DOptions(uint32 iPass,RSD3DRenderPass* pD3DRenderPass)
	{
		list<CD3DRenderPass>::iterator it = m_RenderPasses.begin();
		uint32 i = 0;
		while ((i < iPass) && (it != m_RenderPasses.end()))
		{
			++it;
			++i;
		}

		if (it != m_RenderPasses.end())
		{
			if (!it->pD3DRenderPass)
			{
				return false;
			}

			*pD3DRenderPass = *(it->pD3DRenderPass);
			return true;
		}

		return false;
	}

	RSD3DRenderPass*				GetRenderPass_D3DOptions(uint32 iPass)
	{
		RSD3DRenderPass *pPass = NULL;

		list<CD3DRenderPass>::iterator it = m_RenderPasses.begin();
		uint32 i = 0;
		while (i < iPass && it != m_RenderPasses.end())
		{
			++it;
			++i;
		}

		if (it != m_RenderPasses.end() && NULL != it->pD3DRenderPass)
		{
			pPass = it->pD3DRenderPass;
		}

		return pPass;
	}

	// Helper Functions...
	virtual bool					Load_LTBData(ILTStream* pFileStream);					// Stream in the ltb file...
	virtual bool					Compile();												// Compile the sucker...
	virtual void					SetDefaults();
	virtual bool					CopyRenderStyle(CRenderStyle* pSrcRenderStyle);			// Copy the render style data from a source RS and put it in this one...
	virtual bool					IsSupportedOnDevice();									// Does the current device support this render style...

	// On Alt-Tab we can lose our vertex shader - Use these functions to free/restore them...
	CD3DRenderStyle*				GetNext()												{ return m_pNext; }
	void							SetNext(CD3DRenderStyle* pNext)							{ m_pNext = pNext; }

protected:
	// Lighting Material...
	LightingMaterial				m_LightingMaterial;

	// Render Passes...
	list<CD3DRenderPass>			m_RenderPasses;

	// Direct3D Options...
	RSD3DOptions*					m_pD3DOptions;

	// Renderer Helper member vars...
	CD3DRenderStyle*				m_pNext;												// Helper for keeping a list of these guys...
};

#endif // __D3D_RENDERSTYLE_H__


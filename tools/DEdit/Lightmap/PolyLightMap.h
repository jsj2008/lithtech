#ifndef __CPOLYLIGHTMAP_H__
#define __CPOLYLIGHTMAP_H__

//forward declarations
#ifndef __LIGHTMAPDATA_H__
#	include "LightMapData.h"
#endif

#ifdef DIRECTEDITOR_BUILD
#	ifndef __SURFACELMTEXTUREMGR_H__
#		include "surfacelmtexturemgr.h"
#	endif
#endif

enum {	POLYLMFLAG_DIRTYTEXTURE = (1 << 0),
		POLYLMFLAG_INDIRTYLIST	= (1 << 1)	};

class CPolyLightMap
{
public:

	CPolyLightMap() :
		m_pLightMap(NULL),
		m_pTexture(NULL),
		m_nFlags(0)
	{
	}

	~CPolyLightMap()
	{
		FreeLightMap();
		FreeTexture();
	}

	//retreives the raw lightmap data
	CLightMapData* GetLightMap()		
	{ 
		return m_pLightMap; 
	}

	//sets the raw lightmap data to the one passed in. This orphans the
	//data object and this object will handle freeing it
	void SetLightMap(CLightMapData* pLightMap)
	{
		FreeLightMap();
		m_pLightMap = pLightMap;

		EnableFlag(POLYLMFLAG_DIRTYTEXTURE);
		DisableFlag(POLYLMFLAG_INDIRTYLIST);
	}

	//frees the raw lightmap data
	void FreeLightMap()
	{
		delete m_pLightMap;
		m_pLightMap = NULL;
	}

	void FreeTexture()
	{
		//clean up the texture if needed
#ifdef DIRECTEDITOR_BUILD
		RemoveSurfaceLightMap(m_pTexture);
#endif
	}

	bool	IsTextureDirty() const			
	{
		return IsFlagSet(POLYLMFLAG_DIRTYTEXTURE);
	}

	void	ClearTextureDirty()
	{
		DisableFlag(POLYLMFLAG_DIRTYTEXTURE);
	}

	void	AddedToDirtyList()
	{
		EnableFlag(POLYLMFLAG_INDIRTYLIST);
	}

	bool	IsInDirtyList() const
	{
		return IsFlagSet(POLYLMFLAG_INDIRTYLIST);
	}

	
	//the origin of where to base LM texel 0, 0 at in space
	LTVector			m_vO;

	//the P and Q vectors that orient the lightmap
	LTVector			m_vP;
	LTVector			m_vQ;

	//this is used for the binding to the actual texture
	void*				m_pTexture;

private:

	bool				IsFlagSet(uint8 nFlag) const	{ return (m_nFlags & nFlag) ? true : false; }
	void				EnableFlag(uint8 nFlag)			{ m_nFlags |= nFlag;  }
	void				DisableFlag(uint8 nFlag)		{ m_nFlags &= ~nFlag; }

	//if this lightmap hasn't already been converted to a texture, this contains
	//the raw image data of the lightmap
	CLightMapData*		m_pLightMap;

	//determines if the texture is dirty
	uint8				m_nFlags;

};

#endif

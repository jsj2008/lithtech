// d3d_rendobj_spritearray.h
//	D3D Specific implementation of a SpriteArray Render Object.

#ifndef __D3DRENDOBJ_SPRITEARRAY_H__
#define __D3DRENDOBJ_SPRITEARRAY_H__

#include "VertexBufferController.h"
#include "ltrenderstyle.h"

class SharedTexture;

// To keep track of free data / used data in the array...
struct SSpriteArray_DataChunk 
{
	SSpriteArray_DataChunk(uint32 iStart, uint32 iCount)	
	{ 
		m_iStart = iStart; m_iCount = iCount; 
	}

	uint32							m_iStart;
	uint32							m_iCount; 
};

class CD3DSpriteArray
{
public:

	enum DATATYPE			{ ePosition = (1<<0), eColor = (1<<1), eSize = (1<<2), eTextureIndex = (1<<3), eUVs = (1<<4) };
	typedef uint32			HSPRITE;

	CD3DSpriteArray();
	~CD3DSpriteArray();

	// Resets back to initial conditions (doesn't try to free anything)...
	void							Reset();				
	// Frees all the member vars and resets afterwards...
	void							FreeAll();				

	// Create the VBs and stuff from our sys mem copies...
	void							ReCreateObject();	
	
	// We're loosing focus, free the stuff...
	void							FreeDeviceObjects();

	uint32							GetPolyCount()			{ return m_iSpriteCount; }
	uint32							GetVertexCount()		{ return m_iSpriteCount; }

	// Initialize the SpriteArray (must be done before sprites are added)...
	bool							Init(float fSpriteRadius, DATATYPE iSpriteDataType, uint32 iApproxMaxSpriteCount, uint32 bAllowParticleApproximation = true);
	void							Term()					{ FreeAll(); }

	// Info query fuctions...
	float							GetMaxParticleSize();

	// Add and Remove sprites (by number)...
	HSPRITE							AddSprite();
	void							RemoveSprite(HSPRITE hSprite);

	// Setting the data (note: data must be locked before it can be set, and unlocked before it can be used)...
	bool							Lock();
	bool							IsLocked();
	bool							SetPosition(HSPRITE hSprite, LTVector3f& Position);
	bool							GetPosition(HSPRITE hSprite, LTVector3f* pPosition);
	bool							SetColor(HSPRITE hSprite, uint32 Color);
	bool							GetColor(HSPRITE hSprite, uint32* pColor);
	bool							SetSize(HSPRITE hSprite, float Size);
	bool							GetSize(HSPRITE hSprite, float* pSize);
	bool							SetTextureIndex(HSPRITE hSprite, uint8 Index);
	bool							GetTextureIndex(HSPRITE hSprite, uint8* pIndex);
	bool							SetUVs(HSPRITE hSprite, float* pUVs);
	bool							GetUVs(HSPRITE hSprite, float* pUVs);
	bool							UnLock();

	// Configure how the sprites are rendered...
	bool							SetSpriteRadius(float fSpriteRadius)	{ m_fSpriteRadius = fSpriteRadius; return true; }
	bool							SetBaseTextureIndex(uint8 TextureIndex)	{ m_BaseTextureIndex = TextureIndex; return true; }
	bool							SetMaxTextureIndex(uint8 TextureIndex)	{ m_MaxTextureIndex = uint8(TextureIndex + 1); return true; }
	bool							SetScreenSpaceScaling(float AttConst_A, float AttConst_B, float AttConst_C,float fMinSpriteSize,float fMaxSpriteSize);

	// Poly Sorting...
	void							SortPolies(uint32 Iterations, LTVector3f& SpriteSpace_ViewPos);

	// Render the SpriteArray...
	void							Render(LTMatrix& WorldTransform, CRenderStyle* pRenderStyle, vector<LPDIRECT3DBASETEXTURE9>& TextureList);

private:
	void							Render_PointSprites(vector<LPDIRECT3DBASETEXTURE9>& TextureList);	// Render using point sprites...
	void							Render_Quads(vector<LPDIRECT3DBASETEXTURE9>& TextureList);			// Render using quads...
	inline void						Render_Quads_CalcPos(uint32 StartSprite, uint32 Count);

	bool							m_bInitialized;
	bool							m_bIsLocked;
	bool							m_bUsePointSprites;		// Use Point sprites, or full quads for each sprite...
	bool							m_bSWVertProcessing;	// Created for SW Vert Processing...
	uint8							m_BaseTextureIndex;		// Base texture index (added to the Texture indicies)...
	uint8							m_MaxTextureIndex;
	VertexBufferController			m_VBController;			// Our Vertex Buffer Controller - he ownz the buffers...
	uint32							m_iSpriteCount;
	uint32							m_iMaxSpriteCount;		// Should always be the VB size as well...
	uint32							m_VertFlags; 
	float							m_fSpriteRadius;
	D3DXMATRIX						m_WorldViewTransform;	// Used for Quad rendering of sprites...

	bool							m_bSSScaling;			// ScreenSpaceScaling Params...
	float							m_SSScaling_Att[3];		// ScreenSpaceScaling attenuation params: A, B, C...
	float							m_SSScaling_MinSize;
	float							m_SSScaling_MaxSize;

	uint8*							m_pVertData;			// Sys mem copy of our data...
	uint8*							m_pTexIndexData;		// Texture indicies of the point sprites...
	uint8*							m_pUsedBitArray;		// Bit array that specifies if verts in the VB are active sprites (size should match the VB, which should be MaxSpriteCount)...
	LTVector3f*						m_pPosData;				// Sprite Positions (if we're not point sprites, we'll keep the positions of all the sprites)...
	float*							m_pSizeData;			// Sprite Size (if we're not point sprites, we'll keep the size of all the sprites)...
};

#endif 

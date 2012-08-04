#ifndef __D3D_RENDERSHADER_LIGHTMAP_H__
#define __D3D_RENDERSHADER_LIGHTMAP_H__

#include "d3d_rendershader_base.h"
#include "d3d_texture.h"

// Forward declarations
class TextureFormat;
class RTexture;

// Plain Lightmap shader
struct SVertex_Lightmap
{
	LTVector m_vPos;
	float m_fU, m_fV, m_fW;
};

struct CSection_Lightmap
{
	// Needs to be more functional for tracking the texture
	CSection_Lightmap() : m_pLMTexture(0) {}
	CSection_Lightmap(const CSection_Lightmap &cOther);
	~CSection_Lightmap();
	CSection_Lightmap &operator=(const CSection_Lightmap &cOther);

	bool operator<(const CSection_Lightmap &cOther) const { return m_pLMTexture < cOther.m_pLMTexture; }
	bool operator!=(const CSection_Lightmap &cOther) const { return !(*this == cOther); }
	bool operator==(const CSection_Lightmap &cOther) const { return (m_pLMTexture == cOther.m_pLMTexture); }
	void SetTexture(SharedTexture *pTexture) { }
	SharedTexture *GetTexture() { return 0; }

	bool ShouldAlphaTest() const { return false; }
	bool ShouldGlow() const { return false; }

	IDirect3DTexture9 *m_pLMTexture;

	CTrackedTextureMem	m_TextureMem;
};

enum { k_SVertex_Lightmap_FVF = D3DFVF_XYZ | D3DFVF_TEX1 | D3DFVF_TEXCOORDSIZE3(0) };

class CRenderShader_Lightmap : public CRenderShader_Base<SVertex_Lightmap, CSection_Lightmap, k_SVertex_Lightmap_FVF>
{
public:

	~CRenderShader_Lightmap() { s_bValidateRequired = true; }

	virtual bool ValidateShader(const CRBSection &cSection);

	virtual void DebugTri(
		uint32 nRenderBlock,
		const CRBSection &cSection,
		uint32 nTriIndex,
		const uint16 *aIndices,
		const SRBVertex *aVertices,
		float fX, float fY,
		float fSizeX, float fSizeY);

	void UpdateLightmap(uint32 nRenderBlock, const CRBSection &cSection, const uint8 *pLMData);

	// Count the memory used by this shader
	virtual void GetMemStats(CMemStats_World &cMemStats) const;

	//Support for determining the ID of this shader
	virtual ERenderShader GetShaderID() const { return eShader_Lightmap; }

protected:

	virtual void DrawNormal(const DrawState &cState, uint32 nRenderBlock);

	virtual void DrawGlow(uint32 nRenderBlock) { /* Don't glow */ }

	virtual void PreFlush();
	virtual void FlushChangeSection(CInternalSection &cSection);
	virtual void PreFlushBlock(CInternalSection &cSection);
	virtual void PostFlushBlock(CInternalSection &cSection,
		uint32 nStartIndex, uint32 nEndIndex,
		uint32 nStartVertex, uint32 nEndVertex);
	virtual void PostFlush();

	// Translate a block of vertices from SRBVertex to SVertex
	virtual void TranslateVertices(SVertex_Lightmap *pOut, const SRBVertex *pIn, uint32 nCount);

	// Fill in an internal section from a CRBSection
	virtual void FillSection(CSection_Lightmap &cInternalSection, const CRBSection &cSection);
private:
	// Draw the lighting for the sections
	void DrawLights(const DrawState &cState, uint32 nRenderBlock);

	// Internal function for updating the lightmap data
	void UpdateLightmap_Internal(const CRBSection &cSection, const uint8 *pLMData, TextureFormat* pFormat, CSection_Lightmap *pInternalSection);

	static bool s_bValidateRequired, s_bValidateResult;
};

// Lightmapped texture pass shader
struct SVertex_Lightmap_Texture
{
	LTVector m_vPos;
	float m_fU, m_fV, m_fW;
};

struct CSection_Lightmap_Texture
{
	CSection_Lightmap_Texture()
		: m_pTexture(NULL),
		  m_bFullbright(false)
	{
	}

	bool operator<(const CSection_Lightmap_Texture &cOther) const { return m_pTexture < cOther.m_pTexture; }
	bool operator!=(const CSection_Lightmap_Texture &cOther) const { return !(*this == cOther); }
	bool operator==(const CSection_Lightmap_Texture &cOther) const {
		return
			(m_pTexture == cOther.m_pTexture) &&
			(m_bFullbright == cOther.m_bFullbright);
	}
	void SetTexture(SharedTexture *pTexture) { m_pTexture = pTexture; }
	SharedTexture *GetTexture() { return m_pTexture; }

	bool ShouldAlphaTest() const { return false; }
	bool ShouldGlow() const { return m_bFullbright; }

	SharedTexture *m_pTexture;
	bool	m_bFullbright;
};

enum { k_SVertex_Lightmap_Texture_FVF = D3DFVF_XYZ | D3DFVF_TEX1 | D3DFVF_TEXCOORDSIZE3(0) };

class CRenderShader_Lightmap_Texture :
	public CRenderShader_Base<
		SVertex_Lightmap_Texture,
		CSection_Lightmap_Texture,
		k_SVertex_Lightmap_Texture_FVF>
{
public:

	~CRenderShader_Lightmap_Texture() { s_bValidateRequired = true; }

	virtual bool ValidateShader(const CRBSection &cSection);

	virtual void DebugTri(
		uint32 nRenderBlock,
		const CRBSection &cSection,
		uint32 nTriIndex,
		const uint16 *aIndices,
		const SRBVertex *aVertices,
		float fX, float fY,
		float fSizeX, float fSizeY);

	// Count the memory used by this shader
	virtual void GetMemStats(CMemStats_World &cMemStats) const;

	//Support for determining the ID of this shader
	virtual ERenderShader GetShaderID() const { return eShader_Lightmap_Texture; }

protected:

	// Draw the sections
	virtual void DrawNormal(const DrawState &cState, uint32 nRenderBlock);

	virtual void PreFlush();
	virtual void FlushChangeSection(CInternalSection &cSection);
	virtual void PreFlushBlock(CInternalSection &cSection);
	virtual void PostFlushBlock(CInternalSection &cSection,
		uint32 nStartIndex, uint32 nEndIndex,
		uint32 nStartVertex, uint32 nEndVertex);
	virtual void PostFlush();

	// Translate a block of vertices from SRBVertex to SVertex
	virtual void TranslateVertices(SVertex_Lightmap_Texture *pOut, const SRBVertex *pIn, uint32 nCount);

	// Fill in an internal section from a CRBSection
	virtual void FillSection(CSection_Lightmap_Texture &cInternalSection, const CRBSection &cSection);
private:
	static bool s_bValidateRequired, s_bValidateResult;

	uint32 m_nFlushStateAlpha0;
	uint32 m_nFlushStateAlpha1;
	uint32 m_nFlushStateAlpha2;
	uint32 m_nFlushStateFogColor;

};

// Lightmapped texture+detail pass shader
struct SVertex_Lightmap_Texture_Detail
{
	LTVector m_vPos;
	float m_fU0, m_fV0, m_fW0;
	float m_fU1, m_fV1, m_fW1;
};

struct CSection_Lightmap_Texture_Detail
{
	CSection_Lightmap_Texture_Detail()
		: m_pTexture(NULL),
		  m_bFullbright(false)
	{
	}

	bool operator<(const CSection_Lightmap_Texture_Detail &cOther) const { return m_pTexture < cOther.m_pTexture; }
	bool operator!=(const CSection_Lightmap_Texture_Detail &cOther) const { return !(*this == cOther); }
	bool operator==(const CSection_Lightmap_Texture_Detail &cOther) const {
		return
			(m_pTexture == cOther.m_pTexture) &&
			(m_bFullbright == cOther.m_bFullbright);
	}
	void SetTexture(SharedTexture *pTexture) { m_pTexture = pTexture; }
	SharedTexture *GetTexture() { return m_pTexture; }

	bool ShouldAlphaTest() const { return false; }
	bool ShouldGlow() const { return m_bFullbright; }

	SharedTexture *m_pTexture;
	bool	m_bFullbright;
};

enum { k_SVertex_Lightmap_Texture_Detail_FVF = D3DFVF_XYZ | D3DFVF_TEX2 | D3DFVF_TEXCOORDSIZE3(0) | D3DFVF_TEXCOORDSIZE3(1) };

class CRenderShader_Lightmap_Texture_Detail :
	public CRenderShader_Base<
		SVertex_Lightmap_Texture_Detail,
		CSection_Lightmap_Texture_Detail,
		k_SVertex_Lightmap_Texture_Detail_FVF>
{
public:
	~CRenderShader_Lightmap_Texture_Detail() { s_bValidateRequired = true; }

	// Count the memory used by this shader
	virtual void GetMemStats(CMemStats_World &cMemStats) const;

	//Support for determining the ID of this shader
	virtual ERenderShader GetShaderID() const { return eShader_Lightmap_Texture_Detail; }

protected:

	// Draw the sections
	virtual void DrawNormal(const DrawState &cState, uint32 nRenderBlock);

	virtual void PreFlush();
	virtual void FlushChangeSection(CInternalSection &cSection);
	virtual void PreFlushBlock(CInternalSection &cSection);
	virtual void PostFlushBlock(CInternalSection &cSection,
		uint32 nStartIndex, uint32 nEndIndex,
		uint32 nStartVertex, uint32 nEndVertex);
	virtual void PostFlush();

	virtual bool ValidateShader(const CRBSection &cSection);

	// Translate a block of vertices from SRBVertex to SVertex
	virtual void TranslateVertices(SVertex_Lightmap_Texture_Detail *pOut, const SRBVertex *pIn, uint32 nCount);

	// Fill in an internal section from a CRBSection
	virtual void FillSection(CSection_Lightmap_Texture_Detail &cInternalSection, const CRBSection &cSection);
private:
	static bool s_bValidateRequired, s_bValidateResult;

	uint32 m_nFlushStateAlpha0;
	uint32 m_nFlushStateAlpha1;
	uint32 m_nFlushStateAlpha2;
	uint32 m_nFlushStateFogColor;

};

// Lightmapped texture+envmap pass shader
struct SVertex_Lightmap_Texture_EnvMap
{
	LTVector m_vPos;
	LTVector m_vNormal;
	float m_fU, m_fV, m_fW;
};

struct CSection_Lightmap_Texture_EnvMap
{
	CSection_Lightmap_Texture_EnvMap()
		: m_pTexture(NULL),
		  m_bFullbright(false)
	{
	}

	bool operator<(const CSection_Lightmap_Texture_EnvMap &cOther) const { return m_pTexture < cOther.m_pTexture; }
	bool operator!=(const CSection_Lightmap_Texture_EnvMap &cOther) const { return !(*this == cOther); }
	bool operator==(const CSection_Lightmap_Texture_EnvMap &cOther) const {
		return
			(m_pTexture == cOther.m_pTexture) &&
			(m_bFullbright == cOther.m_bFullbright);
	}
	void SetTexture(SharedTexture *pTexture) { m_pTexture = pTexture; }
	SharedTexture *GetTexture() { return m_pTexture; }

	bool ShouldAlphaTest() const { return false; }
	bool ShouldGlow() const { return m_bFullbright; }

	SharedTexture *m_pTexture;
	bool	m_bFullbright;
};

enum { k_SVertex_Lightmap_Texture_EnvMap_FVF = D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_TEX1 | D3DFVF_TEXCOORDSIZE3(0) };

class CRenderShader_Lightmap_Texture_EnvMap :
	public CRenderShader_Base<
		SVertex_Lightmap_Texture_EnvMap,
		CSection_Lightmap_Texture_EnvMap,
		k_SVertex_Lightmap_Texture_EnvMap_FVF>
{
public:
	~CRenderShader_Lightmap_Texture_EnvMap() { s_bValidateRequired = true; }

	virtual bool ValidateShader(const CRBSection &cSection);

	// Count the memory used by this shader
	virtual void GetMemStats(CMemStats_World &cMemStats) const;

	//Support for determining the ID of this shader
	virtual ERenderShader GetShaderID() const { return eShader_Lightmap_Texture_EnvMap; }

protected:

	// Draw the sections
	virtual void DrawNormal(const DrawState &cState, uint32 nRenderBlock);

	virtual void PreFlush();
	virtual void FlushChangeSection(CInternalSection &cSection);
	virtual void PreFlushBlock(CInternalSection &cSection);
	virtual void PostFlushBlock(CInternalSection &cSection,
		uint32 nStartIndex, uint32 nEndIndex,
		uint32 nStartVertex, uint32 nEndVertex);
	virtual void PostFlush();

	// Translate a block of vertices from SRBVertex to SVertex
	virtual void TranslateVertices(SVertex_Lightmap_Texture_EnvMap *pOut, const SRBVertex *pIn, uint32 nCount);

	// Fill in an internal section from a CRBSection
	virtual void FillSection(CSection_Lightmap_Texture_EnvMap &cInternalSection, const CRBSection &cSection);
private:
	static bool s_bValidateRequired, s_bValidateResult;

	uint32 m_nFlushStateAlpha0;
	uint32 m_nFlushStateAlpha1;
	uint32 m_nFlushStateAlpha2;
	uint32 m_nFlushStateFogColor;
	RTexture *m_pCurEnvMap;
};

// Lightmapped texture+envmap pass shader
struct SVertex_Lightmap_Texture_EnvBumpMap
{
	LTVector m_vPos;
	LTVector m_vNormal;
	float m_fU0, m_fV0, m_fW0;
	float m_fU1, m_fV1, m_fW1;
};

struct CSection_Lightmap_Texture_EnvBumpMap
{
	CSection_Lightmap_Texture_EnvBumpMap()
		: m_pTexture(NULL),
		  m_bFullbright(false)
	{
	}

	bool operator<(const CSection_Lightmap_Texture_EnvBumpMap &cOther) const { return m_pTexture < cOther.m_pTexture; }
	bool operator!=(const CSection_Lightmap_Texture_EnvBumpMap &cOther) const { return !(*this == cOther); }
	bool operator==(const CSection_Lightmap_Texture_EnvBumpMap &cOther) const {
		return
			(m_pTexture == cOther.m_pTexture) &&
			(m_bFullbright == cOther.m_bFullbright);
	}
	void SetTexture(SharedTexture *pTexture) { m_pTexture = pTexture; }
	SharedTexture *GetTexture() { return m_pTexture; }

	bool ShouldAlphaTest() const { return false; }
	bool ShouldGlow() const { return m_bFullbright; }

	SharedTexture *m_pTexture;
	bool	m_bFullbright;
};

enum { k_SVertex_Lightmap_Texture_EnvBumpMap_FVF = D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_TEX2 | D3DFVF_TEXCOORDSIZE3(0) | D3DFVF_TEXCOORDSIZE3(1) };

class CRenderShader_Lightmap_Texture_EnvBumpMap :
	public CRenderShader_Base<
		SVertex_Lightmap_Texture_EnvBumpMap,
		CSection_Lightmap_Texture_EnvBumpMap,
		k_SVertex_Lightmap_Texture_EnvBumpMap_FVF>
{
public:
	~CRenderShader_Lightmap_Texture_EnvBumpMap() { s_bValidateRequired = true; }

	virtual bool ValidateShader(const CRBSection &cSection);

	// Count the memory used by this shader
	virtual void GetMemStats(CMemStats_World &cMemStats) const;

	//Support for determining the ID of this shader
	virtual ERenderShader GetShaderID() const { return eShader_Lightmap_Texture_EnvBumpMap; }

protected:

	// Draw the sections
	virtual void DrawNormal(const DrawState &cState, uint32 nRenderBlock);

	virtual void PreFlush();
	virtual void FlushChangeSection(CInternalSection &cSection);
	virtual void PreFlushBlock(CInternalSection &cSection);
	virtual void PostFlushBlock(CInternalSection &cSection,
		uint32 nStartIndex, uint32 nEndIndex,
		uint32 nStartVertex, uint32 nEndVertex);
	virtual void PostFlush();

	// Translate a block of vertices from SRBVertex to SVertex
	virtual void TranslateVertices(SVertex_Lightmap_Texture_EnvBumpMap *pOut, const SRBVertex *pIn, uint32 nCount);

	// Fill in an internal section from a CRBSection
	virtual void FillSection(CSection_Lightmap_Texture_EnvBumpMap &cInternalSection, const CRBSection &cSection);
private:
	// Which stage each appropriate thing is installed on
	enum {
		knTexStage = 2,
		knBumpStage = 0,
		knEnvMapStage = 1
	};

	static bool s_bValidateRequired, s_bValidateResult;

	uint32 m_nFlushStateAlpha0;
	uint32 m_nFlushStateAlpha1;
	uint32 m_nFlushStateAlpha2;
	uint32 m_nFlushStateFogColor;
	RTexture *m_pCurEnvMap;
};

// Lightmapped dual texture pass shader
struct SVertex_Lightmap_Texture_DualTexture
{
	LTVector m_vPos;
	uint32 m_nColor;
	float m_fU0, m_fV0, m_fW0;
	float m_fU1, m_fV1, m_fW1;
};

struct CSection_Lightmap_Texture_DualTexture
{
	CSection_Lightmap_Texture_DualTexture()
		: m_pTexture0(NULL),
		  m_pTexture1(NULL),
		  m_bAdditive(false)
	{
	}

	bool operator<(const CSection_Lightmap_Texture_DualTexture &cOther) const { return m_pTexture0 < cOther.m_pTexture0; }
	bool operator!=(const CSection_Lightmap_Texture_DualTexture &cOther) const { return !(*this == cOther); }
	bool operator==(const CSection_Lightmap_Texture_DualTexture &cOther) const {
		return
			(m_pTexture0 == cOther.m_pTexture0) &&
			(m_pTexture1 == cOther.m_pTexture1) &&
			(m_bAdditive == cOther.m_bAdditive);
	}
	void SetTexture(SharedTexture *pTexture) { m_pTexture0 = pTexture; }
	SharedTexture *GetTexture() { return m_pTexture0; }

	bool ShouldAlphaTest() const { return false; }
	bool ShouldGlow() const { return false; }

	SharedTexture	*m_pTexture0;
	SharedTexture	*m_pTexture1;
	bool			m_bAdditive;
};

enum { k_SVertex_Lightmap_Texture_DualTexture_FVF = D3DFVF_XYZ | D3DFVF_DIFFUSE | D3DFVF_TEX2 | D3DFVF_TEXCOORDSIZE3(0) | D3DFVF_TEXCOORDSIZE3(1) };

class CRenderShader_Lightmap_Texture_DualTexture :
	public CRenderShader_Base<
		SVertex_Lightmap_Texture_DualTexture,
		CSection_Lightmap_Texture_DualTexture,
		k_SVertex_Lightmap_Texture_DualTexture_FVF>
{
public:
	~CRenderShader_Lightmap_Texture_DualTexture() { s_bValidateRequired = true; }

	// Count the memory used by this shader
	virtual void GetMemStats(CMemStats_World &cMemStats) const;

	//Support for determining the ID of this shader
	virtual ERenderShader GetShaderID() const { return eShader_Lightmap_DualTexture; }

protected:

	// Draw the sections
	virtual void DrawNormal(const DrawState &cState, uint32 nRenderBlock);

	virtual void PreFlush();
	virtual void FlushChangeSection(CInternalSection &cSection);
	virtual void PreFlushBlock(CInternalSection &cSection);
	virtual void PostFlushBlock(CInternalSection &cSection,
		uint32 nStartIndex, uint32 nEndIndex,
		uint32 nStartVertex, uint32 nEndVertex);
	virtual void PostFlush();

	virtual bool ValidateShader(const CRBSection &cSection);

	// Translate a block of vertices from SRBVertex to SVertex
	virtual void TranslateVertices(SVertex_Lightmap_Texture_DualTexture *pOut, const SRBVertex *pIn, uint32 nCount);

	// Fill in an internal section from a CRBSection
	virtual void FillSection(CSection_Lightmap_Texture_DualTexture &cInternalSection, const CRBSection &cSection);
private:
	static bool s_bValidateRequired, s_bValidateResult;

	uint32 m_nFlushStateAlpha0;
	uint32 m_nFlushStateAlpha1;
	uint32 m_nFlushStateAlpha2;
	uint32 m_nFlushStateFogColor;

};




//
//---- CSection_Lightmap_Texture_DOT3BumpMap
//
struct SVertex_Lightmap_Texture_DOT3BumpMap
{
	LTVector m_vPos;
	LTVector m_vNormal;
   float m_fU0, m_fV0, m_fW0;
};

struct CSection_Lightmap_Texture_DOT3BumpMap
{
	bool operator<(const CSection_Lightmap_Texture_DOT3BumpMap &cOther) const { return m_pTexture < cOther.m_pTexture; }
	bool operator!=(const CSection_Lightmap_Texture_DOT3BumpMap &cOther) const { return !(*this == cOther); }
	bool operator==(const CSection_Lightmap_Texture_DOT3BumpMap &cOther) const { 
		return 
			(m_pTexture == cOther.m_pTexture) && 
//			(m_nAlphaTest == cOther.m_nAlphaTest) && 
			(m_bFullbright == cOther.m_bFullbright);
	}
	void SetTexture(SharedTexture *pTexture) { m_pTexture = pTexture; }
	SharedTexture *GetTexture() { return m_pTexture; }

	bool ShouldAlphaTest() const { return false; }
	bool ShouldGlow() const { return m_bFullbright; }

	SharedTexture *m_pTexture;
	bool	m_bFullbright;

};

enum { k_SVertex_Lightmap_Texture_DOT3BumpMap_FVF = D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_TEX1 | D3DFVF_TEXCOORDSIZE3(0)};
   
class CRenderShader_Lightmap_Texture_DOT3BumpMap : 
	public CRenderShader_Base<	SVertex_Lightmap_Texture_DOT3BumpMap, CSection_Lightmap_Texture_DOT3BumpMap, k_SVertex_Lightmap_Texture_DOT3BumpMap_FVF>
{
public:

	CRenderShader_Lightmap_Texture_DOT3BumpMap()	{}
	~CRenderShader_Lightmap_Texture_DOT3BumpMap() { s_bValidateRequired = true; }

	virtual bool ValidateShader(const CRBSection &cSection);

	// Count the memory used by this shader
	virtual void GetMemStats(CMemStats_World &cMemStats) const;

	// Support for determining the ID of this shader
	virtual ERenderShader GetShaderID() const { return eShader_Lightmap_Texture_DOT3BumpMap; }

protected:

	// Draw the sections
	virtual void DrawNormal(const DrawState &cState, uint32 nRenderBlock);

	virtual void PreFlush();
	virtual void FlushChangeSection(CInternalSection &cSection);
	virtual void PreFlushBlock(CInternalSection &cSection);
	virtual void PostFlushBlock(CInternalSection &cSection, uint32 nStartIndex, uint32 nEndIndex, uint32 nStartVertex, uint32 nEndVertex);
	virtual void PostFlush();

	// Translate a block of vertices from SRBVertex to SVertex
	virtual void TranslateVertices(SVertex_Lightmap_Texture_DOT3BumpMap *pOut, const SRBVertex *pIn, uint32 nCount);

	// Fill in an internal section from a CRBSection
	virtual void FillSection(CSection_Lightmap_Texture_DOT3BumpMap &cInternalSection, const CRBSection &cSection);

private:

	// Which stage each appropriate thing is installed on
	enum 
	{ 		
		knBumpStage = 0,     // Normal map
		knTexStage = 1			// texture
	};

	static bool s_bValidateRequired, s_bValidateResult;

	uint32 m_nFlushStateAlpha0;
	uint32 m_nFlushStateAlpha1;
	uint32 m_nFlushStateAlpha2;
	uint32 m_nFlushStateFogColor;

	// saved tfactor
	DWORD	m_nOldTFactor;

};


// Textured+EnvMap Lightmap shader
struct SVertex_Lightmap_Texture_DOT3EnvBumpMap
{
	LTVector m_vPos;
	LTVector m_vNormal;
   float m_fU0, m_fV0, m_fW0;       // DOT3 normal map uv
 	float m_fU1, m_fV1, m_fW1;			// texture uv
};




//
//---- CSection_Lightmap_Texture_DOT3EnvBumpMap
//

struct CSection_Lightmap_Texture_DOT3EnvBumpMap
{
	bool operator<(const CSection_Lightmap_Texture_DOT3EnvBumpMap &cOther) const { return m_pTexture < cOther.m_pTexture; }
	bool operator!=(const CSection_Lightmap_Texture_DOT3EnvBumpMap &cOther) const { return !(*this == cOther); }
	bool operator==(const CSection_Lightmap_Texture_DOT3EnvBumpMap &cOther) const { 
		return 
			(m_pTexture == cOther.m_pTexture) && 
//			(m_nAlphaTest == cOther.m_nAlphaTest) && 
			(m_bFullbright == cOther.m_bFullbright);
	}
	void SetTexture(SharedTexture *pTexture) { m_pTexture = pTexture; }
	SharedTexture *GetTexture() { return m_pTexture; }

	bool ShouldAlphaTest() const { return false; }
	bool ShouldGlow() const { return m_bFullbright; }

	SharedTexture *m_pTexture;
	bool	m_bFullbright;

};

enum { k_SVertex_Lightmap_Texture_DOT3EnvBumpMap_FVF = D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_TEX2 | D3DFVF_TEXCOORDSIZE3(0) | D3DFVF_TEXCOORDSIZE3(1) };
   
class CRenderShader_Lightmap_Texture_DOT3EnvBumpMap : 
	public CRenderShader_Base<
		SVertex_Lightmap_Texture_DOT3EnvBumpMap, 
		CSection_Lightmap_Texture_DOT3EnvBumpMap, 
		k_SVertex_Lightmap_Texture_DOT3EnvBumpMap_FVF>
{
public:

	CRenderShader_Lightmap_Texture_DOT3EnvBumpMap()	{}
	~CRenderShader_Lightmap_Texture_DOT3EnvBumpMap() { s_bValidateRequired = true; }

	virtual bool ValidateShader(const CRBSection &cSection);

	// Count the memory used by this shader
	virtual void GetMemStats(CMemStats_World &cMemStats) const;

	//Support for determining the ID of this shader
	virtual ERenderShader GetShaderID() const { return eShader_Lightmap_Texture_DOT3EnvBumpMap; }

protected:

	// Draw the sections
	virtual void DrawNormal(const DrawState &cState, uint32 nRenderBlock);

	virtual void PreFlush();
	virtual void FlushChangeSection(CInternalSection &cSection);
	virtual void PreFlushBlock(CInternalSection &cSection);
	virtual void PostFlushBlock(CInternalSection &cSection, 
		uint32 nStartIndex, uint32 nEndIndex,
		uint32 nStartVertex, uint32 nEndVertex);
	virtual void PostFlush();

	// Translate a block of vertices from SRBVertex to SVertex
	virtual void TranslateVertices(SVertex_Lightmap_Texture_DOT3EnvBumpMap *pOut, const SRBVertex *pIn, uint32 nCount);

	// Fill in an internal section from a CRBSection
	virtual void FillSection(CSection_Lightmap_Texture_DOT3EnvBumpMap &cInternalSection, const CRBSection &cSection);

private:
	// Which stage each appropriate thing is installed on
	enum { 
		knTexStage = 1,
		knBumpStage = 0,
		knEnvMapStage = 2
	};

	static bool s_bValidateRequired, s_bValidateResult; // Hack for dealing with naughty nVidia drivers

	// Used during flush
	uint32 m_nFlushStateAlpha0;
	uint32 m_nFlushStateAlpha1;
	uint32 m_nFlushStateAlpha2;
	uint32 m_nFlushStateFogColor;


	RTexture *m_pCurEnvMap;

	// saved tfactor
	DWORD	m_nOldTFactor;


};


#endif //__D3D_RENDERSHADER_LIGHTMAP_H__

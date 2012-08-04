//////////////////////////////////////////////////////////////////////////////
// Plain gouraud render shader implementation

#ifndef __D3D_RENDERSHADER_GOURAUD_H__
#define __D3D_RENDERSHADER_GOURAUD_H__

#include "d3d_rendershader_base.h"

class RTexture;

// Plain gouraud shader
struct SVertex_Gouraud
{
	LTVector m_vPos;
	uint32 m_nColor;
};

struct CSection_Gouraud
{
	bool operator<(const CSection_Gouraud &cOther) const { return false; }
	bool operator!=(const CSection_Gouraud &cOther) const { return !(*this == cOther); }
	bool operator==(const CSection_Gouraud &cOther) const { return true; }
	void SetTexture(SharedTexture *pTexture) { }
	SharedTexture *GetTexture() { return 0; }

	bool ShouldAlphaTest() const { return false; }
	bool ShouldGlow() const { return false; }
	
};

enum { k_SVertex_Gouraud_FVF = D3DFVF_XYZ | D3DFVF_DIFFUSE };

class CRenderShader_Gouraud : public CRenderShader_Base<SVertex_Gouraud, CSection_Gouraud, k_SVertex_Gouraud_FVF>
{
public:

	virtual bool ValidateShader(const CRBSection &cSection);

	// Count the memory used by this shader
	virtual void GetMemStats(CMemStats_World &cMemStats) const;

	//Support for determining the ID of this shader
	virtual ERenderShader GetShaderID() const { return eShader_Gouraud; }

protected:

	// Draw the sections
	virtual void DrawNormal(const DrawState &cState, uint32 nRenderBlock);

	void PreFlush();

	// Translate a block of vertices from SRBVertex to SVertex
	virtual void TranslateVertices(SVertex_Gouraud *pOut, const SRBVertex *pIn, uint32 nCount);

	// Fill in an internal section from a CRBSection
	virtual void FillSection(CSection_Gouraud &cInternalSection, const CRBSection &cSection) {}

	// Draw the lighting for the sections
	void DrawLights(const DrawState &cState, uint32 nRenderBlock);
};

// Textured gouraud shader
struct SVertex_Gouraud_Texture
{
	LTVector m_vPos;

	uint32 m_nColor;
	float m_fU, m_fV, m_fW;
};

struct CSection_Gouraud_Texture
{
	bool operator<(const CSection_Gouraud_Texture &cOther) const { return m_pTexture < cOther.m_pTexture; }
	bool operator!=(const CSection_Gouraud_Texture &cOther) const { return !(*this == cOther); }
	bool operator==(const CSection_Gouraud_Texture &cOther) const { 
		return 
			(m_pTexture == cOther.m_pTexture) && 
			(m_nAlphaTest == cOther.m_nAlphaTest) && 
			(m_bFullbright == cOther.m_bFullbright);
	}
	void SetTexture(SharedTexture *pTexture) { m_pTexture = pTexture; }
	SharedTexture *GetTexture() { return m_pTexture; }

	bool ShouldAlphaTest() const { return m_nAlphaTest != 0; }
	bool ShouldGlow() const { return m_bFullbright; }

	SharedTexture *m_pTexture;
	uint32	m_nAlphaTest;
	bool	m_bFullbright;
};

enum { k_SVertex_Gouraud_Texture_FVF = D3DFVF_XYZ | D3DFVF_DIFFUSE | D3DFVF_TEX1 | D3DFVF_TEXCOORDSIZE3(0) };

class CRenderShader_Gouraud_Texture : 
	public CRenderShader_Base<
		SVertex_Gouraud_Texture, 
		CSection_Gouraud_Texture, 
		k_SVertex_Gouraud_Texture_FVF>
{
public:
	CRenderShader_Gouraud_Texture() { }
	~CRenderShader_Gouraud_Texture() { s_bValidateRequired = true; }

	virtual bool ValidateShader(const CRBSection &cSection);

	// Count the memory used by this shader
	virtual void GetMemStats(CMemStats_World &cMemStats) const;

	// Support for determining the ID of this shader
	virtual ERenderShader GetShaderID() const { return eShader_Gouraud_Texture; }

	// This is a temporary hack
	static void Flush(const ViewParams *pParams);

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
	virtual void TranslateVertices(SVertex_Gouraud_Texture *pOut, const SRBVertex *pIn, uint32 nCount);

	// Fill in an internal section from a CRBSection
	virtual void FillSection(CSection_Gouraud_Texture &cInternalSection, const CRBSection &cSection);

	// Used during flush
	uint32 m_nOldAlphaTest;
	uint32 m_nCurAlphaTest;

private:
	static bool s_bValidateRequired, s_bValidateResult;
};

// Textured+detail gouraud shader
struct SVertex_Gouraud_Detail
{
	LTVector m_vPos;
	uint32 m_nColor;
 	float m_fU0, m_fV0, m_fW0;
 	float m_fU1, m_fV1, m_fW1;
};

struct CSection_Gouraud_Detail
{
	bool operator<(const CSection_Gouraud_Detail &cOther) const { return m_pTexture < cOther.m_pTexture; }
	bool operator!=(const CSection_Gouraud_Detail &cOther) const { return !(*this == cOther); }
	bool operator==(const CSection_Gouraud_Detail &cOther) const { 
		return 
			(m_pTexture == cOther.m_pTexture) && 
			(m_nAlphaTest == cOther.m_nAlphaTest) && 
			(m_bFullbright == cOther.m_bFullbright);
	}
	void SetTexture(SharedTexture *pTexture) { m_pTexture = pTexture; }
	SharedTexture *GetTexture() { return m_pTexture; }

	bool ShouldAlphaTest() const { return m_nAlphaTest != 0; }
	bool ShouldGlow() const { return m_bFullbright; }

	SharedTexture *m_pTexture;
	uint32	m_nAlphaTest;
	bool	m_bFullbright;
};

enum { k_SVertex_Gouraud_Detail_FVF = D3DFVF_XYZ | D3DFVF_DIFFUSE | D3DFVF_TEX2 | D3DFVF_TEXCOORDSIZE3(0) | D3DFVF_TEXCOORDSIZE3(1) };

class CRenderShader_Gouraud_Detail : 
	public CRenderShader_Base<
		SVertex_Gouraud_Detail, 
		CSection_Gouraud_Detail, 
		k_SVertex_Gouraud_Detail_FVF>
{
public:

	CRenderShader_Gouraud_Detail() { }
	~CRenderShader_Gouraud_Detail() { s_bValidateRequired = true; }

	virtual bool ValidateShader(const CRBSection &cSection);

	// Count the memory used by this shader
	virtual void GetMemStats(CMemStats_World &cMemStats) const;

	//Support for determining the ID of this shader
	virtual ERenderShader GetShaderID() const { return eShader_Gouraud_Detail; }

	// This is a temporary hack
	static void Flush(const ViewParams *pParams);

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
	virtual void TranslateVertices(SVertex_Gouraud_Detail *pOut, const SRBVertex *pIn, uint32 nCount);

	// Fill in an internal section from a CRBSection
	virtual void FillSection(CSection_Gouraud_Detail &cInternalSection, const CRBSection &cSection);

	// Used during flush
	uint32 m_nOldAlphaTest;
	uint32 m_nCurAlphaTest;

private:

	static bool s_bValidateRequired, s_bValidateResult, s_bDisableAlpha; // Hack for dealing with naughty nVidia drivers
};

// Textured+EnvMap gouraud shader
struct SVertex_Gouraud_EnvMap
{
	LTVector m_vPos;
	LTVector m_vNormal;
	uint32 m_nColor;
	float m_fU, m_fV, m_fW;
};

struct CSection_Gouraud_EnvMap
{
	bool operator<(const CSection_Gouraud_EnvMap &cOther) const { return m_pTexture < cOther.m_pTexture; }
	bool operator!=(const CSection_Gouraud_EnvMap &cOther) const { return !(*this == cOther); }
	bool operator==(const CSection_Gouraud_EnvMap &cOther) const { 
		return 
			(m_pTexture == cOther.m_pTexture) && 
			(m_nAlphaTest == cOther.m_nAlphaTest) && 
			(m_bFullbright == cOther.m_bFullbright);
	}
	void SetTexture(SharedTexture *pTexture) { m_pTexture = pTexture; }
	SharedTexture *GetTexture() { return m_pTexture; }

	bool ShouldAlphaTest() const { return m_nAlphaTest != 0; }
	bool ShouldGlow() const { return m_bFullbright; }

	SharedTexture *m_pTexture;
	uint32	m_nAlphaTest;
	bool	m_bFullbright;
};

enum { k_SVertex_Gouraud_EnvMap_FVF = D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_DIFFUSE | D3DFVF_TEX1 | D3DFVF_TEXCOORDSIZE3(0) };

class CRenderShader_Gouraud_EnvMap : 
	public CRenderShader_Base<
		SVertex_Gouraud_EnvMap, 
		CSection_Gouraud_EnvMap, 
		k_SVertex_Gouraud_EnvMap_FVF>
{
public:

	CRenderShader_Gouraud_EnvMap(bool bAlphaMask = false);
	~CRenderShader_Gouraud_EnvMap() { s_bValidateRequired = true; }

	virtual bool ValidateShader(const CRBSection &cSection);

	// Count the memory used by this shader
	virtual void GetMemStats(CMemStats_World &cMemStats) const;

	//Support for determining the ID of this shader
	virtual ERenderShader GetShaderID() const { return m_bAlphaMask ? eShader_Gouraud_Alpha_EnvMap : eShader_Gouraud_EnvMap; }

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
	virtual void TranslateVertices(SVertex_Gouraud_EnvMap *pOut, const SRBVertex *pIn, uint32 nCount);

	// Fill in an internal section from a CRBSection
	virtual void FillSection(CSection_Gouraud_EnvMap &cInternalSection, const CRBSection &cSection);

	// Used during flush
	uint32 m_nOldAlphaTest;
	uint32 m_nCurAlphaTest;
	RTexture *m_pCurEnvMap;

private:
	// Determines if this shader should render the environment map masked with
	// the texture's alpha mask
	bool		m_bAlphaMask;

	// Determines if the alpha masked environment map passed validation
	static bool s_bValidAlphaMask;
	static bool s_bAlphaMaskUseDualPass;
	static bool s_bValidateRequired, s_bValidateResult; // Hack for dealing with naughty nVidia drivers
	static bool s_bUseDualPass;
};

// Textured+EnvMap gouraud shader
struct SVertex_Gouraud_EnvBumpMap
{
	LTVector m_vPos;
	LTVector m_vNormal;
	uint32 m_nColor;
    float m_fU0, m_fV0, m_fW0;
 	float m_fU1, m_fV1, m_fW1;
};

struct CSection_Gouraud_EnvBumpMap
{
	bool operator<(const CSection_Gouraud_EnvBumpMap &cOther) const { return m_pTexture < cOther.m_pTexture; }
	bool operator!=(const CSection_Gouraud_EnvBumpMap &cOther) const { return !(*this == cOther); }
	bool operator==(const CSection_Gouraud_EnvBumpMap &cOther) const { 
		return 
			(m_pTexture == cOther.m_pTexture) && 
			(m_nAlphaTest == cOther.m_nAlphaTest) && 
			(m_bFullbright == cOther.m_bFullbright);
	}
	void SetTexture(SharedTexture *pTexture) { m_pTexture = pTexture; }
	SharedTexture *GetTexture() { return m_pTexture; }

	bool ShouldAlphaTest() const { return m_nAlphaTest != 0; }
	bool ShouldGlow() const { return m_bFullbright; }

	SharedTexture *m_pTexture;
	uint32	m_nAlphaTest;
	bool	m_bFullbright;
};

enum { k_SVertex_Gouraud_EnvBumpMap_FVF = D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_DIFFUSE | D3DFVF_TEX2 | D3DFVF_TEXCOORDSIZE3(0) | D3DFVF_TEXCOORDSIZE3(1) };
   
class CRenderShader_Gouraud_EnvBumpMap : 
	public CRenderShader_Base<
		SVertex_Gouraud_EnvBumpMap, 
		CSection_Gouraud_EnvBumpMap, 
		k_SVertex_Gouraud_EnvBumpMap_FVF>
{
public:

	CRenderShader_Gouraud_EnvBumpMap()	{}
	~CRenderShader_Gouraud_EnvBumpMap() { s_bValidateRequired = true; }

	virtual bool ValidateShader(const CRBSection &cSection);

	// Count the memory used by this shader
	virtual void GetMemStats(CMemStats_World &cMemStats) const;

	//Support for determining the ID of this shader
	virtual ERenderShader GetShaderID() const { return eShader_Gouraud_EnvBumpMap; }

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
	virtual void TranslateVertices(SVertex_Gouraud_EnvBumpMap *pOut, const SRBVertex *pIn, uint32 nCount);

	// Fill in an internal section from a CRBSection
	virtual void FillSection(CSection_Gouraud_EnvBumpMap &cInternalSection, const CRBSection &cSection);

private:
	// Which stage each appropriate thing is installed on
	enum { 
		knTexStage = 2,
		knBumpStage = 0,
		knEnvMapStage = 1
	};

	static bool s_bValidateRequired, s_bValidateResult; // Hack for dealing with naughty nVidia drivers

	// Used during flush
	uint32 m_nOldAlphaTest;
	uint32 m_nCurAlphaTest;
	RTexture *m_pCurEnvMap;
};

// Dual textured gouraud shader
struct SVertex_Gouraud_DualTexture
{
	LTVector m_vPos;
	uint32 m_nColor;
 	float m_fU0, m_fV0, m_fW0;
 	float m_fU1, m_fV1, m_fW1;
};

struct CSection_Gouraud_DualTexture
{
	bool operator<(const CSection_Gouraud_DualTexture &cOther) const { return m_pTexture0 < cOther.m_pTexture0; }
	bool operator!=(const CSection_Gouraud_DualTexture &cOther) const { return !(*this == cOther); }
	bool operator==(const CSection_Gouraud_DualTexture &cOther) const { 
		return 
			(m_pTexture0 == cOther.m_pTexture0) && 
			(m_pTexture1 == cOther.m_pTexture1) && 
			(m_nAlphaTest == cOther.m_nAlphaTest) && 
			(m_bAdditive == cOther.m_bAdditive);
	}
	void SetTexture(SharedTexture *pTexture) { m_pTexture0 = pTexture; }
	SharedTexture *GetTexture() { return m_pTexture0; }

	bool ShouldAlphaTest() const { return m_nAlphaTest != 0; }
	bool ShouldGlow() const { return false; }

	SharedTexture	*m_pTexture0;
	SharedTexture	*m_pTexture1;
	uint32			m_nAlphaTest;
	bool			m_bAdditive;
};

enum { k_SVertex_Gouraud_DualTexture_FVF = D3DFVF_XYZ | D3DFVF_DIFFUSE | D3DFVF_TEX2 | D3DFVF_TEXCOORDSIZE3(0) | D3DFVF_TEXCOORDSIZE3(1) };

class CRenderShader_Gouraud_DualTexture : 
	public CRenderShader_Base<
		SVertex_Gouraud_DualTexture, 
		CSection_Gouraud_DualTexture, 
		k_SVertex_Gouraud_DualTexture_FVF>
{
public:

	CRenderShader_Gouraud_DualTexture() { }
	~CRenderShader_Gouraud_DualTexture() { s_bValidateRequired = true; }

	virtual bool ValidateShader(const CRBSection &cSection);

	// Count the memory used by this shader
	virtual void GetMemStats(CMemStats_World &cMemStats) const;

	//Support for determining the ID of this shader
	virtual ERenderShader GetShaderID() const { return eShader_Gouraud_DualTexture; }

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
	virtual void TranslateVertices(SVertex_Gouraud_DualTexture *pOut, const SRBVertex *pIn, uint32 nCount);

	// Fill in an internal section from a CRBSection
	virtual void FillSection(CSection_Gouraud_DualTexture &cInternalSection, const CRBSection &cSection);

private:

	static bool s_bValidateRequired, s_bValidateResult, s_bTwoPass;

	// Used during flush
	uint32 m_nOldAlphaTest;
	uint32 m_nCurAlphaTest;
	uint32 m_nOldColorOp1;
	

};




//
//---- CSection_Gouraud_DOT3BumpMap
//
struct SVertex_Gouraud_DOT3BumpMap
{
	LTVector m_vPos;
	LTVector m_vNormal;
	uint32 m_nColor;
   float m_fU0, m_fV0, m_fW0;
};

struct CSection_Gouraud_DOT3BumpMap
{
	bool operator<(const CSection_Gouraud_DOT3BumpMap &cOther) const { return m_pTexture < cOther.m_pTexture; }
	bool operator!=(const CSection_Gouraud_DOT3BumpMap &cOther) const { return !(*this == cOther); }
	bool operator==(const CSection_Gouraud_DOT3BumpMap &cOther) const { 
		return 
			(m_pTexture == cOther.m_pTexture) && 
			(m_nAlphaTest == cOther.m_nAlphaTest) && 
			(m_bFullbright == cOther.m_bFullbright);
	}
	void SetTexture(SharedTexture *pTexture) { m_pTexture = pTexture; }
	SharedTexture *GetTexture() { return m_pTexture; }

	bool ShouldAlphaTest() const { return m_nAlphaTest != 0; }
	bool ShouldGlow() const { return m_bFullbright; }

	SharedTexture * m_pTexture;
	uint32	m_nAlphaTest;
	bool	m_bFullbright;
};

enum { k_SVertex_Gouraud_DOT3BumpMap_FVF = D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_DIFFUSE | D3DFVF_TEX1 | D3DFVF_TEXCOORDSIZE3(0)};
   
class CRenderShader_Gouraud_DOT3BumpMap : 
	public CRenderShader_Base<	SVertex_Gouraud_DOT3BumpMap, CSection_Gouraud_DOT3BumpMap, k_SVertex_Gouraud_DOT3BumpMap_FVF>
{
public:

	CRenderShader_Gouraud_DOT3BumpMap()	{}
	~CRenderShader_Gouraud_DOT3BumpMap() { s_bValidateRequired = true; }

	virtual bool ValidateShader(const CRBSection &cSection);

	// Count the memory used by this shader
	virtual void GetMemStats(CMemStats_World &cMemStats) const;

	// Support for determining the ID of this shader
	virtual ERenderShader GetShaderID() const { return eShader_Gouraud_DOT3BumpMap; }

protected:

	// Draw the sections
	virtual void DrawNormal(const DrawState &cState, uint32 nRenderBlock);

	virtual void PreFlush();
	virtual void FlushChangeSection(CInternalSection &cSection);
	virtual void PreFlushBlock(CInternalSection &cSection);
	virtual void PostFlushBlock(CInternalSection &cSection, uint32 nStartIndex, uint32 nEndIndex, uint32 nStartVertex, uint32 nEndVertex);
	virtual void PostFlush();

	// Translate a block of vertices from SRBVertex to SVertex
	virtual void TranslateVertices(SVertex_Gouraud_DOT3BumpMap *pOut, const SRBVertex *pIn, uint32 nCount);

	// Fill in an internal section from a CRBSection
	virtual void FillSection(CSection_Gouraud_DOT3BumpMap &cInternalSection, const CRBSection &cSection);

private:

	// Which stage each appropriate thing is installed on
	enum 
	{ 		
		knBumpStage = 0,     // Normal map
		knTexStage = 1			// texture
	};

	static bool s_bValidateRequired, s_bValidateResult;

	// Used during flush
	uint32 m_nOldAlphaTest;
	uint32 m_nCurAlphaTest;

	// saved tfactor
	DWORD	m_nOldTFactor;

};


// Textured+EnvMap gouraud shader
struct SVertex_Gouraud_DOT3EnvBumpMap
{
	LTVector m_vPos;
	LTVector m_vNormal;
	uint32 m_nColor;
   float m_fU0, m_fV0, m_fW0;       // DOT3 normal map uv
 	float m_fU1, m_fV1, m_fW1;			// texture uv
};




//
//---- CSection_Gouraud_DOT3EnvBumpMap
//

struct CSection_Gouraud_DOT3EnvBumpMap
{
	bool operator<(const CSection_Gouraud_DOT3EnvBumpMap &cOther) const { return m_pTexture < cOther.m_pTexture; }
	bool operator!=(const CSection_Gouraud_DOT3EnvBumpMap &cOther) const { return !(*this == cOther); }
	bool operator==(const CSection_Gouraud_DOT3EnvBumpMap &cOther) const { 
		return 
			(m_pTexture == cOther.m_pTexture) && 
			(m_nAlphaTest == cOther.m_nAlphaTest) && 
			(m_bFullbright == cOther.m_bFullbright);
	}
	void SetTexture(SharedTexture *pTexture) { m_pTexture = pTexture; }
	SharedTexture *GetTexture() { return m_pTexture; }

	bool ShouldAlphaTest() const { return m_nAlphaTest != 0; }
	bool ShouldGlow() const { return m_bFullbright; }

	SharedTexture *m_pTexture;
	uint32	m_nAlphaTest;
	bool	m_bFullbright;
};

enum { k_SVertex_Gouraud_DOT3EnvBumpMap_FVF = D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_DIFFUSE | D3DFVF_TEX2 | D3DFVF_TEXCOORDSIZE3(0) | D3DFVF_TEXCOORDSIZE3(1) };
   
class CRenderShader_Gouraud_DOT3EnvBumpMap : 
	public CRenderShader_Base<
		SVertex_Gouraud_DOT3EnvBumpMap, 
		CSection_Gouraud_DOT3EnvBumpMap, 
		k_SVertex_Gouraud_DOT3EnvBumpMap_FVF>
{
public:

	CRenderShader_Gouraud_DOT3EnvBumpMap()	{}
	~CRenderShader_Gouraud_DOT3EnvBumpMap() { s_bValidateRequired = true; }

	virtual bool ValidateShader(const CRBSection &cSection);

	// Count the memory used by this shader
	virtual void GetMemStats(CMemStats_World &cMemStats) const;

	//Support for determining the ID of this shader
	virtual ERenderShader GetShaderID() const { return eShader_Gouraud_DOT3EnvBumpMap; }

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
	virtual void TranslateVertices(SVertex_Gouraud_DOT3EnvBumpMap *pOut, const SRBVertex *pIn, uint32 nCount);

	// Fill in an internal section from a CRBSection
	virtual void FillSection(CSection_Gouraud_DOT3EnvBumpMap &cInternalSection, const CRBSection &cSection);

private:
	// Which stage each appropriate thing is installed on
	enum { 
		knTexStage = 1,
		knBumpStage = 0,
		knEnvMapStage = 2
	};

	static bool s_bValidateRequired, s_bValidateResult; // Hack for dealing with naughty nVidia drivers

	// Used during flush
	uint32 m_nOldAlphaTest;
	uint32 m_nCurAlphaTest;
	RTexture *m_pCurEnvMap;

	// saved tfactor
	DWORD	m_nOldTFactor;


};

// Textured gouraud shader
struct SVertex_Gouraud_Effect
{
	LTVector m_vPos;
	LTVector m_vNormal;
	LTVector m_vTangent;
	LTVector m_vBinormal;
	uint32 m_nColor;
	float m_fU, m_fV, m_fW;
};

struct CSection_Gouraud_Effect
{
	bool operator<(const CSection_Gouraud_Effect &cOther) const { return m_pTexture < cOther.m_pTexture; }
	bool operator!=(const CSection_Gouraud_Effect &cOther) const { return !(*this == cOther); }
	bool operator==(const CSection_Gouraud_Effect &cOther) const { 
		return 
			(m_pTexture == cOther.m_pTexture) && 
			(m_nAlphaTest == cOther.m_nAlphaTest) && 
			(m_bFullbright == cOther.m_bFullbright);
	}
	void SetTexture(SharedTexture *pTexture) { m_pTexture = pTexture; }
	SharedTexture *GetTexture() { return m_pTexture; }

	bool ShouldAlphaTest() const { return m_nAlphaTest != 0; }
	bool ShouldGlow() const { return m_bFullbright; }

	SharedTexture *m_pTexture;
	uint32	m_nAlphaTest;
	bool	m_bFullbright;
};

enum { k_SVertex_Gouraud_Effect_FVF = D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_DIFFUSE | D3DFVF_TEX1 | D3DFVF_TEXCOORDSIZE3(0) };

class CRenderShader_Gouraud_Effect : 
	public CRenderShader_Base<
	SVertex_Gouraud_Effect, 
	CSection_Gouraud_Effect, 
	k_SVertex_Gouraud_Effect_FVF>
{
public:
	CRenderShader_Gouraud_Effect();
	~CRenderShader_Gouraud_Effect();

	virtual bool ValidateShader(const CRBSection &cSection);

	// Count the memory used by this shader
	virtual void GetMemStats(CMemStats_World &cMemStats) const;

	// Support for determining the ID of this shader
	virtual ERenderShader GetShaderID() const { return eShader_Gouraud_Effect; }

	// This is a temporary hack
	static void Flush(const ViewParams *pParams);

protected:

	// Draw the sections
	virtual void DrawNormal(const DrawState &cState, uint32 nRenderBlock);

	virtual void PreFlush();
	virtual void FlushChangeSection(CInternalSection &cSection);
	virtual void PreFlushBlock(CInternalSection &cSection);
	virtual bool FlushWithEffect(CInternalSection &cSection, 
		uint32 nStartIndex, uint32 nEndIndex,
		uint32 nStartVertex, uint32 nEndVertex);
	virtual void PostFlushBlock(CInternalSection &cSection, 
		uint32 nStartIndex, uint32 nEndIndex,
		uint32 nStartVertex, uint32 nEndVertex);
	virtual void PostFlush();
	virtual bool HasEffect(CInternalSection &cSection)
	{
		if(cSection.m_pTexture && (cSection.m_pTexture->m_nShaderID != 0))
		{
			return true;
		}else
		{
			return false;
		}
	}

	// Translate a block of vertices from SRBVertex to SVertex
	virtual void TranslateVertices(SVertex_Gouraud_Effect *pOut, const SRBVertex *pIn, uint32 nCount);

	// Fill in an internal section from a CRBSection
	virtual void FillSection(CSection_Gouraud_Effect &cInternalSection, const CRBSection &cSection);

	// Used during flush
	uint32 m_nOldAlphaTest;
	uint32 m_nCurAlphaTest;

private:
	static bool s_bValidateRequired, s_bValidateResult;
};


#endif //__D3D_RENDERSHADER_GOURAUD_H__

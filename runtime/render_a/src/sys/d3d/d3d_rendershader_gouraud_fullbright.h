//////////////////////////////////////////////////////////////////////////////
// Gouraud+fullbright render shader implementations

#ifndef __D3D_RENDERSHADER_GOURAUD_FULLBRIGHT_H__
#define __D3D_RENDERSHADER_GOURAUD_FULLBRIGHT_H__

#include "d3d_rendershader_gouraud.h"

class CRenderShader_Gouraud_Texture_Fullbright : public CRenderShader_Gouraud_Texture
{
public:

	virtual bool ValidateShader(const CRBSection &cSection);

	virtual ERenderShader GetShaderID() const { return eShader_Gouraud_Texture_Fullbright; }

protected:
	virtual void DrawNormal(const DrawState &cState, uint32 nRenderBlock);

	virtual void PreFlush();
	virtual void FlushChangeSection(CInternalSection &cSection);
	virtual void PreFlushBlock(CInternalSection &cSection);
	virtual void PostFlushBlock(CInternalSection &cSection, 
		uint32 nStartIndex, uint32 nEndIndex,
		uint32 nStartVertex, uint32 nEndVertex);
	virtual void PostFlush();

private:
	static bool s_bValidateRequired, s_bValidateResult;
};

class CRenderShader_Gouraud_Detail_Fullbright : public CRenderShader_Gouraud_Detail
{
public:

	virtual bool ValidateShader(const CRBSection &cSection);

	virtual ERenderShader GetShaderID() const { return eShader_Gouraud_Detail_Fullbright; }

protected:
	virtual void DrawNormal(const DrawState &cState, uint32 nRenderBlock);

	virtual void PreFlush();
	virtual void FlushChangeSection(CInternalSection &cSection);
	virtual void PreFlushBlock(CInternalSection &cSection);
	virtual void PostFlushBlock(CInternalSection &cSection, 
		uint32 nStartIndex, uint32 nEndIndex,
		uint32 nStartVertex, uint32 nEndVertex);
	virtual void PostFlush();

private:
	static bool s_bValidateRequired, s_bValidateResult;
};

class CRenderShader_Gouraud_EnvMap_Fullbright : public CRenderShader_Gouraud_EnvMap
{
public:

	virtual bool ValidateShader(const CRBSection &cSection);

	virtual ERenderShader GetShaderID() const { return eShader_Gouraud_EnvMap_Fullbright; }

protected:
	virtual void DrawNormal(const DrawState &cState, uint32 nRenderBlock);

	virtual void PreFlush();
	virtual void FlushChangeSection(CInternalSection &cSection);
	virtual void PreFlushBlock(CInternalSection &cSection);
	virtual void PostFlushBlock(CInternalSection &cSection, 
		uint32 nStartIndex, uint32 nEndIndex,
		uint32 nStartVertex, uint32 nEndVertex);
	virtual void PostFlush();

private:
	static bool s_bValidateRequired, s_bValidateResult;
};

#endif //__D3D_RENDERSHADER_GOURAUD_FULLBRIGHT_H__
#ifndef _LT_EFFECTIMPL_H_
#define _LT_EFFECTIMPL_H_

#include "ltbasedefs.h"

struct ID3DXEffect;

class LTEffectImpl : public LTEffectShader
{
public:
	LTEffectImpl();
	~LTEffectImpl();
	bool Init(ILTStream *pStream, const uint32 *pVertexElements, uint32 VertexElementsSize, HEFFECTPOOL EffectPoolID);
	void Term();
	void FreeDeviceObject();
	bool Recreate();

	// id
	void						SetID(int ShaderID)		{ m_ShaderID = ShaderID; }

	// name
	void						SetName(const char *pName);

	// next pointer
	void						SetNext(LTEffectImpl *pNext)	{ m_pNext = pNext; }

	virtual LTRESULT SetBool(const char* szParam, LTBOOL bBool) const; 
	virtual LTRESULT SetBoolArray(const char* szParam, LTBOOL *bBool, int nCount) const; 
	virtual LTRESULT SetFloat(const char* szParam, float fFloat) const; 
	virtual LTRESULT SetFloatArray(const char* szParam, float *fFloat, int nCount) const; 
	virtual LTRESULT SetInt(const char* szParam, int nInt) const; 
	virtual LTRESULT SetIntArray(const char* szParam, int *nInt, int nCount) const; 
	virtual LTRESULT SetMatrix(const char* szParam, LTMatrix &mMatrix) const; 
	virtual LTRESULT SetMatrixArray(const char* szParam, LTMatrix *mMatrix, int nCount) const; 
	//TODO
	//SetMatrixPointerArray Sets an array of pointers to nontransposed matrices. 
	virtual LTRESULT SetMatrixTranspose(const char* szParam, LTMatrix &mMatrix) const; 
	virtual LTRESULT SetMatrixTransposeArray(const char* szParam, LTMatrix *mMatrix, int nCount) const; 
	//TODO
	//SetMatrixTransposePointerArray Sets an array of pointers to transposed matrices. 
	//SetString Sets a string. 
	virtual LTRESULT SetString(const char* szParam, const char* szString) const; 
	//SetTechnique Sets the active technique. 
	virtual LTRESULT SetTechnique(const char* szTechnique) const; 
	virtual LTRESULT ValidateTechnique(const char* szTechnique) const;
	virtual LTRESULT FindFirstValidTechnique(LTTechniqueInfo* pInfo) const;
	virtual LTRESULT SetTexture(const char* szParam, HTEXTURE hTexture) const; 		
	virtual LTRESULT SetTextureRT(const char* szParam, HRENDERTARGET hRenderTarget) const; 
	//SetValue Set the value of an arbitrary parameter or annotation, including simple types, structs, arrays, strings, shaders and textures.  
	virtual LTRESULT SetVector(const char* szParam, float *fFloat) const;  //4 floats
	virtual LTRESULT SetVectorArray(const char* szParam, float *fFloat, int nCount) const; // 4 floats * nCount

	void			SetEffect(ID3DXEffect* pEffect){m_pEffect = pEffect;}
	ID3DXEffect*	GetEffect() {return m_pEffect;}

	virtual LTRESULT UploadVertexDeclaration();

	IDirect3DVertexDeclaration9* GetVertexDeclaration(){return m_pVertexDeclaration;}

protected:
	ID3DXEffect*					m_pEffect;
	D3DVERTEXELEMENT9*				m_pVertexElements;		// An array of vertex elements used to create the vertex shader declaration
	IDirect3DVertexDeclaration9* 	m_pVertexDeclaration;	// d3d vertex shader input declaration interface
	bool							m_bCompileShader;		// this flag specifies whether the shader is already compiled
	uint8*							m_pByteCode;			// byte code loaded in from the file
	unsigned						m_ByteCodeSize;			// size of byte code array
	HEFFECTPOOL						m_EffectPoolID;
};

#endif

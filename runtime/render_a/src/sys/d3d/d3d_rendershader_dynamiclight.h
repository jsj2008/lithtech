//////////////////////////////////////////////////////////////////////////////
// Dynamic light sub-shader

#ifndef __D3D_RENDERSHADER_DYNAMICLIGHT_H__
#define __D3D_RENDERSHADER_DYNAMICLIGHT_H__

/* 

Usage Notes:
	
	This sub-shader is a singleton, accessed through the GetSingleton static member function.
	Rendering is accomplished by calling SetupLight() for the light to draw, and then drawing
		all of the triangles that might touch the light.
	You must have drawn the ambient light portion of the polygons before telling the dynamic light
		shader to draw, including the Z-Buffer fill.  (For alpha cut-out support.)
	You should call Bind before using this sub-shader, and you must call Release when it is no
		longer going to be used.
	Rendering states which are changed by Bind and SetupLight are restored via ResetStates.
*/
	
class DynamicLight;
class ViewParams;
class DynamicLight;
class CMemStats_World;
struct SRBVertex;

class CRenderShader_DynamicLight
{
public:
	// Don't create these, even if this says you should be able to.
	CRenderShader_DynamicLight();
	~CRenderShader_DynamicLight();

public:
	static CRenderShader_DynamicLight *GetSingleton();

	// Bind the sub-shader to the current rendering device
	void Bind();
	// Release the sub-shader from the rendering device
	void Release();

	// Is rendering possible given the current state of affairs?
	bool IsRenderOK() const { return m_bBound && (m_eLightingMethod != eLight_Unsupported); }

	// Set up the light for a rendering pass
	// Returns false if rendering is not supported/possible
	bool SetupLight(const ViewParams *pParams, const DynamicLight *pLight);

	// Draw a list of lit triangles
	// Note : The rendering state must be set up as if this were a call to DrawIndexedPrimitive.
	// Returns the number of triangles actually drawn
	uint32 DrawTris(
		// VB range from the currently selected vertex buffer
		uint32 nVBStart, uint32 nVBCount,
		// The triangle index list
		const uint16 *pIndices,
		// Number of triangles
		uint32 nNumTris,
		// The raw vertex list
		const SRBVertex *pVertices,
		// The stride of the vertex list, in bytes
		uint32 nVertexStride,
		// The triangle index list/VB offset
		int32 nVertexOffset
	);

	// Sets the modified state values to their previous values
	void ResetStates();

	// Count the memory used by this shader
	void GetMemStats(CMemStats_World &cMemStats) const;

private:
	// Internal implementation details

	// Set up the correct render states for rendering
	void SetStates();

	// Save the current rendering state
	void SaveState();

	// Fill in the lighting textures
	void FillTextures();

	// Is the sub-shader bound to a device?
	bool m_bBound;
	// Which lighting method to use (for device compatibility)
	enum ELightingMethod { eLight_Ambient, eLight_Ambient_Inaccurate, eLight_Unsupported };
	ELightingMethod m_eLightingMethod;

	// Lighting textures
	IDirect3DTexture9 *m_pTextureXY;
	IDirect3DTexture9 *m_pTextureZ;

	// State restoration stuff
	bool m_bStatesSaved;
	enum EModifiedStates { 
		eRS_AlphaBlendEnable,
		eRS_SrcBlend,
		eRS_DestBlend,
		eRS_ZFunc,
		eRS_ZWrite,
		eRS_TextureFactor,
		eRS_AlphaTestEnable,
		eRS_AlphaRef,
		eRS_AlphaFunc,
		eRS_TSS0_CArg1,
		eRS_TSS0_CArg2,
		eRS_TSS0_COp,
		eRS_TSS0_AArg1,
		eRS_TSS0_AArg2,
		eRS_TSS0_AOp,
		eRS_TSS0_TCI,
		eRS_TSS0_TTF,
		eRS_TSS0_AddrU,
		eRS_TSS0_AddrV,
		eRS_TSS1_CArg1,
		eRS_TSS1_CArg2,
		eRS_TSS1_COp,
		eRS_TSS1_AArg1,
		eRS_TSS1_AArg2,
		eRS_TSS1_AOp,
		eRS_TSS1_TCI,
		eRS_TSS1_TTF,
		eRS_TSS1_AddrU,
		eRS_TSS1_AddrV,
		eRS_TSS2_COp,
		eRS_TSS2_AOp,
		eRS_NumStates 
	};
	DWORD m_aSavedStates[eRS_NumStates];
	IDirect3DIndexBuffer9 *m_pSavedIB;
	uint32 m_nSavedIBBase;

	// The temporary index buffer
	IDirect3DIndexBuffer9 *m_pTempIB;
	uint32 m_nTempIBIndex;
	// Number of triangles the temporary index buffer can handle
	enum { k_nTempIBTris = 4096 };
	enum { k_nTempIBSize = k_nTempIBTris * 3 };

	// The current light
	const DynamicLight *m_pCurLight;
	LTVector m_vCurLightPos;
};

#endif //__D3D_RENDERSHADER_DYNAMICLIGHT_H__

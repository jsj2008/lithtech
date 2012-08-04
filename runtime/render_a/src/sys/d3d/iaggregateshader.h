#ifndef __IAGGREGATESHADER_H__
#define __IAGGREGATESHADER_H__

#ifndef __D3D_RENDERBLOCK_H__
#	include "d3d_renderblock.h"
#endif

//Interface for all shaders that want to perform custom rendering on existing shader
//blocks. The class must be derived from this, and then the world will find all appropriate
//intersecting blocks, and for each will call the appropriate function.
//
// For anything that calls an aggregate shader, one thing it relies upon is the assumption
// that no other rendering occurs between its calls to BeginRendering and EndRendering
// this is done for performance reasons to avoid constantly setting states.
//

class IAggregateShader
{
public:

			IAggregateShader()			{}
	virtual ~IAggregateShader()			{}

	//Initialization call. This is called before any calls are made to this shader, this
	//allows it to perform operations such as initialization of vertex shaders, and other
	//states that are constant. If it returns false, nothing should call render...
	virtual bool	BeginRendering()	{ return true; }

	//called when rendering is completed. This should return false if any errors occurred
	//while rendering
	virtual bool	EndRendering()		{ return true; }


	//called to bind any device specific data to this object
	virtual bool	Bind()				{ return true; }

	//called to free any device specific data to this object
	virtual bool	Release()			{ return true; }

	//Called when the world transform changes. This will be called whenever the
	//rendering switches spaces and gives the shader a chance to transform any
	//data it is using
	virtual bool	SetWorldTransform(const LTMatrix& mInvWorldTrans) = 0;


	//called when beginning a shader. This can be used to do shader specific tasks such
	//as setting up the vertex shader, the vertex buffers, etc.
	virtual bool	BeginShader(		uint32 nVertSize,				//the size of the vertices
										uint32 nVertType,				//the type of vertex as passed to D3D
										IDirect3DVertexShader9 *pShader, // for DX9 FVF is separate
										IDirect3DVertexBuffer9 *pVB,	//pointer to the vertex buffer
										IDirect3DIndexBuffer9 *pIB,		//the index buffer
										ERenderShader eShaderID			//the ID of the shader for custom setup
								) = 0;

	//called to render a shader block. It takes in all data needed to render the underlying
	//geometry as well as the ID of the shader type so that it can perform custom rendering
	//operations upon it
	virtual bool	RenderSection(		D3DPRIMITIVETYPE nPrimType,		//primitive type being rendered, D3DPT_, must be indexed
										const uint16 *pIndices,			//the original index lists
										const SRBVertex *pVertices,		//the original vertex lists
										uint32 nTotalIndexCount,		//number of primitives being rendered
										uint32 nStartVert,				//index into the vertex buffer to begin on
										uint32 nTotalVertCount,			//number of vertices
										int32 nVertOffset,				//offset of the vertex in the original vertices list
										uint32 nStartIndex				//index into the index buffer to begin on
								) = 0;

};

#endif

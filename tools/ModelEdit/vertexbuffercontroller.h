// VertexBufferController.h

#ifndef __VERTEXBUFFERCONTROLLER_H__
#define __VERTEXBUFFERCONTROLLER_H__

#include "d3d_vertstructs.h"

#define VERTEXBUFFER_STREAMCOUNT	4

// Vertex Buffer Container Class
//	Hold your steam data
class VertexBufferController {
public:
	VertexBufferController()													{ Reset(); }
	~VertexBufferController()													{ FreeAll(); }

	enum VB_TYPE																{ eVERTSTREAM0, eVERTSTREAM1, eVERTSTREAM2, eVERTSTREAM3, eINDEX };
	enum VERTEX_BLEND_TYPE														{ eNO_WORLD_BLENDS, eNONINDEXED_B1, eNONINDEXED_B2, eNONINDEXED_B3, eINDEXED_B1, eINDEXED_B2, eINDEXED_B3 };

	bool							CreateStream(uint32 iStreamNum, int32 nVertexCount, uint32 iVertDataType, VERTEX_BLEND_TYPE VertBlendType, bool bDynamic, bool bWriteOnly);
	bool							CreateIndexBuffer(int32 iIndexCount);
	void							Reset();									// Resets back to initial conditions (doesn't try to free anything)...
	void							FreeAll();									// Frees all the member vars and resets afterwards...

	void							SetStreamSources();							// Call this before a render call - sets the streams and stuff like that (just need to call it once per group of render calls)...
	void							Render(uint32 MinIndex,uint32 MinVert,uint32 VertCount,uint32 TriCount);	// Transform, light, and render directly using the unprocessed buffer...

	bool							Lock(VB_TYPE Type,bool bDiscardContents);
	bool							UnLock(VB_TYPE Type);
	void*							getVertexData(uint32 iStreamNum)			{ return m_pVert_Data[iStreamNum]; }
	uint16*							getIndexData()								{ return m_pIndex_Data; }
	uint32							getVertexCount(uint32 iStreamNum)			{ return m_VertexCount[iStreamNum]; }
	uint32							getUVSetCount(uint32 iStreamNum)			{ return m_UVSets[iStreamNum]; }
	uint32							getVertexSize(uint32 iStreamNum)			{ return m_VertexSize[iStreamNum]; }
	uint32							getVertexSize_UVData(uint32 iStreamNum)		{ switch (m_UVSets[iStreamNum]) { case 1 : return sizeof(VSTREAM_UV1); case 2 : return sizeof(VSTREAM_UV2); case 3 : return sizeof(VSTREAM_UV3); case 4 : return sizeof(VSTREAM_UV4); } return 0; }
	uint32							getXYZSize()								{ return sizeof(float)*3; }
	uint32							getXYZBlendIndexSize(uint32 iStreamNum)		{ return m_VertexSize[iStreamNum] - getVertexSize_UVData(iStreamNum) - sizeof(float)*3; }
	uint32							getVertexFormat(uint32 iStreamNum)			{ return m_VertexFVFFlags[iStreamNum]; } 

private:
	LPDIRECT3DVERTEXBUFFER8			m_pVB_VertStream[VERTEXBUFFER_STREAMCOUNT];	// Handles to the vertex buffers...
	LPDIRECT3DINDEXBUFFER8			m_pVB_Index;

	void*							m_pVert_Data[VERTEXBUFFER_STREAMCOUNT];		// Data (for when you lock stuff)...
	uint16*							m_pIndex_Data;
	
	VERTEX_BLEND_TYPE				m_VertexBlendType[VERTEXBUFFER_STREAMCOUNT];
	uint32							m_VertexFVFFlags[VERTEXBUFFER_STREAMCOUNT];
	uint32							m_VertexSize[VERTEXBUFFER_STREAMCOUNT];
	uint32							m_VertexCount[VERTEXBUFFER_STREAMCOUNT];
	uint32							m_IndexCount;
	uint32							m_UVSets[VERTEXBUFFER_STREAMCOUNT];
	bool							m_bDynamicVB[VERTEXBUFFER_STREAMCOUNT];		// Is the VB creates as Dynamic?
};

#endif
 
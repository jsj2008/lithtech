// VertexBufferController.cpp

#include "stdafx.h"

#include "VertexBufferController.h"
#include "d3d_device.h"
#include "d3d_vertstructs.h"
#include "d3d_renderstatemgr.h"

// Resets back to initial conditions (doesn't try to free anything)...
void VertexBufferController::Reset()
{
	m_pIndex_Data			= NULL; 
	m_pVB_Index				= NULL; 
	m_IndexCount			= 0;

	for (uint32 i = 0; i < VERTEXBUFFER_STREAMCOUNT; ++i) {
		m_VertexBlendType[i]	= eNO_WORLD_BLENDS; 
		m_VertexFVFFlags[i]		= 0;
		m_VertexCount[i]		= 0; 
		m_VertexSize[i]			= 0;
		m_pVB_VertStream[i]		= NULL; 
		m_pVert_Data[i]			= NULL; }
}

// Frees all the member vars and resets afterwards...
void VertexBufferController::FreeAll()
{
	// Release the VBs...
	for (uint32 i = 0; i < VERTEXBUFFER_STREAMCOUNT; ++i) {
		if (m_pVB_VertStream[i]) { int nCount = m_pVB_VertStream[i]->Release(); m_pVB_VertStream[i] = NULL; assert(nCount == 0); } }
	if (m_pVB_Index)			 { int nCount = m_pVB_Index->Release(); m_pVB_Index = NULL; assert(nCount == 0); }

	Reset();
}

bool VertexBufferController::CreateStream(uint32 iStreamNum, int32 nVertexCount, uint32 iVertDataType, VERTEX_BLEND_TYPE VertBlendType, bool bDynamic, bool bWriteOnly)
{
	assert(m_pVB_VertStream[iStreamNum]==NULL); assert(iStreamNum < VERTEXBUFFER_STREAMCOUNT);

	// Figure out our usage and vert type...
	bool   bIsFVFVertBuffer			= true;
	uint32 iExtraData				= 0;
	uint32 iUsage					= (bDynamic ? D3DUSAGE_DYNAMIC : NULL) | (bWriteOnly ? D3DUSAGE_WRITEONLY : NULL); 
	uint32 iVertFlags				= 0;
	uint32 iBoneCount				= 0;
	if ((iVertDataType & VERTDATATYPE_POSITION) && (iVertDataType & VERTDATATYPE_NORMAL)) {
		switch (VertBlendType) {
		case eNO_WORLD_BLENDS		: iVertFlags = VSTREAM_XYZ_NORMAL_FLAGS; break;
		case eNONINDEXED_B1			: iVertFlags = VSTREAM_XYZ_NORMAL_B1_FLAGS; break;
		case eNONINDEXED_B2			: iVertFlags = VSTREAM_XYZ_NORMAL_B2_FLAGS; break;
		case eNONINDEXED_B3			: iVertFlags = VSTREAM_XYZ_NORMAL_B3_FLAGS; break;
		case eINDEXED_B1			: iVertFlags = VSTREAM_XYZ_NORMAL_B1_INDEX_FLAGS; iBoneCount = 2; break;
		case eINDEXED_B2			: iVertFlags = VSTREAM_XYZ_NORMAL_B2_INDEX_FLAGS; iBoneCount = 3; break;
		case eINDEXED_B3			: iVertFlags = VSTREAM_XYZ_NORMAL_B3_INDEX_FLAGS; iBoneCount = 4; break; 
		default						: assert(0); return false; } } 
	if (iVertDataType & VERTDATATYPE_UVSETS_1)		{ iVertFlags |= VSTREAM_UV1_FLAGS; m_UVSets[iStreamNum] = 1; }
	else if (iVertDataType & VERTDATATYPE_UVSETS_2) { iVertFlags |= VSTREAM_UV2_FLAGS; m_UVSets[iStreamNum] = 2; }
	else if (iVertDataType & VERTDATATYPE_UVSETS_3) { iVertFlags |= VSTREAM_UV3_FLAGS; m_UVSets[iStreamNum] = 3; }
	else if (iVertDataType & VERTDATATYPE_UVSETS_4) { iVertFlags |= VSTREAM_UV4_FLAGS; m_UVSets[iStreamNum] = 4; }
	if (iVertDataType & VERTDATATYPE_BASISVECTORS) {
		iExtraData					+= sizeof(float)*6;
		bIsFVFVertBuffer			= false; }

iUsage |= D3DUSAGE_SOFTWAREPROCESSING; // TODO: Check and remove...

	// Create m_pVB_Verts...
	if (bIsFVFVertBuffer) {			// It's a typical FVF buffer that can go down the standard pipe...
		m_VertexSize[iStreamNum]	= d3d_GetVertexSize(iVertFlags); }
	else {							// It's a vertex shader only type...
		m_VertexSize[iStreamNum]	= d3d_GetVertexSize(iVertFlags) + iExtraData;
		iVertFlags					= NULL; }
	uint32 iVBSizeBytes				= nVertexCount * m_VertexSize[iStreamNum];

	HRESULT hr = PD3DDEVICE->CreateVertexBuffer(iVBSizeBytes,iUsage,iVertFlags,D3DPOOL_MANAGED,&m_pVB_VertStream[iStreamNum],NULL);
	if (FAILED(hr))	
	{
		assert(0); 
		return false; 
	}

	// Set our member vars...
	m_VertexFVFFlags[iStreamNum]	= iVertFlags;
	m_VertexCount[iStreamNum]		= nVertexCount;
	m_VertexBlendType[iStreamNum]	= VertBlendType;
	m_bDynamicVB[iStreamNum]		= bDynamic;

	return true;
}

bool VertexBufferController::CreateIndexBuffer(int32 iIndexCount)
{
	assert(m_pVB_Index==NULL); 

	// Create m_pVB_Index...
	uint32 iUsage					= D3DUSAGE_WRITEONLY;									// Index buffer is always created the same way...

iUsage |= D3DUSAGE_SOFTWAREPROCESSING; // TODO: Check and remove...

	uint32 iVBSizeBytes				= iIndexCount * sizeof(uint16);
	if (FAILED(PD3DDEVICE->CreateIndexBuffer(iVBSizeBytes,iUsage,D3DFMT_INDEX16,D3DPOOL_MANAGED,&m_pVB_Index,NULL)))				{ assert(0); return false; }

	// Set our member vars...
	m_IndexCount					= iIndexCount;

	return true;
}

// Call this before a render call - sets the streams and stuff like that...
//	(Just need to call it once per group of render calls)
void VertexBufferController::SetStreamSources()
{
	// Setup the Streams...
	for (uint32 i = 0; i < VERTEXBUFFER_STREAMCOUNT; ++i) 
	{
		if (m_pVB_VertStream[i]) 
		{
			if (FAILED(PD3DDEVICE->SetStreamSource(i,m_pVB_VertStream[i], 0, m_VertexSize[i])))	{ assert(0); return; } 
		} 
	}

	if (m_pVB_Index) 
	{ 
		if (FAILED(PD3DDEVICE->SetIndices(m_pVB_Index)))									{ assert(0); return; } 
	}
}

// One stop shop - transform/light/render all in one call...
void VertexBufferController::Render(uint32 MinIndex,uint32 MinVert,uint32 VertCount,uint32 TriCount)
{
	// Render the sucker...
	if (FAILED(PD3DDEVICE->DrawIndexedPrimitive(D3DPT_TRIANGLELIST,0,MinVert,VertCount,MinIndex,TriCount)))	{ return; } //assert(0); return; }
}

// Lock the source buffer (pBuffer)...
bool VertexBufferController::Lock(VB_TYPE Type,bool bDiscardContents)
{
	uint32 uFlags = (bDiscardContents ? D3DLOCK_DISCARD | D3DLOCK_NOOVERWRITE : NULL); //(bDiscardContents ? D3DLOCK_DISCARD : NULL) | D3DLOCK_NOOVERWRITE;
	if (!m_bDynamicVB) uFlags = NULL;														// Can't specify DISCARD OR NOOVERWRITE if it's not Dynamic...

	switch (Type) {
	case eVERTSTREAM0 :
		if (m_pVert_Data[0]) return true;
		if (FAILED(m_pVB_VertStream[0]->Lock(0,0,&m_pVert_Data[0],uFlags)))		{ assert(0); return false; } break;
	case eVERTSTREAM1 :
		if (m_pVert_Data[1]) return true;
		if (FAILED(m_pVB_VertStream[1]->Lock(0,0,&m_pVert_Data[1],uFlags)))		{ assert(0); return false; } break;
	case eVERTSTREAM2 :
		if (m_pVert_Data[2]) return true;
		if (FAILED(m_pVB_VertStream[2]->Lock(0,0,&m_pVert_Data[2],uFlags)))		{ assert(0); return false; } break;
	case eVERTSTREAM3 :
		if (m_pVert_Data[3]) return true;
		if (FAILED(m_pVB_VertStream[3]->Lock(0,0,&m_pVert_Data[3],uFlags)))		{ assert(0); return false; } break;
	case eINDEX :
		if (m_pIndex_Data) return true;
		if (FAILED(m_pVB_Index->Lock(0,0,(void**)&m_pIndex_Data,uFlags)))					{ assert(0); return false; } break;
	default : assert(0); return false; }
	return true;
}

bool VertexBufferController::UnLock(VB_TYPE Type)
{
	switch (Type) {
	case eVERTSTREAM0 :
		if (!m_pVert_Data[0]) return true;
		if (FAILED(m_pVB_VertStream[0]->Unlock()))											{ assert(0); return false; } 
		m_pVert_Data[0] = NULL; break;
	case eVERTSTREAM1 :
		if (!m_pVert_Data[1]) return true;
		if (FAILED(m_pVB_VertStream[1]->Unlock()))											{ assert(0); return false; } 
		m_pVert_Data[1] = NULL; break;
	case eVERTSTREAM2 :
		if (!m_pVert_Data[2]) return true;
		if (FAILED(m_pVB_VertStream[2]->Unlock()))											{ assert(0); return false; } 
		m_pVert_Data[2] = NULL; break;
	case eVERTSTREAM3 :
		if (!m_pVert_Data[3]) return true;
		if (FAILED(m_pVB_VertStream[3]->Unlock()))											{ assert(0); return false; } 
		m_pVert_Data[3] = NULL; break;
	case eINDEX :
		if (!m_pIndex_Data) return true;
		if (FAILED(m_pVB_Index->Unlock()))													{ assert(0); return false; } 
		m_pIndex_Data = NULL; break;
	default : assert(0); return false; }
	return true;
}


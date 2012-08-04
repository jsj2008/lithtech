// VertexBufferController.cpp

#include "precompile.h"
#include "VertexBufferController.h"

#include "d3d_device.h"
#include "d3d_renderstatemgr.h"
#include "common_stuff.h"
#include "rendererconsolevars.h"

// Resets back to initial conditions (doesn't try to free anything)...
void VertexBufferController::Reset()
{
	m_pIndex_Data					= NULL; 
	m_pVB_Index						= NULL; 
	m_IndexCount					= 0;

	for (uint32 i = 0; i < VERTEXBUFFER_STREAMCOUNT; ++i) {
		m_VertexBlendType[i]		= eNO_WORLD_BLENDS; 
		m_VertexFVFFlags[i]			= 0;
		m_VertexCount[i]			= 0; 
		m_VertexSize[i]				= 0;
		m_pVB_VertStream[i]			= NULL; 
		m_pVert_Data[i]				= NULL; }
}

// Frees all the member vars and resets afterwards...
void VertexBufferController::FreeAll()
{
	// Release the VBs...
	for (uint32 i = 0; i < VERTEXBUFFER_STREAMCOUNT; ++i) {
		if (m_pVB_VertStream[i]) { 
            m_pVB_VertStream[i]->Release(); 
            m_pVB_VertStream[i] = NULL; 
        } 
    }
	if (m_pVB_Index) { 
        m_pVB_Index->Release(); 
        m_pVB_Index = NULL; 
    }

	Reset();
}

bool VertexBufferController::CreateStream(uint32 iStreamNum, int32 nVertexCount, uint32 iVertDataType, VERTEX_BLEND_TYPE VertBlendType, bool bDynamic, bool bWriteOnly, bool bForceSWVertProc, bool bUsagePoints)
{
	if (!PD3DDEVICE)
		return false;

	assert(m_pVB_VertStream[iStreamNum]==NULL); assert(iStreamNum < VERTEXBUFFER_STREAMCOUNT);

	uint32 iUsage					= (bDynamic ? D3DUSAGE_DYNAMIC : NULL) | (bWriteOnly ? D3DUSAGE_WRITEONLY : NULL) | (bUsagePoints ? D3DUSAGE_POINTS : NULL);
	uint32 iVertFlags				= 0;
	bool   bNonFixPipeData			= false;
	GetVertexFlags_and_Size(VertBlendType,iVertDataType,iVertFlags,m_VertexSize[iStreamNum],m_UVSets[iStreamNum],bNonFixPipeData);

	// Check for HW T&L cap...
	if ((!(g_Device.GetDeviceCaps()->DevCaps & D3DDEVCAPS_HWTRANSFORMANDLIGHT)) || g_CV_ForceSWVertProcess || bForceSWVertProc)
		iUsage |= D3DUSAGE_SOFTWAREPROCESSING;

	// Create m_pVB_Verts...
	uint32 iVBSizeBytes				= nVertexCount * m_VertexSize[iStreamNum];
	if (FAILED(PD3DDEVICE->CreateVertexBuffer(iVBSizeBytes,iUsage,iVertFlags,D3DPOOL_DEFAULT,&m_pVB_VertStream[iStreamNum])))				{ assert(0); return false; }

	// Set our member vars...
	m_VertexFVFFlags[iStreamNum]	= iVertFlags;
	m_VertexCount[iStreamNum]		= nVertexCount;
	m_VertexBlendType[iStreamNum]	= VertBlendType;
	m_bDynamicVB[iStreamNum]		= bDynamic;

	return true;
}

bool VertexBufferController::CreateIndexBuffer(int32 iIndexCount, bool bDynamic, bool bWriteOnly, bool bForceSWVertProc)
{
	if (!PD3DDEVICE)
		return false;

	assert(m_pVB_Index==NULL); 

	// Create m_pVB_Index...
	uint32 iUsage					= (bDynamic ? D3DUSAGE_DYNAMIC : NULL) | (bWriteOnly ? D3DUSAGE_WRITEONLY : NULL); 

	// Check for HW T&L cap...
	if ((!(g_Device.GetDeviceCaps()->DevCaps & D3DDEVCAPS_HWTRANSFORMANDLIGHT)) || g_CV_ForceSWVertProcess || bForceSWVertProc)
		iUsage |= D3DUSAGE_SOFTWAREPROCESSING;

	uint32 iVBSizeBytes				= iIndexCount * sizeof(uint16);
	if (FAILED(PD3DDEVICE->CreateIndexBuffer(iVBSizeBytes,iUsage,D3DFMT_INDEX16,D3DPOOL_DEFAULT,&m_pVB_Index)))				{ assert(0); return false; }

	// Set our member vars...
	m_IndexCount					= iIndexCount;

	return true;
}

// Call this before a render call - sets the streams and stuff like that...
//	(Just need to call it once per group of render calls)
void VertexBufferController::SetStreamSources()
{
	if (m_pVB_Index) 
	{ 
		if (FAILED(PD3DDEVICE->SetIndices(m_pVB_Index)))
		{ 
			assert(0); return; 
		} 
	}

	// Setup the Streams...
	for (uint32 i = 0; i < VERTEXBUFFER_STREAMCOUNT; ++i) 
	{
		if (m_pVB_VertStream[i]) 
		{
			if (FAILED(PD3DDEVICE->SetStreamSource(i,m_pVB_VertStream[i],0, m_VertexSize[i])))	{ assert(0); return; } 
		} 
	}
}

// One stop shop - transform/light/render all in one call...
void VertexBufferController::Render(uint32 MinVertIndexed,uint32 StartIndex,uint32 VertCount,uint32 TriCount, uint32 iVertOffset)
{
	// Render the sucker...
	if (FAILED(PD3DDEVICE->DrawIndexedPrimitive(D3DPT_TRIANGLELIST,iVertOffset,MinVertIndexed,VertCount,StartIndex,TriCount)))	{ return; } //assert(0); return; }
}

// For rendering non-indexed points...
void VertexBufferController::RenderPoints(uint32 StartVert,uint32 PointCount)
{
	if (FAILED(PD3DDEVICE->DrawPrimitive(D3DPT_POINTLIST,StartVert,PointCount)))	{ return; } //assert(0); return; }
}

// Lock the source buffer (pBuffer)...
bool VertexBufferController::Lock(VB_TYPE Type,bool bDiscardContents,bool bNoOverWrite,uint32 iOffsetToLock_Bytes,uint32 iSizeToLock_Bytes)
{
	uint32 uFlags = (bDiscardContents ? D3DLOCK_DISCARD : NULL) | (bNoOverWrite ? D3DLOCK_NOOVERWRITE : NULL); //(bDiscardContents ? D3DLOCK_DISCARD : NULL) | D3DLOCK_NOOVERWRITE;
	if (!m_bDynamicVB[Type]) uFlags = NULL;														// Can't specify DISCARD OR NOOVERWRITE if it's not Dynamic...

	switch (Type) {
	case eVERTSTREAM0 :
		if (m_pVert_Data[0]) return true;
		if (FAILED(m_pVB_VertStream[0]->Lock(iOffsetToLock_Bytes,iSizeToLock_Bytes,(void **)&m_pVert_Data[0],uFlags)))	{ assert(0); return false; } break;
	case eVERTSTREAM1 :
		if (m_pVert_Data[1]) return true;
		if (FAILED(m_pVB_VertStream[1]->Lock(iOffsetToLock_Bytes,iSizeToLock_Bytes,(void **)&m_pVert_Data[1],uFlags)))	{ assert(0); return false; } break;
	case eVERTSTREAM2 :
		if (m_pVert_Data[2]) return true;
		if (FAILED(m_pVB_VertStream[2]->Lock(iOffsetToLock_Bytes,iSizeToLock_Bytes,(void **)&m_pVert_Data[2],uFlags)))	{ assert(0); return false; } break;
	case eVERTSTREAM3 :
		if (m_pVert_Data[3]) return true;
		if (FAILED(m_pVB_VertStream[3]->Lock(iOffsetToLock_Bytes,iSizeToLock_Bytes,(void **)&m_pVert_Data[3],uFlags)))	{ assert(0); return false; } break;
	case eINDEX :
		if (m_pIndex_Data) return true;
		if (FAILED(m_pVB_Index->Lock(iOffsetToLock_Bytes,iSizeToLock_Bytes,(void **)&m_pIndex_Data,uFlags)))			{ assert(0); return false; } break;
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


// d3d_rendobj_spritearray.cpp

#include "precompile.h"
#include "d3d_rendobj_spritearray.h"

#include "d3d_utils.h"
#include "d3d_device.h"
#include "common_stuff.h"
#include "d3d_renderstatemgr.h"
#include "d3d_texture.h"
#include "de_world.h"

// DEFINES
#define MAX_QUADCOUNT_NONPOINTSPRITE_METHOD		64						// Number of index'd quads we're building up in our index buffer...

CD3DSpriteArray::CD3DSpriteArray()
{
	Reset();
}

CD3DSpriteArray::~CD3DSpriteArray()
{
	FreeAll();
}

void CD3DSpriteArray::Reset()
{
	m_VBController.Reset();
	m_bInitialized		= false;
	m_bIsLocked			= false;
	m_bUsePointSprites	= false;
	m_iSpriteCount		= 0;
	m_iMaxSpriteCount	= 0;
	m_bSWVertProcessing	= false;
	m_fSpriteRadius		= 1.0f;
	m_pVertData			= NULL;
	m_pUsedBitArray		= NULL;
	m_pTexIndexData		= NULL;
	m_pSizeData			= NULL;
	m_pPosData			= NULL;
	m_BaseTextureIndex	= 0;
	m_MaxTextureIndex	= 16;

	m_bSSScaling		= true;
	m_SSScaling_Att[0]	= 1.0f;
	m_SSScaling_Att[1]	= 0.0f;
	m_SSScaling_Att[2]	= 5.0f;
	m_SSScaling_MinSize	= 0.0f;
	m_SSScaling_MaxSize	= 1000.0f;
}

void CD3DSpriteArray::FreeAll()
{
	m_VBController.FreeAll();
	if (m_pVertData)	{ delete[] m_pVertData; m_pVertData = NULL; }
	if (m_pSizeData)	{ delete[] m_pSizeData; m_pSizeData = NULL; }
	if (m_pPosData)		{ delete[] m_pPosData; m_pPosData = NULL; }
	if (m_pUsedBitArray){ delete[] m_pUsedBitArray; m_pUsedBitArray = NULL; }
	if (m_pTexIndexData){ delete[] m_pTexIndexData; m_pTexIndexData = NULL; }

	Reset();
}

// Initialize the SpriteArray (must be done before sprites are added)...
bool CD3DSpriteArray::Init(float fSpriteRadius, DATATYPE iSpriteDataType, uint32 iApproxMaxSpriteCount, uint32 bAllowParticleApproximation)
{
	if (m_bInitialized) { FreeAll(); }

	if (g_CV_ForceSWVertProcess) m_bSWVertProcessing = (g_CV_ForceSWVertProcess ? true : false);

	// First figure out if we're going to use point sprites or full quads...
	m_bUsePointSprites = true;
	if (!bAllowParticleApproximation || (iSpriteDataType & eUVs)) m_bUsePointSprites = false;
	if ((iSpriteDataType & eSize) && !(g_Device.GetDeviceCaps()->FVFCaps & D3DFVFCAPS_PSIZE)) m_bUsePointSprites = false;

	m_fSpriteRadius		= fSpriteRadius;
	if (m_bUsePointSprites) {
		m_VertFlags		= (iSpriteDataType & ePosition ? VERTDATATYPE_POSITION : NULL) | (iSpriteDataType & eColor ? VERTDATATYPE_DIFFUSE : NULL) | (iSpriteDataType & eSize ? VERTDATATYPE_PSIZE : NULL); }
	else { 
		m_VertFlags		= (iSpriteDataType & ePosition ? VERTDATATYPE_POSITION : NULL) | (iSpriteDataType & eColor ? VERTDATATYPE_DIFFUSE : NULL) | VERTDATATYPE_UVSETS_1; }
	m_iSpriteCount		= 0;
	m_iMaxSpriteCount	= LTMAX(10,iApproxMaxSpriteCount);

	// Create our empty VBController...
	uint32 iVertCount	= m_bUsePointSprites ? m_iMaxSpriteCount : m_iMaxSpriteCount*4; uint32 FVFVertFlags;
	if (!m_VBController.CreateStream(0,iVertCount,m_VertFlags,eNO_WORLD_BLENDS,true,true,m_bSWVertProcessing,m_bUsePointSprites)) { FreeAll(); return false; }

	// Alloc our sys mem copy of the data...
	if (m_bUsePointSprites) {
		FVFVertFlags	= (iSpriteDataType & ePosition ? D3DFVF_XYZ : NULL) | (iSpriteDataType & eColor ? D3DFVF_DIFFUSE : NULL) | (iSpriteDataType & eSize ? D3DFVF_PSIZE : NULL); }
	else {
		FVFVertFlags	= (iSpriteDataType & ePosition ? D3DFVF_XYZ : NULL) | (iSpriteDataType & eColor ? D3DFVF_DIFFUSE : NULL) | D3DFVF_TEX1; }
	uint32 iVertexSize	= d3d_GetVertexSize(FVFVertFlags);
	uint32 iSize		= iVertexSize * iVertCount;						

	// Alloc the VertData...
	LT_MEM_TRACK_ALLOC(m_pVertData = new uint8[iSize],LT_MEM_TYPE_RENDERER);	
	if (!m_pVertData) { FreeAll(); return false; }

	if (!m_bUsePointSprites) {
		// Allod our indicies and load up our data...
		if (!m_VBController.CreateIndexBuffer(MAX_QUADCOUNT_NONPOINTSPRITE_METHOD*6,false,true,m_bSWVertProcessing)) { FreeAll(); return false; }

		// Setup our indicies...
		m_VBController.Lock(VertexBufferController::eINDEX,true,true);
		uint16* pIndex	= m_VBController.getIndexData();
		for (uint32 i = 0; i < MAX_QUADCOUNT_NONPOINTSPRITE_METHOD; ++i) {
			uint16 i6 = uint16(i*4); 
            *pIndex = i6; 
            ++pIndex; 
            *pIndex = uint16(i6+1);
            ++pIndex; 
            *pIndex = uint16(i6+2); 
            ++pIndex; 
            *pIndex = i6; 
            ++pIndex; 
            *pIndex = uint16(i6+2); 
            ++pIndex; 
            *pIndex = uint16(i6+3); 
            ++pIndex;  
        } 
		m_VBController.UnLock(VertexBufferController::eINDEX);

		// Setup our UVs...
		uint8* pSysVertData	= m_pVertData;
		for (uint32 i = 0; i < m_iMaxSpriteCount; ++i) {
			if (m_VertFlags & VERTDATATYPE_POSITION) pSysVertData += sizeof(float) * 3;
			if (m_VertFlags & VERTDATATYPE_DIFFUSE)	 pSysVertData += sizeof(uint32);
			*((float*)pSysVertData) = 0.0f;			 pSysVertData += sizeof(float);
			*((float*)pSysVertData) = 0.0f;			 pSysVertData += sizeof(float);
			if (m_VertFlags & VERTDATATYPE_POSITION) pSysVertData += sizeof(float) * 3;
			if (m_VertFlags & VERTDATATYPE_DIFFUSE)	 pSysVertData += sizeof(uint32);
			*((float*)pSysVertData) = 1.0f;			 pSysVertData += sizeof(float);
			*((float*)pSysVertData) = 0.0f;			 pSysVertData += sizeof(float);
			if (m_VertFlags & VERTDATATYPE_POSITION) pSysVertData += sizeof(float) * 3;
			if (m_VertFlags & VERTDATATYPE_DIFFUSE)	 pSysVertData += sizeof(uint32);
			*((float*)pSysVertData) = 1.0f;			 pSysVertData += sizeof(float);
			*((float*)pSysVertData) = 1.0f;			 pSysVertData += sizeof(float);
			if (m_VertFlags & VERTDATATYPE_POSITION) pSysVertData += sizeof(float) * 3;
			if (m_VertFlags & VERTDATATYPE_DIFFUSE)	 pSysVertData += sizeof(uint32);
			*((float*)pSysVertData) = 0.0f;			 pSysVertData += sizeof(float);
			*((float*)pSysVertData) = 1.0f;			 pSysVertData += sizeof(float); }
		m_VBController.Lock(VertexBufferController::eVERTSTREAM0,true,true);
		uint8* pVB_Data	= (uint8*)m_VBController.getVertexData(0);
		memcpy(pVB_Data,m_pVertData,iSize);								// Now update the VB...
		m_VBController.UnLock(VertexBufferController::eVERTSTREAM0);

		// May need to keep the sizes of all the sprites...
		if (iSpriteDataType & eSize) {
			LT_MEM_TRACK_ALLOC(m_pSizeData = new float[m_iMaxSpriteCount],LT_MEM_TYPE_RENDERER);
			if (!m_pSizeData) { FreeAll(); return false; } 
			for (uint32 i = 0; i < m_iMaxSpriteCount; ++i) m_pSizeData[i] = m_fSpriteRadius; } 
	
		// Keep the positions of the spites in an array...
		LT_MEM_TRACK_ALLOC(m_pPosData = new LTVector3f[m_iMaxSpriteCount],LT_MEM_TYPE_RENDERER);
		if (!m_pPosData) { FreeAll(); return false; } }

	// Alloc the used bit array (and zero it out)...
	uint32 iUsedArySize	= m_iMaxSpriteCount / 8;
	if (iUsedArySize * 8 != m_iMaxSpriteCount) ++iUsedArySize;			// Round up...
	LT_MEM_TRACK_ALLOC(m_pUsedBitArray = new uint8[iUsedArySize],LT_MEM_TYPE_RENDERER);
	if (!m_pUsedBitArray) { FreeAll(); return false; }
	memset(m_pUsedBitArray,0,iUsedArySize); 

	// Alloc our texture index array...
	if (iSpriteDataType & eTextureIndex) {
		LT_MEM_TRACK_ALLOC(m_pTexIndexData	= new uint8[m_iMaxSpriteCount],LT_MEM_TYPE_RENDERER);
		if (!m_pTexIndexData) { FreeAll(); return false; }
		memset(m_pTexIndexData,0,m_iMaxSpriteCount); }
	
	return (m_bInitialized = true);
}

float CD3DSpriteArray::GetMaxParticleSize() {
	return g_Device.GetDeviceCaps()->MaxPointSize; }					// Query caps for max point/particle size...

// Add a sprites (returns a handle to the sprite)...
CD3DSpriteArray::HSPRITE CD3DSpriteArray::AddSprite()
{
	if (!m_bInitialized) return NULL;

	// Check for space in the VB...
	if (m_iSpriteCount == m_iMaxSpriteCount) {							// If we're at the limit, need to alloc some more space...
		uint32 iOldMaxSpriteCount = m_iMaxSpriteCount;					// If no space, we need to increase the size of the VB...
		m_iMaxSpriteCount	= uint32(m_iMaxSpriteCount * 1.25f);					// Increase the size by 25%...

		// Free and Re-Create the VBController...
		m_VBController.FreeAll();
		uint32 iVertCount	= m_bUsePointSprites ? m_iMaxSpriteCount : m_iMaxSpriteCount*4;
		if (!m_VBController.CreateStream(0,iVertCount,m_VertFlags,eNO_WORLD_BLENDS,true,true,m_bSWVertProcessing,m_bUsePointSprites)) { FreeAll(); return NULL; }

		// Alloc the new sys mem copy of the data...
		uint32 iVertexSize	= m_VBController.getVertexSize(0);
		uint32 iSize		= iVertexSize * iVertCount;					// Alloc the VertData...
		uint8* pNewVertData;
		LT_MEM_TRACK_ALLOC(pNewVertData = new uint8[iSize],LT_MEM_TYPE_RENDERER);
		if (!pNewVertData) { FreeAll(); return NULL; }

		// Alloc the used bit array (and zero it out)...
		uint32 iUsedArySize	= m_iMaxSpriteCount / 8;
		if (iUsedArySize * 8 != m_iMaxSpriteCount) ++iUsedArySize;		// Round up...
		uint8* pNewUsedBitArray;
		LT_MEM_TRACK_ALLOC(pNewUsedBitArray = new uint8[iUsedArySize],LT_MEM_TYPE_RENDERER);
		if (!pNewUsedBitArray) { FreeAll(); return NULL; }

		// Alloc our new TextureIndex array and copy it over...
		uint8* pNewTextureIndexArray = NULL;
		if (m_pTexIndexData) { 
			LT_MEM_TRACK_ALLOC(pNewTextureIndexArray = new uint8[m_iMaxSpriteCount],LT_MEM_TYPE_RENDERER);
			if (!pNewTextureIndexArray) { FreeAll(); return NULL; } 
			memcpy(pNewTextureIndexArray,m_pTexIndexData,iOldMaxSpriteCount);
			delete[] m_pTexIndexData; m_pTexIndexData = pNewTextureIndexArray; 
			memset(m_pTexIndexData+iOldMaxSpriteCount,0,m_iMaxSpriteCount-iOldMaxSpriteCount); }

		// Alloc our new m_pSizeData and copy it over...
		float* pNewSizeData = NULL;
		if (m_pSizeData) {
			LT_MEM_TRACK_ALLOC(pNewSizeData = new float[m_iMaxSpriteCount],LT_MEM_TYPE_RENDERER);
			if (!pNewSizeData) { FreeAll(); return NULL; } 
			memcpy(pNewSizeData,m_pSizeData,iOldMaxSpriteCount * sizeof(float));
			delete[] m_pSizeData; m_pSizeData = pNewSizeData; 
			for (uint32 i = iOldMaxSpriteCount; i < m_iMaxSpriteCount; ++i) m_pSizeData[i] = m_fSpriteRadius; }

		// Alloc our new m_pPosData and copy it over...
		LTVector3f* pNewPosData = NULL;
		if (m_pPosData) {
			LT_MEM_TRACK_ALLOC(pNewPosData = new LTVector3f[m_iMaxSpriteCount],LT_MEM_TYPE_RENDERER);
			if (!pNewPosData) { FreeAll(); return NULL; } 
			memcpy(pNewPosData,m_pPosData,iOldMaxSpriteCount * sizeof(LTVector3f));
			delete[] m_pPosData; m_pPosData = pNewPosData; }

		// Copy over the sys mem vert data...
		uint32 OldVertCount	= m_bUsePointSprites ? iOldMaxSpriteCount : iOldMaxSpriteCount*4;
		uint32 iOldVertSize	= iVertexSize * OldVertCount;
		memcpy(pNewVertData,m_pVertData,iOldVertSize);
		delete[] m_pVertData; m_pVertData = pNewVertData;

		// Setup our UVs...
		if (!m_bUsePointSprites) {
			// Allod our indicies and load up our data...
			if (!m_VBController.CreateIndexBuffer(MAX_QUADCOUNT_NONPOINTSPRITE_METHOD*6,false,true,m_bSWVertProcessing)) { FreeAll(); return false; }

			// Setup our indicies...
			m_VBController.Lock(VertexBufferController::eINDEX,true,true);
			uint16* pIndex	= m_VBController.getIndexData();
			for (uint32 i = 0; i < MAX_QUADCOUNT_NONPOINTSPRITE_METHOD; ++i) {
				uint16 i6 = uint16(i*4); *pIndex = i6; ++pIndex; *pIndex = uint16(i6+1); ++pIndex; *pIndex = uint16(i6+2); ++pIndex; *pIndex = i6; ++pIndex; *pIndex = uint16(i6+2); ++pIndex; *pIndex = uint16(i6+3); ++pIndex;  } 
			m_VBController.UnLock(VertexBufferController::eINDEX);

			uint8* pSysVertData	= (uint8*)(m_pVertData + iOldMaxSpriteCount * iVertexSize * 4);
			for (int i = iOldMaxSpriteCount; i < m_iMaxSpriteCount; ++i) {
				if (m_VertFlags & VERTDATATYPE_POSITION) pSysVertData += sizeof(float) * 3;
				if (m_VertFlags & VERTDATATYPE_DIFFUSE)	 pSysVertData += sizeof(uint32);
				*((float*)pSysVertData) = 0.0f;			 pSysVertData += sizeof(float);
				*((float*)pSysVertData) = 0.0f;			 pSysVertData += sizeof(float);
				if (m_VertFlags & VERTDATATYPE_POSITION) pSysVertData += sizeof(float) * 3;
				if (m_VertFlags & VERTDATATYPE_DIFFUSE)	 pSysVertData += sizeof(uint32);
				*((float*)pSysVertData) = 1.0f;			 pSysVertData += sizeof(float);
				*((float*)pSysVertData) = 0.0f;			 pSysVertData += sizeof(float);
				if (m_VertFlags & VERTDATATYPE_POSITION) pSysVertData += sizeof(float) * 3;
				if (m_VertFlags & VERTDATATYPE_DIFFUSE)	 pSysVertData += sizeof(uint32);
				*((float*)pSysVertData) = 1.0f;			 pSysVertData += sizeof(float);
				*((float*)pSysVertData) = 1.0f;			 pSysVertData += sizeof(float);
				if (m_VertFlags & VERTDATATYPE_POSITION) pSysVertData += sizeof(float) * 3;
				if (m_VertFlags & VERTDATATYPE_DIFFUSE)	 pSysVertData += sizeof(uint32);
				*((float*)pSysVertData) = 0.0f;			 pSysVertData += sizeof(float);
				*((float*)pSysVertData) = 1.0f;			 pSysVertData += sizeof(float); } }

		// Copy over the used data...
		uint32 iOldUsedArySize = iOldMaxSpriteCount / 8;
		if (iOldUsedArySize * 8 != iOldMaxSpriteCount) ++iOldUsedArySize;
		memcpy(pNewUsedBitArray,m_pUsedBitArray,iOldUsedArySize);
		delete[] m_pUsedBitArray; m_pUsedBitArray = pNewUsedBitArray;
		memset(m_pUsedBitArray+iOldUsedArySize,0,iUsedArySize-iOldUsedArySize); 

		// Copy over the verts (to the VB)...
		if (m_bUsePointSprites) {
			m_VBController.Lock(VertexBufferController::eVERTSTREAM0,true,true);
			uint8* pVertData	= (uint8*)m_VBController.getVertexData(0);
			uint32 iCopySize	= m_VBController.getVertexSize(0) * m_iMaxSpriteCount;
			memcpy(pVertData,m_pVertData,iCopySize);
			m_VBController.UnLock(VertexBufferController::eVERTSTREAM0); }
		else {
			m_VBController.Lock(VertexBufferController::eVERTSTREAM0,true,true);
			uint8* pVertData	= (uint8*)m_VBController.getVertexData(0);
			uint32 iCopySize	= m_VBController.getVertexSize(0) * m_iMaxSpriteCount * 4;
			memcpy(pVertData,m_pVertData,iCopySize);
			m_VBController.UnLock(VertexBufferController::eVERTSTREAM0); } }

	// Find a space for it (starting at the front of the used list)...
	for (uint32 i = 0; i < m_iMaxSpriteCount; ++i) {
		uint32 iByteIndex	= (i / 8);
		uint32 iBitMask		= (0x1 << (i % 8)); 
		if ((m_pUsedBitArray[iByteIndex] & iBitMask) == 0) {			// Found a free one: Mark the space as taken and return it's number (the handle to it)...
			m_pUsedBitArray[iByteIndex] |= iBitMask;
			if (m_pTexIndexData) m_pTexIndexData[i] = 0;				// Set the texture index to 0...
			++m_iSpriteCount; return (i+1); } }
	assert(0); return NULL;												// This should never happen (means the sprite count and used array are out of sync)...
}

// Mark a sprite as un-used...
void CD3DSpriteArray::RemoveSprite(HSPRITE hSprite)
{
	if (!m_bInitialized) return;
	if (!hSprite) return; hSprite -= 1;									// Remember: Sprite indices are offset by one...

	// Mark it as not used in the bit array and return;
	uint32 iByteIndex	= (hSprite / 8);
	uint32 iBitMask		= (0x1 << (hSprite % 8)); 
	uint32 iNotBitMask	= ~iBitMask;
	assert((m_pUsedBitArray[iByteIndex] & iBitMask) && "CD3DSpriteArray::RemoveSprite - Trying to remove a sprite that does not exist.");
	m_pUsedBitArray[iByteIndex] &= iNotBitMask;
	--m_iSpriteCount;
}

// Lock the VBController (so they can do sets and such)...
bool CD3DSpriteArray::Lock() 
{
	assert(!m_bIsLocked);
	m_bIsLocked			= true;
	return m_VBController.Lock(VertexBufferController::eVERTSTREAM0,false,true); 
}

bool CD3DSpriteArray::IsLocked()
{
	return m_bIsLocked;
}

bool CD3DSpriteArray::UnLock() 
{
	assert(m_bIsLocked);
	m_bIsLocked			= false;
	return m_VBController.UnLock(VertexBufferController::eVERTSTREAM0); 
}

bool CD3DSpriteArray::SetPosition(HSPRITE hSprite, LTVector3f& Position)
{
	assert(hSprite <= m_iMaxSpriteCount); assert(m_VertFlags & VERTDATATYPE_POSITION);
	if (!hSprite) return false; hSprite -= 1;							// Remember: Sprite indices are offset by one...

	if (m_bUsePointSprites) {
		uint32 iVertexSize		= m_VBController.getVertexSize(0);
		uint8* pVertData		= (uint8*)m_VBController.getVertexData(0);	if (!pVertData) return false;
		pVertData			   += (iVertexSize * hSprite);
		*((float*)pVertData)	= Position.x; pVertData		+= sizeof(float);
		*((float*)pVertData)	= Position.y; pVertData		+= sizeof(float);
		*((float*)pVertData)	= Position.z;

		uint8* pSysVertData	= m_pVertData;									if (!pSysVertData) return false;
		pSysVertData	   += (iVertexSize * hSprite);
		*((float*)pSysVertData)	= Position.x; pSysVertData	+= sizeof(float);
		*((float*)pSysVertData)	= Position.y; pSysVertData	+= sizeof(float);
		*((float*)pSysVertData)	= Position.z; }
	else {
		m_pPosData[hSprite]		= Position; }

	return true;
}

bool CD3DSpriteArray::SetColor(HSPRITE hSprite, uint32 Color)
{
	assert(hSprite <= m_iMaxSpriteCount); assert(m_VertFlags & VERTDATATYPE_DIFFUSE);
	if (!hSprite) return false; hSprite -= 1;							// Remember: Sprite indices are offset by one...

	if (m_bUsePointSprites) {
		uint32 iVertexSize		= m_VBController.getVertexSize(0);
		uint8* pVertData		= (uint8*)m_VBController.getVertexData(0);	if (!pVertData) return false;
		pVertData			   += (iVertexSize * hSprite);
		if (m_VertFlags & VERTDATATYPE_POSITION) pVertData += sizeof(float) * 3;
		if (m_VertFlags & VERTDATATYPE_PSIZE)	 pVertData += sizeof(float);
		*((uint32*)pVertData)	= Color;

		uint8* pSysVertData		= m_pVertData;								if (!pSysVertData) return false;
		pSysVertData		   += (iVertexSize * hSprite);
		if (m_VertFlags & VERTDATATYPE_POSITION) pSysVertData += sizeof(float) * 3;
		if (m_VertFlags & VERTDATATYPE_PSIZE)	 pSysVertData += sizeof(float);
		*((uint32*)pSysVertData)= Color; }
	else {
		uint32 iVertexSize		= m_VBController.getVertexSize(0)*4;
		uint8* pVertData		= (uint8*)m_VBController.getVertexData(0);	if (!pVertData) return false;
		pVertData			   += (iVertexSize * hSprite);
		for (uint32 i = 0; i < 4; ++i) {
			if (m_VertFlags & VERTDATATYPE_POSITION) pVertData += sizeof(float) * 3;
			*((uint32*)pVertData) = Color; pVertData += sizeof(uint32);
			if (m_VertFlags & VERTDATATYPE_UVSETS_1) pVertData += sizeof(float) * 2; }

		uint8* pSysVertData		= m_pVertData;								if (!pSysVertData) return false;
		pSysVertData		   += (iVertexSize * hSprite);
		for (int i = 0; i < 4; ++i) {
			if (m_VertFlags & VERTDATATYPE_POSITION) pSysVertData += sizeof(float) * 3;
			*((uint32*)pSysVertData)= Color; pSysVertData += sizeof(uint32);
			if (m_VertFlags & VERTDATATYPE_UVSETS_1) pSysVertData += sizeof(float) * 2; } }

	return true;
}

bool CD3DSpriteArray::SetSize(HSPRITE hSprite, float Size)
{
	assert(hSprite <= m_iMaxSpriteCount); assert((m_VertFlags & VERTDATATYPE_PSIZE) || m_pSizeData);
	if (!hSprite) return false; hSprite -= 1;							// Remember: Sprite indices are offset by one...

	if (m_bUsePointSprites) {
		uint32 iVertexSize		= m_VBController.getVertexSize(0);
		uint8* pVertData		= (uint8*)m_VBController.getVertexData(0);	if (!pVertData) return false;
		pVertData		       += (iVertexSize * hSprite);
		if (m_VertFlags & VERTDATATYPE_POSITION) pVertData += sizeof(float) * 3;
		*((float*)pVertData)	= Size;

		uint8* pSysVertData		= m_pVertData;								if (!pSysVertData) return false;
		pSysVertData		   += (iVertexSize * hSprite);
		if (m_VertFlags & VERTDATATYPE_POSITION) pSysVertData += sizeof(float) * 3;
		*((float*)pSysVertData)	= Size; }
	else {
		m_pSizeData[hSprite] = Size; }

	return true;
}

bool CD3DSpriteArray::SetTextureIndex(HSPRITE hSprite, uint8 Index) {
	assert(hSprite <= m_iMaxSpriteCount); assert(m_pTexIndexData);
	if (!hSprite) return false; hSprite -= 1;							// Remember: Sprite indices are offset by one...

	m_pTexIndexData[hSprite] = Index;
	return true;
}

bool CD3DSpriteArray::SetUVs(HSPRITE hSprite, float* pUVs) {
	assert(hSprite <= m_iMaxSpriteCount); assert(m_VertFlags & VERTDATATYPE_UVSETS_1);
	if (!hSprite) return false; hSprite -= 1; assert(!m_bUsePointSprites);

	uint32 iVertexSize		= m_VBController.getVertexSize(0)*4;
	uint8* pVertData		= (uint8*)m_VBController.getVertexData(0);	if (!pVertData) return false;
	pVertData			   += (iVertexSize * hSprite);
	for (uint32 i = 0; i < 4; ++i) {
		if (m_VertFlags & VERTDATATYPE_POSITION) pVertData += sizeof(float) * 3;
		if (m_VertFlags & VERTDATATYPE_DIFFUSE)  pVertData += sizeof(uint32); 
		*((float*)pVertData)= pUVs[i*2];		 pVertData += sizeof(float);
		*((float*)pVertData)= pUVs[i*2+1];		 pVertData += sizeof(float); }

	uint8* pSysVertData		= m_pVertData;								if (!pSysVertData) return false;
	pSysVertData		   += (iVertexSize * hSprite);
	for (int i = 0; i < 4; ++i) {
		if (m_VertFlags & VERTDATATYPE_POSITION) pSysVertData += sizeof(float) * 3;
		if (m_VertFlags & VERTDATATYPE_DIFFUSE)  pSysVertData += sizeof(uint32);
		*((float*)pSysVertData) = pUVs[i*2];	 pSysVertData += sizeof(float);
		*((float*)pSysVertData) = pUVs[i*2+1];	 pSysVertData += sizeof(float); }

	return true;
}

bool CD3DSpriteArray::GetPosition(HSPRITE hSprite, LTVector3f* pPosition)
{
	assert(hSprite <= m_iMaxSpriteCount); assert(m_VertFlags & VERTDATATYPE_POSITION);
	if (!hSprite) return false; hSprite -= 1;							// Remember: Sprite indices are offset by one...

	if (m_bUsePointSprites) {
		uint32 iVertexSize		= m_VBController.getVertexSize(0);
		uint8* pSysVertData		= m_pVertData;								if (!pSysVertData) return false;
		pSysVertData		   += (iVertexSize * hSprite);
		pPosition->x			= *((float*)pSysVertData);					pSysVertData += sizeof(float);
		pPosition->y			= *((float*)pSysVertData);					pSysVertData += sizeof(float);
		pPosition->z			= *((float*)pSysVertData); }
	else {
		*pPosition				= m_pPosData[hSprite]; }

	return true;
}

bool CD3DSpriteArray::GetColor(HSPRITE hSprite, uint32* pColor)
{
	assert(hSprite <= m_iMaxSpriteCount); assert(m_VertFlags & VERTDATATYPE_DIFFUSE);
	if (!hSprite) return false; hSprite -= 1;							// Remember: Sprite indices are offset by one...

	if (m_bUsePointSprites) {
		uint32 iVertexSize	= m_VBController.getVertexSize(0);
		uint8* pSysVertData	= m_pVertData;									if (!pSysVertData) return false;
		pSysVertData	   += (iVertexSize * hSprite);
		if (m_VertFlags & VERTDATATYPE_POSITION) pSysVertData += sizeof(float) * 3;
		if (m_VertFlags & VERTDATATYPE_PSIZE) pSysVertData += sizeof(float);
		*pColor				= *((uint32*)pSysVertData);	}	
	else {
		uint32 iVertexSize	= m_VBController.getVertexSize(0);
		uint8* pSysVertData	= m_pVertData;									if (!pSysVertData) return false;
		pSysVertData	   += (iVertexSize * hSprite);
		if (m_VertFlags & VERTDATATYPE_POSITION) pSysVertData += sizeof(float) * 3;
		*pColor				= *((uint32*)pSysVertData);	 }

	return true;
}

bool CD3DSpriteArray::GetSize(HSPRITE hSprite, float* pSize)
{
	assert(hSprite <= m_iMaxSpriteCount); assert((m_VertFlags & VERTDATATYPE_PSIZE) || m_pSizeData);
	if (!hSprite) return false; hSprite -= 1;							// Remember: Sprite indices are offset by one...

	if (m_bUsePointSprites) {
		uint32 iVertexSize	= m_VBController.getVertexSize(0);
		uint8* pSysVertData	= m_pVertData;									if (!pSysVertData) return false;
		pSysVertData	   += (iVertexSize * hSprite);
		if (m_VertFlags & VERTDATATYPE_POSITION)   pSysVertData += sizeof(float) * 3;
		*pSize				= *((float*)pSysVertData); }
	else {
		*pSize				= m_pSizeData[hSprite]; }

	return true;
}

bool CD3DSpriteArray::GetTextureIndex(HSPRITE hSprite, uint8* pIndex) {
	assert(hSprite <= m_iMaxSpriteCount && pIndex); assert(m_pTexIndexData);
	if (!hSprite) return false; hSprite -= 1;							// Remember: Sprite indices are offset by one...

	*pIndex					= m_pTexIndexData[hSprite];
	return true;
}

bool CD3DSpriteArray::GetUVs(HSPRITE hSprite, float* pUVs) {
	assert(hSprite <= m_iMaxSpriteCount); assert(m_VertFlags & VERTDATATYPE_UVSETS_1);
	if (!hSprite) return false; hSprite -= 1; assert(!m_bUsePointSprites);

	uint32 iVertexSize	= m_VBController.getVertexSize(0);
	uint8* pSysVertData	= m_pVertData;									if (!pSysVertData) return false;
	pSysVertData	   += (iVertexSize * hSprite);
	for (uint32 i = 0; i < 4; ++i) {
		if (m_VertFlags & VERTDATATYPE_POSITION)   pSysVertData += sizeof(float) * 3;
		if (m_VertFlags & VERTDATATYPE_DIFFUSE)	   pSysVertData += sizeof(uint32);
		pUVs[i*2]		= *((float*)pSysVertData); pSysVertData += sizeof(float);
		pUVs[i*2+1]		= *((float*)pSysVertData); pSysVertData += sizeof(float); }

	return true;
}

bool CD3DSpriteArray::SetScreenSpaceScaling(float AttConst_A, float AttConst_B, float AttConst_C,float fMinSpriteSize,float fMaxSpriteSize)
{
	m_SSScaling_Att[0]	= AttConst_A;
	m_SSScaling_Att[1]	= AttConst_B;
	m_SSScaling_Att[2]	= AttConst_C;
	m_SSScaling_MinSize	= fMinSpriteSize;
	m_SSScaling_MaxSize	= fMaxSpriteSize;
	return true; 
}

// Create the VBs and stuff from our sys mem copies...
void CD3DSpriteArray::ReCreateObject()
{
	if (m_bInitialized) return;

	// Re-Create our VBController...
	uint32 iVertCount	= m_bUsePointSprites ? m_iMaxSpriteCount : m_iMaxSpriteCount*4;
	if (!m_VBController.CreateStream(0,iVertCount,m_VertFlags,eNO_WORLD_BLENDS,true,true,m_bSWVertProcessing,m_bUsePointSprites)) { assert(0); return; }

	// Copy over the Verts...
	m_VBController.Lock(VertexBufferController::eVERTSTREAM0,true,true);
	uint8* pVertData	= (uint8*)m_VBController.getVertexData(0);
	uint32 iSize		= m_VBController.getVertexSize(0) * m_bUsePointSprites ? m_iSpriteCount : m_iSpriteCount*4;
	memcpy(pVertData,m_pVertData,iSize);
	m_VBController.UnLock(VertexBufferController::eVERTSTREAM0);

	if (!m_bUsePointSprites) {
		// Re-alloc our indicies...
		if (!m_VBController.CreateIndexBuffer(MAX_QUADCOUNT_NONPOINTSPRITE_METHOD*6,false,true,m_bSWVertProcessing)) { FreeAll(); return; }

		// Setup our indicies...
		m_VBController.Lock(VertexBufferController::eINDEX,true,true);
		uint16* pIndex	= m_VBController.getIndexData();
		for (uint32 i = 0; i < MAX_QUADCOUNT_NONPOINTSPRITE_METHOD; ++i) {
			uint16 i6 = uint16(i*4); *pIndex = i6; ++pIndex; *pIndex = uint16(i6+1); ++pIndex; *pIndex = uint16(i6+2); ++pIndex; *pIndex = i6; ++pIndex; *pIndex = uint16(i6+2); ++pIndex; *pIndex = uint16(i6+3); ++pIndex;  } 
		m_VBController.UnLock(VertexBufferController::eINDEX); }

	m_bInitialized		= true;
}

// We're loosing focus, free the stuff...
void CD3DSpriteArray::FreeDeviceObjects()
{
	if (!m_bInitialized) return;

	m_VBController.FreeAll();											// Free our VB...

	m_bInitialized		= false;
}

// Poly Sorting (do some number of sort iterations)...
void CD3DSpriteArray::SortPolies(uint32 Iterations, LTVector3f& SpriteSpace_ViewPos) 
{
	assert(m_VertFlags & VERTDATATYPE_POSITION);

	// Walk the array sorting the polys the requested number of times...
	bool bAlreadyLocked		= m_VBController.getVertexSize(0) ? true : false;
	if (!bAlreadyLocked) m_VBController.Lock(VertexBufferController::eVERTSTREAM0,true,true);
	uint32 VB_SpriteSize	= m_bUsePointSprites ? m_VBController.getVertexSize(0) : m_VBController.getVertexSize(0) * 4;
	uint8* pVB_Data			= (uint8*)m_VBController.getVertexData(0);
	uint8* pSM_Data			= m_pVertData;
	uint8 Buff[128]; assert(VB_SpriteSize < 128); // Buffer for a vert (should always be bigger than a vertex)...
	for (uint32 i = 0; i < Iterations; ++i) {
		// Find the first active sprite...
		uint32 iSpriteIndex	= 0;
		uint32 iByte		= 0;
		uint32 iBitMask		= 0x1; 
		while ((!(m_pUsedBitArray[iByte] & iBitMask)) && (iSpriteIndex < m_iMaxSpriteCount)) {
			++iSpriteIndex;
			iByte			= (iSpriteIndex / 8);
			iBitMask		= (0x1 << (iSpriteIndex % 8)); }
		uint32 iCurSprite	= iSpriteIndex; ++iSpriteIndex;

		// Go through all the sprites...
		while (iSpriteIndex < m_iMaxSpriteCount) {
			iByte			= (iSpriteIndex / 8);						// Find the next active sprite...
			iBitMask		= (0x1 << (iSpriteIndex % 8));
			while ((!(m_pUsedBitArray[iByte] & iBitMask)) && (iSpriteIndex < m_iMaxSpriteCount)) {
				++iSpriteIndex;
				iByte		= (iSpriteIndex / 8);
				iBitMask	= (0x1 << (iSpriteIndex % 8)); }
			if (iSpriteIndex == m_iMaxSpriteCount) continue; 

			// Ok, found a pair, compare and possibly swap...
			LTVector3f* pV1	= (LTVector3f*)&pSM_Data[iCurSprite * VB_SpriteSize];
			float fDstSqr1	= m_bUsePointSprites ? pV1->DistSqr(SpriteSpace_ViewPos) : m_pPosData[iCurSprite].DistSqr(SpriteSpace_ViewPos);
			LTVector3f* pV2	= (LTVector3f*)&pSM_Data[iSpriteIndex * VB_SpriteSize];
			float fDstSqr2	= m_bUsePointSprites ? pV2->DistSqr(SpriteSpace_ViewPos) : m_pPosData[iSpriteIndex].DistSqr(SpriteSpace_ViewPos);
			if (fDstSqr1 < fDstSqr2) {
				memcpy(Buff,pV1,VB_SpriteSize);
				memcpy(pV1,pV2,VB_SpriteSize);
				memcpy(pV2,Buff,VB_SpriteSize); 
				if (m_pTexIndexData) { 
					Buff[0] = m_pTexIndexData[iCurSprite];
					m_pTexIndexData[iCurSprite] = m_pTexIndexData[iSpriteIndex];
					m_pTexIndexData[iSpriteIndex] = Buff[0]; }
				if (!m_bUsePointSprites) {
					LTVector3f Pos = m_pPosData[iCurSprite];
					m_pPosData[iCurSprite] = m_pPosData[iSpriteIndex];
					m_pPosData[iSpriteIndex] = Pos;
					if (m_pSizeData) {
						float Size = m_pSizeData[iCurSprite];
						m_pSizeData[iCurSprite] = m_pSizeData[iSpriteIndex];
						m_pSizeData[iSpriteIndex] = Size; } } }

			iCurSprite		= iSpriteIndex; ++iSpriteIndex; } }			// Update our current sprite index...
	int32 iCopySize	= VB_SpriteSize * m_iSpriteCount;					// Now update the VB...
	memcpy(pVB_Data,m_pVertData,iCopySize);
	if (!bAlreadyLocked) m_VBController.UnLock(VertexBufferController::eVERTSTREAM0);
}

// Render using point sprites...
void CD3DSpriteArray::Render_PointSprites(vector<LPDIRECT3DBASETEXTURE9>& TextureList)
{
	if (m_pTexIndexData) {																	// Render with texture indicies...
		uint32 iSpriteIndex = 0;															// Walk m_pUsedBitArray and render as you go...
		uint8* pTexIndex	= m_pTexIndexData;
		uint8 iTexIndex		= 0;
		while (iSpriteIndex < m_iMaxSpriteCount) {
			uint32 iByte	= (iSpriteIndex / 8);
			uint32 iBitMask	= (0x1 << (iSpriteIndex % 8)); 
			uint32 iRendCnt	= 0;
			if (m_pUsedBitArray[iByte] & iBitMask) {
				uint8 iNewTexIndex = uint8((*pTexIndex + m_BaseTextureIndex) % m_MaxTextureIndex);
				if (iTexIndex != iNewTexIndex) {
					iTexIndex = iNewTexIndex;
					g_RenderStateMgr.SetTexture(0,TextureList[iTexIndex]); }
				while ((m_pUsedBitArray[iByte] & iBitMask) && (iTexIndex == ((*pTexIndex + m_BaseTextureIndex) % m_MaxTextureIndex)) && (iSpriteIndex < m_iMaxSpriteCount)) {	// We're looking for runs to render...
					++iRendCnt; ++iSpriteIndex; ++pTexIndex;
					iByte		= (iSpriteIndex / 8);
					iBitMask	= (0x1 << (iSpriteIndex % 8)); }
				if (iRendCnt) m_VBController.RenderPoints(iSpriteIndex-iRendCnt,iRendCnt); }
			++iSpriteIndex; ++pTexIndex; } }
	else {
		uint32 iSpriteIndex = 0;															// Walk m_pUsedBitArray and render as you go...
		while (iSpriteIndex < m_iMaxSpriteCount) {
			uint32 iByte	= (iSpriteIndex / 8);
			uint32 iBitMask	= (0x1 << (iSpriteIndex % 8)); 
			uint32 iRendCnt	= 0;
			while ((m_pUsedBitArray[iByte] & iBitMask) && (iSpriteIndex < m_iMaxSpriteCount)) {	// We're looking for runs to render...
				++iRendCnt; ++iSpriteIndex;
				iByte		= (iSpriteIndex / 8);
				iBitMask	= (0x1 << (iSpriteIndex % 8)); }
			if (iRendCnt) m_VBController.RenderPoints(iSpriteIndex-iRendCnt,iRendCnt);
			++iSpriteIndex; } }
}

// Fill in the positions into the VB...
inline void CD3DSpriteArray::Render_Quads_CalcPos(uint32 StartSprite, uint32 Count) {
	uint32 VertSize			= m_VBController.getVertexSize(0);
	uint32 SpriteSize		= VertSize * 4;
	m_VBController.Lock(VertexBufferController::eVERTSTREAM0,false,true,SpriteSize*StartSprite,SpriteSize*Count);
	uint8* pVertData		= (uint8*)m_VBController.getVertexData(0);
	for (uint32 i = 0; i < Count; ++i) {													// Need to figure out the screen-space positions...
		uint32 SprPlusI		= StartSprite + i;
		D3DXVECTOR3* pPos	= (D3DXVECTOR3*)(&(m_pPosData[SprPlusI])); D3DXVECTOR3 PosT;
		D3DXVec3TransformCoord(&PosT,pPos,&m_WorldViewTransform);
		LTVector3f* pVB_Pos	= (LTVector3f*)(pVertData + i*SpriteSize);
		float f				= m_pSizeData ? m_pSizeData[SprPlusI] : m_fSpriteRadius;
		pVB_Pos->x			= PosT.x - f;
		pVB_Pos->y			= PosT.y + f;
		pVB_Pos->z			= PosT.z; 
		pVB_Pos = (LTVector3f*)((uint32)pVB_Pos + VertSize);
		pVB_Pos->x			= PosT.x + f;
		pVB_Pos->y			= PosT.y + f;
		pVB_Pos->z			= PosT.z; 
		pVB_Pos = (LTVector3f*)((uint32)pVB_Pos + VertSize);
		pVB_Pos->x			= PosT.x + f;
		pVB_Pos->y			= PosT.y - f;
		pVB_Pos->z			= PosT.z; 
		pVB_Pos = (LTVector3f*)((uint32)pVB_Pos + VertSize);
		pVB_Pos->x			= PosT.x - f;
		pVB_Pos->y			= PosT.y - f;
		pVB_Pos->z			= PosT.z; 
		pVB_Pos = (LTVector3f*)((uint32)pVB_Pos + VertSize); }
	m_VBController.UnLock(VertexBufferController::eVERTSTREAM0);
}

// Render using quads...
void CD3DSpriteArray::Render_Quads(vector<LPDIRECT3DBASETEXTURE9>& TextureList) 
{
	if (m_pTexIndexData) {																	// Render with texture indicies...
		uint32 iSpriteIndex = 0;															// Walk m_pUsedBitArray and render as you go...
		uint8* pTexIndex	= m_pTexIndexData;
		uint8 iTexIndex		= 0;
		while (iSpriteIndex < m_iMaxSpriteCount) {
			uint32 iByte	= (iSpriteIndex / 8);
			uint32 iBitMask	= (0x1 << (iSpriteIndex % 8)); 
			uint32 iRendCnt	= 0;
			if (m_pUsedBitArray[iByte] & iBitMask) {
				uint8 iNewTexIndex = uint8((*pTexIndex + m_BaseTextureIndex) % m_MaxTextureIndex);
				if (iTexIndex != iNewTexIndex) {
					iTexIndex = iNewTexIndex;
					g_RenderStateMgr.SetTexture(0,TextureList[iTexIndex]); } 
				while ((m_pUsedBitArray[iByte] & iBitMask) && (iRendCnt < MAX_QUADCOUNT_NONPOINTSPRITE_METHOD) && (iTexIndex == ((*pTexIndex + m_BaseTextureIndex) % m_MaxTextureIndex)) && (iSpriteIndex < m_iMaxSpriteCount)) {	// We're looking for runs to render...
					++iRendCnt; ++iSpriteIndex; ++pTexIndex;
					iByte		= (iSpriteIndex / 8);
					iBitMask	= (0x1 << (iSpriteIndex % 8)); }
				if (iRendCnt) { 
					uint32 iStartSprite = iSpriteIndex - iRendCnt;
					Render_Quads_CalcPos(iStartSprite,iRendCnt);
					m_VBController.SetStreamSources();
					m_VBController.Render(0,0,iRendCnt*4,iRendCnt*2, iStartSprite*4); 
			} }
			++iSpriteIndex; ++pTexIndex; } }
	else {
		uint32 iSpriteIndex = 0;															// Walk m_pUsedBitArray and render as you go...
		while (iSpriteIndex < m_iMaxSpriteCount) {
			uint32 iByte	= (iSpriteIndex / 8);
			uint32 iBitMask	= (0x1 << (iSpriteIndex % 8)); 
			uint32 iRendCnt	= 0;
			while ((m_pUsedBitArray[iByte] & iBitMask) && (iRendCnt < MAX_QUADCOUNT_NONPOINTSPRITE_METHOD) && (iSpriteIndex < m_iMaxSpriteCount)) {	// We're looking for runs to render...
				++iRendCnt; ++iSpriteIndex;
				iByte		= (iSpriteIndex / 8);
				iBitMask	= (0x1 << (iSpriteIndex % 8)); }
			if (iRendCnt) { 
				uint32 iStartSprite = iSpriteIndex - iRendCnt;
				Render_Quads_CalcPos(iStartSprite,iRendCnt);
				m_VBController.SetStreamSources();
				m_VBController.Render(0,0,iRendCnt*4,iRendCnt*2,iStartSprite*4); }
			++iSpriteIndex; } }
}

// Render the SpriteArray...
void CD3DSpriteArray::Render(LTMatrix& WorldTransform, CRenderStyle* pRenderStyle, vector<LPDIRECT3DBASETEXTURE9>& TextureList)
{
	CD3DRenderStyle* pD3DRenderStyle = (CD3DRenderStyle*)pRenderStyle;

	D3DXMATRIX PrevViewTransform;
	if (!m_bUsePointSprites) 
	{
		PrevViewTransform = *((D3DXMATRIX*)g_RenderStateMgr.GetTransform(D3DTS_VIEW));
		D3DXMatrixMultiply(&m_WorldViewTransform,(D3DXMATRIX*)&WorldTransform,&PrevViewTransform);
		D3DXMATRIX Identity; D3DXMatrixIdentity(&Identity);
		D3DXMATRIX InvView; 
		D3DXMatrixInverse(&InvView,NULL,&PrevViewTransform);
		g_RenderStateMgr.SetTransform(D3DTS_WORLDMATRIX(0), &InvView); 
	}
	else 
	{
		g_RenderStateMgr.SetTransform(D3DTS_WORLDMATRIX(0), (D3DMATRIX*)&WorldTransform); 
	}

	g_RenderStateMgr.SetSoftwareVertexProcessing(m_bSWVertProcessing);

	if (m_bUsePointSprites) {
		// Set your point sprite render states (as long as no one else uses these we're ok)...
		g_RenderStateMgr.SetRenderState(D3DRS_POINTSPRITEENABLE, true);
		g_RenderStateMgr.SetRenderState(D3DRS_POINTSCALEENABLE,  m_bSSScaling); float f = m_fSpriteRadius * 2.0f;
		g_RenderStateMgr.SetRenderState(D3DRS_POINTSIZE,		 *((DWORD*)&f));
		g_RenderStateMgr.SetRenderState(D3DRS_POINTSIZE_MIN,	 *((DWORD*)&m_SSScaling_MinSize));
		g_RenderStateMgr.SetRenderState(D3DRS_POINTSIZE_MAX,	 *((DWORD*)&m_SSScaling_MaxSize));
		g_RenderStateMgr.SetRenderState(D3DRS_POINTSCALE_A,		 *((DWORD*)&m_SSScaling_Att[0]));
		g_RenderStateMgr.SetRenderState(D3DRS_POINTSCALE_B,		 *((DWORD*)&m_SSScaling_Att[1]));
		g_RenderStateMgr.SetRenderState(D3DRS_POINTSCALE_C,		 *((DWORD*)&m_SSScaling_Att[2])); }

	for (uint32 iRenderPass = 0; iRenderPass < pD3DRenderStyle->GetRenderPassCount(); ++iRenderPass) 
	{
		HD3DVERTEXSHADER VertexShader = pD3DRenderStyle->GetVertexShader(iRenderPass,0); RSD3DRenderPass D3DRenderPass;
		if (VertexShader) 
		{																		// Using a custom vertex shader...
			// If we were created for HW Vert Processing & the shader requires that we need to re-created ourselves for SW Vert Processing...
			if (!m_bSWVertProcessing && pD3DRenderStyle->CreatedForSWVertProcessing()) {
				m_bSWVertProcessing = true;
				FreeDeviceObjects(); ReCreateObject(); 
				g_RenderStateMgr.SetSoftwareVertexProcessing(true); }
			else if (m_bSWVertProcessing && !pD3DRenderStyle->CreatedForSWVertProcessing()) {
				pD3DRenderStyle->ForceSWVertProcessing(); }
			if (FAILED(g_RenderStateMgr.SetVertexShader(VertexShader)))							{ return; }
			if (!pD3DRenderStyle->GetRenderPass_D3DOptions(iRenderPass,&D3DRenderPass))			{ return; }
			if (!g_RenderStateMgr.SetVertexShaderConstants(pD3DRenderStyle, &D3DRenderPass,0))	{ return; } 
		}
		else 
		{																					// Using the standand pipe...
			if (!m_VBController.getVertexFormat(0)) return;										// This is a non fixed function pipe VB - bail out...
			if (FAILED(g_RenderStateMgr.SetVertexShader(m_VBController.getVertexFormat(0))))	{ return; } 
		}

		m_VBController.SetStreamSources();
		g_RenderStateMgr.SetRenderStyleStates(pD3DRenderStyle,iRenderPass,TextureList);	// Set your render states with the render state mgr...

		if (m_bUsePointSprites) 
			Render_PointSprites(TextureList);
		else 
			Render_Quads(TextureList); 
	}
}

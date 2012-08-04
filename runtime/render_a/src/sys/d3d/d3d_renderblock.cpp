//////////////////////////////////////////////////////////////////////////////
// Render block implementation

#include "precompile.h"

#include "common_stuff.h"
#include "common_draw.h"
#include "d3d_device.h"
#include "d3d_texture.h"
#include "renderstruct.h"
#include "d3d_draw.h"
#include "d3d_renderworld.h"
#include "d3d_renderblock.h"
#include "d3d_convar.h"
#include "dtxmgr.h"

#include "d3d_rendershader_default.h"

#include "d3d_rendershader_dynamiclight.h"

#include "lightmap_compress.h"

#include "memstats_world.h"
#include "rendererframestats.h"

//for rendering models shadows
#include "modelshadowshader.h"

//for rendering texture scripts
#include "texturescriptmgr.h"
#include "texturescriptinstance.h"

//for loading and managing the sprite data
#include "client_filemgr.h"
#include "setupobject.h"

// Shader types we're going to get from the processor
enum EPCShaderType {
	ePCShader_None = 0, // No shading
	ePCShader_Gouraud = 1, // Textured and vertex-lit
	ePCShader_Lightmap = 2, // Base lightmap
	ePCShader_Lightmap_Texture = 4, // Texturing pass of lightmapping
	ePCShader_Skypan = 5, // Skypan
	ePCShader_SkyPortal = 6,
	ePCShader_Occluder = 7,
	ePCShader_DualTexture = 8,	// Gouraud shaded dual texture
	ePCShader_Lightmap_DualTexture = 9, //Texture stage of lightmap shaded dual texture
	ePCShader_Unknown = 10 // Unknown - draw something to make it obvious there's a problem
};

ILTStream &operator>>(ILTStream &stream, SRBVertex &sVert)
{
	stream >> sVert.m_vPos;
	stream >> sVert.m_fU0;
	stream >> sVert.m_fV0;
	stream >> sVert.m_fU1;
	stream >> sVert.m_fV1;
	stream >> sVert.m_nColor;
	stream >> sVert.m_vNormal;
	stream >> sVert.m_vTangent;
	stream >> sVert.m_vBinormal;
	return stream;
}

// Implement the internal section stuff
CRBSection::CRBSection() : 
	m_nStartIndex(0), 
	m_nTriCount(0), 
	m_nStartVertex(0), 
	m_nVertexCount(0),
	m_nLightmapWidth(0),
	m_nLightmapHeight(0),
	m_nLightmapSize(0),
	m_pLightmapData(NULL),
	m_pTextureEffect(NULL),
	m_eShader(eShader_Invalid),
	m_eBoundShader(eShader_Invalid)
{
	for(uint32 nCurrTex = 0; nCurrTex < kNumTextures; nCurrTex++)
	{
		m_pTexture[nCurrTex]	= NULL;
		m_pSpriteData[nCurrTex] = NULL;
	}
}

CRBSection::~CRBSection()
{
 	// Dereference our textures...
 	for (uint32 nCurrTex = 0; nCurrTex < kNumTextures; nCurrTex++)
   	{
 		if (m_pTexture[nCurrTex])
 		{
 			m_pTexture[nCurrTex]->SetRefCount(m_pTexture[nCurrTex]->GetRefCount() - 1);
 			m_pTexture[nCurrTex] = 0;
 		}
   	}

	// Release our reference to the texture effect
	if (m_pTextureEffect)
	{
		CTextureScriptMgr::GetSingleton().ReleaseInstance(m_pTextureEffect);
	}

	// Release our sprite information if applicable
	for (uint32 nCurrTex = 0; nCurrTex < kNumTextures; nCurrTex++)
		delete m_pSpriteData[nCurrTex];

	// Release the lightmap data
	if (m_pLightmapData)
	{
		delete[] m_pLightmapData;
		m_pLightmapData = 0;
	}
}

CRBSection::CRBSection(const CRBSection &cOther) :
	m_nStartIndex(cOther.m_nStartIndex),
	m_nTriCount(cOther.m_nTriCount),
	m_nStartVertex(cOther.m_nStartVertex),
	m_nVertexCount(cOther.m_nVertexCount),
	m_nLightmapWidth(cOther.m_nLightmapWidth),
	m_nLightmapHeight(cOther.m_nLightmapHeight),
	m_nLightmapSize(cOther.m_nLightmapSize),
	m_pLightmapData(0),
	m_pTextureEffect(NULL),
	m_eShader(cOther.m_eShader),
	m_eBoundShader(cOther.m_eBoundShader)
{
	for (uint32 nCurrTex = 0; nCurrTex < kNumTextures; nCurrTex++)
	{
		m_pTexture[nCurrTex] = cOther.m_pTexture[nCurrTex];

		if (m_pTexture[nCurrTex])
			m_pTexture[nCurrTex]->SetRefCount(m_pTexture[nCurrTex]->GetRefCount() + 1);

		// Copy over the sprite object if necessary
		if (cOther.m_pSpriteData[nCurrTex])
		{
			// Make us our own sprite object that is a copy of the other's
			LT_MEM_TRACK_ALLOC(m_pSpriteData[nCurrTex] = new CSpriteData(*cOther.m_pSpriteData[nCurrTex]),LT_MEM_TYPE_SPRITE);
		}
		else
			m_pSpriteData[nCurrTex] = NULL;
	}

	// Copy over the texture effect from the other object and add our reference
	m_pTextureEffect = cOther.m_pTextureEffect;
	if (m_pTextureEffect)
		m_pTextureEffect->AddRef();

	if (m_nLightmapSize)
	{
		LT_MEM_TRACK_ALLOC(m_pLightmapData = new uint8[m_nLightmapSize],LT_MEM_TYPE_RENDER_LIGHTMAP);
		memcpy(m_pLightmapData, cOther.m_pLightmapData, m_nLightmapSize);
	}
}

CRBSection &CRBSection::operator=(const CRBSection &cOther)
{
	if(&cOther == this)
		return *this;

	for (uint32 nCurrTex = 0; nCurrTex < kNumTextures; nCurrTex++)
	{
		if (m_pTexture[nCurrTex])
			m_pTexture[nCurrTex]->SetRefCount(m_pTexture[nCurrTex]->GetRefCount() - 1);
		m_pTexture[nCurrTex] = cOther.m_pTexture[nCurrTex];
		if (m_pTexture[nCurrTex])
			m_pTexture[nCurrTex]->SetRefCount(m_pTexture[nCurrTex]->GetRefCount() + 1);

		// Free our current sprite object
		delete m_pSpriteData[nCurrTex];
		m_pSpriteData[nCurrTex] = NULL;

		// Copy over the other object's
		if (cOther.m_pSpriteData[nCurrTex])
		{
			// Make us our own sprite object that is a copy of the other's
			LT_MEM_TRACK_ALLOC(m_pSpriteData[nCurrTex] = new CSpriteData(*cOther.m_pSpriteData[nCurrTex]),LT_MEM_TYPE_SPRITE);
		}
	}

	m_nStartIndex = cOther.m_nStartIndex;
	m_nTriCount = cOther.m_nTriCount;
	m_nStartVertex = cOther.m_nStartVertex;
	m_nVertexCount = cOther.m_nVertexCount;
	delete[] m_pLightmapData;
	m_nLightmapWidth = cOther.m_nLightmapWidth;
	m_nLightmapHeight = cOther.m_nLightmapHeight;
	m_nLightmapSize = cOther.m_nLightmapSize;
	m_eShader = cOther.m_eShader;
	m_eBoundShader = cOther.m_eBoundShader;

	// Copy over the texture instance, freeing ours if we have one
	if (m_pTextureEffect)
		CTextureScriptMgr::GetSingleton().ReleaseInstance(m_pTextureEffect);
	m_pTextureEffect = cOther.m_pTextureEffect;
	if (m_pTextureEffect)
		m_pTextureEffect->AddRef();

	if (m_nLightmapSize)
	{
		LT_MEM_TRACK_ALLOC(m_pLightmapData = new uint8[m_nLightmapSize],LT_MEM_TYPE_RENDER_LIGHTMAP);
		memcpy(m_pLightmapData, cOther.m_pLightmapData, m_nLightmapSize);
	}
	return *this;
}

//////////////////////////////////////////////////////////////////////////////
// SRBLightGroup implementation

SRBLightGroup::~SRBLightGroup()
{
	ClearSectionLMList();
}

SRBLightGroup::SRBLightGroup(const SRBLightGroup &sOther) :
	m_nID(sOther.m_nID),
	m_vColor(sOther.m_vColor),
	m_aVertexIntensities(sOther.m_aVertexIntensities)
{
	m_aSectionLMs.reserve(sOther.m_aSectionLMs.size());
	TSectionLMList::const_iterator iCurLM = sOther.m_aSectionLMs.begin();
	for (; iCurLM != sOther.m_aSectionLMs.end(); ++iCurLM)
	{
		if (*iCurLM)
		{
			LT_MEM_TRACK_ALLOC(m_aSectionLMs.push_back(new TSubLMList(**iCurLM)),LT_MEM_TYPE_RENDER_LIGHTGROUP);
		}
		else
			m_aSectionLMs.push_back(0);
	}
}

SRBLightGroup &SRBLightGroup::operator=(const SRBLightGroup &sOther)
{
	if (&sOther == this)
		return *this;

	m_nID = sOther.m_nID;
	m_vColor = sOther.m_vColor;
	m_aVertexIntensities = sOther.m_aVertexIntensities;

	ClearSectionLMList();

	m_aSectionLMs.reserve(sOther.m_aSectionLMs.size());
	TSectionLMList::const_iterator iCurLM = sOther.m_aSectionLMs.begin();
	for (; iCurLM != sOther.m_aSectionLMs.end(); ++iCurLM)
	{
		if (*iCurLM)
		{
			LT_MEM_TRACK_ALLOC(m_aSectionLMs.push_back(new TSubLMList(**iCurLM)),LT_MEM_TYPE_RENDER_LIGHTGROUP);
		}
		else
			m_aSectionLMs.push_back(0);
	}
	return *this;
}

void SRBLightGroup::ClearSectionLMList()
{
	while (!m_aSectionLMs.empty())
	{
		delete m_aSectionLMs.back();
		m_aSectionLMs.pop_back();
	}
}

bool SRBLightGroup::SSubLM::Load(ILTStream &cStream)
{
	cStream >> m_nLeft;
	cStream >> m_nTop;
	cStream >> m_nWidth;
	cStream >> m_nHeight;

	uint32 nDataSize;
	cStream >> nDataSize;
	LT_MEM_TRACK_ALLOC(m_aData.resize(nDataSize), LT_MEM_TYPE_RENDER_LIGHTMAP);
	cStream.Read(static_cast<void*>(&(*m_aData.begin())), nDataSize);

	return true;
}

//////////////////////////////////////////////////////////////////////////////
// CD3D_RenderBlock implementation

CD3D_RenderBlock::CD3D_RenderBlock() :
	m_pWorld(0),
	m_vCenter(0.0f, 0.0f, 0.0f),
	m_vHalfDims(0.0f, 0.0f, 0.0f),
	m_aIndices(0),
	m_aVertices(0),
	m_nTriCount(0),
	m_nVertexCount(0),
	m_bShadersBound(false),
	m_bShadersDirty(true)
{
	for (uint32 nChildLoop = 0; nChildLoop < k_NumChildren; ++nChildLoop)
		m_aChildren[nChildLoop] = reinterpret_cast<CD3D_RenderBlock*>(k_InvalidChild);
	for (uint32 nShaderIndexLoop = 0; nShaderIndexLoop < k_eShader_Num + 2; ++nShaderIndexLoop)
		m_aShaderIndices[nShaderIndexLoop] = k_InvalidShaderIndex;
}

CD3D_RenderBlock::~CD3D_RenderBlock()
{
	if (m_bShadersBound)
		Release();

	delete[] m_aIndices;
	delete[] m_aVertices;
}

ILTStream &operator>>(ILTStream &cStream, SRBGeometryPoly &cPoly)
{
	uint8 nVertCount;
	cStream >> nVertCount;
	ASSERT("Invalid geometry poly found" && nVertCount > 2);

	cPoly.m_vMin.Init(FLT_MAX, FLT_MAX, FLT_MAX);
	cPoly.m_vMax.Init(-FLT_MAX, -FLT_MAX, -FLT_MAX);

	cPoly.m_aVerts.reserve(nVertCount);
	for (; nVertCount; --nVertCount)
	{
		LTVector vPos;
		cStream >> vPos;
		cPoly.m_aVerts.push_back(vPos);

		VEC_MIN(cPoly.m_vMin, cPoly.m_vMin, vPos);
		VEC_MAX(cPoly.m_vMax, cPoly.m_vMax, vPos);
	}
	cStream >> cPoly.m_cPlane.m_Normal;
	cStream >> cPoly.m_cPlane.m_Dist;

	cPoly.m_ePlaneCorner = GetAABBPlaneCorner(cPoly.m_cPlane.m_Normal);

	return cStream;
}

ILTStream &operator>>(ILTStream &cStream, SRBOccluder &cPoly)
{
	cStream >> static_cast<SRBGeometryPoly&>(cPoly);
	cStream >> cPoly.m_nID;

	return cStream;
}

ILTStream &operator>>(ILTStream &cStream, SRBLightGroup &cLightGroup)
{
	uint16 nLength;
	cStream >> (uint16)nLength;
	cLightGroup.m_nID = 0;
	for (; nLength; --nLength)
	{
		uint8 nNextChar;
		cStream >> (uint8)nNextChar;
		cLightGroup.m_nID *= 31;
		cLightGroup.m_nID += (uint32)nNextChar;
	}

	cStream >> cLightGroup.m_vColor;

	uint32 nDataLength;
	cStream >> (uint32)nDataLength;
	LT_MEM_TRACK_ALLOC(cLightGroup.m_aVertexIntensities.resize(nDataLength, 0), LT_MEM_TYPE_RENDER_LIGHTGROUP);
	cStream.Read(static_cast<void*>(&(*cLightGroup.m_aVertexIntensities.begin())), nDataLength);

	// Validate the vertex intensity stream here instead of at binding time
	// NYI

	// Read in the section sub-lightmap list
	cLightGroup.ClearSectionLMList();

	uint32 nSectionLMSize;
	cStream >> nSectionLMSize;
	LT_MEM_TRACK_ALLOC(cLightGroup.m_aSectionLMs.resize(nSectionLMSize), LT_MEM_TYPE_RENDER_LIGHTGROUP);
	SRBLightGroup::TSectionLMList::iterator iCurSection = cLightGroup.m_aSectionLMs.begin();
	for (; iCurSection != cLightGroup.m_aSectionLMs.end(); ++iCurSection)
	{
		uint32 nSubLMSize;
		cStream >> nSubLMSize;
		if (!nSubLMSize)
			continue;
		LT_MEM_TRACK_ALLOC(*iCurSection = new SRBLightGroup::TSubLMList,LT_MEM_TYPE_RENDER_LIGHTGROUP);
		LT_MEM_TRACK_ALLOC((*iCurSection)->resize(nSubLMSize), LT_MEM_TYPE_RENDER_LIGHTMAP);
		SRBLightGroup::TSubLMList::iterator iCurSubLM = (*iCurSection)->begin();
		for (; iCurSubLM != (*iCurSection)->end(); ++iCurSubLM)
		{
			iCurSubLM->Load(cStream);
		}
	}

	return cStream;
}

bool CD3D_RenderBlock::IsSpriteTexture(const char *pFilename)
{
	uint32 nTexNameLen = strlen(pFilename);

	if (nTexNameLen < 4)
		return false;

	return _stricmp(&pFilename[nTexNameLen - 4], ".spr") == 0;
}

SharedTexture *CD3D_RenderBlock::LoadSpriteData(CRBSection &cSection, uint32 nTextureIndex, const char *pFilename)
{
	SharedTexture *pResult = LTNULL;

	// We do indeed have a sprite, so make us a sprite object
	CRBSection::CSpriteData* pNewSprite;
	LT_MEM_TRACK_ALLOC(pNewSprite = new CRBSection::CSpriteData,LT_MEM_TYPE_SPRITE);

	if (!pNewSprite)
		return LTNULL;

	// Fill our our file info
	FileRef FileInfo;
	FileInfo.m_FileType  = FILE_CLIENTFILE;
	FileInfo.m_pFilename = pFilename;

	// Now fill it in appropriately
	LTRESULT Result = LoadSprite(&FileInfo, &pNewSprite->m_pSprite);
	if (Result != LT_OK)
	{
		delete pNewSprite;
		return LTNULL;
	}

	// Now initialize our tracker
	spr_InitTracker(&pNewSprite->m_SpriteTracker, pNewSprite->m_pSprite);
	if (pNewSprite->m_SpriteTracker.m_pCurFrame)
		pResult = pNewSprite->m_SpriteTracker.m_pCurFrame->m_pTex;

	// Assign this sprite to the section
	cSection.m_pSpriteData[nTextureIndex] = pNewSprite;

	return pResult;
}

bool CD3D_RenderBlock::Load(ILTStream *pStream)
{
	if (m_bShadersBound)
		Release();

	// Read the bounding area of this block
	*pStream >> m_vCenter;
	*pStream >> m_vHalfDims;
	m_vBoundsMin = m_vCenter - m_vHalfDims;
	m_vBoundsMax = m_vCenter + m_vHalfDims;

	// Read in the section list
	uint32 nSectionCount;
	uint32 nIndexOffset = 0;
	*pStream >> nSectionCount;
	if (nSectionCount)
	{
		// Note : This would be better off as a resize, but that fails for some reason
		LT_MEM_TRACK_ALLOC(m_aSections.reserve(nSectionCount), LT_MEM_TYPE_RENDER_WORLD);
		for (uint32 nSectionLoop = 0; nSectionLoop < nSectionCount; ++nSectionLoop)
		{
			// Read the data
			char sTextureName[CRBSection::kNumTextures][MAX_PATH+1];

			uint32 nCurrTex;
			for(nCurrTex = 0; nCurrTex < CRBSection::kNumTextures; nCurrTex++)
				pStream->ReadString(sTextureName[nCurrTex], sizeof(sTextureName[nCurrTex]));

			uint8 nShaderCode;
			*pStream >> nShaderCode;
			uint32 nTriCount;
			*pStream >> nTriCount;
			ASSERT(nTriCount != 0);

			char sTextureEffect[MAX_PATH+1];
			pStream->ReadString(sTextureEffect, sizeof(sTextureEffect));

			// Set up the section
			m_aSections.push_back(CRBSection());
			CRBSection &cCurSection = m_aSections[nSectionLoop];
			cCurSection.m_nStartIndex = nIndexOffset;
			cCurSection.m_nTriCount = nTriCount;
			nIndexOffset += nTriCount * 3;

			cCurSection.m_nIndex = nSectionLoop;

			// Determine if the texture is a texture or actually a sprite
			for (nCurrTex = 0; nCurrTex < CRBSection::kNumTextures; nCurrTex++)
			{
				// The texture for this section
				SharedTexture* pSectionTexture = NULL;

				if (IsSpriteTexture(sTextureName[nCurrTex]))
				{
					pSectionTexture = LoadSpriteData(cCurSection, nCurrTex, sTextureName[nCurrTex]);
				}
				else if (sTextureName[nCurrTex][0])
				{
					pSectionTexture = g_pStruct->GetSharedTexture(sTextureName[nCurrTex]);
				}
				cCurSection.m_pTexture[nCurrTex] = pSectionTexture;

				// Load the texture
				// Don't try to load lightmap textures
				// NYI
				if (cCurSection.m_pTexture[nCurrTex])
					cCurSection.m_pTexture[nCurrTex]->SetRefCount(cCurSection.m_pTexture[nCurrTex]->GetRefCount() + 1);
			}

			// Translate the shader code
			cCurSection.m_eShader = TranslatePCShaderCode(nShaderCode);

			if ((cCurSection.m_eShader != eShader_Gouraud) && 
				(cCurSection.m_eShader != eShader_Lightmap) && 
				(cCurSection.m_pTexture[0] == NULL))
			{
				dsi_ConsolePrint("Renderer: Error loading texture %s", sTextureName[0]);
			}

			// Handle the dual texture cases
			if (((cCurSection.m_eShader == eShader_Gouraud_DualTexture) || (cCurSection.m_eShader == eShader_Lightmap_DualTexture)) &&
				(cCurSection.m_pTexture[1] == NULL))
			{
				dsi_ConsolePrint("Renderer: Error loading texture %s", sTextureName[1]);
			}

			// Load the lightmap
			*pStream >> (uint32)cCurSection.m_nLightmapWidth;
			*pStream >> (uint32)cCurSection.m_nLightmapHeight;
			*pStream >> (uint32)cCurSection.m_nLightmapSize;
			if (cCurSection.m_nLightmapSize)
			{
				LT_MEM_TRACK_ALLOC(cCurSection.m_pLightmapData = new uint8[cCurSection.m_nLightmapSize],LT_MEM_TYPE_RENDER_LIGHTMAP);
				pStream->Read(cCurSection.m_pLightmapData, cCurSection.m_nLightmapSize);
			}

			// Load the texture effect
			if (sTextureEffect[0] != '\0')
			{
				cCurSection.m_pTextureEffect = CTextureScriptMgr::GetSingleton().GetInstance(sTextureEffect);
			}
		}
	}

	// Get the vertex data
	*pStream >> (uint32)m_nVertexCount;
	if (m_nVertexCount)
	{
		LT_MEM_TRACK_ALLOC(m_aVertices = new SRBVertex[m_nVertexCount],LT_MEM_TYPE_RENDER_WORLD);

		//read in all the vertices in a single read, this makes the structure layout dependant
		//upon the file format, but saves millions of reads.
		pStream->Read(m_aVertices, sizeof(SRBVertex) * m_nVertexCount);

		/*
		//This is the alternative, where it loads it in vertex by vertex.
		for (uint32 nVertLoop = 0; nVertLoop < m_nVertexCount; ++nVertLoop)
		{
			*pStream >> m_aVertices[nVertLoop];
		}
		*/
	}

	// Get the triangle count
	*pStream >> (uint32)m_nTriCount;
	if (m_nTriCount)
	{
		LT_MEM_TRACK_ALLOC(m_aIndices = new uint16[m_nTriCount * 3],LT_MEM_TYPE_RENDER_WORLD);
		
		// Tracking for the active section vertex span information..
		TSectionList::iterator iCurSection = m_aSections.begin();
		uint32 nSectionLeft = iCurSection->m_nTriCount;
		uint32 nMinVertex = m_nVertexCount;
		uint32 nMaxVertex = 0;
		uint32 nSectionIndex = 0;

		// Read in the indices
		uint32 nIndexOffset = 0;
		for (uint32 nTriLoop = 0; nTriLoop < m_nTriCount; ++nTriLoop)
		{
			uint32 nIndex0, nIndex1, nIndex2;
			*pStream >> nIndex0 >> nIndex1 >> nIndex2;
			ASSERT("Invalid index found" &&
				(nIndex0 < m_nVertexCount) &&
				(nIndex1 < m_nVertexCount) &&
				(nIndex2 < m_nVertexCount));
			m_aIndices[nIndexOffset] = (uint16)nIndex0;
			++nIndexOffset;
			m_aIndices[nIndexOffset] = (uint16)nIndex1;
			++nIndexOffset;
			m_aIndices[nIndexOffset] = (uint16)nIndex2;
			++nIndexOffset;

			// skip the poly index (for now)
			uint32 nPolyIndex;
			*pStream >> nPolyIndex;

			// Track the min/max vertex accessing
			nMinVertex = LTMIN(nMinVertex, nIndex0);
			nMinVertex = LTMIN(nMinVertex, nIndex1);
			nMinVertex = LTMIN(nMinVertex, nIndex2);
			nMaxVertex = LTMAX(nMaxVertex, nIndex0);
			nMaxVertex = LTMAX(nMaxVertex, nIndex1);
			nMaxVertex = LTMAX(nMaxVertex, nIndex2);

			--nSectionLeft;
			// Have we completed a section?
			if (!nSectionLeft)
			{
				// Update the current one
				iCurSection->m_nStartVertex = nMinVertex;
				iCurSection->m_nVertexCount = (nMaxVertex - nMinVertex) + 1;
				// Start the next one
				++iCurSection;
				nMinVertex = m_nVertexCount;
				nMaxVertex = 0;
				if (iCurSection != m_aSections.end())
					nSectionLeft = iCurSection->m_nTriCount;
			}
		}
	}

	// Read the sky portals
	uint32 nSkyPortalCount;
	*pStream >> nSkyPortalCount;
	LT_MEM_TRACK_ALLOC(m_aSkyPortals.reserve(nSkyPortalCount), LT_MEM_TYPE_RENDER_WORLD);
	for (; nSkyPortalCount; --nSkyPortalCount)
	{
		SRBGeometryPoly cPoly;
		*pStream >> cPoly;
		m_aSkyPortals.push_back(cPoly);
	}

	// Read the occluders
	uint32 nOccluderCount;
	*pStream >> nOccluderCount;
	LT_MEM_TRACK_ALLOC(m_aOccluders.reserve(nOccluderCount), LT_MEM_TYPE_RENDER_WORLD);
	for (; nOccluderCount; --nOccluderCount)
	{
		SRBOccluder cPoly;
		*pStream >> cPoly;
		m_aOccluders.push_back(cPoly);
	}

	// Read the light groups
	uint32 nLightGroupCount;
	*pStream >> nLightGroupCount;
	LT_MEM_TRACK_ALLOC(m_aLightGroups.reserve(nLightGroupCount), LT_MEM_TYPE_RENDER_LIGHTGROUP);
	SRBLightGroup LightGroup;
	for (; nLightGroupCount; --nLightGroupCount)
	{
		m_aLightGroups.push_back(LightGroup);
		*pStream >> m_aLightGroups.back();
	}

	// Read the child flags
	uint8 nChildFlags = 0;
	*pStream >> nChildFlags;

	// Read the children
	uint8 nMask = 1;
	for (uint32 nChildReadLoop = 0; nChildReadLoop < k_NumChildren; ++nChildReadLoop)
	{
		uint32 nIndex;
		*pStream >> nIndex;
		if (nChildFlags & nMask)
			m_aChildren[nChildReadLoop] = reinterpret_cast<CD3D_RenderBlock*>(nIndex);
		else
			m_aChildren[nChildReadLoop] = reinterpret_cast<CD3D_RenderBlock*>(k_InvalidChild);
		nMask <<= 1;
	}

	return true;
}

void CD3D_RenderBlock::FixupChildren(CD3D_RenderBlock *pBase)
{
	for (uint32 nChildLoop = 0; nChildLoop < k_NumChildren; ++nChildLoop)
	{
		uint nIndex = reinterpret_cast<uint>(m_aChildren[nChildLoop]);
		if (nIndex == (uint)k_InvalidChild)
			m_aChildren[nChildLoop] = 0;
		else
			m_aChildren[nChildLoop] = &pBase[nIndex];
	}
}

void CD3D_RenderBlock::DrawWireframeBounds()
{
	LTVector aBlockData[8];
	uint16 aBlockIndices[24];
	for (uint32 nBlockInit = 0; nBlockInit < 4; ++nBlockInit)
	{
		aBlockData[nBlockInit].y = m_vCenter.y - m_vHalfDims.y;
		aBlockData[nBlockInit + 4].y = m_vCenter.y + m_vHalfDims.y;

		aBlockData[(nBlockInit << 1)].x = m_vCenter.x - m_vHalfDims.x;
		aBlockData[(nBlockInit << 1) + 1].x = m_vCenter.x + m_vHalfDims.x;
		
		aBlockData[(nBlockInit & 1) + ((nBlockInit & 2) << 1)].z = m_vCenter.z - m_vHalfDims.z;
		aBlockData[(nBlockInit & 1) + ((nBlockInit & 2) << 1) + 2].z = m_vCenter.z + m_vHalfDims.z;
	}
	aBlockIndices[0] = 0;
	aBlockIndices[1] = 1;
	aBlockIndices[2] = 1;
	aBlockIndices[3] = 3;
	aBlockIndices[4] = 3;
	aBlockIndices[5] = 2;
	aBlockIndices[6] = 2;
	aBlockIndices[7] = 0;
	aBlockIndices[8] = 4;
	aBlockIndices[9] = 5;
	aBlockIndices[10] = 5;
	aBlockIndices[11] = 7;
	aBlockIndices[12] = 7;
	aBlockIndices[13] = 6;
	aBlockIndices[14] = 6;
	aBlockIndices[15] = 4;
	aBlockIndices[16] = 0;
	aBlockIndices[17] = 4;
	aBlockIndices[18] = 1;
	aBlockIndices[19] = 5;
	aBlockIndices[20] = 2;
	aBlockIndices[21] = 6;
	aBlockIndices[22] = 3;
	aBlockIndices[23] = 7;

	// Draw in white
	StateSet tfactor(D3DRS_TEXTUREFACTOR, 0xFFFFFFFF);

	// Make sure the texture stage states are correct
	StageStateSet state0(0, D3DTSS_COLORARG1, D3DTA_TFACTOR);
	StageStateSet state1(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
	StageStateSet state2(0, D3DTSS_ALPHAARG1, D3DTA_TFACTOR);
	StageStateSet state3(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
	StageStateSet state4(1, D3DTSS_COLOROP, D3DTOP_DISABLE);
	StageStateSet state5(1, D3DTSS_ALPHAOP, D3DTOP_DISABLE);

	PD3DDEVICE->SetVertexShader(NULL);
	PD3DDEVICE->SetFVF(D3DFVF_XYZ);
	PD3DDEVICE->DrawIndexedPrimitiveUP(D3DPT_LINELIST, 0, 8, 12, 
		(void*)aBlockIndices, D3DFMT_INDEX16, 
		(void*)aBlockData, sizeof(LTVector));
}

void CD3D_RenderBlock::GetLightList(const ViewParams& Params, const DynamicLight * const **pLightList, uint32 *pNumLights)
{
	// I really don't like accessing this global array like this, but it's the best way to 
	// get at this data under the current setup
	if (!g_nNumWorldDynamicLights)
	{
		*pLightList = 0;
		*pNumLights = 0;
		return;
	}

	// Note : This static light vector is evil for 2 reasons.
	//		#1 - It never gets freed
	//		#2 - Its allocated pointer is given to the drawing function
	//  But it's much cheaper than allocating it for every render block
	typedef vector<DynamicLight*> TLightList;
	static TLightList aLightList;
	aLightList.clear();

	DynamicLight **pCurLight = g_WorldDynamicLights;
	DynamicLight **pLastLight = &g_WorldDynamicLights[g_nNumWorldDynamicLights];
	for (; pCurLight != pLastLight; ++pCurLight)
	{
		if (((*pCurLight)->m_Flags & FLAG_ONLYLIGHTOBJECTS) != 0)
			continue;

		LTVector vLightCenter = Params.m_mInvWorld * (*pCurLight)->GetPos();
		float fLightRadius = (*pCurLight)->m_LightRadius;

		if (((vLightCenter.x + fLightRadius) < GetBoundsMin().x) || 
			((vLightCenter.y + fLightRadius) < GetBoundsMin().y) ||
			((vLightCenter.z + fLightRadius) < GetBoundsMin().z) ||
			((vLightCenter.x - fLightRadius) > GetBoundsMax().x) || 
			((vLightCenter.y - fLightRadius) > GetBoundsMax().y) ||
			((vLightCenter.z - fLightRadius) > GetBoundsMax().z))
			continue;

		aLightList.push_back(*pCurLight);
	}

	if (aLightList.empty())
	{
		*pLightList = 0;
		*pNumLights = 0;
		return;
	}

	// Give the pointers to our static light list back to the rest of the drawing stuff
	*pLightList = &aLightList.front();
	*pNumLights = aLightList.size();
}

//renders an aggreagate shader on all of its child shader blocks
bool CD3D_RenderBlock::DrawAggregateShader(IAggregateShader *pShader)
{
	// Make sure we actually have something to render...
	if (!m_aVertices || !m_aIndices)
	{
		return true;
	}

	// Make sure we're good with our shaders
	if (!m_bShadersBound)
		m_pWorld->BindAll();

	// Update the shader data if they're not up to date
	if (m_bShadersDirty)
		UpdateShaderData();

	ASSERT(m_bShadersBound);

	// Draw the shaders
	for (uint32 nShaderLoop = 0; nShaderLoop < k_eShader_Num; ++nShaderLoop)
	{
		if (m_aShaderIndices[nShaderLoop] != k_InvalidShaderIndex)
			m_pWorld->GetShader((ERenderShader)nShaderLoop)->DrawAggregateShader(pShader, m_aShaderIndices[nShaderLoop]);
	}

	return true;
}

void CD3D_RenderBlock::PreDraw(const ViewParams& Params)
{
	// Make sure we're good with our shaders
	if (!m_bShadersBound)
		m_pWorld->BindAll();

	// Update the shader data if they're not up to date
	if (m_bShadersDirty)
		UpdateShaderData();
}

bool CD3D_RenderBlock::Draw(const ViewParams& Params)
{
	// Make sure we actually have something to render...
	if (!m_aVertices || !m_aIndices)
	{
		return true;
	}

	ASSERT(m_bShadersBound);

	IncFrameStat(eFS_HardwareBlocks, 1);

	if (g_CV_DrawRenderBlocks.m_Val)
	{
		DrawWireframeBounds();
	}

	CRenderShader::DrawState cDrawState;
	cDrawState.m_pParams = &Params;

	GetLightList(Params, &(cDrawState.m_pLightList), &(cDrawState.m_nNumLights));

	// Draw the shaders
	for (uint32 nShaderLoop = 0; nShaderLoop < k_eShader_Num; ++nShaderLoop)
	{
		if (m_aShaderIndices[nShaderLoop] == k_InvalidShaderIndex)
			continue;
		CRenderShader *pShader = m_pWorld->GetShader((ERenderShader)nShaderLoop);
		pShader->Draw(cDrawState, m_aShaderIndices[nShaderLoop]);
	}

	IncFrameStat(eFS_RBSections, m_aSections.size());
	
	return true;
}

bool CD3D_RenderBlock::GetLitVertexData(SRBVertex *&pVertices) const
{
	// Default to using the standard data
	pVertices = m_aVertices;

	if (m_aLightGroups.empty())
		return false;

	// Find out if we've got any lights turned on
	bool bLightingNeeded = false;

	TLightGroupList::const_iterator iCurLG = m_aLightGroups.begin();
	for (; iCurLG != m_aLightGroups.end(); ++iCurLG)
	{
		LTVector vLightAdd = iCurLG->m_vColor * 255.0f;
		bLightingNeeded |= ((uint32)vLightAdd.x != 0) || ((uint32)vLightAdd.y != 0) || ((uint32)vLightAdd.z != 0);
	}

	if (!bLightingNeeded)
		return false;

	// Create our new buffer
	LT_MEM_TRACK_ALLOC(pVertices = new SRBVertex[m_nVertexCount],LT_MEM_TYPE_RENDER_WORLD);

	memcpy(pVertices, m_aVertices, sizeof(SRBVertex) * m_nVertexCount);

	// Fill it
	iCurLG = m_aLightGroups.begin();
	for (; iCurLG != m_aLightGroups.end(); ++iCurLG)
	{
		// Skip lights which are turned off
		LTVector vMaxLightAdd = iCurLG->m_vColor * 255.0f;
		if (((uint32)vMaxLightAdd.x == 0) && ((uint32)vMaxLightAdd.y == 0) && ((uint32)vMaxLightAdd.z == 0))
			continue;

		SRBVertex *pOutVert = pVertices;
		SRBVertex *pEndOutVert = &pVertices[m_nVertexCount];

		SRBLightGroup::TVertexIntensityData::const_iterator iCurVI = iCurLG->m_aVertexIntensities.begin();

		for (; pOutVert != pEndOutVert; ++pOutVert)
		{
			uint32 nNextValue = *iCurVI;
			++iCurVI;
			if (iCurVI > iCurLG->m_aVertexIntensities.end())
			{
				ASSERT(!"Vertex intensity underflow encountered");
				break;
			}
			if (!nNextValue)
			{
				uint32 nSkip = *iCurVI;
				++iCurVI;
				if (iCurVI > iCurLG->m_aVertexIntensities.end())
				{
					ASSERT(!"Vertex intensity underflow encountered");
					break;
				}
				pOutVert += nSkip;
				if (pOutVert >= pEndOutVert)
				{
					ASSERT(!"Vertex intensity overflow encountered");
					break;
				}
				continue;
			}

			// Calculate the final lit value for this vertex
			LTVector vLightAdd = iCurLG->m_vColor * (float)nNextValue;
			uint32 nColorR = RGBA_GETR(pOutVert->m_nColor) + (uint32)vLightAdd.x;
			nColorR = LTMIN(nColorR, 0xFF);
			uint32 nColorG = RGBA_GETG(pOutVert->m_nColor) + (uint32)vLightAdd.y;
			nColorG = LTMIN(nColorG, 0xFF);
			uint32 nColorB = RGBA_GETB(pOutVert->m_nColor) + (uint32)vLightAdd.z;
			nColorB = LTMIN(nColorB, 0xFF);
			pOutVert->m_nColor = RGBA_MAKE(nColorR, nColorG, nColorB, RGBA_GETA(pOutVert->m_nColor));
		}
		ASSERT(iCurVI == iCurLG->m_aVertexIntensities.end());
	}

	return true;
}

void CD3D_RenderBlock::GetTriVertexColors(uint32 nBaseIndex, LTRGB aColors[3]) const
{
	// The triangle index array
	uint32 nTriIndices[3];
	nTriIndices[0] = m_aIndices[nBaseIndex];
	nTriIndices[1] = m_aIndices[nBaseIndex + 1];
	nTriIndices[2] = m_aIndices[nBaseIndex + 2];

	// Get the initial values
	for (uint32 nInitLoop = 0; nInitLoop < 3; ++nInitLoop)
	{
		uint32 nColor = m_aVertices[nTriIndices[nInitLoop]].m_nColor;
		aColors[nInitLoop].r = RGBA_GETR(nColor);
		aColors[nInitLoop].g = RGBA_GETG(nColor);
		aColors[nInitLoop].b = RGBA_GETB(nColor);
		aColors[nInitLoop].a = RGBA_GETA(nColor);
	}

	// The index remapping array
	uint32 nResultIndex[3];
	nResultIndex[0] = 0;
	nResultIndex[1] = 1;
	nResultIndex[2] = 2;

	// Sort the index array
	if (nTriIndices[0] > nTriIndices[1])
	{
		std::swap(nTriIndices[0], nTriIndices[1]);
		std::swap(nResultIndex[0], nResultIndex[1]);
	}
	if (nTriIndices[0] > nTriIndices[2])
	{
		std::swap(nTriIndices[0], nTriIndices[2]);
		std::swap(nResultIndex[0], nResultIndex[2]);
	}
	if (nTriIndices[1] > nTriIndices[2])
	{
		std::swap(nTriIndices[1], nTriIndices[2]);
		std::swap(nResultIndex[1], nResultIndex[2]);
	}

	// Fill it
	TLightGroupList::const_iterator iCurLG = m_aLightGroups.begin();
	for (; iCurLG != m_aLightGroups.end(); ++iCurLG)
	{
		// Skip lights which are turned off
		LTVector vMaxLightAdd = iCurLG->m_vColor * 255.0f;
		if (((uint32)vMaxLightAdd.x == 0) && ((uint32)vMaxLightAdd.y == 0) && ((uint32)vMaxLightAdd.z == 0))
			continue;

		uint32 nCurResult = 0;
		uint32 nCurIntensityOfs = 0;

		SRBLightGroup::TVertexIntensityData::const_iterator iCurVI = iCurLG->m_aVertexIntensities.begin();
		for (; (iCurVI != iCurLG->m_aVertexIntensities.end()) && (nCurResult < 3); ++iCurVI)
		{
			// Read in the next value
			uint32 nNextValue = *iCurVI;

			// Figure out the run length
			uint32 nRunLength = 1;
			if (!nNextValue)
			{
				++iCurVI;
				if (iCurVI > iCurLG->m_aVertexIntensities.end())
				{
					ASSERT(!"Vertex intensity underflow encountered");
					break;
				}
				nRunLength = *iCurVI;
				++iCurVI;
				if (iCurVI > iCurLG->m_aVertexIntensities.end())
				{
					ASSERT(!"Vertex intensity underflow encountered");
					break;
				}
				nNextValue = *iCurVI;
			}

			// Move forward
			nCurIntensityOfs += nRunLength;

			// If we've found the droids we're looking for, add in the color
			for (; (nCurResult < 3) && (nCurIntensityOfs > nTriIndices[nCurResult]); ++nCurResult)
			{
				LTRGB &nResult = aColors[nResultIndex[nCurResult]];
				LTVector vLightAdd = iCurLG->m_vColor * (float)nNextValue;
				uint32 nColorR = nResult.r + (uint32)vLightAdd.x;
				nResult.r = LTMIN(nColorR, 0xFF);
				uint32 nColorG = nResult.g + (uint32)vLightAdd.y;
				nResult.g = LTMIN(nColorG, 0xFF);
				uint32 nColorB = nResult.b + (uint32)vLightAdd.z;
				nResult.b = LTMIN(nColorB, 0xFF);
			}
		}
	}
}

void CD3D_RenderBlock::UpdateShaderLightmaps(bool bFullUpdate)
{
	if (m_aShaderIndices[eShader_Lightmap] == k_InvalidShaderIndex)
		return;

	uint8 *pLMScratchPad = 0;

	// Point directly at the lightmap shader
	CRenderShader *pShader = m_pWorld->GetShader(eShader_Lightmap);
	CRenderShader_Lightmap *pLMShader = reinterpret_cast<CRenderShader_Lightmap*>(pShader);

	for (uint32 nCurSection = 0; nCurSection < m_aSections.size(); ++nCurSection)
	{
		CRBSection *pCurSection = &m_aSections[nCurSection];

		// We're only interested in lightmap shaders
		if (pCurSection->m_eBoundShader != eShader_Lightmap)
			continue;

		uint8 *pLMData = 0;
		uint32 nLMDataStride;

		TLightGroupList::const_iterator iCurLG = m_aLightGroups.begin();
		for (; iCurLG != m_aLightGroups.end(); ++iCurLG)
		{
			// Get our sub-lightmap list
			const SRBLightGroup::TSubLMList *pSubLMList = iCurLG->m_aSectionLMs[nCurSection];
			if (pSubLMList == 0)
				continue;

			// Find out if this light's going to contribute anything
			LTVector vMaxLightAdd = iCurLG->m_vColor * 255.0f;
			bool bBlackLG = ((uint32)vMaxLightAdd.x == 0) && ((uint32)vMaxLightAdd.y == 0) && ((uint32)vMaxLightAdd.z == 0);

			// Skip lights that aren't going to contribute to the scene if we're doing the post-bind update
			if (bFullUpdate && bBlackLG)
				continue;

			// Get the initial lightmap data if we haven't done that yet
			if (!pLMData)
			{
				// Allocate a temporary buffer if we haven't done that yet
				if (!pLMScratchPad)
				{
					LT_MEM_TRACK_ALLOC(pLMScratchPad = new uint8[LIGHTMAP_MAX_TOTAL_PIXELS * 3],LT_MEM_TYPE_RENDER_LIGHTMAP);
				}

				// Point at the data
				pLMData = pLMScratchPad;
				nLMDataStride = pCurSection->m_nLightmapWidth * 3;

				// Decompress the section's lightmap
				DecompressLMData(pCurSection->m_pLightmapData, pCurSection->m_nLightmapSize, pLMData);
			}

			// Skip lights that aren't going to contribute to the scene if we're not doing the post-bind update
			// (Note : We'll always need to decompress the lightmap in that case...)
			if (!bFullUpdate && bBlackLG)
				continue;

			// Add in the sub-lightmaps
			SRBLightGroup::TSubLMList::const_iterator iCurSubLM = pSubLMList->begin();
			for (; iCurSubLM != pSubLMList->end(); ++iCurSubLM)
			{
				uint8 *pCurTexel = &pLMData[iCurSubLM->m_nTop * nLMDataStride + iCurSubLM->m_nLeft * 3];
				uint8 *pEndTexel = &pCurTexel[iCurSubLM->m_nHeight * nLMDataStride];
				uint32 nWidthRemaining = iCurSubLM->m_nWidth;
				uint32 nStrideSkip = nLMDataStride - iCurSubLM->m_nWidth * 3;
				SRBLightGroup::SSubLM::TDataList::const_iterator iCurInput = iCurSubLM->m_aData.begin();
				uint8 nRunLength = 0;
				uint8 nRunValueR, nRunValueG, nRunValueB;
				for (; pCurTexel != pEndTexel; pCurTexel += 3)
				{
					if (nRunLength)
					{
						uint32 nColorR = pCurTexel[0] + (uint32)nRunValueR;
						pCurTexel[0] = (uint8)LTMIN(nColorR, 0xFF);
						uint32 nColorG = pCurTexel[1] + (uint32)nRunValueG;
						pCurTexel[1] = (uint8)LTMIN(nColorG, 0xFF);
						uint32 nColorB = pCurTexel[2] + (uint32)nRunValueB;
						pCurTexel[2] = (uint8)LTMIN(nColorB, 0xFF);
						--nRunLength;
					}
					else
					{
						if (*iCurInput == 0xFF)
						{
							++iCurInput;
							nRunLength = *iCurInput;
							++iCurInput;
						}

						LTVector vLightAdd = iCurLG->m_vColor * (float)*iCurInput;
						uint32 nColorR = pCurTexel[0] + (uint32)vLightAdd.x;
						pCurTexel[0] = (uint8)LTMIN(nColorR, 0xFF);
						uint32 nColorG = pCurTexel[1] + (uint32)vLightAdd.y;
						pCurTexel[1] = (uint8)LTMIN(nColorG, 0xFF);
						uint32 nColorB = pCurTexel[2] + (uint32)vLightAdd.z;
						pCurTexel[2] = (uint8)LTMIN(nColorB, 0xFF);

						if (nRunLength)
						{
							nRunValueR = (uint8)LTMIN((uint32)vLightAdd.x, 0xFF);
							nRunValueG = (uint8)LTMIN((uint32)vLightAdd.y, 0xFF);
							nRunValueB = (uint8)LTMIN((uint32)vLightAdd.z, 0xFF);
						}

						++iCurInput;
					}

					--nWidthRemaining;
					if (!nWidthRemaining)
					{
						pCurTexel += nStrideSkip;
						nWidthRemaining = iCurSubLM->m_nWidth;
					}
				}
			}
		}

		// Update the lightmap in the shader
		if (pLMData)
			pLMShader->UpdateLightmap(m_aShaderIndices[eShader_Lightmap], *pCurSection, pLMData);
	}

	if (pLMScratchPad)
		delete[] pLMScratchPad;
}

LTRGB CD3D_RenderBlock::GetLMTexel(const CRBSection &cSection, float fU, float fV) const
{
	// Get the integer coordinates
	uint32 nTextureX = (uint32)(fU * (float)cSection.m_nLightmapWidth);
	uint32 nTextureY = (uint32)(fV * (float)cSection.m_nLightmapHeight);
	
	// Read the base lightmap
	LTRGB nResult;
	if (!GetLMDataTexel(cSection.m_pLightmapData, cSection.m_nLightmapSize, 
		cSection.m_nLightmapWidth, nTextureX, nTextureY, &nResult))
	{
		nResult.r = nResult.g = nResult.b = 0;
		nResult.a = 255;
	}

	TLightGroupList::const_iterator iCurLG = m_aLightGroups.begin();
	for (; iCurLG != m_aLightGroups.end(); ++iCurLG)
	{
		// Get our sub-lightmap list
		const SRBLightGroup::TSubLMList *pSubLMList = iCurLG->m_aSectionLMs[cSection.m_nIndex];
		if (pSubLMList == 0)
			continue;

		// Find out if this light's going to contribute anything
		LTVector vMaxLightAdd = iCurLG->m_vColor * 255.0f;
		bool bBlackLG = ((uint32)vMaxLightAdd.x == 0) && ((uint32)vMaxLightAdd.y == 0) && ((uint32)vMaxLightAdd.z == 0);

		// Skip lights that aren't going to contribute to the scene
		if (bBlackLG)
			continue;

		// Go through the sub-lightmaps
		SRBLightGroup::TSubLMList::const_iterator iCurSubLM = pSubLMList->begin();
		for (; iCurSubLM != pSubLMList->end(); ++iCurSubLM)
		{
			// Skip sub-lightmaps that don't have this texel in them
			if ((iCurSubLM->m_nLeft > nTextureX) || (iCurSubLM->m_nTop > nTextureY) ||
				((iCurSubLM->m_nLeft + iCurSubLM->m_nWidth) <= nTextureX) || 
				((iCurSubLM->m_nTop + iCurSubLM->m_nHeight) <= nTextureY))
				continue;

			// Figure out how far into the sublm data the texel is
			uint32 nAdjX = nTextureX - iCurSubLM->m_nLeft;
			uint32 nAdjY = nTextureY - iCurSubLM->m_nTop;
			uint32 nSubLMDataOfs = nAdjY * iCurSubLM->m_nWidth + nAdjX;
			// Read the LM data
			SRBLightGroup::SSubLM::TDataList::const_iterator iCurInput = iCurSubLM->m_aData.begin();
			uint32 nSubLMCurOfs = 0;
			for (; iCurInput != iCurSubLM->m_aData.end(); ++iCurInput)
			{
				// Get the run length
				uint32 nRunLength = 1;
				if (*iCurInput == 0xFF)
				{
					++iCurInput;
					nRunLength = *iCurInput;
					++iCurInput;
				}

				nSubLMCurOfs += nRunLength;

				// If this run took us past the data offset, we've got a winner
				if (nSubLMCurOfs > nSubLMDataOfs)
				{
					LTVector vLightAdd = iCurLG->m_vColor * (float)*iCurInput;
					uint32 nColorR = nResult.r + (uint32)vLightAdd.x;
					nResult.r = (uint8)LTMIN(nColorR, 0xFF);
					uint32 nColorG = nResult.g + (uint32)vLightAdd.y;
					nResult.g = (uint8)LTMIN(nColorG, 0xFF);
					uint32 nColorB = nResult.b + (uint32)vLightAdd.z;
					nResult.b = (uint8)LTMIN(nColorB, 0xFF);

					// Ok, we're done with this lightmap
					break;
				}
			}
		}
	}

	// And that's the end of the story
	return nResult;
}

void CD3D_RenderBlock::UpdateShaderData(bool bFullUpdate)
{
	UpdateShaderLightmaps(bFullUpdate);

	SRBVertex *pFinalVertices;
	bool bLit = GetLitVertexData(pFinalVertices);

	// Lock the shaders
	uint32 nEndShader = (bFullUpdate) ? (uint32)k_eShader_Num : (uint32)eShader_Lightmap;
	uint32 nCurShader;
	for (nCurShader = 0; nCurShader <= nEndShader; ++nCurShader)
	{
		if (m_aShaderIndices[nCurShader] == k_InvalidShaderIndex)
			continue;

		CRenderShader *pShader = m_pWorld->GetShader((ERenderShader)nCurShader);
		if (pShader)
			pShader->Lock(m_aShaderIndices[nCurShader]);
	}

	// Add the sections to the shaders
	TSectionList::const_iterator iCurSection = m_aSections.begin();
	for (; iCurSection != m_aSections.end(); ++iCurSection)
	{
		if ((uint32)iCurSection->m_eBoundShader > nEndShader)
			continue;

		CRenderShader *pShader = m_pWorld->GetShader(iCurSection->m_eBoundShader);
		if (pShader)
			pShader->AddSection(*iCurSection, m_aIndices, pFinalVertices, m_aVertices);
	}

 	//we now need to mark all the section boundaries
 	for (nCurShader = 0; nCurShader <= nEndShader; ++nCurShader)
 	{
 		if (m_aShaderIndices[nCurShader] == k_InvalidShaderIndex)
 			continue;
 
 		CRenderShader *pShader = m_pWorld->GetShader((ERenderShader)nCurShader);
 		if (pShader)
 			pShader->MarkSectionBreaks();
 	}

	// Unlock the shaders
	for (nCurShader = 0; nCurShader <= nEndShader; ++nCurShader)
	{
		if (m_aShaderIndices[nCurShader] == k_InvalidShaderIndex)
			continue;

		CRenderShader *pShader = m_pWorld->GetShader((ERenderShader)nCurShader);
		if (pShader)
			pShader->Unlock();
	}

	if (bLit)
		ReleaseLitVertexData(pFinalVertices);

	m_bShadersDirty = false;
}

void CD3D_RenderBlock::PreBind()
{
	// Preview the sections
	TSectionList::iterator iCurSection = m_aSections.begin();
	for (; iCurSection != m_aSections.end(); ++iCurSection)
	{
		CRBSection &cSection = *iCurSection;
		CRenderShader *pShader = m_pWorld->AllocShader(cSection);
		if (!pShader)
		{
			// This is valid in some cases..  For example, on the lightmap shader when lightmaps are disabled
			cSection.m_eBoundShader = eShader_Invalid;
			continue;
		}
		cSection.m_eBoundShader = pShader->GetShaderID();
		// Preview the renderblock if we haven't touched this shader yet
		if (m_aShaderIndices[cSection.m_eBoundShader] == k_InvalidShaderIndex)
			m_aShaderIndices[cSection.m_eBoundShader] = pShader->PreviewRenderBlock(*this);
		pShader->PreviewSection(cSection);
	}
}

void CD3D_RenderBlock::Bind()
{
	ASSERT(!m_bShadersBound);

	UpdateShaderData(true);

	m_bShadersBound = true;
}

void CD3D_RenderBlock::Release()
{
	if (!m_bShadersBound)
		return;

	for (uint32 nCurShader = 0; nCurShader < k_eShader_Num; ++nCurShader)
		m_aShaderIndices[nCurShader] = k_InvalidShaderIndex;

	m_bShadersBound = false;
}

ERenderShader CD3D_RenderBlock::TranslatePCShaderCode(uint32 nCode)
{
	switch ((EPCShaderType)nCode)
	{
		case ePCShader_None :
			return eShader_Invalid;
		case ePCShader_Gouraud :
			return eShader_Gouraud;
		case ePCShader_Lightmap :
			return eShader_Lightmap;
		case ePCShader_Lightmap_Texture :
			return eShader_Lightmap_Texture;
		case ePCShader_DualTexture:
			return eShader_Gouraud_DualTexture;
		case ePCShader_Lightmap_DualTexture:
			return eShader_Lightmap_DualTexture;
		default :
			return eShader_Invalid;
	}
}

// Triangle intersection code, adapted from code by Tomas Moller
bool CD3D_RenderBlock::IntersectTri(
	const SRay &sRay, 
	const LTVector &vVert0,
	const LTVector &vVert1,
	const LTVector &vVert2,
	float &fTime, 
	float &fU, 
	float &fV
) const
{
	// find vectors for two edges sharing vert0
	LTVector vEdge1 = vVert1 - vVert0;
	LTVector vEdge2 = vVert2 - vVert0;

	// begin calculating determinant - also used to calculate U parameter
	LTVector vP = vEdge2.Cross(sRay.m_vDir);

	float fDet = vEdge1.Dot(vP);

	// if determinant is near zero, ray lies in plane of triangle
	// Skip back-facing polys as well in this version
	if (fDet < 0.00001f)
		return false;

	// calculate distance from vert0 to ray origin
	LTVector vT = sRay.m_vOrigin - vVert0;
      
    // calculate U parameter and test bounds
    fU = vT.Dot(vP);
	if (fU < 0.0f || fU > fDet)
		return false;
      
	// prepare to test V parameter
	LTVector vQ = vEdge1.Cross(vT);

	// calculate V parameter and test bounds
	fV = vQ.Dot(sRay.m_vDir);
	if (fV < 0.0f || (fU + fV) > fDet)
		return false;

	float fInvDet = 1.0f / fDet;

	// calculate t, ray intersects triangle
	fTime = vEdge2.Dot(vQ) * fInvDet;
	fU *= fInvDet;
	fV *= fInvDet;

	return true;
}

bool Intersect_Ray_AABB(const CD3D_RenderBlock::SRay &sRay, const LTVector &vMin, const LTVector &vMax)
{
	const float k_fEpsilon = 0.0001f;

	float fRangeMin = sRay.m_fMin;
	float fRangeMax = sRay.m_fMax;

	for (uint32 nCurPlane = 0; nCurPlane < 3; ++nCurPlane)
	{
		float fInvRayDir = 1.0f / sRay.m_vDir[nCurPlane];
		float fLocalMin = (vMin[nCurPlane] - sRay.m_vOrigin[nCurPlane]) * fInvRayDir;
		float fLocalMax = (vMax[nCurPlane] - sRay.m_vOrigin[nCurPlane]) * fInvRayDir;
		if (fLocalMax < fLocalMin)
			std::swap(fLocalMax, fLocalMin);
		if ((fLocalMax < (fRangeMin - k_fEpsilon)) || (fLocalMin > (fRangeMax + k_fEpsilon)))
			return false;
		fRangeMin = LTMAX(fRangeMin, fLocalMin);
		fRangeMax = LTMIN(fRangeMax, fLocalMax);
	}
	return fRangeMax > (fRangeMin + k_fEpsilon);
}

uint32 CD3D_RenderBlock::IntersectRay(const SRay &sRay, TIntersectionList &sResults, EIntersectionType eIntersect, uint32 *pTrisChecked) const
{
	const float k_fEqualDistanceEpsilon = 0.0001f;

	sResults.clear();

	if (!Intersect_Ray_AABB(sRay, m_vBoundsMin, m_vBoundsMax))
		return 0;

	uint32 nIndexCount = m_nTriCount * 3;
	uint32 nIndexLoop;
	for (nIndexLoop = 0; nIndexLoop < nIndexCount; nIndexLoop += 3)
	{
		LTVector vPt0 = m_aVertices[m_aIndices[nIndexLoop]].m_vPos;
		LTVector vPt1 = m_aVertices[m_aIndices[nIndexLoop + 1]].m_vPos;
		LTVector vPt2 = m_aVertices[m_aIndices[nIndexLoop + 2]].m_vPos;

		float fTime, fU, fV;
		if (!IntersectTri(sRay, vPt0, vPt1, vPt2, fTime, fU, fV))
			continue;

		if (((fTime + k_fEqualDistanceEpsilon) < sRay.m_fMin) || ((fTime - k_fEqualDistanceEpsilon) > sRay.m_fMax))
			continue;

		SIntersection sCurIntersect(nIndexLoop / 3, fTime, fU, fV);

		// Handle overlapping triangles in the first intersection case
		// (Needed for lightmapped polygon handling..)
		if (eIntersect == eIntersectionType_First && !sResults.empty() && 
			(sResults.front().m_fDistance > (sCurIntersect.m_fDistance + k_fEqualDistanceEpsilon)))
		{
			sResults.clear();
		}

		sResults.push_back(sCurIntersect);

		if (eIntersect == eIntersectionType_Any)
			break;
	}

	if (pTrisChecked)
		*pTrisChecked = nIndexLoop + 1;

	return sResults.size();
}

void CD3D_RenderBlock::DebugTri(const SIntersection &sIntersect, float fX, float fY, float fSizeX, float fSizeY) const
{
	// Find the section containing the tri
	const CRBSection *pSection = 0;
	uint32 nTriIndex = sIntersect.m_nTriIndex;
	for (uint32 nCurSection = 0; nCurSection < m_aSections.size(); ++nCurSection)
	{
		if (nTriIndex < m_aSections[nCurSection].m_nTriCount)
		{
			pSection = &m_aSections[nCurSection];
			break;
		}
		nTriIndex -= m_aSections[nCurSection].m_nTriCount;
	}

	// Make sure we found a section
	if (!pSection)
	{
		ASSERT(!"Section including debug triangle not found");
		return;
	}

	// Make sure we're using a bound shader
	if (pSection->m_eBoundShader == eShader_Invalid)
	{
		return;
	}

	// Hand it off to the shader for the actual drawing
	m_pWorld->GetShader(pSection->m_eBoundShader)->DebugTri(
		m_aShaderIndices[pSection->m_eBoundShader],
		*pSection, sIntersect.m_nTriIndex * 3, m_aIndices, m_aVertices, fX, fY, fSizeX, fSizeY);
}

// Note : Currently, this doesn't do a very accurate job of clipping, and can end up with the entire
// screen filled with sky when it shouldn't.
void CD3D_RenderBlock::ExtendSkyBounds(const ViewParams& Params, float &fMinX, float &fMinY, float &fMaxX, float &fMaxY) const
{
	TGeometryPolyList::const_iterator iCurPoly = m_aSkyPortals.begin();
	for (; iCurPoly != m_aSkyPortals.end(); ++iCurPoly)
	{
		const SRBGeometryPoly &sCurPoly = *iCurPoly;

		uint32 nNear = 0;
		float fLocalMinX = FLT_MAX, fLocalMinY = FLT_MAX, fLocalMaxX = -FLT_MAX, fLocalMaxY = -FLT_MAX;

		LTVector vPrevVert = sCurPoly.m_aVerts.back();
		float fPrevNearDist = Params.m_ClipPlanes[CPLANE_NEAR_INDEX].DistTo(vPrevVert);
		bool bPrevClip = fPrevNearDist > 0.0f;

		SRBGeometryPoly::TVertList::const_iterator iCurVert = sCurPoly.m_aVerts.begin();
		for (; iCurVert != sCurPoly.m_aVerts.end(); ++iCurVert)
		{
			LTVector vCurVert = *iCurVert;
			// Calc the clipping state
			float fCurNearDist = Params.m_ClipPlanes[CPLANE_NEAR_INDEX].DistTo(vCurVert);
			bool bCurClip = fCurNearDist > 0.0f;

			bool bVertOK = true;

			if (bCurClip != bPrevClip)
			{
				float fInterpolant = (fCurNearDist != fPrevNearDist) ? -fPrevNearDist / (fCurNearDist - fPrevNearDist) : 0.0f;
				LTVector vTemp;
				VEC_LERP(vTemp, vPrevVert, vCurVert, fInterpolant);
				vCurVert = vTemp;
			}

			// Transform all the way to screen space
			MatVMul_InPlace_H(&(Params.m_FullTransform), &vCurVert);
			// Update the extents
			fLocalMinX = LTMIN(fLocalMinX, vCurVert.x);
			fLocalMinY = LTMIN(fLocalMinY, vCurVert.y);
			fLocalMaxX = LTMAX(fLocalMaxX, vCurVert.x);
			fLocalMaxY = LTMAX(fLocalMaxY, vCurVert.y);

			vPrevVert = *iCurVert;
			fPrevNearDist = fCurNearDist;
			bPrevClip = bCurClip;
		}
		// Skip over polys that are entirely outside the clipping area
		if ((nNear == sCurPoly.m_aVerts.size()) || 
		// SKYBOX FIX for Multiple Cameras
			/*
			(fLocalMaxX < 0.0f) ||
			(fLocalMaxY < 0.0f) ||
			(fLocalMinX > Params.m_fScreenWidth) ||
			(fLocalMinY > Params.m_fScreenHeight))
			*/
			(fLocalMaxX < static_cast<float>(Params.m_Rect.left))  ||
			(fLocalMaxY < static_cast<float>(Params.m_Rect.top))   ||
			(fLocalMinX > static_cast<float>(Params.m_Rect.right)) ||
			(fLocalMinY > static_cast<float>(Params.m_Rect.bottom)) )
			continue;

		// SKYBOX FIX for Multiple Cameras
		/*
		// Clamp to the screen
		fLocalMinX = LTMAX(0.0f, fLocalMinX);
		fLocalMinY = LTMAX(0.0f, fLocalMinY);
		fLocalMaxX = LTMIN(Params.m_fScreenWidth, fLocalMaxX);
		fLocalMaxY = LTMIN(Params.m_fScreenHeight, fLocalMaxY);
		*/
		//Clamp to the viewport
		fLocalMinX = LTMAX(static_cast<float>(Params.m_Rect.left), fLocalMinX);
		fLocalMinY = LTMAX(static_cast<float>(Params.m_Rect.top), fLocalMinY);
		fLocalMaxX = LTMIN(static_cast<float>(Params.m_Rect.right), fLocalMaxX);
		fLocalMaxY = LTMIN(static_cast<float>(Params.m_Rect.bottom), fLocalMaxY);

		// Extend the bounds
		fMinX = LTMIN(fMinX, fLocalMinX);
		fMinY = LTMIN(fMinY, fLocalMinY);
		fMaxX = LTMAX(fMaxX, fLocalMaxX);
		fMaxY = LTMAX(fMaxY, fLocalMaxY);

		IncFrameStat(eFS_SkyPortals, 1);
	}
}

void CD3D_RenderBlock::SetLightGroupColor(uint32 nID, const LTVector &vColor)
{
	TLightGroupList::iterator iCurLG = m_aLightGroups.begin();
	for (; iCurLG != m_aLightGroups.end(); ++iCurLG)
	{
		if (iCurLG->m_nID == nID)
		{
			iCurLG->m_vColor = vColor;
			m_bShadersDirty = true;
			break;
		}
	}
}

void CD3D_RenderBlock::GetMemStats(CMemStats_World &cMemStats) const
{
	++cMemStats.m_nRenderBlockCount;
	cMemStats.m_nRenderBlockData += sizeof(CD3D_RenderBlock);

	ASSERT(m_bShadersBound);

	for (uint32 nShaderLoop = 0; nShaderLoop < k_eShader_Num; ++nShaderLoop)
	{
		if (m_aShaderIndices[nShaderLoop] != k_InvalidShaderIndex)
			m_pWorld->GetShader((ERenderShader)nShaderLoop)->GetMemStats(cMemStats);
	}

	// Add in the dynamic light memory usage
	// Note : The dynamic light shader automatically only reports its usage once
	//		per frame so that it doesn't have to be reported from a global level
	CRenderShader_DynamicLight::GetSingleton()->GetMemStats(cMemStats);

	// Add in the light animation data
	cMemStats.m_nLightAnimData += sizeof(SRBLightGroup) * m_aLightGroups.size();
	TLightGroupList::const_iterator iCurLG = m_aLightGroups.begin();
	for (; iCurLG != m_aLightGroups.end(); ++iCurLG)
	{
		cMemStats.m_nLightAnimData += iCurLG->m_aVertexIntensities.size();
	}
}

void CD3D_RenderBlock::GetIntersectInfo(const SIntersection &sIntersect, 
	LTRGB *pResult_LightingColor, 
	SharedTexture **pResult_Texture, LTRGB *pResult_TextureColor,
	LTVector *pResult_InterpNormal, LTVector *pResult_TriNormal) const
{
	uint32 nTriBaseIndex = sIntersect.m_nTriIndex * 3;

	const SRBVertex &sVert0 = m_aVertices[m_aIndices[nTriBaseIndex]];
	const SRBVertex &sVert1 = m_aVertices[m_aIndices[nTriBaseIndex + 1]];
	const SRBVertex &sVert2 = m_aVertices[m_aIndices[nTriBaseIndex + 2]];

	// Get the normals
	if (pResult_InterpNormal)
	{
		LTVector vUInterpNormal;
		VEC_LERP(vUInterpNormal, sVert0.m_vNormal, sVert1.m_vNormal, sIntersect.m_fU);
		LTVector vVInterpNormal;
		VEC_LERP(vVInterpNormal, sVert0.m_vNormal, sVert2.m_vNormal, sIntersect.m_fV);
		*pResult_InterpNormal = vUInterpNormal + vVInterpNormal;
		pResult_InterpNormal->Normalize();
	}

	if (pResult_TriNormal)
	{
		*pResult_TriNormal = (sVert2.m_vPos - sVert1.m_vPos).Cross(sVert0.m_vPos - sVert1.m_vPos);
		pResult_TriNormal->Normalize();
	}

	// If we don't need any texture stuff, we're done
	if (!pResult_LightingColor && !pResult_Texture && !pResult_TextureColor)
		return;

	// Find the section containing the tri
	const CRBSection *pSection = 0;
	uint32 nSectionTriIndex = sIntersect.m_nTriIndex;
	for (uint32 nCurSection = 0; nCurSection < m_aSections.size(); ++nCurSection)
	{
		if (nSectionTriIndex < m_aSections[nCurSection].m_nTriCount)
		{
			pSection = &m_aSections[nCurSection];
			break;
		}
		nSectionTriIndex -= m_aSections[nCurSection].m_nTriCount;
	}

	// Make sure we found a section
	if (!pSection)
	{
		ASSERT(!"Section including debug triangle not found");
		return;
	}

	// Make sure we're using a bound shader
	if (pSection->m_eBoundShader == eShader_Invalid)
	{
		return;
	}

	// Figure out the texture UV's
	float fTextureU = LTLERP(sVert0.m_fU0, sVert1.m_fU0, sIntersect.m_fU) + 
						LTLERP(sVert0.m_fU0, sVert2.m_fU0, sIntersect.m_fV);
	float fTextureV = LTLERP(sVert0.m_fV0, sVert1.m_fV0, sIntersect.m_fU) + 
						LTLERP(sVert0.m_fV0, sVert2.m_fV0, sIntersect.m_fV);
	// Wrap the texture coords
	// Note : There should probably be some shader interaction on this one so clamped addressing will work
	fTextureU = fmodf(fTextureU, 1.0f);
	if (fTextureU < 0.0f)
		fTextureU = fmodf(fTextureU + 1.0f, 1.0f);
	fTextureV = fmodf(fTextureV, 1.0f);
	if (fTextureV < 0.0f)
		fTextureV = fmodf(fTextureV + 1.0f, 1.0f);

	// Get the texture information
	// Note : The lightmap shader doesn't have a texture associated with it...
	if (pSection->m_eBoundShader != eShader_Lightmap)
	{
		// Get the texture
		if (pResult_Texture)
		{
			*pResult_Texture = pSection->m_pTexture[0];
		}

		// Get the texture color at the UV coords
		if (pResult_TextureColor)
		{
			// Find the texture's data
			TextureData* pTextureData = g_pStruct->GetTexture(pSection->m_pTexture[0]);
			if (pTextureData)
			{
				TextureMipData &cTopMip = pTextureData->m_Mips[0];
				if (cTopMip.m_Data)
				{
					PFormat cTextureFormat; 
					pTextureData->SetupPFormat(&cTextureFormat);
					
					// Point at the texel we hit
					// Note : There should probably be some shader interaction on this one so panning will work
					uint32 nTextureX = (uint32)(fTextureU * (float)cTopMip.m_Width);
					uint32 nTextureY = (uint32)(fTextureV * (float)cTopMip.m_Height);
					uint32 nTexelIndex = cTopMip.m_Pitch * nTextureY + cTextureFormat.GetBytesPerPixel() * nTextureX;
					GenericColor *pTexel = (GenericColor *)cTopMip.m_Data[nTexelIndex];

					// Convert it from the texture format
					PValue nFinalColor;
					g_FormatMgr.PValueFromFormatColor(&cTextureFormat, *pTexel, nFinalColor);
					pResult_TextureColor->r = PValue_GetR(nFinalColor);
					pResult_TextureColor->g = PValue_GetG(nFinalColor);
					pResult_TextureColor->b = PValue_GetB(nFinalColor);
					pResult_TextureColor->a = PValue_GetA(nFinalColor);
				}
			}
		}
	}

	// Get the lighting value
	if ((pResult_LightingColor) && (pSection->m_eBoundShader <= eShader_Lightmap))
	{
		// Get the lightmap texel
		if (pSection->m_eBoundShader == eShader_Lightmap)
		{
			*pResult_LightingColor = GetLMTexel(*pSection, fTextureU, fTextureV);
		}
		// Otherwise get the vertex-based lighting value
		else
		{
			LTRGB aVertexColors[3];
			GetTriVertexColors(nTriBaseIndex, aVertexColors);
			pResult_LightingColor->r = (uint8)(
				LTLERP((float)aVertexColors[0].r, (float)aVertexColors[1].r, sIntersect.m_fU) + 
				LTLERP((float)aVertexColors[0].r, (float)aVertexColors[2].r, sIntersect.m_fV));
			pResult_LightingColor->g = (uint8)(
				LTLERP((float)aVertexColors[0].g, (float)aVertexColors[1].g, sIntersect.m_fU) + 
				LTLERP((float)aVertexColors[0].g, (float)aVertexColors[2].g, sIntersect.m_fV));
			pResult_LightingColor->b = (uint8)(
				LTLERP((float)aVertexColors[0].b, (float)aVertexColors[1].b, sIntersect.m_fU) + 
				LTLERP((float)aVertexColors[0].b, (float)aVertexColors[2].b, sIntersect.m_fV));
			pResult_LightingColor->a = (uint8)(
				LTLERP((float)aVertexColors[0].a, (float)aVertexColors[1].a, sIntersect.m_fU) + 
				LTLERP((float)aVertexColors[0].a, (float)aVertexColors[2].a, sIntersect.m_fV));
		}
	}
}

bool CD3D_RenderBlock::SetOccluderEnabled(uint32 nID, bool bEnabled)
{
	bool bResult = false;
	TOccluderList::iterator iCurOccluder = m_aOccluders.begin();
	for (; iCurOccluder != m_aOccluders.end(); ++iCurOccluder)
	{
		if (iCurOccluder->m_nID == nID)
		{
			iCurOccluder->m_bEnabled = bEnabled;
			bResult = true;
		}
	}

	return bResult;
}

bool CD3D_RenderBlock::GetOccluderEnabled(uint32 nID, bool *pResult) const
{
	TOccluderList::const_iterator iCurOccluder = m_aOccluders.begin();
	for (; iCurOccluder != m_aOccluders.end(); ++iCurOccluder)
	{
		if (iCurOccluder->m_nID == nID)
		{
			*pResult = iCurOccluder->m_bEnabled;
			return true;
		}
	}

	return false;
}

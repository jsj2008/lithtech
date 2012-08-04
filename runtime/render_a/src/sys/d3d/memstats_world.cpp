//////////////////////////////////////////////////////////////////////////////
// CMemStats_World implementation

#include "precompile.h"

#include "memstats_world.h"

#include "strtools.h"

CMemStats_World::CMemStats_World()
{
	Clear();
}

CMemStats_World::CMemStats_World(const CMemStats_World &cOther) :
	m_nTextureCount(cOther.m_nTextureCount), m_nTextureData(cOther.m_nTextureData),
	m_nLightmapCount(cOther.m_nLightmapCount), m_nLightmapData(cOther.m_nLightmapData),
	m_nRenderBlockCount(cOther.m_nRenderBlockCount), m_nRenderBlockData(cOther.m_nRenderBlockData),
	m_nVertexCount(cOther.m_nVertexCount), m_nVertexData(cOther.m_nVertexData),
	m_nTriangleCount(cOther.m_nTriangleCount), m_nTriangleData(cOther.m_nTriangleData),
	m_nDynamicTextureCount(cOther.m_nDynamicTextureCount), m_nDynamicTextureData(cOther.m_nDynamicTextureData),
	m_nLightAnimData(cOther.m_nLightAnimData),
	m_cKnownTextures(cOther.m_cKnownTextures)
{
}

CMemStats_World::~CMemStats_World()
{
}
	
void CMemStats_World::Clear()
{
	m_nTextureCount = 0;
	m_nTextureData = 0;
	m_nLightmapCount = 0;
	m_nLightmapData = 0;
	m_nRenderBlockCount = 0;
	m_nRenderBlockData = 0;
	m_nVertexCount = 0;
	m_nVertexData = 0;
	m_nTriangleCount = 0;
	m_nTriangleData = 0;
	m_nDynamicTextureCount = 0;
	m_nDynamicTextureData = 0;
	m_nLightAnimData = 0;
	m_cKnownTextures.clear();
}

void CMemStats_World::Report(const char *pPrefix)
{
	uint32 nTotal = (m_nTextureData + m_nDynamicTextureData + m_nLightmapData + 
		m_nLightAnimData + m_nRenderBlockData + m_nVertexData + m_nTriangleData);

	float fToMeg = float(1024*1024);

	dsi_ConsolePrint("%sTextures         : %7d (%5.2f MB)", pPrefix, m_nTextureCount, float(m_nTextureData) / fToMeg);
	dsi_ConsolePrint("%sDynamic Textures : %7d (%5.2f MB)", pPrefix, m_nDynamicTextureCount, float(m_nDynamicTextureData) / fToMeg);
	dsi_ConsolePrint("%sLightmaps        : %7d (%5.2f MB)", pPrefix, m_nLightmapCount, float(m_nLightmapData) / fToMeg);
	dsi_ConsolePrint("%sLight Animations :         (%5.2f MB)", pPrefix, float(m_nLightAnimData) / fToMeg);
	dsi_ConsolePrint("%sRenderBlocks     : %7d (%5.2f MB)", pPrefix, m_nRenderBlockCount, float(m_nRenderBlockData) / fToMeg);
	dsi_ConsolePrint("%sVertices         : %7d (%5.2f MB)", pPrefix, m_nVertexCount, float(m_nVertexData) / fToMeg);
	dsi_ConsolePrint("%sTriangles        : %7d (%5.2f MB)", pPrefix, m_nTriangleCount, float(m_nTriangleData) / fToMeg);
	dsi_ConsolePrint("%s=====================================", pPrefix);
	dsi_ConsolePrint("%sTotal            :         (%5.2f MB)", pPrefix, float(nTotal) / fToMeg);
	dsi_ConsolePrint("%s=====================================", pPrefix);
}

CMemStats_World &CMemStats_World::operator=(const CMemStats_World &cOther)
{
	if (&cOther == this)
		return *this;

	m_nTextureCount = cOther.m_nTextureCount;
	m_nTextureData = cOther.m_nTextureData;
	m_nLightmapCount = cOther.m_nLightmapCount;
	m_nLightmapData = cOther.m_nLightmapData;
	m_nRenderBlockCount = cOther.m_nRenderBlockCount;
	m_nRenderBlockData = cOther.m_nRenderBlockData;
	m_nVertexCount = cOther.m_nVertexCount;
	m_nVertexData = cOther.m_nVertexData;
	m_nTriangleCount = cOther.m_nTriangleCount;
	m_nTriangleData = cOther.m_nTriangleData;
	m_nDynamicTextureCount = cOther.m_nDynamicTextureCount;
	m_nDynamicTextureData = cOther.m_nDynamicTextureData;
	m_nLightAnimData = cOther.m_nLightAnimData;

	m_cKnownTextures = cOther.m_cKnownTextures;

	return *this;
}

CMemStats_World &CMemStats_World::operator+=(const CMemStats_World &cOther)
{
	m_nTextureCount += cOther.m_nTextureCount;
	m_nTextureData += cOther.m_nTextureData;
	m_nLightmapCount += cOther.m_nLightmapCount;
	m_nLightmapData += cOther.m_nLightmapData;
	m_nRenderBlockCount += cOther.m_nRenderBlockCount;
	m_nRenderBlockData += cOther.m_nRenderBlockData;
	m_nVertexCount += cOther.m_nVertexCount;
	m_nVertexData += cOther.m_nVertexData;
	m_nTriangleCount += cOther.m_nTriangleCount;
	m_nTriangleData += cOther.m_nTriangleData;
	m_nDynamicTextureCount += cOther.m_nDynamicTextureCount;
	m_nDynamicTextureData += cOther.m_nDynamicTextureData;
	m_nLightAnimData += cOther.m_nLightAnimData;

	if (&cOther != this)
		m_cKnownTextures.insert(cOther.m_cKnownTextures.begin(), cOther.m_cKnownTextures.end());

	return *this;
}

void CMemStats_World::CountTexture(const char *pName, uint32 nDataSize)
{
	uint32 nNameHash = st_GetHashCode(pName);
	if (m_cKnownTextures.find(nNameHash) != m_cKnownTextures.end())
		return;

	m_cKnownTextures.insert(nNameHash);

	++m_nTextureCount;
	m_nTextureData += nDataSize;
}

void CMemStats_World::FilterTextures(const CMemStats_World &cOther)
{
	m_cKnownTextures.insert(cOther.m_cKnownTextures.begin(), cOther.m_cKnownTextures.end());
}
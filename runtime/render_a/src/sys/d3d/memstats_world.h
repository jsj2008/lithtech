//////////////////////////////////////////////////////////////////////////////
// Memory stats for world data

#ifndef __MEMSTATS_WORLD_H__
#define __MEMSTATS_WORLD_H__

#include <set>

class CMemStats_World
{
public:
	CMemStats_World();
	CMemStats_World(const CMemStats_World &cOther);
	~CMemStats_World();
	
	// Reset the accumulated data
	void Clear();
	// Dump the accumulated data to the console
	void Report(const char *pPrefix = "  ");

	// Accumulate the given texture into the count, filtering out duplicates
	void CountTexture(const char *pName, uint32 nDataSize);

	// Consider the textures from another stats container to already be "used"
	void FilterTextures(const CMemStats_World &cOther);

	// Operators
	CMemStats_World &operator=(const CMemStats_World &cOther);
	CMemStats_World &operator+=(const CMemStats_World &cOther);

public:

	// Direct-access statistics variables
	uint32 m_nLightmapCount, m_nLightmapData;
	uint32 m_nRenderBlockCount, m_nRenderBlockData;
	uint32 m_nVertexCount, m_nVertexData;
	uint32 m_nTriangleCount, m_nTriangleData;
	uint32 m_nDynamicTextureCount, m_nDynamicTextureData;
	uint32 m_nLightAnimData;

private:
	// Texture data is private because it is tracked based on unique texture names
	uint32 m_nTextureCount, m_nTextureData;

	typedef set<uint32> TTextureSet;
	TTextureSet m_cKnownTextures;
};

#endif //__MEMSTATS_WORLD_H__
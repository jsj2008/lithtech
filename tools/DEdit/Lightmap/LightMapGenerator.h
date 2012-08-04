#ifndef __LIGHTMAPGENERATOR_H__
#define __LIGHTMAPGENERATOR_H__

#ifndef __LIGHTMAPALLOCATOR_H__
#	include "LightMapAllocator.h"
#endif

#ifndef __LMSAMPLEGEN_H__
#	include "LMSampleGen.h"
#endif

#ifndef __HOLDERLIST_H__
#	include "HolderList.h"
#endif

class CHolderNode;
class ILightMapHolder;
class ILitVertexHolder;


class CLightMapGenerator
{
public:

	CLightMapGenerator();
	~CLightMapGenerator();

	//registers a holder as dirty, this will treat this polygon as freshly dirtied
	//and if it is being processed, that processing will be cancelled.
	bool DirtyLightMapHolder(ILightMapHolder* pHolder, bool bSearchForDuplicates);

	//registers an object that is vertex lit as being dirtied. Duplicates will not
	//be added and if it is being processed, it will be reset
	bool DirtyVertexHolder(ILitVertexHolder* pHolder);

	//sorts the holder list based upon a camera position and orientation
	bool SortDirtyList(const LTVector& vCameraPos, const LTVector& vCameraDir);

	//called to caluclate lightmaps for the specified amount of time (in ms)
	//This will proceed to calculate lightmaps for that period of time. Returns
	//the number of lightmaps finished
	uint32 CalculateLightmaps(uint32 nNumMS);

	//called when a holder is being removed. It then needs to be pulled out of the
	//list of holders to be processed
	bool RemoveLightMapHolder(ILightMapHolder* pHolder);
	bool RemoveVertexHolder(ILitVertexHolder* pHolder);

	//determines if there are any holders left to be processed
	bool IsHolderListEmpty() const;

	//this is called to clear the entire list of holders
	void ClearHolderList();

	//the holder lists
	//--------------------------------------------------------------------

	//the list of the holders that are to have vertex lighting calculated
	CHolderList				m_VertexList;

	//the list of the holders that will have lightmaps calculated for them
	CHolderList				m_LightMapList;


	//the allocator that handles the creation of surfaces
	CLightMapAllocator		m_Allocator;

	//the actual sample calculator
	CLMSampleGen			m_SampleGen;
	
private:

	// Lightmap calculations
	//--------------------------------------------------------------------

	//resets the lightmap that is currently being generated
	void						ResetActiveLightMap();

	//sets up the active lightmap data to correspond to the information needed to build
	//the lightmap information for the first holder in the list
	bool						SetupNewActiveLightMap();

	//finishes the currently active lightmap
	void						FinishActiveLightMap();

	//calculates the next active lightmap texel
	void						CalculateActiveTexel();

	// Vertex calculations
	//--------------------------------------------------------------------

	//sets up for a new vertex light holder
	void						SetupNewVertex();

	//resets the vertex data that is currently being generated
	void						ResetActiveVertex();

	//finishes the currently active vertex holder
	void						FinishActiveVertex();

	//calculates the next active vertex color
	void						CalculateActiveVertex();

	//Variables used to track the current vertex that is being generated
	//--------------------------------------------------------------------

	//the current vertex index that needs to be calculated
	uint32						m_nCurrVertex;

	//the options used for both types of calculations
	CLightHolderOptions			m_LightOptions;

	//Variables used to track the current lightmap that is being generated
	//--------------------------------------------------------------------

	//the lightmap currently being processed
	CLightMapData*				m_pCurrLightMap;

	//the offset of the current pixel being generated
	uint8						m_nCurrX;
	uint8						m_nCurrY;

	//the vector of the current holder
	LTVector					m_vCurrNormal;

	//the origin in space of this holder
	LTVector					m_vCurrO;

	//the vector needed to move a texel in space along the X axis
	LTVector					m_vCurrXInc;

	//the vector needed to move a texel in space along the Y axis
	LTVector					m_vCurrYInc;
};

#endif


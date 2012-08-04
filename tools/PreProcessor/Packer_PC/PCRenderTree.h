//////////////////////////////////////////////////////////////////////////////
// Octree representation of the PC world

#ifndef __PCRenderTree_H__
#define __PCRenderTree_H__

class CPreMainWorld;
class CPreWorld;
class CPCRenderTreeNode;
class CPreLightMap;
class CPreLightAnim;
class CPrePoly;
class CPreLightGroup;

#include <map>
#include <vector>

class CPCRenderTree
{
public:
	CPCRenderTree();
	CPCRenderTree(const CPCRenderTree &cOther);
	CPCRenderTree &operator=(const CPCRenderTree &cOther);
	~CPCRenderTree();

	// Read in the initial node of the octree based on the polys in the provided world
	bool ReadWorld(const CPreMainWorld *pMainWorld, const CPreWorld *pWorld);

	// Write the results of all processing to the specified file
	bool Write(CAbstractIO &file);

	// Optimize the octree (split to threshold, insert t-junctions, all that jazz)
	bool Optimize(bool bDisplayStats);

	//////////////////////////////////////////////////////////////////////////////
	// Utilities that the nodes use while optimizing

	// Find the lightmap data associated with the given poly/animation combo
	// Returns NULL if the poly/anim combo isn't found
	const CPreLightMap *GetPolyLMData(const CPreLightAnim *pLMAnim, const CPrePoly *pPoly) const;

private:
	// Count the number of nodes in the tree
	uint32 CPCRenderTree::CountNodes();
	
	// The root node of the octree
	CPCRenderTreeNode *m_pRoot;

	typedef std::map<const CPrePoly *, const CPreLightMap *> TPolyLMDataMap;
	typedef std::map<const CPreLightAnim *, TPolyLMDataMap> TLMDataMap;
	TLMDataMap m_cLMData;

	// We need to keep around the world's light group list for a while during processing...
	typedef std::vector<CPreLightGroup*> TLightGroupList;
	TLightGroupList m_aLightGroupList;
};

#endif //__PCRenderTree_H__
#ifndef __CREATEPOLYEDGES_H__
#define __CREATEPOLYEDGES_H__

class CEditRegion;

//given a world, it will go through finding the brushes
//marked for generation of edges, and upon finding them
//will break the brush apart into polygon brushes, and
//generate the appropriate polygons for the edging
bool CreatePolyEdges(CEditRegion* pRegion);

#endif

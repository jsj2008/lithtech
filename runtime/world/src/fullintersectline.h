#ifndef __FULLINTERSECTLINE_H__
#define __FULLINTERSECTLINE_H__

class WorldTree;

bool i_BoundingBoxTest(const LTVector& Point1, const LTVector& Point2, const LTObject *pServerObj, 
    LTVector *pIntersectPt, LTPlane *pIntersectPlane);
bool i_IntersectSegment(IntersectQuery* pQuery, IntersectInfo *pInfo, WorldTree* pWorldTree);

#endif




#ifndef __INTERSECT_LINE_H__
#define __INTERSECT_LINE_H__


typedef void (*ILineCallback)(Node *pNode);

    
class IntersectRequest
{
public:
    IntersectRequest()
    {
        m_pPoints[0] = NULL;
        m_pPoints[1] = NULL;
        m_pIPos      = NULL;

        m_pNodeHit   = NULL;
        m_pQuery     = NULL;
        m_pWorldBsp  = NULL;
    }

// Input to the routine.
public:
    LTVector        *m_pPoints[2];
    LTVector        *m_pIPos;   // Intersection position.
    IntersectQuery  *m_pQuery;
    const WorldBsp  *m_pWorldBsp;

// Output (if it returns LTTRUE).
public:
    const Node          *m_pNodeHit;
};


// Intersect the line segment with the BSP.  If callback is not LTNULL, it is called at
// each node the intersection routine touches.  The node that this returns is NOT
// guaranteed to be the node it actually hit but it will lie on the same plane.
const Node* IntersectLine(const Node *pRoot, LTVector *pPoint1, LTVector *pPoint2, 
    LTVector *pIPos, LTPlane *pIPlane);

// This version returns the node that the line segment hit but is MUCH slower
// than IntersectLine.  Only use it as a last resort.
LTBOOL IntersectLineNode(const Node *pRoot, IntersectRequest *pRequest);


#endif  // __INTERSECT_LINE_H__




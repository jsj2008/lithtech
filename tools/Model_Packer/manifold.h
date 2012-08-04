// manifold.h

#ifndef MANIFOLD_H
#define MANIFOLD_H

#pragma warning(disable:4786)

/* ------------------------------------------------------------
   Manifold Class
   ------------------------------------------------------------
   This class provides methods for finding adjacent triangles and
   finding adjacent vertices. I am using it to create triangle strips
   for the PS2 model packer.

   I'm assuming that the surfaces that I'm dealing with are manifold,
   which pretty much means that each triangle has exactly one neighbor
   across each of its edges.
   ------------------------------------------------------------ */

#include <vector>
#include <map>
#include <algorithm>

typedef int IndexType;

typedef std::pair<IndexType, IndexType> IndexPair;

class Point
{
 public:
    Point(float px, float py, float pz):x(px),y(py),z(pz){}

    // edges using this point in no particular order
    std::vector<unsigned int> edges;
    float x,y,z;

    int distanceFromPatchCenter;
};

class VertAuxData
{
 public:
    float normal[3];
    unsigned char rgba[4];
    float uv[2];

    bool operator==(VertAuxData& d);

    static float normalTolerance;
    static float colorTolerance;
    static float textureTolerance;

    // whether to even check colors to determine equality
    static bool m_bCheckColor;
};

class Triangle
{
 public:
    // verts are intended to be specified in a right hand
    // (counterclockwise) fashion. But you do what you want.
    
    unsigned int vertIndex[3];

    // Aha. This is interesting. Each vertex has associated with it
    // some amount of data (perhaps that amount is zero, but at least
    // in the PS2 case, it's not) like normals, UVs, and color. You
    // want to make sure that all the vertex data is shared - for
    // instance if you had per-face normals, you wouldn't want to use
    // the same vertex info on more than one face.
    
    VertAuxData vertData[3];
    
    // edge 0 is between vert 0 and vert 1.
    
    unsigned int edgeIndex[3];

    // adjTri 0 is over edge 0
    
    unsigned int adjTriIndex[3];

    // adjTriEdge 0 is the edge of adjTri 0 that connects vert
    // 0 and vert 1.
    
    unsigned int adjTriEdgeIndex[3];

    unsigned int vertPos(unsigned int vIndex);

    unsigned int edgePos(unsigned int eIndex);

    unsigned int edgeFromVertIndices(unsigned int vIndex0,
                                     unsigned int vIndex1);

    /* returns 0, 1, or 2 for which edge these verts describe. */
    
    unsigned int edgePosFromVertIndices(unsigned int vIndex0,
                                        unsigned int vIndex1);
    
    unsigned int oppositeVertex(unsigned int vIndex0,
                                unsigned int vIndex1);

    void setVertData(void *data,
                     unsigned int size,
                     unsigned int pos);
};

class Edge
{
public:
    // vert 0 is to the right of vert 1 if you're standing on
    // the 'top' of the face looking 'out' towards this edge

    // no, that's not the way you'd think of doing that, but if
    // you start with a CCW triangle vertex ordering, it makes
    // sense.
    
    unsigned int verts[2];
    unsigned int face;

    Edge(unsigned int v0,
         unsigned int v1,
         unsigned int f)
        {
            verts[0]=v0;
            verts[1]=v1;
            face=f;
        }
};


class Manifold
{
public:
    std::vector<Point>     m_Points;
    std::vector<Edge>      m_Edges;
    std::vector<Triangle>  m_Triangles;

    unsigned int addPoint(float x, float y, float z);
    unsigned int addEdge(unsigned int v0, unsigned int v1);
    unsigned int addTriangle(unsigned int v0,
                             unsigned int v1,
                             unsigned int v2);
    unsigned int addTriangle(Triangle &t);

    std::vector<IndexType>  edgeFromVertIndices(unsigned int v0,
                                                unsigned int v1);
    
    void dump();
    
    static unsigned int INVALID_INDEX;

    bool edgeCrossable(unsigned int edge);
};

#endif // MANIFOLD_H



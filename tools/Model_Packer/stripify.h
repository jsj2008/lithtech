// stripify.h
// (c) 2000 LithTech, inc.
// generate patches, and within the patches, strips.
//------------------------------------------------------------

#ifndef STRIPIFY_H
#define STRIPIFY_H

#include <vector>
#include <list>
#include <deque>
#include "ltinteger.h"
#include "manifold.h"

class PieceLOD;

class VertexInfo
{
 public:
    IndexType m_VertexIndex;
    IndexType m_TriangleIndex;
    uint8 m_IndOfVertInTri;
};

class TriangleStrip
{
 public:
    // a strip is an ordered collection of vert indices, but also
    // triangle indices, so that we can encode which UV and normal to
    // use from that vert.
    bool m_Clockwise;
    std::vector< VertexInfo > m_Verts;
};

class Patch
{
 public:
    std::vector<TriangleStrip> m_Strips;

    std::vector<IndexType> m_points;
    std::vector<IndexType> m_tris;
};

class ProgressDisplay
{
 public:
    void setPieceCount(int pieceCount){m_pieceCount=pieceCount;}
    void setCurrentPiece(int whichPiece){m_whichPiece=whichPiece;}

    void setLODCount(int lodCount){m_lodCount=lodCount;}
    void setCurrentLOD(int whichLOD){m_whichLOD=whichLOD;}

    void setTriangleCount(int triCount){m_triCount=triCount;}
    void setTrisUsed(int trisUsed){m_trisUsed=trisUsed;}

    void display();
 private:
    int m_pieceCount;
    int m_whichPiece;
    int m_lodCount;
    int m_whichLOD;
    int m_triCount;
    int m_trisUsed;
};


class Stripification
{
 public:
    Stripification(PieceLOD *lod, int numVerts);
    std::vector<Patch> m_Patches;

    void dump();
    static uint32 ms_costPerPatch;
    static uint32 ms_costPerStrip;
    static uint32 ms_costPerVert;
 private:
    void makeStrips(Patch& patch);
    void makeStrip(Patch& patch, std::list<IndexType> *triIndices);
    void makeStripCandidate(Patch& patch,
                            std::list<IndexType> *triIndices,
                            std::vector<IndexType> *usedTris,
                            TriangleStrip& strip,
                            unsigned int direction);
    uint32 memoryCost(Patch& p);
    void trimPatchForMemory(Patch& p, int memorySize);
    void removePatchElements(Patch& p);

    
    bool makePatch(Patch &patch);
    void subtractPoints(Patch& patch);

    void displayProgress(int pieceNum,
                         int pieceCount,
                         int trisUsed,
                         int trisAvail);
    
    std::list<IndexType> m_PointsInConsideration;
    std::list<IndexType> m_TrisInConsideration;
    int m_numVerts;
    PieceLOD *lod;
    Manifold m_Manifold;
};




#endif

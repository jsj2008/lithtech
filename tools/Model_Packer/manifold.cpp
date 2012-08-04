// manifold.cpp

#include <math.h>
#include <assert.h>
#include "manifold.h"
#include <stdio.h>

unsigned int Manifold::INVALID_INDEX=((unsigned int) -1);

float VertAuxData::normalTolerance=.01f;
float VertAuxData::colorTolerance=.001f;
float VertAuxData::textureTolerance=.01f;
bool VertAuxData::m_bCheckColor=true;

unsigned int Manifold::addPoint(float x, float y, float z)
{
    Point p(x,y,z);

    m_Points.push_back(p);
    return (m_Points.size()-1);
}

unsigned int Manifold::addEdge(unsigned int v0,
                               unsigned int v1)
{
    Edge e(v0,v1,INVALID_INDEX);

    m_Edges.push_back(e);

    int edgeIndex=m_Edges.size()-1;

/*	IndexPair p(v0,v1);
    
    EdgeLookup::iterator pr=edgeLookup.find(p);

    if (pr==edgeLookup.end()){
        std::vector<IndexType> v;
        v.push_back(edgeIndex);
        edgeLookup[p]=v;
    } else {
        (*pr).second.push_back(edgeIndex);
        }
*/

    m_Points[v0].edges.push_back(edgeIndex);
    m_Points[v1].edges.push_back(edgeIndex);
    
    return edgeIndex;    
}

unsigned int Manifold::addTriangle(Triangle &t)
{
    int triIndex=m_Triangles.size();

    for (int i=0;i<3;++i){
        t.edgeIndex[i]=addEdge(t.vertIndex[i],
                               t.vertIndex[(i+1)%3]);
        
        std::vector <IndexType> adjVect=
            edgeFromVertIndices(t.vertIndex[(i+1)%3],
                                t.vertIndex[i]);
        
        if (!adjVect.size()){
            t.adjTriIndex[i]=INVALID_INDEX;
            t.adjTriEdgeIndex[i]=INVALID_INDEX;
        } else {
            int a;
            for (a=0;a<(int)adjVect.size();++a){
                IndexType candidateAdjTriIndex=m_Edges[adjVect[a]].face;
                Triangle *candidateAdjTri=&m_Triangles[candidateAdjTriIndex];
                
                int k;
                for (k=0;k<3;++k){
                    if ((IndexType)(candidateAdjTri->edgeIndex[k])==adjVect[a]){
                        break;
                    }
                }
                
                if (k==3){
                    assert (0);
                    continue;
                }
                
                if (candidateAdjTri->adjTriIndex[k]!=INVALID_INDEX){
                    // that edge was already hooked up to somebody
                    // else's triangle so we don't get to use it. Keep
                    // on going on.
                    continue;
                }

                t.adjTriIndex[i]=candidateAdjTriIndex;
				t.adjTriEdgeIndex[i]=candidateAdjTri->edgeIndex[k];
                candidateAdjTri->adjTriIndex[k]=triIndex;
                candidateAdjTri->adjTriEdgeIndex[k]=t.edgeIndex[i];
                break;
            }
            if (a==(int)adjVect.size()){
                // there are edges there, but they're all already
                // spoken for. So we'll be out on our own.
                t.adjTriIndex[i]=INVALID_INDEX;
            }
        }   
    }
    m_Triangles.push_back(t);
    m_Edges[t.edgeIndex[0]].face=triIndex;
    m_Edges[t.edgeIndex[1]].face=triIndex;
    m_Edges[t.edgeIndex[2]].face=triIndex;


    return triIndex;
}

unsigned int Manifold::addTriangle(unsigned int v0,
                                   unsigned int v1,
                                   unsigned int v2)
{
    Triangle t;

    int triIndex=m_Triangles.size();

    t.vertIndex[0]=v0;
    t.vertIndex[1]=v1;
    t.vertIndex[2]=v2;

    t.edgeIndex[0]=addEdge(v0,v1);
    t.edgeIndex[1]=addEdge(v1,v2);
    t.edgeIndex[2]=addEdge(v2,v0);

    assert(0);
/*
    t.adjTriEdgeIndex[0]=edgeFromVertIndices(v1,v0);
    t.adjTriEdgeIndex[1]=edgeFromVertIndices(v2,v1);
    t.adjTriEdgeIndex[2]=edgeFromVertIndices(v0,v2);
*/
    for (int i=0;i<3;++i){
        IndexType adj=t.adjTriEdgeIndex[i];
        if (adj!=(IndexType)INVALID_INDEX){
            t.adjTriIndex[i]=m_Edges[adj].face;

            Triangle *adjTri=&m_Triangles[t.adjTriIndex[i]];

            for (int k=0;k<3;++k){
                if ((IndexType)(adjTri->edgeIndex[k])==adj){
                    break;    
                }
            }

            adjTri->adjTriIndex[k]=triIndex;
            adjTri->adjTriEdgeIndex[k]=t.edgeIndex[i];
        } else {
            t.adjTriIndex[i]=INVALID_INDEX;
        }        
    }

    m_Triangles.push_back(t);
    
    m_Edges[t.edgeIndex[0]].face=triIndex;
    m_Edges[t.edgeIndex[1]].face=triIndex;
    m_Edges[t.edgeIndex[2]].face=triIndex;

    return triIndex;
}

std::vector<IndexType> Manifold::edgeFromVertIndices(unsigned int v0,
                                                     unsigned int v1)
{
    Point* pPoint0=&(m_Points[v0]);
    std::vector<IndexType> edgeVect;
    
    for (int e=0;e<(int)pPoint0->edges.size();++e){
        IndexType edgeIndex=pPoint0->edges[e];
        
        Edge* pEdge=&(m_Edges[edgeIndex]);
        
        assert((pEdge->verts[0]==v0)||
               (pEdge->verts[1]==v0));

        // this gets the oriented edge
        if (pEdge->verts[1]==v1){
            edgeVect.push_back(edgeIndex);
        }
    }

    return edgeVect;
}

unsigned int Triangle::edgePosFromVertIndices(unsigned int v0,
                                              unsigned int v1)
{
    if (v0==vertIndex[0]){
        if (v1==vertIndex[1]){
            return 0;            
        } else {
            return 2;
        }
    } else if (v0==vertIndex[1]){
        if (v1==vertIndex[0]){
            return 0;
        } else {
            return 1;            
        }
    } else {
        if (v1==vertIndex[0]){
            return 2;
        } else {
            return 1;            
        }
    }
}

unsigned int Triangle::edgeFromVertIndices(unsigned int v0,
                                           unsigned int v1)
{
    return edgeIndex[edgePosFromVertIndices(v0,v1)];
}

unsigned int Triangle::vertPos(unsigned int v)
{
    if (v==vertIndex[0]){
        return 0;
    } else if (v==vertIndex[1]){
        return 1;
    }
    return 2;
}

unsigned int Triangle::edgePos(unsigned int e)
{
    if (e==edgeIndex[0]){
        return 0;
    } else if (e==edgeIndex[1]){
        return 1;
    }
    return 2;
}


unsigned int Triangle::oppositeVertex(unsigned int v0,
                                      unsigned int v1)
{
    unsigned int edge=edgePosFromVertIndices(v0,v1);

    return vertIndex[(edge+2)%3];
}

bool Manifold::edgeCrossable(unsigned int edgeIndex)
{
    Edge* edge=&(m_Edges[edgeIndex]);
    unsigned int thisFaceIndex=edge->face;
    Triangle* thisFace=&(m_Triangles[thisFaceIndex]);

    int edgePos=thisFace->edgePosFromVertIndices(
        edge->verts[0],
        edge->verts[1]);
    
    unsigned int otherFaceIndex=
        thisFace->adjTriIndex[edgePos];

    if (otherFaceIndex==Manifold::INVALID_INDEX){
        return false;
    }
    
    Triangle* otherFace=&(m_Triangles[otherFaceIndex]);

    int otherEdgePos=otherFace->edgePosFromVertIndices(
        edge->verts[0],
        edge->verts[1]);

    // OK, so now we have the two faces, the two edge locations
    // (0,1,2) of our edges, and it's just a matter of checking the
    // data in the vertData fields.

    VertAuxData* thisVertData=&(thisFace->vertData[edgePos]);
    VertAuxData* otherVertData=&(otherFace->vertData[(otherEdgePos+1)%3]);

    if (!(*thisVertData==*otherVertData)){
        // the data is different
        return false;
    }

    thisVertData=&(thisFace->vertData[(edgePos+1)%3]);
    otherVertData=&(otherFace->vertData[otherEdgePos]);
    
    if (!(*thisVertData==*otherVertData)){
        // the data is different
        return false;
    }

    return true;
}

bool VertAuxData::operator==(VertAuxData& d)
{
    int i;    
    for (i=0;i<3;++i){
        if (fabs(d.normal[i]-normal[i])>normalTolerance){
            return false;
        }
    }
    
    if (m_bCheckColor){
        for (i=0;i<4;++i){
            if (fabs((float)(d.rgba[i]-rgba[i]))>colorTolerance){
                return false;
            }
        }
    }
    
    for (i=0;i<2;++i){
        if (fabs(d.uv[i]-uv[i])>textureTolerance){
            return false;
        }
    }
    return true;
}

void Manifold::dump()
{
    printf("Num Points: %d\n",m_Points.size());

    for (int i=0;i<(int)m_Points.size();++i){
        printf("%4.2f %4.2f %4.2f\n",
               m_Points[i].x,
               m_Points[i].y,
               m_Points[i].z);
    }

    printf("Num Tris: %d\n",m_Triangles.size());

    for (i=0;i<(int)m_Triangles.size();++i){
        printf("Tri %d -- ",i);        
        printf("%.2d %.2d %.2d\n",
               m_Triangles[i].vertIndex[0],
               m_Triangles[i].vertIndex[1],
               m_Triangles[i].vertIndex[2]);
        printf("adj: %2d %2d %2d\n\n",
               m_Triangles[i].adjTriIndex[0],
               m_Triangles[i].adjTriIndex[1],
               m_Triangles[i].adjTriIndex[2]);        
    }          
}

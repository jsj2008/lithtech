// Stripify.cpp
// (c) 2000 LithTech, inc.
// generate patches, and within the patches, strips.
//------------------------------------------------------------

#pragma warning(disable:4786)

#include <vector>
#include <algorithm>
#include <iterator>
#include <model.h>
#include "stripify.h"

//uint32 Stripification::ms_costPerPatch=2;
//uint32 Stripification::ms_costPerStrip=1;
//uint32 Stripification::ms_costPerVert=3;
uint32 Stripification::ms_costPerPatch=0;
uint32 Stripification::ms_costPerStrip=3;
uint32 Stripification::ms_costPerVert=3;

extern ProgressDisplay gProgress;

extern bool g_bProgress;

class LonelyPred
{
public:
    static Patch* m_patch;
    static Manifold* m_manifold;

    static bool moreLonely(IndexType a, IndexType b){
        Triangle *triA=&(m_manifold->m_Triangles[a]);
        Triangle *triB=&(m_manifold->m_Triangles[b]);

        int aNeighbors=0;
        int bNeighbors=0;

        for (int i=0;i<3;++i){
            IndexType aAdj=triA->adjTriIndex[i];
            IndexType bAdj=triB->adjTriIndex[i];

            if (std::find(m_patch->m_tris.begin(),
                          m_patch->m_tris.end(),
                          aAdj)!=m_patch->m_tris.end()){
                ++aNeighbors;
            }
            if (std::find(m_patch->m_tris.begin(),
                          m_patch->m_tris.end(),
                          bAdj)!=m_patch->m_tris.end()){
                ++bNeighbors;
            }
        }
                
        return aNeighbors<bNeighbors;
    }
};

Patch* LonelyPred::m_patch=NULL;
Manifold* LonelyPred::m_manifold=NULL;


bool Stripification::makePatch(Patch &patch)
{
    patch.m_Strips.clear();
    patch.m_points.clear();
    patch.m_tris.clear();
    
    if (m_PointsInConsideration.empty()){
        return false;
    }
    
    // the pointStack is the waiting list for points in
    // consideration. FIFO.
    
    std::deque<IndexType> pointStack;
    IndexType point;
    while ((int)patch.m_points.size()<m_numVerts){
        if (pointStack.empty()){
            std::list<IndexType>::iterator it;
            IndexType seed=(IndexType)-1;
            for (it=m_PointsInConsideration.begin();
                 it!=m_PointsInConsideration.end();
                 ++it){
                if (std::find(patch.m_points.begin(),
                              patch.m_points.end(),
                              *it)
                    ==patch.m_points.end()){
                    seed=*it;
                    break;
                }
            }
            if (it==m_PointsInConsideration.end()){
                break;
            }
            
            point=*it;
            m_Manifold.m_Points[point].distanceFromPatchCenter=0;
            pointStack.push_back(point);
        }
        
        IndexType pt=pointStack[0];
        pointStack.pop_front();

        Point *p=&(m_Manifold.m_Points[pt]);
        for (int i=0;i<(int)p->edges.size();++i){
            IndexType segIndex=p->edges[i];
            Edge* seg=&(m_Manifold.m_Edges[segIndex]);

            if (std::find(m_TrisInConsideration.begin(),
                          m_TrisInConsideration.end(),
                          seg->face)==m_TrisInConsideration.end()){
                continue;
            }

            IndexType newPointIndex=seg->verts[(seg->verts[0]==(uint)pt)?1:0];
            Point* newPoint=&(m_Manifold.m_Points[newPointIndex]);
            
            if ((std::find(patch.m_points.begin(),
                           patch.m_points.end(),
                           newPointIndex)==patch.m_points.end())&&
                (std::find(pointStack.begin(),
                           pointStack.end(),
                           newPointIndex)==pointStack.end())&&
                (std::find(m_PointsInConsideration.begin(),
                           m_PointsInConsideration.end(),
                           newPointIndex)!=m_PointsInConsideration.end())){
                pointStack.push_back(newPointIndex);
                newPoint->distanceFromPatchCenter=
                    p->distanceFromPatchCenter+1;
            }
        }
        patch.m_points.push_back(pt);
    }
    return true;
}

void Stripification::subtractPoints(Patch& patch)
{
    int i;
    for (i=0; i<(int)patch.m_points.size(); ++i){
        IndexType pointIndex=patch.m_points[i];

        Point *pt=&(m_Manifold.m_Points[pointIndex]);

        for (int e=0; e<(int)pt->edges.size();++e){
            Edge *edge=&(m_Manifold.m_Edges[pt->edges[e]]);
            Triangle *tri=&(m_Manifold.m_Triangles[edge->face]);
            
            int j;
            for (j=0;j<3;++j){
                if (std::find(patch.m_points.begin(),
                              patch.m_points.end(),
                              tri->vertIndex[j])==patch.m_points.end()){
                    break;
                }    
            }

            if (j<3){
                continue;
            }

            // Now we know that this triangle is completely contained
            // in the lump, so we can remove it from future
            // consideration.

            if ((std::find(patch.m_tris.begin(),
                           patch.m_tris.end(),
                           edge->face)!=patch.m_tris.end())||
                (std::find(m_TrisInConsideration.begin(),
                           m_TrisInConsideration.end(),
                           edge->face)==m_TrisInConsideration.end())){

                // either the triangle is in our patch, or it's
                // already been removed.
                continue;
            }
            
            patch.m_tris.push_back(edge->face);
       }
    }

/*    for (i=0;i<patch.m_points.size();++i){
        bool foundAnyUnusedTris=false;
        Point *p=&(m_Manifold.m_Points[patch.m_points[i]]);
        for (int j=0;j<p->edges.size();++j){
            IndexType edgeIndex=p->edges[j];
            if (std::find(m_TrisInConsideration.begin(),
                          m_TrisInConsideration.end(),
                          m_Manifold.m_Edges[edgeIndex].face)!=
                m_TrisInConsideration.end()){
                foundAnyUnusedTris=true;
                break;
            }
        }
        if (!foundAnyUnusedTris){
            m_PointsInConsideration.remove(patch.m_points[i]);
        }
    }
*/
    LonelyPred::m_patch=&patch;
    LonelyPred::m_manifold=&m_Manifold;    
    
    std::sort(patch.m_tris.begin(),
              patch.m_tris.end(),
              LonelyPred::moreLonely);    
}

/*
bool Stripification::makePatchPass()
{
    Patch patch;
    makePatch(patch);
    subtractPoints(patch);
    
    if (patch.m_tris.empty()){
        // check to see if there are any unused points
        return (!m_PointsInConsideration.empty());
    }

    // we made a useful patch. Push it and go on.
    m_Patches.push_back(patch);
    return true;
}
*/

void Stripification::makeStrips(Patch &p)
{
    std::list<IndexType> triIndices;

    for (std::vector<IndexType>::iterator triIter=p.m_tris.begin();
         triIter!=p.m_tris.end();
         ++triIter){
        
        triIndices.push_back(*triIter);
    }
    
    while (!triIndices.empty()){
        makeStrip(p, &triIndices);
    }
}

void Stripification::makeStripCandidate(Patch& patch,
                                        std::list<IndexType> *triIndices,
                                        std::vector<IndexType> *usedTris,
                                        TriangleStrip& strip,
                                        unsigned int direction)
{   
	unsigned int triCount;
    IndexType seedTriIndex= *(triIndices->begin());
    Triangle *seedTri=&(m_Manifold.m_Triangles[seedTriIndex]);

    triIndices->pop_front();
    usedTris->push_back(seedTriIndex);
    int i;
    
    // find the neighboring triangle in our specified direction
    IndexType neighborTriIndex=Manifold::INVALID_INDEX;
    int initialDirection=direction>>1;
    
    if (m_Manifold.edgeCrossable(seedTri->edgeIndex[initialDirection])){
        IndexType neighborIndex=seedTri->adjTriIndex[initialDirection];
        if (std::find(triIndices->begin(),
                      triIndices->end(),
                      neighborIndex)!=triIndices->end()){
            neighborTriIndex=neighborIndex;
        }
    }
    
	triCount=1;
    
    if (neighborTriIndex==(IndexType)Manifold::INVALID_INDEX){
        // we can't find a neighbor, which means we've got an isolated
        // triangle.
        
        strip.m_Clockwise=false;
        
        for (int i=0;i<3;++i){
            VertexInfo vi;
            vi.m_VertexIndex=seedTri->vertIndex[i];
            vi.m_TriangleIndex=seedTriIndex;
            vi.m_IndOfVertInTri=i;
            strip.m_Verts.push_back(vi);            
        }
        return;                    
    }
    
    triIndices->remove(neighborTriIndex);
    usedTris->push_back(neighborTriIndex);
    
    int whichNeighborOfSeed=initialDirection;
    
    Triangle *neighborTri=&(m_Manifold.m_Triangles[neighborTriIndex]);
    
	++triCount;
    
    // pick a neighbor of THAT triangle (if possible)
    
    int secondaryDirection=(direction%2+1+
                            seedTri->adjTriEdgeIndex[initialDirection])%3;
    
    if ((neighborTri->adjTriIndex[secondaryDirection]==(uint)seedTriIndex)||
        ((std::find(triIndices->begin(),
                    triIndices->end(),
                    neighborTri->adjTriIndex[secondaryDirection])==
          triIndices->end()))||
        (!m_Manifold.edgeCrossable(
            neighborTri->edgeIndex[secondaryDirection]))){
        
        // There are two cases here.
        //
        // A) we've got back to the seed triangle somehow. This could
        // be either a case of bad pointers or a triangle which is
        // adjacent on two sides to another triangle.
        // 
        // B) we can't find a neighbor - which means we've got two
        // tris. That's still OK.

        strip.m_Clockwise=false;
        
        VertexInfo vi;
        
        for (int i=0;i<3;++i){
            vi.m_VertexIndex=seedTri->vertIndex[(i+initialDirection+2)%3];
            vi.m_TriangleIndex=seedTriIndex;
            vi.m_IndOfVertInTri=(i+initialDirection+2)%3;
            strip.m_Verts.push_back(vi);            
        }
        
        vi.m_IndOfVertInTri=(seedTri->adjTriEdgeIndex[initialDirection]+2)%3;
        vi.m_VertexIndex=neighborTri->vertIndex[vi.m_IndOfVertInTri];
        vi.m_TriangleIndex=neighborTriIndex;
        strip.m_Verts.push_back(vi);        
        
        return;
    }
    
    std::list<VertexInfo> outVerts;
    
    IndexType thisIndex=neighborTri->adjTriIndex[secondaryDirection];
    Triangle *thisTri=&(m_Manifold.m_Triangles[thisIndex]);
    IndexType thisIncoming=thisTri->edgePos(
        neighborTri->adjTriEdgeIndex[secondaryDirection]);
    
    // the interesting(?) thing about strips is that if you look at
    // the deltas between the indices on each triangle, we flip
    // between +1 and +2; this means that we might come into a
    // triangle across edge 2 and go out 1 (that's a +2 delta), but
    // then on the next triangle, maybe we come in across edge 0, but
    // we go out on edge 1 (+1 delta). This means that once we know
    // one delta, we can pick out neighbors really quickly.

    int delta=2;

    int whichNeighborOfNeighbor=secondaryDirection;
    
    IndexType testSeed=(whichNeighborOfNeighbor+2)%3;
    
    if (neighborTri->adjTriIndex[testSeed]!=(uint)seedTriIndex){
        delta=1;
    }
    
	int firstLegCount=0;
	int initialDelta=delta;

    for (i=0;i<3;++i){
        VertexInfo vi;
        int pos;
        if (delta==1){
            pos=(whichNeighborOfSeed+2+i)%3;
        } else {
            pos=(whichNeighborOfSeed+2-i)%3;
        }
        
        vi.m_VertexIndex=seedTri->vertIndex[pos];
        vi.m_TriangleIndex=seedTriIndex;
        vi.m_IndOfVertInTri=pos;
        outVerts.push_back(vi);
    }
    VertexInfo vi;
    vi.m_VertexIndex=neighborTri->vertIndex[
        (seedTri->adjTriEdgeIndex[whichNeighborOfSeed]+2)%3];
    vi.m_TriangleIndex=seedTri->adjTriIndex[whichNeighborOfSeed];
    vi.m_IndOfVertInTri=(seedTri->adjTriEdgeIndex[
        whichNeighborOfSeed]+2)%3;
    outVerts.push_back(vi);
    
    // now we have the pump primed - let it run
    
    while (true) {
		firstLegCount++;
        triIndices->remove(thisIndex);
        usedTris->push_back(thisIndex);
        VertexInfo vi;
        vi.m_VertexIndex=thisTri->vertIndex[(thisIncoming+2)%3];
        vi.m_TriangleIndex=thisIndex;
        vi.m_IndOfVertInTri=(thisIncoming+2)%3;
        outVerts.push_back(vi);
        
        IndexType thisOutgoing=(thisIncoming+delta)%3;
        IndexType runCandidateIndex=thisTri->adjTriIndex[thisOutgoing];
        
		if (!m_Manifold.edgeCrossable(thisTri->edgeIndex[thisOutgoing])){
			// uh, oh - much as we'd like to move on, that next triangle's across a seam.
			break;
		}
        if (std::find(triIndices->begin(),
                      triIndices->end(),
                      runCandidateIndex)==triIndices->end()){
            // we've run to the end of the strip.
            break;
        }
        
        delta=((delta==2)?1:2);
        
        Triangle *nextTri=&(m_Manifold.
                            m_Triangles[runCandidateIndex]);
        thisIndex=runCandidateIndex;
        thisIncoming=nextTri->edgePos(thisTri->adjTriEdgeIndex[thisOutgoing]);
        thisTri=nextTri;
		++triCount;
    }
    
// turn the chain around
    
    outVerts.reverse();

    thisTri=seedTri;
    thisIndex=seedTriIndex;
    thisIncoming=whichNeighborOfSeed;

    delta=1;
    
    if (neighborTri->adjTriIndex[testSeed]!=(uint)seedTriIndex){
        delta=2;
    }

    IndexType thisOutgoing=(thisIncoming+delta)%3;
    IndexType runCandidateIndex=thisTri->adjTriIndex[thisOutgoing];
	IndexType runCandidateEdgeIndex=thisTri->edgeIndex[thisOutgoing];

    if ((std::find(triIndices->begin(),
                  triIndices->end(),	
                  runCandidateIndex)!=triIndices->end())
		&&(m_Manifold.edgeCrossable(runCandidateEdgeIndex))){

        delta=((delta==2)?1:2);
        
        Triangle *nextTri=&(m_Manifold.m_Triangles[runCandidateIndex]);
        thisIndex=runCandidateIndex;
        thisIncoming=nextTri->edgePos(thisTri->adjTriEdgeIndex[thisOutgoing]);
        thisTri=nextTri;
        
        while (true) {
            triIndices->remove(thisIndex);
            usedTris->push_back(thisIndex);
            VertexInfo vi;
            vi.m_VertexIndex=thisTri->vertIndex[(thisIncoming+2)%3];
            vi.m_TriangleIndex=thisIndex;
            vi.m_IndOfVertInTri=(thisIncoming+2)%3;
            outVerts.push_back(vi);
            
            IndexType thisOutgoing=(thisIncoming+delta)%3;
            IndexType runCandidateIndex=thisTri->adjTriIndex[thisOutgoing];

			if (!m_Manifold.edgeCrossable(thisTri->edgeIndex[thisOutgoing])){
				// uh, oh - much as we'd like to move on, that next triangle's across a seam.
				break;
			}

            if (std::find(triIndices->begin(),
                          triIndices->end(),
                          runCandidateIndex)==triIndices->end()){
                // we've run to the end of the strip.
                break;
            }

            delta=((delta==2)?1:2);
            
            Triangle *nextTri=&(m_Manifold.m_Triangles[runCandidateIndex]);
            thisIndex=runCandidateIndex;
            thisIncoming=nextTri->edgePos(thisTri->adjTriEdgeIndex[thisOutgoing]);
            thisTri=nextTri;
			++triCount;
        }
    }
    
	bool winding=((firstLegCount%2)==0);
	winding ^= (initialDelta==1);

    strip.m_Clockwise=winding;

    for (std::list<VertexInfo>::iterator listIter=outVerts.begin();
         listIter!=outVerts.end();
         ++listIter){
        strip.m_Verts.push_back(*listIter);
    }
    return;    
}


void Stripification::makeStrip(Patch& patch,
                               std::list<IndexType> *triIndices)
{
    TriangleStrip strips[6];
    std::list<IndexType> candidateIndices[6];
    std::vector<IndexType> usedTris[6];

    int i;
    
    for (i=0;i<6;++i){
        std::copy(triIndices->begin(),
                  triIndices->end(),
                  std::inserter(candidateIndices[i],
                                candidateIndices[i].begin()));
        makeStripCandidate(patch,
                           &candidateIndices[i],
                           &usedTris[i],
                           strips[i],
                           i);
    }

    int bestCand=-1;
    int bestLen=-1;
    
    for (i=0;i<6;++i){
		int len=strips[i].m_Verts.size();
        if (len>bestLen){
            bestLen=strips[i].m_Verts.size();
            bestCand=i;
        }
    }

    patch.m_Strips.push_back(strips[bestCand]);

    for (i=0;i<bestLen;++i){
        triIndices->remove(usedTris[bestCand][i]);
    }    
}

uint32 Stripification::memoryCost(Patch& p)
{
    int numVerts=0;
    for (int i=0;i<(int)p.m_Strips.size();++i){
        numVerts+=p.m_Strips[i].m_Verts.size();
    }
    
    return (ms_costPerPatch+
            ms_costPerStrip*p.m_Strips.size()+
            ms_costPerVert*numVerts);
}

void Stripification::trimPatchForMemory(Patch& p, int memorySize)
{
    while (memoryCost(p)>(uint32)memorySize){
        // go around to all strip ends, removing the triangle that is
        // furthest from the patch center.

        int maxDist=-1;
        bool trimFromFront=false;
        int bestStrip=-1;
        for (int i=0;i<(int)p.m_Strips.size();++i){
            VertexInfo *vi=p.m_Strips[i].m_Verts.begin();
			int dist=m_Manifold.m_Points[vi->m_VertexIndex].distanceFromPatchCenter;
            if (dist>maxDist){
                maxDist=dist;
                trimFromFront=true;
                bestStrip=i;
            }
            vi=p.m_Strips[i].m_Verts.end()-1;
			dist=m_Manifold.m_Points[vi->m_VertexIndex].distanceFromPatchCenter;
            if (dist>maxDist){
                maxDist=dist;
                trimFromFront=true;
                bestStrip=i;
            }
        }

        if (p.m_Strips[bestStrip].m_Verts.size()==3){
            // we're removing the entire strip
            p.m_Strips.erase(p.m_Strips.begin()+bestStrip);
        } else {
            if (trimFromFront){
                p.m_Strips[bestStrip].m_Verts.erase(
                    p.m_Strips[bestStrip].m_Verts.begin());
                p.m_Strips[bestStrip].m_Clockwise=
                    (!p.m_Strips[bestStrip].m_Clockwise);
            } else {
                p.m_Strips[bestStrip].m_Verts.erase(
                    p.m_Strips[bestStrip].m_Verts.end()-1);
            }
        }
    }
}

void Stripification::removePatchElements(Patch& p)
{
    std::list<IndexType>::iterator it;

#if 0
    printf("Tris In Consideration ");
    
	for (it=m_TrisInConsideration.begin();
         it!=m_TrisInConsideration.end();
         ++it){
		printf("%d ",*it);
	}
    printf("\n");
#endif

    std::vector<IndexType> removeList;
    
    for (int s=0;s<(int)p.m_Strips.size();++s){
        for (int i=0;i<(int)p.m_Strips[s].m_Verts.size();++i){
			IndexType triIndex=p.m_Strips[s].m_Verts[i].m_TriangleIndex;

			Triangle t=m_Manifold.m_Triangles[triIndex];

            // because of the way we trim the strips, it's possible
            // that the triangle that caused us to insert this vert no
            // longer is part of our strip. We shouldn't remove that
            // triangle from consideration in that case.

			bool allIn=true;            
			for (int triVert=0;triVert<3;++triVert){
                int stripIndex;
                for (stripIndex=0;
                     stripIndex<(int)p.m_Strips[s].m_Verts.size();
                     ++stripIndex){
                    if (p.m_Strips[s].m_Verts[stripIndex].m_VertexIndex
                        == (IndexType)t.vertIndex[triVert]){
                        break;
                    }
                }
                if (stripIndex ==
                    (int)p.m_Strips[s].m_Verts.size()){

					printf("failed to find %d\n",t.vertIndex[triVert]);
                    allIn=false;
                    break;
                }
            }

            if (allIn){
                removeList.push_back(triIndex);
			}
        }
    }

    std::sort(removeList.begin(), removeList.end());
    std::vector<IndexType>::iterator newEnd=
        std::unique(removeList.begin(), removeList.end());

    removeList.erase(newEnd, removeList.end());
    
    int i;

#if 0
    printf("removeList: ");
    for (i=0;i<removeList.size();++i){
        printf("%d ",removeList[i]);
    }
    printf("\n");
#endif
    
    for (i=0;i<(int)removeList.size();++i){
        IndexType index=removeList[i];

        it=std::find(m_TrisInConsideration.begin(),
                     m_TrisInConsideration.end(),
                     index);
        assert(it!=m_TrisInConsideration.end());
        m_TrisInConsideration.remove(index);
    }

    for (i=0;i<(int)p.m_points.size();++i){
        bool foundAnyUnusedTris=false;
        Point *pt=&(m_Manifold.m_Points[p.m_points[i]]);
        for (int j=0;j<(int)pt->edges.size();++j){
            IndexType edgeIndex=pt->edges[j];
            if (std::find(m_TrisInConsideration.begin(),
                          m_TrisInConsideration.end(),
                          m_Manifold.m_Edges[edgeIndex].face)!=
                m_TrisInConsideration.end()){
                foundAnyUnusedTris=true;
                break;
            }
        }
        if (!foundAnyUnusedTris){
            m_PointsInConsideration.remove(p.m_points[i]);
        }
    }
}

void ProgressDisplay::display()
{
    float fractionalLOD=m_trisUsed/(float)m_triCount;
    float fractionalPiece=(m_whichLOD+fractionalLOD)/(float)m_lodCount;
    float progress=(m_whichPiece+fractionalPiece)/(float)m_pieceCount;
    
    printf("\rProgress: %5.1f%%",progress*100.0f);
}    

Stripification::Stripification(PieceLOD *lod,
                               int memorySize)
{
    m_numVerts=(memorySize-(ms_costPerPatch+ms_costPerStrip))/
        ms_costPerVert;
    
    for (int vertIndex=0;vertIndex<(int)lod->NumVerts();++vertIndex){
        ModelVert v=lod->m_Verts[vertIndex];
        LTVector vec=v.m_Vec;
        m_Manifold.addPoint(vec[0], vec[1], vec[2]);
    }
    
	int triIndex;
    for (triIndex=0;triIndex<(int)lod->m_Tris;++triIndex){
        ModelTri *t=&lod->m_Tris[triIndex];
        Triangle tri;
        for (int i=0;i<3;++i){
            tri.vertIndex[i]=t->m_Indices[i];
            VertAuxData *vi=&(tri.vertData[i]);
            for (int j=0;j<3;++j){
                vi->normal[j]=t->m_Normals[i][j];
            }
            vi->rgba[0]=(uint8)(255*t->m_Colors[i][0]);
            vi->rgba[1]=(uint8)(255*t->m_Colors[i][1]);
            vi->rgba[2]=(uint8)(255*t->m_Colors[i][2]);
            vi->rgba[3]=255;

            vi->uv[0]=t->m_UVs[i].tu;
            vi->uv[1]=t->m_UVs[i].tv;
        }
        
        m_Manifold.addTriangle(tri);
    }


#if 0
    for (triIndex=0;triIndex<lod->m_Tris;++triIndex){
        Triangle *t=&(m_Manifold.m_Triangles[triIndex]);

		printf("T%4d  %4d %4d %4d ",
			triIndex,
			t->vertIndex[0],
			t->vertIndex[1],
			t->vertIndex[2]);
		printf("| %4d %4d %4d ",
			t->edgeIndex[0],
			t->edgeIndex[1],
			t->edgeIndex[2]);
		printf("| %4d %4d %4d ",
			t->adjTriIndex[0],
			t->adjTriIndex[1],
			t->adjTriIndex[2]);
		printf("| %4d %4d %4d\n",
			t->adjTriEdgeIndex[0],
			t->adjTriEdgeIndex[1],
			t->adjTriEdgeIndex[2]);
	}
#endif
    
    for (IndexType i=0;i<(IndexType)m_Manifold.m_Points.size();++i){
        m_PointsInConsideration.push_back(i);
    }

    for (IndexType t=0;t<(IndexType)m_Manifold.m_Triangles.size();++t){
        m_TrisInConsideration.push_back(t);
    }
    
    gProgress.setTriangleCount(m_Manifold.m_Triangles.size());
    
    Patch p;
    while (makePatch(p)){
        subtractPoints(p);

        makeStrips(p);
        trimPatchForMemory(p,memorySize);

#if 0
        printf("%d %d %d\n",
               p.m_Strips.size(),
               p.m_points.size(),
               p.m_tris.size());
#endif
        removePatchElements(p);
        m_Patches.push_back(p);
#if 0
        printf("made patch %d\n",m_Patches.size());
#endif

        gProgress.setTrisUsed(m_Manifold.m_Triangles.size()-
                              m_TrisInConsideration.size());

        if (g_bProgress){
            gProgress.display();
        }
    }
}

void Stripification::dump()
{
    printf("Dumping Stripification\n");
    printf("--------------------\n");
    
    for (int patch=0;patch<(int)m_Patches.size();++patch){
        for (int strip=0;strip<(int)m_Patches[patch].m_Strips.size();++strip){
            TriangleStrip* s=&(m_Patches[patch].m_Strips[strip]);
            if (s->m_Clockwise){
                printf("Clockwise\n");
            } else {
                printf("CounterClockwise\n");
            }

            for (int v=0;v<(int)s->m_Verts.size();++v){
                printf("%4d ",s->m_Verts[v]);
            }
            printf("\n");
        }
    }
}


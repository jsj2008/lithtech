#include "bdefs.h"
#include "world_particle_blocker_data.h"


//------------------------
//--- CParticleBlocker ---
//------------------------

class CParticleBlocker
{
public:
	CParticleBlocker();
	~CParticleBlocker();

	bool Load( ILTStream* pStream );

	LTVector m_MinBounds;
	LTVector m_MaxBounds;

	uint32 m_NumVerts;
	LTPlane m_Plane;
	LTVector* m_Verts;
	LTPlane* m_EdgeXZPlanes;

private:
	void InitializeXZPlanes();
};


CParticleBlocker::CParticleBlocker()
{
	m_Verts = NULL;
	m_EdgeXZPlanes = NULL;
}


CParticleBlocker::~CParticleBlocker()
{
	delete [] m_Verts;
	delete [] m_EdgeXZPlanes;
}


bool CParticleBlocker::Load( ILTStream* pStream )
{
	// load the blocker plane
	*pStream >> m_Plane.m_Normal.x;
	*pStream >> m_Plane.m_Normal.y;
	*pStream >> m_Plane.m_Normal.z;
	*pStream >> m_Plane.m_Dist;
	m_Plane.m_Dist *= -1.0f;

	// get the number of verts
	*pStream >> m_NumVerts;

	LT_MEM_TRACK_ALLOC(m_Verts = new LTVector[m_NumVerts],LT_MEM_TYPE_WORLD);
	LT_MEM_TRACK_ALLOC(m_EdgeXZPlanes = new LTPlane[m_NumVerts],LT_MEM_TYPE_WORLD);

	// load the blocker verts
	for( uint32 i = 0; i < m_NumVerts; i++ )
	{
		*pStream >> m_Verts[i].x;
		*pStream >> m_Verts[i].y;
		*pStream >> m_Verts[i].z;

		// build the bounds
		if( i == 0 )
		{
			m_MinBounds = m_MaxBounds = m_Verts[0];
		}
		else
		{
			if( m_Verts[i].x < m_MinBounds.x )
				m_MinBounds.x = m_Verts[i].x;
			else if( m_Verts[i].x > m_MaxBounds.x )
				m_MaxBounds.x = m_Verts[i].x;
			if( m_Verts[i].y < m_MinBounds.y )
				m_MinBounds.y = m_Verts[i].y;
			else if( m_Verts[i].y > m_MaxBounds.y )
				m_MaxBounds.y = m_Verts[i].y;
			if( m_Verts[i].z < m_MinBounds.z )
				m_MinBounds.z = m_Verts[i].z;
			else if( m_Verts[i].z > m_MaxBounds.z )
				m_MaxBounds.z = m_Verts[i].z;
		}
	}

	// initialize the xz blocker planes
	InitializeXZPlanes();

	return true;
}


void CParticleBlocker::InitializeXZPlanes( void )
{
	// See GGems IV chap I.4
	ASSERT( m_NumVerts >= 3 );

	bool flipEdge = (m_Verts[0].x - m_Verts[1].x) * (m_Verts[1].z - m_Verts[2].z) > (m_Verts[0].z - m_Verts[1].z) * (m_Verts[1].x - m_Verts[2].x);

	uint32 p2 = 0;
	for( uint32 p1 = m_NumVerts-1; p2 < m_NumVerts; p1 = p2, p2++ )
	{
		m_EdgeXZPlanes[p2].m_Normal.x = m_Verts[p1].z - m_Verts[p2].z;
		m_EdgeXZPlanes[p2].m_Normal.y = 0.0f;
		m_EdgeXZPlanes[p2].m_Normal.z = m_Verts[p2].x - m_Verts[p1].x;

		m_EdgeXZPlanes[p2].m_Dist = m_EdgeXZPlanes[p2].m_Normal.x * m_Verts[p1].x + m_EdgeXZPlanes[p2].m_Normal.z * m_Verts[p1].z;

		if( flipEdge )
		{
			m_EdgeXZPlanes[p2].m_Normal.x *= -1.0f;
			m_EdgeXZPlanes[p2].m_Normal.z *= -1.0f;
			m_EdgeXZPlanes[p2].m_Dist *= -1.0f;
		}
	}

	// randomize the edges for speed improvement when testing point in polygon
	LTPlane* rndPlane = m_EdgeXZPlanes;
	for( uint32 i = 0; i < m_NumVerts; i++ )
	{
		uint32 swapWith = rand() % m_NumVerts;

		LTPlane tmpPlane = *rndPlane;
		*rndPlane = m_EdgeXZPlanes[swapWith];
		m_EdgeXZPlanes[swapWith] = tmpPlane;
	}
}



//---------------------------------
//--- CWorldParticleBlockerData ---
//---------------------------------

class CWorldParticleBlockerData : public IWorldParticleBlockerData
{
public:
    declare_interface(CWorldParticleBlockerData);

	CWorldParticleBlockerData();
	virtual ~CWorldParticleBlockerData();

	virtual void Term();
	
    virtual ELoadWorldStatus Load( ILTStream *pStream );

	virtual bool GetBlockersInAABB( const LTVector& pos, const LTVector& dims, std::vector<uint32>& indices );
	virtual bool GetBlockerEdgesXZ( uint32 index, LTPlane& blockerPlane, uint32& numEdges, LTPlane*& edgePlanes );

private:
	std::vector<CParticleBlocker*> m_Blockers;
};

define_interface(CWorldParticleBlockerData, IWorldParticleBlockerData);



CWorldParticleBlockerData::CWorldParticleBlockerData()
{
}


CWorldParticleBlockerData::~CWorldParticleBlockerData()
{
	Term();
}


void CWorldParticleBlockerData::Term()
{
	for( std::vector<CParticleBlocker*>::iterator it = m_Blockers.begin(); it != m_Blockers.end(); it++ )
	{
		delete *it;
	}

	std::vector<CParticleBlocker*>().swap( m_Blockers );
}


ELoadWorldStatus CWorldParticleBlockerData::Load( ILTStream *pStream )
{
	// kill any left over data
	Term();

	// read in the number of blocker polys
	uint32 numPolys;
	*pStream >> numPolys;

	// load the blockers
	LT_MEM_TRACK_ALLOC(m_Blockers.reserve( numPolys ), LT_MEM_TYPE_MISC);

	for( uint32 i = 0; i < numPolys; i++ )
	{
		CParticleBlocker* curBlocker;
		LT_MEM_TRACK_ALLOC(curBlocker = new CParticleBlocker,LT_MEM_TYPE_WORLD);
		curBlocker->Load( pStream );
		m_Blockers.push_back( curBlocker );
	}

	// read in the dummy field
	uint32 dummy;
	*pStream >> dummy;
	ASSERT( dummy == 0 );

	return LoadWorld_Ok;
}


bool CWorldParticleBlockerData::GetBlockersInAABB( const LTVector& pos, const LTVector& dims, std::vector<uint32>& indices )
{
	indices.clear();

	LTVector min = pos - dims;
	LTVector max = pos + dims;

	// a hierarchical approach might be better, but the number of blockers in a scene should be quite small in practice
	for( uint32 i = 0; i < m_Blockers.size(); i++ )
	{
		// check if the AABBs are disjoint
		if( ((min.x > m_Blockers[i]->m_MaxBounds.x) || (m_Blockers[i]->m_MinBounds.x > max.x)) ||
			((min.y > m_Blockers[i]->m_MaxBounds.y) || (m_Blockers[i]->m_MinBounds.y > max.y)) ||
			((min.z > m_Blockers[i]->m_MaxBounds.z) || (m_Blockers[i]->m_MinBounds.z > max.z)) )
				continue;

		// they overlap, so add the blocker to the vector
		indices.push_back( i );
	}

	return true;
}


bool CWorldParticleBlockerData::GetBlockerEdgesXZ( uint32 index, LTPlane& blockerPlane, uint32& numEdges, LTPlane*& edgePlanes )
{
	if( index >= m_Blockers.size() )
	{
		ASSERT(0);
		return false;
	}

	blockerPlane = m_Blockers[index]->m_Plane;
	numEdges = m_Blockers[index]->m_NumVerts;
	edgePlanes = m_Blockers[index]->m_EdgeXZPlanes;

	return true;
}

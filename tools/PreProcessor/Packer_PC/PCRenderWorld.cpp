//////////////////////////////////////////////////////////////////////////////
// PC-specific world geometry representation packer

#include "bdefs.h"

#include "preworld.h"
#include "prepoly.h"
#include "pcrenderworld.h"
#include "pcrendertree.h"

#include "processing.h"

//////////////////////////////////////////////////////////////////////////////
// CPCRenderWorld::CWorldModel implementation

CPCRenderWorld::CWorldModel::CWorldModel() : 
	m_pOctree(0) 
{
}

CPCRenderWorld::CWorldModel::~CWorldModel()
{
	delete m_pOctree;
}

CPCRenderWorld::CWorldModel::CWorldModel(const char *pName, CPCRenderTree *pOctree) :
	m_sName(pName), m_pOctree(pOctree)
{
}

CPCRenderWorld::CWorldModel::CWorldModel(const CPCRenderWorld::CWorldModel &cOther) :
	m_sName(cOther.m_sName), m_pOctree((cOther.m_pOctree) ? new CPCRenderTree(*cOther.m_pOctree) : 0)
{
}

CPCRenderWorld::CWorldModel &CPCRenderWorld::CWorldModel::operator=(const CPCRenderWorld::CWorldModel &cOther) 
{
	m_sName = cOther.m_sName;
	delete m_pOctree;
	m_pOctree = new CPCRenderTree(*cOther.m_pOctree);
	return *this;
}

bool CPCRenderWorld::CWorldModel::Write(CAbstractIO &file) const
{
	if (!m_pOctree)
	{
		ASSERT(!"Found null octree on write");
		return false;
	}

	file.WriteString(const_cast<char*>(m_sName.c_str()));
	if (!m_pOctree->Write(file))
		return false;

	// Indicate that this world model doesn't have any children (makes things much easier on the loading side)
	file << (uint32)0;

	return true;
}


//////////////////////////////////////////////////////////////////////////////
// CPCRenderWorld implementation

CPCRenderWorld::CPCRenderWorld() :
	m_pOctree(new CPCRenderTree)
{
}

CPCRenderWorld::~CPCRenderWorld()
{
	delete m_pOctree;
}

bool CPCRenderWorld::Process(const CPreMainWorld *pWorld)
{
	DrawStatusText(eST_Normal, "  Gathering processed data");

	if (!m_pOctree->ReadWorld(pWorld, pWorld->GetPhysicsBSP()))
		return false;

	DrawStatusText(eST_Normal, "  Packing geometry and lightmaps");

	if (!m_pOctree->Optimize(true))
		return false;

	if (pWorld->m_WorldModels.GetSize() > 1)
	{
		DrawStatusText(eST_Normal, "  Packing worldmodels");

		m_aWorldModels.resize(pWorld->m_WorldModels.GetSize() - 1);

		for (uint32 nCurWorldModelIndex = 0, nOutputWorldModelIndex = 0;
			nCurWorldModelIndex < pWorld->m_WorldModels.GetSize(); 
			++nCurWorldModelIndex, ++nOutputWorldModelIndex)
		{
			const CPreWorld *pCurWorldModel = pWorld->GetWorld(nCurWorldModelIndex);

			// Skip the physics BSP
			if ((pCurWorldModel->m_WorldInfoFlags & WIF_PHYSICSBSP) != 0)
			{
				--nOutputWorldModelIndex;
				continue;
			}

			CWorldModel &cNewWorldModel = m_aWorldModels[nOutputWorldModelIndex];

			cNewWorldModel.m_sName = pCurWorldModel->m_WorldName;
			cNewWorldModel.m_pOctree = new CPCRenderTree;

			if (!cNewWorldModel.m_pOctree->ReadWorld(pWorld, pCurWorldModel))
			{
				DrawStatusText(eST_Error, "Packing world %s failed", cNewWorldModel.m_sName.c_str());
				m_aWorldModels.pop_back();
				continue;
			}

			if (!cNewWorldModel.m_pOctree->Optimize(false))
			{
				DrawStatusText(eST_Error, "optimizing world %s failed", cNewWorldModel.m_sName.c_str());
				m_aWorldModels.pop_back();
				continue;
			}
		}
	}

	return true;
}

bool CPCRenderWorld::Write(CAbstractIO &file)
{
	// Write out the main world
	if (!m_pOctree->Write(file))
		return false;

	// Write out the world models (adjust for the physics and vis WM slots)
	file << (uint32)m_aWorldModels.size();

	TWorldModelList::iterator iCurWorldModel = m_aWorldModels.begin();
	for (; iCurWorldModel != m_aWorldModels.end(); ++iCurWorldModel)
	{
		// Skip the physics and vis world model slots
		if (!iCurWorldModel->m_pOctree)
			continue;

		if (!iCurWorldModel->Write(file))
			return false;
	}

	return true;
}

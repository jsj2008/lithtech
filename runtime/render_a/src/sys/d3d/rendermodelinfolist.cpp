#include "precompile.h"
#include "rendermodelinfolist.h"
#include "relevantlightlist.h"

//singleton access
CRenderModelInfoList& CRenderModelInfoList::GetSingleton()
{
	static CRenderModelInfoList sModelInfoList;
	return sModelInfoList;
}

//This will add an element for the model information and return the index of it
uint32 CRenderModelInfoList::QueueModelInfo(ModelInstance* pInstance)
{
	//figure out what index it will be in
	uint32 nIndex = m_cInfoList.size();

	//grow the list
	LT_MEM_TRACK_ALLOC(m_cInfoList.resize(nIndex + 1), LT_MEM_TYPE_RENDERER);

	//setup the instance pointer
	m_cInfoList[nIndex].m_pInstance			= pInstance;

	//give the user the index so they can refer to the data
	return nIndex;
}

//accesses the device light list of the specified model
CDeviceLightList& CRenderModelInfoList::GetDeviceLightList(uint32 nIndex)
{
	assert((nIndex < m_cInfoList.size()) && "Out of bounds access in the model information");
	return m_cInfoList[nIndex].m_LightList;
}

//accesses the model hook data for the specified model
ModelHookData& CRenderModelInfoList::GetModelHookData(uint32 nIndex)
{
	//sanity check
	assert((nIndex < m_cInfoList.size()) && "Out of bounds access in the model information");
	return m_cInfoList[nIndex].m_ModelHookData;
}

//called to free any list memory, this should be called between levels or so
void CRenderModelInfoList::FreeListMemory()
{
	TModelInfoList EmptyInfoList;
	m_cInfoList.swap(EmptyInfoList);
}

//clears the list of all the information it has stored
void CRenderModelInfoList::Clear()
{
	m_cInfoList.clear();
}

//gets the number of models that are queued up
uint32 CRenderModelInfoList::GetNumQueuedModelInfo() const
{
	return m_cInfoList.size();
}

//gets the instance of the specified index
ModelInstance* CRenderModelInfoList::GetModelInstance(uint32 nIndex)
{
	//sanity check
	assert((nIndex < m_cInfoList.size()) && "Out of bounds access in the model information");
	return m_cInfoList[nIndex].m_pInstance;
}
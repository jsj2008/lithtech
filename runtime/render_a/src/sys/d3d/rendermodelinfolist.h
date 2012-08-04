#ifndef __RENDERMODELINFOLIST_H__
#define __RENDERMODELINFOLIST_H__

class CRelevantLightList;
class ModelInstance;


#ifndef __DEVICELIGHTLIST_H__
#	include "devicelightlist.h"
#endif

class CRenderModelInfoList
{
public:

	//singleton access
	static CRenderModelInfoList&	GetSingleton();	

	//This will add an element for the model information and return the index of it
	uint32	QueueModelInfo(ModelInstance* pInstance);

	//accesses the device light list of the specified model
	CDeviceLightList&		GetDeviceLightList(uint32 nIndex);

	//gets the number of models that are queued up
	uint32					GetNumQueuedModelInfo() const;

	//gets the instance of the specified index
	ModelInstance*			GetModelInstance(uint32 nIndex);

	//accesses the model hook data for the specified model
	ModelHookData&			GetModelHookData(uint32 nIndex);	

	//clears the list of all the information it has stored
	void					Clear();

	//called to free any list memory, this should be called between levels or so
	void					FreeListMemory();

private:

	class CQueuedModelInfo
	{
	public:
		ModelInstance*		m_pInstance;
		ModelHookData		m_ModelHookData;
		CDeviceLightList	m_LightList;
	};

	//prevent instantiation, only the singleton should exist
	CRenderModelInfoList()				{}

	//the list of model information
	typedef vector<CQueuedModelInfo>	TModelInfoList;
	TModelInfoList						m_cInfoList;
};

#endif

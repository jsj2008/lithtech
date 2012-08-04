#ifndef __SETUPMODEL_H__
#define __SETUPMODEL_H__

#include "common_draw.h"

class ViewParams;
class ModelInstance;
class WorldTreeObj;
class Model;
class ModelPiece;
class ModelInstance;
class CDIModelDrawable;

#ifndef __RELEVANTLIGHTLIST_H__
#	include "relevantlightlist.h"
#endif

// ------------------------------------------------------------------------
// ModelDraw
// functions and states for drawing models.
// ------------------------------------------------------------------------
class ModelDraw
{
public:

						ModelDraw();
						~ModelDraw();

	// This function will take all the queued up model information and run through building up the appropriate
	//lighting information
	void				QueueAllModelInfo();
 
	// The main function to queue the models components into the appropriate lists
	void				QueueModel(ModelInstance *pInstance);
	
private:
 
	//queues all the pieces of the model into the global list
	void				QueueModelPieces(ModelInstance* pInstance, const ModelHookData& HookData, CDeviceLightList& DeviceLightList);

	// Calls the model hook function, if any.
	void				CallModelHook(ModelInstance* pInstance, ModelHookData& HookData);

	//determines if the current model should have shadows rendered for it
	bool				ShouldDrawShadows(ModelInstance* pInstance, const ModelHookData& HookData) const;

	// Sets up m_LightAdd and g_ModelDrawFlags.
	static void			StaticLightCB(WorldTreeObj *pObj, void *pUser);
	float				GetDirLightAmount(ModelInstance* pInstance, const LTVector& vInstancePos);
	void				SetupModelLight(ModelInstance* pInstance, const ModelHookData& HookData, CRelevantLightList& LightList);

};

// ---------------------------------------------------------------- //
// Externs.
// ---------------------------------------------------------------- //
extern void d3d_QueueModel(const ViewParams& Params, LTObject *pModel);

#endif  // __SETUPMODEL_H__




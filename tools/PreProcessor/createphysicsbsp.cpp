
#include "bdefs.h"
#include "createphysicsbsp.h"
#include "processing.h"
#include "editregion.h"
#include "preworld.h"
#include "findworldmodel.h"

// Gather brushes used in the physics BSP.
static void GetPhysicsBSPBrushes(
	GenList<CEditBrush*> &in,
	GenList<CEditBrush*> &mergeBrushes)
{
	GenListPos pos;
	CEditBrush *pBrush;

	for(pos=in.GenBegin(); in.GenIsValid(pos); )
	{
		pBrush = in.GenGetNext(pos);
		mergeBrushes.GenAppend(pBrush);
	}
}


bool CreatePhysicsBSP(
	CEditRegion *pRegion,
	CWorldModelDef *pWorldModel,
	CPreWorld *pPreWorld,
	bool bMainWorld)
{
	PVector min, max;
	CLinkedList<CEditBrush*> mergeBrushes;

	//only setup our subtasks for the main world
	if(bMainWorld)
		ActivateSubTask("Gathering Physics Brushes");

	GetPhysicsBSPBrushes(
		pWorldModel->m_Brushes,
		mergeBrushes);

	if(bMainWorld)
		ActivateSubTask("Generating Surfaces");

	GenerateSurfaces(mergeBrushes, pPreWorld);

	//only setup our subtasks for the main world
	if(bMainWorld)
		ActivateSubTask("Merging Physics Brushes");

	// Merge stuff together.
	if(!MergeBrushes(mergeBrushes, pPreWorld))
		return false;

	mergeBrushes.GenRemoveAll();

	//only setup our subtasks for the main world
	if(bMainWorld)
		ActivateSubTask("Physics BSP");

	if(!MakeBsp(pPreWorld) )
	{
		DrawStatusText(eST_Error, "failed to create BSP for world model: %s. Make sure that there are some non-render only brushes beneath this world model.", pPreWorld->m_WorldName);
		return false;
	}
	
	// Get the centerpoint.  The engine actually moves its object to the
	// center when it is created.
	pPreWorld->GetBoundingBox(&min, &max);
	pPreWorld->m_WorldTranslation = min + (max - min) * 0.5f;

	return true;
}


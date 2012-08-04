#include "bdefs.h"
#include "editregion.h"
#include "proputils.h"
#include "processing.h"
#include "editpoly.h"
#include "node_ops.h"

static bool ApplyAmbientOverrideR(CWorldNode* pNode, bool bApply, const LTVector& vColor, CMoArray<CWorldNode*>& OverrideList)
{
	//bail if no node
	if(!pNode)
		return true;

	bool		bFinalApply = bApply;
	LTVector	vFinalColor = vColor;

	//see if this object is an ambient override object
	if(pNode->GetType() == Node_Object)
	{
		//we have an object, determine if this is an ambient override
		if(stricmp(pNode->GetClassName(), "AmbientOverride") == 0)
		{
			CBaseProp* pProp = pNode->GetPropertyList()->GetProp("AmbientLight");
			if(pProp && ((pProp->GetType() == PT_COLOR) || (pProp->GetType() == PT_VECTOR)))
			{
				vFinalColor = ((CVectorProp*)pProp)->m_Vector;
			}

			//add that to the list of ambient overrides
			OverrideList.Append(pNode);

			//now we override
			bFinalApply = true;
		}
	}
	else if((pNode->GetType() == Node_Brush) && bApply)
	{
		//this is a brush, try and override its ambient light property
		CBaseProp* pProp = pNode->GetPropertyList()->GetProp("AmbientLight");

		if(pProp && ((pProp->GetType() == PT_COLOR) || (pProp->GetType() == PT_VECTOR)))
		{
			((CVectorProp*)pProp)->m_Vector = vColor;
		}
	}

	//by this time, all prefab references MUST be detached
	assert(pNode->GetType() != Node_PrefabRef);

	//now recurse into all the children
	GPOS Pos = pNode->m_Children;
	while(Pos)
	{
		CWorldNode* pChild = pNode->m_Children.GetNext(Pos);

		if(!ApplyAmbientOverrideR(pChild, bFinalApply, vFinalColor, OverrideList))
			return false;
	}

	return true;
}

static bool RemoveAmbientOverrideR(CEditRegion* pRegion, CMoArray<CWorldNode*>& OverrideList)
{
	for(uint32 nCurrOverride = 0; nCurrOverride < OverrideList.GetSize(); nCurrOverride++)
	{
		no_DestroyNode(pRegion, OverrideList[nCurrOverride], false);
	}

	return true;
}


bool ApplyAmbientOverride(CEditRegion* pRegion)
{
	CMoArray<CWorldNode*> OverrideList;

	//just recurse and apply....
	if(!ApplyAmbientOverrideR(pRegion->GetRootNode(), false, LTVector(0, 0, 0), OverrideList))
		return false;

	return RemoveAmbientOverrideR(pRegion, OverrideList);
}

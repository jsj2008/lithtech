#include "bdefs.h"
#include "editregion.h"
#include "proputils.h"
#include "processing.h"
#include "editpoly.h"
#include "node_ops.h"

static bool ApplyRenderGroupsR(CWorldNode* pNode, bool bApply, uint32 nGroup, CMoArray<CWorldNode*>& RenderGroupList)
{
	//bail if no node
	if(!pNode)
		return true;

	bool		bFinalApply = bApply;
	uint32		nFinalGroup = nGroup;

	//see if this object is an ambient override object
	if(pNode->GetType() == Node_Object)
	{
		//we have an object, determine if this is an ambient override
		if(stricmp(pNode->GetClassName(), "RenderGroup") == 0)
		{
			CBaseProp* pProp = pNode->GetPropertyList()->GetProp("RenderGroup");
			if(pProp && ((pProp->GetType() == PT_LONGINT) || (pProp->GetType() == PT_REAL)))
			{
				nFinalGroup = (uint32)(((CRealProp*)pProp)->m_Value + 0.5f);
			}

			//add that to the list of ambient overrides
			RenderGroupList.Append(pNode);

			//now we override
			bFinalApply = true;
		}
		else
		{
			//see if we need to apply the current render group to the current object
			if(bApply)
			{
				CBaseProp* pProp = pNode->GetPropertyList()->GetProp("RenderGroup");
				if(pProp)
				{
					if((pProp->GetType() == PT_LONGINT) || (pProp->GetType() == PT_REAL))
					{
						((CRealProp*)pProp)->SetValue((float)nGroup);
					}
				}
				else
				{
					//we need to add this property
					CRealProp* pProp = new CRealProp("RenderGroup");

					if(pProp)
					{
						pProp->SetValue((float)nGroup);
						pNode->GetPropertyList()->m_Props.Append(pProp);
					}
				}
			}
		}
	}

	//by this time, all prefab references MUST be detached
	assert(pNode->GetType() != Node_PrefabRef);

	//now recurse into all the children
	GPOS Pos = pNode->m_Children;
	while(Pos)
	{
		CWorldNode* pChild = pNode->m_Children.GetNext(Pos);

		if(!ApplyRenderGroupsR(pChild, bFinalApply, nFinalGroup, RenderGroupList))
			return false;
	}

	return true;
}

static bool RemoveRenderGroupsR(CEditRegion* pRegion, CMoArray<CWorldNode*>& RenderGroupList)
{
	for(uint32 nCurrGroup = 0; nCurrGroup < RenderGroupList.GetSize(); nCurrGroup++)
	{
		no_DestroyNode(pRegion, RenderGroupList[nCurrGroup], false);
	}

	return true;
}


bool ApplyRenderGroups(CEditRegion* pRegion)
{
	CMoArray<CWorldNode*> RenderGroupList;

	//just recurse and apply....
	if(!ApplyRenderGroupsR(pRegion->GetRootNode(), false, 0, RenderGroupList))
		return false;

	return RemoveRenderGroupsR(pRegion, RenderGroupList);
}

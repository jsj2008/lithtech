#include "bdefs.h"
#include "EditRegion.h"
#include "processing.h"
#include "proputils.h"

#define GROUP_CLASS_NAME		"Group"
#define PROPERTY_NAME			"Object%d"
#define STARTING_PROPERTY		1

static bool AddObjectToList(CBaseEditObj* pGroup, const char* pszName)
{
	CPropList& PropList = pGroup->m_PropList;

	//we need to run through the property list and make sure that this name
	//isn't already in there, and also tag a blank space to put it in. 
	CStringProp*	pPutIn		= NULL;
	uint32			nCurrProp	= STARTING_PROPERTY;

	while(1)
	{
		//build up the property name
		char pszPropName[256];
		sprintf(pszPropName, PROPERTY_NAME, nCurrProp);

		CBaseProp* pProp = PropList.GetProp(pszPropName);

		//see if we have hit the end
		if(!pProp)
			break;

		//make sure that this is a string property
		if(pProp->GetType() == LT_PT_STRING)
		{
			//we have a string
			CStringProp* pString = (CStringProp*)pProp;

			//see if this matches
			if(stricmp(pszName, pString->m_String) == 0)
			{
				//it does match, we don't need to add it
				return true;
			}

			//see if this is blank
			if(!pPutIn && (strlen(pString->m_String) == 0))
			{
				pPutIn = pString;
			}
		}

		//next property
		nCurrProp++;
	}

	//see if we had room
	if(!pPutIn)
	{
		//no room
		DrawStatusText(eST_Error, "There was not enough room in group object %s to add the object %s", pGroup->GetName(), pszName);
		return false;
	}

	//ok, we have room
	strncpy(pPutIn->m_String, pszName, MAX_STRINGPROP_LEN);

	//success
	return true;
}

static void RecurseFillInGroups(CWorldNode* pNode, CBaseEditObj* pPutInGroup)
{
	//sanity check
	if(!pNode)
		return;

	//the object that we will give to the children to put their objects in
	CBaseEditObj* pRecursePutIn = pPutInGroup;

	//first off, see if this node is an object
	if(pNode->GetType() == Node_Object)
	{
		//we have an object, lets see if this is a group object, if so this will be fed to
		//the children
		if(stricmp(pNode->GetClassName(), GROUP_CLASS_NAME) == 0)
		{
			//this is indeed a group
			pRecursePutIn = (CBaseEditObj*)pNode;
		}

		//we need to add this into the actual group object if it exists
		if(pPutInGroup)
		{
			AddObjectToList(pPutInGroup, pNode->GetName());
		}
	}

	//now we need to recurse into all of the children
	GPOS Pos = pNode->m_Children;
	while(Pos)
	{
		CWorldNode* pChild = pNode->m_Children.GetNext(Pos);
		RecurseFillInGroups(pChild, pRecursePutIn);
	}
}


bool FillInGroupObjects(CEditRegion* pRegion)
{
	//we need to run through all of the nodes, and when we find a group object, fill it out
	RecurseFillInGroups(pRegion->GetRootNode(), NULL);

	return false;
}

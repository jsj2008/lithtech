#include "bdefs.h"
#include "errordetector.h"

CErrorDetector::CErrorDetector(const char* pszHelp) :
	m_bNotifyBrush(false),
	m_bNotifyObject(false),
	m_bNotifyPoly(false),
	m_bNotifyPrefab(false),
	m_pHelpText(NULL)
{
	if(pszHelp)
	{
		SetHelpText(pszHelp);
	}
}


CErrorDetector::~CErrorDetector()
{
	FreeHelpText();
}

//determines if this detector is enabled
bool CErrorDetector::IsEnabled() const
{
	return m_bEnabled;
}

//specifies if this should be enabled
void CErrorDetector::SetEnabled(bool bEnable)
{
	m_bEnabled = bEnable;
}


//determines the name of this item
const char* CErrorDetector::GetName() const
{
	//this must be overridden
	ASSERT(false);

	return "DefaultErrorDetector";
}

//get the help text of this item
const char* CErrorDetector::GetHelpText() const
{
	return m_pHelpText;
}

//called to build up a list of error objects based upon the given region. The
//list contains the error objects, which the callee is repsonsible for
//freeing
bool CErrorDetector::BuildErrorList(	CRegionDoc* pDoc, 
										CMoArray<CLevelError*>& ErrorList)
{
	//we want to give the user the chance to build up their list
	if(InternalBuildErrorList(pDoc) == false)
	{
		//clear our list
		m_ErrorList.RemoveAll();

		return false;
	}

	//now we need to copy over our list into the other list
	ErrorList.AppendArray(m_ErrorList);

	//now clean out our list
	m_ErrorList.RemoveAll();

	return true;
}


bool CErrorDetector::InternalBuildErrorList(CRegionDoc* pDoc)
{
	RecurseOnList(pDoc->GetRegion()->GetRootNode());
	return true;
}

//will recurse on the given node, and call the appropriate notify functions
//upon encountering any types that are specified
bool CErrorDetector::RecurseOnList(CWorldNode* pRoot)
{
	//sanity check
	ASSERT(pRoot);

	//see who we should notify about this node
	switch(pRoot->GetType())
	{
	case Node_PrefabRef:
		if(m_bNotifyPrefab)
			OnPrefab((CPrefabRef*)pRoot);
		break;
	case Node_Object:
		if(m_bNotifyObject)
			OnObject((CBaseEditObj*)pRoot);
		break;
	case Node_Brush:
		{
			CEditBrush* pBrush = (CEditBrush*)pRoot;

			if(m_bNotifyBrush)
				OnBrush(pBrush);

			//see if we need to notify for each polygon
			if(m_bNotifyPoly)
			{
				for(uint32 nCurrPoly = 0; nCurrPoly < pBrush->m_Polies.GetSize(); nCurrPoly++)
				{
					if(!OnPoly(pBrush, pBrush->m_Polies[nCurrPoly]))
						break;
				}
			}
		}
		break;
	default:
		//most likely a container node
		break;
	}

	//run through all the node's children
	GPOS Pos = pRoot->m_Children;

	while(Pos)
	{
		CWorldNode* pChild = pRoot->m_Children.GetNext(Pos);

		RecurseOnList(pChild);
	}

	return true;
}

//adds a new error to the list to be returned
void CErrorDetector::AddNewError(CLevelError* pError)
{
	m_ErrorList.Append(pError);
}


//frees memory associated with the help text
void CErrorDetector::FreeHelpText()
{
	delete [] m_pHelpText;
	m_pHelpText = NULL;
}

//sets the help text associated with this item
bool CErrorDetector::SetHelpText(const char* pszText)
{
	FreeHelpText();

	if(pszText == NULL)
		return true;

	uint32 nStrLen = strlen(pszText);

	m_pHelpText = new char [nStrLen + 1];
	
	if(m_pHelpText == NULL)
		return false;

	strcpy(m_pHelpText, pszText);

	return true;
}



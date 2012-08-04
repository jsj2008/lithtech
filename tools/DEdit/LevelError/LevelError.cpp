#include "bdefs.h"
#include "worldnode.h"
#include "levelerror.h"
#include "regiondoc.h"

CLevelError::CLevelError(	const char* pszName, 
							EErrorSev eSeverity, 
							CWorldNode* pNode, const char* pszText,
							CErrorDetector* pDetector) :
	m_Severity(eSeverity),
	m_pszHelpText(NULL),
	m_bResolved(false),
	m_pszName(NULL),
	m_pszNodeName(NULL),
	m_pDetector(pDetector),
	m_bHasNode(false)
{
	SetName(pszName);
	SetHelp(pszText);

	if(pNode)
	{
		//save the node information. We cannot hold onto this since it can be
		//deleted, etc
		m_bHasNode	= true;
		m_nNodeID	= pNode->GetUniqueID();

		uint32 nStrLen = strlen(pNode->GetName());
		m_pszNodeName = new char[nStrLen + 1];
		if(m_pszNodeName)
		{
			strcpy(m_pszNodeName, pNode->GetName());
		}
	}
}

CLevelError::~CLevelError()
{
	FreeHelp();
	FreeName();
	FreeNodeName();
}

//gets the help text associated with this error object
const char* CLevelError::GetHelp() const
{
	return m_pszHelpText;
}

CErrorDetector* CLevelError::GetDetector()
{
	return m_pDetector;
}

//gets the name
const char* CLevelError::GetName() const
{
	return m_pszName;
}

//determines the severity of this issue
EErrorSev CLevelError::GetSeverity() const
{
	return m_Severity;
}

//sets the help string
bool CLevelError::SetHelp(const char* pszText)
{
	FreeHelp();

	//bail if there is no text
	ASSERT(pszText);

	//allocate a buffer
	uint32 nStrLen = strlen(pszText);
	m_pszHelpText = new char [nStrLen + 1];

	//allocation check
	if(m_pszHelpText == NULL)
		return false;

	//and finally copy it over
	strcpy(m_pszHelpText, pszText);
	return true;
}

//sets the name
bool CLevelError::SetName(const char* pszName)
{
	FreeName();

	//bail if there is no text
	ASSERT(pszName);

	//allocate a buffer
	uint32 nStrLen = strlen(pszName);
	m_pszName = new char [nStrLen + 1];

	//allocation check
	if(m_pszName == NULL)
		return false;

	//and finally copy it over
	strcpy(m_pszName, pszName);
	return true;
}

//frees the name memory
void CLevelError::FreeName()
{
	delete [] m_pszName;
	m_pszName = NULL;
}

//frees memory associated with the help text
void CLevelError::FreeHelp()
{
	delete [] m_pszHelpText;
	m_pszHelpText = NULL;
}

//frees memory associated with the node's name
void CLevelError::FreeNodeName()
{
	delete [] m_pszNodeName;
	m_pszNodeName = NULL;
}

//gets the node associated with this error
const char* CLevelError::GetNodeName() const
{
	ASSERT(HasNode());
	return m_pszNodeName;
}

//see if this item has a node
bool CLevelError::HasNode() const
{
	return m_bHasNode;
}

//get the ID of the node associated with it
uint32 CLevelError::GetNodeID() const
{
	ASSERT(HasNode());
	return m_nNodeID;
}

static CWorldNode* RecurseFindNode(CWorldNode* pNode, uint32 nID)
{
	ASSERT(pNode);

	if(nID == pNode->GetUniqueID())
	{
		return pNode;
	}

	//run through all the node's children
	GPOS Pos = pNode->m_Children;

	while(Pos)
	{
		CWorldNode* pChild = pNode->m_Children.GetNext(Pos);

		CWorldNode* pRV = RecurseFindNode(pChild, nID);

		if(pRV)
			return pRV;
	}

	return NULL;
}


//given a document and a unique ID it will find a node if it exists, or null
//otherwise
CWorldNode*	CLevelError::FindNodeInRegion(CRegionDoc* pDoc, uint32 nID)
{
	//safety first
	if((pDoc == NULL) || (pDoc->GetRegion() == NULL) || (pDoc->GetRegion()->GetRootNode() == NULL))
		return NULL;

	return RecurseFindNode(pDoc->GetRegion()->GetRootNode(), nID);
}

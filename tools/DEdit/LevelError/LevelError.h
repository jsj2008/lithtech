#ifndef __LEVELERROR_H__
#define __LEVELERROR_H__

//the types of severity that an issue can have
enum EErrorSev	{	ERRORSEV_CRITICAL,
					ERRORSEV_HIGH,
					ERRORSEV_MEDIUM,
					ERRORSEV_LOW,
					ERRORSEV_VERYLOW	
				};



//forward declaration
class CErrorDetector;
class CRegionDoc;

class CLevelError
{
public:

	CLevelError(const char* pszName, EErrorSev eSeverity, 
				CWorldNode* pNode, const char* pszHelp, CErrorDetector* pDetector);

	~CLevelError();

	//a short name describing what this issue is
	const char*		GetName() const;

	//gets the node associated with this error
	const char*		GetNodeName() const;

	//see if this item has a node
	bool			HasNode() const;

	//get the ID of the node associated with it
	uint32			GetNodeID() const;

	//gets the help text associated with this error object
	const char*		GetHelp() const;

	//determines the severity of this issue
	EErrorSev		GetSeverity() const;

	//sets the help string
	bool			SetHelp(const char* pszText);

	//sets the name
	bool			SetName(const char* pszName);

	//gets the detector that created this error
	CErrorDetector*	GetDetector();

	//given a document and a unique ID it will find a node if it exists, or null
	//otherwise
	static CWorldNode*	FindNodeInRegion(CRegionDoc* pDoc, uint32 nID);

private:

	//frees the memory associated with the name
	void			FreeName();

	//frees memory associated with the help text
	void			FreeHelp();

	//frees memory associated with the node's name
	void			FreeNodeName();

	//the severity of this issue
	EErrorSev				m_Severity;

	//boolean specifying if a node is related to this object
	bool					m_bHasNode;

	//the ID of the node
	uint32					m_nNodeID;

	//the name of the node
	char*					m_pszNodeName;

	//the help text
	char*					m_pszHelpText;

	//the name
	char*					m_pszName;

	//whether or not this issue is resolved
	bool					m_bResolved;

	//opaque pointer to creating detector
	CErrorDetector*			m_pDetector;

};

#endif

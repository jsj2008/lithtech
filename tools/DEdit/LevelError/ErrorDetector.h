#ifndef __ERRORDETECTOR_H__
#define __ERRORDETECTOR_H__

#include "RegionDoc.h"

#ifndef __LEVELERROR_H__
#	include "levelerror.h"
#endif

class CErrorDetector
{
public:
	CErrorDetector(const char* pszHelp = NULL);
	virtual ~CErrorDetector();

	//determines the name of this item
	virtual const char*		GetName() const;

	//called to build up a list of error objects based upon the given region. The
	//list contains the error objects, which the callee is repsonsible for
	//freeing
	bool					BuildErrorList(	CRegionDoc* pDoc, 
											CMoArray<CLevelError*>& ErrorList);

	//gets the help string associeated with this item
	const char*				GetHelpText() const;

	//determines if this detector is enabled
	bool					IsEnabled() const;

	//specifies if this should be enabled
	void					SetEnabled(bool bEnable);

protected:

	//sets the help text associated with this item
	bool					SetHelpText(const char* pszText);

	//this is the funtion that is derived and is called for creating the error list
	//MUST be overriden
	virtual bool			InternalBuildErrorList(CRegionDoc* pDoc);

	//will recurse on the given node, and call the appropriate notify functions
	//upon encountering any types that are specified
	virtual bool			RecurseOnList(CWorldNode* pRoot);

	//adds a new error to the list to be returned
	void					AddNewError(CLevelError* pError);

	//notify callbacks for each type of node. 
	virtual void			OnBrush(CEditBrush* pBrush)						{}
	virtual void			OnObject(CBaseEditObj* pObj)					{}
	virtual void			OnPrefab(CPrefabRef* pPrefab)					{}

	//this will be called for each polygon. If it returns false, it will stop going over the
	//brush's polygons
	virtual bool			OnPoly(CEditBrush* pBrush, CEditPoly* pPoly)	{ return false; }

	//determine what nodes should have their callback called
	bool					m_bNotifyBrush;
	bool					m_bNotifyObject;
	bool					m_bNotifyPoly;
	bool					m_bNotifyPrefab;


private:

	//the enabled status
	bool					m_bEnabled;

	//frees memory associated with the help text
	void					FreeHelpText();

	//the help text
	char					*m_pHelpText;

	//the internal list of objects to be returned
	CMoArray<CLevelError*>	m_ErrorList;
};

#endif


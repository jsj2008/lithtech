//
//   (c) 1998-1999 Monolith Productions, Inc.  All Rights Reserved
//
// ---------------------------------------------------------------
//
//------------------------------------------------------------------
//
//	FILE	  : Navigator.h
//
//	PURPOSE	  : Defines the CNavigator class, used to provide 
//              functionality to assist the viewer interface.
//
//	CREATED	  : October 3 1996
//
//
//------------------------------------------------------------------

#ifndef __NAVIGATOR_H__
#define __NAVIGATOR_H__


// Includes....
#include "ccs.h"


// Defines....

class CLTANode;
class CLTAFile;

/************************************************************************/
class CNavigator : public CCS
{
	public:

		// Updates m_LookAtDist. This assumes that m_Look at is set
		void		UpdateViewerDistance();

		// Makes it look at m_LookAt. This will update the position of
		// the navigator based upon the viewing distance (Note: The distance
		// must be set before making this call)
		void		UpdateLooking();

		//tells the navigator to look at a specific point. This will update
		//both the distance and position
		void		LookAt(LTVector vPt);
				
		// Load/Save
		BOOL		LoadLTA(CLTANode* pParseNode);
		BOOL		SaveLTA(CLTAFile* pFile, uint32 level);

		bool		LoadTBW(CAbstractIO& InFile);
		bool		SaveTBW(CAbstractIO& OutFile);

	public:

		CVector		m_LookAt;
		CReal		m_LookAtDist;

};

/************************************************************************/
// This is used to store navigator items so that the user can switch
// between them while editing the level.  It is basically the same as
// the navigator class with the added description field.
class CNavigatorPosItem
{
public:
	// Constructors
	CNavigatorPosItem();
	
	// Destructor
	virtual ~CNavigatorPosItem();

	// Termination
	void		Term();

	// Load/Save
	BOOL		LoadLTA(CLTANode* pParseNode);
	BOOL		SaveLTA(CLTAFile* pFile, uint32 level);

	bool		LoadTBW(CAbstractIO& InFile);
	bool		SaveTBW(CAbstractIO& OutFile);

	// Access to member variables
	void		SetDescription(char *lpszDescription);
	char		*GetDescription()						{ return m_lpszDescription; }

	void		AddNavigator(CNavigator *pNav)			{ m_NavigatorArray.Add(pNav); }
	CNavigator	*GetNavigator(int nIndex)				{ return m_NavigatorArray[nIndex]; }
	
protected:	
	CMoArray<CNavigator *>	m_NavigatorArray;
	char					*m_lpszDescription;
};

/************************************************************************/
// Array of navigator pos items with save/load support
class CNavigatorPosArray : public CMoArray<CNavigatorPosItem *>
{	
public:
	// Load/Save
	BOOL		LoadLTA(CLTANode* pParseNode);
	BOOL		SaveLTA(CLTAFile* pFile, uint32 level);

	bool		LoadTBW(CAbstractIO& InFile);
	bool		SaveTBW(CAbstractIO& OutFile);
};

#endif  // __NAVIGATOR_H__

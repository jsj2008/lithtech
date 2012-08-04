//
//   (c) 1998-1999 Monolith Productions, Inc.  All Rights Reserved
//
// ---------------------------------------------------------------
//
// AdvancedSelect.h: interface for the CAdvancedSelect class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_ADVANCEDSELECT_H__0F99AA03_F1C5_11D2_BE13_0060971BDC6D__INCLUDED_)
#define AFX_ADVANCEDSELECT_H__0F99AA03_F1C5_11D2_BE13_0060971BDC6D__INCLUDED_

#include "afxtempl.h"

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CRegionDoc;
class CAdvancedSelect  
{
public:
	CAdvancedSelect();
	virtual ~CAdvancedSelect();

	// Initialization
	BOOL	Init(CRegionDoc *pRegionDoc);

	// Performs a selection/unselection based on the current criteria
	void	Select(BOOL bIncludeChildren=FALSE, BOOL bShowResults=FALSE);
	void	Unselect(BOOL bIncludeChildren=FALSE, BOOL bShowResults=FALSE);

	// Clears the current selection criteria
	void	ClearCriteria();

	// These add selection criteria.  Adding the same type of criteria
	// twice makes it an "or" option, while adding two different criteria
	// makes it an "and" option.  For example, if you added the following criteria:
	//
	// Class - Light
	// Class - Door
	// Name - Front
	// Name - Back
	//
	// You would end up with the following selection command:
	// "Select nodes that are (of class light or door) and have the name (front or back)
	//
	// Editorial note:  What the?  I don't think the interface supports this - David C.

	void	AddClassCriteria(CString sClassType);
	void	AddNameCriteria(CString sName, BOOL bPartialString=FALSE);	
	void	AddPropertyCriteria(CBaseProp* pProp, bool matchValue);
	
protected:
	// Recurses and selects nodes based on the criteria.  Returns the number it found
	int		RecurseAndSelect(CWorldNode *pParentNode, bool bSelect, bool bIncludeChildren);

	// Returns true if the node passes the class criteria.  True is also
	// returned if there isn't any class criteria.
	BOOL	IsPassClassCriteria(CWorldNode *pNode);

	// Returns true if the node passes the name criteria.  True is also
	// returned if there isn't any naming criteria.
	BOOL	IsPassNameCriteria(CWorldNode *pNode);

	// Returns true if the node passes the property criteria.  True is also
	// returned if there isn't any naming criteria.
	BOOL	IsPassPropertyCriteria(CWorldNode *pNode);
	
protected:
	///////////////////////////////////////////
	// Advanced select criteria options

	// The class criteria class
	class CClassCriteria
	{
	public:
		void		SetClass(CString sClassType)	{ m_sClass=sClassType; }
		CString		GetClass()						{ return m_sClass; }

	protected:
		CString		m_sClass;
	};

	// The name criteria class
	class CNameCriteria
	{
	public:
		void		SetName(CString sName)			{ m_sName=sName; }
		CString		GetName()						{ return m_sName; }
		
		BOOL		IsPartialString()				{ return m_bPartialString; }
		void		SetPartialString(BOOL bPartial)	{ m_bPartialString=bPartial; }

	protected:
		CString		m_sName;
		BOOL		m_bPartialString;
	};

	// The name criteria class
	class CPropertyCriteria
	{
	public:
		void		SetProp(CBaseProp* prop)		{ m_Prop = prop; }
		CBaseProp*	GetProp()						{ return m_Prop; }

		void		SetMatchValue(bool matchValue)  { m_bMatchValue = matchValue; }
		const bool	GetMatchValue()					{ return m_bMatchValue; }


		 CPropertyCriteria() { m_Prop = NULL;  m_bMatchValue = false; }

		 // manual deletion is better for our purposes
		 void Cleanup() { if (m_Prop != NULL)  delete m_Prop; }

	protected:
		CBaseProp *m_Prop;
		bool m_bMatchValue;
	};

protected:
	// The document to operate on
	CRegionDoc								*m_pRegionDoc;

	// The current class criteria
	CArray<CClassCriteria,CClassCriteria>		m_classCriteria;
	CArray<CNameCriteria,CNameCriteria>			m_nameCriteria;
	CArray<CPropertyCriteria,CPropertyCriteria>	m_propertyCriteria;
};

#endif // !defined(AFX_ADVANCEDSELECT_H__0F99AA03_F1C5_11D2_BE13_0060971BDC6D__INCLUDED_)

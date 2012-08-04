//--------------------------------------------------------------
//OptionsPrefabs.h
//
// Contains the definition for COptionsPrefabs which holds the
// user options for displaying prefabs within DEdit
//
// Author: Kevin Francis
// Created: 7/02/01
// Modification History:
//
//---------------------------------------------------------------
#ifndef __OPTIONSPREFABS_H__
#define __OPTIONSPREFABS_H__

#include "optionsbase.h"

class COptionsPrefabs : public COptionsBase  
{
public:

	enum EViewMode	{	VIEWPREFAB_BOX,
						VIEWPREFAB_CONTENTS,
						VIEWPREFAB_NONE,
					};

	COptionsPrefabs();
	virtual ~COptionsPrefabs();

	// Load/Save
	BOOL	Load();
	BOOL	Save();

	// Access to the options
	EViewMode	GetContentsView() const				{return m_eContentsView;}
	void		SetContentsView(EViewMode eView)	{m_eContentsView = eView;}

	bool		IsShowOutline() const				{return m_bShowOutline;}
	void		SetShowOutline(bool bVal)			{m_bShowOutline = bVal;}

	bool		IsShowOrientation() const			{return m_bShowOrientation;}
	void		SetShowOrientation(bool bVal)		{m_bShowOrientation = bVal;}

protected:

	// The rendering method to use
	EViewMode	m_eContentsView;

	// Draw the dimensions outline?
	bool		m_bShowOutline;
	// Draw the orientation lines?
	bool		m_bShowOrientation;
};

#endif 

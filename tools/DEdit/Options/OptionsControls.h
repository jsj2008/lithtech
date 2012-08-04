//--------------------------------------------------------------
//OptionsControls.h
//
// Contains the definition for COptionsControls which holds the
// user options for controls
//
// Author: John O'Rorke
// Created: 4/3/01
// Modification History:
//
//---------------------------------------------------------------
#ifndef __OPTIONSCONTROLS_H__
#define __OPTIONSCONTROLS_H__

#include "optionsbase.h"

class COptionsControls : public COptionsBase  
{
public:

	COptionsControls();
	virtual ~COptionsControls();

	// Load/Save
	BOOL	Load();
	BOOL	Save();

	// Access to the options
	bool		IsInvertMouseY() const				{return m_bInvertMouseY;}
	void		SetInvertMouseY(bool bVal)			{m_bInvertMouseY = bVal;}

	bool		IsZoomToCursor() const				{return m_bZoomToCursor;}
	void		SetZoomToCursor(bool bVal)			{m_bZoomToCursor = bVal;}

	bool		IsOrbitAroundSel() const			{return m_bOrbitAroundSel;}
	void		SetOrbitAroundSel(bool bVal)		{m_bOrbitAroundSel = bVal;}

	bool		IsAutoCaptureFocus() const			{return m_bAutoCaptureFocus;}
	void		SetAutoCaptureFocus(bool bVal)		{m_bAutoCaptureFocus = bVal;}

protected:

	//should the mouse Y axis be inverted
	bool		m_bInvertMouseY;

	//should the zoom functionality go to the cursor or just center
	bool		m_bZoomToCursor;

	//if in orbit mode, the selection should be orbited around
	bool		m_bOrbitAroundSel;

	//if the mouse movements should cause different views to obtain focus
	bool		m_bAutoCaptureFocus;
};

#endif 

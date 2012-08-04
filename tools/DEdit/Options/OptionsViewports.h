//--------------------------------------------------------------
//OptionsViewports.h
//
// Contains the definition for COptionsViewports which provides
// a means of storing and accessing settings set per viewport
//
// Author: John O'Rorke
// Created: 11/08/01
// Modification History:
//
//---------------------------------------------------------------
#ifndef __OPTIONSVIEWPORTS_H__
#define __OPTIONSVIEWPORTS_H__

#include "optionsbase.h"

class COptionsViewports : public COptionsBase  
{
public:

	enum { NUM_VIEWPORTS	= 4 };

	COptionsViewports();
	virtual ~COptionsViewports();

	// Load/Save
	BOOL	Load();
	BOOL	Save();
	
	// Access to the options
	bool	IsShowGrid(uint32 nVp) const			{ return m_bShowGrid[nVp]; }
	void	SetShowGrid(uint32 nVp, bool bVal)		{ m_bShowGrid[nVp] = bVal; }

	bool	IsShowWireframe(uint32 nVp) const		{ return m_bShowWireframe[nVp]; }
	void	SetShowWireframe(uint32 nVp, bool bVal)	{ m_bShowWireframe[nVp] = bVal; }

	uint32	GetShadeMode(uint32 nVp) const			{ return m_nShadeMode[nVp]; }
	void	SetShadeMode(uint32 nVp, uint32 nMode)	{ m_nShadeMode[nVp] = nMode; }

	bool	IsShowNormals(uint32 nVp) const			{ return m_bShowNormals[nVp]; }
	void	SetShowNormals(uint32 nVp, bool bVal)	{ m_bShowNormals[nVp] = bVal; }

	bool	IsSelectBackfaces(uint32 nVp) const		{ return m_bSelectBackface[nVp]; }
	void	SetSelectBackfaces(uint32 nVp, bool bVal){ m_bSelectBackface[nVp] = bVal; }

	bool	IsShowObjects(uint32 nVp) const			{ return m_bShowObjects[nVp]; }
	void	SetShowObjects(uint32 nVp, bool bVal)	{ m_bShowObjects[nVp] = bVal; }

	bool	IsShowMarker(uint32 nVp) const			{ return m_bShowMarker[nVp]; }
	void	SetShowMarker(uint32 nVp, bool bVal)	{ m_bShowMarker[nVp] = bVal; }


protected:

	bool		m_bShowGrid[NUM_VIEWPORTS];
	bool		m_bShowWireframe[NUM_VIEWPORTS];
	bool		m_bShowNormals[NUM_VIEWPORTS];
	bool		m_bSelectBackface[NUM_VIEWPORTS];
	bool		m_bShowObjects[NUM_VIEWPORTS];
	bool		m_bShowMarker[NUM_VIEWPORTS];

	uint32		m_nShadeMode[NUM_VIEWPORTS];

};

#endif 

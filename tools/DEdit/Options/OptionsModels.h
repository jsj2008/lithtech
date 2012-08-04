//--------------------------------------------------------------
//OptionsModels.h
//
// Contains the definition for COptionsModels which holds the
// user options for displaying models within DEdit
//
// Author: John O'Rorke
// Created: 3/20/01
// Modification History:
//
//---------------------------------------------------------------
#ifndef __OPTIONSMODELS_H__
#define __OPTIONSMODELS_H__

#include "optionsbase.h"

class COptionsModels : public COptionsBase  
{
public:

	enum EViewMode	{	VIEWMODEL_BOX,
						VIEWMODEL_WIREFRAME,
						VIEWMODEL_SOLID,
						VIEWMODEL_TEXTURED
					};

	COptionsModels();
	virtual ~COptionsModels();

	// Load/Save
	BOOL	Load();
	BOOL	Save();

	// Access to the options
	bool		IsRunLowPriority() const			{return m_bRunLowPriority;}
	void		SetRunLowPriority(bool bVal)		{m_bRunLowPriority = bVal;}

	bool		IsLimitMemoryUse() const			{return m_bLimitMemoryUse;}
	void		SetLimitMemoryUse(bool bVal)		{m_bLimitMemoryUse = bVal;}

	uint32		GetMaxMemoryUse() const				{return m_nMaxMemoryUse;}
	void		SetMaxMemoryUse(uint32 nMax)		{m_nMaxMemoryUse = nMax;}

	bool		IsRenderBoxAtDist() const			{return m_bRenderBoxAtDist;}
	void		SetRenderBoxAtDist(bool bVal)		{m_bRenderBoxAtDist = bVal;}

	uint32		GetRenderBoxDist() const			{return m_nRenderBoxDist;}
	void		SetRenderBoxDist(uint32 nDist)		{m_nRenderBoxDist = nDist;}

	uint32		GetPerspectiveMode() const			{return m_nPerspectiveMode;}
	void		SetPerspectiveMode(uint32 Mode)		{m_nPerspectiveMode = Mode;}

	uint32		GetOrthoMode() const				{return m_nOrthoMode;}
	void		SetOrthoMode(uint32 Mode)			{m_nOrthoMode = Mode;}

	bool		IsAlwaysShowModels() const			{return m_bAlwaysShowModels;}
	void		SetAlwaysShowModels(bool bVal)		{m_bAlwaysShowModels = bVal;}

protected:

	//the view mode for orthographic views
	uint32		m_nOrthoMode;

	//the view mode for perspective views
	uint32		m_nPerspectiveMode;

	//determines if the models should be rendered as simply boxes
	//at a distance, and at what distance
	bool		m_bRenderBoxAtDist;
	uint32		m_nRenderBoxDist;

	//determines if memory should be limited to a certain amount,
	//and what that amount is
	bool		m_bLimitMemoryUse;
	uint32		m_nMaxMemoryUse;

	//determines if models should always be displayed regardless
	bool		m_bAlwaysShowModels;

	//determines if the model loading thread
	//should be run at a lower priority than the main thread.
	bool		m_bRunLowPriority;
};

#endif 

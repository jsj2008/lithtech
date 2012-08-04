// ----------------------------------------------------------------------- //
//
// MODULE  : ScreenPerformanceGPU.h
//
// PURPOSE : Declares the CScreenPerformanceGPU class.  This class sets
//           the GPU performance settings.
//
// CREATED : 04/05/05
//
// (c) 2005 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#pragma once
#ifndef __SCREENPERFORMANCEGPU_H__
#define __SCREENPERFORMANCEGPU_H__

#include "ScreenPerformanceAdvanced.h"
#include "ScreenDisplay.h"

class CScreenPerformanceGPU : public CScreenPerformanceAdvanced
{
public:
	enum eFlag
	{
		eFlag_SoftShadowWarning			= 0x00000001,
		eFlag_TextureResolutionWarning	= 0x00000002,
		eFlag_ScreenResolutionWarning	= 0x00000004,
		eFlag_VolumeLightsWarning		= 0x00000008,

		eFlagWarningMask				= (	eFlag_SoftShadowWarning |
											eFlag_TextureResolutionWarning |
											eFlag_ScreenResolutionWarning |
											eFlag_VolumeLightsWarning )
	};

public:
	CScreenPerformanceGPU();
	virtual ~CScreenPerformanceGPU();

	// Build the screen
	virtual bool	Build();

	virtual void    OnFocus(bool bFocus);

	// checks to see if the user has blown their video memory limit
	void			CheckResolutionMemory();

protected:
	ScreenDisplayRenderer			m_rendererData;
	uint32							m_dwFlags;
	CLTGUIColumnCtrl*				m_pCtrlResolution;

	virtual void	UpdateOption( CLTGUIColumnCtrl* pCtrl, int nGroup, int nIndex );
	void			UpdateOption_Resolution( CLTGUIColumnCtrl* pCtrl, int nGroup, int nIndex );
	void			UpdateOption_SoftShadows( CLTGUIColumnCtrl* pCtrl, int nGroup, int nIndex );
	void			UpdateOption_TextureResolution( CLTGUIColumnCtrl* pCtrl, int nGroup, int nIndex );
	void			UpdateOption_VolumeLights( CLTGUIColumnCtrl* pCtrl, int nGroup, int nIndex );

	virtual void	Repair();

	// update the color of the resolution based on memory usage
	void			UpdateResolutionColor();

	//take the values from the game and set the controls
	virtual void	Load();

	//take the values from the controls and set them in game
	virtual void	Save();
};


#endif  // __SCREENPERFORMANCEGPU_H__

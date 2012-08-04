// ----------------------------------------------------------------------- //
//
// MODULE  : ScreenPerformanceCPU.h
//
// PURPOSE : Declares the CScreenPerformanceCPU class.  This class sets
//           the CPU performance options.
//
// CREATED : 04/05/05
//
// (c) 2005 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#pragma once
#ifndef __SCREENPERFORMANCECPU_H__
#define __SCREENPERFORMANCECPU_H__

#include "ScreenPerformanceAdvanced.h"

class CScreenPerformanceCPU : public CScreenPerformanceAdvanced
{
public:
	enum eFlag
	{
		eFlag_WorldDetailWarning		= 0x0000001,

		eFlagWarningMask				= (	eFlag_WorldDetailWarning )
	};

	CScreenPerformanceCPU();
	virtual ~CScreenPerformanceCPU();

	virtual void OnFocus(bool bFocus);

protected:
	//take the values from the game and set the controls
	void	Load();

	//take the values from the controls and set them in game
	void	Save();

	virtual void	UpdateOption( CLTGUIColumnCtrl* pCtrl, int nGroup, int nIndex );
	void			UpdateOption_WorldDetail( CLTGUIColumnCtrl *pCtrl, int nGroup, int nIndex );

	uint32							m_dwFlags;

};


#endif  // __SCREENPERFORMANCECPU_H__

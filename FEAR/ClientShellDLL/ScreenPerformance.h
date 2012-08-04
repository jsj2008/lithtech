// ----------------------------------------------------------------------- //
//
// MODULE  : ScreenPerformance.h
//
// PURPOSE : screen to set performance options
//
// CREATED : 09/23/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __SCREENPERFORMANCE_H__
#define __SCREENPERFORMANCE_H__

#include "BaseScreen.h"
#include "PerformanceMgr.h"

struct SPerformanceStats;

class CScreenPerformance : public CBaseScreen
{
public:
	CScreenPerformance();
	virtual ~CScreenPerformance();

	// Build the screen
	bool	Build();

	// called when the user escapes out of the screen
	void	Escape();

	void    OnFocus(bool bFocus);

protected:
	uint32  OnCommand(uint32 dwCommand, uint32 dwParam1, uint32 dwParam2);

	// Launches the performance test level.
	void	LaunchPerformanceTest( );

	//take the values from the game and set the controls
	void	Load();

	//take the values from the controls and set them in game
	void	Save();

    //set controls to specific values
	void	SetOverall(uint32 nType, DetailLevel eLevel);
	void	SetBasedOnPerformanceStats();

	//repairs values that may have been set incorrectly
	void	Repair();

	//update overall based on changes to controls
	void	UpdateOverall(uint32 nType);

	// checks to see if the user has blown their video memory limit
	void	CheckResolutionMemory();
	// update the color of the global GPU option based on memory usage
	void	UpdateGPUColor();

	CLTGUICycleCtrl*			m_pOverallCPU;
	CLTGUICycleCtrl*			m_pOverallGPU;
	bool						m_bTrueExit;
	uint32						m_nWarningColor;

	static uint8 CycleCallback(CLTGUICycleCtrl* pCycle, uint8 nCurIndex, bool bIncrement );
};


#endif  // __SCREENPERFORMANCE_H__

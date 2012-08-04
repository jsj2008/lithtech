//----------------------------------------------------------------------------------------
//
// MODULE  : FrameStatConsoleInfo.h
//
// PURPOSE : Handles displaying the information from the frame statistics to the console
//			 based upon different console variables that are enabled/disabled
//
// CREATED : 5/30/03
//
// (c) 1999-2003 Monolith Productions, Inc.  All Rights Reserved
//
//----------------------------------------------------------------------------------------
#ifndef __FRAMESTATCONSOLEINFO_H__
#define __FRAMESTATCONSOLEINFO_H__

//forward declarations
class CRendererFrameStats;

class CFrameStatConsoleInfo
{
public:

	//this function allows the frame stat console variables to be initialized
	static void		Init();

	//called to display frame statistics based upon the currently enabled console variables
	//and the provided rendering information
	static bool		DisplayConsoleInfo(const CRendererFrameStats& FrameStats, float fFrameTime);
};

#endif

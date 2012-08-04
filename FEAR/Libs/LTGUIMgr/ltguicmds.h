// ----------------------------------------------------------------------- //
//
// MODULE  : ltguicmds.h
//
// PURPOSE : Contains global control messages
//
// CREATED : 06/21/05
//
// (c) 2005 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __LTGUICMDS_H__
#define __LTGUICMDS_H__

enum eGUICtrlCmd
{
	// reserve the lower 16bit range for screens
	eGUICtrlCmd_Base = 0x0000FFFF,
	// sets the control specified by nParam1 as the control that captures all input
	eGUICtrlCmd_SetCapture,
	// clears the input capture state
	eGUICtrlCmd_ReleaseCapture,
	// plays the requested sound specified in nParam1
	eGUICtrlCmd_PlaySound,
	// sets the cursor, nParam1 is a pointer to a string indicating the cursor record
	eGUICtrlCmd_SetCursor,
	// tells the screen to update its help text
	eGUICtrlCmd_UpdateHelp,
};

#endif  // __LTGUICMDS_H__

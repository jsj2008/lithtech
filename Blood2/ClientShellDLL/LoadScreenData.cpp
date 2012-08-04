//********************************************************************************
//********************************************************************************
//***** MODULE  : LoadScreenData.cpp
//***** PURPOSE : Blood 2 Loading Screen Information
//***** CREATED : 10/13/98
//********************************************************************************
//********************************************************************************

#include "LoadScreenData.h"
#include "ClientRes.h"

//********************************************************************************
//********** NOTES NOTES NOTES ***************************************************
//********************************************************************************
// Always format these locations and widths to a 640x480 bumper screen,
// and they will automatically get scaled for other resolutions.
//
// If this doesn't work out well, I'll change it... but for now that's how
// it'll work.
//
// The 'Text Wrap Width' is the length that text will get drawn from it's
// left position until it wraps back down to a new line (this gets scaled too)
//
//********************************************************************************
//
// TITLES WILL AUTOMATICALLY BE CENTERED ON THE COORDINATES YOU PUT IN!
//
//********************************************************************************
//
// TEXT WILL AUTOMATICALLY BE LEFT JUSTIFIED AND WRAP AROUND WITH THE WIDTH!
// USE THE TILDE (~) TO TAB THE TEXT OVER, AND THE BAR (|) TO FORCE A LINE BREAK!
//
//********************************************************************************

LoadScreenData g_LoadScreenData[MAX_LOADSCREEN_DATA_TYPES] =
{
	// BS Example for the format (this one never gets used!!)
	{
		0,
		0,
		350,		// Title X
		75,			// Title Y
		200,		// Text X
		100,		// Text Y
		300,		// Text Wrap Width
	},
	// Level 01
	{
		IDS_BUMPER_TITLE_1,
		IDS_BUMPER_TEXT_1,
		350,		// Title X
		75,			// Title Y
		200,		// Text X
		100,		// Text Y
		300,		// Text Wrap Width
	},
	// Level 02 A
	{
		IDS_BUMPER_TITLE_2,
		IDS_BUMPER_TEXT_2,
		350,		// Title X
		75,			// Title Y
		200,		// Text X
		100,		// Text Y
		300,		// Text Wrap Width
	},
	// Level 02 B
	{
		IDS_BUMPER_TITLE_3,
		IDS_BUMPER_TEXT_3,
		350,		// Title X
		75,			// Title Y
		200,		// Text X
		100,		// Text Y
		300,		// Text Wrap Width
	},
	// Level 02 C
	{
		IDS_BUMPER_TITLE_4,
		IDS_BUMPER_TEXT_4,
		350,		// Title X
		75,			// Title Y
		200,		// Text X
		100,		// Text Y
		300,		// Text Wrap Width
	},
	// Level 03
	{
		IDS_BUMPER_TITLE_5,
		IDS_BUMPER_TEXT_5,
		350,		// Title X
		75,			// Title Y
		200,		// Text X
		100,		// Text Y
		300,		// Text Wrap Width
	},
	// Level 04
	{
		IDS_BUMPER_TITLE_6,
		IDS_BUMPER_TEXT_6,
		350,		// Title X
		75,			// Title Y
		200,		// Text X
		100,		// Text Y
		300,		// Text Wrap Width
	},
	// Level 05 A
	{
		IDS_BUMPER_TITLE_7,
		IDS_BUMPER_TEXT_7,
		350,		// Title X
		75,		// Title Y
		200,		// Text X
		100,		// Text Y
		300,		// Text Wrap Width
	},
	// Level 05 B
	{
		IDS_BUMPER_TITLE_8,
		IDS_BUMPER_TEXT_8,
		350,		// Title X
		75,			// Title Y
		200,		// Text X
		100,		// Text Y
		300,		// Text Wrap Width
	},
	// Level 06 A
	{
		IDS_BUMPER_TITLE_9,
		IDS_BUMPER_TEXT_9,
		350,		// Title X
		75,			// Title Y
		200,		// Text X
		100,		// Text Y
		300,		// Text Wrap Width
	},
	// Level 06 B
	{
		IDS_BUMPER_TITLE_10,
		IDS_BUMPER_TEXT_10,
		350,		// Title X
		75,			// Title Y
		200,		// Text X
		100,		// Text Y
		300,		// Text Wrap Width
	},
	// Level 07
	{
		IDS_BUMPER_TITLE_11,
		IDS_BUMPER_TEXT_11,
		350,		// Title X
		75,			// Title Y
		200,		// Text X
		100,		// Text Y
		300,		// Text Wrap Width
	},
	// Level 08
	{
		IDS_BUMPER_TITLE_12,
		IDS_BUMPER_TEXT_12,
		350,		// Title X
		75,			// Title Y
		200,		// Text X
		100,		// Text Y
		300,		// Text Wrap Width
	},
	// Level 09
	{
		IDS_BUMPER_TITLE_13,
		IDS_BUMPER_TEXT_13,
		350,		// Title X
		75,			// Title Y
		200,		// Text X
		100,		// Text Y
		300,		// Text Wrap Width
	},
	// Level 10
	{
		IDS_BUMPER_TITLE_14,
		IDS_BUMPER_TEXT_14,
		350,		// Title X
		75,			// Title Y
		200,		// Text X
		100,		// Text Y
		300,		// Text Wrap Width
	},
	// Level 11
	{
		IDS_BUMPER_TITLE_15,
		IDS_BUMPER_TEXT_15,
		350,		// Title X
		75,			// Title Y
		200,		// Text X
		100,		// Text Y
		300,		// Text Wrap Width
	},
	// Level 12 A
	{
		IDS_BUMPER_TITLE_16,
		IDS_BUMPER_TEXT_16,
		350,		// Title X
		75,			// Title Y
		200,		// Text X
		100,		// Text Y
		300,		// Text Wrap Width
	},
	// Level 12 B
	{
		IDS_BUMPER_TITLE_17,
		IDS_BUMPER_TEXT_17,
		350,		// Title X
		75,			// Title Y
		200,		// Text X
		100,		// Text Y
		300,		// Text Wrap Width
	},
	// Level 13
	{
		IDS_BUMPER_TITLE_18,
		IDS_BUMPER_TEXT_18,
		350,		// Title X
		75,			// Title Y
		200,		// Text X
		100,		// Text Y
		300,		// Text Wrap Width
	},
	// Level 14
	{
		IDS_BUMPER_TITLE_19,
		IDS_BUMPER_TEXT_19,
		350,		// Title X
		75,			// Title Y
		200,		// Text X
		100,		// Text Y
		300,		// Text Wrap Width
	},
	// Level 15
	{
		IDS_BUMPER_TITLE_20,
		IDS_BUMPER_TEXT_20,
		350,		// Title X
		75,			// Title Y
		200,		// Text X
		100,		// Text Y
		300,		// Text Wrap Width
	},
	// Level 16
	{
		IDS_BUMPER_TITLE_21,
		IDS_BUMPER_TEXT_21,
		350,		// Title X
		75,			// Title Y
		200,		// Text X
		100,		// Text Y
		300,		// Text Wrap Width
	},
	// Level 17
	{
		IDS_BUMPER_TITLE_22,
		IDS_BUMPER_TEXT_22,
		350,		// Title X
		75,			// Title Y
		200,		// Text X
		100,		// Text Y
		300,		// Text Wrap Width
	},
	// Level 18
	{
		IDS_BUMPER_TITLE_23,
		IDS_BUMPER_TEXT_23,
		350,		// Title X
		75,			// Title Y
		200,		// Text X
		100,		// Text Y
		300,		// Text Wrap Width
	},
	// Level 19 A
	{
		IDS_BUMPER_TITLE_24,
		IDS_BUMPER_TEXT_24,
		350,		// Title X
		75,			// Title Y
		200,		// Text X
		100,		// Text Y
		300,		// Text Wrap Width
	},
	// Level 19 B
	{
		IDS_BUMPER_TITLE_25,
		IDS_BUMPER_TEXT_25,
		350,		// Title X
		75,			// Title Y
		200,		// Text X
		100,		// Text Y
		300,		// Text Wrap Width
	},
	// Level 19 C
	{
		IDS_BUMPER_TITLE_26,
		IDS_BUMPER_TEXT_26,
		350,		// Title X
		75,			// Title Y
		200,		// Text X
		100,		// Text Y
		300,		// Text Wrap Width
	},
	// Level 20 A
	{
		IDS_BUMPER_TITLE_27,
		IDS_BUMPER_TEXT_27,
		350,		// Title X
		75,			// Title Y
		200,		// Text X
		100,		// Text Y
		300,		// Text Wrap Width
	},
	// Level 20 B
	{
		IDS_BUMPER_TITLE_28,
		IDS_BUMPER_TEXT_28,
		350,		// Title X
		75,			// Title Y
		200,		// Text X
		100,		// Text Y
		300,		// Text Wrap Width
	},
	// Level 21
	{
		IDS_BUMPER_TITLE_29,
		IDS_BUMPER_TEXT_29,
		350,		// Title X
		75,			// Title Y
		200,		// Text X
		100,		// Text Y
		300,		// Text Wrap Width
	},
	// Level 22
	{
		IDS_BUMPER_TITLE_30,
		IDS_BUMPER_TEXT_30,
		350,		// Title X
		75,			// Title Y
		200,		// Text X
		100,		// Text Y
		300,		// Text Wrap Width
	},
	// Level 23
	{
		IDS_BUMPER_TITLE_31,
		IDS_BUMPER_TEXT_31,
		350,		// Title X
		75,			// Title Y
		200,		// Text X
		100,		// Text Y
		300,		// Text Wrap Width
	},
	// Level 24 A
	{
		IDS_BUMPER_TITLE_32,
		IDS_BUMPER_TEXT_32,
		350,		// Title X
		75,			// Title Y
		200,		// Text X
		100,		// Text Y
		300,		// Text Wrap Width
	},
	// Level 24 B
	{
		IDS_BUMPER_TITLE_33,
		IDS_BUMPER_TEXT_33,
		350,		// Title X
		75,			// Title Y
		200,		// Text X
		100,		// Text Y
		300,		// Text Wrap Width
	},
	// Level 25
	{
		IDS_BUMPER_TITLE_34,
		IDS_BUMPER_TEXT_34,
		350,		// Title X
		75,			// Title Y
		200,		// Text X
		100,		// Text Y
		300,		// Text Wrap Width
	}
};

//********************************************************************************

int g_LoadScreenMessages[MAX_LOADING_MESSAGES] =
{
	IDS_LOADING_MESSAGE_1,
	IDS_LOADING_MESSAGE_2,
	IDS_LOADING_MESSAGE_3,
	IDS_LOADING_MESSAGE_4,
	IDS_LOADING_MESSAGE_5,
	IDS_LOADING_MESSAGE_6,
	IDS_LOADING_MESSAGE_7,
	IDS_LOADING_MESSAGE_8,
	IDS_LOADING_MESSAGE_9,
	IDS_LOADING_MESSAGE_10,
	IDS_LOADING_MESSAGE_11,
	IDS_LOADING_MESSAGE_12,
	IDS_LOADING_MESSAGE_13,
	IDS_LOADING_MESSAGE_14,
	IDS_LOADING_MESSAGE_15,
	IDS_LOADING_MESSAGE_16,
	IDS_LOADING_MESSAGE_17,
	IDS_LOADING_MESSAGE_18,
	IDS_LOADING_MESSAGE_19,
	IDS_LOADING_MESSAGE_20,
};

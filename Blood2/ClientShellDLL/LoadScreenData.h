//*************************************************************************
//*************************************************************************
//***** MODULE  : LoadScreenData.h
//***** PURPOSE : Blood 2 Loading Screen Information
//***** CREATED : 10/13/98
//*************************************************************************
//*************************************************************************

#ifndef __LOADSCREENDATA_H__
#define __LOADSCREENDATA_H__

//*************************************************************************

#define		MAX_LOADSCREEN_DATA_TYPES	35
#define		MAX_LOADING_MESSAGES		20

//*************************************************************************

typedef struct LoadScreenData
{
	int		nTitle;
	int		nText;
	short	nTitleX;
	short	nTitleY;
	short	nTextX;
	short	nTextY;
	short	nTextWrapWidth;
}	LoadScreenData;

//*************************************************************************

extern LoadScreenData g_LoadScreenData[MAX_LOADSCREEN_DATA_TYPES];
extern int g_LoadScreenMessages[MAX_LOADING_MESSAGES];

//*************************************************************************

#endif  // __LOADSCREENDATA_H__
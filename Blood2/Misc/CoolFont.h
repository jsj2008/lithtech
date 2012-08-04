//*****************************************************************//
//*****		Project:	Cool Font System
//*****		Author:		Andy Mattingly
//*****		Date:		April 8, 1998
//*****************************************************************//
//*****		File:		CoolFont.h
//*****		Update:		April 8, 1998
//*****************************************************************//

#ifndef		_COOL_FONT_H
#define		_COOL_FONT_H

//*****************************************************************//

#include "basedefs_de.h"
#include "cpp_clientshell_de.h"

//*****************************************************************//

#define		CF_JUSTIFY_LEFT			0
#define		CF_JUSTIFY_CENTER		1
#define		CF_JUSTIFY_RIGHT		2

#define		CF_SOURCE_COLOR			0
#define		CF_DEST_COLOR			1

#define		CF_LOCS					0
#define		CF_WIDTHS				1

//*****************************************************************//

class	CoolFont
{
	public:
		//***** Constructors and destructors *****//
		CoolFont();
		~CoolFont()		{	}

		//***** Memory allocation and deallocation functions *****//
		char	Init(CClientDE *pClient, char *szFile);
		void	Free();

		//***** Drawing functions *****//
		void	Draw(const char *szString, HSURFACE hDest, short x, short y, short justify);
		void	DrawFormat(const char *szString, HSURFACE hDest, short x, short y, short width);
		void	DrawFormatTimed(const char *szString, HSURFACE hDest, short x, short y, short width, DFLOAT ratio, DBOOL fromStart);
		void	DrawSolid(const char *szString, HSURFACE hDest, short x, short y, short justify, HDECOLOR color);
		void	DrawSolidFormat(const char *szString, HSURFACE hDest, short x, short y, short width, HDECOLOR color);

		// Returns the height and width of a text string
		// SHP (10/1/1998)
		DIntPt	GetTextExtents(const char *szString);
		DIntPt	GetTextExtentsFormat(const char *szString, short width);

		short	GetHeight()						{ return height; }

		//***** Data loading functions *****//
		char	LoadXLocs(char *szFontCfg);
		char	LoadXWidths(char *szFontCfg);

		char	CalcAll(short x, short y);
		char	CalcAll();
		char	CalcXLocs();
		char	CalcXWidths();

		//***** Public data members *****//
		short	height;

	protected:
		//***** Private data members *****//
		CClientDE*	client;					// the client interface

		HSURFACE	font;					// buffer to store the font
		HDECOLOR	transColor;				// transparent color for text buffer
		DRect		rect;					// location within buffer to draw from

		char		effects;				// special effect types for the font

		short		XLocs[95];				// X positions for upper case letters
		short		XWidths[94];			// X offsets for upper case letters
};

//*****************************************************************//

class	CoolFontCursor
{
	public:
		//***** Constructors and destructors *****//
		CoolFontCursor();
		~CoolFontCursor()		{	}

		//***** Memory allocation and deallocation functions *****//

		//***** Position and source functions *****//
		void	SetFont(CoolFont *pFont)	{	font = pFont;	}
		void	SetDest(HSURFACE hDest)		{	dest = hDest;	}
		void	SetLoc(short xx, short yy)	{	x = xx; y = yy;	}
		void	GetLoc(short &xx, short &yy){	xx = x; yy = y; }
		short	GetX()						{	return	x;	}
		short	GetY()						{	return	y;	}	
		short	GetHeight()					{	return	font->height;	}
		void	SetJustify(short jj)		{	justify = jj;	}
		void	NewLine()					{	y += font->height;	}
		void	PrevLine()					{	y -= font->height;	}

		//***** Effects functions *****//
		void	SetColor(char type, float r, float g, float b);
		void	SetStartTime(float time)	{	startTime = time;	}
		void	SetTotalTime(float time)	{	totalTime = time;	}

		//***** Drawing functions *****//
		void	Draw(const char *szString)
			{	font->Draw(szString, dest, x, y, justify);	}
		void	Draw(const char *szString, short xx, short yy)
			{	font->Draw(szString, dest, x + xx, y + yy, justify);	}

		void	DrawFormat(const char *szString, short width)
			{	font->DrawFormat(szString, dest, x, y, width);	}

		void	DrawSolid(const char *szString)
			{	font->DrawSolid(szString, dest, x, y, justify, sourceC);	}
		void	DrawSolid(const char *szString, short xx, short yy)
			{	font->DrawSolid(szString, dest, x + xx, y + yy, justify, sourceC);	}

		void	DrawSolid(const char *szString, HDECOLOR color)
			{	font->DrawSolid(szString, dest, x, y, justify, color);	}
		void	DrawSolid(const char *szString, short xx, short yy, HDECOLOR color)
			{	font->DrawSolid(szString, dest, x + xx, y + yy, justify, color);	}

		void	DrawSolidFormat(const char *szString, short width, HDECOLOR color)
			{	font->DrawSolidFormat(szString, dest, x, y, width, color);	}

		void	DrawFormatTimed(const char *szString, short width, DFLOAT ratio, DBOOL fromStart)
			{	font->DrawFormatTimed(szString, dest, x, y, width, ratio, fromStart);	}

	protected:
		//***** Private data members *****//
		CoolFont*	font;					// the client interface
		HSURFACE	dest;					// buffer to draw to

		short		x;						// cursor location X
		short		y;						// cursor location Y
		short		justify;				// current justification setting

		HDECOLOR	sourceC;				// source color for solid drawing
		HDECOLOR	tempC;					// temporary color for gradiant calculations
		HDECOLOR	destC;					// destination color for gradiants

		float		sourceR;
		float		sourceG;
		float		sourceB;

		float		destR;
		float		destG;
		float		destB;

		float		startTime;				// starting time of special effect
		float		totalTime;				// total time to run special effect
};

//*****************************************************************

#endif
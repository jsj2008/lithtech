// ----------------------------------------------------------------------- //
//
// MODULE  : watermark.h
//
// PURPOSE : Draws the LithTech watermark on the screen.
//
// CREATED : 5/20/2002
//
// (c) 2002 LithTech, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //


#ifndef __WATERMARK_H__
#define __WATERMARK_H__


#ifdef COMPILE_JUPITER_EVAL

#include <ltbasetypes.h>
#include <ltbasedefs.h>
#include <ilttexinterface.h>
#include <iltdrawprim.h>



class WaterMark
{
public:

	WaterMark();
	~WaterMark();

	// Load the water mark.
	void		Init();

	// Destroy the water mark.
	void		Term();

	// Display the water mark.
	void		Draw();

private:

	void		CalcPolygon();

private:

	HTEXTURE		m_hWaterMark; 	// water mark texture
	LT_POLYFT4 		m_Poly;			// water mark polygon
};


#endif // COMPILE_JUPITER_EVAL


#endif // __WATERMARK_H__

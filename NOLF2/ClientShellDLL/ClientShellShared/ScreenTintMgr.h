// ----------------------------------------------------------------------- //
//
// MODULE  : ScreenTintMgr.h
//
// PURPOSE : Definition of ScreenTintMgr class
//
// CREATED : 02/02/00
//
// (c) 2000-2002 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __SCREENTINT_H
#define __SCREENTINT_H

#include "ltbasedefs.h"

enum eTintEffect
{
	TINT_DAMAGEFX = 0,
	TINT_SPYVISION,
	TINT_INK,
	TINT_SCREEN_FLASH,
	TINT_CONTAINER,
	NUM_TINT_EFFECTS
};

class CScreenTintMgr
{
public:
	CScreenTintMgr();
	~CScreenTintMgr();

	void Update();

    void Set(eTintEffect eEffect, LTVector *pvColor);
	void Clear(eTintEffect eEffect);

	void ClearAll();

private:
    LTVector m_avTints[NUM_TINT_EFFECTS];
    LTBOOL   m_bChanged;
};


#endif // __SCREENTINT_H
//------------------------------------------------------------------
//
//   MODULE    : SCREENTRAIL.H
//
//   PURPOSE   : Defines screen poly draw trail stuff
//
//   CREATED   : On 7/19/99 At 5:46:08 PM
//
//   COPYRIGHT : (C) 1999 Monolith Productions Inc
//
//------------------------------------------------------------------

#ifndef __SCREENTRAIL_H_
	#define __SCREENTRAIL_H_

	// Includes....

	#include "basefx.h"
	#include "clientfx.h"

	struct TRAIL_SECTION
	{
		LTVector			m_vPos;
		LTVector			m_vTran;
		LTVector			m_vBisector;
		uint8				m_red;
		uint8				m_blue;
		uint8				m_green;
		uint8				m_alpha;
		float				m_fScale;
		float				m_uVal;
	};

	void RenderPolyTrail(ILTClient *pClientDE, 
						 CLinkList<TRAIL_SECTION> *pList, 
						 HOBJECT hCamera, 
						 float fTrailWidth,
						 uint8 r,
						 uint8 g,
						 uint8 b,
						 uint8 a,
						 HTEXTURE hTexture,
						 uint32 dwExtraFlags);
	
#endif
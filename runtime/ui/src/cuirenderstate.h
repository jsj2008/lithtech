//-------------------------------------------------------------------
//
//   MODULE    : CUIRENDERSTATE.H
//
//   PURPOSE   : defines the CUIRenderState Utility Class
//
//   CREATED   : 1/01 
//
//   COPYRIGHT : (C) 2001 LithTech Inc
//
//-------------------------------------------------------------------


#ifndef __CUIRENDERSTATE_H__
#define __CUIRENDERSTATE_H__


#ifndef __ILTTEXINTERFACE_H__
#include "ilttexinterface.h" // for HTEXTURE
#endif


/*!  CUIRenderState class.  This is a utility class designed to set
	 the drawprim render state for various CUI drawers. 

\see interface ILTDrawPrim

Used for: Text and UI.   */

class CUIRenderState
{

public:
	
	static void	SetRenderState(HTEXTURE hTex);

};


#endif __CUIRENDERSTATE_H__

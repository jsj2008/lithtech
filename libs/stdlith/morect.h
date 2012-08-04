//------------------------------------------------------------------
//
//  FILE      : MoRect.h
//
//  PURPOSE   : Defines the CMoRect class.
//
//  CREATED   : November 7 1996
//
//  COPYRIGHT : Microsoft 1996 All Rights Reserved
//
//------------------------------------------------------------------

#ifndef __MORECT_H__
#define __MORECT_H__

#ifndef __STDLITHDEFS_H__
#include "stdlithdefs.h"
#endif

class CMoRect {
public:

    LTBOOL  Init(int x1, int y1, int x2, int y2);
    
    int     Width()     { return right-left; }
    int     Height()    { return bottom-top; }

    void    NormalizeRect();
            

    int     left, top, right, bottom;
};


inline LTBOOL CMoRect::Init(int x1, int y1, int x2, int y2) {
    left = x1;
    top = y1;
    right = x2;
    bottom = y2;

    NormalizeRect();
    return TRUE;
}


inline void CMoRect::NormalizeRect() { 
    int temp;

    if (top > bottom) {
        temp = top;
        top = bottom;
        bottom = temp;
    }

    if (left > right) {
        temp = left;
        left = right;
        right = temp;
    }
}


#endif  // __MORECT_H__


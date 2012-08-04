/*!
A PValue is an RGBA pixel value.  This file defines the PValue type and the functions
used to manipulate it.
*/

#ifndef __LTPVALUE_H__
#define __LTPVALUE_H__

#ifndef __LTBASETYPES_H__
#include "ltbasetypes.h"
#endif


/*!
Generic color value.
*/
typedef uint32 PValue;

/*!
The color planes are indexed by these.
*/
enum
{
    CP_ALPHA =    0,
    CP_RED =      1,
    CP_GREEN =    2,
    CP_BLUE =     3,
    NUM_COLORPLANES = 4,
};

/*!
Mask for each component.
*/
#define PVALUE_ALPHAMASK    0xFF000000
#define PVALUE_REDMASK      0x00FF0000
#define PVALUE_GREENMASK    0x0000FF00
#define PVALUE_BLUEMASK     0x000000FF

#define PLANEOFFSET_ALPHA   3
#define PLANEOFFSET_RED     2
#define PLANEOFFSET_GREEN   1
#define PLANEOFFSET_BLUE    0

/*!
Get the byte offset into a \b PValue for the given color plane.
*/
inline uint32 PValue_GetPlaneOffset(uint32 iPlane) {
    if (iPlane == CP_ALPHA) {
        return 3;
    }
    else if (iPlane == CP_RED) {
        return 2;
    }
    else if (iPlane == CP_GREEN) {
        return 1;
    }
    else {
        return 0;
    }
}

inline PValue PValue_Set(uint32 a, uint32 r, uint32 g, uint32 b) {
    return (a << 24) | (r << 16) | (g << 8) | b;
}

inline uint32 PValue_GetA(PValue value) {
    return value >> 24;
}

inline uint32 PValue_GetR(PValue value) {
    return (value >> 16) & 0xFF;
}

inline uint32 PValue_GetG(PValue value) {
    return (value >> 8) & 0xFF;
}

inline uint32 PValue_GetB(PValue value) {
    return value & 0xFF;
}

inline void PValue_Get(PValue value, uint32 &a, uint32 &r, uint32 &g, uint32 &b) {
    a = PValue_GetA(value);
    r = PValue_GetR(value);
    g = PValue_GetG(value);
    b = PValue_GetB(value);
}

#endif  //! __LTPVALUE_H__

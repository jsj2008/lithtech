// dirtyrect.h

// Defines dirty rectangle regions to allow partial screen updates

#ifndef __DIRTYRECT_H__

// Function prototypes...
void InvalidateRect(LTRect *pRect);
void DirtyRectSwap();
void ClearDirtyRects();



#endif

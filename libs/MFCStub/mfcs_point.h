// mfcs_point.h - MFC Stub CPoint class header

#ifndef __MFCS_POINT_H__
#define __MFCS_POINT_H__

#include "..\mfcstub\mfcs_types.h"

#ifndef _WINDOWS_
	// Make sure POINT is defined...
	typedef struct tagPOINT {
		tagPOINT() {}
		int32 x,y;
	} POINT;
#endif

class CPoint : public POINT
{
public:
	CPoint();
	CPoint(const CPoint &cPt);
	CPoint(const POINT *pPt);
	CPoint(int32 newx, int32 newy);
};

#endif // __MFCS_POINT_H__

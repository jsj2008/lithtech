#ifndef __WORLD_H__
#define __WORLD_H__

#pragma warning( disable : 4786 )
#include <vector>

class CVolume;

typedef std::vector<CVolume*> VOLUMELIST;
typedef	std::vector<POINT> POINTLIST;

struct FPoint
{
	float x;
	float y;
};

typedef std::vector<FPoint> FPOINTLIST;


class CWorld
{
public:

	  CWorld();
	 ~CWorld() {};

	void	AddVolume(long nLeft, long nTop, long nRight, long nBottom);
	void	SetStart(long x, long y) { m_ptStart.x = x; m_ptStart.y = y; }
	void	SetEnd(long x, long y) { m_ptEnd.x = x; m_ptEnd.y = y; }

	void	Draw(HDC hdc);

protected:

	void	DrawGrid(HDC hdc);
	void	DrawVolumes(HDC hdc);
	void	DrawPath(HDC hdc);

	void	BuildPath();
	void	EvaluateCurveC2Interp(const POINTLIST& lstControlPts, FPOINTLIST& lstEvaluatedCurvePoints);

	void	DisplayBezier(const FPoint& p0, const FPoint& p1, 
							const FPoint& p2, const FPoint& p3, 
							FPOINTLIST& lstEvaluatedCurvePoints);
	bool	FlatEnough(const FPoint& p0, const FPoint& p1, 
						const FPoint& p2, const FPoint& p3);
	bool	FlatEnough2(const FPoint& p0, const FPoint& p1, 
						const FPoint& p2, const FPoint& p3);
	float	DotProduct(const FPoint& v1, const FPoint& v2);
	FPoint	Normalize(const FPoint& v);
	float	Magnitude(const FPoint& v);
	double	Len(const FPoint& p0, const FPoint& p1);
	double	LenSqr(const FPoint& p0, const FPoint& p1);
	void	Subdivide(const FPoint& p0, const FPoint& p1,
						const FPoint& p2, const FPoint& p3,
						FPoint& l0, FPoint& l1, FPoint& l2, FPoint& l3,
						FPoint& r0, FPoint& r1, FPoint& r2, FPoint& r3);
	FPoint	linearlyInterpolate(const FPoint& first_point, const FPoint& second_point, double t);

protected:

	VOLUMELIST	m_lstVolumes;

	POINT		m_ptStart;
	POINT		m_ptEnd;

	long		m_nWidth;

	FPOINTLIST	m_lstEvaluatedCurvePoints;
};

extern CWorld* g_pWorld;

#endif
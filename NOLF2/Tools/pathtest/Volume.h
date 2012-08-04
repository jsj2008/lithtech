#ifndef __VOLUME_H__
#define __VOLUME_H__

#include "world.h"


enum ENUMCONNECTIONTYPE
{
	kLeft	= 0x01,
	kRight	= 0x02,
	kTop	= 0x04,
	kBottom	= 0x08,
};


class CVolume
{
public:

	 CVolume(long nLeft, long nTop, long nRight, long nBottom);
	~CVolume() {}

	void	FindConnection();

	void	Draw(HDC hdc);
	void	DrawConnection(HDC hdc);

	void		SetPrev(CVolume* pVolPrev) { m_pVolPrev = pVolPrev; }
	CVolume*	GetPrev() { return m_pVolPrev; }
	void		SetNext(CVolume* pVolNext) { m_pVolNext = pVolNext; }
	CVolume*	GetNext() { return m_pVolNext; }

	POINT				GetConnectMidPt();
	ENUMCONNECTIONTYPE	GetConnectionType() { return m_eConnectionType; }
	POINT				FindNearest( POINT ptLast, long nWidth );

	void		BoundPoints( FPOINTLIST& lstEvaluatedCurvePoints, long iStart, long nWidth );

protected:

	bool		AvoidConnectionEdges(bool& bInConnection, ENUMCONNECTIONTYPE eConnectionType,
								    POINT ptConnect1, POINT ptConnect2,
									FPoint ptLast, FPoint ptCur, 
									FPoint& ptNew, float fThreshold,
									bool bReverse);

protected:

	RECT		m_rRect;
	POINT		m_ptConnect1;
	POINT		m_ptConnect2;
	long		m_nConnectionLen;

	ENUMCONNECTIONTYPE	m_eConnectionType;

	CVolume*	m_pVolPrev; 
	CVolume*	m_pVolNext; 
};

#endif
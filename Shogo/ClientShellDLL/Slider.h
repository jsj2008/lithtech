#ifndef __SLIDER_H
#define __SLIDER_H

#include "iltclient.h"
#include "clientUtilities.h"

class ILTClient;

class CSlider
{
public:
	
	CSlider()						{ m_pClientDE = LTNULL; m_bSelected = LTFALSE; m_bEnabled = LTFALSE; m_nWidth = 0; m_nHeight = 0; m_nStops = 0; 
									  m_nPos = 0; m_hBarNormal = LTNULL; m_hBarDisabled = LTNULL; m_hThumbNormal = LTNULL; m_hThumbSelected = LTNULL; 
									  m_hThumbDisabled = LTNULL; m_hThumbFillNormal = LTNULL; m_hThumbFillSelected = LTNULL; m_hThumbFillDisabled = LTNULL; 
									  m_hSlider = LTNULL; }
	~CSlider();

	LTBOOL	Init (ILTClient* pClientDE, int nWidth, int nStops);
	void	Term();
	void	Draw (HSURFACE hDest, int x, int y);

	void	SetSelected (LTBOOL bSelected = LTTRUE)	{ if (m_bSelected == bSelected) return; m_bSelected = bSelected; UpdateSlider(); }
	void	SetEnabled (LTBOOL bEnabled = LTTRUE)		{ if (m_bEnabled == bEnabled) return; m_bEnabled = bEnabled; UpdateSlider(); }
	int		GetPos()								{ return m_nPos; }
	void	SetPos (int nPos)						{ if (nPos >= 0 && nPos < m_nStops) m_nPos = nPos; UpdateSlider(); }
	int		GetWidth()								{ return m_nWidth; }
	int		GetHeight()								{ return m_nHeight; }
	int		GetMin()								{ return 0; }
	int		GetMax()								{ return m_nStops - 1; }
	LTBOOL	IncPos()								{ if (m_nPos < m_nStops - 1) { SetPos (m_nPos + 1); return LTTRUE; } return LTFALSE; }
	LTBOOL	DecPos()								{ if (m_nPos > 0) { SetPos (m_nPos - 1); return LTTRUE; } return LTFALSE; }

protected:

	void	GetThumbPos (int* pX, int* pY);
	void	UpdateSlider();

	ILTClient*	m_pClientDE;
	LTBOOL		m_bSelected;
	LTBOOL		m_bEnabled;
	int			m_nWidth;
	int			m_nHeight;
	int			m_nStops;
	int			m_nPos;
	HSURFACE	m_hBarNormal;
	HSURFACE	m_hBarDisabled;
	HSURFACE	m_hThumbNormal;
	HSURFACE	m_hThumbSelected;
	HSURFACE	m_hThumbDisabled;
	HSURFACE	m_hThumbFillNormal;
	HSURFACE	m_hThumbFillSelected;
	HSURFACE	m_hThumbFillDisabled;
	HSURFACE	m_hSlider;
	CSize		m_szThumb;
};

#endif

#ifndef __LMLIGHT_H__
#define __LMLIGHT_H__

class CBaseEditObj;
class CEditBrush;

class CLMLight
{
public:

	CLMLight() :
		m_pSrcObject(NULL),
		m_pLastHitBrush(NULL),
		m_pNext(NULL),
		m_pPrev(NULL)
	{
	}

	//the object that this light originally came from
	CBaseEditObj*	m_pSrcObject;

	//if it is omni or not (if it isn't, it is a directional spotlight
	bool			m_bIsOmni;

	//dimensions of the light
	CReal			m_fRadius;
	CReal			m_fRadiusSqr;

	//position and orientation
	LTVector		m_vPos;
	LTVector		m_vDir;

	//directional light's cosine of the half angle
	CReal			m_fCosHalfAngle;

	//directional light's tangent of the half angle (for culling purposes)
	CReal			m_fTanHalfAngle;

	//color of inner part of light
	LTVector		m_vInnerColor;

	//color of outer part of light
	LTVector		m_vOuterColor;

	//bright scale of light
	float			m_fBrightScale;

	//the coefficients
	float			m_fCoA;
	float			m_fCoB;
	float			m_fCoC;

	//the brush that was last hit while trying to take a sample from
	//this light. This aids in ray casting coherency greatly.
	CEditBrush*		m_pLastHitBrush;

	//linked list info
	CLMLight*		m_pNext;
	CLMLight*		m_pPrev;

private:

};

#endif


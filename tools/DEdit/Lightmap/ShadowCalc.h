#ifndef __SHADOWCALC_H__
#define __SHADOWCALC_H__

class CEditRegion;
class CEditBrush;
class CLMLight;

class CShadowCalc
{
public:

	CShadowCalc();
	~CShadowCalc();

	//sets the region that this shadow calculator is associated with
	void	SetRegion(CEditRegion* pRegion);

	//specify the amount of light that is allowed to leak through walls.
	void	SetLightLeakAmount(uint32 nAmount);

	//retreives the amount of light that is allowed to leak through walls.
	uint32	GetLightLeakAmount() const;

	//determines if a segment traveling from the starting point to the ending
	//point is intersected by any geometry in the region. This takes a light pointer
	//so that it can do per light coherency
	bool	IsSegmentBlocked(const LTVector& vStart, const LTVector& vEnd, CLMLight* pLight);

	//similar to above, but is given a unit direction along with the length of the
	//segment
	bool	IsSegmentBlocked(const LTVector& vStart, const LTVector& vDir, float fLen, CLMLight* pLight);

private:

	//the number of units to allow light leakign to prevent dark corners
	uint32			m_nLightLeakAmount;

	//the region in which the segments are intersected
	CEditRegion*	m_pRegion;
};


#endif


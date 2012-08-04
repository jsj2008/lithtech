#ifndef __LMSAMPLEGEN_H__
#define __LMSAMPLEGEN_H__

#ifndef __SHADOWCALC_H__
#	include "ShadowCalc.h"
#endif

#ifndef __SORTABLEHOLDER_H__
#	include "SortableHolder.h"
#endif

class CLMLight;
class CBaseEditObj;
class CEditBrush;

class CLMSampleGen
{
public:

	CLMSampleGen();
	~CLMSampleGen();

	//inserts a light into the list of lights that can contribute to the lightmaps
	bool		UpdateLightList(CMoArray<CBaseEditObj*> *pObjList);


	//calculates a light value for the specified point and normal
	void		CalcSample(	const LTVector& vPos, const LTVector& vNormal, 
							const CLightHolderOptions& Options,
							uint8& nR, uint8& nG, uint8& nB);

	//gets the specified LMLight given the object light
	CLMLight*	GetLight(CBaseEditObj* pLight);

	//converts a light object to a lightmap light object
	void		ConvertLight(CBaseEditObj* pSrc, CLMLight *pDest, bool bIsOmni);

	//Adds a light onto the list. The list will be in charge of deleting and maintaining
	//the light
	void		AddLight(CLMLight* pLight);

	//removes a light from the list. This will also delete the light
	void		DeleteLight(CLMLight* pLight);

	//clears the entire light list
	void		ClearLightList();

	//gets the first light. This should not be held on to, since it can change
	const CLMLight* GetHeadLight() const		{return m_pHead;}

	//determines if lambertian is being used
	bool		IsLambertian() const			{return m_bLambertian;}
	void		SetLambertian(bool bVal)		{m_bLambertian = bVal;}

	//determines if shadows are to be cast
	bool		IsShadows() const				{return m_bShadows;}
	void		SetShadows(bool bVal)			{m_bShadows = bVal;}

	//the ambient light in the world...ranges from 0..255
	LTVector	m_vAmbient;

	//the shadow calculator
	CShadowCalc	m_ShadowCalc;

private:

	//use lambertian
	bool		m_bLambertian;

	//cast shadows
	bool		m_bShadows;

	//the list of lights
	CLMLight*		m_pHead;

};

#endif


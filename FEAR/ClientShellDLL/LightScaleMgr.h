#ifndef __LIGHTSCALEMGR_H
#define __LIGHTSCALEMGR_H

#include "iltclient.h"

class CLightScaleMgr
{
public:

	//the different types of light scales we support, the order that they
	//are listed in here is the priority, with the first items having
	//highest priority
	enum ELightScaleType
	{
		eLightScaleDamage,
		eLightScaleEnvironment,

		//this value must come last
		eNumLightScaleTypes
	};

    CLightScaleMgr();
	~CLightScaleMgr();

	//called to reset all the items in the light scale manager
    bool      Init();
	void		Term();

	void		Enable() const;
	void		Disable() const;

	//sets the light scale for a particular type
    void        SetLightScale (const LTVector& vColor, ELightScaleType eType);

	//clears the light scale so any light scales of lower priority can be displayed
    void        ClearLightScale (ELightScaleType eType);

	//clears all light scales
    void        ClearAllLightScales ();

private:

	//installs the highest priority active light scale
	void		SetLightScale() const;

	//determines if the particular light scale type is enabled
	bool		IsEnabled(ELightScaleType eType) const;

	//the current values of our light scales
	LTVector	m_vLightScales[eNumLightScaleTypes];

};

#endif
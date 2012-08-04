#ifndef __SORTABLEHOLDER_H__
#define __SORTABLEHOLDER_H__

struct CLightHolderOptions
{
	//any additional ambient light that should be added on for this holder's lighting
	uint8	m_nAmbientR;
	uint8	m_nAmbientG;
	uint8	m_nAmbientB;

	//whether or not shadows should cast on this object
	bool	m_bReceiveShadows;

	//whether light should be added onto this surface
	bool	m_bReceiveLight;
};


class ISortableHolder
{
public:

	ISortableHolder()				{}
	virtual ~ISortableHolder()		{}

	//called to calculate a score that is based upon a viewer position and direction
	//the lower the score the better
	virtual uint32	CalcScore(	const LTVector& vCameraPos, 
								const LTVector& vCameraDir)							= 0;

	//called to obtain the lighting options for this holder
	virtual void	GetLightOptions( CLightHolderOptions& Options)					= 0;

private:

};

#endif

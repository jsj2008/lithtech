#ifndef __RELEVANTLIGHTLIST_H__
#define __RELEVANTLIGHTLIST_H__

#ifndef __RENDERLIGHT_H__
#	include "renderlight.h"
#endif

// DEFINES
#define			MAX_LIGHTS_SUPPORTED_BY_D3D					8


//the light list in a format more easily installed on the device quickly
class CDeviceLightList;

class CRelevantLightList
{
public:

	CRelevantLightList();
	~CRelevantLightList();

	//sets the position of the object that this light list applies to (this is needed so that
	//lights can be properly setup for it). This must be set before any lights are added.
	void				SetObjectPos(const LTVector& vPos);

	//sets the dimensions of the object that this light list applies to (this is needed so that
	//lights can be properly setup for it). This must be set before any lights are added.
	void				SetObjectDims(const LTVector& vDims);

	//given a light, this will add it into the relevant light list if there is room,
	//or find its contribution and add it to the ambient light. It will also take a sample
	//at the object position and handle the appropriate partial conversion to ambient
	void				InsertLight(const CRenderLight& Light, float fConvertToAmbient);

	//this will retreive the number of active lights in this list
	uint32				GetNumLights() const;

	//sets the maximum number of lights that can be in the set. This will automatically
	//clip this to the largest amount that D3D can handle
	void				SetMaxLights(uint32 nMaxLights);

	//clears all lights out of the list
	void				ClearList();

	//adds the specified amount of light to the ambient light
	void				AddAmbient(const LTVector& vAmbient);

	//This will take all the lights in the relevant light list and convert them into a device
	//light list which doesn't have any modifying capabilities and cannot be queryed, but is
	//fast to setup on the device
	void				CreateDeviceLightList(CDeviceLightList& DevLightList) const;

	//this gets a light at a specific location
	const CRenderLight&	GetLight(uint32 nIndex) const;

	//gets the sample of light at the position of the object
	const LTVector&		GetLightSample(uint32 nIndex) const;

	//this will multiply all the lights by the specified color
	void				MultiplyLightColors(const LTVector& vColorScale);


private:

	//information about a single scored light, such as what it's score is
	//and where in the list it is located
	struct SLightScore
	{
		float		m_fScore;
		LTVector	m_vSample;
		uint32		m_nIndex;
	};
	
	//the position of the object being lit
	LTVector		m_vObjectPos;

	//the dimensions of the object being lit
	LTVector		m_vObjectDims;

	//the list of actual light objects
	CRenderLight	m_Lights[MAX_LIGHTS_SUPPORTED_BY_D3D];

	//the list of lighting scores
	SLightScore		m_Scores[MAX_LIGHTS_SUPPORTED_BY_D3D];

	//the ambient light
	LTVector		m_vAmbient;

	//the number of lights we have in the list
	uint16			m_nNumLights;

	//the maximum number of lights that can be added
	uint16			m_nMaxLights;

};

#endif

#ifndef __DEVICELIGHTLIST_H__
#define __DEVICELIGHTLIST_H__

#ifndef __RELEVANTLIGHTLIST_H__
#	include "relevantlightlist.h"
#endif


class CDeviceLightList
{
public:

	CDeviceLightList();
	CDeviceLightList(const CDeviceLightList& rhs);
	~CDeviceLightList();

	//clears all lights from this list
	void		ClearLights();

	//gets a light for writing to from the list. This does not do a copy due to the large
	//size of the light structure and instead returns the light that can be used to put in.
	//This returns NULL if no room is available
	D3DLIGHT9*	AddNewLight();

	//Installs the light list into the device
	void			InstallLightList() const;

	//sets the ambient color
	void			SetAmbientColor(const LTVector& vAmbient);

	//adds to the ambient color and then scales all light colors by the specified amount
	void			AddAmbientAndScale(const LTVector& vAmbient, const LTVector& vColorScales);

	//given another light list, this will determine if it is the same. Note that this is not a full
	//check, it is meant to serve as a quick check for situations such as fully lit
	inline bool		IsSameAs(const CDeviceLightList* pRHS) const;

private:

	D3DLIGHT9	m_Lights[MAX_LIGHTS_SUPPORTED_BY_D3D];
	uint32		m_nAmbient;
	uint32		m_nNumLights;
};


//--------------------------------------------------------------
// Inlines

inline bool CDeviceLightList::IsSameAs(const CDeviceLightList* pRHS) const
{
	//see if this is the same light
	if(pRHS == this)
		return true;

	//this is a different light, so lets just do a quick check to see if we both have
	//no lights and the ambients are the same (common for pitch black, or fully bright
	//models)
	if(m_nNumLights || pRHS->m_nNumLights)
		return false;

	if(m_nAmbient != pRHS->m_nAmbient)
		return false;

	//we are the same
	return true;
}

#endif

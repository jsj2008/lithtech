#include "precompile.h"
#include "d3d_renderstatemgr.h"
#include "devicelightlist.h"

CDeviceLightList::CDeviceLightList() :
	m_nAmbient(0),
	m_nNumLights(0)
{
}

CDeviceLightList::CDeviceLightList(const CDeviceLightList& rhs) :
	m_nAmbient(rhs.m_nAmbient),
	m_nNumLights(rhs.m_nNumLights)
{
	for(uint32 nCurrLight = 0; nCurrLight < rhs.m_nNumLights; nCurrLight++)
		m_Lights[nCurrLight] = rhs.m_Lights[nCurrLight];
}


CDeviceLightList::~CDeviceLightList()
{
}

//clears all lights from this list
void CDeviceLightList::ClearLights()
{
	m_nAmbient = 0;
	m_nNumLights = 0;
}

//gets a light for writing to from the list. This does not do a copy due to the large
//size of the light structure and instead returns the light that can be used to put in.
//This returns NULL if no room is available
D3DLIGHT9* CDeviceLightList::AddNewLight()
{
	if(m_nNumLights >= MAX_LIGHTS_SUPPORTED_BY_D3D)
		return NULL;

	m_nNumLights++;

	return &m_Lights[m_nNumLights - 1];
}

//Installs the light list into the device
void CDeviceLightList::InstallLightList() const
{
	//setup the ambient light
	PD3DDEVICE->SetRenderState(D3DRS_AMBIENT, m_nAmbient);

	// Add the directional/point lights...
	uint32 nCurrLight = 0;

	for (; nCurrLight < m_nNumLights; nCurrLight++)
	{
		g_RenderStateMgr.LightEnable(nCurrLight, true);
		g_RenderStateMgr.SetLight(nCurrLight, &m_Lights[nCurrLight]);
	}

 	for (; nCurrLight < MAX_LIGHTS_SUPPORTED_BY_D3D; nCurrLight++)
	{
		// Disable the rest of the lights...
		g_RenderStateMgr.LightEnable(nCurrLight, false);
	}
}

//sets the ambient color
void CDeviceLightList::SetAmbientColor(const LTVector& vAmbient)
{
	long r = (long)(LTMIN(255.0f, vAmbient.x));
	long g = (long)(LTMIN(255.0f, vAmbient.y));
	long b = (long)(LTMIN(255.0f, vAmbient.z));
	m_nAmbient = D3DRGB_255(r, g, b);
}

//adds to the ambient color
void CDeviceLightList::AddAmbientAndScale(const LTVector& vAmbient, const LTVector& vColorScales)
{
	//add the ambient and scale it by the color scale
	SetAmbientColor(LTVector((RGBA_GETR(m_nAmbient) + vAmbient.x) * vColorScales.x,
							 (RGBA_GETG(m_nAmbient) + vAmbient.y) * vColorScales.y,
							 (RGBA_GETB(m_nAmbient) + vAmbient.z) * vColorScales.z));

	//scale all of our actual lights to match as well
	for(uint32 nCurrLight = 0; nCurrLight < m_nNumLights; nCurrLight++)
	{
		//we only need to worry about the diffuse since nothing else is set up
		m_Lights[nCurrLight].Diffuse.r *= vColorScales.x;
		m_Lights[nCurrLight].Diffuse.g *= vColorScales.y;
		m_Lights[nCurrLight].Diffuse.b *= vColorScales.z;
	}
}


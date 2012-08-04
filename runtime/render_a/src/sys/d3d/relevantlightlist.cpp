#include "precompile.h"
#include "d3d_renderstatemgr.h"
#include "relevantlightlist.h"
#include "devicelightlist.h"

CRelevantLightList::CRelevantLightList() :
	m_nNumLights(0),
	m_nMaxLights(MAX_LIGHTS_SUPPORTED_BY_D3D),
	m_vAmbient(0, 0, 0),
	m_vObjectPos(0, 0, 0),
	m_vObjectDims(0, 0, 0)
{
}

CRelevantLightList::~CRelevantLightList()
{
}

//given a light, this will add it into the relevant light list if there is room,
//or find its contribution and add it to the ambient light
void CRelevantLightList::InsertLight(const CRenderLight& Light, float fConvertToAmbient)
{
	//figure out the largest lighting component of the light
	LTVector vSample = Light.GetLightSample(m_vObjectPos);
	float fScore = LTMAX(vSample.x, LTMAX(vSample.y, vSample.z));

	//we need to scale the score by the amount that we are converting to ambient though, 
	//so we don't waste slots on lights that are going to be largely ambient anyway
	fScore *= 1.0f - fConvertToAmbient;

	//see where this light fits into our list
	uint32 nLightIndex = 0;
	for(; nLightIndex < m_nNumLights; nLightIndex++)
	{
		//see if this light is stronger than the existing one..
		if(fScore > m_Scores[nLightIndex].m_fScore)
		{
			//it is, so use this index
			break;
		}
	}

	//we now have the index where we want to put this light, see if we have room
	if(nLightIndex >= m_nMaxLights)
	{
		//no room, just add it to the ambient
		m_vAmbient += vSample;
		return;
	}

	//ok, we have room, but we first need to shift all the elements beyond us down
	//one, possibly pushing one out of the list...but before we do that we need to
	//figure out the light we will use
	uint32 nUseLight = m_nNumLights;

	//however, if our list is full, use the last item's light since it will be going away
	if(m_nNumLights >= m_nMaxLights)
	{
		nUseLight = m_Scores[m_nNumLights - 1].m_nIndex;

		//but we also need to make sure to add that light's contribution to the ambient light
		m_vAmbient += m_Lights[nUseLight].GetLightSample(m_vObjectPos);
	}

	//adjust our lighting count
	if(m_nNumLights < m_nMaxLights)
		m_nNumLights++;

	//alright, so now we just need to shift
	for(uint32 nCurrShift = m_nNumLights - 1; nCurrShift > nLightIndex; nCurrShift--)
	{
		m_Scores[nCurrShift] = m_Scores[nCurrShift - 1];
	}

	//and now insert our light
	m_Scores[nLightIndex].m_fScore	= fScore;
	m_Scores[nLightIndex].m_vSample = vSample;
	m_Scores[nLightIndex].m_nIndex	= nUseLight;

	//all done, fill out the light
	m_Lights[nUseLight] = Light;

	//also scale the color by the amount that we converted to ambient
	m_Lights[nUseLight].ScaleColor(1.0f - fConvertToAmbient);

	//add in the ambient contribution
	m_vAmbient += fConvertToAmbient * vSample;
}

//this will retreive the number of active lights in this list
uint32 CRelevantLightList::GetNumLights() const
{
	return m_nNumLights;
}

//sets the maximum number of lights that can be in the set. This will automatically
//clip this to the largest amount that D3D can handle
void CRelevantLightList::SetMaxLights(uint32 nMaxLights)
{
	m_nMaxLights = LTMIN(nMaxLights, MAX_LIGHTS_SUPPORTED_BY_D3D);

	//make sure that we don't have any lights beyond that
	m_nNumLights = LTMIN(m_nNumLights, m_nMaxLights);
}

//clears all lights out of the list
void CRelevantLightList::ClearList()
{
	m_vAmbient.Init(0, 0, 0);
	m_nNumLights = 0;
}

//this gets a light at a specific location
const CRenderLight&	CRelevantLightList::GetLight(uint32 nIndex) const
{
	//bounds check
	assert(nIndex < m_nNumLights);
	return m_Lights[m_Scores[nIndex].m_nIndex];
}


//adds the specified amount of light to the ambient light
void CRelevantLightList::AddAmbient(const LTVector& vAmbient)
{
	m_vAmbient += vAmbient;
}

//This will take all the lights in the relevant light list and convert them into a device
//light list which doesn't have any modifying capabilities and cannot be queryed, but is
//fast to setup on the device
void CRelevantLightList::CreateDeviceLightList(CDeviceLightList& DevLightList) const
{
	//clear out any old lights from the list
	DevLightList.ClearLights();

	for(uint32 nCurrLight = 0; nCurrLight < m_nNumLights; nCurrLight++)
	{
		D3DLIGHT9* pDevLight = DevLightList.AddNewLight();

		if(!pDevLight)
		{
			//no more room in the light list
			break;
		}

		//we now need to convert the light
		m_Lights[nCurrLight].CreateDeviceLight(m_vObjectPos, m_vObjectDims, *pDevLight);
	}

	//setup the ambient
	DevLightList.SetAmbientColor(m_vAmbient);
}

//sets the position of the object that this light list applies to (this is needed so that
//lights can be properly setup for it). This must be set before any lights are added.
void CRelevantLightList::SetObjectPos(const LTVector& vPos)
{
	m_vObjectPos = vPos;
}

//sets the dimensions of the object that this light list applies to (this is needed so that
//lights can be properly setup for it). This must be set before any lights are added.
void CRelevantLightList::SetObjectDims(const LTVector& vDims)
{
	m_vObjectDims = vDims;
}


//gets the sample of light at the position of the object
const LTVector& CRelevantLightList::GetLightSample(uint32 nIndex) const
{
	//bounds check
	assert(nIndex < m_nNumLights);
	return m_Scores[nIndex].m_vSample;
}

//this will multiply all the lights by the specified color
void CRelevantLightList::MultiplyLightColors(const LTVector& vColorScale)
{
	for(uint32 nCurrLight = 0; nCurrLight < m_nNumLights; nCurrLight++)
	{
		//we just want to modify the light color
		m_Lights[nCurrLight].ScaleColor(vColorScale);
	}

	//we also need to scale our ambient
	VEC_MUL(m_vAmbient, m_vAmbient, vColorScale);
}

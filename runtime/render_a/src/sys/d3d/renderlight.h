#ifndef __RENDERLIGHT_H__
#define __RENDERLIGHT_H__

//include this for the light attenuation type
#ifndef __DE_WORLD_H__
#	include "de_world.h"
#endif

class CRenderLight
{
public:

	//-------------------------
	//class types
	//-------------------------

	//the different types of lights available
	enum ELightType
	{
		eLight_Invalid,
		eLight_Dir,
		eLight_Point,
		eLight_Spot
	};


	//-------------------------
	//constructors
	//-------------------------
	CRenderLight();
	~CRenderLight();

	//-------------------------
	//light creation
	//-------------------------
	//sets up the light to be a directional light
	void SetupDirLight(const LTVector& vDir, const LTVector& vColor, uint32 nFlags = 0);

	//sets up the light to be a point light source
	void SetupPointLight(const LTVector& vLightPos,
						 const LTVector& vColor, const LTVector& vAttCoeff,
						 float fRadius, ELightAttenuationType eAttenuation, 
						 uint32 nFlags = 0);

	//sets up the light to be a spotlight
	void SetupSpotLight( const LTVector& vLightPos,
						 const LTVector& vLightDir, const LTVector& vColor, 
						 const LTVector& vAttCoeff,
						 float fRadius, float fCosFOV,
						 ELightAttenuationType eAttenuation, 
						 uint32 nFlags = 0);

	//-------------------------
	//light sampling
	//-------------------------
	//given a point, this will determine the light intensity of the point
	LTVector GetLightSample(const LTVector& vPos) const;

	//-------------------------
	//light instantiation
	//-------------------------
	//this will install the currently setup light into the device in the specified
	//slot
	void InstallLight(uint32 nSlot, const LTVector& vObjectPos, const LTVector& vObjectDims) const;

	//this will convert the specified light to a light in the device format
	void CreateDeviceLight(const LTVector& vObjectPos, const LTVector& vObjectDims, D3DLIGHT9& DevLight) const;


	//-------------------------
	//accessors
	//-------------------------

	ELightType				GetType() const			{ return m_eLightType; }
	const LTVector&			GetPos() const			{ return m_vLightPos; }
	const LTVector&			GetDir() const			{ return m_vLightDir; }
	const LTVector&			GetColor() const		{ return m_vColor; }
	const LTVector&			GetAttCoeff() const		{ return m_vAttCoeff; }
	float					GetRadius() const		{ return m_fRadius; }
	float					GetRadiusSqr() const	{ return m_fRadiusSqr; }
	float					GetCosFOV() const		{ return m_fCosFOV; }
	ELightAttenuationType	GetAttType() const		{ return m_eAttenuation; }
	uint32					GetFlags() const		{ return m_nFlags; }


	void	ScaleColor(float fScale)				{ m_vColor *= fScale; }
	void	ScaleColor(const LTVector& vScale)		{ VEC_MUL(m_vColor, m_vColor, vScale); }



	// Note: This checks only certain params of the struct
	bool operator == (const CRenderLight &rhs) const 
	{
		return ((GetPos() == rhs.GetPos()) && 
				(GetRadius() == rhs.GetRadius()) && 
				(GetColor() == rhs.GetColor())); 
	}


private:

	//installers for each specific light type

	//sets up the dirlight in the specified slot
	void CreateDirLight(D3DLIGHT9& DevLight) const;

	//handles setting up the point light source
	void CreatePointLight(const LTVector& vObjectPos, const LTVector& vObjectDims, D3DLIGHT9& DevLight) const;
	void CreatePointLightD3D(const LTVector& vObjectPos, const LTVector& vObjectDims, D3DLIGHT9& DevLight) const;
	void CreatePointLightQuartic(const LTVector& vObjectPos, const LTVector& vObjectDims, D3DLIGHT9& DevLight) const;
	void CreatePointLightLinear(const LTVector& vObjectPos, const LTVector& vObjectDims, D3DLIGHT9& DevLight) const;

	//handles setting up the point light source
	void CreateSpotLight(const LTVector& vObjectPos, const LTVector& vObjectDims, D3DLIGHT9& DevLight) const;

	//this form of installing will take a sample at the 
	//specified viewer position and will create a dirlight of that color facing
	//in the direction from the light to the viewer
	void CreateSampledDirLight(const LTVector& vObjectPos, D3DLIGHT9& DevLight) const;

	//this form of installing will take a sample at the 
	//specified viewer position and at each corner of the dimension box, and setup a directional
	//light appropriately
	void CreateMultiSampledDirLight(const LTVector& vObjectPos, const LTVector& vObjectDims, D3DLIGHT9& DevLight) const;

	//the type of this light
	ELightType				m_eLightType;

	//the attenuation type
	ELightAttenuationType	m_eAttenuation;

	//the position of the light
	LTVector				m_vLightPos;

	//the direction of the light (normalized)
	LTVector				m_vLightDir;

	//the color of the light
	LTVector				m_vColor;

	//Attenuation coefficients
	LTVector				m_vAttCoeff;

	//the maximum radius of this light
	float					m_fRadius;

	//the square of the above value
	float					m_fRadiusSqr;

	//the field of view of spotlights
	float					m_fCosFOV;

	//the flags for this light
	uint32					m_nFlags;

};

#endif


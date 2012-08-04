// ----------------------------------------------------------------------- //
//
// MODULE  :	LightBase.h
//
// PURPOSE :	Provides the definition for the light base class which
//				is a base class for all light objects so that they can
//				receive and process the same messages.
//
// CREATED :	2/21/03
//
// (c) 2003 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef __LIGHTBASE_H__
#define __LIGHTBASE_H__

#ifndef __GAMEBASE_H__
#	include "GameBase.h"
#endif

#ifndef __ENGINELODPROPUTIL_H__
#	include "EngineLODPropUtil.h"
#endif

LINKTO_MODULE( LightBase );

//plugin class that can be used by derived lighting classes that will fill in the LOD strings
//for any properties named 'LightLOD', 'WorldShadowsLOD', and 'ObjectShadowsLOD'
class LightBase_Plugin: 
	public IObjectPlugin
{
public:

	virtual LTRESULT PreHook_EditStringList(
		const char* /*szRezPath*/,				
		const char* szPropName,				
		char** aszStrings,					
		uint32* pcStrings,	
		const uint32 cMaxStrings,		
		const uint32 cMaxStringLength)		
	{
		//handle setting up any LOD properties on the light
		if(	(LTStrEquals(szPropName, "LightLOD")) ||
			(LTStrEquals(szPropName, "WorldShadowsLOD")) ||
			(LTStrEquals(szPropName, "ObjectShadowsLOD")))
		{
			return CEngineLODPropUtil::AddLODStrings(aszStrings, pcStrings, cMaxStrings, cMaxStringLength);
		}
		return LT_UNSUPPORTED;
	}
};


class LightBase :
	public GameBase
{
public:

	LightBase();
	virtual ~LightBase();

protected:

	//virtual function that derived classes must override to handle loading in of
	//property data
	virtual void	ReadLightProperties(const GenericPropList *pProps);
	//virtual function that derived classes may override to change the creation struct before the object is created
	virtual void	PostReadProp(ObjectCreateStruct *pStruct);

	//handles events sent from the engine. These are primarily messages
	//associated with saving and loading
	uint32 EngineMessageFn(uint32 messageID, void *pData, float fData);

	//handles saving and loading all data to a message stream
	void Save(ILTMessage_Write *pMsg, uint32 dwSaveFlags);
	void Load(ILTMessage_Read *pMsg, uint32 dwLoadFlags);

	//called to setup all variables on the engine object once it has been
	//created
	void SetupEngineLight();

	//the color of the light
	LTVector			m_vColor;

	//the translucent color of the light
	LTVector			m_vTranslucentColor;

	//the specular color of the light
	LTVector			m_vSpecularColor;

	//the radius of the light
	float				m_fLightRadius;

	//the intensity of the light
	float				m_fIntensityScale;

	//the type of this light
	EEngineLightType	m_eLightType;

	//the spot projector parameters
	float				m_fSpotFovX;
	float				m_fSpotFovY;
	float				m_fSpotNearClip;

	//the directional light parameters
	LTVector			m_vDirectionalDims;

	//the LOD settings for this light
	EEngineLOD			m_eLightLOD;
	EEngineLOD			m_eWorldShadowsLOD;
	EEngineLOD			m_eObjectShadowsLOD;

	//the name of the texture associated with the light
	std::string			m_sLightTexture;

	//the name of the attenuation texture associated with the light
	std::string			m_sLightAttenuationTexture;

	// Message Handlers...
	DECLARE_MSG_HANDLER( LightBase, HandleDimsMsg );
	DECLARE_MSG_HANDLER( LightBase, HandleColorMsg );
	DECLARE_MSG_HANDLER( LightBase, HandleRadiusMsg );
	DECLARE_MSG_HANDLER( LightBase, HandleIntensityMsg );
	DECLARE_MSG_HANDLER( LightBase, HandleTextureMsg );
	DECLARE_MSG_HANDLER( LightBase, HandleSpotFOVMsg );
	DECLARE_MSG_HANDLER( LightBase, HandleScaleAlphaMsg );
};

#endif

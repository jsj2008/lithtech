// ----------------------------------------------------------------------- //
//
// MODULE  : RenderTarget.h
//
// PURPOSE : This defines the render target object which is used to handle
//			 rendering scenes to a texture and applying those rendered scenes
//			 to materials in the world 
//
// CREATED : 8/4/03
//
// (c) 2003 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __RENDERTARGET_H__
#define __RENDERTARGET_H__

LINKTO_MODULE( RenderTarget );

#include "GameBase.h"

#ifndef __ENGINELODPROPUTIL_H__
#	include "EngineLODPropUtil.h"
#endif

//plugin class that can be used by derived lighting classes that will fill in the LOD strings
class RenderTarget_Plugin: 
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
		if(LTStrEquals(szPropName, "LOD"))
		{
			return CEngineLODPropUtil::AddLODStrings(aszStrings, pcStrings, cMaxStrings, cMaxStringLength);
		}
		return LT_UNSUPPORTED;
	}
};


class RenderTarget : 
	public GameBase
{
public:

	RenderTarget();
	~RenderTarget();

private:

	//handle engine messages
	uint32	EngineMessageFn(uint32 messageID, void *pData, float fData);

	//given a message, this will fill it out with the polygrid data
	void	CreateSFXMessage(ILTMessage_Write& cMsg);

	//handle reading in of properties from the level
	void	ReadProp(const GenericPropList *pProps);

	//update the creation structure as needed when creating the object
	void	PostReadProp(ObjectCreateStruct *pStruct);
	
	//handle updates
	void	Update();

	//update the special effect message assocaited with this object
	void	UpdateSpecialFXMessage();

	//handle serialization
	void	Save(ILTMessage_Write *pMsg, uint32 dwSaveFlags);
	void	Load(ILTMessage_Read *pMsg, uint32 dwLoadFlags);

	//flag indicating whether or not we were created from a save game. Not serialized.
	bool		m_bCreatedFromSave;

	//the render target group object that we refer to for our surface. Not serialized
	std::string	m_sRenderTargetGroup;

	//properties of the render target

	//the ID of the render target group
	uint32		m_nRenderTargetGroupID;

	//the LOD associated with this render target
	uint32		m_nRenderTargetLOD;

	//the update frequency of this render target (in frames, 0 means skip no frames, so render each frame)
	uint32		m_nUpdateFrequency;

	//the update offset of this render target to allow for staggering (in frames)
	uint32		m_nUpdateOffset;

	//the field of view for the camera
	LTVector2	m_vFOV;

	//flag indicating whether or not this is a mirror render target
	bool		m_bMirror;

	//flag indicating whether or not this is a refraction render target
	bool		m_bRefraction;

	//biasing distance for the clip plane when rendering a refraction target
	float		m_fRefractionClipPlaneBias;

	//the material that this applies to
	std::string	m_sMaterial;

	//the parameter within the material that this applies to
	std::string m_sParam;
};

#endif // __RENDERTARGET_H__

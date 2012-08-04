// ----------------------------------------------------------------------- //
//
// MODULE  : RenderTargetGroup.h
//
// PURPOSE : This defines the render target group object which is used to 
//			 specify information about a single render target surface, of
//			 which other render target objects can refer to in order to
//			 have a surface of which to render to.
//
// CREATED : 5/11/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __RENDERTARGETGROUP_H__
#define __RENDERTARGETGROUP_H__

LINKTO_MODULE( RenderTargetGroup );

#include "GameBase.h"

class RenderTargetGroup : 
	public GameBase
{
public:

	RenderTargetGroup();
	~RenderTargetGroup();

	//called to get the unique integer identifier associated with this group. This
	//is guaranteed to be less than 0xFFFF
	uint32	GetUniqueGroupID() const		{ return m_nUniqueGroupID; }

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

	//properties of the render target

	//a unique ID associated with this group so that a link can be established on the 
	//client
	uint32		m_nUniqueGroupID;

	//render target dimensions
	LTVector2n	m_nDimensions[3];

	//flag indicating whether or not this is a cubic render target
	bool		m_bCubeMap;

	//flags indicating which rendering features are supported
	bool		m_bMipMap;
	bool		m_bFogVolumes;
	bool		m_bLastFrameEffects;
	bool		m_bCurrFrameEffects;
};

#endif // __RENDERTARGETGROUP_H__

// ----------------------------------------------------------------------- //
//
// MODULE  : SnowVolume.h
//
// PURPOSE : Large-scale procedural snow volume declaration:
//           - snow specific parameters
//
// CREATED : 1/3/02
//
// (c) 2002 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef __SNOWVOLUME_H__
#define __SNOWVOLUME_H__

#include "VolumeEffect.h"


LINKTO_MODULE( SnowVolume );


class SnowVolume : public VolumeEffect
{
public:
	SnowVolume();
	~SnowVolume();

protected:
	float m_fDensity;
	float m_fParticleRadius;
	float m_fFallRate;
	float m_fTumbleRate;
	float m_fTumbleRadius;
	float m_fMaxDrawDist;
	bool  m_bTranslucent;
	bool  m_bTranslucentLight;
	bool  m_bBackFaces;
	uint32 m_nBaseColor;
	std::string	m_sMaterialName;
	bool m_bOn;																						

	uint32			EngineMessageFn( uint32 messageID, void* pData, float fData );

	
private:
	void WriteSnowInfo( ILTMessage_Write& cMsg );
	void InitialUpdate();
	bool ReadProp( const GenericPropList *pProps );

	virtual void	Save( ILTMessage_Write *pMsg );
	virtual void	Load( ILTMessage_Read *pMsg );

	void CreateSpecialFX( bool bUpdateClients = false );

	void TurnOn( bool bOn );


	// Message Handlers....

	DECLARE_MSG_HANDLER( SnowVolume, HandleOnMsg );
	DECLARE_MSG_HANDLER( SnowVolume, HandleOffMsg );
};


#endif // __SNOWVOLUME_H__

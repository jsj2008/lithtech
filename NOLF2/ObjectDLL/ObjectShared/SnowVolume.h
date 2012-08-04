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
	LTVector m_vAmbientColor;
	bool m_bUseLighting;
	bool m_bUseSaturate;
	HSTRING m_hstrTextureName;

	uint32 EngineMessageFn( uint32 messageID, void* pData, LTFLOAT fData );

private:
	void WriteSnowInfo( ILTMessage_Write& cMsg );
	void InitialUpdate();
	bool ReadProp( ObjectCreateStruct* pOCS );
};


#endif // __SNOWVOLUME_H__
// ----------------------------------------------------------------------- //
//
// MODULE  : ScatterVolume.h
//
// PURPOSE : Scattered surface particles volume declaration:
//           - scatter specific parameters
//
// CREATED : 4/3/02
//
// (c) 2002 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef __SCATTERVOLUME_H__
#define __SCATTERVOLUME_H__

#include "VolumeEffect.h"


LINKTO_MODULE( ScatterVolume );


class ScatterVolume : public VolumeEffect
{
public:
	ScatterVolume();
	~ScatterVolume();

protected:
	uint32 m_nBlindDataIndex;
	float m_fHeight;
	float m_fWidth;
	float m_fTilt;
	float m_fMaxScale;
	float m_fWaveRate;
	float m_fWaveDist;
	float m_fMaxDrawDist;
	HSTRING m_hstrTextureName;
	bool m_bUseSaturate;

	uint32 EngineMessageFn( uint32 messageID, void* pData, LTFLOAT fData );

private:
	void WriteScatterInfo( ILTMessage_Write& cMsg );
	void InitialUpdate();
	bool ReadProp( ObjectCreateStruct* pOCS );
};


#endif // __SCATTERVOLUME_H__
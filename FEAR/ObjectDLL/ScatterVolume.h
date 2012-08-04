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
	float m_fMaxScale;
	float m_fWaveRate;
	float m_fWaveDist;
	float m_fMaxDrawDist;
	std::string	m_sMaterialName;
	bool  m_bTranslucent;
	bool  m_bTranslucentLight;
	bool  m_bBackFaces;
	uint32 m_nBaseColor;
	uint32 m_nNumImages;

	uint32	EngineMessageFn( uint32 messageID, void* pData, float fData );

private:
	void WriteScatterInfo( ILTMessage_Write& cMsg );
	void InitialUpdate();
	bool ReadProp( const GenericPropList *pProps );

	void	CreateSpecialFX( bool bUpdateClients = false );
	
	virtual void	Save( ILTMessage_Write *pMsg );
	virtual void	Load( ILTMessage_Read *pMsg );
};


#endif // __SCATTERVOLUME_H__

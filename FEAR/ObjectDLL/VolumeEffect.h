// ----------------------------------------------------------------------- //
//
// MODULE  : VolumeEffect.h
//
// PURPOSE : Volume effect declaration:
//           - base class for a volume based effects
//           - axis-aligned volume
//
// CREATED : 1/3/02
//
// (c) 2002 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef __VOLUMEEFFECT_H__
#define __VOLUMEEFFECT_H__

//
// Includes...
//
	
	#include "GameBase.h"

LINKTO_MODULE( VolumeEffect );


class VolumeEffect : public GameBase
{
public:
	VolumeEffect();
	virtual ~VolumeEffect();

protected:
	LTVector m_vDims;

	uint32 EngineMessageFn( uint32 messageID, void* pData, float fData );

	virtual void	Save( ILTMessage_Write *pMsg );
	virtual void	Load( ILTMessage_Read *pMsg );

	
private:
	void InitialUpdate();
	bool ReadProp( ObjectCreateStruct* pOCS );
};


#endif // __VOLUMEEFFECT_H__

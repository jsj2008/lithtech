//---------------------------------------------------------------------------------------------
// SoundZoneVolume.h
// 
// Provides the implementation for the sound zone volume type. This represents a single oriented
// bounding box stored as its half extents. This object isn't valid until after a successful call
// to Create or Load.
//
//---------------------------------------------------------------------------------------------

#ifndef __SOUNDZONEVOLUME_H__
#define __SOUNDZONEVOLUME_H__

#include "ltengineobjects.h"
#include "GameBase.h"

class SoundFilterDBPlugin;

LINKTO_MODULE( ServerSoundZoneVolume );

class SoundZoneVolume : public GameBase
{
public :

	SoundZoneVolume();
	~SoundZoneVolume();
	LTVector	GetHalfDims();

protected :

	uint32		EngineMessageFn(uint32 messageID, void *pData, float fData);
	bool		ReadProp(const GenericPropList *pProps);
	void		PostPropRead(ObjectCreateStruct *pStruct);
	bool		InitialUpdate();
	void		Save(ILTMessage_Write *pMsg, uint32 dwSaveFlags);
	void		Load(ILTMessage_Read *pMsg, uint32 dwLoadFlags);



private:
	LTVector	m_vHalfDims;
};


#endif   // __SOUNDZONEVOLUME_H__


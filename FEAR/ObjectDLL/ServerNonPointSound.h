// ----------------------------------------------------------------------- //
//
// MODULE  : ServerNonPointSound.cpp
//
// PURPOSE : A non-point sound object. This is the server side representation
//			and mostly exists for the editor. Info is then transmitted to the
//			client for actual work.
//
// CREATED : 08/18/04
//
// (c) 1998-2004 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __NONPOINT_SOUND_H__
#define __NONPOINT_SOUND_H__

#include "ltengineobjects.h"
#include "GameBase.h"
#include "SharedFXStructs.h"

class SoundFilterDBPlugin;

LINKTO_MODULE( ServerNonPointSound );

class SoundNonPoint : public GameBase
{
public :

	SoundNonPoint();
	~SoundNonPoint();

protected :

	uint32		EngineMessageFn(uint32 messageID, void *pData, float fData);
	virtual	void	CreateSpecialFX( bool bUpdateClients = false );

protected :

	bool		ReadProp(const GenericPropList *pProps);
	void		PostPropRead(ObjectCreateStruct *pStruct);
	bool		InitialUpdate();
	bool		AllObjectsCreated();
	bool		Update();
	virtual void	SendToggleMsg();

	void		Save(ILTMessage_Write *pMsg, uint32 dwSaveFlags);
	void		Load(ILTMessage_Read *pMsg, uint32 dwLoadFlags);

	// Member Variables

	bool			m_bLoadFromSave;
	std::string		m_sSoundZone[MAX_SOUND_VOLUMES];
	HOBJECT			m_hSoundZoneObj[MAX_SOUND_VOLUMES];

	SOUNDNONPOINTCREATESTRUCT m_SCS;


	// Message Handlers....

	DECLARE_MSG_HANDLER( SoundNonPoint, HandleToggleOnMsg );
	DECLARE_MSG_HANDLER( SoundNonPoint, HandleToggleOffMsg );
};

class CServerNonPointSoundPlugin : public IObjectPlugin
{
public:
	CServerNonPointSoundPlugin();
	virtual ~CServerNonPointSoundPlugin();

private:
	virtual LTRESULT PreHook_EditStringList(
		const char* szRezPath,
		const char* szPropName,
		char** aszStrings,
		uint32* pcStrings,
		const uint32 cMaxStrings,
		const uint32 cMaxStringLength);

protected :
	SoundFilterDBPlugin* m_pSoundFilterDBPlugin;
};

#endif // __NONPOINT_SOUND_H__

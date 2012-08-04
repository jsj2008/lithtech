/****************************************************************************
;
;	 MODULE:		VOICEMGR (.H)
;
;	PURPOSE:		Voice Manager for character voice sounds
;
;	HISTORY:		10/11/98 [blg] This file was created
;
;	COMMENT:		Copyright (c) 1998, Monolith Productions Inc.
;
****************************************************************************/


#ifndef _VOICEMGR_H_
#define _VOICEMGR_H_


// Includes...

#include "cpp_client_de.h"
#include "VoiceMgrDefs.h"


// Defines...

#define VM_MAX_SOUNDS		64
#define VM_MAX_STRING		128
#define VM_MAX_CHARACTERS	VMC_MAX


// Classes...

class CVoiceGroup
{
	// Member functions...

public:
	CVoiceGroup() { Clear(); }
	~CVoiceGroup() { Term(); }

	DBOOL				Init(CClientDE* pClientDE, char* sDir, char* sCustomDir);
	void				Term();
	void				Clear();

	int					GetNumSounds() { return(m_cSounds); }
	char*				GetSoundString(int iSound);
	char*				GetRandomSoundString();

	int					AddSoundStrings(char* sDir);
	DBOOL				AddSoundString(char* sSound);


	// Member variables...

private:
	char				m_sSounds[VM_MAX_SOUNDS][VM_MAX_STRING];
	int					m_cSounds;
	int					m_iLast;
	CClientDE*			m_pClientDE;
};

class CVoiceGroupMgr
{
	// Member functions...

public:
	CVoiceGroupMgr() { Clear(); }
	~CVoiceGroupMgr() { Term(); }

	DBOOL				Init(CClientDE* pClientDE, char* sDirPrefix, char* sCustomDirPrefix);
	void				Term();
	void				Clear();

	CVoiceGroup*		GetGroup(int iGroup);
	char*				GetGroupSoundString(int iGroup, int iSound);

	int					AddGroup(int iGroup, char* sDirPrefix, char* sCustomDirPrefix, char* sDir);


	// Member variables...

private:
	CVoiceGroup			m_aGroups[VME_MAX];
	CClientDE*			m_pClientDE;
};

class CVoiceMgr
{
	// Member functions...

public:
	CVoiceMgr() { m_hCurSound = DNULL; Clear(); }
	~CVoiceMgr() { Term(); }

	DBOOL				Init(CClientDE* pClientDE);
	void				Term();
	void				Clear();

	CVoiceGroupMgr*		GetGroupMgr(int iCharacter);
	CVoiceGroup*		GetGroup(int iCharacter, int iEvent);
	char*				GetRandomGroupSoundString(int iCharacter, int iEvent);
	DBOOL				GetUniqueEventSoundString(int iCharacter, int iUnique, char* sPath);

	void				SetNextPlayTime(DFLOAT fTimeAdd = 0.5f);
	void				SetOnOff(DBOOL bOn) { m_bOn = bOn; if (!IsOn()) StopAll(); }

	DBOOL				PlayEventSound(int iCharacter, int iEvent);
	DBOOL				PlayUniqueSound(int iCharacter, int iUnique);
	DBOOL				PlaySound(char* sSound, DFLOAT fTimeAdd = 0.5f);
	DBOOL				PlaySoundNoStream(char* sSound, DFLOAT fTimeAdd = 0.5f);

	DBOOL				IsOkToPlaySound();
	DBOOL				IsOn() { return(m_bOn); }

	DBOOL				IsUniqueSoundPlayed(int iCharacter, int iUnique);
	void				FlagUniqueSoundPlayed(int iCharacter, int iUnique);

	void				StopAll();
	void				Reset();


	// Member variables...

private:
	CClientDE*			m_pClientDE;
	CVoiceGroupMgr		m_aGroupMgrs[VM_MAX_CHARACTERS];
	DBOOL				m_aUniquePlayFlags[VMC_MAX][VMU_MAX];
	DBOOL				m_bOn;
	DFLOAT				m_fNextPlayTime;
	HSOUNDDE			m_hCurSound;
};


// Inlines...

inline void CVoiceGroup::Clear()
{
	for (int i = 0; i < VM_MAX_SOUNDS; i++)
	{
		m_sSounds[i][0] = '\0';
	}

	m_cSounds = 0;
	m_iLast   = -1;
	m_pClientDE  = DNULL;
}

inline char* CVoiceGroup::GetSoundString(int iSound)
{
	if (iSound < 0) return(DNULL);
	if (iSound >= VM_MAX_SOUNDS) return(DNULL);

	return(m_sSounds[iSound]);
}

inline void CVoiceGroupMgr::Clear()
{
	m_pClientDE = NULL;
}

inline CVoiceGroup* CVoiceGroupMgr::GetGroup(int iGroup)
{
	if (iGroup < 0) return(DNULL);
	if (iGroup >= VME_MAX) return(DNULL);

	return(&m_aGroups[iGroup]);
}

inline char* CVoiceGroupMgr::GetGroupSoundString(int iGroup, int iSound)
{
	CVoiceGroup* pGroup = GetGroup(iGroup);
	if (!pGroup) return(DNULL);

	return(pGroup->GetSoundString(iSound));
}

inline CVoiceGroupMgr* CVoiceMgr::GetGroupMgr(int iCharacter)
{
	if (iCharacter < 0) return(DNULL);
	if (iCharacter >= VMC_MAX) return(DNULL);

	return(&m_aGroupMgrs[iCharacter]);
}

inline CVoiceGroup* CVoiceMgr::GetGroup(int iCharacter, int iEvent)
{
	CVoiceGroupMgr* pMgr = GetGroupMgr(iCharacter);
	if (!pMgr) return(NULL);

	return(pMgr->GetGroup(iEvent));
}

inline char* CVoiceMgr::GetRandomGroupSoundString(int iCharacter, int iEvent)
{
	CVoiceGroup* pGroup = GetGroup(iCharacter, iEvent);
	if (!pGroup) return(DNULL);

	return(pGroup->GetRandomSoundString());
}

inline void CVoiceMgr::SetNextPlayTime(DFLOAT fTimeDelta)
{
	m_fNextPlayTime = m_pClientDE->GetTime() + fTimeDelta;
}


// EOF...

#endif

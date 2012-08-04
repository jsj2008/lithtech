/****************************************************************************
;
;	 MODULE:		FLAGOBJECTS (.H)
;
;	PURPOSE:		Flag Objects for CTF
;
;	HISTORY:		12/30/98 [blg] This file was created
;
;	COMMENT:		Copyright (c) 1998, Monolith Productions Inc.
;
****************************************************************************/


#ifndef _FLAGOBJECTS_H_
#define _FLAGOBJECTS_H_


// Includes...

#include "B2BaseClass.h"


// Classes...

class FlagObject : public B2BaseClass
{
	// Member functions...

public:
	FlagObject();
	~FlagObject();

	int				GetTeamID() { return(m_nTeamID); }

	void			SetTeamID(int nID) { m_nTeamID = nID; }
	void			SetPos(DVector* pPos);

	DDWORD			EngineMessageFn(DDWORD messageID, void* pData, DFLOAT fData);
	DDWORD			ObjectMessageFn(HOBJECT hSender, DDWORD messageID, HMESSAGEREAD hRead);

	DBOOL			IsInFlagStand() { return(m_bInStand); }
	DBOOL			IsOnGround() { return(m_bOnGround); }
	DBOOL			IsWithPlayer() { return(m_bWithPlayer); }
	DBOOL			IsOtherFlagInStand();

	void			GiveToPlayer();
	void			ReturnToFlagStand();
	void			DropToGround();

	void			TriggerPlayer(HOBJECT hPlayer);

protected:
	DBOOL			ReadProp(ObjectCreateStruct* pStruct);
	void			PostPropRead(ObjectCreateStruct* pStruct);

	void			OnInitialUpdate(void* pData, DFLOAT fData);
	void			OnTouchNotify(HOBJECT hObj);


	// Member variables...

private:
	int				m_nTeamID;
	DVector			m_vStand;
	DRotation		m_rStand;
	HSTRING			m_hFlagGrabString;
	DBOOL			m_bInStand;
	DBOOL			m_bOnGround;
	DBOOL			m_bWithPlayer;
};

class FlagStand : public B2BaseClass
{
	// Member functions...

public:
	FlagStand();
	~FlagStand();

	int				GetTeamID() { return(m_nTeamID); }

	DDWORD			EngineMessageFn(DDWORD messageID, void* pData, DFLOAT fData);
	DDWORD			ObjectMessageFn(HOBJECT hSender, DDWORD messageID, HMESSAGEREAD hRead);

	DBOOL			SpawnFlag();
	void			TriggerPlayer(HOBJECT hPlayer);

protected:
	DBOOL			ReadProp(ObjectCreateStruct* pStruct);
	void			PostPropRead(ObjectCreateStruct* pStruct);

	void			OnInitialUpdate(void* pData, DFLOAT fData);
	void			OnTouchNotify(HOBJECT hObj);


	// Member variables...

private:
	int				m_nTeamID;
	HOBJECT			m_hFlagObject;
	HSTRING			m_hFlagGiveString;
};


// Inlines...

inline FlagObject::FlagObject() : B2BaseClass(OT_MODEL)
{
	m_nTeamID         = 0;
	m_hFlagGrabString = DNULL;
	m_bInStand        = DFALSE;
	m_bOnGround       = DFALSE;
	m_bWithPlayer     = DFALSE;
}

inline FlagStand::FlagStand() : B2BaseClass(OT_MODEL)
{
	m_nTeamID         = 0;
	m_hFlagGiveString = DNULL;
	m_hFlagObject     = DNULL;
}

inline FlagStand::~FlagStand()
{
	if (g_pServerDE && m_hFlagGiveString)
	{
		g_pServerDE->FreeString(m_hFlagGiveString);
	}
}


// EOF...

#endif



// ----------------------------------------------------------------------- //
//
// MODULE  : GameStartPoint.h
//
// PURPOSE : GameStartPoint - Definition
//
// CREATED : 9/30/97
//
// (c) 1997-2002 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __GAME_START_POINT_H__
#define __GAME_START_POINT_H__

#include "GameBase.h"
#include "NetDefs.h"
#include "ModelButeMgr.h"
#include "SharedMovement.h"
#include "CommandMgr.h"

LINKTO_MODULE( GameStartPoint );


class GameStartPoint : public GameBase
{
	public :

		GameStartPoint();
		~GameStartPoint();

		HSTRING		GetName()			const { return m_hstrName; }
        LTVector    GetPitchYawRoll()	const { return m_vPitchYawRoll; }

		ModelId		GetPlayerModelId()	const { return m_eModelId;}

		HSTRING		GetCommand()		const { return m_hstrCommand; }

        PlayerPhysicsModel  GetPlayerPhysicsModel() const { return m_ePPhysicsModel; }

		bool		IsLocked()			const { return m_bLocked; }
		bool		SendCommandOnRespawn() const { return m_bSendCommandOnRespawn; }

		typedef std::vector< GameStartPoint* > StartPointList;
		static StartPointList const& GetStartPointList() { return m_lstStartPoints; }

		//used to determine the least recently used startpoint
		float		GetLastUse()		const { return m_fLastUse;}
		void		SetLastUse(float fLastUse) { m_fLastUse = fLastUse;}

		uint8		GetTeamID()			const { return m_nTeamID; }

		bool		IsStartPoint()		const { return m_bStartPoint; }
		bool		IsSpawnPoint()		const { return m_bSpawnPoint; }

	protected :

        uint32	EngineMessageFn(uint32 messageID, void *pData, LTFLOAT lData);
		bool	OnTrigger( HOBJECT hSender, const CParsedMsg &cMsg );
		
	private :

		ModelId		m_eModelId;				// Template model
        HSTRING     m_hstrName;             // Name of start point
        LTVector    m_vPitchYawRoll;        // Pitch, yaw, and roll of start point
		bool		m_bLocked;				// If locked the player should not use this start point.
		bool		m_bSendCommandOnRespawn;// Should the command be sent every respawn?

		PlayerPhysicsModel	m_ePPhysicsModel;	// Starting player physics model

		HSTRING		m_hstrCommand;	// Command to process

        static uint32 m_dwCounter;  // Counts gamestartpoints created for the level.

        LTBOOL ReadProp(ObjectCreateStruct *pStruct);

		static StartPointList m_lstStartPoints;

		float		m_fLastUse;
		
		uint8		m_nTeamID;		// When in a team game this specifies which team can spawn at this start point.
		
		bool		m_bStartPoint;	// Can this start point be used as an initial start point?
		bool		m_bSpawnPoint;	// Can this start point be used as a respawn point?
};


class GameStartPointLesser
{
public:
	
	bool operator()(const GameStartPoint* x, const GameStartPoint* y) const
	{
		return (x->GetLastUse() < y->GetLastUse() );
	}
};


class CGameStartPointPlugin : public IObjectPlugin
{
  public:

    virtual LTRESULT PreHook_EditStringList(
		const char* szRezPath,
		const char* szPropName,
		char** aszStrings,
        uint32* pcStrings,
        const uint32 cMaxStrings,
        const uint32 cMaxStringLength);

	virtual LTRESULT PreHook_PropChanged( 
		const	char		*szObjName,
		const	char		*szPropName,
		const	int			nPropType,
		const	GenericProp	&gpPropValue,
				ILTPreInterface	*pInterface,
		const	char		*szModifiers );

  protected :

        CModelButeMgrPlugin m_ModelButeMgrPlugin;
		CCommandMgrPlugin	m_CommandMgrPlugin;
};


#endif // __GAME_START_POINT_H__
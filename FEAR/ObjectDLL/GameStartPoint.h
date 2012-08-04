// ----------------------------------------------------------------------- //
//
// MODULE  : GameStartPoint.h
//
// PURPOSE : GameStartPoint - Definition
//
// CREATED : 9/30/97
//
// (c) 1997-2003 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __GAME_START_POINT_H__
#define __GAME_START_POINT_H__

#include "GameBase.h"
#include "NetDefs.h"
#include "SharedMovement.h"
#include "CommandMgr.h"

LINKTO_MODULE( GameStartPoint );


class GameStartPoint : public GameBase
{
	public :

		GameStartPoint();
		~GameStartPoint();

		const char*	GetName()			const { return m_sName.c_str(); }

		ModelsDB::HMODEL GetPlayerModel()	const { return m_hModel; }

		const char*	GetCommand()		const { return m_sCommand.c_str(); }

		PlayerPhysicsModel  GetPlayerPhysicsModel() const { return m_ePPhysicsModel; }

		bool		IsLocked()			const { return m_bLocked; }
		bool		SendCommandOnRespawn() const { return m_bSendCommandOnRespawn; }

		typedef std::vector< GameStartPoint*, LTAllocator<GameStartPoint*, LT_MEM_TYPE_OBJECTSHELL> > StartPointList;
		static StartPointList const& GetStartPointList() { return m_lstStartPoints; }

		//used to determine the least recently used startpoint
		double		GetLastUse()		const { return m_fLastUse;}
		void		SetLastUse(double fLastUse) { m_fLastUse = fLastUse;}

		uint8		GetTeamID()			const { return m_nTeamID; }

		bool		IsStartPoint()		const { return m_bStartPoint; }
		bool		IsSpawnPoint()		const { return m_bSpawnPoint; }

	protected :

		uint32		EngineMessageFn(uint32 messageID, void *pData, float lData);

	private:

		void		Save(ILTMessage_Write *pMsg, uint32 dwSaveFlags);
		void		Load(ILTMessage_Read *pMsg, uint32 dwLoadFlags);
		bool		ReadProp(const GenericPropList *pProps);

	private :

		ModelsDB::HMODEL	m_hModel;					// Template model
		std::string	m_sName;					// Name of start point
		bool		m_bLocked;					// If locked the player should not use this start point.
		bool		m_bSendCommandOnRespawn;	// Should the command be sent every respawn?

		PlayerPhysicsModel	m_ePPhysicsModel;	// Starting player physics model

		std::string		m_sCommand;	// Command to process

		double		m_fLastUse;
		
		uint8		m_nTeamID;		// When in a team game this specifies which team can spawn at this start point.
		
		bool		m_bStartPoint;	// Can this start point be used as an initial start point?
		bool		m_bSpawnPoint;	// Can this start point be used as a respawn point?


		static StartPointList m_lstStartPoints;

		// Message Handlers...

		DECLARE_MSG_HANDLER( GameStartPoint, HandleLockMsg );
		DECLARE_MSG_HANDLER( GameStartPoint, HandleUnlockMsg );
		DECLARE_MSG_HANDLER( GameStartPoint, HandleTeamMsg );
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

		CCommandMgrPlugin	m_CommandMgrPlugin;
};


#endif // __GAME_START_POINT_H__

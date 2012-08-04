// ----------------------------------------------------------------------- //
//
// MODULE  : GameStartPoint.h
//
// PURPOSE : GameStartPoint - Definition
//
// CREATED : 9/30/97
//
// ----------------------------------------------------------------------- //

#ifndef __GAME_START_POINT_H__
#define __GAME_START_POINT_H__

#include "GameBase.h"
#include "NetDefs.h"
#include "ModelButeMgr.h"
#include "SharedMovement.h"

class GameStartPoint : public GameBase
{
	public :

		GameStartPoint();
		~GameStartPoint();

		GameType	GetGameType()		const { return m_eGameType; }
		HSTRING		GetName()			const { return m_hstrName; }
        LTVector     GetPitchYawRoll()   const { return m_vPitchYawRoll; }

		HSTRING		GetTriggerTarget()	const { return m_hstrTriggerTarget; }
		HSTRING		GetTriggerMessage()	const { return m_hstrTriggerMessage; }

		ModelStyle	GetPlayerModelStyle() const { return m_ePlayerModelStyle; }
        PlayerPhysicsModel  GetPlayerPhysicsModel() const { return m_ePPhysicsModel; }
        uint8       GetTeam()           const { return m_byTeam; }

	protected :

        uint32 EngineMessageFn(uint32 messageID, void *pData, LTFLOAT lData);

	private :

		GameType	m_eGameType;			// Single/Multiplayer modes
		ModelStyle	m_ePlayerModelStyle;	// Style of player model
        HSTRING     m_hstrName;             // Name of start point
        LTVector     m_vPitchYawRoll;        // Pitch, yaw, and roll of start point
        uint8       m_byTeam;               // Which team uses this start point

		PlayerPhysicsModel	m_ePPhysicsModel;	// Starting player physics model

		HSTRING		m_hstrTriggerTarget;	// Name of object to trigger
		HSTRING		m_hstrTriggerMessage;	// Message to send to object

        static uint32 m_dwCounter;  // Counts gamestartpoints created for the level.

        LTBOOL ReadProp(ObjectCreateStruct *pStruct);

		void CacheFiles();
		void CachePlayerFiles();
		void CacheSurfaceFiles();
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

  protected :

        CModelButeMgrPlugin m_ModelStylePlugin;
};

#endif // __GAME_START_POINT_H__
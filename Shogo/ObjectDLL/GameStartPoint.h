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

#include "cpp_engineobjects_de.h"
#include "..\Shared\NetDefs.h"
#include "SurfaceTypes.h"

enum GameType { SINGLE=NGT_SINGLE, COOPERATIVE=NGT_COOPERATIVE, DEATHMATCH=NGT_DEATHMATCH, CAPTURE_FLAG=NGT_CAPTUREFLAG };

class GameStartPoint : public StartPoint
{
	public :

		GameStartPoint();
		~GameStartPoint();

		DDWORD		GetPlayerMode()		const { return m_nPlayerMode; }
		GameType	GetGameType()		const { return m_eGameType; }
		HSTRING		GetName()			const { return m_hstrName; }
		DVector		GetPitchYawRoll()	const { return m_vPitchYawRoll; }

		HSTRING		GetTriggerTarget()	const { return m_hstrTriggerTarget; }
		HSTRING		GetTriggerMessage()	const { return m_hstrTriggerMessage; }

	protected :

		DDWORD EngineMessageFn(DDWORD messageID, void *pData, DFLOAT lData);
	
	private :

		DDWORD		m_nPlayerMode;		// Mode of player in this level
		GameType	m_eGameType;		// Single/Multiplayer modes
		HSTRING		m_hstrName;			// Name of start point
		DVector		m_vPitchYawRoll;	// Pitch, yaw, and roll of start point

		HSTRING		m_hstrTriggerTarget;	// Name of object to trigger
		HSTRING		m_hstrTriggerMessage;	// Message to send to object

		static DDWORD m_dwCounter;	// Counts gamestartpoints created for the level.

		DBOOL ReadProp(ObjectCreateStruct *pStruct);

		void CacheFiles();
		void CacheOnFootFiles();
		void CacheMechaFiles();
		void CachePlayerModeFiles();
		void CacheSurfaceFiles( );
};

#endif // __GAME_START_POINT_H__

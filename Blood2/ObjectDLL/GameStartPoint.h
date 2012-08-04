// ----------------------------------------------------------------------- //
//
// MODULE  : GameStartPoint.h
//
// PURPOSE : GameStartPoint - Definition
//
// CREATED : 3/26/98
//
// ----------------------------------------------------------------------- //

#ifndef __GAMESTARTPOINT_H__
#define __GAMESTARTPOINT_H__

#include "cpp_engineobjects_de.h"


class GameStartPoint : public StartPoint
{
	public :

		GameStartPoint();
		~GameStartPoint();

		HSTRING		GetName()			const { return m_hstrName; }
		DVector		GetPitchYawRoll()	const { return m_vPitchYawRoll; }
		DBOOL		IsMultiplayer()		const { return m_bMultiplayer; }
		int			GetTeamID() { return(m_nTeamID); }
		void		SendTrigger();

	protected :

		DDWORD EngineMessageFn(DDWORD messageID, void *pData, DFLOAT lData);
	
	private :

		DBOOL		m_bMultiplayer;		// Multiplayer startpoint
		HSTRING		m_hstrName;			// Name of start point
		DVector		m_vPitchYawRoll;	// Pitch, yaw, and roll of start point
		HSTRING		m_hstrTriggerTarget;
		HSTRING		m_hstrTriggerMessage;
		int			m_nTeamID;			// Team ID (1 or 2, or 0 for any)
		DBOOL		ReadProp();
};

#endif // __GAMESTARTPOINT_H__

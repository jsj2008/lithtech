/****************************************************************************
;
;	MODULE:			Obstacle.h
;
;	PURPOSE:		Base obstacle class for TRON
;
;	HISTORY:		12/19/2001 [kml] This file was created
;
;	COMMENT:		Copyright (c) 2001, Monolith Productions, Inc.
;
****************************************************************************/


#ifndef _OBSTACLE_H_
#define _OBSTACLE_H_

#include "GameBase.h"

class Obstacle : public GameBase
{
	public :

		Obstacle();
		virtual ~Obstacle();

		uint8	GetPSets() { return m_byPSets; }
		void	StartMiniGame();

	protected :

        virtual uint32 EngineMessageFn(uint32 messageID, void *pData, LTFLOAT fData);
		virtual uint32 ObjectMessageFn(HOBJECT hSender, ILTMessage_Read *pMsg);

	private :

        void Save(ILTMessage_Write *pMsg, uint32 dwSaveFlags);
        void Load(ILTMessage_Read *pMsg, uint32 dwLoadFlags);

        LTBOOL ReadProp(ObjectCreateStruct *pData);

        uint8	m_byPSets;				// Permission sets (each bit)
		HSTRING m_hstrGoodPSetsTrigger;	// When user comes in with correct PSets
		HSTRING m_hstrBadPSetsTrigger;	// When user comes in with incorrect PSets
		HSTRING m_hstrGoodGameTrigger;	// When user finishes game
		HSTRING m_hstrBadGameTrigger;	// When user fails game
		HSTRING m_hstrFinishedMsg;		// When this object is finished
		LTBOOL	m_bPlayingMiniGame;		// Is the player playing the minigame?
		BYTE	m_byGameID;				// Type of game
};

#endif // _OBSTACLE_H_
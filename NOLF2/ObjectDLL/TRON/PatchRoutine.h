/****************************************************************************
;
;	MODULE:			PatchRoutine.h
;
;	PURPOSE:		Patch routine class for TRON
;
;	HISTORY:		02/12/2002 [kml] This file was created
;
;	COMMENT:		Copyright (c) 2001, Monolith Productions, Inc.
;
****************************************************************************/


#ifndef _PATCHROUTINE_H_
#define _PATCHROUTINE_H_

#include "GameBase.h"

class CTronPlayerObj;

class PatchRoutine : public GameBase
{
	public :

		PatchRoutine();
		~PatchRoutine();

	protected :

        virtual uint32 EngineMessageFn(uint32 messageID, void *pData, LTFLOAT fData);
		virtual bool OnTrigger(HOBJECT hSender, const CParsedMsg &cMsg);
		void Reset() { m_nCurrentHealth = m_nTotalHealth; m_nCurrentEnergy = m_nTotalEnergy; }

	private :

        void Save(ILTMessage_Write *pMsg, uint32 dwSaveFlags);
        void Load(ILTMessage_Read *pMsg, uint32 dwLoadFlags);
		void HandleActivate(HOBJECT hSender);
		void HandleStopActivate();
		LTBOOL Update();

        LTBOOL ReadProp(ObjectCreateStruct *pData);

		// We're giving this object total and current values just in case
		// a recharge or reset feature is added
		int m_nTotalHealth;
		int m_nTotalEnergy;
		int	m_nCurrentHealth;
		int m_nCurrentEnergy;

		uint8 m_bActive;
		CTronPlayerObj* m_pPlayer;
};

#endif // _PATCHROUTINE_H_
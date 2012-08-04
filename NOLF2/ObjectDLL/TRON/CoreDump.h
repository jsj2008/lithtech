/****************************************************************************
;
;	MODULE:			CoreDump.h
;
;	PURPOSE:		Core Dump object class for TRON
;
;	HISTORY:		03/22/2002 [kml] This file was created
;
;	COMMENT:		Copyright (c) 2001, Monolith Productions, Inc.
;
****************************************************************************/


#ifndef _COREDUMP_H_
#define _COREDUMP_H_

#include "GameBase.h"
#include "GeneralInventory.h"

#define MAX_CORE_DUMP_FX 4

class CTronPlayerObj;

class CoreDump : public GameBase
{
	public :

		CoreDump();
		~CoreDump();

		void SetInventory(GEN_INVENTORY_LIST& lst) { m_lstInventory = lst; }
		void Init(float fEnergy) { m_fCurrentEnergy = m_fTotalEnergy = fEnergy; m_bActive = false; }

	protected :

        virtual uint32 EngineMessageFn(uint32 messageID, void *pData, LTFLOAT fData);
		virtual bool OnTrigger(HOBJECT hSender, const CParsedMsg &cMsg);

	private :

		void Term();
        void Save(ILTMessage_Write *pMsg, uint32 dwSaveFlags);
        void Load(ILTMessage_Read *pMsg, uint32 dwLoadFlags);
		void HandleActivate(HOBJECT hSender);
		void HandleStopActivate();
		void HandleComplete(HOBJECT hSender);
		
		LTBOOL InitialUpdate();
		LTBOOL Update();
        LTBOOL ReadProp(ObjectCreateStruct *pData);

		// Energy
		float m_fTotalEnergy;
		float m_fCurrentEnergy;

		uint8 m_bActive;
		CTronPlayerObj* m_pPlayer;
		HOBJECT m_collFX[MAX_CORE_DUMP_FX];

		GEN_INVENTORY_LIST	m_lstInventory;	// General inventory items (see GeneralInventory.h)
};

#endif // _COREDUMP_H_
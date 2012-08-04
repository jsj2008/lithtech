/****************************************************************************
;
;	MODULE:			BinaryBit.h
;
;	PURPOSE:		Binary Bit class for TRON
;
;	HISTORY:		2/14/2002 [kml] This file was created
;
;	COMMENT:		Copyright (c) 2002, Monolith Productions, Inc.
;
****************************************************************************/

#ifndef _BINARYBIT_H_
#define _BINARYBIT_H_

#include "GameBase.h"
#include "CommandMgr.h"

enum BinaryBitState
{
	BBState_PoweredDown = 0,
	BBState_GoingToSocket,
	BBState_On,
	BBState_Off
};

class BinaryBit : public GameBase
{
	public :

		BinaryBit();
		virtual ~BinaryBit();

	protected :

        virtual	uint32 EngineMessageFn(uint32 messageID, void *pData, LTFLOAT fData);
		virtual	uint32 ObjectMessageFn(HOBJECT hSender, ILTMessage_Read *pMsg);

	private :

        void	Save(ILTMessage_Write *pMsg, uint32 dwSaveFlags);
        void	Load(ILTMessage_Read *pMsg, uint32 dwLoadFlags);
		
        LTBOOL	ReadProp(ObjectCreateStruct *pStruct);
		LTBOOL	InitialUpdate();
		LTBOOL	HandleModelString(ArgList* pArgList);

		LTBOOL	ChangeState(BinaryBitState eNewState);
		void	TurnOn();
		void	TurnOff();
		void	HandleGoingToSocket();
		void	HandleArriveAtSocket();
		void	HandleActivateMessage(HOBJECT hSender);
		void	DoPowerUp();

		// Member variablez, yo
		LTBOOL			m_bLocked;					// Is the binary bit locked
		LTBOOL			m_bStartOnInSocket;			// Are we on or off when we first reach the socket?
		BinaryBitState	m_eState;					// What's the state
		int				m_nOriginalEnergyRequired;	// How much is originally required to power us
		int				m_nCurrentEnergyRequired;	// How much is currently required
		LTBOOL			m_bPlayingTransitionAnim;	// Are we playing a short on/off animation?

		// Commands
		HSTRING			m_hstrPowerUpCommand;
		HSTRING			m_hstrTurnOnCommand;
		HSTRING			m_hstrTurnOffCommand;

		uint8			m_byPSets;

		HOBJECT			m_hFX;
};

class CBinaryBitPlugin : public IObjectPlugin
{
	public :

		virtual LTRESULT PreHook_PropChanged(
				const char *szObjName,
				const char *szPropName,
				const int nPropType,
				const GenericProp &gpPropValue,
				ILTPreInterface *pInterface );

	protected :

		CCommandMgrPlugin m_CommandMgrPlugin;
};

#endif // _BINARYBIT_H_
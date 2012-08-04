// ----------------------------------------------------------------------- //
//
// MODULE  : DisplayMeter.h
//
// PURPOSE : DisplayMeter - Definition
//
// CREATED : 7/19/00
//
// (c) 2000 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __DISPLAY_METER_H__
#define __DISPLAY_METER_H__

#include "ltengineobjects.h"
#include "CommandMgr.h"
#include "GameBase.h"

#define DM_MAX_NUMBER_OF_PHASES		5

LINKTO_MODULE( DisplayMeter );

struct PhaseData
{
    PhaseData()
	{
		nValue		= 0;
        hstrCmd     = LTNULL;
	}

    virtual ~PhaseData()
	{
		FREE_HSTRING(hstrCmd);
	}

	inline void Save(ILTMessage_Write *pMsg)
	{
		SAVE_INT(nValue);
		SAVE_HSTRING(hstrCmd);
	}

	inline void Load(ILTMessage_Read *pMsg)
	{
		LOAD_INT(nValue);
		LOAD_HSTRING(hstrCmd);
	}

	// Data...

	int		nValue;
	HSTRING	hstrCmd;
};




class DisplayMeter : public GameBase
{
	public :

		DisplayMeter();
		~DisplayMeter();

	protected :

        uint32  EngineMessageFn(uint32 messageID, void *pData, LTFLOAT fData);
		bool	OnTrigger(HOBJECT hSender, const CParsedMsg &cMsg);

	private :

		PhaseData	m_PhaseData[DM_MAX_NUMBER_OF_PHASES];
        LTBOOL		m_bRemoveWhenEmpty;  // Remove when the meter is empty?
		uint8		m_nValue;

		void	ReadProp(ObjectCreateStruct *pData);
		void	InitialUpdate();
		void	Update();

        void    HandleShow(uint8 initVal);
        void    HandleSet(uint8 val);
        void    HandlePlus(uint8 val);
        void    HandleMinus(uint8 val);
        void    HandleEnd();
		void	UpdateClients();

		void	Save(ILTMessage_Write *pMsg);
		void	Load(ILTMessage_Read *pMsg);
};

class CDisplayMeterPlugin : public IObjectPlugin
{
	public:

		virtual LTRESULT PreHook_PropChanged( 
			const	char		*szObjName,
			const	char		*szPropName,
			const	int			nPropType,
			const	GenericProp	&gpPropValue,
					ILTPreInterface	*pInterface,
			const	char		*szModifiers );

	protected:

		CCommandMgrPlugin	m_CommandMgrPlugin;

};

#endif // __DISPLAY_METER_H__
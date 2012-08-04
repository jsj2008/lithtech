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
	:	nValue	( 0 ),
		sCmd	( )
		
	{
	}

	virtual ~PhaseData()
	{
		
	}

	inline void Save(ILTMessage_Write *pMsg)
	{
		SAVE_INT(nValue);
		SAVE_STDSTRING(sCmd);
	}

	inline void Load(ILTMessage_Read *pMsg)
	{
		LOAD_INT(nValue);
		LOAD_STDSTRING(sCmd);
	}

	// Data...

	int			nValue;
	std::string	sCmd;
};




class DisplayMeter : public GameBase
{
	public :

		DisplayMeter();
		~DisplayMeter();

	protected :

		uint32	EngineMessageFn(uint32 messageID, void *pData, float fData);

	private :

		PhaseData	m_PhaseData[DM_MAX_NUMBER_OF_PHASES];
		bool		m_bRemoveWhenEmpty;  // Remove when the meter is empty?
		uint8		m_nValue;

		void	ReadProp(const GenericPropList *pProps);
		void	InitialUpdate();
		void	Update();

		void	Show(uint8 initVal);
		void	Set(uint8 val);
		void	Plus(uint8 val);
		void	Minus(uint8 val);
		void	End();
		void	UpdateClients();

		void	Save(ILTMessage_Write *pMsg);
		void	Load(ILTMessage_Read *pMsg);


		// Message Handlers...

		DECLARE_MSG_HANDLER( DisplayMeter, HandleOnMsg );
		DECLARE_MSG_HANDLER( DisplayMeter, HandleIncrementMsg );
		DECLARE_MSG_HANDLER( DisplayMeter, HandleDecrementMsg );
		DECLARE_MSG_HANDLER( DisplayMeter, HandleSetValMsg );
		DECLARE_MSG_HANDLER( DisplayMeter, HandleOffMsg );
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

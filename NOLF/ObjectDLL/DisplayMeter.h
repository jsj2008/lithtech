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


#define DM_MAX_NUMBER_OF_PHASES		5


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

	inline void Save(HMESSAGEWRITE hWrite)
	{
		SAVE_INT(nValue);
		SAVE_HSTRING(hstrCmd);
	}

	inline void Load(HMESSAGEREAD hRead)
	{
		LOAD_INT(nValue);
		LOAD_HSTRING(hstrCmd);
	}

	// Data...

	int		nValue;
	HSTRING	hstrCmd;
};




class DisplayMeter : public BaseClass
{
	public :

		DisplayMeter();
		~DisplayMeter();

	protected :

        uint32  EngineMessageFn(uint32 messageID, void *pData, LTFLOAT fData);
        uint32  ObjectMessageFn(HOBJECT hSender, uint32 messageID, HMESSAGEREAD hRead);

	private :

		PhaseData	m_PhaseData[DM_MAX_NUMBER_OF_PHASES];
        LTBOOL		m_bRemoveWhenEmpty;  // Remove when the meter is empty?
		uint8		m_nValue;

		void	ReadProp(ObjectCreateStruct *pData);
		void	InitialUpdate();
		void	Update();

		void	TriggerMsg(HOBJECT hSender, const char *pMsg);

        void    HandleShow(uint8 initVal);
        void    HandleSet(uint8 val);
        void    HandlePlus(uint8 val);
        void    HandleMinus(uint8 val);
        void    HandleEnd();
		void	UpdateClients();

		void	Save(HMESSAGEWRITE hWrite);
		void	Load(HMESSAGEREAD hRead);
};

#endif // __DISPLAY_METER_H__
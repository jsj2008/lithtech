// ----------------------------------------------------------------------- //
//
// MODULE  : DisplayTimer.h
//
// PURPOSE : DisplayTimer - Definition
//
// CREATED : 10/15/99
//
// (c) 1999 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __DISPLAY_TIMER_H__
#define __DISPLAY_TIMER_H__

#include "ltengineobjects.h"
#include "Timer.h"
#include "CommandMgr.h"
#include "GameBase.h"

LINKTO_MODULE( DisplayTimer );

class DisplayTimer : public GameBase
{
	public :

		DisplayTimer();
		~DisplayTimer();

		// Start the timer.
		void	Start( float fTime );

	protected :

        uint32  EngineMessageFn(uint32 messageID, void *pData, LTFLOAT fData);
		bool	OnTrigger(HOBJECT hSender, const CParsedMsg &cMsg);

	private :

		HSTRING	m_hstrStartCmd;		// Command to send when timer starts
		HSTRING	m_hstrEndCmd;		// Command to send when timer ends
        LTBOOL   m_bRemoveWhenDone;  // Remove the timer when done?

		CTimer	m_Timer;			// Timer
		uint8	m_nTeamId;			// Team this timer is for.

		void	ReadProp(ObjectCreateStruct *pData);
		void	InitialUpdate();
		void	Update();

        void    HandleStart(LTFLOAT fTime);
        void    HandleAdd(LTFLOAT fTime);
		void	HandlePause();
		void	HandleResume();
		void	HandleEnd();
		void	HandleAbort();
		void	UpdateClients();

		void	Save(ILTMessage_Write *pMsg);
		void	Load(ILTMessage_Read *pMsg);
};

class CDisplayTimerPlugin : public IObjectPlugin
{
	public:

    virtual LTRESULT PreHook_EditStringList(
		const char* szRezPath,
		const char* szPropName,
		char** aszStrings,
        uint32* pcStrings,
        const uint32 cMaxStrings,
        const uint32 cMaxStringLength);

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

#endif // __DISPLAY_TIMER_H__
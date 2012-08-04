// ----------------------------------------------------------------------- //
//
// MODULE  : DisplayTimer.h
//
// PURPOSE : DisplayTimer - Definition
//
// CREATED : 10/15/99
//
// (c) 1999-2003 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __DISPLAY_TIMER_H__
#define __DISPLAY_TIMER_H__

#include "ltengineobjects.h"
#include "CommandMgr.h"
#include "GameBase.h"
#include "EngineTimer.h"

LINKTO_MODULE( DisplayTimer );

class DisplayTimer : public GameBase
{
	public :

		DisplayTimer();
		~DisplayTimer();

		// Start the timer.
		void			Start( double fTime );

	protected :

		uint32			EngineMessageFn(uint32 messageID, void *pData, float fData);

	private :

		std::string		m_sStartCmd;		// Command to send when timer starts
		std::string		m_sEndCmd;			// Command to send when timer ends
		bool			m_bRemoveWhenDone;	// Remove the timer when done?

		StopWatchTimer	m_Timer;			// Timer
		uint8			m_nTeamId;			// Team this timer is for.

		void			ReadProp(const GenericPropList *pProps);
		void			InitialUpdate();
		void			Update();

		void			End();
		void			Abort();
		void			UpdateClients();

		void			Save(ILTMessage_Write *pMsg);
		void			Load(ILTMessage_Read *pMsg);


		// Message Handlers...

		DECLARE_MSG_HANDLER( DisplayTimer, HandleOnMsg );
		DECLARE_MSG_HANDLER( DisplayTimer, HandleOffMsg );
		DECLARE_MSG_HANDLER( DisplayTimer, HandleIncMsg );
		DECLARE_MSG_HANDLER( DisplayTimer, HandleDecMsg );
		DECLARE_MSG_HANDLER( DisplayTimer, HandleAbortMsg );
		DECLARE_MSG_HANDLER( DisplayTimer, HandlePauseMsg );
		DECLARE_MSG_HANDLER( DisplayTimer, HandleResumeMsg );
		DECLARE_MSG_HANDLER( DisplayTimer, HandleTeamMsg );
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

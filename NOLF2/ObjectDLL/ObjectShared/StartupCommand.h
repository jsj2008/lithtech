// ----------------------------------------------------------------------- //
//
// MODULE  : StartupCommand.h
//
// PURPOSE : The StartupCommand object
//
// CREATED : 11/27/01
//
// (c) 2001 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //


#ifndef __STARTUP_COMMAND_H__
#define __STARTUP_COMMAND_H__

//
// Includes...
//
	
	#include "GameBase.h"

LINKTO_MODULE( StartupCommand );


class StartupCommand : public GameBase
{
	public : // Methods...

		StartupCommand( );
		~StartupCommand( );


	protected : // Methods...

		uint32	EngineMessageFn(uint32 messageID, void *pData, LTFLOAT fData);

		void	ReadProps( ObjectCreateStruct *pOCS );

		void	SendCommand( );


	protected : // Members...

		std::string		m_sStartupCmd;
		
		enum RoundCondition
		{
			kRoundCondition_All,
			kRoundCondition_Odd,
			kRoundCondition_Even,
		};

		RoundCondition m_eRoundCondition;
		bool m_bSafeExecute;

	private:

        void    Save(ILTMessage_Write *pMsg, uint32 dwSaveFlags);
        void    Load(ILTMessage_Read *pMsg, uint32 dwLoadFlags);

};


class CStartupCommandPlugin : public IObjectPlugin 
{
	public: // Methods...

		virtual LTRESULT PreHook_PropChanged(
				const	char		*szObjName,
				const	char		*szPropName,
				const	int			nPropType,
				const	GenericProp	&gpPropValue,
						ILTPreInterface	*pInterface,
				const	char		*szModifiers );

		virtual LTRESULT PreHook_EditStringList(const char* szRezPath,
												   const char* szPropName,
												   char** aszStrings,
												   uint32* pcStrings,
												   const uint32 cMaxStrings,
												   const uint32 cMaxStringLength);

	private: // Members...

		CCommandMgrPlugin m_CmdMgrPlugin;
};

#endif // __STARTUP_COMMAND_H__
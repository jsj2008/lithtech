// ----------------------------------------------------------------------- //
//
// MODULE  : Dialogue.h
//
// PURPOSE : The Dialogue object
//
// CREATED : 10/15/01
//
// (c) 2001-2002 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __DIALOGUE_H__
#define __DIALOGUE_H__

//
// Includes...
//

	#include "GameBase.h"

//
// Defines...
//

LINKTO_MODULE( Dialogue );

	#define MAX_DIALOGUES	20

//
// Classes...
//

class Dialogue : public GameBase
{
	public: // Methods...

		Dialogue();
		~Dialogue();

		static void GetPrefetchResourceList(const char* pszObjectName, IObjectResourceGatherer* pInterface, ResourceList& Resources );

	protected: // Methods...

		uint32	EngineMessageFn( uint32 messageID, void *pData, float fData );

		void	Update( );
		bool	NotifySpeakers( );
		bool	UpdateDialogue( );

		void	ReadProps( const GenericPropList *pProps );
		void	Save( ILTMessage_Write *pMsg );
		void	Load( ILTMessage_Read *pMsg );

		bool	StartDialogue( );
		void	TurnOff( );

		void	ResetTalkGoals( );


	protected: // Members...

		bool		m_bNotifiedSpeakers;					// Did we notify and link all dialogue participants
		LTObjRef	m_hCurSpeaker;							// Current dialogue participant that is speaking
		uint8		m_nCurDialogue;							// Which dialogue we are currently on
		double		m_fNextDialogueStart;					// Time to start next dialogue

		std::string	m_sStartCommand;						// A command we process when the sequence begins
		std::string	m_sFinishedCommand;						// A command we process when the sequence ends
		std::string	m_sCleanUpCommand;						// A command that gets processed whenever the dialogue is stopped (finishes, messaged, skipped...)

		bool		m_bOn;									// Is the Dialogue active and playing
		
		typedef std::vector<float, LTAllocator<float, LT_MEM_TYPE_OBJECTSHELL> > FloatArray;
		FloatArray	m_faDialogueDelay;						// Array of every delay before a dialogue plays
		StringArray	m_saDialogue;							// Array of dialogue .wzv id and speaker pairs
		StringArray	m_saIcon;								// Array of dialogue transmission icons
		StringArray	m_saDialogueStartCmd;					// Array of conmmands to process when a dialogue is done

		bool		m_bRemoveWhenComplete;					// Remove the object when done.


		// Message Handlers...

		DECLARE_MSG_HANDLER( Dialogue, HandleOnMsg );
		DECLARE_MSG_HANDLER( Dialogue, HandleOffMsg );

};

// Plugin Class

class CDialoguePlugin : public IObjectPlugin
{
	public:

		virtual LTRESULT PreHook_PropChanged( 
			const	char		*szObjName,
			const	char		*szPropName,
			const	int			nPropType,
			const	GenericProp	&gpPropValue,
					ILTPreInterface	*pInterface,
			const	char		*szModifiers );

		virtual LTRESULT PreHook_EditStringList(
			const char* szRezPath,
			const char* szPropName,
			char** aszStrings,
			uint32* pcStrings,
			const uint32 cMaxStrings,
			const uint32 cMaxStringLength);
	protected:

		CCommandMgrPlugin	m_CommandMgrPlugin;

};

#endif // __DIALOGUE_H__

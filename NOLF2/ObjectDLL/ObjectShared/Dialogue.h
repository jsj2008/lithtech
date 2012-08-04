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


	protected: // Methods...

        uint32	EngineMessageFn( uint32 messageID, void *pData, LTFLOAT fData );
		bool	OnTrigger( HOBJECT hSender, const CParsedMsg &cMsg );

		void	Update( );
		LTBOOL	NotifySpeakers( );
		LTBOOL	UpdateDialogue( );

		void	ReadProps( ObjectCreateStruct *pStruct );
		void    Save( ILTMessage_Write *pMsg );
        void    Load( ILTMessage_Read *pMsg );

		LTBOOL	StartDialogue( );

		void	ResetTalkGoals( );

		void	HandleOff( );
		void	HandleOn( );


	protected: // Members...

		LTBOOL		m_bNotifiedSpeakers;					// Did we notify and link all dialogue participants
		LTObjRef	m_hCurSpeaker;							// Current dialogue participant that is speaking
		uint8		m_nCurDialogue;							// Which dialogue we are currently on
		LTFLOAT		m_fNextDialogueStart;					// Time to start next dialogue

		HSTRING		m_hstrStartCommand;						// A command we process when the sequence begins
		HSTRING		m_hstrFinishedCommand;					// A command we process when the sequence ends
		HSTRING		m_hstrCleanUpCommand;					// A command that gets processed whenever the dialogue is stopped (finishes, messaged, skipped...)

		LTBOOL		m_bOn;									// Is the Dialogue active and playing

		LTFLOAT		m_fDialogueDelay[MAX_DIALOGUES];		// Array of every delay before a dialogue plays
		HSTRING		m_hstrDialogue[MAX_DIALOGUES];			// Array of dialogue .wzv id and speaker pairs
		HSTRING		m_hstrDialogueStartCmd[MAX_DIALOGUES];	// Array of conmmands to process when a dialogue is done

		bool		m_bRemoveWhenComplete;					// Remove the object when done.

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

	protected:

		CCommandMgrPlugin	m_CommandMgrPlugin;

};

#endif // __DIALOGUE_H__
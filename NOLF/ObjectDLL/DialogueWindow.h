// ----------------------------------------------------------------------- //
//
// MODULE  : DialogueWindow.h
//
// PURPOSE : DialogueWindow - Definition
//
// CREATED : 4/17/99
//
// COMMENT : Copyright (c) 1999, Monolith Productions, Inc.
//
// ----------------------------------------------------------------------- //

#ifndef _DIALOGUEWINDOW_H_
#define _DIALOGUEWINDOW_H_

#include "SCDefs.h"
#include "Character.h"
#define CPrBase BaseClass
#define CActor CCharacter


class CinematicTrigger;
class CDialogueWindow : public CPrBase
{
	public:

		CDialogueWindow();
		~CDialogueWindow() { Term(); }

		BOOL				Init();
		void				Term();
		BOOL				IsInitialized() { return m_bInitialized; }
        void                Finished(uint8 byDecision = 0, uint32 dwDecision = 0);
        uint32              EngineMessageFn(uint32 messageID, void *pData, LTFLOAT fData);
		BOOL				IsPlaying() { return m_bPlaying; }
        BOOL                PlayDialogue(uint32 dwID, CActor* pSpeaker, BOOL bStayOpen, const char *szActorName,
											char *szDecisions = NULL);
        BOOL                PlayDialogue(uint32 dwID, BOOL bStayOpen, const char *szActorName,
								   char *szDecisions, CinematicTrigger* pCinematic);
		void				StopDialogue() { if(m_pCurSpeaker) m_pCurSpeaker->KillDialogueSound(); }
        BOOL                SendMessage(uint32 dwID, BOOL bStayOpen, const char *szActorName, char *szDecisions);

	private:

		BOOL				m_bInitialized;
		CActor*				m_pCurSpeaker;
		CinematicTrigger*	m_pCinematic;
		BOOL				m_bPlaying;
		BOOL				m_byDecision;
};

inline CDialogueWindow::CDialogueWindow()
{
	m_bInitialized = FALSE;
	m_pCurSpeaker = NULL;
	m_pCinematic = NULL;
	m_bPlaying = FALSE;
	m_byDecision = 0;
}


inline BOOL CDialogueWindow::SendMessage(uint32 dwID, BOOL bStayOpen, const char *szActorName, char *szDecisions)
{
	// Bring up a dialogue window on the client (NOTE: This goes to ALL CLIENTS)
    HMESSAGEWRITE hMsg = g_pLTServer->StartMessage(LTNULL, SCM_PLAY_DIALOGUE);
    g_pLTServer->WriteToMessageDWord(hMsg, dwID);
    g_pLTServer->WriteToMessageString(hMsg,(char *)szActorName);
    g_pLTServer->WriteToMessageByte(hMsg,bStayOpen);
    g_pLTServer->WriteToMessageString(hMsg,szDecisions);
    g_pLTServer->EndMessage(hMsg);
	m_bPlaying = TRUE;
	return TRUE;
}

extern CDialogueWindow g_DialogueWindow;

#endif
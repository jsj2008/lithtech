// ----------------------------------------------------------------------- //
//
// MODULE  : DialogueWindow.cpp
//
// PURPOSE : DialogueWindow - Implementation
//
// CREATED : 4/17/99
//
// COMMENT : Copyright (c) 1999, Monolith Productions, Inc.
//
// ----------------------------------------------------------------------- //

#include "StdAfx.h"
#include "DialogueWindow.h"
#include "CinematicTrigger.h"

BEGIN_CLASS(CDialogueWindow)
END_CLASS_DEFAULT_FLAGS(CDialogueWindow, BaseClass, NULL, NULL, CF_HIDDEN)

uint32 CDialogueWindow::EngineMessageFn(uint32 messageID, void *pData, LTFLOAT fData)
{
	switch (messageID)
	{
		case MID_LINKBROKEN:
		{
			ASSERT(FALSE);
			if (m_pCurSpeaker && (HOBJECT)pData == m_pCurSpeaker->m_hObject)
			{
				m_pCurSpeaker = NULL;
			}

			if (m_pCinematic && (HOBJECT)pData == m_pCinematic->m_hObject)
			{
				m_pCinematic = NULL;
			}
			break;
		}
	}

	return BaseClass::EngineMessageFn(messageID, pData, fData);
}

BOOL CDialogueWindow::Init()
{
	if(m_bInitialized)
		return FALSE;

	m_bInitialized = TRUE;
	m_bPlaying = FALSE;
	m_byDecision = NULL;
	m_pCurSpeaker = NULL;
	m_pCinematic = NULL;
	return TRUE;
}

void CDialogueWindow::Term()
{
	if(!m_bInitialized)
		return;

	m_bInitialized = FALSE;
}

void CDialogueWindow::Finished(uint8 byDecision, uint32 dwDecision)
{
	m_bPlaying = FALSE;

	// Unlink us
	ASSERT(m_pCurSpeaker || m_pCinematic);
	if(m_pCinematic)
	{
		// Since CinematicTrigger::DialogueFinished can start another piece
		// of dialogue, we need to break our link first
		CinematicTrigger* pCin = m_pCinematic;
        g_pLTServer->BreakInterObjectLink(m_hObject, m_pCinematic->m_hObject);
		m_pCinematic = NULL;
		pCin->DialogueFinished(byDecision, dwDecision);
	}
	else
	if(m_pCurSpeaker)
	{
		m_pCurSpeaker->StopDialogue();
        g_pLTServer->BreakInterObjectLink(m_hObject, m_pCurSpeaker->m_hObject);
		m_pCurSpeaker = NULL;
	}
}

BOOL CDialogueWindow::PlayDialogue(uint32 dwID, CActor* pSpeaker, BOOL bStayOpen,
								   const char *szActorName, char *szDecisions)
{
	// Link us up
	ASSERT(pSpeaker);
	m_pCurSpeaker = pSpeaker;
    g_pLTServer->CreateInterObjectLink(m_hObject, pSpeaker->m_hObject);

	// Bring up a dialogue window on the client (NOTE: This goes to ALL CLIENTS)
	return(SendMessage(dwID,bStayOpen,szActorName,szDecisions));
}

BOOL CDialogueWindow::PlayDialogue(uint32 dwID, BOOL bStayOpen, const char *szActorName,
								   char *szDecisions, CinematicTrigger* pCinematic)
{
	// Link us up
	ASSERT(pCinematic);
	m_pCinematic = pCinematic;
    g_pLTServer->CreateInterObjectLink(m_hObject, m_pCinematic->m_hObject);

	// Bring up a dialogue window on the client (NOTE: This goes to ALL CLIENTS)
	return(SendMessage(dwID,bStayOpen,szActorName,szDecisions));
}
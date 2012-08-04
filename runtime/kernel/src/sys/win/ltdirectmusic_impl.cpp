/****************************************************************************

	MODULE:		LTDirectMusic_impl (.CPP)

	PURPOSE:	Implement DirectMusic capability for LithTech engine

	!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	!!! vvvvvvvvvvvvvvvvvvvvvvvvv READ THIS vvvvvvvvvvvvvvvvvvvvvvvvvvv !!!
	!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

	There are two sections of code at the start of this file.
	This file is shared between two locations and depending on where it is
	it needs to have the correct section un-commented.

	If you make changes to this file you must make sure it is updated in
	both places with the top section commented or un-commented as necessary.

	It exists in engine\engine\kernel\src\sys\win\ltdirectmusic_impl.cpp
	and tools\pc\src\ltdmtest\ltdirectmusic_impl.cpp.

	!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

	If NOLITHTECH is defined then special lithtech things are not done.

***************************************************************************/


// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
//	THIS CODE SHOULD BE UNCOMMENTED IF THIS FILE IS IN THE ENGINE
//	THIS CODE SHOULD BE COMMENTED OUT IF THIS FILE IS IN LTDMTEST
// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
#include "bdefs.h"
#include <process.h>
#include "dmksctrl.h"
#include "console.h"
#include "ltpvalue.h"
#include "lith.h"
#include "ltdirectmusic_impl.h"
#include "ltdirectmusiccontrolfile.h"
//instantiate our implemenation class of ILTDirectMusicMgr
define_interface(CLTDirectMusicMgr, ILTDirectMusicMgr);

// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
//	THIS CODE SHOULD BE UNCOMMENTED OUT IF THIS FILE IS IN LTDMTEST
//	THIS CODE SHOULD BE COMMENTED IF THIS FILE IS IN THE ENGINE
// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
/*
#include "afxwin.h"
#include "lith.h"
#include "lithtmpl.h"
#include "ltbasedefs.h"
#include "dmksctrl.h"
#include <process.h>
#include "lith.h"
#include "ltdirectmusic_impl.h"
#include "ltdirectmusiccontrolfile.h"
#include "dmksctrl.h"
#ifdef _DEBUG
#include <crtdbg.h>
#include "dxerr8.h"
#endif

#ifndef NOLITHTECH
#define NOLITHTECH
#endif
#define DONTUSELTDIRECTSOUND
*/

// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
// THIS IS THE END OF THE CODE UNCOMMENTED STUFF
// EVERYTHING ELSE IS THE SAME NO MATTER WHERE THIS FILE IS LOCATED
// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
#define DEFAULT_PCHANNELS	256

// console output level for LTDirectMusic
// 0 - no output 
// 1 - only display errors 
// 2 - only display errors and warnings (default)
// 3 - also display objects that are loaded like segments styles as well as init and term calls with their parameters
// 4 - also display calls and parameters to all main interface functions like Play, Stop, ChangeIntensity, etc...
// 5 - also display file open information from loader
// 6 - also display file read and write information (this is a ton of output and should rarely be used)

#ifndef NOLITHTECH
extern int32 g_CV_LTDMConsoleOutput;
#else
extern signed int g_CV_LTDMConsoleOutput;
extern void DebugConsoleOutput(const char* msg, uint8 nRed = 255, uint8 nBlue = 255, uint8 nGreen = 255);
#endif

#define SAFE_RELEASE(p)      { if(p) { (p)->Release(); (p)=NULL; } }

// output error to console
void LTDMConOutError(char *pMsg, ...)
{
	if (g_CV_LTDMConsoleOutput >= 1)
	{
		char msg[500] = "";
		va_list marker;

		va_start(marker, pMsg);
		LTVSNPrintF(msg, sizeof(msg), pMsg, marker);
		va_end(marker);

#ifndef NOLITHTECH
		if (msg[strlen(msg)-1] == '\n') msg[strlen(msg)-1] = '\0';
		con_PrintString(CONRGB(255, 0, 128), 0, msg);
#else
		DebugConsoleOutput(msg, 255, 0, 0);
#endif
	}
}

// output warning to console
void LTDMConOutWarning(char *pMsg, ...)
{
	if (g_CV_LTDMConsoleOutput >= 2)
	{
		char msg[500] = "";
		va_list marker;

		va_start(marker, pMsg);
		LTVSNPrintF(msg, sizeof(msg), pMsg, marker);
		va_end(marker);

#ifndef NOLITHTECH
		if (msg[strlen(msg)-1] == '\n') msg[strlen(msg)-1] = '\0';
		con_PrintString(CONRGB(0, 255, 128), 0, msg);
#else
		DebugConsoleOutput(msg, 0, 255, 0);
#endif
	}
}

// output message to console
void LTDMConOutMsg(int nLevel, char *pMsg, ...)
{
	if (g_CV_LTDMConsoleOutput >= nLevel)
	{
		char msg[500] = "";
		va_list marker;

		va_start(marker, pMsg);
		LTVSNPrintF(msg, sizeof(msg), pMsg, marker);
		va_end(marker);

#ifndef NOLITHTECH
		if (msg[strlen(msg)-1] == '\n') msg[strlen(msg)-1] = '\0';
		con_PrintString(CONRGB(128, 255, 128), 0, msg);
#else
		DebugConsoleOutput(msg, 0, 0, 0);
#endif
	}
}

// if the mgr is not initialized we fail
void LTDMConOutMgrNotInitialized(const char* sFuncName)
{
	LTDMConOutError("ERROR! LTDirectMusic not Initialized (%s)\n", sFuncName);
}

// we will fail if level has not been initialized
void LTDMConOutLevelNotInitialized(const char* sFuncName)
{
	LTDMConOutError("ERROR! LTDirectMusic not Initialized (%s)\n", sFuncName);
}


///////////////////////////////////////////////////////////////////////////////////////////
// macro for converting from normal to wide strings
///////////////////////////////////////////////////////////////////////////////////////////
#define MULTI_TO_WIDE( x,y )	MultiByteToWideChar( CP_ACP, MB_PRECOMPOSED, y, -1, x, _MAX_PATH );


///////////////////////////////////////////////////////////////////////////////////////////
// define the static chunk allocators  
///////////////////////////////////////////////////////////////////////////////////////////
CLithChunkAllocator<CLTDirectMusicMgr::CCommandItem> CLTDirectMusicMgr::CCommandItem::m_ChunkAllocator;
CLithChunkAllocator<CLTDirectMusicMgr::CSegment> CLTDirectMusicMgr::CSegment::m_ChunkAllocator;
CLithChunkAllocator<CLTDirectMusicMgr::CSegmentState> CLTDirectMusicMgr::CSegmentState::m_ChunkAllocator;
CLithChunkAllocator<CLTDirectMusicMgr::CBand> CLTDirectMusicMgr::CBand::m_ChunkAllocator;
CLithChunkAllocator<CLTDirectMusicMgr::CStyle> CLTDirectMusicMgr::CStyle::m_ChunkAllocator;
CLithChunkAllocator<CLTDirectMusicMgr::CDLSBank> CLTDirectMusicMgr::CDLSBank::m_ChunkAllocator;


///////////////////////////////////////////////////////////////////////////////////////////
// used for the notification thread for DirectMusic notifications
///////////////////////////////////////////////////////////////////////////////////////////

void CLTDirectMusicMgr::HandleSegmentNotification_SegAlmostEnd( DMUS_NOTIFICATION_PMSG* pPmsg, IDirectMusicSegmentState8* pDMSegState, IDirectMusicSegment* pDMSegment)
{
	// check if this notification came from a secondary segment
	{
		// check for secondary segment in list
		CSegmentState* pSegState = m_lstSecondarySegmentsPlaying.Find(pDMSegState);

		// if segment was found then we should not process this message
		if (pSegState != NULL) return;
	}

	// check if this notification came from a MOTIF
	{
		// check for MOTIF in list
		CSegmentState* pSegState = m_lstMotifsPlaying.Find(pDMSegState);

		// if motif was found then we should not process this message
		if (pSegState != NULL) return;
	}

	// make sure we are on the last played primairy segment state
	// otherwise we should not be processing the command loop
	{
		// get the first segment state
		CSegmentState* pFirstSegState = m_lstPrimairySegmentsPlaying.GetFirst();

		// if there is an item in the primairy segment playing list
		if (pFirstSegState != NULL)
		{
			// if this is not a notification for the last segment state in the list
			if (pFirstSegState->GetDMSegmentState() != pDMSegState) return;
		}
	}

	// holds the current command we are working on
	CCommandItem* pCurCommand;

	// check if there was a previous command 
	if (m_pLastCommand != LTNULL)
	{
		// go to the next command
		pCurCommand = m_pLastCommand->Next();
	}
	else
	{
		// get the first command
		pCurCommand = m_lstCommands.GetFirst();
	}

	// loop to execute commands
	bool bDoneProcessing = false;
	while (!bDoneProcessing && (pCurCommand != LTNULL))
	{
		switch ( pCurCommand->GetCommandType() )
		{
			case LTDMCommandLoopToStart :
			{
				// change command pointer to a loop pointer
				CCommandItemLoopToStart* pCmdLoop = (CCommandItemLoopToStart*)pCurCommand;

				// check if we were finished looping previously
				if (pCmdLoop->GetNumLoops() == 0)
				{
					// go to the next command
					pCurCommand = pCurCommand->Next();
					break;
				}

				// check if we need to decrement the loop counter
				if (pCmdLoop->GetNumLoops() > 0)
				{
					pCmdLoop->SetNumLoops(pCmdLoop->GetNumLoops()-1);
				}

				// check if we are finished looping
				if (pCmdLoop->GetNumLoops() == 0)
				{
					// go to the next command
					pCurCommand = pCurCommand->Next();
					break;
				}

				// if this was not the last loop
				else
				{
					// go to start of command queue
					pCurCommand = m_lstCommands.GetFirst();
				}

				break;
			}

			case LTDMCommandChangeIntensity :
			{
				// change command pointer to a loop pointer
				CCommandChangeIntensity* pCmdChangeIntensity = (CCommandChangeIntensity*)pCurCommand;

				// change the intensity
				if (ChangeIntensity(pCmdChangeIntensity->GetNewIntensity()) == LT_OK)
				{
					// Changing intensity clears the command queue, so reset the current command
					pCurCommand = m_pLastCommand;

					// we are done processing commands for now
					bDoneProcessing = true;
				}

				break;
			}

			case LTDMCommandPauseQueue :
			{
				// we are done processing commands for now
				bDoneProcessing = true;
				break;
			}

			case LTDMCommandPlaySegment :
			{
				// change command pointer to a loop pointer
				CCommandItemPlaySegment* pCmdSeg = (CCommandItemPlaySegment*)pCurCommand;

				// new directmusic state to get when playing the segment
				IDirectMusicSegmentState* pDMSegmentState = LTNULL;
				IDirectMusicSegmentState8* pDMSegmentState8 = LTNULL;

				// play this new segment queued 
				m_pPerformance->PlaySegment( pCmdSeg->GetSegment()->GetDMSegment(), DMUS_SEGF_SEGMENTEND, 0, &pDMSegmentState );

				// Get the IDirectMusicSegmentState8 interface
				HRESULT hr = pDMSegmentState->QueryInterface(IID_IDirectMusicSegmentState8,
					(void**)&pDMSegmentState8);

				if ( SUCCEEDED( hr ) )
					// create new segment state in the primairy segment state list
					m_lstPrimairySegmentsPlaying.CreateSegmentState(pDMSegmentState8, pCmdSeg->GetSegment());

				// we are done processing commands for now
				bDoneProcessing = true;
				break;
			}

			case LTDMCommandStopSegment :
			{
				// change command pointer to a loop pointer
				CCommandItemStopSegment* pCmdSeg = (CCommandItemStopSegment*)pCurCommand;

				// stop this segment 
				m_pPerformance->Stop( pCmdSeg->GetSegment()->GetDMSegment(), LTNULL, 0, DMUS_SEGF_MEASURE );

				// go to the next command
				pCurCommand = pCurCommand->Next();

				break;
			}

			case LTDMCommandPlayTransition :
			{
				// change command pointer to a loop pointer
				CCommandItemPlayTransition* pCmdSeg = (CCommandItemPlayTransition*)pCurCommand;

				// new directmusic state to get when playing the segment
				IDirectMusicSegmentState* pDMSegmentState = LTNULL;
				IDirectMusicSegmentState8* pDMSegmentState8 = LTNULL;

				// play this new segment queued 
				m_pPerformance->PlaySegment( pCmdSeg->GetTransition()->GetDMSegment(), DMUS_SEGF_MEASURE, 0, &pDMSegmentState );

				// Get the IDirectMusicSegmentState8 interface
				HRESULT hr = pDMSegmentState->QueryInterface(IID_IDirectMusicSegmentState8,
					(void**)&pDMSegmentState8);

				// create new segment state in the primairy segment state list
				if ( SUCCEEDED( hr ) )
					m_lstPrimairySegmentsPlaying.CreateSegmentState(pDMSegmentState8, NULL);

				// delete this command from the command queue
				m_lstCommands.Delete(pCurCommand);

				// set the last command to start of the command queue
				pCurCommand = NULL;

				// We're done processing
				bDoneProcessing = true;
				break;
			}

			default :
			{
				// these commands have not been implemented so just skip them
				// LTDMCommandNull
				// LTDMCommandStopPlaying
				// LTDMCommandAdjustVolume
				// LTDMCommandPlaySecondarySegment
				// LTDMCommandPlayMotif
				// LTDMCommandClearOldCommands

				// go to the next command
				pCurCommand = pCurCommand->Next();
				break;
			}
		}
	}

	// update the last command pointer
	m_pLastCommand = pCurCommand;

	// go through secondary command queue
	// just used for secondary segment and motif play commands
	pCurCommand = m_lstCommands2.GetFirst();

	// loop to execute commands
	while (pCurCommand != LTNULL)
	{
		switch ( pCurCommand->GetCommandType() )
		{
			case LTDMCommandPlaySecondarySegment :
			{
				// change command pointer to a loop pointer
				CCommandItemPlaySecondarySegment* pCmdSeg = (CCommandItemPlaySecondarySegment*)pCurCommand;

				// new directmusic state to get when playing the segment
				IDirectMusicSegmentState8* pDMSegmentState8 = LTNULL;
				IDirectMusicSegmentState* pDMSegmentState = LTNULL;

				// play this new segment queued 
				m_pPerformance->PlaySegment( pCmdSeg->GetSegment()->GetDMSegment(), DMUS_SEGF_SECONDARY | DMUS_SEGF_MEASURE | DMUS_SEGF_CONTROL, 0, &pDMSegmentState );

				// Get the IDirectMusicSegmentState8 interface
				HRESULT hr = pDMSegmentState->QueryInterface(IID_IDirectMusicSegmentState8,
					(void**)&pDMSegmentState8);

				if ( SUCCEEDED( hr ) )
					// create new segment state in the secondary segment state list
					m_lstSecondarySegmentsPlaying.CreateSegmentState(pDMSegmentState8, pCmdSeg->GetSegment());

				break;
			}

			case LTDMCommandPlayMotif :
			{
				// change command pointer to a loop pointer
				CCommandItemPlayMotif* pCmdSeg = (CCommandItemPlayMotif*)pCurCommand;

				// new directmusic state to get when playing the segment
				IDirectMusicSegmentState* pDMSegmentState = LTNULL;
				IDirectMusicSegmentState8* pDMSegmentState8 = LTNULL;

				// play this new segment queued 
				m_pPerformance->PlaySegment( pCmdSeg->GetSegment()->GetDMSegment(), DMUS_SEGF_SECONDARY | DMUS_SEGF_MEASURE, 0, &pDMSegmentState );

				// Get the IDirectMusicSegmentState8 interface
				HRESULT hr = pDMSegmentState->QueryInterface(IID_IDirectMusicSegmentState8,
					(void**)&pDMSegmentState8);

				if ( SUCCEEDED( hr ) )
					// create new segment state in the secondary segment state list
					m_lstMotifsPlaying.CreateSegmentState(pDMSegmentState8, pCmdSeg->GetSegment());

				break;
			}

			default :
			{
				break;
			}
		}

		// go to the next command
		CCommandItem* pNextCommand = pCurCommand->Next();

		// delete this command from the command queue
		m_lstCommands2.Delete(pCurCommand);

		// set the next command
		pCurCommand = pNextCommand;
	}
}

void CLTDirectMusicMgr::HandleSegmentNotification_SegEnd( DMUS_NOTIFICATION_PMSG* pPmsg, IDirectMusicSegmentState8* pDMSegState, IDirectMusicSegment* pDMSegment)
{
	// make sure directmusic segment state exists
	if (pDMSegState == LTNULL)
		return;

	// search for a primairy segment that matches this segment state pointer
	CSegmentState* pFindSegState = m_lstPrimairySegmentsPlaying.Find(pDMSegState);

	// if we found our primairy segment
	if (pFindSegState != LTNULL)
	{
		// clean up the segment state
		m_lstPrimairySegmentsPlaying.CleanupSegmentState(this, pFindSegState, LTDMEnactInvalid);
	}

	else
	{
		// search for a secondary segment that matches this segment pointer for this segment
		CSegmentState* pFindSegState = m_lstSecondarySegmentsPlaying.Find(pDMSegState);

		// if we found our secondary segment
		if (pFindSegState != LTNULL)
		{
			// clean up the segment state
			m_lstSecondarySegmentsPlaying.CleanupSegmentState(this, pFindSegState);
		}

		// if we didn't find it
		else
		{
			// search for a motif segment that matches this segment pointer for this segment
			CSegmentState* pFindSegState = m_lstMotifsPlaying.Find(pDMSegState);

			// if we found our motif segment
			if (pFindSegState != LTNULL)
			{
				// clean up the motif segment state
				m_lstMotifsPlaying.CleanupSegmentState(this, pFindSegState);
			}
		}	
	}
}

void CLTDirectMusicMgr::HandleSegmentNotification( DMUS_NOTIFICATION_PMSG* pPmsg)
{
	CWinSync_CSAuto cProtection(m_CommandQueueCriticalSection);

	IDirectMusicSegmentState8* pDMSegState = LTNULL;
	IDirectMusicSegment* pDMSegment = LTNULL;

	// make sure the punkUser field is not null
	if (pPmsg->punkUser != LTNULL)
	{
		// set up the segment state
		pDMSegState = (IDirectMusicSegmentState8*)pPmsg->punkUser;

		// set up the segment
		if (FAILED(pDMSegState->GetSegment(&pDMSegment))) pDMSegment = LTNULL;
	}
	else
	{
		ASSERT(!"pDMSegState/pDMSegment needs to be static across calls to HandleSegmentNotification");
	}

	switch (pPmsg->dwNotificationOption)
	{
		case DMUS_NOTIFICATION_SEGSTART :
		{
			// new segment has started so we don't need to worry
			// about the previous one
			m_nPrevIntensity = 0;
			break;
		}

		// if this is a notification of a segment about to finish
		case DMUS_NOTIFICATION_SEGALMOSTEND :
		{
			HandleSegmentNotification_SegAlmostEnd(pPmsg, pDMSegState, pDMSegment);
			break;
		}

		// if this is a segment finished notification
		case DMUS_NOTIFICATION_SEGEND:
		{
			HandleSegmentNotification_SegEnd(pPmsg, pDMSegState, pDMSegment);
			break;
		}

		default :
		{
			break;
		}
	}
}

DWORD CLTDirectMusicMgr::CommandThread_Bootstrap(void *pData)
{
	CLTDirectMusicMgr *pMgr = reinterpret_cast<CLTDirectMusicMgr *>(pData);
	return pMgr->CommandThread();
}

uint32 CLTDirectMusicMgr::CommandThread()
{
	// initialize com for this thread - more voodoo ? than what is required ... not sure
	if (!FAILED(CoInitialize(LTNULL)))
	{
	
		DMUS_NOTIFICATION_PMSG* pPmsg;

		HANDLE aWait[2];
		aWait[0] = m_cExitNotificationThread.GetEvent();
		aWait[1] = m_cNotify.GetEvent();;

		// loop until our exit flag is set
		while (WaitForMultipleObjects(2, aWait, FALSE, INFINITE) != WAIT_OBJECT_0)
		{
			// loop through all directmusic notification messages
			while (S_OK == m_pPerformance->GetNotificationPMsg(&pPmsg))
			{

				// if this is a segment notification
				if (pPmsg->guidNotificationType == GUID_NOTIFICATION_SEGMENT)
				{
					HandleSegmentNotification(pPmsg);
				}

				// free the notification message we are finished with it
					m_pPerformance->FreePMsg((DMUS_PMSG*)pPmsg); 
			}
		}

		// stop directmusic notifications
		m_pPerformance->SetNotificationHandle(0, 0);

		CoUninitialize();

		return 0;

	}
	else
	{
		return 1;
	}
	
}


///////////////////////////////////////////////////////////////////////////////////////////
// default constructor
///////////////////////////////////////////////////////////////////////////////////////////
CLTDirectMusicMgr::CLTDirectMusicMgr()
{
	// mgr is initially not initialized
	m_bInitialized = false;
	m_bLevelInitialized = false;

	// set pointers to initial LTNULL value
	m_pDirectMusic = LTNULL;
	m_pLoader = LTNULL;
	m_pPerformance = LTNULL;
	m_pAudiopath = LTNULL;
	m_pAudiopathBuffer = LTNULL;
	m_pReverb = LTNULL;
	m_aryTransitions = LTNULL;
	m_aryIntensities = LTNULL;
	m_sWorkingDirectoryAny = LTNULL;
	m_sWorkingDirectoryControlFile = LTNULL;
	m_nNumPChannels = DEFAULT_PCHANNELS;
	m_nNumVoices = 64;
	m_nSynthSampleRate = 44100;

	ZeroMemory( &m_ReverbParameters, sizeof( DSFXWavesReverb ) );
	ZeroMemory(	&m_ReverbDesc, sizeof( DSEFFECTDESC ) );

#ifdef NOLITHTECH
    m_sRezFileName = LTNULL;
#endif
	m_pLastCommand = LTNULL;

	m_nPaused = 0;

	m_hThread = NULL;
};


///////////////////////////////////////////////////////////////////////////////////////////
// default destructor (calls Term if it has not been called)
///////////////////////////////////////////////////////////////////////////////////////////
CLTDirectMusicMgr::~CLTDirectMusicMgr()
{
	// call term if the class is still initialized
	if (m_bInitialized) Term();
};


///////////////////////////////////////////////////////////////////////////////////////////
// Initialize the Mgr getting initial parameters from the described control file
///////////////////////////////////////////////////////////////////////////////////////////
LTRESULT CLTDirectMusicMgr::Init()
{

//	_CrtSetBreakAlloc(309);

	LTDMConOutMsg(3, "CLTDirectMusicMgr::Init\n");

	// init will fail if the Mgr is already initialized
	if (m_bInitialized) 
	{
		LTDMConOutError("ERROR! CLTDirectMusicMgr alread initialized. (CLTDirectMusicMgr::Init)\n");

		return LT_ERROR;
	}

	// initialize com, if not we fail
	if (FAILED(CoInitialize(LTNULL))) 
	{
		LTDMConOutError("ERROR! Unable to initialize com. (CLTDirectMusicMgr::Init)\n");

		return LT_ERROR;
	}

	if (!InitDirectMusic()) 
	{
		LTDMConOutError("ERROR! Unable to initialize DirectMusic. (CLTDirectMusicMgr::Init)\n");

	    // Release COM.
	    CoUninitialize();

		return LT_ERROR;
	}
	
	// set up the notification GUID
	m_guid = GUID_NOTIFICATION_SEGMENT;

	// add the notification to directmusic
	m_pPerformance->AddNotificationType(m_guid);

	// set the notification handle
	m_pPerformance->SetNotificationHandle(m_cNotify.GetEvent(), -1);

	// this should be false until we are read to exit the thread
	m_cExitNotificationThread.Clear();

	// set up chunk allocators
	LT_MEM_TRACK_ALLOC(CCommandItem::m_ChunkAllocator.Init(100,1), LT_MEM_TYPE_MUSIC);
	LT_MEM_TRACK_ALLOC(CSegment::m_ChunkAllocator.Init(20,1), LT_MEM_TYPE_MUSIC);
	LT_MEM_TRACK_ALLOC(CSegmentState::m_ChunkAllocator.Init(10,1), LT_MEM_TYPE_MUSIC);
	LT_MEM_TRACK_ALLOC(CBand::m_ChunkAllocator.Init(5,0), LT_MEM_TYPE_MUSIC);
	LT_MEM_TRACK_ALLOC(CStyle::m_ChunkAllocator.Init(5,0), LT_MEM_TYPE_MUSIC);
	LT_MEM_TRACK_ALLOC(CDLSBank::m_ChunkAllocator.Init(5,0), LT_MEM_TYPE_MUSIC);

	// Begin the notification thread (I would like to check to see if this was successful but
	// the documentation states that it returns an unsigned long and returns -1 if an error
	// occurs.  Since this is not possible I am not sure what to check for.)
	unsigned long nThreadID;
	m_hThread = CreateThread(NULL, 0, CommandThread_Bootstrap, (void*)this, LTNULL, &nThreadID);

	// set Mgr to initialized we have succeeded
	m_bInitialized = true;

	// return LT_OK we have succeeded
	return LT_OK;
};


///////////////////////////////////////////////////////////////////////////////////////////
// Terminate the mgr
///////////////////////////////////////////////////////////////////////////////////////////
LTRESULT CLTDirectMusicMgr::Term()
{
	LTDMConOutMsg(3, "CLTDirectMusicMgr::Term\n");

	// if we are not initialized then just exit 
	if (!m_bInitialized) 
	{
		LTDMConOutWarning("WARNING! LTDirectMusicMgr already terminated or never initialized. (CLTDirectMusicMgr::Term)\n");

		return LT_OK;
	}

	// check if the level has been terminated, if not we must terminate it
	if (m_bLevelInitialized)
	{
		TermLevel();
	}

	// signal the notification thread that it is time to exit
	m_cExitNotificationThread.Set();

	// wait for the thread to exit (wait up to 1 second)
	if (WaitForSingleObject(m_hThread, 1000) != WAIT_OBJECT_0)
	{
		// if waiting for thread to exit failed we ought to put out some kind of warning that we gave up waiting
		LTDMConOutWarning("WARNING! LTDirectMusicMgr thread did not exit on time!\n");
	}

	// Release our handle
	CloseHandle( m_hThread );

	m_hThread = NULL;

	// Terminate directmusic 
	TermDirectMusic();

   // Release COM.
   CoUninitialize();
	

	// terminate chunk allocators
	CCommandItem::m_ChunkAllocator.Term();
	CSegment::m_ChunkAllocator.Term();
   CSegmentState::m_ChunkAllocator.Term();
	CBand::m_ChunkAllocator.Term();
	CStyle::m_ChunkAllocator.Term();
	CDLSBank::m_ChunkAllocator.Term();

	// set initialized to false we are finished
	m_bInitialized = false;

	// return LT_OK we have succeeded
	return LT_OK;
};


///////////////////////////////////////////////////////////////////////////////////////////
// Initialize a game level using the parameters in the given control file
///////////////////////////////////////////////////////////////////////////////////////////
LTRESULT CLTDirectMusicMgr::InitLevel(const char* sWorkingDirectory, const char* sControlFileName, const char* sDefine1,
		  						     const char* sDefine2, const char* sDefine3)
{
	LTDMConOutMsg(3, "CLTDirectMusicMgr::InitLevel sWorkingDirectory=%s sControlFileName=%s sDefine1=%s sDefine2=%s sDeine3=%s\n",
					  sWorkingDirectory, sControlFileName, sDefine1, sDefine2, sDefine3);

	// control file mgr object used for reading the control file
#ifdef NOLITHTECH
	CControlFileMgrRezFile controlFile(m_sRezFileName);
#else
	CControlFileMgrDStream controlFile;
#endif

	// if the mgr is not initialized we fail
	if (m_bInitialized == false) 
	{
		LTDMConOutMgrNotInitialized("CLTDirectMusicMgr::InitLevel"); 
		return LT_ERROR;
	}

	// we will fail if InitLevel has already been called and has not been terminated
	if (m_bLevelInitialized) { LTDMConOutError("ERROR! InitLevel has already been called and not terminated. (CLTDirectMusicMgr::InitLevel)\n"); return LT_ERROR; }

	// make sure that control file name is not null
	if (sControlFileName == LTNULL) {	LTDMConOutError("ERROR! Control file name not valid. (CLTDirectMusicMgr::InitLevel)\n"); return LT_ERROR; }

	// make sure that working directory file name is not null
	if (sWorkingDirectory == LTNULL) { LTDMConOutError("ERROR! Working directory not valid. (CLTDirectMusicMgr::InitLevel)\n"); return LT_ERROR; }

	// set working directory name
	SetWorkingDirectory(sWorkingDirectory);

	// set defines for control file if they are used
	if (sDefine1 != LTNULL) controlFile.AddDefine(sDefine1);
	if (sDefine2 != LTNULL) controlFile.AddDefine(sDefine2);
	if (sDefine3 != LTNULL) controlFile.AddDefine(sDefine3);

	// check if control file name contains no path
	if ((strchr(sControlFileName,'\\') == LTNULL) && (strchr(sControlFileName,'/') == LTNULL) &&
		(strchr(sControlFileName,':') == LTNULL))
	{
		// create a string to hold the new control file name
		char * sNewControlFileName;
		uint32 nNewControlFileNameLen = strlen(sWorkingDirectory)+strlen(sControlFileName)+2;
		LT_MEM_TRACK_ALLOC(sNewControlFileName = new char[nNewControlFileNameLen], LT_MEM_TYPE_MUSIC);
		if (sNewControlFileName == LTNULL) return LT_ERROR;

		// copy working directory to string
		LTStrCpy(sNewControlFileName, sWorkingDirectory, nNewControlFileNameLen);

		// see if we need to append a backslash to directory
		if (strlen(sWorkingDirectory) > 0)
		{
			int nLastPos = strlen(sWorkingDirectory)-1;
			if ((sWorkingDirectory[nLastPos] != '\\') && 
				(sWorkingDirectory[nLastPos] != '/') &&
				(sWorkingDirectory[nLastPos] != ':'))
			{
				// append backslash
				LTStrCat(sNewControlFileName, "\\", nNewControlFileNameLen);
			}
		}

		// append the file name
		LTStrCat(sNewControlFileName, sControlFileName, nNewControlFileNameLen);

		// initialize control file
		if (!controlFile.Init(sNewControlFileName)) 
		{ 
			LTDMConOutError("ERROR! Unable to read control file. (CLTDirectMusicMgr::InitLevel)\n"); 
			
			// delete the new control file name
			delete [] sNewControlFileName;

			return LT_ERROR; 
		}

		// delete the new control file name
		delete [] sNewControlFileName;
	}
	// control file name has a path so just use it
	else 
	{
		// initialize control file
		if (!controlFile.Init(sControlFileName)) 
        { 
            LTDMConOutError("ERROR! Unable to read control file. (CLTDirectMusicMgr::InitLevel)\n"); 
            return LT_ERROR; 
        }
	}

	// set misc variable defaults
	m_nNumIntensities = 0;
	m_nInitialIntensity = 0;
	m_nInitialVolume = 0;
	m_nVolumeOffset = 0;
	m_nNumPChannels = DEFAULT_PCHANNELS;
	m_nNumVoices = 64;
	m_nSynthSampleRate = 44100;

	// read in misc variables from control file if they are present
	controlFile.GetKeyVal(LTNULL, "NUMINTENSITIES", m_nNumIntensities);
	controlFile.GetKeyVal(LTNULL, "INITIALINTENSITY", m_nInitialIntensity);
	controlFile.GetKeyVal(LTNULL, "INITIALVOLUME", m_nInitialVolume);
	controlFile.GetKeyVal(LTNULL, "VOLUMEOFFSET", m_nVolumeOffset);
	controlFile.GetKeyVal(LTNULL, "PCHANNELS", m_nNumPChannels);
	controlFile.GetKeyVal(LTNULL, "VOICES", m_nNumVoices);
	controlFile.GetKeyVal(LTNULL, "SYNTHSAMPLERATE", m_nSynthSampleRate);


	// get the dls bank directory if user specified it and set it in the loader
	{
		CControlFileKey* pKey = controlFile.GetKey(LTNULL, "DLSBANKDIRECTORY");
		if (pKey != LTNULL)
		{
			CControlFileWord* pWord = pKey->GetFirstWord();
			if (pWord != LTNULL)
			{
				m_pLoader->SetSearchDirectory(CLSID_DirectMusicCollection, pWord->GetVal(), false);
			}
		}
	}

	// Initialize reverb parameters from control file
	InitReverb(controlFile);

	// if number of intensities is less than 1 or insensity array or transition matrix 
	// were not allocated we can not proceed
	if (m_nNumIntensities < 1) 
	{
		LTDMConOutError("ERROR! Number of intensities is not valid. NumIntensities = %i (CLTDirectMusicMgr::InitLevel)\n", m_nNumIntensities);

		// terminate the control file
		controlFile.Term();

		// exit function we have failed
		return LT_ERROR;
	}

	// allocate intensity array (we make one extra because we don't use the 0 index)
	LT_MEM_TRACK_ALLOC(m_aryIntensities = new CIntensity[m_nNumIntensities+1],LT_MEM_TYPE_MUSIC);

	// set up default intensity array values
	{
		// loop through all intensities in array
		for (int nLoop = 0; nLoop <= m_nNumIntensities; nLoop++)
		{
			m_aryIntensities[nLoop].SetNumLoops(0);
			m_aryIntensities[nLoop].SetIntensityToSetAtFinish(0);
		}
	}

	// calculate the number of transitions (we waste a column in each dimension beacuse
	// we don't use the zero index of the intensity array)
	//
	// Add 1 to take into account transitioning from the last intensity to the last 
	// intensity.  This is required because the GetTransition function appears to be 
	// adding 1 to ignore the first element in the table.
	m_nNumTransitions = (m_nNumIntensities+1)*(m_nNumIntensities+1) + 1;

	// allocate transition matrix
	LT_MEM_TRACK_ALLOC(m_aryTransitions = new CTransition[m_nNumTransitions],LT_MEM_TYPE_MUSIC);

	// set up default transition matrix values
	{
		// loop through all transitions in array
		for (int nLoop = 0; nLoop < m_nNumTransitions; nLoop++)
		{
			m_aryTransitions[nLoop].SetEnactTime(LTDMEnactNextMeasure);
			m_aryTransitions[nLoop].SetManual(true);
			m_aryTransitions[nLoop].SetDMSegment(LTNULL);
		}
	}

	// if insensity array or transition matrix were not allocated we can not proceed
	if ((m_aryIntensities == LTNULL) || (m_aryTransitions == LTNULL)) 
	{
		LTDMConOutError("ERROR! Unable to allocate memory for Intensity and Transition arrays. (CLTDirectMusicMgr::InitLevel)\n", m_nNumIntensities);

		// terminate the control file
		controlFile.Term();

		// free the intensity array
		if (m_aryIntensities != LTNULL) 
		{
			delete [] m_aryIntensities;
			m_aryIntensities = LTNULL;
		}

		// free the transition matrix
		if (m_aryTransitions != LTNULL)
		{
			delete [] m_aryTransitions;
			m_aryTransitions = LTNULL;
		}

		// exit function we have failed
		return LT_ERROR;
	}

	// read in and set up DLS Banks
	ReadDLSBanks(controlFile);

	// read in and load styles and bands
	ReadStylesAndBands(controlFile);

	// read in intensity descriptions
	ReadIntensities(controlFile);

	// read in secondary segments
	ReadSecondarySegments(controlFile);

	// read in motifs
	ReadMotifs(controlFile);

	// read in transition matrix
	ReadTransitions(controlFile);

	// set level initialized to true we have succeeded
	m_bLevelInitialized = true;

	// terminate the control file mgr
	controlFile.Term();

	// Set the initial volume
	SetVolume( m_nInitialVolume );

	// set the last command to LTNULL because it hasn't been done yet
	m_pLastCommand = LTNULL;

	// set up the current intensity
	m_nCurIntensity = 0;

	// set up the previous intensity
	m_nPrevIntensity = 0;

	// return LT_OK we have succeeded
	return LT_OK;
};


///////////////////////////////////////////////////////////////////////////////////////////
// Teminate the current game level
///////////////////////////////////////////////////////////////////////////////////////////
LTRESULT CLTDirectMusicMgr::TermLevel()
{
	LTDMConOutMsg(3, "CLTDirectMusicMgr::TermLevel\n");

	// if the mgr is not initialized we fail
	if (m_bInitialized == false) 
	{ 
		LTDMConOutMgrNotInitialized("CLTDirectMusicMgr::TermLevel"); 
		return LT_ERROR; 
	}

	// if level is not initialized just exit 
	if (!m_bLevelInitialized) 
	{
		LTDMConOutWarning("WARNING! Level already terminated or never initialized. (CLTDirectMusicMgr::TermLevel)\n");

		return LT_OK;
	}

	// enter critical section for messing with the command queue
	m_CommandQueueCriticalSection.Enter();

	// clear the command queue
	ClearCommands();

	// stop music playing
	Stop();

	// terminate any reverb effects
	TermReverb();

	// release all primairy and secondary segments
	m_lstSegments.CleanupSegments(this);

	// release all motif segments
	m_lstMotifs.CleanupSegments(this);

	// clean up all the primairy segment states
	m_lstPrimairySegmentsPlaying.CleanupSegmentStates(this);

	// clean up all the secondary segments 
	m_lstSecondarySegmentsPlaying.CleanupSegmentStates(this);

	// clean up all the motif segments 
	m_lstMotifsPlaying.CleanupSegmentStates(this);

	// exit critical section for messing with the command queue
	m_CommandQueueCriticalSection.Leave();

	// unload all bands
	{
		CBand* pBand;
		while ((pBand = m_lstBands.GetFirst()) != LTNULL)
		{
			// release the directmusic band segment
			pBand->GetDMSegment()->Release();

			// release the directmusic Band
			pBand->GetBand()->Unload(m_pPerformance);

			// remove our Band object from the Band list
			m_lstBands.Delete(pBand);

			// delete our Band object
			delete pBand;
		}
	}

	// rlease all style files
	{
		CStyle* pStyle;
		while ((pStyle = m_lstStyles.GetFirst()) != LTNULL)
		{
			// release the directmusic style
			pStyle->GetDMStyle()->Release();

			// remove our style object from the style list
			m_lstStyles.Delete(pStyle);

			// delete our style object
			delete pStyle;
		}
	}

	// unload all dls banks
	{
		CDLSBank* pDLSBank;
		while ((pDLSBank = m_lstDLSBanks.GetFirst()) != LTNULL)
		{
			// release the directmusic DLSBank
			pDLSBank->GetDLSBank()->Release();

			// remove our DLSBank object from the DLSBank list
			m_lstDLSBanks.Delete(pDLSBank);

			// delete our DLSBank object
			delete pDLSBank;
		}
	}

	// delete working directory string for any type
	if (m_sWorkingDirectoryAny != LTNULL)
	{
		delete [] m_sWorkingDirectoryAny;
		m_sWorkingDirectoryAny = LTNULL;
	}

	// delete working directory string for control files
	if (m_sWorkingDirectoryControlFile != LTNULL)
	{
		delete [] m_sWorkingDirectoryControlFile;
		m_sWorkingDirectoryControlFile = LTNULL;
	}

	// let loader clean up it's object lists
	m_pLoader->ClearObjectList();

#ifdef NOLITHTECH
	// delete working directory string for any type
	if (m_sRezFileName != LTNULL)
	{
		delete [] m_sRezFileName;
		m_sRezFileName = LTNULL;
	}
#endif

	// free the intensity array
	if (m_aryIntensities != LTNULL) 
	{
		delete [] m_aryIntensities;
		m_aryIntensities = LTNULL;
	}

	// free the transition matrix
	if (m_aryTransitions != LTNULL)
	{
		delete [] m_aryTransitions;
		m_aryTransitions = LTNULL;
	}

	// flush the directmusic cache here!!!!!

	// set level initalized to false we are done
	m_bLevelInitialized = false;

	return LT_OK;
};


///////////////////////////////////////////////////////////////////////////////////////////
// Begin playing music
///////////////////////////////////////////////////////////////////////////////////////////
LTRESULT CLTDirectMusicMgr::Play()
{
	LTDMConOutMsg(4, "CLTDirectMusicMgr::Play\n");

	// if the mgr is not initialized we fail
	if (m_bInitialized == false) { LTDMConOutMgrNotInitialized("CLTDirectMusicMgr::Play"); return LT_ERROR; }

	// we will fail if level has not been initialized
	if (m_bLevelInitialized == false) { LTDMConOutLevelNotInitialized("CLTDirectMusicMgr::Play"); return LT_ERROR; }

	// set the intensity to the initial intensity
	return ChangeIntensity(m_nInitialIntensity, LTDMEnactImmediately);
}


///////////////////////////////////////////////////////////////////////////////////////////
// Stop playing music
///////////////////////////////////////////////////////////////////////////////////////////
LTRESULT CLTDirectMusicMgr::Stop(const LTDMEnactTypes nStart)
{
	LTDMConOutMsg(4, "CLTDirectMusicMgr::Stop nStart=%i\n", (int)nStart);

	// if the mgr is not initialized we fail
	if (m_bInitialized == false) { LTDMConOutMgrNotInitialized("CLTDirectMusicMgr::Stop"); return LT_ERROR; }

	// we will fail if level has not been initialized
	if (m_bLevelInitialized == false) { LTDMConOutLevelNotInitialized("CLTDirectMusicMgr::Stop"); return LT_ERROR; }

	// make sure performance exists
	if (m_pPerformance != LTNULL)
	{
		// stop the performance (unless we are not supposed to stop until end of segment)
		if (nStart != LTDMEnactNextSegment) m_pPerformance->Stop( LTNULL, LTNULL, 0, EnactTypeToFlags(nStart) );
	}

	// Don't be paused
	while (m_nPaused)
		UnPause();

	// enter critical section for messing with the command queue
	CWinSync_CSAuto cProtection(m_CommandQueueCriticalSection);

	// clear command queue
	{
		CCommandItem* pCommand;
		while ((pCommand = m_lstCommands.GetFirst()) != LTNULL)
		{
			// remove our Command object from the Command list
			m_lstCommands.Delete(pCommand);

			// delete our Command object
			delete pCommand;
		}
	}

	// clear 2nd command queue
	{
		CCommandItem* pCommand;
		while ((pCommand = m_lstCommands2.GetFirst()) != LTNULL)
		{
			// remove our Command object from the Command list
			m_lstCommands2.Delete(pCommand);

			// delete our Command object
			delete pCommand;
		}
	}

	// clean up all primairy segment sates
	m_lstPrimairySegmentsPlaying.CleanupSegmentStates(this);

	// clean up all secondary segment states
	m_lstSecondarySegmentsPlaying.CleanupSegmentStates(this);

	// clean up all motif segment states
	m_lstMotifsPlaying.CleanupSegmentStates(this);

	// set last commnd to LTNULL
	m_pLastCommand = LTNULL;

	return LT_OK;
}


///////////////////////////////////////////////////////////////////////////////////////////
// Pause music playing
///////////////////////////////////////////////////////////////////////////////////////////
LTRESULT CLTDirectMusicMgr::Pause(const LTDMEnactTypes nStart)
{
	LTDMConOutMsg(4, "CLTDirectMusicMgr::Pause nStart=%i\n", (int)nStart);

	// if the mgr is not initialized we fail
	if (m_bInitialized == false) { LTDMConOutMgrNotInitialized("CLTDirectMusicMgr::Pause"); return LT_ERROR; }

	// we will fail if level has not been initialized
	if (m_bLevelInitialized == false) { LTDMConOutLevelNotInitialized("CLTDirectMusicMgr::Pause"); return LT_ERROR; }

	if (m_nPaused == 0)
	{
		REFERENCE_TIME nTime;
		if (FAILED(m_pPerformance->GetTime(&nTime, NULL)))
			return LT_ERROR;
		REFERENCE_TIME nEnactTime;
		m_pPerformance->GetResolvedTime(nTime, &nEnactTime, EnactTypeToFlags(nStart));

		MUSIC_TIME nEnactMusicTime;
		m_pPerformance->ReferenceToMusicTime(nEnactTime, &nEnactMusicTime);

		IDirectMusicSegmentState* pSegState;
		if (FAILED(m_pPerformance->GetSegmentState(&pSegState, nEnactMusicTime)))
			return LT_ERROR;

		MUSIC_TIME nStartTime;
		pSegState->GetStartPoint(&nStartTime);
		m_cCurPauseState.SetPlayTime(nEnactMusicTime - nStartTime);

		IDirectMusicSegment* pSegment;
		if (FAILED(pSegState->GetSegment(&pSegment)))
		{
			pSegState->Release();
			return LT_ERROR;
		}
		pSegState->Release();
		m_cCurPauseState.SetSegment(pSegment);
		// Let go of our reference to the segment
		pSegment->Release();

		m_pPerformance->Stop(0, 0, 0, 0);

		// Pause the thread
		SuspendThread(m_hThread);
	}

	++m_nPaused;

	return LT_OK;
}


///////////////////////////////////////////////////////////////////////////////////////////
// UnPause music playing
///////////////////////////////////////////////////////////////////////////////////////////
LTRESULT CLTDirectMusicMgr::UnPause()
{
	LTDMConOutMsg(4, "CLTDirectMusicMgr::UnPause\n");

	// if the mgr is not initialized we fail
	if (m_bInitialized == false) { /* LTDMConOutMgrNotInitialized("CLTDirectMusicMgr::UnPause"); */ return LT_ERROR; }

	// we will fail if level has not been initialized
	if (m_bLevelInitialized == false) { /* LTDMConOutLevelNotInitialized("CLTDirectMusicMgr::UnPause"); */ return LT_ERROR; }

	if (!m_nPaused)
		return LT_OVERFLOW;

	--m_nPaused;
	if (m_nPaused == 0)
	{
		m_cCurPauseState.GetSegment()->SetStartPoint(m_cCurPauseState.GetPlayTime());
		if (FAILED(m_pPerformance->PlaySegment(
					m_cCurPauseState.GetSegment(), 
					0, 
					0, NULL)))
		{
			return LT_ERROR;
		}

		// Release the stuff we saved so we could pause
		m_cCurPauseState.Clear();

		// Resume threading
		ResumeThread(m_hThread);
	}

	return LT_OK;
}


///////////////////////////////////////////////////////////////////////////////////////////
// Set current volume
///////////////////////////////////////////////////////////////////////////////////////////
LTRESULT CLTDirectMusicMgr::SetVolume(const long nVolume)
{
	// Add in the volume offset to the incoming volume.
	long nSetVolume = nVolume + m_nVolumeOffset;

	LTDMConOutMsg(4, "CLTDirectMusicMgr::SetVolume nVolume=%i\n", nVolume);

	// if the mgr is not initialized we fail
	if (m_bInitialized == false) { LTDMConOutMgrNotInitialized("CLTDirectMusicMgr::SetVolume"); return LT_ERROR; }

	// we will fail if level has not been initialized
	if (m_bLevelInitialized == false) { LTDMConOutLevelNotInitialized("CLTDirectMusicMgr::SetVolume"); return LT_ERROR; }

	// set the volume
	HRESULT hr = m_pPerformance->SetGlobalParam(GUID_PerfMasterVolume, &nSetVolume, sizeof(nVolume));

	// return LT_OK or false depending on if the called failed or not
	if (FAILED(hr)) return LT_ERROR;
	else return LT_OK;
}


///////////////////////////////////////////////////////////////////////////////////////////
// Change the intensity level
///////////////////////////////////////////////////////////////////////////////////////////
LTRESULT CLTDirectMusicMgr::ChangeIntensity(const int nNewIntensity, const LTDMEnactTypes nStart)
{
	IDirectMusicSegmentState* pDMSegmentState = LTNULL;
	IDirectMusicSegmentState8* pDMSegmentState8 = LTNULL;
	CTransition* pTransition;
	CSegment* pSegment = LTNULL;
	CSegment* pQueuedSegStart = LTNULL;
	uint32 nFlags;

	LTDMConOutMsg(4, "CLTDirectMusicMgr::ChangeIntensity nNewIntensity=%i nStart=%i\n", nNewIntensity, (int)nStart);

	// if the mgr is not initialized we fail
	if (m_bInitialized == false) { LTDMConOutMgrNotInitialized("CLTDirectMusicMgr::ChangeIntensity"); return LT_ERROR; }

	// we will fail if level has not been initialized
	if (m_bLevelInitialized == false) { LTDMConOutLevelNotInitialized("CLTDirectMusicMgr::ChangeIntensity"); return LT_ERROR; }

	// we fail if there is no performance
	if (m_pPerformance == LTNULL) return LT_ERROR;

	// if we asked for an intensity of 0 we really meant stop so stop sounds
	if (nNewIntensity == 0)
	{
		Stop(nStart);
		return LT_ERROR;
	}

	// we fail if intensity is not valid
	if ((nNewIntensity <= 0) || (nNewIntensity > m_nNumIntensities)) return LT_ERROR;

	// enter critical section for messing with the command queue
	CWinSync_CSAuto cProtection(m_CommandQueueCriticalSection);

	// we don't need to do anything if the new intensity is the same as the current intensity
	if (m_nCurIntensity == nNewIntensity) 
	{
		LTDMConOutMsg(4, "CLTDirectMusicMgr::ChangeIntensity new intensity same as old nothing to do, exiting.\n");

		return LT_OK;
	}

	// get the transition that we will be using

	// if we have a value for the previous transition,
	// then we're waiting for the new one to start
	// and we can't hear it yet, therefore,
	// grab the transition for the old intensity, so it sounds right.
	if ( m_nPrevIntensity )
		pTransition = GetTransition(m_nPrevIntensity, nNewIntensity);
	else
		pTransition = GetTransition(m_nCurIntensity, nNewIntensity);

	// keep track of the last intensity in case 
	// we have to make a change before the new one
	// starts, so we can play the correct transition
	m_nPrevIntensity = m_nCurIntensity;

	// make sure transition is valid
	if (pTransition == LTNULL) 
	{
		LTDMConOutError("ERROR! Invalid transition found, exiting. (CLTDirectMusicMgr::ChangeIntensity)\n");

		return LT_ERROR;
	}

	// clear command queue
	ClearCommands();

	// this is the enact time value we are going to use
	LTDMEnactTypes nEnactValue;

	// figure out which enact value to use
	if (nStart == LTDMEnactInvalid) nEnactValue = pTransition->GetEnactTime();
	else nEnactValue = nStart;

	// figure out the correct flags to pass into PlaySegment based on the enact time
	nFlags = EnactTypeToFlags(nEnactValue) | DMUS_SEGF_NOINVALIDATE;

	bool bSkipFirstInQueue = false;

	// if transition segment is not LTNULL then play it
	if (pTransition->GetDMSegment() != LTNULL)
	{
		// play segment
		m_pPerformance->PlaySegment(pTransition->GetDMSegment(), nFlags, 0, &pDMSegmentState );

		// [kml] Check that pointer!
		if(pDMSegmentState)
		{
		// Get the IDirectMusicSegmentState8 interface
		HRESULT hr = pDMSegmentState->QueryInterface(IID_IDirectMusicSegmentState8,
			(void**)&pDMSegmentState8);

		if ( SUCCEEDED( hr ) )
			// add segment state to list of primairy segment states that are playing
			m_lstPrimairySegmentsPlaying.CreateSegmentState(pDMSegmentState8, NULL);
		}

		// Queue the segments for this intensity
		pQueuedSegStart = m_aryIntensities[nNewIntensity].GetSegmentList().GetFirst();
	}

	// if there is no transition segment then play the first segment in the command queue
	else
	{
		// Play the first intensity segment
		pSegment = m_aryIntensities[nNewIntensity].GetSegmentList().GetFirst();
		while (pSegment != LTNULL)
		{
			// get the initial segment to play
			IDirectMusicSegment8* pStartDMSegment = pSegment->GetDMSegment();
			if (pStartDMSegment != LTNULL)
			{
				HRESULT hr;

				// play the segment
				hr = m_pPerformance->PlaySegment(pStartDMSegment, nFlags, 0, &pDMSegmentState  );

				// Get the IDirectMusicSegmentState8 interface
				hr = pDMSegmentState->QueryInterface(IID_IDirectMusicSegmentState8,
					(void**)&pDMSegmentState8);

				if ( SUCCEEDED( hr ) )
					// add segment state to list of primairy segment states that are playing
					m_lstPrimairySegmentsPlaying.CreateSegmentState(pDMSegmentState8, pSegment);

				pQueuedSegStart = pSegment;
				bSkipFirstInQueue = true;
				break;
			}
			else
				pSegment = pSegment->Next();
		}
		// If there's nothing valid to queue, don't.
		if (!pSegment)
			pQueuedSegStart = LTNULL;
	}

	// Queue the remaining segments for this intensity
	while (pQueuedSegStart != LTNULL)
	{
		// make sure the directmusic segment is not null
		if (pQueuedSegStart->GetDMSegment() != LTNULL)
		{
			// create a new command queue play segment item
			CCommandItemPlaySegment* pComItemSeg;
			LT_MEM_TRACK_ALLOC(pComItemSeg = new CCommandItemPlaySegment,LT_MEM_TYPE_MUSIC);

			// set the directmusic segment inside the new item
			pComItemSeg->SetSegment(pQueuedSegStart);

			// add the new item to the command queue
			m_lstCommands.InsertLast(pComItemSeg);
		}

		// get the next segment
		pQueuedSegStart = pQueuedSegStart->Next();
	}

	// Start the queue over next time something is processed
	if (bSkipFirstInQueue)
		m_pLastCommand = m_lstCommands.GetFirst();
	else
		m_pLastCommand = LTNULL;

	// add a loop command to the end of the command queue
	// create a new command to loop
	CCommandItemLoopToStart* pComItemLoop;
	LT_MEM_TRACK_ALLOC(pComItemLoop = new CCommandItemLoopToStart,LT_MEM_TYPE_MUSIC);

	// set the number of times to loop (-1 for infinite)
	int nNumLoops = m_aryIntensities[nNewIntensity].GetNumLoops();
	// If we're going to run this intensity again, do an infinite loop
	if (m_aryIntensities[nNewIntensity].GetIntensityToSetAtFinish() == nNewIntensity)
		nNumLoops = -1;
	pComItemLoop->SetNumLoops(nNumLoops);

	// add the new item to the command queue
	m_lstCommands.InsertLast(pComItemLoop);

	// add a change intensity command to the end of the command queue if it is present
	if (m_aryIntensities[nNewIntensity].GetIntensityToSetAtFinish() > 0)
	{
		// create a new command to change intensity
		CCommandChangeIntensity* pComItemChangeIntensity;
		LT_MEM_TRACK_ALLOC(pComItemChangeIntensity = new CCommandChangeIntensity,LT_MEM_TYPE_MUSIC);

		// set the intensity to change to
		pComItemChangeIntensity->SetNewIntensity(m_aryIntensities[nNewIntensity].GetIntensityToSetAtFinish());

		// add the new item to the command queue
		m_lstCommands.InsertLast(pComItemChangeIntensity);
	}

	// set the new intensity
	m_nCurIntensity = nNewIntensity;

	return LT_OK;
}


///////////////////////////////////////////////////////////////////////////////////////////
// Play a secondary segment
///////////////////////////////////////////////////////////////////////////////////////////
LTRESULT CLTDirectMusicMgr::PlaySecondary(const char* sSecondarySegment, const LTDMEnactTypes nStart)
{
	IDirectMusicSegmentState* pDMSegmentState = LTNULL;
	IDirectMusicSegmentState8* pDMSegmentState8 = LTNULL;
	CSegment* pMainSegment;
	LTDMEnactTypes nEnactTime = nStart;

	LTDMConOutMsg(4, "CLTDirectMusicMgr::PlaySecondary sSecondarySegment=%s nStart=%i\n", sSecondarySegment, (int)nStart);

	// if the mgr is not initialized we fail
	if (m_bInitialized == false) { LTDMConOutMgrNotInitialized("CLTDirectMusicMgr::PlaySecondary"); return LT_ERROR; }

	// we will fail if level has not been initialized
	if (m_bLevelInitialized == false) { LTDMConOutLevelNotInitialized("CLTDirectMusicMgr::PlaySecondary"); return LT_ERROR; }

	// find the main segment
	pMainSegment = m_lstSegments.Find(sSecondarySegment);

	// if the segment is not found we can't play it so fail
	if (pMainSegment == LTNULL) return LT_ERROR;

	// make sure the main segment has a directmusic segment
	if (pMainSegment->GetDMSegment() == LTNULL) return LT_ERROR;

	// if default enact time was passed in then get default enact time from the original segment
	if (nStart == LTDMEnactDefault) nEnactTime = pMainSegment->GetDefaultEnact();

	// enter critical section for messing with the command queue
	CWinSync_CSAuto cProtection(m_CommandQueueCriticalSection);

	// play the new segment (unless we are not supposed to play until the next segment)
	if (nEnactTime != LTDMEnactNextSegment) 
	{
		// play segment
		m_pPerformance->PlaySegment(pMainSegment->GetDMSegment(), DMUS_SEGF_SECONDARY | EnactTypeToFlags(nEnactTime) | DMUS_SEGF_CONTROL, 0, &pDMSegmentState );

		// Get the IDirectMusicSegmentState8 interface
		HRESULT hr = pDMSegmentState->QueryInterface(IID_IDirectMusicSegmentState8,
				(void**)&pDMSegmentState8);

		if ( SUCCEEDED( hr ) )
			// add segment state to list of seceondary segment states that are playing
			m_lstSecondarySegmentsPlaying.CreateSegmentState(pDMSegmentState8, pMainSegment);
	}

	// if we are supposed to wait until the end of the segment then queue up a play secondary command
	else
	{
		// create a new command queue play segment item
		CCommandItemPlaySecondarySegment* pComItemSeg;
		LT_MEM_TRACK_ALLOC(pComItemSeg = new CCommandItemPlaySecondarySegment,LT_MEM_TYPE_MUSIC);

		// make sure allocation worked
		if (pComItemSeg != LTNULL)
		{
			// set the directmusic segment inside the new item
			pComItemSeg->SetSegment(pMainSegment);

			// add the new item to the command queue so it will be executed next when a segment ends
			m_lstCommands2.Insert(pComItemSeg);
		}
	}

	return LT_OK;
}


///////////////////////////////////////////////////////////////////////////////////////////
// Stop all secondary segments with the specified name (if LTNULL it stops them all)
///////////////////////////////////////////////////////////////////////////////////////////
LTRESULT CLTDirectMusicMgr::StopSecondary(const char* sSecondarySegment, const LTDMEnactTypes nStart)
{
	CSegmentState* pStopSegmentState;
	CSegmentState* pNextSegmentState;
	LTDMEnactTypes nEnactTime = nStart;

	LTDMConOutMsg(4, "CLTDirectMusicMgr::StopSecondary sSecondarySegment=%s nStart=%i\n", sSecondarySegment, (int)nStart);

	// if the mgr is not initialized we fail
	if (m_bInitialized == false) { LTDMConOutMgrNotInitialized("CLTDirectMusicMgr::StopSecondary"); return LT_ERROR; }

	// we will fail if level has not been initialized
	if (m_bLevelInitialized == false) { LTDMConOutLevelNotInitialized("CLTDirectMusicMgr::StopSecondary"); return LT_ERROR; }

	// enter critical section for messing with the command queue
	CWinSync_CSAuto cProtection(m_CommandQueueCriticalSection);

	// if name is LTNULL then stop them all
	if (sSecondarySegment == LTNULL)
	{
		// get the first secondary segment
		pStopSegmentState = m_lstSecondarySegmentsPlaying.GetFirst();

		while (pStopSegmentState != LTNULL)
		{
			// get the next segment to process after this one
			pNextSegmentState = pStopSegmentState->Next();

			// make sure the directmusic segment pointer is OK if not leave
			if (pStopSegmentState->GetDMSegmentState() != LTNULL)
			{
				// if default enact time was passed in then get default enact time from the original segment
				if (nStart == LTDMEnactDefault) nEnactTime = pStopSegmentState->GetSegment()->GetDefaultEnact();

				// clean up this segment (unless it is supposed to play to the end)
				if (nEnactTime != LTDMEnactNextSegment) m_lstSecondarySegmentsPlaying.CleanupSegmentState(this, pStopSegmentState, nEnactTime);
		
				// update stop segment to next segment
				pStopSegmentState = pNextSegmentState;
			}
		}
	}

	// find the one we want to stop
	else
	{
		// loop until all of the segments with this name have been found
		for(;;)
		{

			// find the secondary segment
			pStopSegmentState = m_lstSecondarySegmentsPlaying.Find(sSecondarySegment);

			// make sure we found something
			if (pStopSegmentState != LTNULL)
			{
				// if default enact time was passed in then get default enact time from the original segment
				if (nStart == LTDMEnactDefault) nEnactTime = pStopSegmentState->GetSegment()->GetDefaultEnact();

				// make sure the directmusic segment pointer is OK if not leave
				if (pStopSegmentState->GetSegment()->GetDMSegment() != LTNULL)
				{
					// clean up this segment (unless it is supposed to play to the end)
					if (nEnactTime != LTDMEnactNextSegment) m_lstSecondarySegmentsPlaying.CleanupSegmentState(this, pStopSegmentState, nEnactTime);
				}
			}

			// time to exit loop
			else break;
		}
	}

	return LT_OK;
}


///////////////////////////////////////////////////////////////////////////////////////////
// Play a motif
///////////////////////////////////////////////////////////////////////////////////////////
LTRESULT CLTDirectMusicMgr::PlayMotif(const char* sMotifName, const LTDMEnactTypes nStart)
{
	IDirectMusicSegmentState* pDMSegmentState = LTNULL;
	IDirectMusicSegmentState8* pDMSegmentState8 = LTNULL;
	CSegment* pMainSegment;
	LTDMEnactTypes nEnactTime = nStart;

	LTDMConOutMsg(4, "CLTDirectMusicMgr::PlayMotif sMotifName=%s nStart=%i\n", sMotifName, (int)nStart);

	// if the mgr is not initialized we fail
	if (m_bInitialized == false) { LTDMConOutMgrNotInitialized("CLTDirectMusicMgr::PlayMotif"); return LT_ERROR; }

	// we will fail if level has not been initialized
	if (m_bLevelInitialized == false) { LTDMConOutLevelNotInitialized("CLTDirectMusicMgr::PlayMotif"); return LT_ERROR; }

	// find the main segment
	pMainSegment = m_lstMotifs.Find(sMotifName);

	// if the segment is not found we can't play it so fail
	if (pMainSegment == LTNULL) return LT_ERROR;

	// make sure the main segment has a directmusic segment
	if (pMainSegment->GetDMSegment() == LTNULL) return LT_ERROR;

	// if default enact time was passed in then get default enact time from the original segment
	if (nStart == LTDMEnactDefault) nEnactTime = pMainSegment->GetDefaultEnact();

	// play the new segment (unless we are not supposed to play until the next segment)
	if (nEnactTime != LTDMEnactNextSegment) 
	{
		// play segment
		m_pPerformance->PlaySegment(pMainSegment->GetDMSegment(), DMUS_SEGF_SECONDARY | EnactTypeToFlags(nEnactTime), 0, &pDMSegmentState );

		// Get the IDirectMusicSegmentState8 interface
		HRESULT hr = pDMSegmentState->QueryInterface(IID_IDirectMusicSegmentState8,
				(void**)&pDMSegmentState8);

		if ( SUCCEEDED( hr ) )
		{
			CWinSync_CSAuto cProtection(m_CommandQueueCriticalSection);

			// add segment state to list of motif segment states that are playing
			m_lstMotifsPlaying.CreateSegmentState(pDMSegmentState8, pMainSegment);
		}
	}

	// if we are supposed to wait until the end of the segment then queue up a play secondary command
	else
	{
		// create a new command queue play segment item
		CCommandItemPlayMotif* pComItemSeg;
		LT_MEM_TRACK_ALLOC(pComItemSeg = new CCommandItemPlayMotif,LT_MEM_TYPE_MUSIC);

		// make sure allocation worked
		if (pComItemSeg != LTNULL)
		{
			// set the directmusic segment inside the new item
			pComItemSeg->SetSegment(pMainSegment);

			CWinSync_CSAuto cProtection(m_CommandQueueCriticalSection);

			// add the new item to the command queue so it will be executed next when a segment ends
			m_lstCommands2.Insert(pComItemSeg);
		}
	}

	return LT_OK;
}


///////////////////////////////////////////////////////////////////////////////////////////
// Play a motif which also masses in a style
///////////////////////////////////////////////////////////////////////////////////////////
LTRESULT CLTDirectMusicMgr::PlayMotif(const char* sStyleName, const char* sMotifName, const LTDMEnactTypes nStart)
{
	return PlayMotif(sMotifName, nStart);
}


///////////////////////////////////////////////////////////////////////////////////////////
// Stop all motifs with the specified name (if LTNULL it stops them all)
///////////////////////////////////////////////////////////////////////////////////////////
LTRESULT CLTDirectMusicMgr::StopMotif(const char* sMotifName, const LTDMEnactTypes nStart)
{
	CSegmentState* pStopSegmentState;
	CSegmentState* pNextSegmentState;
	LTDMEnactTypes nEnactTime = nStart;

	LTDMConOutMsg(4, "CLTDirectMusicMgr::StopMotif sMotifName=%s nStart=%i\n", sMotifName, (int)nStart);

	// if the mgr is not initialized we fail
	if (m_bInitialized == false) { LTDMConOutMgrNotInitialized("CLTDirectMusicMgr::StopMotif"); return LT_ERROR; }

	// we will fail if level has not been initialized
	if (m_bLevelInitialized == false) { LTDMConOutLevelNotInitialized("CLTDirectMusicMgr::StopMotif"); return LT_ERROR; }

	CWinSync_CSAuto cProtection(m_CommandQueueCriticalSection);

	// if name is LTNULL then stop them all
	if (sMotifName == LTNULL)
	{
		// get the first secondary segment
		pStopSegmentState = m_lstMotifsPlaying.GetFirst();

		while (pStopSegmentState != LTNULL)
		{
			// get the next segment to process after this one
			pNextSegmentState = pStopSegmentState->Next();

			// make sure the directmusic segment pointer is OK if not leave
			if (pStopSegmentState->GetDMSegmentState() != LTNULL)
			{
				// if default enact time was passed in then get default enact time from the original segment
				if (nStart == LTDMEnactDefault) nEnactTime = pStopSegmentState->GetSegment()->GetDefaultEnact();

				// clean up this segment (unless it is supposed to play to the end)
				if (nEnactTime != LTDMEnactNextSegment) m_lstMotifsPlaying.CleanupSegmentState(this, pStopSegmentState, nEnactTime);
		
				// update stop segment to next segment
				pStopSegmentState = pNextSegmentState;
			}
		}
	}

	// find the one we want to stop
	else
	{
		// loop until all of the segments with this name have been found
		for(;;)
		{

			// find the secondary segment
			pStopSegmentState = m_lstMotifsPlaying.Find(sMotifName);

			// make sure we found something
			if (pStopSegmentState != LTNULL)
			{
				// if default enact time was passed in then get default enact time from the original segment
				if (nStart == LTDMEnactDefault) nEnactTime = pStopSegmentState->GetSegment()->GetDefaultEnact();

				// make sure the directmusic segment pointer is OK if not leave
				if (pStopSegmentState->GetSegment()->GetDMSegment() != LTNULL)
				{
					// clean up this segment (unless it is supposed to play to the end)
					if (nEnactTime != LTDMEnactNextSegment) m_lstMotifsPlaying.CleanupSegmentState(this, pStopSegmentState, nEnactTime);
				}
			}

			// time to exit loop
			else break;
		}
	}

	return LT_OK;
}


///////////////////////////////////////////////////////////////////////////////////////////
// Stop all motifs with the specified name (if LTNULL it stops them all)
///////////////////////////////////////////////////////////////////////////////////////////
LTRESULT CLTDirectMusicMgr::StopMotif(const char* sStyleName, const char* sMotifName, const LTDMEnactTypes nStart)
{
	return StopMotif(sMotifName, nStart); 
}


///////////////////////////////////////////////////////////////////////////////////////////
// Initialize DirectMusic
///////////////////////////////////////////////////////////////////////////////////////////
bool CLTDirectMusicMgr::InitDirectMusic()
{

//#if (defined(NOLITHTECH) || defined(DONTUSELTDIRECTSOUND))
	// create the directmusic performance
	if (FAILED(CoCreateInstance(CLSID_DirectMusicPerformance, LTNULL,	CLSCTX_INPROC, IID_IDirectMusicPerformance8, (void**)&m_pPerformance)))
    {
		// we failed set performance to LTNULL
		m_pPerformance = LTNULL;

		// we have failed exit function
		return false;
    }
/*
#else
	// performance was created when sound was started, get a pointer instead
	CSoundMgr* pSndMgr = GetClientILTSoundMgrImpl();
	if (pSndMgr != LTNULL)
	{
		if (pSndMgr->IsValid())
		{
			m_pPerformance = pSndMgr->GetDirectMusicPerformance();
			m_pDirectMusic = pSndMgr->GetDirectMusic();
		}
		if ( !m_pPerformance )
			return false;
	}
	else
		return false;
#endif
*/

	// create the directmusic loader
	LT_MEM_TRACK_ALLOC(m_pLoader = new CLTDMLoader,LT_MEM_TYPE_MUSIC);
	if (m_pLoader == LTNULL)
    {
		// we failed set loader to LTNULL
        m_pLoader = LTNULL;

//#if (defined(NOLITHTECH) || defined(DONTUSELTDIRECTSOUND))
		// close down and release the performance
		m_pPerformance->CloseDown();
		SAFE_RELEASE(m_pPerformance);
//#endif

		// we have failed exit function
		return false;
    }
	m_pLoader->Init();
        
	// enable caching for all directmusic data objects
	// this is not necessary, we load everything in manually ourselves!!!
	//m_pLoader->EnableCache(GUID_DirectMusicAllTypes, true);

    // Initialize the synthesizer
	if ( !InitPerformance() )
// 	if (FAILED(m_pPerformance->Init(&pDirectMusic, pDirectSound, LTNULL)))
	{
		// Release the loader object.
	    SAFE_RELEASE(m_pLoader);

//#if (defined(NOLITHTECH) || defined(DONTUSELTDIRECTSOUND))
		// close down and release the performance
		m_pPerformance->CloseDown();
		SAFE_RELEASE(m_pPerformance);
//#endif

		// we have failed exit function
		return false;
	}
 
	return true;
}


// Initialize the synthesizer
bool CLTDirectMusicMgr::InitPerformance()
{
	HRESULT hr;

	// setup synthesizer parameters
	memset( (void*) &m_audioParams, 0, sizeof( m_audioParams ) );
	m_audioParams.dwSize = sizeof( DMUS_AUDIOPARAMS );
	m_audioParams.fInitNow = TRUE;
	m_audioParams.dwFeatures = DMUS_AUDIOF_ALL;
	m_audioParams.dwVoices = m_nNumVoices;
	m_audioParams.dwSampleRate = m_nSynthSampleRate;

#if (!defined(NOLITHTECH) && !defined(DONTUSELTDIRECTSOUND))

	LPDIRECTSOUND pDirectSound = LTNULL;
	// get a pointer to the direct sound object we are using, if appropriate
	CSoundMgr* pSndMgr = GetClientILTSoundMgrImpl();
	if (pSndMgr != LTNULL)
	{
		if (pSndMgr->IsValid())
		{
			pDirectSound = pSndMgr->GetDirectSound();

			// set a callback to our callback function in case the directmusic pointer changes
//			pSndMgr->SetInitCallBack(DirectSoundCallBackFunc);
		}
	}

	// create a default audiopath
	hr = m_pPerformance->InitAudio(&m_pDirectMusic, &pDirectSound, LTNULL, LTNULL, LTNULL, LTNULL, &m_audioParams );
	hr = m_pPerformance->CreateStandardAudioPath( DMUS_APATH_DYNAMIC_STEREO, m_nNumPChannels, TRUE, &m_pAudiopath );
	if ( FAILED(hr) )
		goto Done;
	hr = m_pPerformance->SetDefaultAudioPath( m_pAudiopath );
	if ( FAILED(hr) )
		goto Done;
#else
	hr = m_pPerformance->InitAudio(&m_pDirectMusic, LTNULL, LTNULL, LTNULL, LTNULL, LTNULL, &m_audioParams );
	if ( FAILED(hr) )
		goto Done;
	hr = m_pPerformance->CreateStandardAudioPath( DMUS_APATH_DYNAMIC_STEREO, m_nNumPChannels, TRUE, &m_pAudiopath );
	if ( FAILED(hr) )
		goto Done;
	hr = m_pPerformance->SetDefaultAudioPath( m_pAudiopath );
	if ( FAILED(hr) )
		goto Done;
#endif

	// get a pointer to the default audiopath buffer
	hr = m_pAudiopath->GetObjectInPath( DMUS_PCHANNEL_ALL,
                DMUS_PATH_BUFFER, 0, GUID_NULL, 0, IID_IDirectSoundBuffer8, 
                (LPVOID*) &m_pAudiopathBuffer );

	// set up the reverb fx descriptor
    m_ReverbDesc.dwSize = sizeof(DSEFFECTDESC);
	m_ReverbDesc.dwFlags = 0;
    m_ReverbDesc.guidDSFXClass = GUID_DSFX_WAVES_REVERB;

Done:
	return SUCCEEDED( hr );
}

///////////////////////////////////////////////////////////////////////////////////////////
// Terminate DirectMusic
///////////////////////////////////////////////////////////////////////////////////////////
void CLTDirectMusicMgr::TermDirectMusic()
{
	// stop the performance from playing
	if (m_pPerformance != LTNULL) 
	{
		m_pPerformance->Stop( LTNULL, LTNULL, 0, 0 );
	}

	// make sure direct sound object is removed and clear directmusic pointer
	SAFE_RELEASE( m_pAudiopath );
	SAFE_RELEASE( m_pAudiopathBuffer );
	SAFE_RELEASE( m_pDirectMusic );

	// close down and release the performance
	if (m_pPerformance != LTNULL)
	{
		m_pPerformance->CloseDown();
		SAFE_RELEASE( m_pPerformance );
	}
 
    // Release the loader object.
	if (m_pLoader != LTNULL)
	{
		delete m_pLoader;
		m_pLoader = LTNULL;
	}
}


///////////////////////////////////////////////////////////////////////////////////////////
// Set the DirectMusic working directory
///////////////////////////////////////////////////////////////////////////////////////////
void CLTDirectMusicMgr::SetWorkingDirectory(const char* sWorkingDirectory, LTDMFileTypes nFileType)
{
	// store the any type locally
	if (nFileType == LTDMFileTypeAny)
	{
		// remove the old value
		if (m_sWorkingDirectoryAny != LTNULL) 
		{
			delete [] m_sWorkingDirectoryAny;
			m_sWorkingDirectoryAny = LTNULL;
		}

		// store the new value if it is not null
		if (sWorkingDirectory != LTNULL)
		{
			LT_MEM_TRACK_ALLOC(m_sWorkingDirectoryAny = new char[strlen(sWorkingDirectory)+1],LT_MEM_TYPE_MUSIC);
			strcpy(m_sWorkingDirectoryAny, sWorkingDirectory);
		}
	}

	// store the control file type locally
	if (nFileType == LTDMFileTypeControlFile)
	{
		// remove the old value
		if (m_sWorkingDirectoryControlFile != LTNULL) 
		{
			delete m_sWorkingDirectoryControlFile;
			m_sWorkingDirectoryControlFile = LTNULL;
		}

		// store the new value if it is not null
		if (sWorkingDirectory != LTNULL)
		{
			LT_MEM_TRACK_ALLOC(m_sWorkingDirectoryControlFile = new char[strlen(sWorkingDirectory)+1],LT_MEM_TYPE_MUSIC);
			strcpy(m_sWorkingDirectoryControlFile, sWorkingDirectory);
		}
	}

	// convert file type to GUID type
	CLSID m_clsID = GUID_NULL;
	switch (nFileType)
	{
		case LTDMFileTypeAny : { m_clsID = GUID_DirectMusicAllTypes; break; }
		case LTDMFileTypeDLS : { m_clsID = CLSID_DirectMusicCollection; break; }
		case LTDMFileTypeStyle : { m_clsID = CLSID_DirectMusicStyle; break; }
		case LTDMFileTypeSegment : { m_clsID = CLSID_DirectMusicSegment; break; }
		case LTDMFileTypeChordMap : { m_clsID = CLSID_DirectMusicChordMap; break; }
	}

	// set the search directory for directmusic if GUID is valid
    if (m_clsID != GUID_NULL) 
	{
		m_pLoader->SetSearchDirectory( m_clsID, sWorkingDirectory, false );
	}
}


#ifdef NOLITHTECH
///////////////////////////////////////////////////////////////////////////////////////////
// rez file name to load from (this should be called before InitLevel and can be called multiple times for multiple rez files)
///////////////////////////////////////////////////////////////////////////////////////////
void CLTDirectMusicMgr::SetRezFile(const char* sRezFile)
{
	// remove the old value
	if (m_sRezFileName != LTNULL) 
	{
		delete [] m_sRezFileName;
		m_sRezFileName = LTNULL;
	}

	// store the new value if it is not null
	if (sRezFile != LTNULL)
	{
		LT_MEM_TRACK_ALLOC(m_sRezFileName = new char[strlen(sRezFile)+1],LT_MEM_TYPE_MUSIC);
		strcpy(m_sRezFileName, sRezFile);
	}

	m_pLoader->SetRezFile(sRezFile);
}
#endif


///////////////////////////////////////////////////////////////////////////////////////////
// Load Segment
///////////////////////////////////////////////////////////////////////////////////////////
bool CLTDirectMusicMgr::LoadSegment(const char* sSegmentName )
{
    HRESULT         hr;
    DMUS_OBJECTDESC ObjDesc; 
	IDirectMusicSegment8* pDMSegment;
	CSegment*		pSegment;

	// first check and see if this segment has already been loaded
	if (m_lstSegments.Find(sSegmentName) != LTNULL)
	{
		// it has already been loaded to we don't need to load it again just exit happily
		return true;
	}

	// make a new segment item
	LT_MEM_TRACK_ALLOC(pSegment = new CSegment,LT_MEM_TYPE_MUSIC);
	if (pSegment == LTNULL) return false;

	// set up object descriptor
    ObjDesc.guidClass = CLSID_DirectMusicSegment;
    ObjDesc.dwSize = sizeof(DMUS_OBJECTDESC);
	MULTI_TO_WIDE( ObjDesc.wszFileName, sSegmentName);
    ObjDesc.dwValidData = DMUS_OBJ_CLASS | DMUS_OBJ_FILENAME;

	// load the object
    hr = m_pLoader->GetObject( &ObjDesc, IID_IDirectMusicSegment8, (void**)&pDMSegment );

	// check if the load succeeded
    if (SUCCEEDED(hr)) 
	{

		// load all dls banks this segment refrences
//		pDMSegment->SetParam(GUID_Download, 0xFFFFFFFF, 0, 0, LTNULL);
		hr = pDMSegment->Download( m_pPerformance );

		// set the new segment up 
		pSegment->SetDMSegment(pDMSegment);
		pSegment->SetSegmentName(sSegmentName);
		m_lstSegments.Insert(pSegment);

		// output debut info
		LTDMConOutMsg(3, "LTDirectMusic loaded segment %s\n", sSegmentName);

	    return true;
	}
	// if the load failed
	else
	{
		LTDMConOutWarning("WARNING! LTDirectMusic failed to load segment %s\n", sSegmentName);

		delete pSegment;

		return false;
	}
}



///////////////////////////////////////////////////////////////////////////////////////////
// Load DLS Bank
///////////////////////////////////////////////////////////////////////////////////////////
bool CLTDirectMusicMgr::LoadDLSBank(const char* sFileName)
{
	IDirectMusicCollection8* pDMCollection = LTNULL;
	CDLSBank* pDLSBank;

	// convert the specified file name to wide characters
    WCHAR wszFileName[_MAX_PATH];
    MULTI_TO_WIDE( wszFileName, sFileName );

	// check if the filename is empty then don't load anything
	if ( wcscmp(wszFileName, L"") == 0 )
		return true;

	// create DLSBank
	LT_MEM_TRACK_ALLOC(pDLSBank = new CDLSBank(),LT_MEM_TYPE_MUSIC);
	if (pDLSBank == LTNULL) return false;

	// add to DLSBank list
	pDLSBank->SetDLSBank(pDMCollection);
	m_lstDLSBanks.Insert(pDLSBank);

	return true;
}


///////////////////////////////////////////////////////////////////////////////////////////
// Load Style and associated bands
///////////////////////////////////////////////////////////////////////////////////////////
bool CLTDirectMusicMgr::LoadStyleAndBands(char* sStyleFileName, CControlFileMgr& controlFile)
{
	// convert the specified file name to wide characters
    WCHAR wszFileName[_MAX_PATH];
    MULTI_TO_WIDE( wszFileName, sStyleFileName );
	IDirectMusicStyle8* pDMStyle;
	CStyle* pStyle;

	// check if the filename is empty then don't load anything
	if ( wcscmp(wszFileName, L"") == 0 )
		return true;

	// allocate the style class
	LT_MEM_TRACK_ALLOC(pStyle = new CStyle,LT_MEM_TYPE_MUSIC);
	if (pStyle == LTNULL) return false;

	// set up the object description
	DMUS_OBJECTDESC ObjectDescript;
	ObjectDescript.guidClass = CLSID_DirectMusicStyle;
	wcscpy(ObjectDescript.wszFileName, wszFileName);
	ObjectDescript.dwValidData = DMUS_OBJ_CLASS | DMUS_OBJ_FILENAME ;
	ObjectDescript.dwSize = sizeof(DMUS_OBJECTDESC);

	// load the style object
	if (FAILED(m_pLoader->GetObject(&ObjectDescript, IID_IDirectMusicStyle8, (void**)&pDMStyle)))
	{
		// load failed
		LTDMConOutWarning("WARNING! LTDirectMusic failed to style %s\n", sStyleFileName);
		delete pStyle;
		return false;
	}
	else
	{
		// output debug message that we loaded the style
		LTDMConOutMsg(3, "LTDirectMusic loaded style %s\n", sStyleFileName);

		// load all the band commands for this level and load any of them that are
		// specified for this style file
		CControlFileKey* pKey = controlFile.GetKey(LTNULL, "BAND");
		CControlFileWord* pWord;
		CControlFileWord* pWord2;
		while (pKey != LTNULL)
		{
			// get the name of the style file this band is for
			pWord = pKey->GetFirstWord();
			if (pWord != LTNULL)
			{
				// check if this matches the style we are loading
				if (stricmp(pWord->GetVal(), sStyleFileName) == 0)
				{
					// get name of the band to load
					pWord2 = pWord->Next();
					if (pWord2 != LTNULL)
					{
						// load the band
						LoadBand(pDMStyle, pWord2->GetVal());
					}
				}
			}
			pKey = pKey->NextWithSameName();
		}

		// add style to list of styles
		pStyle->SetDMStyle(pDMStyle);
		pStyle->SetStyleName(sStyleFileName);
		m_lstStyles.Insert(pStyle);

		return true;
	}
}


///////////////////////////////////////////////////////////////////////////////////////////
// Load Band
///////////////////////////////////////////////////////////////////////////////////////////
bool CLTDirectMusicMgr::LoadBand(IDirectMusicStyle8* pStyle, const char* sBandName)
{
	IDirectMusicBand8* pDMBand;
	CBand* pBand;

	// check if style is null
	if (pStyle == LTNULL) return false;

	// create new band object
	LT_MEM_TRACK_ALLOC(pBand = new CBand,LT_MEM_TYPE_MUSIC);
	if (pBand == LTNULL) return false;
		
	// convert band name to wide characters
	WCHAR wszBandName[_MAX_PATH];
    MULTI_TO_WIDE( wszBandName, sBandName );

	// check if name is empty
	if ( wcscmp(wszBandName, L"") != 0 )
	{
		// load band
		pStyle->GetBand(wszBandName, &pDMBand);

		// if we failed then exit
		if (pDMBand == LTNULL)
		{
			delete pBand;
			return false;
		}

		// download all of the instruments in the band
		HRESULT hr = pDMBand->Download(m_pPerformance);

		// check if download worked
		if (FAILED(hr))
		{
			LTDMConOutWarning("WARNING! LTDirectMusic failed to load band %s\n", sBandName);
		}

		// add band to global list of bands
		pBand->SetBand(pDMBand);
		m_lstBands.Insert(pBand);

		// create a band segment
		IDirectMusicSegment* pNewSeg;
		IDirectMusicSegment8* pNewSeg8;
		hr = pDMBand->CreateSegment(&pNewSeg);
	
		// Get the IDirectMusicSegment8 interface
		if ( SUCCEEDED( hr )  )
		{
			hr = pNewSeg->QueryInterface(IID_IDirectMusicSegment8, (void**)&pNewSeg8);
		}
		
		// check if band segment was created OK
		if (FAILED(hr))
		{
			LTDMConOutWarning("WARNING! LTDirectMusic failed to create band segment for band %s\n", sBandName);
		}
		else
		{
			// play the band segment
			m_pPerformance->PlaySegment(pNewSeg8, DMUS_SEGF_SECONDARY, 0, LTNULL);

			// set the band segment in the band structure
			pBand->SetDMSegment(pNewSeg8);
		}

		// output debug message that we loaded the style
		LTDMConOutMsg(3, "LTDirectMusic loaded band %s\n", sBandName);
	}

	// we succeeded 
	return true;
}


///////////////////////////////////////////////////////////////////////////////////////////
// Read in and set up DLS Banks from control file
///////////////////////////////////////////////////////////////////////////////////////////
bool CLTDirectMusicMgr::ReadDLSBanks(CControlFileMgr& controlFile)
{
	CControlFileKey* pKey = controlFile.GetKey(LTNULL, "DLSBANK");
	CControlFileWord* pWord;

	// loop through all DLSBANK keys
	while (pKey != LTNULL)
	{
		// get the first value
		pWord = pKey->GetFirstWord();
		if (pWord != LTNULL)
		{
			// load the DLS Bank
			LoadDLSBank(pWord->GetVal());
		}

		// get the next key
		pKey = pKey->NextWithSameName();
	}

	return true;
}


///////////////////////////////////////////////////////////////////////////////////////////
// Read in and load styles and bands from control file
///////////////////////////////////////////////////////////////////////////////////////////
bool CLTDirectMusicMgr::ReadStylesAndBands(CControlFileMgr& controlFile)
{
	CControlFileKey* pKey = controlFile.GetKey(LTNULL, "STYLE");
	CControlFileWord* pWord;

	// loop through all STYLE keys
	while (pKey != LTNULL)
	{
		// get the first value
		pWord = pKey->GetFirstWord();
		if (pWord != LTNULL)
		{
			// load the style and any associated bands
			LoadStyleAndBands(pWord->GetVal(), controlFile);
		}

		// get the next key
		pKey = pKey->NextWithSameName();
	}

	return true;
}


///////////////////////////////////////////////////////////////////////////////////////////
// Read in intensity descriptions from control file
///////////////////////////////////////////////////////////////////////////////////////////
bool CLTDirectMusicMgr::ReadIntensities(CControlFileMgr& controlFile)
{
	CControlFileKey* pKey = controlFile.GetKey(LTNULL, "INTENSITY");
	CControlFileWord* pWord;
	int nIntensity;
	int nLoop;
	int nNextIntensity;

	// loop through all INTENSITY keys
	while (pKey != LTNULL)
	{
		// get the first word which is the intensity number
		pWord = pKey->GetFirstWord();

		// if this is LTNULL then we have an invalid Intensity definition go on to next one
		if (pWord == LTNULL) 
		{
			pKey = pKey->NextWithSameName();
			continue;
		}

		// get the value for the intensity
		nIntensity = 0;
		pWord->GetVal(nIntensity);

		// make sure the intensity number is valid
		if ((nIntensity <= 0) || (nIntensity > m_nNumIntensities))
		{
			// this is an invalid intensity go on to the next one
			// TO DO!!! Add OutputDebug information here and other places in this function!
			pKey = pKey->NextWithSameName();
			continue;
		}

		// get the next word which is the loop value
		pWord = pWord->Next();

		// if this is LTNULL then we have an invalid Intensity definition go on to next one
		if (pWord == LTNULL) 
		{
			pKey = pKey->NextWithSameName();
			continue;
		}

		// get the loop value
		nLoop = -1;
		pWord->GetVal(nLoop);

		// get the next word with is the next intensity to switch to when this one finishes
		pWord = pWord->Next();

		// if this is LTNULL then we have an invalid Intensity definition go on to next one
		if (pWord == LTNULL) 
		{
			pKey = pKey->NextWithSameName();
			continue;
		}

		// get the next intensity to switch to value
		nNextIntensity = 0;
		pWord->GetVal(nNextIntensity);

		// get the next word which is the first segment name for this intensity
		pWord = pWord->Next();

		// if this is LTNULL then we have an invalid Intensity definition go on to next one
		if (pWord == LTNULL) 
		{
			pKey = pKey->NextWithSameName();
			continue;
		}

		// loop through all segment names and add them to the intensity and load them in
		while (pWord != LTNULL)
		{
			// load in the segment
			LoadSegment(pWord->GetVal());

			// find the loaded segment in the master segment list
			CSegment* pMasterSegment = m_lstSegments.Find(pWord->GetVal());
			if (pMasterSegment == LTNULL) break;
			if (pMasterSegment->GetDMSegment() == LTNULL) break;

			// add the segment to this intensity
			CSegment* pNewSeg;
			LT_MEM_TRACK_ALLOC(pNewSeg = new CSegment,LT_MEM_TYPE_MUSIC);
			if (pNewSeg == LTNULL) break;
			pNewSeg->SetDMSegment(pMasterSegment->GetDMSegment());
			m_aryIntensities[nIntensity].GetSegmentList().InsertLast(pNewSeg);

			// get the next word which is the next segment
			pWord = pWord->Next();
		}

		// set the other values in our intensity
		m_aryIntensities[nIntensity].SetNumLoops(nLoop);
		m_aryIntensities[nIntensity].SetIntensityToSetAtFinish(nNextIntensity);

		// get the next key
		pKey = pKey->NextWithSameName();
	}

	return true;
}


///////////////////////////////////////////////////////////////////////////////////////////
// Read in secondary segments from control file
///////////////////////////////////////////////////////////////////////////////////////////
bool CLTDirectMusicMgr::ReadSecondarySegments(CControlFileMgr& controlFile)
{
	CControlFileKey* pKey = controlFile.GetKey(LTNULL, "SECONDARYSEGMENT");
	CControlFileWord* pWord;

	// loop through all SECONDARYSEGMENT keys
	while (pKey != LTNULL)
	{
		// get the first value
		pWord = pKey->GetFirstWord();
		if (pWord != LTNULL)
		{
			// load the segment
			LoadSegment(pWord->GetVal());

			// find the loaded segment in the master segment list
			CSegment* pMasterSegment = m_lstSegments.Find(pWord->GetVal());
			if (pMasterSegment != LTNULL) 
			{
				// get the next word which is the first
				pWord = pWord->Next();
				if (pWord != LTNULL)
				{
					// set the enact time in the segment
					pMasterSegment->SetDefaultEnact(StringToEnactType(pWord->GetVal()));
				}
			}
		}

		// get the next key
		pKey = pKey->NextWithSameName();
	}

	return true;
}

	
///////////////////////////////////////////////////////////////////////////////////////////
// Read in secondary segments from control file
///////////////////////////////////////////////////////////////////////////////////////////
bool CLTDirectMusicMgr::ReadMotifs(CControlFileMgr& controlFile)
{
	CControlFileKey* pKey = controlFile.GetKey(LTNULL, "MOTIF");
	CControlFileWord* pWord;

	// loop through all MOTIF keys
	while (pKey != LTNULL)
	{
		// get the first word
		pWord = pKey->GetFirstWord();
		if (pWord != LTNULL)
		{
			// find the name of the motif
			char* sStyleName = pWord->GetVal();

			// find the style
			CStyle* pStyle = m_lstStyles.Find(sStyleName);

			// get the second word
			pWord = pWord->Next();
			if (pWord != LTNULL)
			{
				// find the name of the motif
				char* sMotifName = pWord->GetVal();

				// make sure we have a valid style and motif name
				if ((pStyle != LTNULL) && (sMotifName != LTNULL))
				{
					// load in the motif returning a motif segment
					IDirectMusicSegment* pDMSeg;
					IDirectMusicSegment8* pDMSeg8;
	
					WCHAR wszMotif[_MAX_PATH];
				  	MULTI_TO_WIDE( wszMotif, sMotifName );
					HRESULT hr = pStyle->GetDMStyle()->GetMotif( wszMotif, &pDMSeg );

					if ( hr == S_OK )
					{
						// Get the IDirectMusicSegment8 interface
						HRESULT hr = pDMSeg->QueryInterface(IID_IDirectMusicSegment8,
							(void**)&pDMSeg8);
					}

					// make sure we got a segment
					if ((hr == S_OK) && (pDMSeg8 != LTNULL))
					{
						// create a new motif segment
						CSegment* pSegment;
						LT_MEM_TRACK_ALLOC(pSegment = new CSegment,LT_MEM_TYPE_MUSIC);
						if (pSegment != LTNULL)
						{
							// set the members of the new segment
							pSegment->SetDMSegment(pDMSeg8);
							pSegment->SetSegmentName(sMotifName);

							// see if there is an enact time present and set it
							pWord = pWord->Next();
							if (pWord != LTNULL)
							{
								// set the enact time in the segment
								pSegment->SetDefaultEnact(StringToEnactType(pWord->GetVal()));
							}

							// add new segment to the master motif segment list
							m_lstMotifs.Insert(pSegment);
						}
					}
				}
			}
		}

		// get the next key
		pKey = pKey->NextWithSameName();
	}

	return true;
}

	
///////////////////////////////////////////////////////////////////////////////////////////
// Read in transition matrix from control file
///////////////////////////////////////////////////////////////////////////////////////////
bool CLTDirectMusicMgr::ReadTransitions(CControlFileMgr& controlFile)
{
	CControlFileKey* pKey = controlFile.GetKey(LTNULL, "TRANSITION");
	CControlFileWord* pWord;
	int nTransitionFrom;
	int nTransitionTo;
	LTDMEnactTypes m_nEnactTime;
	bool m_bManual;

	// loop through all TRANSITION keys
	while (pKey != LTNULL)
	{
		// get the first word which is the from intensity transition value
		pWord = pKey->GetFirstWord();

		// if this is LTNULL then we have an invalid tranisition definition go on to next one
		if (pWord == LTNULL) 
		{
			pKey = pKey->NextWithSameName();
			continue;
		}

		// get the value for the intensity
		nTransitionFrom = 0;
		pWord->GetVal(nTransitionFrom);

		// make sure the intensity number is valid
		if ((nTransitionFrom <= 0) || (nTransitionFrom > m_nNumIntensities))
		{
			// this is an invalid transition go on to the next one
			// TO DO!!! Add OutputDebug information here and other places in this function!
			pKey = pKey->NextWithSameName();
			continue;
		}

		// get the next word which is the intensity transition to value
		pWord = pWord->Next();

		// if this is LTNULL then we have an invalid tranisition definition go on to next one
		if (pWord == LTNULL) 
		{
			pKey = pKey->NextWithSameName();
			continue;
		}

		// get the value for the intensity
		nTransitionTo = 0;
		pWord->GetVal(nTransitionTo);

		// make sure the intensity number is valid
		if ((nTransitionTo <= 0) || (nTransitionTo > m_nNumIntensities))
		{
			// this is an invalid transition go on to the next one
			// TO DO!!! Add OutputDebug information here and other places in this function!
			pKey = pKey->NextWithSameName();
			continue;
		}

		// get the next word which is the when to enact the transition value
		pWord = pWord->Next();

		// if this is LTNULL then we have an invalid tranisition definition go on to next one
		if (pWord == LTNULL) 
		{
			pKey = pKey->NextWithSameName();
			continue;
		}

		// figure out which enact value user specified
		m_nEnactTime = StringToEnactType(pWord->GetVal());

		// make sure we got a valid enact time
		if (m_nEnactTime == LTDMEnactInvalid)
		{
			pKey = pKey->NextWithSameName();
			continue;
		}

		// get the next word which defines if this is an automatic or manual transition
		pWord = pWord->Next();

		// if this is LTNULL then we have an invalid tranisition definition go on to next one
		if (pWord == LTNULL) 
		{
			pKey = pKey->NextWithSameName();
			continue;
		}

		// figure out which enact value user specified
		if (stricmp(pWord->GetVal(), "MANUAL") == 0) m_bManual = true;
		else
		{
			if (stricmp(pWord->GetVal(), "AUTOMATIC") == 0) m_bManual = false;
			else
			{
				pKey = pKey->NextWithSameName();
				continue;
			}
		}
		
		// get the next word which is the when to enact the transition value
		pWord = pWord->Next();

		// check if there is a segment name for this transition
		if (pWord != LTNULL)
		{
			// load in the segment
			LoadSegment(pWord->GetVal());

			// find the loaded segment in the master segment list
			CSegment* pMasterSegment = m_lstSegments.Find(pWord->GetVal());
			if (pMasterSegment != LTNULL) 
			{
				GetTransition(nTransitionFrom, nTransitionTo)->SetDMSegment(pMasterSegment->GetDMSegment());
			}
			else
			{
				GetTransition(nTransitionFrom, nTransitionTo)->SetDMSegment(LTNULL);
			}
		}

		// set the other values in our transition
		GetTransition(nTransitionFrom, nTransitionTo)->SetEnactTime(m_nEnactTime);
		GetTransition(nTransitionFrom, nTransitionTo)->SetManual(m_bManual);

		// get the next key
		pKey = pKey->NextWithSameName();
	}

	return true;
}


///////////////////////////////////////////////////////////////////////////////////////////
// Set the name for the segment
///////////////////////////////////////////////////////////////////////////////////////////
bool CLTDirectMusicMgr::CSegment::SetSegmentName(const char* sSegmentName)
{
	// if there was an old long segment name delete it
	if (m_sSegmentNameLong != LTNULL)
	{
		delete [] m_sSegmentNameLong;
		m_sSegmentNameLong = LTNULL;
	}

	// if new name is null 
	if (sSegmentName == LTNULL) 
	{
		// just set the segment name to nothing
		m_sSegmentName[0] = '\0';
	}

	// if new name is valid then proceed
	else
	{
		// find out the length of the string
		int nLen = strlen(sSegmentName);

		// check if we need to use the long storage method
		if (nLen > 63)
		{
			// allocate the new string
			LT_MEM_TRACK_ALLOC(m_sSegmentNameLong = new char[nLen+1],LT_MEM_TYPE_MUSIC);

			// if allocation failed then exit 
			if (m_sSegmentNameLong == LTNULL) return false;

			// copy over the string contents
			strcpy(m_sSegmentNameLong, sSegmentName);

			// set the short name to nothing
			m_sSegmentName[0] = '\0';
		}

		// it is a short name so just copy it over
		else
		{
			LTStrCpy(m_sSegmentName, sSegmentName, sizeof(m_sSegmentName));
		}
	}

	// exit successfully
	return true;
}


///////////////////////////////////////////////////////////////////////////////////////////
// Get the name for the segment
///////////////////////////////////////////////////////////////////////////////////////////
const char* CLTDirectMusicMgr::CSegment::GetSegmentName()
{
	if (m_sSegmentNameLong == LTNULL) return m_sSegmentName;
	else return m_sSegmentNameLong;
};


///////////////////////////////////////////////////////////////////////////////////////////
// Find the segment object with the specified name return null if not found
///////////////////////////////////////////////////////////////////////////////////////////
CLTDirectMusicMgr::CSegment* CLTDirectMusicMgr::CSegmentList::Find(const char* sName)
{
	// if the name passed in is LTNULL just return LTNULL
	if (sName == LTNULL) return LTNULL;

	// get the first item in the segment list
	CSegment* pSegment = GetFirst();

	// loop through all segments
	while (pSegment != LTNULL)
	{
		// make sure the segment name is not null
		if (pSegment->GetSegmentName() != LTNULL)
		{
			// compare and see if this is the one
			if (stricmp(pSegment->GetSegmentName(), sName) == 0)
			{
				// we have found it exit
				return pSegment;
			}
		}

		// get the next segment
		pSegment = pSegment->Next();
	}

	// we did not find it exit with LTNULL
	return LTNULL;
}


///////////////////////////////////////////////////////////////////////////////////////////
// Find the segment object with the specified name return null if not found
///////////////////////////////////////////////////////////////////////////////////////////
CLTDirectMusicMgr::CSegment* CLTDirectMusicMgr::CSegmentList::Find(const IDirectMusicSegment8* pDMSegment)
{
	// get the first item in the segment list
	CSegment* pSegment = GetFirst();

	// loop through all segments
	while (pSegment != LTNULL)
	{
		// compare and see if this is the one
		if (pDMSegment == pSegment->GetDMSegment())
		{
			// we have found it exit
			return pSegment;
		}

		// get the next segment
		pSegment = pSegment->Next();
	}

	// we did not find it exit with LTNULL
	return LTNULL;
}


///////////////////////////////////////////////////////////////////////////////////////////
// Find the segment object with the specified name return null if not found
///////////////////////////////////////////////////////////////////////////////////////////
CLTDirectMusicMgr::CSegmentState* CLTDirectMusicMgr::CSegmentStateList::Find(const char* sName)
{
	// if the name passed in is LTNULL just return LTNULL
	if (sName == LTNULL) return LTNULL;

	// get the first item in the segment list
	CSegmentState* pSegmentState = GetFirst();

	// loop through all segment states
	while (pSegmentState != LTNULL)
	{
		// get the segmenet
		CSegment* pSegment = pSegmentState->GetSegment();

		// make sure segment is not LTNULL
		if (pSegment != LTNULL)
		{
			// make sure the segment name is not null
			if (pSegment->GetSegmentName() != LTNULL)
			{
				// compare and see if this is the one
				if (stricmp(pSegment->GetSegmentName(), sName) == 0)
				{
					// we have found it exit
					return pSegmentState;
				}
			}
		}

		// get the next segment
		pSegmentState = pSegmentState->Next();
	}

	// we did not find it exit with LTNULL
	return LTNULL;
}


///////////////////////////////////////////////////////////////////////////////////////////
// find the segment state object that has the given direct music segment state pointer
///////////////////////////////////////////////////////////////////////////////////////////
CLTDirectMusicMgr::CSegmentState* CLTDirectMusicMgr::CSegmentStateList::Find(const IDirectMusicSegmentState8* pDMSegmentState)
{
	// get the first item in the segment list
	CSegmentState* pSegmentState = GetFirst();

	// loop through all segments
	while (pSegmentState != LTNULL)
	{
		// compare and see if this is the one
		if (pDMSegmentState == pSegmentState->GetDMSegmentState())
		{
			// we have found it exit
			return pSegmentState;
		}

		// get the next segment
		pSegmentState = pSegmentState->Next();
	}

	// we did not find it exit with LTNULL
	return LTNULL;
}

///////////////////////////////////////////////////////////////////////////////////////////
// Set the name for the style
///////////////////////////////////////////////////////////////////////////////////////////
bool CLTDirectMusicMgr::CStyle::SetStyleName(const char* sStyleName)
{
	// if there was an old long segment name delete it
	if (m_sStyleNameLong != LTNULL)
	{
		delete [] m_sStyleNameLong;
		m_sStyleNameLong = LTNULL;
	}

	// if new name is null 
	if (sStyleName == LTNULL) 
	{
		// just set the style name to nothing
		m_sStyleName[0] = '\0';
	}

	// if new name is valid then proceed
	else
	{
		// find out the length of the string
		int nLen = strlen(sStyleName);

		// check if we need to use the long storage method
		if (nLen > 63)
		{
			// allocate the new string
			LT_MEM_TRACK_ALLOC(m_sStyleNameLong = new char[nLen+1],LT_MEM_TYPE_MUSIC);

			// if allocation failed then exit 
			if (m_sStyleNameLong == LTNULL) return false;

			// copy over the string contents
			strcpy(m_sStyleNameLong, sStyleName);

			// set the short name to nothing
			m_sStyleName[0] = '\0';
		}

		// it is a short name so just copy it over
		else
		{
			strcpy(m_sStyleName, sStyleName);
		}
	}

	// exit successfully
	return true;
}


///////////////////////////////////////////////////////////////////////////////////////////
// Get the name for the style
///////////////////////////////////////////////////////////////////////////////////////////
const char* CLTDirectMusicMgr::CStyle::GetStyleName()
{
	if (m_sStyleNameLong == LTNULL) return m_sStyleName;
	else return m_sStyleNameLong;
};


///////////////////////////////////////////////////////////////////////////////////////////
// Find the style object with the specified name return null if not found
///////////////////////////////////////////////////////////////////////////////////////////
CLTDirectMusicMgr::CStyle* CLTDirectMusicMgr::CStyleList::Find(const char* sName)
{
	// if the name passed in is LTNULL just return LTNULL
	if (sName == LTNULL) return LTNULL;

	// get the first item in the segment list
	CStyle* pStyle = GetFirst();

	// loop through all segments
	while (pStyle != LTNULL)
	{
		// make sure the segment name is not null
		if (pStyle->GetStyleName() != LTNULL)
		{
			// compare and see if this is the one
			if (stricmp(pStyle->GetStyleName(), sName) == 0)
			{
				// we have found it exit
				return pStyle;
			}
		}

		// get the next segment
		pStyle = pStyle->Next();
	}

	// we did not find it exit with LTNULL
	return LTNULL;
}

///////////////////////////////////////////////////////////////////////////////////////////
// destructor for intensity class
///////////////////////////////////////////////////////////////////////////////////////////
CLTDirectMusicMgr::CIntensity::~CIntensity()
{
	// free all of the segment classes that were specified
	// we don't have to release the directmusic segment here
	// because that is done in the global segment list in TermLevel
	CSegment* pSegment;
	while ((pSegment = m_lstSegments.GetFirst()) != LTNULL)
	{
		// make sure the segment name is de-allocated
		pSegment->SetSegmentName(LTNULL);

		// remove our Segment object from the Segment list
		m_lstSegments.Delete(pSegment);

		// delete our Segment object
		delete pSegment;
	}
}


///////////////////////////////////////////////////////////////////////////////////////////
// Clear the command queue
///////////////////////////////////////////////////////////////////////////////////////////
void CLTDirectMusicMgr::ClearCommands()
{
	CCommandItem* pCommand;

	// loop through all the commands
	while ((pCommand = m_lstCommands.GetFirst()) != LTNULL)
	{
		// remove our Command object from the Command list
		m_lstCommands.Delete(pCommand);

		// delete our Command object
		delete pCommand;
	}

	// loop through all the commands in 2nd command queue
	while ((pCommand = m_lstCommands2.GetFirst()) != LTNULL)
	{
		// remove our Command object from the Command list
		m_lstCommands2.Delete(pCommand);

		// delete our Command object
		delete pCommand;
	}
}


///////////////////////////////////////////////////////////////////////////////////////////
// Clear the command queue
///////////////////////////////////////////////////////////////////////////////////////////
CLTDirectMusicMgr::CTransition* CLTDirectMusicMgr::GetTransition(int nFrom, int nTo)
{
	int Index = nFrom+1+(nTo*(m_nNumIntensities+1));

	if ((nFrom > m_nNumTransitions) || (nFrom <= 0) || 
		(nTo > m_nNumTransitions) || (nTo <= 0) || 
		(Index >= m_nNumTransitions) || (Index < 0))
	{
		return &m_aryTransitions[0];
	}
	else
	{

		return &m_aryTransitions[Index];
	}
}


///////////////////////////////////////////////////////////////////////////////////////////
// Cleanup non-playing segments in this segment list
///////////////////////////////////////////////////////////////////////////////////////////
void CLTDirectMusicMgr::CSegmentList::CleanupSegments(CLTDirectMusicMgr* pLTDMMgr, bool bOnlyIfNotPlaying)
{
	// get the first item in the secondary segment playing list
	CSegment* pSegment = GetFirst();
	CSegment* pNextSegment;

	// loop through all the seondary segments that are playing
	while (pSegment != LTNULL)
	{
		// get the next segment
		pNextSegment = pSegment->Next();

		// clean up the segment
		CleanupSegment(pLTDMMgr, pSegment, LTDMEnactDefault, bOnlyIfNotPlaying);

		// set segment to next segment
		pSegment = pNextSegment;
	}
};


///////////////////////////////////////////////////////////////////////////////////////////
// cleanup a segment (stops, deletes, and removes from list)
///////////////////////////////////////////////////////////////////////////////////////////
void CLTDirectMusicMgr::CSegmentList::CleanupSegment(CLTDirectMusicMgr* pLTDMMgr, CSegment* pSegment, LTDMEnactTypes nStart, bool bOnlyIfNotPlaying)
{
	// make sure parameters passed in are OK
	if ((pLTDMMgr != LTNULL) && (pSegment != LTNULL))
	{
		// make sure directmusic segment is not LTNULL
		if (pSegment->GetDMSegment() != LTNULL)
		{
			// check if it is playing
			bool bPlaying = pLTDMMgr->GetDMPerformance()->IsPlaying(pSegment->GetDMSegment(), LTNULL) != 0;

			// check if this segment is not playing or if we don't have to check
			if ((!bPlaying) || (bOnlyIfNotPlaying == false))
			{
				// stop if it is playing
//				This check does not work for some reason so just stop all the time.
//				if (bPlaying) 
				{
					pLTDMMgr->GetDMPerformance()->Stop( pSegment->GetDMSegment(), LTNULL, 0, 0 /*pLTDMMgr->EnactTypeToFlags(nStart)*/ );
				}

				// unload all the instruments associated with the segment
				pSegment->GetDMSegment()->Unload( pLTDMMgr->GetPerformance() );

				// release the directmusic Segment
				pSegment->GetDMSegment()->Release();

				// make sure the segment name is de-allocated
				pSegment->SetSegmentName(LTNULL);

				// remove our Segment object from the Segment list
				Delete(pSegment);

				// delete our Segment object
				delete pSegment;
			}
		}
	}
}


///////////////////////////////////////////////////////////////////////////////////////////
// add a new segment state (creates, and add to the list)
///////////////////////////////////////////////////////////////////////////////////////////
void CLTDirectMusicMgr::CSegmentStateList::CreateSegmentState(IDirectMusicSegmentState8* pDMSegmentState, CSegment* pSegment)
{
	CSegmentState* pNewSegmentState;

	// make new direct music segment state is not NULL
	if (pDMSegmentState != LTNULL)
	{
		// create a new secondary segment state item
		LT_MEM_TRACK_ALLOC(pNewSegmentState = new CSegmentState,LT_MEM_TYPE_MUSIC);

		// make sure allocation worked
		if (pNewSegmentState != LTNULL)
		{
			// set the directmusic segment inside the new segment state
			pNewSegmentState->SetDMSegmentState(pDMSegmentState);

			// set the originating segment inside the new segment state
			pNewSegmentState->SetSegment(pSegment);

			// add the new segment to the list of secondary segments
			Insert(pNewSegmentState);
		}
	}
}


///////////////////////////////////////////////////////////////////////////////////////////
// cleanup segment states
///////////////////////////////////////////////////////////////////////////////////////////
void CLTDirectMusicMgr::CSegmentStateList::CleanupSegmentStates(CLTDirectMusicMgr* pLTDMMgr, bool bOnlyIfNotPlaying)
{
	// get the first item in the segment playing list
	CSegmentState* pSegmentState = GetFirst();
	CSegmentState* pNextSegmentState;

	// loop through all the seondary segments that are playing
	while (pSegmentState != LTNULL)
	{
		// get the next segment
		pNextSegmentState = pSegmentState->Next();

		// clean up the segment
		CleanupSegmentState(pLTDMMgr, pSegmentState, LTDMEnactDefault, bOnlyIfNotPlaying);

		// set segment to next segment
		pSegmentState = pNextSegmentState;
	}
};


///////////////////////////////////////////////////////////////////////////////////////////
// cleanup segment state
///////////////////////////////////////////////////////////////////////////////////////////
void CLTDirectMusicMgr::CSegmentStateList::CleanupSegmentState(CLTDirectMusicMgr* pLTDMMgr, CSegmentState* pSegmentState, LTDMEnactTypes nStart, bool bOnlyIfNotPlaying)
{
	// make sure parameters passed in are OK
	if ((pLTDMMgr != LTNULL) && (pSegmentState != LTNULL))
	{
		// make sure directmusic segment is not LTNULL
		if (pSegmentState->GetDMSegmentState() != LTNULL)
		{
			// check if it is playing
			bool bPlaying = pLTDMMgr->GetDMPerformance()->IsPlaying(LTNULL, pSegmentState->GetDMSegmentState()) != 0;

			// check if this segment is not playing or if we don't have to check
			if ((!bPlaying) || (bOnlyIfNotPlaying == false))
			{
				// stop if it is playing
//				This check does not work for some reason so just stop all the time.
//				if (bPlaying) 
				{
					// if the enact type is invalid then do not call stop on this segment state
					if (nStart != LTDMEnactInvalid)
					{
						pLTDMMgr->GetDMPerformance()->Stop( LTNULL, pSegmentState->GetDMSegmentState(), 0, 0 /*pLTDMMgr->EnactTypeToFlags(nStart)*/ );
					}
				}

				// release the directmusic Segment
				if (pSegmentState->GetDMSegmentState() != LTNULL) 
					pSegmentState->GetDMSegmentState()->Release();

				// remove our Segment object from the Segment list
				Delete(pSegmentState);

				// delete our Segment object
				delete pSegmentState;
			}
		}
	}
}




///////////////////////////////////////////////////////////////////////////////////////////
// return the directmusic flags value that corresponds to the specified enact value
///////////////////////////////////////////////////////////////////////////////////////////
uint32 CLTDirectMusicMgr::EnactTypeToFlags(LTDMEnactTypes nEnactVal)
{
	uint32 nFlags = 0;

	switch(nEnactVal)
	{
		case LTDMEnactDefault :
		{
			nFlags = DMUS_SEGF_DEFAULT;
			break;
		}
		case LTDMEnactImmediately :
		{
			nFlags = 0;
			break;
		}
		case LTDMEnactNextBeat :
		{
			nFlags = DMUS_SEGF_BEAT;
			break;
		}
		case LTDMEnactNextMeasure :
		{
			nFlags = DMUS_SEGF_MEASURE;
			break;
		}
		case LTDMEnactNextGrid :
		{
			nFlags = DMUS_SEGF_GRID;
			break;
		}
		case LTDMEnactNextSegment :
		{
			nFlags = DMUS_SEGF_SEGMENTEND;
			break;
		}
		case LTDMEnactNextMarker :
		{
			nFlags = DMUS_SEGF_MARKER;
			break;
		}
		default :
		{
			nFlags = 0;
		}
	}

	return nFlags;
};


///////////////////////////////////////////////////////////////////////////////////////////
// Set reverb parameters
///////////////////////////////////////////////////////////////////////////////////////////
bool CLTDirectMusicMgr::SetReverbParameters(LPDSFXWavesReverb pParams)
{
	// make sure the buffer and the reverb object exist
	if ( (m_pAudiopathBuffer == LTNULL) || (m_pReverb == LTNULL) )
		return false;

    HRESULT hr = m_pReverb->SetAllParameters( pParams );

	return ( SUCCEEDED( hr ) );
};


///////////////////////////////////////////////////////////////////////////////////////////
// Enable reverb
///////////////////////////////////////////////////////////////////////////////////////////
bool CLTDirectMusicMgr::EnableReverb()
{
	DWORD dwResult;

	if ( (m_pAudiopath == LTNULL) || (m_pAudiopathBuffer == LTNULL) )
		return false;

	// need to deactivate audiopath to set fx
    HRESULT hr = m_pAudiopath->Activate( false );

	if ( FAILED( hr ) )
		goto done;

    hr = m_pAudiopathBuffer->SetFX( 1, &m_ReverbDesc, &dwResult );

	if ( FAILED( hr ) )
		goto done;

    hr = m_pAudiopath->Activate( true );
	
	if ( FAILED( hr ) )
		goto done;

	// get a pointer to the reverb object for the audiopath
	hr = m_pAudiopathBuffer->GetObjectInPath( GUID_DSFX_WAVES_REVERB, 0, IID_IDirectSoundFXWavesReverb8,
				(LPVOID*) &m_pReverb );

done:
	return ( SUCCEEDED( hr ) );
};


///////////////////////////////////////////////////////////////////////////////////////////
// Disable reverb
///////////////////////////////////////////////////////////////////////////////////////////
bool CLTDirectMusicMgr::DisableReverb()
{
	if ( (m_pAudiopath == LTNULL ) || (m_pAudiopathBuffer == LTNULL) )
		return false;

	SAFE_RELEASE(m_pReverb);
	
	// this disables all effects, if we ever any other FX, this
	// will need to change
	
	// need to deactivate audiopath to set fx
    HRESULT hr = m_pAudiopath->Activate( false );

	if ( FAILED( hr ) )
		goto done;

    hr = m_pAudiopathBuffer->SetFX( 0, NULL, NULL );

	if ( FAILED( hr ) )
		goto done;

	hr = m_pAudiopath->Activate( true );

done:
	return ( SUCCEEDED( hr ) );
};


///////////////////////////////////////////////////////////////////////////////////////////
// Initialize reverb parameters from control file and set up reverb in DirectMusic
///////////////////////////////////////////////////////////////////////////////////////////
bool CLTDirectMusicMgr::InitReverb(CControlFileMgr& controlFile)
{
	// set reverb defaults
	m_bUseReverb = false;
	m_ReverbParameters.fInGain = 0.0; 
	m_ReverbParameters.fHighFreqRTRatio = -10.0;
	m_ReverbParameters.fReverbMix = 1000.0;
	m_ReverbParameters.fReverbTime = 0.001f;
 
	// check if reverb is on or off in control file
	{
		char sVal[5];
		controlFile.GetKeyVal(LTNULL, "REVERB", sVal, 4);
		if (stricmp(sVal,"ON") == 0) m_bUseReverb = true;
	}

	// read in reverb parameters
	controlFile.GetKeyVal(LTNULL, "REVERBINGAIN", m_ReverbParameters.fInGain); 
	controlFile.GetKeyVal(LTNULL, "REVERBHIGHFREQRTRATIO", m_ReverbParameters.fHighFreqRTRatio);
	controlFile.GetKeyVal(LTNULL, "REVERBMIX", m_ReverbParameters.fReverbMix);
	controlFile.GetKeyVal(LTNULL, "REVERBTIME", m_ReverbParameters.fReverbTime);

	// set up the reverb parameters
	if ( m_bUseReverb )
	{
		EnableReverb();
		SetReverbParameters(&m_ReverbParameters);
	}

	return true;
}


///////////////////////////////////////////////////////////////////////////////////////////
// Terminate reverb if it was enabled.
///////////////////////////////////////////////////////////////////////////////////////////
bool CLTDirectMusicMgr::TermReverb()
{
	// check if reverb was on
	if (m_bUseReverb)
	{
		// Disable it if it was on
		return DisableReverb();
	}
	// we don't need to do anything if it wasn't on
	else return true;
}


///////////////////////////////////////////////////////////////////////////////////////////
// convert an enact type to a string for display (sName must be large enough no checking is done!)
///////////////////////////////////////////////////////////////////////////////////////////
void CLTDirectMusicMgr::EnactTypeToString(LTDMEnactTypes nType, char* sName)
{
	switch (nType)
	{
		case LTDMEnactInvalid :
		{
			strcpy(sName, "Invalid");
			break;
		}
		case LTDMEnactDefault :
		{
			strcpy(sName, "Default");
			break;
		}
		case LTDMEnactImmediately :
		{
			strcpy(sName, "Immediate");
			break;
		}
		case LTDMEnactNextBeat :
		{
			strcpy(sName, "Beat");
			break;
		}
		case LTDMEnactNextMeasure :
		{
			strcpy(sName, "Measure");
			break;
		}
		case LTDMEnactNextGrid :
		{
			strcpy(sName, "Grid");
			break;
		}
		case LTDMEnactNextSegment :
		{
			strcpy(sName, "Segment");
			break;
		}
		case LTDMEnactNextMarker :
		{	
			strcpy(sName, "Marker");
			break;
		}
		default :
		{
			strcpy(sName, "");
			break;
		}
	}
}


///////////////////////////////////////////////////////////////////////////////////////////
// convert a string to an enact type
///////////////////////////////////////////////////////////////////////////////////////////
LTDMEnactTypes CLTDirectMusicMgr::StringToEnactType(const char* sName)
{
	if (sName == LTNULL) return LTDMEnactInvalid;
	if (stricmp(sName, "Invalid") == 0) return LTDMEnactInvalid;
	if (stricmp(sName, "Default") == 0) return LTDMEnactDefault;
	if (stricmp(sName, "Immediatly") == 0) return LTDMEnactImmediately;
	if (stricmp(sName, "Immediately") == 0) return LTDMEnactImmediately;
	if (stricmp(sName, "Immediate") == 0) return LTDMEnactImmediately;
	if (stricmp(sName, "NextBeat") == 0) return LTDMEnactNextBeat;
	if (stricmp(sName, "NextMeasure") == 0) return LTDMEnactNextMeasure;
	if (stricmp(sName, "NextGrid") == 0) return LTDMEnactNextGrid;
	if (stricmp(sName, "NextSegment") == 0) return LTDMEnactNextSegment;
	if (stricmp(sName, "NextMarker") == 0) return LTDMEnactNextMarker;
	if (stricmp(sName, "Beat") == 0) return LTDMEnactNextBeat;
	if (stricmp(sName, "Measure") == 0) return LTDMEnactNextMeasure;
	if (stricmp(sName, "Grid") == 0) return LTDMEnactNextGrid;
	if (stricmp(sName, "Segment") == 0) return LTDMEnactNextSegment;
	if (stricmp(sName, "Marker") == 0) return LTDMEnactNextMarker;
	return LTDMEnactInvalid;										 
}


// return the current intenisty
int CLTDirectMusicMgr::GetCurIntensity() 
{ 
	return m_nCurIntensity; 
};


// return the number of intensities currently in this level (undefined if not in a level)
int CLTDirectMusicMgr::GetNumIntensities() 
{ 
	return m_nNumIntensities; 
};


// return the initial intensity value for this level
int CLTDirectMusicMgr::GetInitialIntensity() 
{ 
	return m_nInitialIntensity; 
};


// return the initial intensity value for this level
int CLTDirectMusicMgr::GetInitialVolume() 
{ 
	return m_nInitialVolume; 
};


// return the volume offset.  This offset is applied to whatever volume is set.
int CLTDirectMusicMgr::GetVolumeOffset() 
{ 
	return m_nVolumeOffset; 
};

// BackEndDialog.cpp : implementation file
//

#include "stdafx.h"
#include "winpacker.h"
#include "BackEndDialog.h"
#include "IPackerImpl.h"
#include "IPackerOutput.h"
#include "PackerPropList.h"
#include "RegistryUtil.h"
#include "PackerRegLoc.h"
#if _MSC_VER >= 1300
#include <fstream>
#else
#include <fstream.h>
#endif
#include <mmsystem.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

//scale to convert the progress to for an integer
#define PROGRESS_SCALE					1000

#define TIMER_EVENT_ID					200
#define USER_COMMAND_ADD_MESSAGE		(WM_USER + 500)
#define USER_COMMAND_ADD_TASK			(WM_USER + 501)
#define USER_COMMAND_ACTIVATE_TASK		(WM_USER + 502)
#define USER_COMMAND_UPDATE_PROGRESS	(WM_USER + 503)
#define USER_COMMAND_ACTIVATE_SUBTASK	(WM_USER + 504)

/////////////////////////////////////////////////////////////////////////////
// Util functions
bool ShouldShowMessage(CTaskMessage* pMsg, EMsgType eFilter)
{
	if(pMsg->GetSeverity() <= eFilter)
		return true;

	return false;
}


/////////////////////////////////////////////////////////////////////////////
// Thread data

struct SThreadData
{
	CString				m_sFilename;
	IPackerImpl*		m_pIPackerImpl;
	IPackerOutput*		m_pIPackerOutput;
	CPackerPropList*	m_pPropList;
};

static SThreadData		g_ThreadData;

static DWORD WINAPI LaunchPackerThreadMain(LPVOID pParam)
{
	ASSERT(g_ThreadData.m_pIPackerImpl);
	ASSERT(g_ThreadData.m_pIPackerOutput);
	ASSERT(g_ThreadData.m_pPropList);

	bool bRV = g_ThreadData.m_pIPackerImpl->Process(	g_ThreadData.m_sFilename,
														g_ThreadData.m_pPropList, 
														g_ThreadData.m_pIPackerOutput);

	return bRV ? 1 : 0;
}


/////////////////////////////////////////////////////////////////////////////
// CBackEndDialog dialog


CBackEndDialog::CBackEndDialog(IPackerImpl* pImpl, CPackerPropList* pPropList, CWnd* pParent)
	: CDialog(CBackEndDialog::IDD, pParent),
	m_pPropList(pPropList),
	m_pIPackerImpl(pImpl),
	m_pHeadTask(NULL),
	m_pCurrTask(NULL),
	m_bPauseWhenDone(TRUE)
{
	ASSERT(m_pIPackerImpl);
	ASSERT(m_pPropList);

	//load in the icon for the save button
	m_hMainIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	m_hSaveIcon = LoadIcon(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDI_SAVEICON));
	m_hOptionsIcon = LoadIcon(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDI_OPTIONSICON));

	//{{AFX_DATA_INIT(CBackEndDialog)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}

CBackEndDialog::~CBackEndDialog()
{
	delete m_pHeadTask;
}


BEGIN_MESSAGE_MAP(CBackEndDialog, CDialog)
	//{{AFX_MSG_MAP(CBackEndDialog)
	ON_WM_TIMER()
	ON_LBN_SELCHANGE(IDC_LIST_TASKS, OnTaskChanged)
	ON_CBN_SELCHANGE(IDC_COMBO_THREAD_PRIORITY, OnThreadPriorityChanged)
	ON_BN_CLICKED(IDC_BUTTON_SAVE_LOG, OnSaveLog)
	ON_CBN_SELCHANGE(IDC_COMBO_MESSAGE_FILTER, OnChangeMessageFilter)
	ON_WM_SIZE()
	ON_MESSAGE(USER_COMMAND_ADD_MESSAGE, OnAddMessage)
	ON_MESSAGE(USER_COMMAND_ADD_TASK, OnAddTask)
	ON_MESSAGE(USER_COMMAND_ACTIVATE_TASK, OnActivateTask)
	ON_MESSAGE(USER_COMMAND_ACTIVATE_SUBTASK, OnActivateSubTask)
	ON_MESSAGE(USER_COMMAND_UPDATE_PROGRESS, OnUpdateProgress)
	ON_BN_CLICKED(IDC_BUTTON_MESSAGE_OPTIONS, OnButtonMessageOptions)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CBackEndDialog message handlers
// This will create a task and add it to the task queue. Tasks
// can be associated with messages as well as a progress indicator.
bool CBackEndDialog::CreateTask(const char* pszTaskName)
{
	//create the new task
	CTask* pNewTask = new CTask(pszTaskName);

	if(pNewTask)
	{
		//send it so the other thread can add it
		PostMessage(USER_COMMAND_ADD_TASK, (WPARAM)pNewTask);
		return true;		
	}

	return false;
}

// This function allows for a task to be activated, meaning that
// all messages sent will be associated with this task, as will
// all progress messages
bool CBackEndDialog::ActivateTask(const char* pszTaskName)
{
	//copy the string (it could be lost on this thread by the time it gets to the other
	//thread)
	char* pszTask = CloneString(pszTaskName);

	PostMessage(USER_COMMAND_ACTIVATE_TASK, (WPARAM)pszTask);

	return true;

}

// This function allows for a sub task to be started. This does not create
// a new task or redirect messages, only changes possible UI for a sub
// task that is being performed as part of a larger task. When you switch
// to a task, that task also becomes the active subtask
bool CBackEndDialog::ActivateSubTask(const char* pszSubTaskName)
{
	//copy the string (it could be lost on this thread by the time it gets to the other
	//thread)
	char* pszSubTask = CloneString(pszSubTaskName);

	PostMessage(USER_COMMAND_ACTIVATE_SUBTASK, (WPARAM)pszSubTask);

	return true;

}

// This will update the progress of the currently active task. The
// value can range from 0 to 100, where 0 is starting, and 100 is
// completed
bool CBackEndDialog::UpdateProgress(float fProgress)
{
	//we want to remove any existing progress update message in the message queue
	//(if we don't it tends to flood the queue and prevents repainting from
	//occurring)
	MSG msg;
	PeekMessage(&msg, GetSafeHwnd(), USER_COMMAND_UPDATE_PROGRESS, USER_COMMAND_UPDATE_PROGRESS, PM_REMOVE);

	//tell the progress to be updated...scale it first to the range desired
	PostMessage(USER_COMMAND_UPDATE_PROGRESS, (WPARAM)(fProgress * PROGRESS_SCALE));

	return true;
}

//---------------------
// Messaging

// This will log a message. It will be associated with the current
// task.
bool CBackEndDialog::LogMessage(EMsgType nStatus, const char* pszMsg)
{
	InternalLogMessage(nStatus, NULL, pszMsg);
	return true;
}

// This will log a message. It will be associated with the current
// task. This version allows the inclusion of a help string so that
// the user can get more information about the message
bool CBackEndDialog::LogMessageH(EMsgType nStatus, const char* pszHelpMsg, const char* pszMsg)
{
	InternalLogMessage(nStatus, pszHelpMsg, pszMsg);
	return true;
}

bool CBackEndDialog::InternalLogMessage(EMsgType nStatus, const char* pszHelp,
										const char* pszStr)
{
	//create the message
	CTaskMessage* pNewMsg = new CTaskMessage(nStatus, pszStr, pszHelp);

	//check for out of memory
	if(pNewMsg == NULL)
		return false;

	//now let the main thread handle things
	PostMessage(USER_COMMAND_ADD_MESSAGE, (WPARAM)pNewMsg);

	return true;
}

BOOL CBackEndDialog::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	//setup the icon for this window
	SetIcon(m_hMainIcon, TRUE);			// Set big icon
	SetIcon(m_hMainIcon, FALSE);		// Set small icon

	//make it so that the OK button is hidden, it will be revealed again once the processing
	//is done
	((CButton*)GetDlgItem(IDOK))->ModifyStyle(WS_VISIBLE, 0);

	//set up the save button to be invisible and have the disk icon
	((CButton*)(GetDlgItem(IDC_BUTTON_SAVE_LOG)))->SetIcon(m_hSaveIcon);
	((CButton*)GetDlgItem(IDC_BUTTON_SAVE_LOG))->ModifyStyle(WS_VISIBLE, 0);

	((CButton*)(GetDlgItem(IDC_BUTTON_MESSAGE_OPTIONS)))->SetIcon(m_hOptionsIcon);

	//setup the progress bar
	CProgressCtrl* pProgress = ((CProgressCtrl*)GetDlgItem(IDC_PROGRESS_TASK));
	pProgress->SetRange(0, 1000);

	//the base registry location
	CString sRegBase = PACKER_REGISTRY_DIRECTORY;

	//set the thread priority to normal
	uint32 nThreadPri = atoi(GetRegistryKey(HKEY_CURRENT_USER, sRegBase + "ThreadPri", "2"));
	((CComboBox*)GetDlgItem(IDC_COMBO_THREAD_PRIORITY))->SetCurSel(nThreadPri);

	//set the message filter to show everything
	uint32 nFilter = atoi(GetRegistryKey(HKEY_CURRENT_USER, sRegBase + "Filter", "5"));
	((CComboBox*)GetDlgItem(IDC_COMBO_MESSAGE_FILTER))->SetCurSel(nFilter);
	OnChangeMessageFilter();

	//create the thread data
	g_ThreadData.m_sFilename		= m_sFilename;
	g_ThreadData.m_pIPackerImpl		= m_pIPackerImpl;
	g_ThreadData.m_pIPackerOutput	= (IPackerOutput*)this;
	g_ThreadData.m_pPropList		= m_pPropList;

	//load the options for the severities
	LoadSevOptionsFromReg();

	//setup the tooltips
	m_ToolTip.Create(this);
	m_ToolTip.AddWindowTool(GetDlgItem(IDC_BUTTON_SAVE_LOG), IDS_TOOLTIP_SAVE_LOG);
	m_ToolTip.AddWindowTool(GetDlgItem(IDC_COMBO_THREAD_PRIORITY), IDS_TOOLTIP_THREAD_PRIORITY);
	m_ToolTip.AddWindowTool(GetDlgItem(IDC_COMBO_MESSAGE_FILTER), IDS_TOOLTIP_MESSAGE_FILTER);
	m_ToolTip.AddWindowTool(GetDlgItem(IDC_PROGRESS_TASK), IDS_TOOLTIP_TASK_PROGRESS);
	m_ToolTip.AddWindowTool(GetTaskList(), IDS_TOOLTIP_TASK_LIST);
	m_ToolTip.AddWindowTool(GetMessageList(), IDS_TOOLTIP_MESSAGE_LIST);
	m_ToolTip.AddWindowTool(GetDlgItem(IDC_BUTTON_MESSAGE_OPTIONS), IDS_TOOLTIP_MESSAGE_OPTIONS);

	//now we need to launch the background thread which will spawn the packer to do its
	//thing
	DWORD nThreadID;
	m_hThread = CreateThread(NULL, 0, LaunchPackerThreadMain, NULL, 0, &nThreadID); 
	
	//now that the thread is created, we need to ensure that the proper priority is set
	//on it
	OnThreadPriorityChanged();
	
	SetTimer(TIMER_EVENT_ID, 50, NULL);	
	
	return TRUE;  
}


void CBackEndDialog::OnTimer(UINT nIDEvent) 
{
	if(nIDEvent == TIMER_EVENT_ID)
	{
		//kill the timer
		KillTimer(TIMER_EVENT_ID);

		//check to see if the thread is still running
		if(IsThreadDone())
		{
			//we are done processing, allow the user to hit OK or
			//save the log
			((CButton*)GetDlgItem(IDOK))->ModifyStyle(0, WS_VISIBLE);
			((CButton*)GetDlgItem(IDC_BUTTON_SAVE_LOG))->ModifyStyle(0, WS_VISIBLE);
			Invalidate();

			//update the title bar to reflect the status
			CString sTitle;
			sTitle.Format("Finished Packing %s", m_sFilename);
			SetWindowText(sTitle);

			//play a 'ding' noise to notify the user
			PlaySound("Default", NULL, SND_ASYNC | SND_NOWAIT | SND_NOSTOP);

			//hide our progress bar since it no longer serves a purpose
			GetDlgItem(IDC_STATIC_TASK_NAME)->EnableWindow(FALSE);
			GetDlgItem(IDC_PROGRESS_TASK)->EnableWindow(FALSE);
			GetDlgItem(IDC_COMBO_THREAD_PRIORITY)->EnableWindow(FALSE);
			GetDlgItem(IDC_STATIC_PRIORITY)->EnableWindow(FALSE);

			//see if we want to just bail
			if(!m_bPauseWhenDone)
				OnOK();
		}
		else
		{
			SetTimer(TIMER_EVENT_ID, 50, NULL);
		}
	}
	
	CDialog::OnTimer(nIDEvent);
}

CListBox* CBackEndDialog::GetTaskList()
{
	return (CListBox*)GetDlgItem(IDC_LIST_TASKS);
}

void CBackEndDialog::SetTaskProgress(float fProgress)
{
	//get the progress bar control
	CProgressCtrl* pProgress = ((CProgressCtrl*)GetDlgItem(IDC_PROGRESS_TASK));

	//set the position (*10 is since it takes integers, and the range is 
	//0 to 1000 for the bar, and 0 to 100 for the progress var (gives finer
	//granularity))
	pProgress->SetPos((int)(fProgress * 10));

	//we also want to update the title bar
	if(m_sSubTaskName.GetLength() > 0)
	{
		if(IsIconic())
		{
			CString sTitle;
			sTitle.Format("%d%% of %s - %s", (int)fProgress, m_sSubTaskName, m_sFilename);

			//we want to avoid changing the text a whole lot since it causes flickering, so
			//make sure that the text is actually different
			CString sOldTitle;
			GetWindowText(sOldTitle);

			if(sOldTitle != sTitle)
			{
				SetWindowText(sTitle);
			}
		}		
	}
}



void CBackEndDialog::OnTaskChanged() 
{
	ResetMessageList();
}

//gets the task that is currently being viewed
CTask* CBackEndDialog::GetViewedTask()
{
	//first get the selected task
	int nSel = GetTaskList()->GetCurSel();

	if(nSel == LB_ERR)
	{
		//no selection
		return NULL;
	}

	//get the actual task
	return (CTask*)GetTaskList()->GetItemData(nSel);
}

//resets the contents of the message list based upon the currently viewed task
void CBackEndDialog::ResetMessageList()
{
	//we need to fill the message list up with the message associated with the
	//newly selected task

	//flush existing messages 
	EmptyMessageList();

	CTask* pTask = GetViewedTask();

	if(pTask)
	{
		//now fill up the message list with all of our messages
		CTaskMessage* pCurrMsg = pTask->GetMessageHead();
		while(pCurrMsg)
		{
			if(ShouldShowMessage(pCurrMsg, m_eMsgFilter))
			{
				//add this string and a pointer to the orinignal message
				AddMessageToList(pCurrMsg);
			}

			pCurrMsg = pCurrMsg->GetNext();
		}
	}

	RedrawMessageList();
}

void CBackEndDialog::OnThreadPriorityChanged() 
{
	//get the selection and map it to a constant
	DWORD nPriorities[] =	{	THREAD_PRIORITY_ABOVE_NORMAL,
								THREAD_PRIORITY_NORMAL,
								THREAD_PRIORITY_BELOW_NORMAL,
								THREAD_PRIORITY_LOWEST	
							};

	DWORD nVal = ((CComboBox*)GetDlgItem(IDC_COMBO_THREAD_PRIORITY))->GetCurSel();
	ASSERT(nVal < (sizeof(nPriorities) / sizeof(nPriorities[0])));

	SetThreadPriority(m_hThread, nPriorities[nVal]);	

	//save this new value to the registry
	CString sVal;
	sVal.Format("%d", nVal);
	CString sRegKey = PACKER_REGISTRY_DIRECTORY;

	SetRegistryKey(HKEY_CURRENT_USER, sRegKey + "ThreadPri", sVal);
}

void CBackEndDialog::OnCancel()
{
	//we are canceling, we need to kill the thread
	TerminateThread(m_hThread, 0);

	CDialog::OnCancel();
}

void CBackEndDialog::OnSaveLog() 
{
	CFileDialog Dlg(FALSE, "*.txt", NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, 
					"Text Files (*.txt)|*.txt|All Files (*.*)|*.*||");

	if(Dlg.DoModal() == IDOK)
	{
		if(SaveLogToFile(Dlg.GetPathName()) == false)
		{
			MessageBox("Unable to open the specified file for saving", "Error", MB_ICONEXCLAMATION | MB_OK);
		}		
	}
}

//saves all the messages and tasks to a file of the specified name
bool CBackEndDialog::SaveLogToFile(const char* pszFilename)
{
	//open up the file and write out all the tasks and all their messages
#if _MSC_VER >= 1300
	std::ofstream OutFile(pszFilename);
#else
	ofstream OutFile(pszFilename);
#endif

	if(OutFile.fail())
	{
		return false;
	}

	CTask* pCurrTask = m_pHeadTask;

	while(pCurrTask)
	{
		//write out the task title
		OutFile << "Task: " << pCurrTask->GetName() << "\n\n";

		//write out the messages
		CTaskMessage* pCurrMsg = pCurrTask->GetMessageHead();
		
		while(pCurrMsg)
		{
			OutFile << "\t" << FormatFinalMsg(pCurrMsg);
			pCurrMsg = pCurrMsg->GetNext();
		}

		pCurrTask = pCurrTask->GetNextTask();
	}

	OutFile.close();

	return true;
}

LRESULT CBackEndDialog::OnAddMessage(WPARAM wParam, LPARAM lParam)
{
	//get the text
	CTaskMessage* pMsg = (CTaskMessage*)wParam;
	
	//now add that message to the current task
	if(m_pCurrTask)
	{
		m_pCurrTask->AddMessage(pMsg->GetSeverity(), pMsg->GetMessageText(), pMsg->GetHelp());

		//see if this is the task currently being viewed
		CTask* pViewed = GetViewedTask();

		if(pViewed == m_pCurrTask)
		{
			if(ShouldShowMessage(pMsg, m_eMsgFilter))
			{
				//we need to add this message to the list box
				AddMessageToList(pMsg, true);
			}
		}
	}
	
	//free the memory
	delete pMsg;
	
	return 0;
}

LRESULT CBackEndDialog::OnAddTask(WPARAM wParam, LPARAM lParam)
{
	//extract the task
	CTask* pNewTask = (CTask*)wParam;
	ASSERT(pNewTask);

	//see if this task is already in the list
	CTask* pCurr = m_pHeadTask;
	while(pCurr)
	{
		if(stricmp(pCurr->GetName(), pNewTask->GetName()) == 0)
		{
			//already have a task of that name
			delete pNewTask;
			return 0;
		}
		pCurr = pCurr->GetNextTask();
	}

	if(m_pHeadTask == NULL)
	{
		m_pHeadTask = pNewTask;
	}
	else
	{
		//add it onto the end
		CTask* pCurr = m_pHeadTask;
		while(pCurr->GetNextTask())
			pCurr = pCurr->GetNextTask();

		pCurr->SetNextTask(pNewTask);
	}

	//now we can update the list control
	int nIndex = GetTaskList()->AddString(pNewTask->GetName());
	GetTaskList()->SetItemData(nIndex, (DWORD)pNewTask);

	return 0;
}

LRESULT CBackEndDialog::OnActivateTask(WPARAM wParam, LPARAM lParam)
{
	//get the text
	char* pszTaskName = (char*)wParam;

	//track the old task
	CTask* pOldTask = m_pCurrTask;

	//run through the list and find the task
	CTask* pCurr = m_pHeadTask;

	while(pCurr)
	{
		if(stricmp(pszTaskName, pCurr->GetName()) == 0)
		{
			m_pCurrTask = pCurr;
			break;
		}
		pCurr = pCurr->GetNextTask();
	}

	//clean up the name now
	delete [] pszTaskName;
	pszTaskName = NULL;

	//see if we didn't find the task
	if(pCurr == NULL)
	{
		return false;
	}

	//set the active task name around the progress bar
	CStatic* pTaskName = (CStatic*)GetDlgItem(IDC_STATIC_TASK_NAME);
	CString sText;
	sText.Format("Task: %s", (m_pCurrTask) ? m_pCurrTask->GetName() : "None");
	((CStatic*)GetDlgItem(IDC_STATIC_TASK_NAME))->SetWindowText(sText);

	//also update the progress bar to reflect the change
	SetTaskProgress((m_pCurrTask) ? m_pCurrTask->GetProgress() : 0.0f);

	//if the user was viewing the previous task, then we want to start viewing the new
	//task, the same goes for if the user isn't viewing any tasks
	CTask* pViewedTask = GetViewedTask();
	if((pOldTask == pViewedTask) || (pViewedTask == NULL))
	{
		//find this new task
		for(uint32 nCurrTask = 0; nCurrTask < (uint32)GetTaskList()->GetCount(); nCurrTask++)
		{
			if((CTask*)GetTaskList()->GetItemData(nCurrTask) == m_pCurrTask)
			{
				//this is a match, select it
				GetTaskList()->SetCurSel(nCurrTask);
				ResetMessageList();
				break;
			}
		}
	}

	//save this task as our current subtask
	m_sSubTaskName = (m_pCurrTask) ? m_pCurrTask->GetName() : "";

	//update the title bar
	UpdateTitleNonIconic();	
	
	
	return 0;
}

LRESULT CBackEndDialog::OnActivateSubTask(WPARAM wParam, LPARAM lParam)
{
	//get the text
	char* pszTaskName = (char*)wParam;

	//save this task name as our current subtask
	m_sSubTaskName = pszTaskName;

	//clean up the name now
	delete [] pszTaskName;
	pszTaskName = NULL;

	//set the active task name around the progress bar
	CStatic* pTaskName = (CStatic*)GetDlgItem(IDC_STATIC_TASK_NAME);
	CString sText;
	sText.Format("Task: %s", m_sSubTaskName);
	((CStatic*)GetDlgItem(IDC_STATIC_TASK_NAME))->SetWindowText(sText);

	//also update the progress bar to reflect the change
	SetTaskProgress(0.0f);
	
	//update the title bar
	UpdateTitleNonIconic();	
	
	return 0;
}

LRESULT CBackEndDialog::OnUpdateProgress(WPARAM wParam, LPARAM lParam)
{
	float fProgress = wParam / (float)PROGRESS_SCALE;

	if(m_pCurrTask)
	{
		m_pCurrTask->SetProgress(fProgress);

		//update the progress bar
		SetTaskProgress(m_pCurrTask->GetProgress());
	}

	return 0;
}


void CBackEndDialog::OnChangeMessageFilter() 
{
	//get the selected item
	int nSel = ((CComboBox*)GetDlgItem(IDC_COMBO_MESSAGE_FILTER))->GetCurSel();

	if(nSel == CB_ERR)
		return;

	//update the minimum severity
	m_eMsgFilter = static_cast<EMsgType>(nSel);
	ResetMessageList();

	//save this new value to the registry
	CString sVal;
	sVal.Format("%d", nSel);
	CString sRegKey = PACKER_REGISTRY_DIRECTORY;

	SetRegistryKey(HKEY_CURRENT_USER, sRegKey + "Filter", sVal);
}

void CBackEndDialog::OnSize(UINT nType, int cx, int cy) 
{
	CDialog::OnSize(nType, cx, cy);

	//just update the title text

	//lock to make sure we don't conflict with other sets to the title
	if(!IsIconic())
	{
		UpdateTitleNonIconic();	
	}
}

//given a message it will return a string that should be displayed based upon the
//options
CString CBackEndDialog::FormatFinalMsg(const CTaskMessage* pMsg) const
{
	CString rv;

	rv.Format("%s%s\r\n", m_sSevPrefix[pMsg->GetSeverity()], pMsg->GetMessageText());

	return rv;
}

//this will update the text of the title window when it is not iconic
void CBackEndDialog::UpdateTitleNonIconic()
{
	CString sTitle;

	if(m_pCurrTask)
	{
		sTitle.Format("%s - %s", m_pCurrTask->GetName(), m_sFilename);
	}
	else
	{
		sTitle.Format("%s", m_sFilename);
	}

	SetWindowText(sTitle);	
}


//adds a message for displaying on the message list
void CBackEndDialog::AddMessageToList(CTaskMessage* pMsg, bool bForceVisible)
{
	CRichEditCtrl* pEdit = GetMessageList();

	//save the original selection
	long nOrigStart, nOrigEnd;
	pEdit->GetSel(nOrigStart, nOrigEnd);

	pEdit->SetSel(-1, -1);

	//save this selection
	long nStartSel, nEndSel;
	pEdit->GetSel(nStartSel, nEndSel);

	//now add in our message text
	pEdit->ReplaceSel(FormatFinalMsg(pMsg));

	//select the text we just added
	pEdit->SetSel(nStartSel, -1);
	
	//change the font for the text we just selected
	CHARFORMAT Format;

	pEdit->GetSelectionCharFormat(Format);
	GetMessageFormat(pMsg, Format);
	pEdit->SetSelectionCharFormat(Format);

	//now clear the selection
	pEdit->SetSel(nOrigStart, nOrigEnd);

	if(bForceVisible)
	{
		//pEdit->LineScroll(10000);
	}
}

//causes the message list to be refreshed
void CBackEndDialog::RedrawMessageList()
{
	GetMessageList()->Invalidate();
}

//clears out all contents from the message list
void CBackEndDialog::EmptyMessageList()
{
	CRichEditCtrl* pEdit = GetMessageList();

	pEdit->SetSel(0, -1);
	pEdit->ReplaceSel("");
}

CRichEditCtrl* CBackEndDialog::GetMessageList()
{
	return (CRichEditCtrl*)GetDlgItem(IDC_RICHEDIT_MESSAGES);
}

//gets the formatting options for a specified message
void CBackEndDialog::GetMessageFormat(const CTaskMessage* pMsg, CHARFORMAT& Format) const
{
	Format.dwMask		|= CFM_COLOR;
	Format.dwEffects	&= ~CFE_AUTOCOLOR;
	Format.crTextColor	= m_SevColor[pMsg->GetSeverity()];
}


void CBackEndDialog::OnButtonMessageOptions() 
{
	//bring up a message options dialog
	CMessageOptions Dlg;

	//load all the settings into the dialog
	for(uint32 nCurrSev = 0; nCurrSev < NUM_SEVERITIES; nCurrSev++)
	{
		Dlg.m_Colors[nCurrSev]		= m_SevColor[nCurrSev];
		Dlg.m_sPrefixes[nCurrSev]	= m_sSevPrefix[nCurrSev];
	}

	if(Dlg.DoModal() == IDOK)
	{
		//extract the settings from the dialog
		for(uint32 nCurrSev = 0; nCurrSev < NUM_SEVERITIES; nCurrSev++)
		{
			m_SevColor[nCurrSev]  = Dlg.m_Colors[nCurrSev];
			m_sSevPrefix[nCurrSev] = Dlg.m_sPrefixes[nCurrSev];
		}

		//save it to the registry as well
		SaveSevOptionsToReg();

		//reset the contents of the view
		ResetMessageList();
	}	
}

//builds up the regiistry key name for the given severity
static CString BuildSevKeyName(uint32 nCurrSev)
{
	CString rv;
	rv.Format("%sSeverity%d", PACKER_REGISTRY_DIRECTORY, nCurrSev);
	return rv;
}

//loads all the severity options from the registry (for color and things like that)
void CBackEndDialog::LoadSevOptionsFromReg()
{
	for(uint32 nCurrSev = 0; nCurrSev < NUM_SEVERITIES; nCurrSev++)
	{
		CString sKey = BuildSevKeyName(nCurrSev);

		m_SevColor[nCurrSev]	= atoi(GetRegistryKey(HKEY_CURRENT_USER, sKey + "Color", "0"));
		m_sSevPrefix[nCurrSev]	= GetRegistryKey(HKEY_CURRENT_USER, sKey + "Prefix", "");
	}
}

//saves severity options to the registry
void CBackEndDialog::SaveSevOptionsToReg()
{
	for(uint32 nCurrSev = 0; nCurrSev < NUM_SEVERITIES; nCurrSev++)
	{
		CString sKey = BuildSevKeyName(nCurrSev);

		CString sColor;
		sColor.Format("%d", m_SevColor[nCurrSev]);
		SetRegistryKey(HKEY_CURRENT_USER, sKey + "Color", sColor);
		SetRegistryKey(HKEY_CURRENT_USER, sKey + "Prefix", m_sSevPrefix[nCurrSev]);
	}
}

//determines if the thread is done or not
BOOL CBackEndDialog::IsThreadDone()
{
	DWORD nStatus;
	BOOL bRV = GetExitCodeThread(m_hThread, &nStatus);

	if(bRV && (nStatus != STILL_ACTIVE))
	{
		return TRUE;
	}

	return FALSE;
}

void CBackEndDialog::OnOK() 
{
	//we can't bail unless the thread is done
	if(!IsThreadDone())
		return;
	
	CDialog::OnOK();
}

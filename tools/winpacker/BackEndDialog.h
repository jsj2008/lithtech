#ifndef __BACKENDDIALOG_H__
#define __BACKENDDIALOG_H__

#ifndef __IPACKEROUTPUT_H__
#	include "IPackerOutput.h"
#endif

#ifndef __TASK_H__
#	include "Task.h"
#endif

#ifndef __AUTOTOOLTIPCTRL_H__
#	include "AutoToolTipCtrl.h"
#endif

#ifndef __MESSAGEOPTIONS_H__
#	include "MessageOptions.h"
#endif

class IPackerImpl;
class CPackerPropList;

/////////////////////////////////////////////////////////////////////////////
// CBackEndDialog dialog

class CBackEndDialog : 
	public CDialog, public IPackerOutput
{
// Construction
public:
	CBackEndDialog(IPackerImpl* m_pImpl, CPackerPropList* pPropList, CWnd* pParent = NULL);
	~CBackEndDialog();

	//the name of the file being packed
	CString		m_sFilename;

	//whether or not it should pause at the end or if it should simply close
	BOOL		m_bPauseWhenDone;

// Dialog Data
	//{{AFX_DATA(CBackEndDialog)
	enum { IDD = IDD_BACKEND_DIALOG };
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CBackEndDialog)
	//}}AFX_VIRTUAL

// IPackerOutput

	// This will create a task and add it to the task queue. Tasks
	// can be associated with messages as well as a progress indicator.
	virtual bool	CreateTask(const char* pszTaskName);

	// This function allows for a task to be activated, meaning that
	// all messages sent will be associated with this task, as will
	// all progress messages
	virtual bool	ActivateTask(const char* pszTaskName);

	// This function allows for a sub task to be started. This does not create
	// a new task or redirect messages, only changes possible UI for a sub
	// task that is being performed as part of a larger task. When you switch
	// to a task, that task also becomes the active subtask
	virtual bool	ActivateSubTask(const char* pszSubTaskName);

	// This will update the progress of the currently active task. The
	// value can range from 0 to 100, where 0 is starting, and 100 is
	// completed
	virtual bool	UpdateProgress(float fProgress);

	//---------------------
	// Messaging

	// This will log a message. It will be associated with the current
	// task.
	virtual bool	LogMessage(	EMsgType nStatus, const char* pszMsg);

	// This will log a message. It will be associated with the current
	// task. This version allows the inclusion of a help string so that
	// the user can get more information about the message
	virtual bool	LogMessageH(EMsgType nStatus, const char* pszHelpMsg,
								const char* pszMsg);

// Implementation
protected:

	//-------------------------
	// Message list handlers
	//
	// these are broken apart to allow 
	// easy replacement of the message list
	// with different control types

	//adds a message for displaying on the message list
	void					AddMessageToList(CTaskMessage* pMsg, bool bForceVisible = false);

	//causes the message list to be refreshed
	void					RedrawMessageList();

	//clears out all contents from the message list
	void					EmptyMessageList();

	//gets the formatting options for a specified message
	void					GetMessageFormat(const CTaskMessage* pMsg, CHARFORMAT& Format) const;



	//given a message it will return a string that should be displayed based upon the
	//options
	CString					FormatFinalMsg(const CTaskMessage* pMsg) const;

	//this will update the text of the title window when it is not iconic
	void					UpdateTitleNonIconic();

	//saves all the messages and tasks to a file of the specified name
	bool					SaveLogToFile(const char* pszFilename);

	void					OnCancel();

	CListBox*				GetTaskList();
	CRichEditCtrl*			GetMessageList();

	void					SetTaskProgress(float fProgress);

	bool					InternalLogMessage(EMsgType nStatus, const char* pszHelp,
												const char* pszStr);

	//loads all the severity options from the registry (for color and things like that)
	void					LoadSevOptionsFromReg();

	//saves severity options to the registry
	void					SaveSevOptionsToReg();

	//gets the task that is currently being viewed
	CTask*					GetViewedTask();

	//resets the contents of the message list based upon the currently viewed task
	void					ResetMessageList();

	//determines if the thread is done or not
	BOOL					IsThreadDone();

	//the icon for the save button
	HICON					m_hSaveIcon;
	HICON					m_hMainIcon;
	HICON					m_hOptionsIcon;

	CAutoToolTipCtrl		m_ToolTip;

	//the head task
	CTask*					m_pHeadTask;

	//the currently active task, messages and progress updates go here
	CTask*					m_pCurrTask;

	//the name of the subtask that we are currently working on
	CString					m_sSubTaskName;

	//the minimum severity that a message must have to be displayed
	EMsgType				m_eMsgFilter;

	//the packer implementation used
	IPackerImpl*			m_pIPackerImpl;

	//the property list to give to the packer
	CPackerPropList*		m_pPropList;
	
	//the thread handle
	HANDLE					m_hThread;

	//colors used for each severity
	COLORREF				m_SevColor[NUM_SEVERITIES];

	//prefixes to append onto each severity
	CString					m_sSevPrefix[NUM_SEVERITIES];

	// Generated message map functions
	//{{AFX_MSG(CBackEndDialog)
	virtual BOOL OnInitDialog();
	afx_msg void OnTimer(UINT nIDEvent);
	afx_msg void OnTaskChanged();
	afx_msg void OnThreadPriorityChanged();
	afx_msg void OnSaveLog();
	afx_msg void OnChangeMessageFilter();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg LRESULT OnAddMessage(WPARAM, LPARAM);
	afx_msg LRESULT OnAddTask(WPARAM, LPARAM);
	afx_msg LRESULT OnActivateTask(WPARAM, LPARAM);
	afx_msg LRESULT OnActivateSubTask(WPARAM, LPARAM);
	afx_msg LRESULT OnUpdateProgress(WPARAM, LPARAM);
	afx_msg void OnButtonMessageOptions();
	virtual void OnOK();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}

#endif

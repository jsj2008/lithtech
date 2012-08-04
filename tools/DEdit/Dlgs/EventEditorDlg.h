#ifndef __EVENTEDITORDLG_H__
#define __EVENTEDITORDLG_H__

#ifndef __AUTOTOOLTIPCTRL_H__
#	include "AutoToolTipCtrl.h"
#endif

/////////////////////////////////////////////////////////////////////////////
// CKeyEventHandler
//
//  A class whos only purpose is to subclass a dialog
//  control for keyboard input and forward it to the
//  specified dialog
class CEventEditorDlg;

class CKeyEventHandler :
	public CWnd
{
public:

	CKeyEventHandler()		{}
	~CKeyEventHandler()		{}

	void SetDlg(CEventEditorDlg* pDlg)	{ m_pDlg = pDlg; }

	afx_msg UINT OnGetDlgCode();
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCount, UINT nFlags);

	DECLARE_MESSAGE_MAP();

private:
	CEventEditorDlg*	m_pDlg;
};

/////////////////////////////////////////////////////////////////////////////
// CObjectEvent
//
//  the actual object that holds information about an event
class CObjectEvent
{
public:
	
	CObjectEvent() :
		m_pNext(NULL),
		m_nTrack(0),
		m_nTrackTime(0),
		m_bSelected(false)
	{
	}

	//Deletes all objects on down the chain when destroyed
	~CObjectEvent()
	{
		delete m_pNext;
	}

	//the track it is located on
	uint32			m_nTrack;

	//the time it is located at (MS from the start)
	uint32			m_nTrackTime;

	//the string associated with the event
	CString			m_sText;

	//whether or not this is selected
	bool			m_bSelected;

	//next event in the list
	CObjectEvent*	m_pNext;
};

/////////////////////////////////////////////////////////////////////////////
// CEventEditorDlg dialog
//
// Dialog that handles the editing of an event schematic. 

class CPropertiesControls;

class CEventEditorDlg : public CDialog
{
// Construction
public:
	CEventEditorDlg(CPropertiesControls* pControlDlg, CWnd* pParent = NULL);   // standard constructor

	enum { IDD = IDD_EVENTEDITOR };

	enum { MAX_TRACKS		= 8 };

	//--------------------------------
	//interface for key handler

	//will nudge all selected nodes the specified number of MS
	void			NudgeSelection(int32 nMS);

	//deletes all the currently selected nodes
	void			DeleteSelectedEvents();

	//adds an event at the current mouse position
	void			AddEvent(const char* pszText = "");

protected:

	//accessors for a variety of controls
	CEdit*				GetScaleCtrl()			{ return ((CEdit*)GetDlgItem(IDC_EDIT_EVENT_SCALE)); }
	CEdit*				GetLengthCtrl()			{ return ((CEdit*)GetDlgItem(IDC_EDIT_EVENT_LENGTH)); }
	CEdit*				GetMessageCtrl()		{ return ((CEdit*)GetDlgItem(IDC_EDIT_EVENT_MESSAGE)); }
	CEdit*				GetTimeCtrl()			{ return ((CEdit*)GetDlgItem(IDC_EDIT_EVENT_TIME)); }
	CEdit*				GetImmMessageCtrl()		{ return ((CEdit*)GetDlgItem(IDC_EDIT_EVENT_IMM_MESSAGE)); }
	CSpinButtonCtrl*	GetScaleSpinCtrl()		{ return ((CSpinButtonCtrl*)GetDlgItem(IDC_SPIN_EVENT_SCALE)); }
	CSpinButtonCtrl*	GetLengthSpinCtrl()		{ return ((CSpinButtonCtrl*)GetDlgItem(IDC_SPIN_EVENT_LENGTH)); }
	CScrollBar*			GetTimeScroll()			{ return ((CScrollBar*)GetDlgItem(IDC_SCROLLBAR_EVENT_TIME)); }

	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();

	void	OnOK();
	void	OnCancel();

	//cleans up a dialog after either OK or cancel
	void	CleanupDialog();

	//sets the time text for the dialog, passing a negative value indicates an invalid time
	//The value is in milliseconds
	void	SetTimeText(int32 nTime);

	//sets the number of messages text
	void	SetNumMessagesText(uint32 nNumMsgs);

	//sets the track messages text, negative indicates that it should be set to an invalid
	//state
	void	SetTrackMessagesText(int32 nNumMsgs);

	//updates the counts of the objects and displays them. 
	void	UpdateMessageCounts();

	//gets the top position of a control
	uint32	GetTrackTop(uint32 nTrack);

	//gets the extents of the messages on a track
	void	GetTrackExtents(uint32 nTrack, uint32& nMin,  uint32& nMax);

	//renders an individual track
	void	RenderTrack(uint32 nTrack, CDC& dc);

	//renders the actual object view
	void	RenderEventView(CDC& dc);

	//renders the ruler beneath the track view
	void	RenderRuler(CDC& dc);

	//initializes the dialog from the member object
	bool	InitFromPropList();

	//given the ID of a control, it will find it, remove it from the dialog, and return
	//the rectangle that it occupied
	CRect	RemovePlaceholderCtrl(uint32 nID, bool bHide = true);

	//Gets the default font object of the dialog
	CFont*	GetDefaultFont();

	//converts a time to a x coordinate on the screen
	int32	TimeToScreen(uint32 nTime);

	//converts an x coordinate on the scree to a time coordinate
	int32	ScreenToTime(uint32 nScreen);

	//updates the scrollbar with the appropriate info
	void	UpdateScrollBar();

	//deletes the specified node
	void	DeleteEvent(CObjectEvent* pEvent);

	//deletes all events on the specified track
	void	DeleteTrack(uint32 nTrack);

	//renders the main controls when something has changed. This should not be called for a repaint of the screen
	void	RedrawView(bool bRedrawRuler = true);

	//gets the number of events on a specified track
	uint32	GetNumEventsOnTrack(uint32 nTrack);
	
	//gets the name of the property used to store the specified event
	CString	GetPropName(uint32 nTrack, uint32 nMessage);

	//saves the results of the dialog back into the member property list
	void	SavePropList();

	//Sorts the events in the order of track first, then time
	void	SortEvents();

	//gets a property from the held list, returns NULL if not found or it is of a different
	//type
	CBaseProp*		GetProperty(const char* pszPropName, uint32 nType = PT_STRING);

	//gets the event that is beneath the specified point. NULL if no event is beneath it
	CObjectEvent*	GetEventAt(CPoint Pt);

	//given a node, it will move it to the end of the event list
	void			BringToFront(CObjectEvent* pEvent);

	//clears the selected status from all events
	void			DeselectAll();

	//redraws the specified event
	void			RedrawEvent(CObjectEvent* pEvent);

	//call this to update the UI to reflect any selection changes
	void			UpdateSelections();

	//handles updating the text inside of the message box, should be called when a selection changes
	void			UpdateMessageText();

	//handles updating the text inside of the time box, should be called when a selection changes
	void			UpdateTimeText();

	//the scale (number of seconds visible in the window)
	uint32			m_nScale;

	//the length of the entire event
	uint32			m_nLength;

	//the current position along the time bar
	uint32			m_nTimePos;

	//the number of tracks in use
	uint32			m_nNumTracks;

	//the maximum number of events per track
	uint32			m_nMaxEventsPerTrack;

	//the currently active track (the track under the mouse)
	uint32			m_nActiveTrack;

	//the string that is currently on the clipboard
	CString			m_sClipBoard;

	//flag indicating if the clipboard is valid
	bool			m_bValidClipBoard;

	//the prefix for the actual properties
	CString			m_sPropPrefix;
	
	//rectangles for the major user drawn controls
	CRect			m_rRuler;
	CRect			m_rEventView;
	CRect			m_rTrackLabels;

	//Drag information

	//the event being potentially dragged
	CObjectEvent*	m_pDragEvent;

	//whether or not we are in an actual drag
	bool			m_bInDrag;

	//the point where the potentially dragged event started being dragged from
	CPoint			m_ptDragStart;

	//controls for editing the labels of each track
	CEdit*			m_pLabels[MAX_TRACKS];

	//the control dialog that created this dialog (used for property notification, etc)
	CPropertiesControls*	m_pPropCtrls;

	//The object which will be edited
	CPropList*		m_pPropList;

	//The head of the list of the events
	CObjectEvent*	m_pHeadEvent;

	//the previous mouse position
	CPoint			m_ptLastMouse;

	//determines whether or not it is safe to update the text of the selected
	//nodes with changed message text (avoids annoying windows message issues)
	bool			m_bSupressMessageChanges;
	bool			m_bSupressTimeChanges;

	//the control that handles all the tooltips
	CAutoToolTipCtrl	m_ToolTip;

	//The key event handler which handles all control keyboard input
	CKeyEventHandler	m_KeyHandler;
	
	// Generated message map functions
	//{{AFX_MSG(CEventEditorDlg)
	afx_msg void OnPaint(void);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnScaleChanged();
	afx_msg void OnLengthChanged();
	afx_msg void OnMessageChanged();
	afx_msg void OnTimeChanged();
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint Pt);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint Pt);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint Pt);
	afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint Pt);
	afx_msg void OnButtonList();
	afx_msg void OnButtonEditMessage();


	//Handler for popup menus
	afx_msg void OnPopupDeleteAll();
	afx_msg void OnPopupDelete();
	afx_msg void OnPopupDeleteTrack();
	afx_msg void OnPopupAdd();
	afx_msg void OnPopupCut();
	afx_msg void OnPopupCopy();
	afx_msg void OnPopupPaste();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

#endif 

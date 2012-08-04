// EventEditorDlg.cpp : implementation file
//

#include "bdefs.h"
#include "dedit.h"
#include "editobjects.h"
#include "EventEditorDlg.h"
#include "EventListDlg.h"
#include "RegionDoc.h"
#include "PropertyHelpers.h"
#include "PropertiesDlg.h"
#include "MultiLineStringDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

//Macros
#define BASE_LABEL_ID				1000
#define MAX_EVENT_LENGTH			3000
#define MAX_EVENT_SCALE				3000
#define TRACK_HEIGHT				20
#define VIEW_VERTICAL_PADDING		10
#define RULER_LABEL_PADDING			3
#define RULER_TICK_PADDING			5
#define EXTENT_VERTICAL_PADDING		3
#define VIEW_EVENT_RADIUS			(TRACK_HEIGHT / 2)
#define INVALID_TRACK				(CEventEditorDlg::MAX_TRACKS + 1)

//the amout of pixels the user needs to move the mouse before drag takes into effect
// (prevent minute accidental adjustments)
#define MOUSE_DRAG_DEADZONE			2

//number of lines to scroll per mouse wheel unit (very hard to define)
#define	MOUSE_WHEEL_SCALE			10

//number of scroll bar positions per second (note that this * MAX_EVENT_LENGTH must be less than 2^15)
#define SCROLL_BAR_RES				10

//amount of scroll bar notches to move with one line
#define SCROLL_LINE_AMOUNT			1

//converts from seconds to milliseconds
#define S_TO_MS(x)					((x) * 1000)

//converts to seconds from milliseconds
#define MS_TO_S(x)					((x) / 1000)

//converts from MS to scroll bar units
#define MS_TO_SB(x)					((x) * SCROLL_BAR_RES / 1000)

//converts from SB units to MS
#define SB_TO_MS(x)					((x) * 1000 / SCROLL_BAR_RES)

//------------------------------------------------------
//Helper Functions
//------------------------------------------------------
bool MyStrTok(const char* pszStr, uint32& nOffset, char* pszBuff, uint32 nBuffLen)
{
	assert(pszStr);
	assert(pszBuff);
	assert(nBuffLen > 0);

	uint32 nBuffIndex	= 0;

	//skip over preceding whitespace
	while(isspace(pszStr[nOffset]))
		nOffset++;

	//check for if we hit the eos
	if(pszStr[nOffset] == '\0')
		return false;

	//see if we hit a quote, if so, read a string
	if(pszStr[nOffset] == '\'')
	{
		nOffset++;
		while(pszStr[nOffset] != '\'')
		{
			if(pszStr[nOffset] == '\0')
				return false;

			if(nBuffIndex < (nBuffLen - 1))
			{
				pszBuff[nBuffIndex] = pszStr[nOffset];
				nBuffIndex++;
			}
			nOffset++;
		}

		//move past the ending quote
		nOffset++;
	}
	//not a string, so just read in a single word
	else
	{
		while(1)
		{
			if(isspace(pszStr[nOffset]) || (pszStr[nOffset] == '\0'))
				break;

			if(nBuffIndex < (nBuffLen - 1))
			{
				pszBuff[nBuffIndex] = pszStr[nOffset];
				nBuffIndex++;
			}
			nOffset++;
		}
	}

	//make sure we have an ending character
	pszBuff[nBuffIndex] = '\0';

	return true;
}

//------------------------------------------------------
// CKeyEventHandler
//------------------------------------------------------

BEGIN_MESSAGE_MAP(CKeyEventHandler, CWnd)
	ON_WM_KEYDOWN()
	ON_WM_GETDLGCODE()
END_MESSAGE_MAP()

UINT CKeyEventHandler::OnGetDlgCode()
{
	return DLGC_WANTARROWS | DLGC_WANTCHARS;
}

void CKeyEventHandler::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	switch(nChar)
	{
	case VK_LEFT:	
		m_pDlg->NudgeSelection(-10 * nRepCnt);	
		break;
	case VK_RIGHT:	
		m_pDlg->NudgeSelection(10 * nRepCnt);	
		break;
	case VK_DELETE:
		m_pDlg->DeleteSelectedEvents();
		break;
	case VK_INSERT:
		m_pDlg->AddEvent();
		break;
	default:
		CWnd::OnSysChar(nChar, nRepCnt, nFlags);
		break;
	}
}

/////////////////////////////////////////////////////////////////////////////
// CEventEditorDlg dialog
BEGIN_MESSAGE_MAP(CEventEditorDlg, CDialog)
	ON_WM_PAINT()
	ON_WM_MOUSEMOVE()
	ON_WM_HSCROLL()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_CONTEXTMENU()
	ON_WM_MOUSEWHEEL()

	//buttons
	ON_BN_CLICKED(IDC_BUTTON_LIST, OnButtonList)
	ON_BN_CLICKED(IDC_BUTTON_EDIT_MESSAGE, OnButtonEditMessage)

	//edit boxes
	ON_EN_CHANGE(IDC_EDIT_EVENT_SCALE, OnScaleChanged)
	ON_EN_CHANGE(IDC_EDIT_EVENT_LENGTH, OnLengthChanged)
	ON_EN_CHANGE(IDC_EDIT_EVENT_MESSAGE, OnMessageChanged)
	ON_EN_CHANGE(IDC_EDIT_EVENT_TIME, OnTimeChanged)

	//popup items
	ON_COMMAND(ID_EVENTEDITOR_DELETEALL, OnPopupDeleteAll)
	ON_COMMAND(ID_EVENTEDITOR_DELETE, OnPopupDelete)
	ON_COMMAND(ID_EVENTEDITOR_DELETETRACK, OnPopupDeleteTrack)
	ON_COMMAND(ID_EVENTEDITOR_ADD, OnPopupAdd)
	ON_COMMAND(ID_EVENTEDITOR_CUT, OnPopupCut)
	ON_COMMAND(ID_EVENTEDITOR_COPY, OnPopupCopy)
	ON_COMMAND(ID_EVENTEDITOR_PASTE, OnPopupPaste)

END_MESSAGE_MAP()


CEventEditorDlg::CEventEditorDlg(CPropertiesControls* pPropCtrls, CWnd* pParent /*=NULL*/) :
	CDialog(CEventEditorDlg::IDD, pParent),
	m_pPropCtrls(pPropCtrls),
	m_pPropList(pPropCtrls->GetPropList()),
	m_pHeadEvent(NULL),
	m_bValidClipBoard(false),
	m_nTimePos(0),
	m_nScale(10),
	m_nLength(60),
	m_nActiveTrack(INVALID_TRACK),
	m_nNumTracks(1),
	m_pDragEvent(NULL),
	m_bInDrag(false),
	m_bSupressMessageChanges(false),
	m_bSupressTimeChanges(false)

{
	//make sure that we have an object to get/store properties
	assert(m_pPropList);

	for(uint32 nCurrLabel = 0; nCurrLabel < CEventEditorDlg::MAX_TRACKS; nCurrLabel++)
	{
		m_pLabels[nCurrLabel] = NULL;
	}

	//setup the handler's reference to this
	m_KeyHandler.SetDlg(this);
}

//sets the time text for the dialog, passing a negative value indicates an invalid time
void CEventEditorDlg::SetTimeText(int32 nTime)
{
	//check for invalid
	if(nTime < 0)
	{
		GetDlgItem(IDC_STATIC_CURR_TIME)->SetWindowText("N/A");
	}
	else
	{
		CString sText;
		sText.Format("%.1fs", (nTime + 50) / 1000.0f);
		GetDlgItem(IDC_STATIC_CURR_TIME)->SetWindowText(sText);
	}
}

//sets the number of messages text
void CEventEditorDlg::SetNumMessagesText(uint32 nNumMsgs)
{
	CString sText;
	sText.Format("%d", nNumMsgs);
	GetDlgItem(IDC_STATIC_NUM_MESSAGES)->SetWindowText(sText);
}

//sets the track messages text
void CEventEditorDlg::SetTrackMessagesText(int32 nNumMsgs)
{
	CString sText;

	if(nNumMsgs < 0)
		sText = "N/A";
	else
		sText.Format("%d/%d", nNumMsgs, m_nMaxEventsPerTrack);

	GetDlgItem(IDC_STATIC_TRACK_MESSAGES)->SetWindowText(sText);
}

void CEventEditorDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSpherePrimitiveDlg)
	//}}AFX_DATA_MAP
}

//Updates the counts of messages in the display fields
void CEventEditorDlg::UpdateMessageCounts()
{
	//run through the list and count up the messages
	uint32 nActive = 0;
	uint32 nTotal = 0;

	CObjectEvent* pCurr = m_pHeadEvent;
	while(pCurr)
	{
		if(pCurr->m_nTrack == m_nActiveTrack)
			nActive++;
		nTotal++;

		pCurr = pCurr->m_pNext;
	}


	SetNumMessagesText(nTotal);

	if(m_nActiveTrack == INVALID_TRACK)
		SetTrackMessagesText(-1);
	else
		SetTrackMessagesText(nActive);
}

//gets the top position of a control
uint32 CEventEditorDlg::GetTrackTop(uint32 nTrack)
{
	assert(nTrack < m_nNumTracks);

	//start it out at the top
	uint32 nRV = m_rTrackLabels.top + TRACK_HEIGHT / 2 + VIEW_VERTICAL_PADDING;

	//now we need to shift it to the approprate position
	uint32 nHeight = m_rTrackLabels.Height() - TRACK_HEIGHT - VIEW_VERTICAL_PADDING * 2;

	//now get the position
	if(m_nNumTracks > 1)
		nRV += nHeight * nTrack / (m_nNumTracks - 1);
	else
		nRV += nHeight / 2;

	//shift it back
	nRV -= TRACK_HEIGHT / 2;

	return nRV;
}

//gets the name of the property used to store the specified event
CString CEventEditorDlg::GetPropName(uint32 nTrack, uint32 nMessage)
{
	CString rv;
	rv.Format("%sT%dE%d", m_sPropPrefix, nTrack, nMessage);
	return rv;
}

#define TOKEN_BUFF_SIZE		256

//initializes the dialog from the member object
bool CEventEditorDlg::InitFromPropList()
{
	uint32 nTokOffset = 0;
	char pszToken[TOKEN_BUFF_SIZE];

	//make sure that the property list has the appropriate string we can use
	//for storing/loading information from
	CStringProp* pEventStr = (CStringProp*)GetProperty("EventDialogInfo");

	if(!pEventStr)
		return false;
	
	//Read in the number of tracks first
	if(MyStrTok(pEventStr->m_String, nTokOffset, pszToken, TOKEN_BUFF_SIZE) == false)
		return false;
	m_nNumTracks = LTMAX(0, LTMIN(MAX_TRACKS, atoi(pszToken)));

	//Read in the number of events per track
	if(MyStrTok(pEventStr->m_String, nTokOffset, pszToken, TOKEN_BUFF_SIZE) == false)
		return false;
	m_nMaxEventsPerTrack = LTMAX(0, atoi(pszToken));

	//Read in the prefix of the properties
	if(MyStrTok(pEventStr->m_String, nTokOffset, pszToken, TOKEN_BUFF_SIZE) == false)
		return false;
	m_sPropPrefix = pszToken;

	//now read in the view parameters

	//setup the defaults in case they aren't specified
	m_nTimePos	= 0;
	m_nScale	= 10;
	m_nLength	= 60;

	//read in the time offset
	if(MyStrTok(pEventStr->m_String, nTokOffset, pszToken, TOKEN_BUFF_SIZE))
	{
		m_nTimePos = atoi(pszToken);
	}

	//read in the length
	if(MyStrTok(pEventStr->m_String, nTokOffset, pszToken, TOKEN_BUFF_SIZE))
	{
		m_nLength = LTMAX(1, LTMIN(MAX_EVENT_LENGTH, atoi(pszToken)));
	}

	//read in the scale
	if(MyStrTok(pEventStr->m_String, nTokOffset, pszToken, TOKEN_BUFF_SIZE))
	{
		m_nScale = LTMAX(1, LTMIN( LTMIN(MAX_EVENT_SCALE, m_nLength), atoi(pszToken)));
	}

	//make sure that we have all the properties in place, and also use this to recreate
	//the events
	uint32 nCurrTrack;
	for(nCurrTrack = 0; nCurrTrack < m_nNumTracks; nCurrTrack++)
	{
		for(uint32 nCurrMessage = 0; nCurrMessage < m_nMaxEventsPerTrack; nCurrMessage++)
		{
			CStringProp* pStrProp = (CStringProp*)GetProperty(GetPropName(nCurrTrack, nCurrMessage));

			if(pStrProp == NULL)
				return false;

			CString sStr = pStrProp->m_String;

			//we need to create a new event if there is a string of the proper formatting
			int nBarPos = sStr.Find('|');
			if(nBarPos != -1)
			{
				CString sTime = sStr.Left(nBarPos);
				CString sMsg  = sStr.Mid(nBarPos + 1);

				CObjectEvent* pNewEvent = new CObjectEvent;

				if(pNewEvent == NULL)
					return false;

				pNewEvent->m_nTrack			= nCurrTrack;
				pNewEvent->m_nTrackTime		= atoi(sTime);
				pNewEvent->m_bSelected		= false;
				pNewEvent->m_sText			= sMsg;
				pNewEvent->m_pNext			= m_pHeadEvent;
				m_pHeadEvent = pNewEvent;
			}
		}
	}

	//now create each of the tracks
	for(nCurrTrack = 0; nCurrTrack < m_nNumTracks; nCurrTrack++)
	{
		m_pLabels[nCurrTrack] = new CEdit;

		//setup the rectangle for it
		CRect rRect;
		rRect.left		= m_rTrackLabels.left;
		rRect.right		= m_rTrackLabels.right;
		rRect.top		= GetTrackTop(nCurrTrack);
		rRect.bottom	= rRect.top + TRACK_HEIGHT;

		m_pLabels[nCurrTrack]->Create(WS_CHILD | WS_VISIBLE | WS_BORDER | WS_TABSTOP | ES_AUTOHSCROLL | ES_WANTRETURN, rRect, this, BASE_LABEL_ID + nCurrTrack);
		m_pLabels[nCurrTrack]->SetFont(GetDefaultFont());
		m_pLabels[nCurrTrack]->SetLimitText(16);

		//read in the name of the track
		if(MyStrTok(pEventStr->m_String, nTokOffset, pszToken, TOKEN_BUFF_SIZE))
		{
			m_pLabels[nCurrTrack]->SetWindowText(pszToken);
		}
	}

	return true;
}

//saves the results of the dialog back into the member property list
void CEventEditorDlg::SavePropList()
{
	//get the document and setup an undo
	CRegionDoc* pDoc = GetActiveRegionDoc();
	if( pDoc && m_pPropList )
	{
		pDoc->SetupUndoForSelections( FALSE );
	}

	//first off, save out the string that holds the info about the dialog
	CString sDlgInfo;
	sDlgInfo.Format("%d %d \'%s\' %d %d %d ", 
					m_nNumTracks, m_nMaxEventsPerTrack, m_sPropPrefix,
					m_nTimePos, m_nLength, m_nScale);

	//append all of the track names
	uint32 nCurrTrack;
	for(nCurrTrack = 0; nCurrTrack < m_nNumTracks; nCurrTrack++)
	{
		CString sTrackName;
		GetDlgItem(BASE_LABEL_ID + nCurrTrack)->GetWindowText(sTrackName);

		//make sure that they don't have any characters that will mess up the string
		sTrackName.Replace('\"', ' ');
		sTrackName.Replace('\'', ' ');

		sDlgInfo += '\'' + sTrackName + "\' ";
	}

	//save that property
	CStringProp* pDlgInfo = (CStringProp*)GetProperty("EventDialogInfo");
	strcpy(pDlgInfo->m_String, sDlgInfo);
	m_pPropCtrls->NotifyChange(pDlgInfo, FALSE);

	//now we need to sort all the elements to make it just a bit cleaner and more
	//predictable
	SortEvents();

	//now save out each one
	for(nCurrTrack = 0; nCurrTrack < m_nNumTracks; nCurrTrack++)
	{

		CObjectEvent* pCurr = m_pHeadEvent;

		uint32 nCurrMessage = 0;
		while(pCurr)
		{
			if(pCurr->m_nTrack == nCurrTrack)
			{
				//found a message on this track, setup its string in the form
				// Time|Message
				CStringProp* pProp = (CStringProp*)GetProperty(GetPropName(nCurrTrack, nCurrMessage));
				sprintf(pProp->m_String, "%d|%s", pCurr->m_nTrackTime, pCurr->m_sText);
				m_pPropCtrls->NotifyChange(pProp, FALSE);

				nCurrMessage++;
			}
			pCurr = pCurr->m_pNext;
		}

		//now clear out all the other messages
		for(; nCurrMessage < m_nMaxEventsPerTrack; nCurrMessage++)
		{
			//clear out this string
			CStringProp* pProp = (CStringProp*)GetProperty(GetPropName(nCurrTrack, nCurrMessage));

			if(strlen(pProp->m_String) > 0)
			{
				pProp->m_String[0] = '\0';
				m_pPropCtrls->NotifyChange(pProp, FALSE);
			}
		}
	}
}

//Sorts the events in the order of track first, then time
void CEventEditorDlg::SortEvents()
{
	//just do a bubble sort. Not efficient, but should be fast enough...
	bool bDone = false;

	while(!bDone)
	{
		bDone = true;

		CObjectEvent* pCurr			= m_pHeadEvent;
		CObjectEvent** pPrevLink	= &m_pHeadEvent;

		while(pCurr)
		{
			CObjectEvent* pNext = pCurr->m_pNext;

			//see if we want to swap
			if(pNext && 
				((pNext->m_nTrack < pCurr->m_nTrack) ||
				 ((pNext->m_nTrack == pCurr->m_nTrack) &&
				  (pNext->m_nTrackTime < pCurr->m_nTrackTime))))
			{
				//we need to swap

				//outer links first
				*pPrevLink = pNext;
				pCurr->m_pNext = pNext->m_pNext;

				//inner links
				pNext->m_pNext = pCurr;

				//update our loop...
				pPrevLink = &pNext->m_pNext;

				//can't be done yet
				bDone = false;
			}
			else
			{
				//move along
				pPrevLink = &pCurr->m_pNext;
				pCurr = pNext;				
			}
		}
	}
}

//given the ID of a control, it will find it, remove it from the dialog, and return
//the rectangle that it occupied
CRect CEventEditorDlg::RemovePlaceholderCtrl(uint32 nID, bool bHide)
{
	//get the window first
	CWnd* pWnd = GetDlgItem(nID);

	//now get the rectangle
	CRect rv;
	pWnd->GetWindowRect(rv);

	//get the rectangle of the dialog
	CRect rDialog;
	GetWindowRect(rDialog);

	//now we need to remove the title bar part....a lot of work considering this is just
	//to get an offset in the dialog....stupid windows.
	CRect rDlgClient;
	GetClientRect(rDlgClient);

	//now we need to offset the holder rectangle by the dialog's upper left corner
	rv.OffsetRect(	-(rDialog.left + (rDialog.Width() - rDlgClient.Width()) - GetSystemMetrics(SM_CXDLGFRAME)),
					-(rDialog.top  + (rDialog.Height() - rDlgClient.Height()) - GetSystemMetrics(SM_CYDLGFRAME)));

	//now make it so this control is no longer visible
	if(bHide)
		pWnd->ModifyStyle(WS_VISIBLE, 0, 0);

	//all done
	return rv;
}


BOOL CEventEditorDlg::OnInitDialog()
{
	m_nActiveTrack				= INVALID_TRACK;
	m_pDragEvent				= NULL;
	m_bInDrag					= false;
	m_bSupressMessageChanges	= false;
	m_bSupressTimeChanges		= false;

	if(!CDialog::OnInitDialog())
		return FALSE;

	//get the areas for our controls
	m_rTrackLabels	= RemovePlaceholderCtrl(IDC_STATIC_TRACK_NAME_AREA);
	m_rRuler		= RemovePlaceholderCtrl(IDC_STATIC_RULER_AREA);
	m_rEventView	= RemovePlaceholderCtrl(IDC_STATIC_EVENT_VIEW_AREA, false);

	//initialize all of our tracks from the object given
	if(!InitFromPropList())
	{
		//trigger an error
		CString sError;
		sError.Format("The object did not have the proper fields or formatting for using this dialog. Please consult an engineer.");
		MessageBox(sError, "Error", MB_ICONEXCLAMATION | MB_OK);

		//failure
		return FALSE;
	}


	GetLengthSpinCtrl()->SetRange(1, MAX_EVENT_LENGTH);
	GetScaleSpinCtrl()->SetRange(1, MAX_EVENT_SCALE);

	//setup the text for the scale and length
	CString sText;
	sText.Format("%d", m_nLength);
	GetLengthCtrl()->SetWindowText(sText);
	sText.Format("%d", m_nScale);
	GetScaleCtrl()->SetWindowText(sText);

	SetTimeText(-1);
	UpdateMessageCounts();
	UpdateSelections();

	m_KeyHandler.SubclassWindow(GetDlgItem(IDC_STATIC_EVENT_VIEW_AREA)->m_hWnd);

	//set the limit on the text in the message box
	GetMessageCtrl()->SetLimitText(255);

	//setup the tooltips
	m_ToolTip.Create(this);
	m_ToolTip.AddWindowTool(GetDlgItem(IDC_BUTTON_LIST), IDS_TT_EVENT_LIST);
	m_ToolTip.AddWindowTool(GetDlgItem(IDC_SCROLLBAR_EVENT_TIME), IDS_TT_EVENT_TIME);
	m_ToolTip.AddWindowTool(GetDlgItem(IDC_STATIC_RULER_AREA), IDS_TT_EVENT_RULER);
	m_ToolTip.AddWindowTool(GetDlgItem(IDC_EDIT_EVENT_MESSAGE), IDS_TT_EVENT_MESSAGE);
	m_ToolTip.AddWindowTool(GetDlgItem(IDC_EDIT_EVENT_IMM_MESSAGE), IDS_TT_EVENT_IMM_MESSAGE);
	m_ToolTip.AddWindowTool(GetDlgItem(IDC_EDIT_EVENT_SCALE), IDS_TT_EVENT_SCALE);
	m_ToolTip.AddWindowTool(GetDlgItem(IDC_EDIT_EVENT_LENGTH), IDS_TT_EVENT_LENGTH);

	return TRUE;
}

void CEventEditorDlg::OnOK()
{
	SavePropList();
	CleanupDialog();

	CDialog::OnOK();
}

void CEventEditorDlg::OnCancel()
{
	CleanupDialog();

	CDialog::OnCancel();
}

//Gets the default font object of the dialog
CFont* CEventEditorDlg::GetDefaultFont()
{
	//just grab it from one of the static controls
	return GetDlgItem(IDC_STATIC_CURR_TIME)->GetFont();
}

//cleans up a dialog after either OK or cancel
void CEventEditorDlg::CleanupDialog()
{
	//free all of our labels
	for(uint32 nCurrLabel = 0; nCurrLabel < CEventEditorDlg::MAX_TRACKS; nCurrLabel++)
	{
		delete m_pLabels[nCurrLabel];
		m_pLabels[nCurrLabel] = NULL;
	}

	//delete the events
	delete m_pHeadEvent;
	m_pHeadEvent = NULL;
}

//gets the extents of the messages on a track
void CEventEditorDlg::GetTrackExtents(uint32 nTrack, uint32& nMin,  uint32& nMax)
{
	bool bHitExtent = false;

	nMin = 0;
	nMax = 0;

	CObjectEvent* pCurr = m_pHeadEvent;
	while(pCurr)
	{
		if(pCurr->m_nTrack == nTrack)
		{
			//grow the extents
			if(!bHitExtent)
			{
				nMin = pCurr->m_nTrackTime;
				nMax = pCurr->m_nTrackTime;
				bHitExtent = true;
			}
			else
			{
				nMin = LTMIN(nMin, pCurr->m_nTrackTime);
				nMax = LTMAX(nMax, pCurr->m_nTrackTime);
			}
		}

		pCurr = pCurr->m_pNext;
	}
}


//converts a time to a x coordinate on the screen
int32 CEventEditorDlg::TimeToScreen(uint32 nTime)
{
	//get the time offset
	int32 nTimeOffset = nTime - m_nTimePos;

	//find the ranges
	int32 nTimeRange = m_nScale * 1000;
	int32 nViewRange = m_rEventView.right - m_rEventView.left;

	//convert this to a scaled pixel coordinate
	return (int32)(m_rEventView.left + nTimeOffset * (float)nViewRange / (float)nTimeRange);
}

//converts an x coordinate on the scree to a time coordinate
int32 CEventEditorDlg::ScreenToTime(uint32 nScreen)
{
	//find the offset time
	int32 nScreenOffset = nScreen - m_rEventView.left;

	//now find the ranges
	int32 nTimeRange = m_nScale * 1000;
	int32 nViewRange = m_rEventView.right - m_rEventView.left;

	//convert this to a scaled pixel coordinate
	return (int32)(m_nTimePos + nScreenOffset * (float)nTimeRange / (float)nViewRange);
}


void CEventEditorDlg::RenderTrack(uint32 nTrack, CDC& dc)
{
	//get the top and bottom of this track
	uint32 nTop		= GetTrackTop(nTrack);
	uint32 nBottom	= nTop + TRACK_HEIGHT;
	uint32 nLeft	= m_rEventView.left + 1;
	uint32 nRight	= m_rEventView.right - 1;

	//setup the clipping rectangle
	CRgn ClipRegion;
	ClipRegion.CreateRectRgn(nLeft, nTop, nRight, nBottom);
	dc.SelectClipRgn(&ClipRegion);

	//now we need to draw the extent bar
	uint32 nExtentMin;
	uint32 nExtentMax;
	GetTrackExtents(nTrack, nExtentMin, nExtentMax);

	//convert the extent rectangle to pixels and draw it
	CBrush BGExtent(RGB(0, 0, 0));
	dc.FillRect(CRect(	TimeToScreen(nExtentMin), nTop + EXTENT_VERTICAL_PADDING, 
						TimeToScreen(nExtentMax), nBottom - EXTENT_VERTICAL_PADDING), &BGExtent);

	//now we need to draw all the actual message objects

	//setup the fill for the objects
	CBrush BGEvent(RGB(0, 0, 255));
	CBrush BGSelEvent(RGB(0, 255, 0));


	CObjectEvent* pCurr = m_pHeadEvent;
	while(pCurr)
	{
		if(pCurr->m_nTrack == nTrack)
		{
			CBrush *pOldFill = dc.SelectObject((pCurr->m_bSelected) ? &BGSelEvent : &BGEvent);

			//we need to render this message
			int32 nXPos = TimeToScreen(pCurr->m_nTrackTime);
			int32 nYPos = (nTop + nBottom) / 2;

			dc.Ellipse(	nXPos - VIEW_EVENT_RADIUS, nYPos - VIEW_EVENT_RADIUS, 
						nXPos + VIEW_EVENT_RADIUS, nYPos + VIEW_EVENT_RADIUS);

			dc.SelectObject(pOldFill);
		}

		pCurr = pCurr->m_pNext;
	}
}

//renders the actual object view
void CEventEditorDlg::RenderEventView(CDC& dc)
{
	//fill the region with the background color, and selection bar
	CBrush BGFill(RGB(255, 255, 255));

	CRect rFill = m_rEventView;
	rFill.DeflateRect(1, 1, 1, 1);

	//see if this is the active track
	if(m_nActiveTrack != INVALID_TRACK)
	{
		uint32 nTop		= GetTrackTop(m_nActiveTrack);
		uint32 nBottom	= nTop + TRACK_HEIGHT;
		uint32 nLeft	= m_rEventView.left + 1;
		uint32 nRight	= m_rEventView.right - 1;

		//it is, so we need to render the bar for the background
		CBrush BGHighlight(RGB(255, 0, 0));
		dc.FillRect(CRect(nLeft, nTop, nRight, nBottom), &BGHighlight);

		//now fill in the white parts
		dc.FillRect(CRect(rFill.left, rFill.top, rFill.right, nTop - 1), &BGFill);
		dc.FillRect(CRect(rFill.left, nBottom, rFill.right, rFill.bottom), &BGFill);

		//draw the top and bottom lines to make it look framed
		dc.MoveTo(nLeft, nTop);
		dc.LineTo(nRight, nTop);
		dc.MoveTo(nLeft, nBottom - 1);
		dc.LineTo(nRight, nBottom - 1);
	}
	else
	{
		//no selection, just fill the entire thing
		dc.FillRect(rFill, &BGFill);
	}

	//run through and draw all tracks
	for(uint32 nCurrTrack = 0; nCurrTrack < m_nNumTracks; nCurrTrack++)
	{
		RenderTrack(nCurrTrack, dc);
	}
}

//renders the ruler beneath the track view
void CEventEditorDlg::RenderRuler(CDC& dc)
{
	//find out the amount of seconds that are displayed (these are in MS)
	uint32 nStart		= m_nTimePos;
	uint32 nRange		= m_nScale * 1000;
	uint32 nEnd			= nStart + nRange;

	//setup the clipping rectangle
	CRgn ClipRegion;
	ClipRegion.CreateRectRgn(m_rRuler.left, m_rRuler.top, m_rRuler.right, m_rRuler.bottom);
	dc.SelectClipRgn(&ClipRegion);


	//now find out how many tick marks per second we want to display per second span
	uint32 nNumTicks	= 10 - LTMIN(9, (nRange / 2000));

	//setup the DC
	dc.SetBkMode(TRANSPARENT);
	CFont* pOldFont = dc.SelectObject(GetDefaultFont());

	//increment and end
	float fInc = 1000.0f / (float)nNumTicks;
	float fEnd = (float)nEnd;

	//we also need to consider very large ranges, so need to consider that as well
	uint32 nSkipTicks = 1;

	if(nNumTicks == 1)
	{
		//make sure that there are at least the specified number of pixels between
		float fTicksPerArea = RULER_TICK_PADDING * (float)m_nScale / (float)m_rRuler.Width();

		//now convert that to an integer value
		nSkipTicks = LTMAX(1, (uint32)fTicksPerArea);
	}

	//adjust our values
	fInc *= nSkipTicks;		

	//snap this to the prior second
	int32 nOffsetStart = nStart - (nStart % 1000);

	//the extent of the previous label string plus padding
	int32 nPrevLabelEnd = 0;

	//draw all the ticks
	uint32 nTickIndex = 0;
	for(float fCurr = (float)nOffsetStart; fCurr < fEnd; fCurr += fInc, nTickIndex += nSkipTicks)
	{
		uint32 nHeight;

		int32 nTicXPos = TimeToScreen((uint32)fCurr);

		if((nTickIndex % nNumTicks) == 0)
		{
			//this is a second, it should be taller
			nHeight = m_rRuler.Height() / 2;

			//we should also label the second beneath it
			CString sText;
			sText.Format("%d", (nOffsetStart / 1000) + (nTickIndex / nNumTicks)); 

			//find the width of this text
			CSize TextSize = dc.GetTextExtent(sText);

			//find the X position for the centered on tick string
			int32 nXPos = nTicXPos - TextSize.cx / 2;

			//if it is beyond the previous label, we should display it
			if(nXPos >= nPrevLabelEnd)
			{
				//now draw that text
				dc.TextOut(nXPos, m_rRuler.top + nHeight, sText);

				//update the end label position
				nPrevLabelEnd = nXPos + TextSize.cx + RULER_LABEL_PADDING;
			}
		}
		else
		{
			//not a second
			nHeight = m_rRuler.Height() / 4;
		}

		//now draw the tick
		dc.MoveTo(nTicXPos, m_rRuler.top);
		dc.LineTo(nTicXPos, m_rRuler.top + nHeight);
	}	

	//restore changed properties in the DC
	dc.SelectObject(pOldFont);
}

//updates the scrollbar with the appropriate info
void CEventEditorDlg::UpdateScrollBar()
{
	SCROLLINFO si;
	si.cbSize	= sizeof(si);
	si.fMask	= SIF_PAGE | SIF_RANGE | SIF_POS;
	si.nMin		= 0;
	si.nMax		= m_nLength * SCROLL_BAR_RES;
	si.nPage	= m_nScale * SCROLL_BAR_RES;

	//adjust the position if it is going off of the edge
	if(MS_TO_SB(m_nTimePos) + si.nPage > si.nMax)
	{
		m_nTimePos = SB_TO_MS(si.nMax - si.nPage);
	}

	si.nPos = MS_TO_SB(m_nTimePos);

	//install the actual info
	GetTimeScroll()->SetScrollInfo(&si);
}

//renders the main controls when something has changed. This should not be called for a repaint of the screen
void CEventEditorDlg::RedrawView(bool bRedrawRuler)
{
	CDC* pDC = GetDC();
	RenderEventView(*pDC);
	ReleaseDC(pDC);

	if(bRedrawRuler)
		InvalidateRect(m_rRuler);
}

//gets the event that is beneath the specified point. NULL if no event is beneath it
CObjectEvent* CEventEditorDlg::GetEventAt(CPoint Pt)
{
	//first do a bounding check on the box
	if(!m_rEventView.PtInRect(Pt))
		return NULL;

	CObjectEvent* pRV = NULL;

	//now run through each event and see if it intersects
	CObjectEvent* pCurr = m_pHeadEvent;
	while(pCurr)
	{
		//figure out the position
		uint32 nX = TimeToScreen(pCurr->m_nTrackTime);
		uint32 nY = GetTrackTop(pCurr->m_nTrack) + TRACK_HEIGHT / 2;

		//figure out the distance
		if((Pt.x - nX) * (Pt.x - nX) + (Pt.y - nY) * (Pt.y - nY) < (VIEW_EVENT_RADIUS * VIEW_EVENT_RADIUS))
		{
			//the point intersects the sphere.
			pRV = pCurr;

			//don't break though, since the top most item will be the last one, and that should therefore
			//be picked
		}
		pCurr = pCurr->m_pNext;
	}

	return pRV;
}

//clears the selected status from all events
void CEventEditorDlg::DeselectAll()
{
	CObjectEvent* pCurr = m_pHeadEvent;
	while(pCurr)
	{
		if(pCurr->m_bSelected)
		{
			pCurr->m_bSelected = false;
			RedrawEvent(pCurr);
		}
		pCurr = pCurr->m_pNext;
	}
	UpdateSelections();
}

//redraws the specified event
void CEventEditorDlg::RedrawEvent(CObjectEvent* pEvent)
{
	assert(pEvent);

	//get the position
	uint32 nX = TimeToScreen(pEvent->m_nTrackTime);
	uint32 nY = GetTrackTop(pEvent->m_nTrack) + TRACK_HEIGHT / 2;

	//now invalidate the rectangle
	InvalidateRect(CRect(	nX - VIEW_EVENT_RADIUS, nY - VIEW_EVENT_RADIUS, 
							nX + VIEW_EVENT_RADIUS, nY + VIEW_EVENT_RADIUS), FALSE);
}

//call this to update the UI to reflect any selection changes
void CEventEditorDlg::UpdateSelections()
{
	UpdateMessageText();
	UpdateTimeText();
}

//handles updating the text inside of the message box, should be called when a selection changes
void CEventEditorDlg::UpdateMessageText()
{
	//first off, count up the nodes, and get a string from the selected
	uint32 nNumSel = 0;
		
	//figure out the text we are going to put in there
	CString sText;
	bool	bFoundFirst = false;

	CObjectEvent* pCurr = m_pHeadEvent;
	while(pCurr)
	{
		if(pCurr->m_bSelected)
		{
			nNumSel++;
			
			if(!bFoundFirst)
			{
				//found the first string, just use this
				sText		= pCurr->m_sText;
				bFoundFirst = true;
			}
			else
			{
				//this is not the first string, we need to keep as much as is the
				//same
				if(pCurr->m_sText != sText)
					sText = "";
			}
		}
		pCurr = pCurr->m_pNext;
	}

	CEdit* pMessage = GetMessageCtrl();

	//handle graying out of window
	pMessage->EnableWindow(nNumSel > 0);


	//set the message text, not allowing it to change the selected objects
	m_bSupressMessageChanges = true;
	pMessage->SetWindowText(sText);
	m_bSupressMessageChanges = false;
}

//handles updating the text inside of the time box, should be called when a selection changes
void CEventEditorDlg::UpdateTimeText()
{
	//first off, count up the nodes, and get a string from the selected
	uint32 nNumSel = 0;
		
	//figure out the text we are going to put in there
	uint32  nTime = 0xFFFFFFFF;
	bool	bFoundFirst = false;

	CObjectEvent* pCurr = m_pHeadEvent;
	while(pCurr)
	{
		if(pCurr->m_bSelected)
		{
			nNumSel++;
			
			if(!bFoundFirst)
			{
				//found the first string, just use this
				nTime		= pCurr->m_nTrackTime;
				bFoundFirst = true;
			}
			else
			{
				//this is not the first string, we need to keep as much as is the
				//same
				if(pCurr->m_nTrackTime != nTime)
					nTime = 0xFFFFFFFF;
			}
		}
		pCurr = pCurr->m_pNext;
	}

	CEdit* pTime = GetTimeCtrl();

	//handle graying out of window
	pTime->EnableWindow(nNumSel > 0);

	//now format the string
	CString sText;

	if(nTime != 0xFFFFFFFF)
		sText.Format("%d", nTime);

	//set the message text, not allowing it to change the selected objects
	m_bSupressTimeChanges = true;
	pTime->SetWindowText(sText);
	m_bSupressTimeChanges = false;
}

//gets the number of events on a specified track
uint32 CEventEditorDlg::GetNumEventsOnTrack(uint32 nTrack)
{
	uint32 nRV = 0;

	CObjectEvent* pCurr = m_pHeadEvent;
	while(pCurr)
	{
		if(pCurr->m_nTrack == nTrack)
			nRV++;
		
		pCurr = pCurr->m_pNext;
	}

	return nRV;
}

//gets a property from the held list, returns NULL if not found or it is of a different
//type
CBaseProp* CEventEditorDlg::GetProperty(const char* pszPropName, uint32 nType)
{
	CBaseProp* pProp = m_pPropList->GetProp(pszPropName);

	if(pProp && (pProp->GetType() == nType))
		return pProp;

	return NULL;
}

//given a node, it will move it to the end of the event list
void CEventEditorDlg::BringToFront(CObjectEvent* pEvent)
{
	assert(pEvent);

	CObjectEvent** pPrevLink = &m_pHeadEvent;
	CObjectEvent*  pCurr = m_pHeadEvent;

	bool bFound = false;

	while(pCurr)
	{
		//see if this is the object
		if(pCurr == pEvent)
		{
			//found the match, remove it from the list
			*pPrevLink = pCurr->m_pNext;
			bFound = true;
		}
		else
		{
			pPrevLink = &pCurr->m_pNext;
		}
		pCurr = pCurr->m_pNext;
	}

	//at the end of the list, so now we need to attach it to the end
	if(bFound)
	{
		*pPrevLink = pEvent;
		pEvent->m_pNext = NULL;
	}
}

//will nudge all selected nodes the specified number of MS
void CEventEditorDlg::NudgeSelection(int32 nMS)
{
	CObjectEvent*  pCurr = m_pHeadEvent;
	while(pCurr)
	{
		if(pCurr->m_bSelected)
		{
			pCurr->m_nTrackTime = LTMAX(0, LTMIN((int32)S_TO_MS(m_nLength), (int32)pCurr->m_nTrackTime + nMS));
		}
		pCurr = pCurr->m_pNext;
	}

	UpdateTimeText();
	RedrawView(false);
}

//deletes all the currently selected nodes
void CEventEditorDlg::DeleteSelectedEvents()
{
	CObjectEvent*  pCurr = m_pHeadEvent;
	CObjectEvent** pPrevLink = &m_pHeadEvent;
	while(pCurr)
	{
		//cache this in case this link is deleted
		CObjectEvent* pNext = pCurr->m_pNext;
		if(pCurr->m_bSelected)
		{
			//patch up the links
			*pPrevLink = pCurr->m_pNext;
			//remove the link
			pCurr->m_pNext = NULL;
			delete pCurr;
		}
		else
		{
			//save the previous link for patching up
			pPrevLink = &pCurr->m_pNext;
		}
		pCurr = pNext;
	}
	RedrawView(false);
	UpdateSelections();
}

//deletes the specified node
void CEventEditorDlg::DeleteEvent(CObjectEvent* pEvent)
{
	CObjectEvent*  pCurr = m_pHeadEvent;
	CObjectEvent** pPrevLink = &m_pHeadEvent;
	while(pCurr)
	{
		if(pCurr == pEvent)
		{
			//patch up the links
			*pPrevLink = pCurr->m_pNext;
			//remove the link
			pCurr->m_pNext = NULL;
			delete pCurr;

			RedrawView(false);
			UpdateSelections();

			return;
		}
		//save the previous link for patching up
		pPrevLink = &pCurr->m_pNext;
		pCurr = pCurr->m_pNext;
	}
}

//deletes all events on the specified track
void CEventEditorDlg::DeleteTrack(uint32 nTrack)
{
	CObjectEvent*  pCurr = m_pHeadEvent;
	CObjectEvent** pPrevLink = &m_pHeadEvent;
	while(pCurr)
	{
		//cache this in case this link is deleted
		CObjectEvent* pNext = pCurr->m_pNext;
		if(pCurr->m_nTrack == nTrack)
		{
			//patch up the links
			*pPrevLink = pCurr->m_pNext;
			//remove the link
			pCurr->m_pNext = NULL;
			delete pCurr;
		}
		else
		{
			//save the previous link for patching up
			pPrevLink = &pCurr->m_pNext;
		}
		pCurr = pNext;
	}
	RedrawView(false);
	UpdateSelections();
}

//adds an event at the current mouse position
void CEventEditorDlg::AddEvent(const char* pszText)
{
	//bail if we aren't on a valid track
	if(m_nActiveTrack == INVALID_TRACK)
	{
		MessageBeep(MB_ICONEXCLAMATION);
		return;
	}

	//make sure that the current track has room for another event
	uint32 nNumEvents = GetNumEventsOnTrack(m_nActiveTrack);

	//bail if not enough room
	if(nNumEvents >= m_nMaxEventsPerTrack)
	{
		MessageBeep(MB_ICONEXCLAMATION);
		return;
	}

	//allocate the node
	CObjectEvent* pNewEvent = new CObjectEvent;

	if(pNewEvent == NULL)
		return;

	//clear out the selected nodes
	DeselectAll();

	//setup the event
	pNewEvent->m_nTrack			= m_nActiveTrack;
	pNewEvent->m_nTrackTime		= ScreenToTime(m_ptLastMouse.x);
	pNewEvent->m_pNext			= m_pHeadEvent;
	pNewEvent->m_bSelected		= true;
	pNewEvent->m_sText			= pszText;
	
	m_pHeadEvent = pNewEvent;

	//bring this to the front...
	BringToFront(pNewEvent);

	//redraw the control
	RedrawView(false);

	UpdateSelections();
}

//----------------------------------------------------------------------------------------------------------
// Message handlers
//----------------------------------------------------------------------------------------------------------
void CEventEditorDlg::OnPaint() 
{
	CPaintDC dc(this);

	RenderEventView(dc);
	RenderRuler(dc);
}

void CEventEditorDlg::OnMouseMove(UINT nFlags, CPoint Pt)
{
	//let the base class handle it
	CDialog::OnMouseMove(nFlags, Pt);

	//save this position
	m_ptLastMouse = Pt;

	//determine if the point is within the event view area
	bool bInRect = m_rEventView.PtInRect(Pt);

	//handle dragging the node
	if(m_pDragEvent)
	{
		//see if we are actually in a drag mode
		if(m_bInDrag)
		{
			//we need to update the position of the event
			if(bInRect)
			{
				//save the original screen pos
				int32 nStartPos = TimeToScreen(m_pDragEvent->m_nTrackTime);
				m_pDragEvent->m_nTrackTime = ScreenToTime(Pt.x);

				if(	(m_nActiveTrack != INVALID_TRACK) && 
					(m_nActiveTrack != m_pDragEvent->m_nTrack) &&
					(GetNumEventsOnTrack(m_nActiveTrack) < m_nMaxEventsPerTrack))
				{
					//switch tracks
					m_pDragEvent->m_nTrack = m_nActiveTrack;

					//update the counts
					UpdateMessageCounts();

					//redraw the whole thing
					RedrawView(false);
				}
				else
				{
					//we can just update the rectangle that we changed
					uint32 nLeft  = LTMIN(nStartPos, Pt.x) - VIEW_EVENT_RADIUS - 1;
					uint32 nRight = LTMAX(nStartPos, Pt.x) + VIEW_EVENT_RADIUS + 1;
					uint32 nTop	  = GetTrackTop(m_pDragEvent->m_nTrack);
					InvalidateRect(CRect(nLeft, nTop, nRight, nTop + TRACK_HEIGHT), FALSE);				
				}

				//update the time display
				UpdateTimeText();
			}
		}
		else
		{
			//not in drag mode yet, lets see if the new position is far enough away
			//from the initial position to instantiate drag mode
			if( (Pt.x - m_ptDragStart.x) * (Pt.x - m_ptDragStart.x) + 
				(Pt.y - m_ptDragStart.y) * (Pt.y - m_ptDragStart.y) > MOUSE_DRAG_DEADZONE * MOUSE_DRAG_DEADZONE)
			{
				//we can enter a full drag
				m_bInDrag = true;
			}
		}
	}

	//see if it is within the rectangle of the event view
	uint32 nNewActiveTrack = INVALID_TRACK;

	//the text in the immediate message box
	CString sImmMsg = "";

	if(bInRect)
	{
		//update the time text
		SetTimeText(ScreenToTime(Pt.x));

		//default to the first track
		nNewActiveTrack = 0;

		//the bottom of the previous track
		uint32 nPrevTrackBottom = GetTrackTop(0) + TRACK_HEIGHT;

		//run through and see if it is within any tracks
		for(uint32 nCurrTrack = 1; nCurrTrack < m_nNumTracks; nCurrTrack++)
		{
			//find the threshold between the previous and current track
			uint32 nTop = GetTrackTop(nCurrTrack);
			uint32 nThresh = (nPrevTrackBottom + nTop) / 2;

			//determine if the point is above the the threshold, if it is, we are done
			if(Pt.y < nThresh)
				break;

			//otherwise, we need to move along
			nNewActiveTrack++;
			nPrevTrackBottom = nTop + TRACK_HEIGHT;
		}

		//grab the focus away from other controls
		GetDlgItem(IDC_STATIC_EVENT_VIEW_AREA)->SetFocus();

		//update the immediate message if we are over an event
		CObjectEvent* pOver = GetEventAt(Pt);
		if(pOver)
			sImmMsg = pOver->m_sText;
	}
	else
	{
		//disable the time position
		SetTimeText(-1);
	}

	//now handle redrawing if appropriate
	if(nNewActiveTrack != m_nActiveTrack)
	{
		//now invalidate the rectangle that contains the tracks
		uint32 nTop		= GetTrackTop((m_nActiveTrack == INVALID_TRACK) ? nNewActiveTrack : m_nActiveTrack);
		uint32 nBottom	= GetTrackTop((nNewActiveTrack == INVALID_TRACK) ? m_nActiveTrack : nNewActiveTrack);

		//swap the top and bottom if appropriate
		if(nBottom < nTop)
		{
			uint32 nTemp = nTop;
			nTop		 = nBottom;
			nBottom		 = nTemp;
		}

		//offset the bottom appropriately
		nBottom += TRACK_HEIGHT;

		//update our active track
		m_nActiveTrack = nNewActiveTrack;

		//dirty the appropriate portion of the window
		InvalidateRect(CRect(m_rEventView.left, nTop, m_rEventView.right, nBottom), FALSE);

		//handle updating the messages with respect to the new active track
		UpdateMessageCounts();
	}

	//setup the text in the immediate message box, avoiding duplicate setting to 
	//prevent flicker
	CString sOldImmMsg;
	GetImmMessageCtrl()->GetWindowText(sOldImmMsg);

	if(sOldImmMsg != sImmMsg)
		GetImmMessageCtrl()->SetWindowText(sImmMsg);

}

void CEventEditorDlg::OnScaleChanged()
{
	//get the string from the edit control
	CEdit* pEdit = GetScaleCtrl();

	CString sText;
	pEdit->GetWindowText(sText);

	int32 nScale = atoi(sText);

	//clamp it to the appropriate range
	int32 nClamped = LTMAX(1, LTMIN( LTMIN(m_nLength, MAX_EVENT_SCALE), nScale));

	if(nClamped != nScale)
	{
		sText.Format("%d", nClamped);
		pEdit->SetWindowText(sText);
	}

	//save this value
	m_nScale = nClamped;

	UpdateScrollBar();

	//invalidate the ruler and view
	RedrawView();
}

void CEventEditorDlg::OnLengthChanged()
{
	//get the string from the edit control
	CEdit* pEdit = GetLengthCtrl();

	CString sText;
	pEdit->GetWindowText(sText);

	int32 nLength = atoi(sText);

	//find the largest time length, since that will be the minimum
	uint32 nMaxTime = 0;
	CObjectEvent* pCurr = m_pHeadEvent;
	while(pCurr)
	{
		nMaxTime = LTMAX(nMaxTime, pCurr->m_nTrackTime);
		pCurr = pCurr->m_pNext;
	}

	//now convert the track time to seconds
	nMaxTime = (nMaxTime + 999) / 1000;

	//clamp it to the appropriate range
	int32 nClamped = LTMAX(nMaxTime, LTMIN(MAX_EVENT_LENGTH, nLength));

	if(nClamped != nLength)
	{
		sText.Format("%d", nClamped);
		pEdit->SetWindowText(sText);
	}

	//save this value
	m_nLength = nClamped;

	//we need to make sure that the scale does not exceed this maximum length
	if(m_nScale > m_nLength)
	{
		CString sVal;
		sVal.Format("%d", m_nLength);
		GetScaleCtrl()->SetWindowText(sVal);
	}

	UpdateScrollBar();
}

void CEventEditorDlg::OnHScroll(UINT nSBCode, UINT nThumbPos, CScrollBar* pScrollBar)
{
	//get the scroll bar
	CScrollBar* pSB = GetTimeScroll();

	//check out the code, and update the position accordingly
	int32 nPos = MS_TO_SB(m_nTimePos);

	switch(nSBCode)
	{
	case SB_LINELEFT:	nPos -= SCROLL_LINE_AMOUNT;						break;
	case SB_LINERIGHT:	nPos += SCROLL_LINE_AMOUNT;						break;
	case SB_PAGELEFT:	nPos -= m_nScale * SCROLL_BAR_RES;				break;
	case SB_PAGERIGHT:	nPos += m_nScale * SCROLL_BAR_RES;				break;
	case SB_LEFT:		nPos = 0;										break;
	case SB_RIGHT:		nPos = (m_nLength - m_nScale) * SCROLL_BAR_RES;	break;
	case SB_THUMBTRACK:	nPos = nThumbPos;								break;
	//case SB_ENDSCROLL:	nPos = nThumbPos;								break;
	default:
		break;
	}

	//bind it to range
	nPos = LTMAX(0, LTMIN((int32)(m_nLength - m_nScale) * SCROLL_BAR_RES, nPos));

	//update the scrollbar position to accurately reflect the new position
	pSB->SetScrollPos(nPos);

	//bail if it didn't change
	if(nPos == m_nTimePos)
		return;

	//now we can save it and rerender the appropriate windows
	m_nTimePos = SB_TO_MS(nPos);

	RedrawView();
}

void CEventEditorDlg::OnLButtonDown(UINT nFlags, CPoint Pt)
{
	//see if this is over an event
	CObjectEvent* pEvent = GetEventAt(Pt);

	if(pEvent)
	{
		if(nFlags & MK_SHIFT)
		{
			//shift, so toggle the selected status
			pEvent->m_bSelected = !pEvent->m_bSelected;
		}
		else
		{
			//no shift, so select only that node
			DeselectAll();
			pEvent->m_bSelected = true;

			//mark this event up for possible dragging
			m_pDragEvent	= pEvent;
			m_ptDragStart	= Pt;

			//if the user is holding control, lets just start out in drag
			if(nFlags & MK_CONTROL)
				m_bInDrag = true;
		}

		UpdateSelections();
		BringToFront(pEvent);
		RedrawEvent(pEvent);

		//make sure we get that mouse up notification
		SetCapture();
	}
}

void CEventEditorDlg::OnLButtonUp(UINT nFlags, CPoint Pt)
{
	//stop any drag in progress
	m_pDragEvent	= NULL;
	m_bInDrag		= false;

	ReleaseCapture();
}

#define BOOLTOENABLED(x)	((x) ? MF_ENABLED : MF_GRAYED)

void CEventEditorDlg::OnContextMenu(CWnd* pWnd, CPoint Pt)
{
	POINT ClientPt = Pt;
	ScreenToClient(&ClientPt);

	//only bother if it is in our rectangle
	if(!m_rEventView.PtInRect(ClientPt))
		return;

	//save this point
	m_ptLastMouse = ClientPt;

	//load the main menu
	CMenu Menu;
	Menu.LoadMenu(CG_IDR_POPUP_EVENTEDITOR);

	//grab the submenu
	CMenu* pPopup = Menu.GetSubMenu(0);
	assert(pPopup);

	bool bOnNode	= (GetEventAt(ClientPt) != NULL);
	bool bOnTrack	= m_nActiveTrack != INVALID_TRACK;

	uint32 nNumOnTrack = GetNumEventsOnTrack(m_nActiveTrack);

	//enable the appropriate items
	pPopup->EnableMenuItem(ID_EVENTEDITOR_DELETEALL,	BOOLTOENABLED(m_pHeadEvent != NULL));
	pPopup->EnableMenuItem(ID_EVENTEDITOR_DELETE,		BOOLTOENABLED(bOnNode));
	pPopup->EnableMenuItem(ID_EVENTEDITOR_DELETETRACK,	BOOLTOENABLED(nNumOnTrack != 0));
	pPopup->EnableMenuItem(ID_EVENTEDITOR_ADD,			BOOLTOENABLED(bOnTrack && (nNumOnTrack < m_nMaxEventsPerTrack)));
	pPopup->EnableMenuItem(ID_EVENTEDITOR_CUT,			BOOLTOENABLED(bOnNode));
	pPopup->EnableMenuItem(ID_EVENTEDITOR_COPY,			BOOLTOENABLED(bOnNode));
	pPopup->EnableMenuItem(ID_EVENTEDITOR_PASTE,		BOOLTOENABLED(m_bValidClipBoard));	

	//launcht the popup menu
	pPopup->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, Pt.x, Pt.y,	pWnd);			
}

BOOL CEventEditorDlg::OnMouseWheel(UINT nFlags, short zDelta, CPoint Pt)
{
	OnHScroll(SB_THUMBTRACK, MS_TO_SB(m_nTimePos) - zDelta * MOUSE_WHEEL_SCALE / WHEEL_DELTA, GetTimeScroll());
	
	return TRUE;		
}

void CEventEditorDlg::OnMessageChanged()
{
	//make sure we are allowed to proces these changes
	if(!m_bSupressMessageChanges)
	{
		//get the text from the edit control
		CString sText;
		GetMessageCtrl()->GetWindowText(sText);

		//now apply it to all selected events
		CObjectEvent*  pCurr = m_pHeadEvent;
		while(pCurr)
		{
			if(pCurr->m_bSelected)
			{
				pCurr->m_sText = sText;
			}
			pCurr = pCurr->m_pNext;
		}
	}		
}

void CEventEditorDlg::OnTimeChanged()
{
	//make sure we are allowed to proces these changes
	if(!m_bSupressTimeChanges)
	{
		//get the text from the edit control
		CString sText;
		GetTimeCtrl()->GetWindowText(sText);

		//convert it to a number
		uint32 nTime = atoi(sText);

		//clamp it to the range
		nTime = LTCLAMP(nTime, 0, S_TO_MS(m_nLength));

		//now apply it to all selected events
		CObjectEvent*  pCurr = m_pHeadEvent;
		while(pCurr)
		{
			if(pCurr->m_bSelected)
			{
				pCurr->m_nTrackTime = nTime;
			}
			pCurr = pCurr->m_pNext;
		}
		
		RedrawView(false);
	}		
}

void CEventEditorDlg::OnButtonList()
{
	//make it appear a bit more orderly
	SortEvents();

	CEventListDlg Dlg(m_pHeadEvent, this);

	//copy all of the track names into the dialog
	for(uint32 nCurrTrack = 0; nCurrTrack < m_nNumTracks; nCurrTrack++)
	{
		if(nCurrTrack < CEventListDlg::MAX_TRACKS)
		{
			GetDlgItem(BASE_LABEL_ID + nCurrTrack)->GetWindowText(Dlg.m_sTrackNames[nCurrTrack]);
		}
	}

	Dlg.DoModal();

	//just in case they changed the text of a selected node...
	UpdateSelections();
}

void CEventEditorDlg::OnButtonEditMessage()
{
	//create the string dialog
	CMultiLineStringDlg TextDlg;

	//get the string from the edit box
	CString sMessage;
	GetMessageCtrl()->GetWindowText(sMessage);

	TextDlg.m_Caption	= "Enter Message Text";
	TextDlg.m_String	= sMessage;

	if( TextDlg.DoModal() == IDOK )
	{
		GetMessageCtrl()->SetWindowText( TextDlg.m_String );
	}
}

//-------------------------------------------------------------------------------------------------
// Popup menu handlers
//-------------------------------------------------------------------------------------------------

void CEventEditorDlg::OnPopupDeleteAll()
{
	delete m_pHeadEvent;
	m_pHeadEvent = NULL;
	RedrawView(false);
	UpdateSelections();
}

void CEventEditorDlg::OnPopupDelete()
{
	//get the node
	CObjectEvent* pEvent = GetEventAt(m_ptLastMouse);
	assert(pEvent);
	DeleteEvent(pEvent);
}

void CEventEditorDlg::OnPopupDeleteTrack()
{
	DeleteTrack(m_nActiveTrack);
}

void CEventEditorDlg::OnPopupAdd()
{
	AddEvent();
}

void CEventEditorDlg::OnPopupCut()
{
	//get the node
	CObjectEvent* pEvent = GetEventAt(m_ptLastMouse);
	assert(pEvent);

	m_bValidClipBoard	= true;
	m_sClipBoard		= pEvent->m_sText;

	DeleteEvent(pEvent);
}

void CEventEditorDlg::OnPopupCopy()
{
	//get the node
	CObjectEvent* pEvent = GetEventAt(m_ptLastMouse);
	assert(pEvent);

	m_bValidClipBoard	= true;
	m_sClipBoard		= pEvent->m_sText;
}

void CEventEditorDlg::OnPopupPaste()
{
	if(m_bValidClipBoard)
	{
		AddEvent(m_sClipBoard);
	}
}


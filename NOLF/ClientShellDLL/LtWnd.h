/****************************************************************************
;
;	 MODULE:		LTWnd (.h)
;
;	PURPOSE:		LithTech Window class
;
;	HISTORY:		11/30/98 [kml] This file was created
;
;	COMMENT:		Copyright (c) 1998, Monolith Productions, Inc.
;
****************************************************************************/

#ifndef _LTWND_H_
#define _LTWND_H_

// Includes
#include "ltbasedefs.h"
#include "iclientshell.h"
#include "clientheaders.h"


// Defines
#define LTWF_NORMAL				0
#define LTWF_VDRAG				1
#define LTWF_HDRAG				2
#define LTWF_DRAGGABLE			(LTWF_VDRAG | LTWF_HDRAG)
#define LTWF_TRANSPARENT		4
#define LTWF_SIZEABLE			8
#define LTWF_FIXEDCHILD			16
#define LTWF_PARENTNOTIFY		32
#define LTWF_DISABLED			64
#define LTWF_TOPMOST			128
#define LTWF_PARENTDRAG			256
#define LTWF_NOCLOSE			512
#define LTWF_MANUALDRAW			1024
#define LTWF_NOUPDATE			2048
#define LTWF_TERM				4096

// States
#define LTWS_NORMAL			0
#define LTWS_CLOSED			1
#define LTWS_MINIMIZED		2

#define DLG_TOPLEFTFRAME		0
#define DLG_TOPFRAME			1
#define DLG_TOPRIGHTFRAME		2
#define DLG_RIGHTFRAME			3
#define DLG_BOTTOMRIGHTFRAME	4
#define DLG_BOTTOMFRAME			5
#define DLG_BOTTOMLEFTFRAME		6
#define DLG_LEFTFRAME			7


extern HLTCOLOR g_hColorTransparent;
class CLTWnd;

typedef CArray<HSURFACE,HSURFACE> CLTSurfaceArray;

class LTWNDCREATESTRUCT
{
public:
    LTWNDCREATESTRUCT();
	int nControlID;
	char *szWndName;
	CLTWnd* pParentWnd;
	HSURFACE hSurf;
	char *szBitmap;
	int xPos;
	int yPos;
	DWORD dwFlags;
	DWORD dwState;
};

inline LTWNDCREATESTRUCT::LTWNDCREATESTRUCT()
{
	memset(this, 0, sizeof(LTWNDCREATESTRUCT));
	szWndName = "LTWnd";
}

// Classes
class CLTWnd
{

public:

	CLTWnd();
	virtual ~CLTWnd() { Term(); }

	typedef BOOL (*tSendMessageFunc)(CLTWnd* pSender, int nMsg, int nParam1 = 0,int nParam2 = 0);

	// Initialization and termination
	virtual BOOL			Init(LTWNDCREATESTRUCT *pcs);
	virtual BOOL			Init(int nControlID, char* szWndName, CLTWnd* pParentWnd, HSURFACE hSurf, int xPos = 0, int yPos = 0, DWORD dwFlags = LTWF_NORMAL, DWORD dwState = LTWS_NORMAL);
	virtual BOOL			InitFromBitmap(int nControlID, char* szWndName, CLTWnd* pParentWnd, char* szBitmap, int xPos = 0, int yPos = 0, DWORD dwFlags = LTWF_NORMAL, DWORD dwState = LTWS_NORMAL);
	virtual void			Term();
	void					TermNext() { m_bEnabled = FALSE; m_bVisible = FALSE; m_dwFlags |= LTWF_TERM; }
	virtual void			FreeAllSurfaces(CLTSurfaceArray* pcollSurfs = NULL);
	BOOL					IsInitialized() { return m_bInitialized; }

	HSURFACE				GetSurface() { return m_hSurf; }
	POSITION				GetPos() { return(m_pos); }
	CString&				GetName() { return m_sWndName; }
	DWORD					GetFlags() { return m_dwFlags; }
	CLTWnd*					GetParent() { return m_pParentWnd; }
	int						GetCursorXClick() { return m_xCursorClick; }
	int						GetCursorYClick() { return m_yCursorClick; }
	BOOL					IsFlagSet(DWORD dwFlag) { return (m_dwFlags & dwFlag); }
	//void					GetTransparentColor(float* r, float* g, float* b);
    HLTCOLOR                GetTransparentColor();
	DWORD					GetState() { return m_dwState; }
	int						GetWidth() { return m_nWidth; }
	int						GetHeight() { return m_nHeight; }
	void					SetWidth(int nWidth) { m_nWidth = nWidth; }
	void					SetHeight(int nHeight) { m_nHeight = nHeight; }
	void					SetWidthHeight(int nWidth, int nHeight) { m_nWidth = nWidth; m_nHeight = nHeight; }
	void					SetSurfaceOffsets(int x, int y, int w, int h);

	void					SetControlID(int nControlID) { m_nControlID = nControlID; }
	int						GetControlID() { return m_nControlID; }

	void					SetFlag(DWORD dwFlag) { m_dwFlags |= dwFlag; }
	void					ClearFlag(DWORD dwFlag) { m_dwFlags &= ~dwFlag; }
	void					SetPos(POSITION pos) { m_pos = pos; }
	HSURFACE				SetSurface(HSURFACE hSurf, BOOL bDeleteSurf = FALSE);
	void					SetTransparentColor(float r = 1.0, float g = 0.0, float b = 1.0);
	void					DeleteTransparentColor();
    void                    SetTransparentColor(HLTCOLOR hColor) { m_hColorTransparent = hColor; }
	void					SetDeleteSurf(BOOL bDelete) { m_bDeleteSurfOnTerm = bDelete; }

	virtual BOOL			Update(float fTimeDelta);
	virtual BOOL			Draw(HSURFACE hSurf);
	CLTWnd*					SetFocus();
	BOOL					IsActiveWindow() { return (this == s_pWndActive); }
	BOOL					SetWindowPos(CLTWnd* pInsertAfter, int xPos, int yPos, int nWidth, int nHeight, DWORD dwFlags);

	virtual BOOL			MoveWindow(int xPos, int yPos, int nWidth, int nHeight, BOOL bReDraw = TRUE);
	virtual BOOL			MoveWindow(CRect& rcPos, BOOL bReDraw = TRUE);
	virtual BOOL 			MoveWindow(int xPos, int yPos, BOOL bReDraw = TRUE) { m_xPos = xPos; m_yPos = yPos; return TRUE; }
	virtual BOOL			MoveWindowX(int xPos, BOOL bReDraw = TRUE) { m_xPos = xPos; return TRUE; }
	virtual BOOL			MoveWindowY(int yPos, BOOL bReDraw = TRUE) { m_yPos = yPos; return TRUE; }

	void					SetShowSounds(const char *szShowSound = NULL, const char *szHideSound = NULL);
	virtual BOOL			ShowWindow(BOOL bShow = TRUE, BOOL bPlaySound = TRUE, BOOL bAnimate = TRUE);
	BOOL					IsVisible() { return m_bVisible; }
	virtual BOOL			EnableWindow(BOOL bEnable = TRUE);
	BOOL					IsEnabled() const { return m_bEnabled; }
	void					ShowAllChildren(BOOL bShow = TRUE);

	virtual BOOL			PtInWnd(int x, int y);  // x,y are screen coords
	CLTWnd*					GetWndFromPt(int x, int y);

	void					ArrangeAllChildren(DWORD dwWidth, DWORD dwHeight, DWORD dwOldWidth, DWORD dwOldHeight);

	void					SetCapture(CLTWnd* pWnd) { s_pWndCapture = pWnd; }

	// Relative coords
	int						GetRelativeLeft() { return m_xPos; }
	int						GetRelativeTop() { return m_yPos; }

	// Absolute screen coordinates
	int						GetWindowLeft();
	int						GetWindowTop();
	void					GetWindowRect(CRect* pRect);
	void					GetClipRect(CRect* pRect);
	virtual void			GetSurfaceRect(CRect *pRect);

	virtual BOOL			SendMessage(CLTWnd* pSender, int nMsg, int nParam1 = 0,int nParam2 = 0);
	void					SetSendMessageFunc(tSendMessageFunc pFunc) { m_pSendMessageFunc = pFunc; }

	// These will get passed down to the appropriate window
	BOOL					HandleMouseMove(int xPos, int yPos);
	BOOL					HandleLButtonDown(int xPos, int yPos);
	BOOL					HandleLButtonUp(int xPos, int yPos);
	BOOL					HandleLButtonDblClick(int xPos, int yPos);
	BOOL					HandleRButtonDown(int xPos, int yPos);
	BOOL					HandleRButtonUp(int xPos, int yPos);

	void					RemoveAllChildren();
	void					RemoveFromParent();

	void					AddChild(CLTWnd* pWnd, BOOL bIgnoreTopmost = FALSE);
	void					SetTopmost();
	CPtrList				m_lstChildren;			// List of windows that cannot go outside of our borders

protected:

	virtual BOOL			DrawToSurface(HSURFACE hSurfDest);
	// NOTE: return TRUE on these if you are capturing the message.
	// i.e. if you do not want the game to process them.
	virtual BOOL			OnLButtonUp(int xPos, int yPos);
	virtual BOOL			OnLButtonDown(int xPos, int yPos);
	virtual BOOL			OnLButtonDblClick(int xPos, int yPos) { return(OnLButtonDown(xPos,yPos)); }

	virtual BOOL			OnRButtonUp(int xPos, int yPos) { return FALSE; }
	virtual BOOL			OnRButtonDown(int xPos, int yPos);
	virtual BOOL			OnRButtonDblClick(int xPos, int yPos) { return FALSE; }

	virtual BOOL			OnMouseMove(int xPos, int yPos);
	virtual void			OnMouseEnter() {}
	virtual void			OnMouseLeave() {}
	virtual BOOL			OnDrag(int xPos, int yPos);
	virtual BOOL			OnSetFocus(CLTWnd* pOldWnd) { return FALSE; }
	virtual BOOL			OnKillFocus(CLTWnd* pNewWnd) { return FALSE; }

	BOOL					m_bEnabled;				// Are we enabled (accepting clicks/commands)
	BOOL					m_bVisible;				// Are we visible
	int						m_nControlID;			// ID of this window
	int						m_xPos;					// x pos relative to parent's top/left
	int						m_yPos;					// y pos relative to parent's top/left
	int						m_nWidth;				// Window width
	int						m_nHeight;				// Window height

	int						m_xSurfPos;				// Surface pos relative to top/left
	int						m_ySurfPos;				// Surface pos relative to top/left
	int						m_nSurfWidth;			// Surface width
	int						m_nSurfHeight;			// Surface height

	int						m_xCursor;				// Cursor relative to top/left
	int						m_yCursor;				// Cursor relative to top/left
	int						m_xCursorClick;			// Where they clicked relative to top/left
	int						m_yCursorClick;			// Where they clicked relative to top/left

	HSURFACE				m_hSurf;				// Main surface

	CLTWnd*					m_pParentWnd;			// Parent window
	CString					m_sWndName;				// Name of the window as string
	BOOL					m_bInitialized;			// Are we initialized
	POSITION				m_pos;					// Position in paren't list

    HLTCOLOR                m_hColorTransparent;    // Transparent color (handle)
	DWORD					m_dwState;				// Normal, minimized, closed, etc...
	CString					m_sShowSound;			// Sound to play when the window is shown
	CString					m_sHideSound;			// Sound to play when the window hides

	BOOL					(*m_pSendMessageFunc)(CLTWnd* pSender, int nMsg, int nParam1 = 0,int nParam2 = 0);

	/*struct
	{
		float r;
		float g;
		float b;
	}						m_colorTransparent;		// Transparent color as (r,g,b)*/
	BOOL					m_bDeleteSurfOnTerm;	// Should we delete the surface?
	DWORD					m_dwFlags;				// Window flags

	static CLTWnd*			s_pWndActive;			// Window that has the input focus
	static CLTWnd*			s_pMainWnd;				// Main screen window
	static CLTWnd*			s_pWndCapture;			// Window that has the mouse capture
	static CLTWnd*			s_pLastMouseWnd;		// Last window the mouse was over
};

typedef CArray<CLTWnd*,CLTWnd*> CLTWndArray;

// Inlines
inline CLTWnd::CLTWnd()
{
	m_pos					= NULL;
	m_dwFlags				= 0;
	m_xPos					= 0;
	m_yPos					= 0;
	m_nWidth				= 0;
	m_nHeight				= 0;
	m_xSurfPos				= 0;
	m_ySurfPos				= 0;
	m_nSurfWidth			= 0;
	m_nSurfHeight			= 0;
	m_bEnabled				= TRUE;
	m_xCursor				= 0;
	m_yCursor				= 0;
	m_xCursorClick			= 0;
	m_yCursorClick			= 0;
	m_hSurf					= 0;
	m_pos					= NULL;
	m_bInitialized			= FALSE;
	m_bVisible				= TRUE;
    m_hColorTransparent     = g_hColorTransparent;//g_pLTClient->CreateColor(0.0, 0.0, 0.0, FALSE);
	m_bDeleteSurfOnTerm		= FALSE;
	m_nControlID			= 0;
	m_dwState				= LTWS_NORMAL;
	m_pSendMessageFunc		= NULL;
}

inline void CLTWnd::AddChild(CLTWnd* pWnd, BOOL bIgnoreTopmost)
{
	ASSERT(m_bInitialized);
	ASSERT(pWnd);

	if(bIgnoreTopmost || pWnd->IsFlagSet(LTWF_TOPMOST))
	{
		// Stick it into our list of children and set its position variable
		pWnd->SetPos(m_lstChildren.AddTail(pWnd));
	}
	else
	{
		POSITION pos = m_lstChildren.GetTailPosition();
		while (pos)
		{
			CLTWnd* pWndTest = (CLTWnd*)m_lstChildren.GetPrev(pos);
			ASSERT(pWndTest);
			if(!pWndTest->IsFlagSet(LTWF_TOPMOST))
			{
				pWnd->SetPos(m_lstChildren.InsertAfter(pWndTest->GetPos(),pWnd));
				return;
			}
		}
		pWnd->SetPos(m_lstChildren.AddHead(pWnd));
	}
}

/*inline void CLTWnd::GetTransparentColor(float* r, float* g, float* b)
{
	*r = m_colorTransparent.r;
	*g = m_colorTransparent.g;
	*b = m_colorTransparent.b;
}*/

inline HLTCOLOR CLTWnd::GetTransparentColor()
{
	return m_hColorTransparent;
}

inline void CLTWnd::RemoveFromParent()
{
	if(m_pParentWnd)
	{
		ASSERT(m_pos);
		m_pParentWnd->m_lstChildren.RemoveAt(m_pos);
		m_pParentWnd = NULL;
		m_pos = NULL;
	}
}

inline int CLTWnd::GetWindowLeft()
{
	if (m_pParentWnd)
		return m_xPos + m_pParentWnd->GetWindowLeft();
	return 0; // root window so offset is 0
}

inline int CLTWnd::GetWindowTop()
{
	if (m_pParentWnd)
		return m_yPos + m_pParentWnd->GetWindowTop();
	return 0; // root window so offset is 0
}

inline void CLTWnd::SetSurfaceOffsets(int x, int y, int w, int h)
{
	m_xSurfPos = x;
	m_ySurfPos = y;
	m_nSurfWidth = w;
	m_nSurfHeight = h;
}

inline BOOL CLTWnd::SendMessage(CLTWnd* pSender, int nMsg, int nParam1, int nParam2)
{
	if (m_pSendMessageFunc && m_pSendMessageFunc(pSender,nMsg,nParam1,nParam2))
		return TRUE;

	return FALSE;
}

inline BOOL CLTWnd::Init(LTWNDCREATESTRUCT* pcs)
{
	if(!pcs)
		return FALSE;

	if(pcs->szBitmap)
	{
		return(InitFromBitmap(pcs->nControlID,pcs->szWndName,pcs->pParentWnd,pcs->szBitmap,
							pcs->xPos,pcs->yPos,pcs->dwFlags,pcs->dwState));
	}
	else
	{
		return(Init(pcs->nControlID,pcs->szWndName,pcs->pParentWnd,pcs->hSurf,
							pcs->xPos,pcs->yPos,pcs->dwFlags,pcs->dwState));
	}
}

inline void CLTWnd::SetTopmost()
{
	if(m_pParentWnd)
	{
		m_pParentWnd->m_lstChildren.RemoveAt(GetPos());
		m_pParentWnd->AddChild(this);
	}
}


inline void CLTWnd::SetShowSounds(const char *szShowSound, const char *szHideSound)
{
	m_sShowSound = szShowSound;
	m_sHideSound = szHideSound;
}

#endif
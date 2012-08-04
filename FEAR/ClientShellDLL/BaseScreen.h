// BaseScreen.h: interface for the CBaseScreen class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(_BASE_SCREEN_H_)
#define _BASE_SCREEN_H_


#include "LTGUIMgr.h"
#include "LTPoly.h"

class CScreenMgr;

const uint16 kNoSelection = 0xFFFF;

class CBaseScreen : public CLTGUICommandHandler
{
public:
	CBaseScreen();
	virtual ~CBaseScreen();

	// Initialization/Termination
    virtual bool   Init(int nScreenID);
	virtual void	Term();

	// Builds the screen
    virtual bool   Build();
    inline  bool   IsBuilt() const { return m_bBuilt; }


	virtual void	Escape();

    virtual uint32 OnCommand(uint32 dwCommand, uint32 dwParam1, uint32 dwParam2);

	// Get the screen ID
	inline int	GetScreenID() const	{ return m_nScreenID; }


	// Renders the screen to a surface
    virtual bool   Render();
	// Returns false if the screen should exit as a result of this update
	virtual bool	UpdateInterfaceSFX();

    bool	CreateTitle(const char* szStringID);
    bool	CreateTitle(const wchar_t *pszTitle);

	//set the background for the screen
    void         UseBack(bool	bBack=true,bool	bOK=false, bool bReturn = false );

	// This is called when the screen gets or loses focus
    virtual void    OnFocus(bool bFocus);

	// This is called when the selected item changes
    virtual void    OnSelectionChange()			{};

	// Handles a key press.  Returns FALSE if the key was not processed through this method.
	// Left, Up, Down, Right, and Enter are automatically passed through OnUp(), OnDown(), etc.
    virtual bool   HandleKeyDown(int key, int rep);
    virtual bool   HandleKeyUp(int key) { LTUNREFERENCED_PARAMETER( key ); return false; }
    virtual bool   HandleChar(wchar_t c);
    virtual bool   HandleForceUpdate() { return false; }

	//reset selection and help display
	virtual void	ForceMouseUpdate();
	virtual void	UpdateHelpText();

	// Mouse messages
    virtual bool   OnLButtonDown(int x, int y);
    virtual bool   OnLButtonUp(int x, int y);
    virtual bool   OnLButtonDblClick(int x, int y);
    virtual bool   OnRButtonDown(int x, int y);
    virtual bool   OnRButtonUp(int x, int y);
    virtual bool   OnRButtonDblClick(int x, int y);
    virtual bool   OnMouseMove(int x, int y);
	virtual bool   OnMouseWheel(int x, int y, int zDelta);

	uint16			AddControl(CLTGUICtrl* pCtrl);
	void			RemoveControl(CLTGUICtrl* pControl,bool bDelete = true);

	uint16			SetSelection(uint16 select, bool bFindSelectable = false);
	uint16			NextSelection();
	uint16			PreviousSelection();
	inline uint16	GetSelection() const		{return m_nSelection;}
	inline uint16	GetOldSelection() const		{return m_nOldSelection;}

	void			SetCapture(CLTGUICtrl *pCtrl, bool bCaptureAlways = false)	{m_pCaptureCtrl = pCtrl; m_bCaptureAlways = bCaptureAlways; }
	CLTGUICtrl*		GetCapture()					{return m_pCaptureCtrl;}

	inline CLTGUICtrl*		GetSelectedControl()  {return GetControl(m_nSelection);}
	CLTGUICtrl*				GetControl(uint16 nIndex);
	uint16					GetIndex(CLTGUICtrl* pCtrl);

	inline void		SetItemSpacing(int space)	{m_nItemSpacing = space;}
	inline int		GetItemSpacing() const		{ return m_nItemSpacing; }
	virtual void	ScreenDimsChanged();

	LTRect2n const&	GetScreenRect( ) const { return m_ScreenRect; }

	HRECORD			GetLayoutRecord( ) const { return m_hLayout; }

	virtual         void GetHelpString(const char* szHelpId, uint16 nControlIndex, wchar_t *buffer, int bufLen);

	// These AddXXX() functions call CreateXXX() and then add the control to the control list
    CLTGUITextCtrl*     AddTextItem(const char* szStringID, CLTGUICtrl_create cs, bool bFixed = false, const char* szFont = NULL, uint32 nFontHeight = 0);
    CLTGUITextCtrl*     AddTextItem(const wchar_t *pString, CLTGUICtrl_create cs, bool bFixed = false, const char* szFont = NULL, uint32 nFontHeight = 0);

	CLTGUIListCtrl*		AddList( CLTGUIListCtrl_create cs );
	CLTGUIListCtrlEx* AddListEx( CLTGUIListCtrlEx_create cs );

	CLTGUIHeaderCtrl*	AddHeaderCtrl( CLTGUIHeaderCtrl_create cs );
	CLTGUIFillFrame*	AddFillFrame( CLTGUIFillFrame_create cs );

	CLTGUITabCtrl*	AddTabCtrl( CLTGUITabCtrl_create cs );
	
    CLTGUICycleCtrl*     AddCycle(const char* szStringID, CLTGUICycleCtrl_create cs , bool bFixed = false, const char* szFont = NULL, uint32 nFontHeight = 0);
    CLTGUICycleCtrl*     AddCycle(const wchar_t *, CLTGUICycleCtrl_create cs, bool bFixed = false, const char* szFont = NULL, uint32 nFontHeight = 0);

    CLTGUIToggle*     AddToggle(const char* szStringID, CLTGUIToggle_create cs, bool bFixed = false, const char* szFont = NULL, uint32 nFontHeight = 0);
    CLTGUIToggle*     AddToggle(const wchar_t *, CLTGUIToggle_create cs, bool bFixed = false, const char* szFont = NULL, uint32 nFontHeight = 0);

    CLTGUISlider*     AddSlider(const char* szStringID,  CLTGUISlider_create cs, bool bFixed = false, const char* szFont = NULL, uint32 nFontHeight = 0);
    CLTGUISlider*     AddSlider(const wchar_t *,  CLTGUISlider_create cs, bool bFixed = false, const char* szFont = NULL, uint32 nFontHeight = 0);

	CLTGUIScrollBar*	AddScrollBar(CLTGUIScrollBar_create cs );

	CLTGUIColumnCtrl* AddColumnCtrl(CLTGUICtrl_create cs, bool bFixed = false, const char* szFont = NULL, uint32 nFontHeight = 0);


	// These CreateXXX() create controls but do not add them to the control list
    CLTGUITextCtrl*     CreateTextItem(const char* szStringID, CLTGUICtrl_create cs, bool bFixed = false, const char* szFont = NULL, uint32 nFontHeight = 0);
    CLTGUITextCtrl*     CreateTextItem(const wchar_t *, CLTGUICtrl_create cs, bool bFixed = false, const char* szFont = NULL, uint32 nFontHeight = 0);

	CLTGUIListCtrl*			CreateList(CLTGUIListCtrl_create cs );
	CLTGUIListCtrlEx*	CreateListEx(CLTGUIListCtrlEx_create cs );

	CLTGUIHeaderCtrl*		CreateHeaderCtrl( CLTGUIHeaderCtrl_create cs );

	CLTGUIFillFrame*		CreateFillFrame( CLTGUIFillFrame_create cs );

	CLTGUITabCtrl*		CreateTabCtrl( CLTGUITabCtrl_create cs );

	CLTGUICycleCtrl*     CreateCycle(const char* szStringID, CLTGUICycleCtrl_create cs , bool bFixed = false, const char* szFont = NULL, uint32 nFontHeight = 0);
	CLTGUICycleCtrl*     CreateCycle(const wchar_t *, CLTGUICycleCtrl_create cs , bool bFixed = false, const char* szFont = NULL, uint32 nFontHeight = 0);
	bool				CreateCycle(CLTGUICycleCtrl*, const wchar_t *, CLTGUICycleCtrl_create cs , bool bFixed = false, const char* szFont = NULL, uint32 nFontHeight = 0);

    CLTGUIToggle*     CreateToggle(const char* szStringID, CLTGUIToggle_create cs, bool bFixed = false, const char* szFont = NULL, uint32 nFontHeight = 0);
    CLTGUIToggle*     CreateToggle(const wchar_t *, CLTGUIToggle_create cs, bool bFixed = false, const char* szFont = NULL, uint32 nFontHeight = 0);
	bool			CreateToggle(CLTGUIToggle* pToggle, const wchar_t *pString, CLTGUIToggle_create cs, bool bFixed = false, const char* szFont = NULL, uint32 nFontHeight = 0);

    CLTGUISlider*     CreateSlider(const char* szStringID, CLTGUISlider_create cs, bool bFixed = false, const char* szFont = NULL, uint32 nFontHeight = 0);
    CLTGUISlider*     CreateSlider(const wchar_t *,  CLTGUISlider_create cs, bool bFixed = false, const char* szFont = NULL, uint32 nFontHeight = 0);
    bool			CreateSlider(CLTGUISlider* pCtrl,  const wchar_t *,  CLTGUISlider_create cs, bool bFixed = false, const char* szFont = NULL, uint32 nFontHeight = 0);

	CLTGUIScrollBar*	CreateScrollBar( CLTGUIScrollBar_create cs );

	CLTGUIColumnCtrl* CreateColumnCtrl(CLTGUICtrl_create cs, bool bFixed = false, const char* szFont = NULL, uint32 nFontHeight = 0);
	bool CreateColumnCtrl(CLTGUIColumnCtrl* pCtrl, CLTGUICtrl_create cs, bool bFixed = false, const char* szFont = NULL, uint32 nFontHeight = 0);

	// Calls UpdateData on each control in the screen
    virtual void            UpdateData(bool bSaveAndValidate=true);

	// returns the default position for the next control to be added
	const LTVector2n GetDefaultPos() const {return m_DefaultPos;}


protected:

	// Handle input
    virtual bool   OnUp();
    virtual bool   OnDown();
    virtual bool   OnLeft();
    virtual bool   OnRight();
    virtual bool   OnEnter();

	// Gets the index of the control that is under the specific screen point.
	// Returns FALSE if there isn't one under the specified point.
    bool       GetControlUnderPoint(int xPos, int yPos, uint16 *pnIndex);

protected:

	void			RemoveAll(bool bDelete = true);

	void			CreateBack(bool bOK = false, bool bReturn = false);

	virtual void	CreateInterfaceSFX();
	virtual void	RemoveInterfaceSFX();


protected:

    bool			m_bInit;
    bool			m_bBuilt;
	bool			m_bVisited;
	HRECORD			m_hLayout;

	
	CScreenMgr*		m_pScreenMgr;

	int				m_nScreenID;		// The ID of this screen
	int				m_nContinueID;		// The ID of the screen to show when continue is clicked

	//title stuff
	CLTGUIString	m_TitleString;		// The title string
	LTVector2n		m_BaseTitlePos;
	std::string		m_TitleFont;
	uint32			m_TitleSize;
    uint32			m_TitleColor;

	bool			m_bHaveFocus;

	// Array of free controls that this screen owns
	ControlArray		m_controlArray;

	int				m_nItemSpacing;

	CLTGUICtrl*		m_pCaptureCtrl;
	// if true then the capture control always gets input messages
	// if false then the capture control will only get messages if the mouse is over the control
	bool			m_bCaptureAlways;

    LTRect2n        m_ScreenRect;
    uint32			m_SelectedColor;
    uint32			m_NonSelectedColor;
    uint32			m_DisabledColor;

	const char*     m_szCurrHelpID;

	bool	m_bBack;
	
	LTVector2n		m_DefaultPos;

	LTVector2		m_vfLastScale;

	uint16			m_nSelection;
	uint16			m_nOldSelection;
	uint16			m_nLMouseDownItemSel;
	uint16			m_nRMouseDownItemSel;

	StopWatchTimer	m_TransitionTimer;

    static LTVector		s_vPos; 
	static LTVector		s_vU; 
	static LTVector		s_vR; 
	static LTVector		s_vF;
    static LTRotation	s_rRot;

	TextureReference	m_hBackTexture;
	LTPoly_GT4			m_BackPoly;

	// the cursor to set at the end of a mouse move
	HRECORD				m_hCursorRecord;

};

inline  CLTGUITextCtrl* CBaseScreen::AddTextItem(const char* szStringID, CLTGUICtrl_create cs, bool bFixed /* = false */, const char* szFont /* = NULL */, uint32 nFontHeight /* = 0 */)
{
	CLTGUITextCtrl* pCtrl = CreateTextItem(szStringID, cs, bFixed, szFont, nFontHeight);
	if (pCtrl)
		AddControl(pCtrl);
	return pCtrl;
}
inline  CLTGUITextCtrl* CBaseScreen::AddTextItem(const wchar_t *pString, CLTGUICtrl_create cs, bool bFixed /* = false */, const char* szFont /* = NULL */, uint32 nFontHeight /* = 0 */)
{
	CLTGUITextCtrl* pCtrl = CreateTextItem(pString, cs, bFixed, szFont, nFontHeight);
	if (pCtrl)
		AddControl(pCtrl);
	return pCtrl;
}

inline CLTGUIListCtrl* CBaseScreen::AddList(CLTGUIListCtrl_create cs )
{
	
	CLTGUIListCtrl* pList = CreateList(cs);
    if (pList)
		AddControl(pList);	
	return pList;
}

inline CLTGUIListCtrlEx* CBaseScreen::AddListEx(CLTGUIListCtrlEx_create cs )
{
	CLTGUIListCtrlEx* pList = CreateListEx(cs);
	if (pList)
		AddControl(pList);	
	return pList;
}

inline CLTGUIHeaderCtrl* CBaseScreen::AddHeaderCtrl(CLTGUIHeaderCtrl_create cs )
{
	CLTGUIHeaderCtrl* pCtrl = CreateHeaderCtrl(cs);
	if (pCtrl)
		AddControl(pCtrl);	
	return pCtrl;
}

inline CLTGUITabCtrl* CBaseScreen::AddTabCtrl(CLTGUITabCtrl_create cs )
{
	CLTGUITabCtrl* pCtrl = CreateTabCtrl(cs);
	if (pCtrl)
		AddControl(pCtrl);	
	return pCtrl;
}

inline CLTGUIFillFrame* CBaseScreen::AddFillFrame(CLTGUIFillFrame_create cs )
{
	CLTGUIFillFrame* pCtrl = CreateFillFrame(cs);
	if (pCtrl)
		AddControl(pCtrl);	
	return pCtrl;
}

inline  CLTGUICycleCtrl* CBaseScreen::AddCycle(const char* szStringID, CLTGUICycleCtrl_create cs, bool bFixed /* = false */, const char* szFont /* = NULL */, uint32 nFontHeight /* = 0 */)
{
	CLTGUICycleCtrl* pCtrl = CreateCycle(szStringID, cs, bFixed, szFont, nFontHeight);
	if (pCtrl)
		AddControl(pCtrl);
	return pCtrl;
}
inline  CLTGUICycleCtrl* CBaseScreen::AddCycle(const wchar_t *pString, CLTGUICycleCtrl_create cs, bool bFixed /* = false */, const char* szFont /* = NULL */, uint32 nFontHeight /* = 0 */)
{
	CLTGUICycleCtrl* pCtrl = CreateCycle(pString, cs, bFixed, szFont, nFontHeight);
	if (pCtrl)
		AddControl(pCtrl);
	return pCtrl;
}


inline  CLTGUIToggle* CBaseScreen::AddToggle(const char* szStringID, CLTGUIToggle_create cs, bool bFixed, const char* szFont, uint32 nFontHeight)
{
	CLTGUIToggle* pCtrl = CreateToggle(szStringID, cs, bFixed, szFont, nFontHeight);
	if (pCtrl)
		AddControl(pCtrl);
	return pCtrl;
}
inline  CLTGUIToggle* CBaseScreen::AddToggle(const wchar_t *pString, CLTGUIToggle_create cs, bool bFixed, const char* szFont, uint32 nFontHeight)
{
	CLTGUIToggle* pCtrl = CreateToggle(pString, cs, bFixed, szFont, nFontHeight);
	if (pCtrl)
		AddControl(pCtrl);
	return pCtrl;
}


inline  CLTGUISlider* CBaseScreen::AddSlider(const char* szStringID, CLTGUISlider_create cs, bool bFixed /* = false */, const char* szFont /* = NULL */, uint32 nFontHeight /* = 0 */)
{
	CLTGUISlider* pCtrl = CreateSlider(szStringID, cs, bFixed, szFont, nFontHeight);
	if (pCtrl)
		AddControl(pCtrl);
	return pCtrl;
}
inline  CLTGUISlider* CBaseScreen::AddSlider(const wchar_t *pString, CLTGUISlider_create cs, bool bFixed /* = false */, const char* szFont /* = NULL */, uint32 nFontHeight /* = 0 */)
{
	CLTGUISlider* pCtrl = CreateSlider(pString, cs, bFixed, szFont, nFontHeight);
	if (pCtrl)
		AddControl(pCtrl);
	return pCtrl;
}

inline CLTGUIScrollBar* CBaseScreen::AddScrollBar(CLTGUIScrollBar_create cs )
{
	CLTGUIScrollBar* pCtrl = CreateScrollBar( cs );
	if( pCtrl )
		AddControl(pCtrl);
	return pCtrl;
}

inline  CLTGUIColumnCtrl* CBaseScreen::AddColumnCtrl(CLTGUICtrl_create cs, bool bFixed /* = false */, const char* szFont /* = NULL */, uint32 nFontHeight /* = 0 */)
{
	
	CLTGUIColumnCtrl* pCtrl = CreateColumnCtrl(cs, bFixed, szFont, nFontHeight);
	if (pCtrl)
		AddControl(pCtrl);
	return pCtrl;
}


#endif // _BASE_SCREEN_H_

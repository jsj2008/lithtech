// BaseScreen.h: interface for the CBaseScreen class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(_BASE_SCREEN_H_)
#define _BASE_SCREEN_H_


#include "LTGUIMgr.h"
#include "BaseScaleFX.h"
#include "ChainedFX.h"
#include "LTPoly.h"

struct INT_CHAR;

#define MAX_INT_ATTACHMENTS 5
struct INT_ATTACH
{
	INT_ATTACH() { fScale = 1.0f;nAttachmentID = -1;szSocket[0]=LTNULL;}

	LTFLOAT	fScale;
	int		nAttachmentID;
	char	szSocket[64];
};
struct AttachmentData
{
	AttachmentData() { socket = INVALID_MODEL_SOCKET; }

	CBaseScaleFX	sfx;
	HMODELSOCKET	socket;
};

class CScreenMgr;



const uint16 kNoSelection = 0xFFFF;
const LTIntPt kDefaultPos(-1,-1);

class CBaseScreen : public CLTGUICommandHandler
{
public:
	CBaseScreen();
	virtual ~CBaseScreen();

	// Initialization/Termination
    virtual LTBOOL   Init(int nScreenID);
	virtual void	Term();

	// Builds the screen
    virtual LTBOOL   Build();
    inline  LTBOOL   IsBuilt() const { return m_bBuilt; }


	virtual void	Escape();

    virtual uint32 OnCommand(uint32 dwCommand, uint32 dwParam1, uint32 dwParam2);

	// Get the screen ID
	inline int	GetScreenID() const	{ return m_nScreenID; }


	// Renders the screen to a surface
    virtual LTBOOL   Render(HSURFACE hDestSurf);
	// Returns false if the screen should exit as a result of this update
	virtual bool	UpdateInterfaceSFX();

	// Creates the title for the screen
    void	SetTitlePos(LTIntPt pt);
	void	SetTitleFont(uint8 nFont);
	void	SetTitleSize(uint8 nFontSize);
    void    SetTitleColor(uint32 titleColor);

    LTBOOL	CreateTitle(int nStringID);
    LTBOOL	CreateTitle(char *lpszTitle);

	//set the background for the screen
    void         UseBack(LTBOOL	bBack=LTTRUE,LTBOOL	bOK=LTFALSE, LTBOOL bReturn = LTFALSE );

	// This is called when the screen gets or loses focus
    virtual void    OnFocus(LTBOOL bFocus);

	// This is called when the selected item changes
    virtual void    OnSelectionChange()			{};

	// Handles a key press.  Returns FALSE if the key was not processed through this method.
	// Left, Up, Down, Right, and Enter are automatically passed through OnUp(), OnDown(), etc.
    virtual LTBOOL   HandleKeyDown(int key, int rep);
    virtual LTBOOL   HandleKeyUp(int key) { return LTFALSE; }
    virtual LTBOOL   HandleChar(unsigned char c);
    virtual LTBOOL   HandleForceUpdate() { return LTFALSE; }

	//reset selection and help display
	virtual void	ForceMouseUpdate();
	virtual void	UpdateHelpText();

	// Mouse messages
    virtual LTBOOL   OnLButtonDown(int x, int y);
    virtual LTBOOL   OnLButtonUp(int x, int y);
    virtual LTBOOL   OnLButtonDblClick(int x, int y);
    virtual LTBOOL   OnRButtonDown(int x, int y);
    virtual LTBOOL   OnRButtonUp(int x, int y);
    virtual LTBOOL   OnRButtonDblClick(int x, int y);
    virtual LTBOOL   OnMouseMove(int x, int y);

	inline int 		GetPageLeft() const {return m_PageRect.left;}
	inline int 		GetPageRight() const {return m_PageRect.right;}
	inline int 		GetPageTop() const {return m_PageRect.top;}
	inline int 		GetPageBottom() const {return m_PageRect.bottom;}

	uint16			AddControl(CLTGUICtrl* pCtrl);
	void			RemoveControl(CLTGUICtrl* pControl,LTBOOL bDelete = LTTRUE);

	uint16			SetSelection(uint16 select, LTBOOL bFindSelectable = LTFALSE);
	uint16			NextSelection();
	uint16			PreviousSelection();
	inline uint16	GetSelection() const		{return m_nSelection;}
	inline uint16	GetOldSelection() const		{return m_nOldSelection;}

	void			SetCapture(CLTGUICtrl *pCtrl)	{m_pCaptureCtrl = pCtrl;}
	CLTGUICtrl*		GetCapture()					{return m_pCaptureCtrl;}

	inline CLTGUICtrl*		GetSelectedControl()  {return GetControl(m_nSelection);}
	CLTGUICtrl*				GetControl(uint16 nIndex);
	uint16					GetIndex(CLTGUICtrl* pCtrl);

	inline void		SetItemSpacing(int space)	{m_nItemSpacing = space;}
	inline int		GetItemSpacing() const		{ return m_nItemSpacing; }
	virtual void	ScreenDimsChanged();


	//this function creates a string, the caller must free it
    virtual         void GetHelpString(uint32 dwHelpId, uint16 nControlIndex, char *buffer, int bufLen);

	//default font for items is the large font
	// These AddXXX() functions call CreateXXX() and then add the control to the control list
    CLTGUITextCtrl*     AddTextItem(int stringID, uint32 commandID, int helpID, LTIntPt pos = kDefaultPos, LTBOOL bFixed = LTFALSE, int nFont = -1);
    CLTGUITextCtrl*     AddTextItem(char *pString, uint32 commandID, int helpID, LTIntPt pos = kDefaultPos, LTBOOL bFixed = LTFALSE, int nFont = -1);

	CLTGUIListCtrl*		AddList(LTIntPt pos, uint16 nHeight, LTBOOL bUseArrows = LTFALSE, uint16 nArrowOffset = 0);

    CLTGUICycleCtrl*     AddCycle(int stringID, int helpID, int nHeaderWidth, uint8 *pnValue=LTNULL, LTIntPt pos = kDefaultPos, LTBOOL bFixed = LTFALSE, int nFont = -1);
    CLTGUICycleCtrl*     AddCycle(char *pString, int helpID, int nHeaderWidth, uint8 *pnValue=LTNULL, LTIntPt pos = kDefaultPos, LTBOOL bFixed = LTFALSE, int nFont = -1);

    CLTGUIToggle*     AddToggle(int stringID, int helpID, int nHeaderWidth, LTBOOL *pbValue=LTNULL, LTIntPt pos = kDefaultPos, LTBOOL bFixed = LTFALSE, int nFont = -1);
    CLTGUIToggle*     AddToggle(char *pString, int helpID, int nHeaderWidth, LTBOOL *pbValue=LTNULL, LTIntPt pos = kDefaultPos, LTBOOL bFixed = LTFALSE, int nFont = -1);

    CLTGUISlider*     AddSlider(int stringID,  int helpID, int nHeaderWidth, int nBarWidth, int nBarHeight = -1, 
								int *pnValue = LTNULL, LTIntPt pos = kDefaultPos, LTBOOL bFixed = LTFALSE, int nFont = -1);
    CLTGUISlider*     AddSlider(char *pString,  int helpID, int nHeaderWidth, int nBarWidth, int nBarHeight = -1, 
								int *pnValue = LTNULL, LTIntPt pos = kDefaultPos, LTBOOL bFixed = LTFALSE, int nFont = -1);

	CLTGUIColumnCtrl* AddColumnCtrl(uint32 commandID, int helpID, LTIntPt pos = kDefaultPos, LTBOOL bFixed = LTFALSE, int nFont = -1);


	// These CreateXXX() create controls but do not add them to the control list
    CLTGUITextCtrl*     CreateTextItem(int stringID, uint32 commandID, int helpID, LTIntPt pos = kDefaultPos, LTBOOL bFixed = LTFALSE, int nFont = -1);
    CLTGUITextCtrl*     CreateTextItem(char *pString, uint32 commandID, int helpID, LTIntPt pos = kDefaultPos, LTBOOL bFixed = LTFALSE, int nFont = -1);

	CLTGUIListCtrl*		CreateList(LTIntPt pos, uint16 nHeight, LTBOOL bUseArrows = LTFALSE, uint16 nArrowOffset = 0);

    CLTGUICycleCtrl*     CreateCycle(int stringID, int helpID, int nHeaderWidth, uint8 *pnValue=LTNULL, LTIntPt pos = kDefaultPos, LTBOOL bFixed = LTFALSE, int nFont = -1);
    CLTGUICycleCtrl*     CreateCycle(char *pString, int helpID, int nHeaderWidth, uint8 *pnValue=LTNULL, LTIntPt pos = kDefaultPos, LTBOOL bFixed = LTFALSE, int nFont = -1);

    CLTGUIToggle*     CreateToggle(int stringID, int helpID, int nHeaderWidth, LTBOOL *pbValue=LTNULL, LTIntPt pos = kDefaultPos, LTBOOL bFixed = LTFALSE, int nFont = -1);
    CLTGUIToggle*     CreateToggle(char *pString, int helpID, int nHeaderWidth, LTBOOL *pbValue=LTNULL, LTIntPt pos = kDefaultPos, LTBOOL bFixed = LTFALSE, int nFont = -1);

    CLTGUISlider*     CreateSlider(int stringID,  int helpID, int nHeaderWidth, int nBarWidth, int nBarHeight = -1, 
								int *pnValue = LTNULL, LTIntPt pos = kDefaultPos, LTBOOL bFixed = LTFALSE, int nFont = -1);
    CLTGUISlider*     CreateSlider(char *pString,  int helpID, int nHeaderWidth, int nBarWidth, int nBarHeight = -1, 
								int *pnValue = LTNULL, LTIntPt pos = kDefaultPos, LTBOOL bFixed = LTFALSE, int nFont = -1);

	CLTGUIColumnCtrl* CreateColumnCtrl(uint32 commandID, int helpID, LTIntPt pos = kDefaultPos, LTBOOL bFixed = LTFALSE, int nFont = -1);

	// Calls UpdateData on each control in the screen
    virtual void            UpdateData(LTBOOL bSaveAndValidate=LTTRUE);

protected:

	// Handle input
    virtual LTBOOL   OnUp();
    virtual LTBOOL   OnDown();
    virtual LTBOOL   OnLeft();
    virtual LTBOOL   OnRight();
    virtual LTBOOL   OnEnter();

	// Gets the index of the control that is under the specific screen point.
	// Returns FALSE if there isn't one under the specified point.
    LTBOOL       GetControlUnderPoint(int xPos, int yPos, uint16 *pnIndex);

protected:

	void			RemoveAll(LTBOOL bDelete = LTTRUE);

	void			CreateBack(LTBOOL bOK = LTFALSE, LTBOOL bReturn = LTFALSE);

	virtual void	CreateInterfaceSFX();
	virtual void	RemoveInterfaceSFX();

	CBaseScaleFX*	CreateScaleFX(char *szFXName);

	void			CreateLightFX(char *szFXName);
	void			CreateCharFX(INT_CHAR *pChar);
	void			CreateAttachFX(INT_ATTACH *pAttach);
	virtual void	ClearAttachFX();

	void			SetPolyRenderState();
	void			InitPoly(LTPoly_GT4* pPoly, LTIntPt pos, HTEXTURE hTex);
	void			ScalePoly(LTPoly_GT4* pPoly, LTIntPt pos, HTEXTURE hTex);

protected:

    LTBOOL			m_bInit;
    LTBOOL			m_bBuilt;
	LTBOOL			m_bVisited;

	
	CScreenMgr*		m_pScreenMgr;

	int				m_nScreenID;		// The ID of this screen
	int				m_nContinueID;		// The ID of the screen to show when continue is clicked

	//title stuff
	CUIPolyString*	m_pTitleString;		// The title string
    LTIntPt			m_TitlePos;
	uint8			m_TitleFont;
	uint8			m_TitleSize;
    uint32			m_TitleColor;

	LTBOOL			m_bHaveFocus;

	LTBOOL			m_bHaveLights;

	// Array of SFX owned by this screen
	SFXArray		m_SFXArray;

	// Array of free controls that this screen owns
	ControlArray		m_controlArray;

	int				m_nItemSpacing;

	CLTGUICtrl*		m_pCaptureCtrl;

    LTRect          m_PageRect;
	int				m_nAlignment;
    uint32			m_SelectedColor;
    uint32			m_NonSelectedColor;
    uint32			m_DisabledColor;


    uint32          m_dwCurrHelpID;

	static CUIFormattedPolyString* s_pHelpStr;

	CLTGUITextCtrl	*m_pNext;

	LTBOOL	m_bBack;
	
	static	CLTGUIButton	s_BackArrow;

    static  LTBOOL      s_bReadLayout;
    static  LTRect      s_HelpRect;
    static  uint8		s_HelpSize;
    static  uint16		s_HelpWidth;
    static  LTIntPt     s_BackPos;
    static  LTIntPt     s_NextPos;


	CBaseScaleFX	m_CharSFX;
	int				m_nNumAttachments;
	AttachmentData	m_aAttachment[MAX_INT_ATTACHMENTS];

	LTIntPt			m_nextPos;

	LTFLOAT			m_fLastScale;
    LTBOOL			m_bSelectFXCenter;

	uint16			m_nSelection;
	uint16			m_nOldSelection;
	uint16			m_nLMouseDownItemSel;
	uint16			m_nRMouseDownItemSel;

	ChainFXList		m_Chains;

    static LTVector		s_vPos; 
	static LTVector		s_vU; 
	static LTVector		s_vR; 
	static LTVector		s_vF;
    static LTRotation	s_rRot;


};

inline  CLTGUITextCtrl* CBaseScreen::AddTextItem(int stringID, uint32 commandID, int helpID, LTIntPt pos, LTBOOL bFixed, int nFont)
{
	CLTGUITextCtrl* pCtrl = CreateTextItem(stringID, commandID, helpID,  pos, bFixed, nFont);
	if (pCtrl)
		AddControl(pCtrl);
	return pCtrl;
}
inline  CLTGUITextCtrl* CBaseScreen::AddTextItem(char *pString, uint32 commandID, int helpID, LTIntPt pos, LTBOOL bFixed, int nFont)
{
	CLTGUITextCtrl* pCtrl = CreateTextItem(pString, commandID, helpID,  pos, bFixed, nFont);
	if (pCtrl)
		AddControl(pCtrl);
	return pCtrl;
}

inline CLTGUIListCtrl* CBaseScreen::AddList(LTIntPt pos, uint16 nHeight, LTBOOL bUseArrows, uint16 nArrowOffset)
{
	
	CLTGUIListCtrl* pList = CreateList(pos,nHeight,bUseArrows,nArrowOffset);
    if (pList)
		AddControl(pList);	
	return pList;
}


inline  CLTGUICycleCtrl* CBaseScreen::AddCycle(int stringID, int helpID, int nHeaderWidth, uint8 *pnValue, LTIntPt pos, LTBOOL bFixed, int nFont)
{
	CLTGUICycleCtrl* pCtrl = CreateCycle(stringID, helpID, nHeaderWidth, pnValue,  pos, bFixed, nFont);
	if (pCtrl)
		AddControl(pCtrl);
	return pCtrl;
}
inline  CLTGUICycleCtrl* CBaseScreen::AddCycle(char *pString, int helpID, int nHeaderWidth, uint8 *pnValue, LTIntPt pos, LTBOOL bFixed, int nFont)
{
	CLTGUICycleCtrl* pCtrl = CreateCycle(pString, helpID, nHeaderWidth, pnValue, pos, bFixed, nFont);
	if (pCtrl)
		AddControl(pCtrl);
	return pCtrl;
}


inline  CLTGUIToggle* CBaseScreen::AddToggle(int stringID, int helpID, int nHeaderWidth, LTBOOL *pbValue, LTIntPt pos, LTBOOL bFixed, int nFont)
{
	CLTGUIToggle* pCtrl = CreateToggle(stringID, helpID, nHeaderWidth, pbValue,  pos, bFixed, nFont);
	if (pCtrl)
		AddControl(pCtrl);
	return pCtrl;
}
inline  CLTGUIToggle* CBaseScreen::AddToggle(char *pString, int helpID, int nHeaderWidth, LTBOOL *pbValue, LTIntPt pos, LTBOOL bFixed, int nFont)
{
	CLTGUIToggle* pCtrl = CreateToggle(pString, helpID, nHeaderWidth, pbValue, pos, bFixed, nFont);
	if (pCtrl)
		AddControl(pCtrl);
	return pCtrl;
}


inline  CLTGUISlider* CBaseScreen::AddSlider(int stringID, int helpID, int nHeaderWidth,  int nBarWidth, int nBarHeight, 
												int *pnValue, LTIntPt pos, LTBOOL bFixed, int nFont)
{
	CLTGUISlider* pCtrl = CreateSlider(stringID, helpID, nHeaderWidth, nBarWidth, nBarHeight, pnValue, pos, bFixed, nFont);
	if (pCtrl)
		AddControl(pCtrl);
	return pCtrl;
}
inline  CLTGUISlider* CBaseScreen::AddSlider(char *pString, int helpID, int nHeaderWidth,  int nBarWidth, int nBarHeight, 
												int *pnValue, LTIntPt pos, LTBOOL bFixed, int nFont)
{
	CLTGUISlider* pCtrl = CreateSlider(pString, helpID, nHeaderWidth, nBarWidth, nBarHeight, pnValue, pos, bFixed, nFont);
	if (pCtrl)
		AddControl(pCtrl);
	return pCtrl;
}

inline  CLTGUIColumnCtrl* CBaseScreen::AddColumnCtrl(uint32 commandID, int helpID, LTIntPt pos, LTBOOL bFixed, int nFont)
{
	
	CLTGUIColumnCtrl* pCtrl = CreateColumnCtrl(commandID, helpID, pos, bFixed, nFont);
	if (pCtrl)
		AddControl(pCtrl);
	return pCtrl;
}


#endif // _BASE_SCREEN_H_

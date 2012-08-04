// ----------------------------------------------------------------------- //
//
// MODULE  : LTGUIListCtrl.h
//
// PURPOSE : Control which maintains a scrolling list of other controls.
//
// (c) 2001 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#if !defined(_LTGUILISTCTRL_H_)
#define _LTGUILISTCTRL_H_

#include "LTGUICtrl.h"

class CLTGUITextureButton;

struct CLTGUIListCtrl_create : public CLTGUICtrl_create
{
	CLTGUIListCtrl_create();
	bool bArrows;
	uint32 nArrowOffset;
	LTVector2n vnArrowSz;
	TextureReference hUpNormal;
	TextureReference hUpSelected;
	TextureReference hDownNormal;
	TextureReference hDownSelected;
	bool bAutoSelect; // Automatically handle select and deselects.
};

inline CLTGUIListCtrl_create::CLTGUIListCtrl_create() : 
	bArrows(false), nArrowOffset(0), bAutoSelect(true)
{
};


class CLTGUIListCtrl : public CLTGUICtrl
{
public:
	CLTGUIListCtrl();
	virtual ~CLTGUIListCtrl();

	// Creation
    bool	Create (const CLTGUIListCtrl_create& cs );

	// Enable/Disable the control
    virtual void    Enable ( bool bEnabled );


	// Destroy the control
	void		Destroy ( );

	// Render the control
	virtual void Render();
	virtual void RenderTransition(float fTrans);

	// Handle a keypress
    bool       HandleKeyDown(int key, int rep);

	// Commonly used keyboard messages
    virtual bool   OnUp ( );
    virtual bool   OnDown ( );
    virtual bool   OnLeft ( );
    virtual bool   OnRight ( );
    virtual bool   OnEnter ( );
    
	bool			OnPageUp ( );
	bool			OnPageDown ( );

	// Mouse messages
    virtual bool   OnLButtonDown(int x, int y);
    virtual bool   OnLButtonUp(int x, int y);
    virtual bool   OnRButtonDown(int x, int y);
    virtual bool   OnRButtonUp(int x, int y);
    virtual bool   OnMouseMove(int x, int y);

	// Set the number of pixels between items
	void		SetItemSpacing ( int nSpacing )		{ m_nItemSpacing=nSpacing; m_bNeedsRecalculation = true;}

	// Sets the width of the list's frame, set to 0 to not show the frame
	void		SetFrameWidth(uint8 nFrameWidth);
	void		SetFrame(HTEXTURE hNormalFrame,HTEXTURE hSelectedFrame, uint8 nExpand);

	// Add/Remove controls to the array
	static const uint32	kMaxNumControls;
	static const uint32	kNoSelection;
	uint32		AddControl ( CLTGUICtrl *pControl );
	bool		InsertControl( uint32 nIndex, CLTGUICtrl *pControl );
	void		RemoveControl ( CLTGUICtrl *pControl, bool bDelete = true );
	void		RemoveControl ( uint32 nIndex, bool bDelete = true );
	void		RemoveAll(bool bDelete = true );
	uint32		GetNumControls ( )							{ return m_controlArray.size(); }
	ControlArray&	GetControlArray( )						{ return m_controlArray; }

	CLTGUICtrl	*GetControl ( uint32 nIndex ) const;
	uint32		GetIndex(CLTGUICtrl	*pCtrl) const;

	//when list is selected/deselected
	void		OnSelChange();

	// Update data
    virtual void    UpdateData ( bool bSaveAndValidate = true );


	//select/deslect items in list
	uint32		SetSelection( uint32 nIndex );
	void		ClearSelection();


	uint32		GetSelectedIndex ( ) const					{ return m_nCurrentIndex; }
	CLTGUICtrl	*GetSelectedControl ( )	const			{ return GetControl(m_nCurrentIndex); }
	
	uint32		PreviousSelection();
	uint32		NextSelection();

	uint32		GetLastShown ( );
	uint32		GetStartIndex ( )					{ return m_nFirstShown; }
	void		SetStartIndex ( uint32 nIndex )		{ m_nFirstShown=nIndex; m_bNeedsRecalculation = true;}

	// Gets the index of the control that is under the specific screen point.
	// Returns FALSE if there isn't one under the specified point.
	CLTGUICtrl	*GetControlUnderPoint(int xPos, int yPos, uint32 *pnIndex);

	virtual void SetIndent(LTVector2n indent)	{ m_indent = indent; m_bNeedsRecalculation = true;}
	virtual void    SetBasePos (const LTVector2n& pos);
	virtual void	SetScale(const LTVector2& vfScale);
	virtual void	SetSize(const LTVector2n& sz);

	void		SetScrollWrap(bool bWrap) {m_bScrollWrap = bWrap;}
	void		SetScrollByPage(bool bByPage) {m_bScrollByPage = bByPage;}

	void		CalculatePositions();

    virtual const char* GetHelpID();

	// free texture memory by flushing any texture strings owned by the control
	virtual void	FlushTextureStrings();

	// rebuild any texture strings owned by the control
	virtual void	RecreateTextureStrings();

	//swap the positions of two items
	void	SwapItems(uint32 nIndex1,uint32 nIndex2);


protected:
	
	void	SetRenderState();
	void	SelectionChanged();


protected:
	int			m_nItemSpacing;				// The number of pixels between items
	uint32		m_nCurrentIndex;			// Selected item
	uint32		m_nFirstShown;				// First item displayed on the screen
	uint32		m_nLastShown;				// First item displayed on the screen
	uint32		m_nLBDownSel;				// The control index that is selected from the current left button message
	uint32		m_nRBDownSel;				// The control index that is selected from the current right button message


	LTVector2n		m_indent;

	uint8		m_nFrameWidth;
	LT_POLYG4	m_Frame[4];

	CLTGUIFrame	m_TexFrame;
	TextureReference m_hFrame[2];			// normal and highlighted textures

	bool		m_bNeedsRecalculation;
	bool		m_bScrollWrap;
	bool		m_bScrollByPage;			// scroll by pages rather than by individual items

	ControlArray	m_controlArray;

	CLTGUITextureButton*	m_pUp;
	CLTGUITextureButton*	m_pDown;
	uint32			m_nArrowOffset;

	// Automatically select/deselect items.
	bool		m_bAutoSelect;

};

#endif // !defined(_LTGUILISTCTRL_H_)
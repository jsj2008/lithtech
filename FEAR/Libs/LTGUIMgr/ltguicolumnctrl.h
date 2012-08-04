// ----------------------------------------------------------------------- //
//
// MODULE  : LTGUIColumnCtrl.h
//
// PURPOSE : Control to display columns of text
//
// (c) 2001 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#if !defined(_LTGUICOLUMNCTRL_H_)
#define _LTGUICOLUMNCTRL_H_

#include "LTGUITextCtrl.h"


typedef std::vector<CLTGUITextCtrl*, LTAllocator<CLTGUITextCtrl*, LT_MEM_TYPE_CLIENTSHELL> > TextControlArray;

class CLTGUIColumnCtrl : public CLTGUICtrl
{
public:
	CLTGUIColumnCtrl();
	virtual ~CLTGUIColumnCtrl();

	// Create the control
    virtual bool   Create(const CFontInfo& Font, const CLTGUICtrl_create& cs);

	// Destroys the control
	virtual void	Destroy ( );

	virtual	void	SetBasePos( const LTVector2n& pos );
	virtual void	SetScale( const LTVector2& vfScale );
	virtual void	SetSize( const LTVector2n& sz );

	virtual void    SetColors(uint32 argbSelected, uint32 argbNormal, uint32 argbDisabled);

	// Enable/Disable the control
    virtual void    Enable ( bool bEnabled );

	virtual void	AllowColumnSelection(bool bAllow) { m_bAllowColumnSelection = bAllow; }


	// Adds a column to the control
	// nWidth	  - Width of the column
	// hString	  - The initial text for the column
	//returns column index
	static const uint8	kMaxNumColumns;
    uint8          AddColumn(const wchar_t *pString, uint32 nWidth, bool bClip = false);
	uint8          AddColumn(const wchar_t *pString, uint32 nWidth, bool bClip, const CLTGUICtrl_create& cs);

	// Gets a string at a specific column index.  This returns a copy (new handle).
	CLTGUITextCtrl*			GetColumn(uint8 nColumnIndex) const;

	// Sets a string for a column.  This copies the string from hString to an internal string.
	void			SetString(uint8 nColumnIndex, const wchar_t *pString);

	void			RemoveColumn(uint8 nIndex);			// Removes a column
	void			RemoveAllColumns();					// Removes all of the columns

	void			ShowColumn(uint8 nIndex, bool bShow);	// Hide/show a column


	// Return the number of columns
	uint8			GetNumColumns()						{ ASSERT(m_columnArray.size() < 255 ); return ( uint8 )m_columnArray.size(); }

	// Render the control
	void	Render ();
	void	RenderTransition ( float fTrans );


	// Set the font
	void            SetFont (const CFontInfo& Font, bool bSetForAll = false);
	void            SetFontHeight (uint32 nFontHeight, bool bSetForAll = false);

	// Handle the Enter key being pressed
	virtual bool   OnLeft ( );
	virtual bool   OnRight ( );
	virtual bool   OnEnter ( );

	// Mouse messages
	virtual bool   OnLButtonDown(int x, int y);
	virtual bool   OnLButtonUp(int x, int y);
	virtual bool   OnRButtonDown(int x, int y);
	virtual bool   OnRButtonUp(int x, int y);
	virtual bool   OnMouseMove(int x, int y);

	// Gets the index of the control that is under the specific screen point.
	// Returns FALSE if there isn't one under the specified point.
	CLTGUICtrl	*GetControlUnderPoint(int xPos, int yPos, uint8 *pnIndex);


	virtual const char* GetHelpID();

	// free texture memory by flushing any texture strings owned by the control
	virtual void	FlushTextureStrings();

	// rebuild any texture strings owned by the control
	virtual void	RecreateTextureStrings();


protected:
	virtual void	OnSelChange();
	virtual void	ResetColumns();
	virtual uint8	SelectColumn(uint8 nIndex);


protected:

	CFontInfo	m_Font;				// The font for this control

	TextControlArray	m_columnArray;	// The array of columns

	bool		m_bAllowColumnSelection;
	uint8		m_nCurrentIndex;			// Selected item
	uint8		m_nLBDownSel;				// The control index that is selected from the current left button message
	uint8		m_nRBDownSel;				// The control index that is selected from the current left button message



};


#endif // !defined(_LTGUICOLUMNCTRL_H_)
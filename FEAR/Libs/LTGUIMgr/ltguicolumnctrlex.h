// ----------------------------------------------------------------------- //
//
// MODULE  : ltguicolumnctrlex.h
//
// PURPOSE : Control to display columns of mixed types of controls
//
// CREATED : 12/15/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __LTGUICOLUMNCTRLEX_H__
#define __LTGUICOLUMNCTRLEX_H__


#include "LTGUICtrl.h"


class CLTGUIColumnCtrlEx : public CLTGUICtrl
{
public:
	CLTGUIColumnCtrlEx();
	virtual ~CLTGUIColumnCtrlEx();

	// Create the control
    virtual void   Create(const CLTGUICtrl_create& cs);

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
	uint8          AddColumn(CLTGUICtrl* pCtrl);
	uint8          AddTextColumn(const wchar_t *pString, uint32 nWidth, bool bClip = false, eTextAlign align = kLeft);
	uint8          AddTextColumn(const wchar_t *pString, uint32 nWidth, bool bClip, const CLTGUICtrl_create& cs);

	uint8			AddIconColumn(const char *pTex, uint32 nWidth, const LTVector2n& vSize );

	inline bool		SetColumnWidth( uint8 nColumnIndex, uint32 nWidth );

	// Gets a string at a specific column index.  This returns a copy (new handle).
	CLTGUICtrl*		GetColumn(uint8 nColumnIndex) const;

	void			RemoveColumn(uint8 nIndex);			// Removes a column
	void			RemoveAllColumns();					// Removes all of the columns

	void			ShowColumn(uint8 nIndex, bool bShow);	// Hide/show a column


	// Return the number of columns
	uint8			GetNumColumns()						{ ASSERT(m_columnArray.size() < 255 ); return ( uint8 )m_columnArray.size(); }

	// Render the control
	void	Render ();
	void	RenderTransition ( float fTrans );

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

	// Set the font
	void            SetFont (const CFontInfo& Font);

	virtual void	ResetColumns();

	void			SetTextIndent( int32 nIndent ) { m_nTextIndent = nIndent; }
	int32			GetTextIndent() { return m_nTextIndent; }

protected:
	virtual void	OnSelChange();
	virtual uint8	SelectColumn(uint8 nIndex);


protected:

	ControlArray	m_columnArray;

	CFontInfo	m_Font;

	bool		m_bAllowColumnSelection;
	uint8		m_nCurrentIndex;			// Selected item
	uint8		m_nLBDownSel;				// The control index that is selected from the current left button message
	uint8		m_nRBDownSel;				// The control index that is selected from the current left button message

	int32		m_nTextIndent;				// The indentation of the text

};

/*
 *	Inlines
 */

 // sets the width of a column
 bool CLTGUIColumnCtrlEx::SetColumnWidth( uint8 nColumnIndex, uint32 nWidth )
 {
	 if (nColumnIndex > kMaxNumColumns || nColumnIndex >= m_columnArray.size())
		 return false;
	  
	 CLTGUICtrl* pCtrlThis = m_columnArray[nColumnIndex];
	 if( !pCtrlThis )
		 return false;

	 if( pCtrlThis->ShouldIndent() )
		 nWidth -= (m_nTextIndent + m_nTextIndent);

	 pCtrlThis->SetBaseWidth( nWidth );

	 return true;
}

#endif  // __LTGUICOLUMNCTRLEX_H__

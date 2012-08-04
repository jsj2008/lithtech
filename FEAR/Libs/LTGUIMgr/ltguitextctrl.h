// ----------------------------------------------------------------------- //
//
// MODULE  : LTGUITextCtrl.h
//
// PURPOSE : Simple text control which may be used as a menu item.
//
// (c) 2001-2003 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //


#if !defined(_LTGUITEXTCTRL_H_)
#define _LTGUITEXTCTRL_H_

#include "ilttexturestring.h"

#include "LTGUICtrl.h"

// This control displays a string of text which fades out upon being unselected.
class CLTGUITextCtrl : public CLTGUICtrl
{
public:
	CLTGUITextCtrl();
	virtual ~CLTGUITextCtrl();

	// Create the control
    bool           Create (	const wchar_t *pText, const CFontInfo& Font, const CLTGUICtrl_create& cs, const bool bEllipsis = false );

	// Destroys the control
	virtual void	Destroy ( ); 

	virtual	void	SetBasePos( const LTVector2n& pos );
	virtual void	SetScale( const LTVector2& vfScale );
	virtual void	SetSize( const LTVector2n& sz );

	virtual void	SetWordWrap(bool bWrap);
	virtual void	SetClipping(bool bClip) {m_bClip = bClip;}

	virtual void	SetDropShadow(uint8 drop);

	virtual void	SetIndent(uint32 nIndent);

	virtual void	SetString(const wchar_t *pText, bool bEllipsis = false);

	// Render the control
	virtual void Render();
	virtual void RenderTransition(float fTrans);

	// Set the font, note this always expects an unscaled height
    virtual bool	SetFont ( const CFontInfo& Font );
	virtual bool	SetFontHeight (uint32 nFontHeight);

	// Commonly used keyboard messages
    virtual bool   OnEnter ( );
    virtual bool   OnLButtonUp(int x, int y)
	{
		LTUNREFERENCED_PARAMETER(x); LTUNREFERENCED_PARAMETER(y);
		return OnEnter();
	}

	// free texture memory by flushing any texture strings owned by the control
	virtual void	FlushTextureStrings();

	// rebuild any texture strings owned by the control
	virtual void	RecreateTextureStrings();

	virtual const wchar_t* GetString() const;

	void SetAlignment(eTextAlign align);

	bool	GetExtents(LTRect2n& rExtents);

	// this is used by parent controls that can indent child controls
	virtual bool	ShouldIndent() { return true; }

protected:

	CLTGUIString	m_Text;
	uint32			m_nBaseFontSize;		// The font size before scaling

    uint32      m_nIndent;
	bool		m_bWordWrap;
	bool		m_bClip;

};

#endif // _LTGUITEXTCTRL_H_
// ----------------------------------------------------------------------- //
//
// MODULE  : SkillControl.h
//
// PURPOSE : GUI control to display of objectives.
//
// (c) 2001 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //


#if !defined(_SKL_CTRL_H_)
#define _SKL_CTRL_H_

#include "LTGuiMgr.h"
#include "LTPoly.h"
#include "SkillsButeMgr.h"



class CSkillCtrl : public CLTGUICtrl
{
public:

	CSkillCtrl();
	virtual ~CSkillCtrl();



    LTBOOL			Create (eSkill skl, uint32 nCommandID, CUIFont *pFont, uint8 nFontSize, CLTGUICommandHandler *pCommandHandler, uint16 nNameGap);
	virtual void	Destroy();

    virtual void    SetBasePos ( LTIntPt pos );
	virtual void	SetScale(float fScale);

	void			SetSkillLevel(eSkillLevel level);

	// Render the control
	virtual void	Render ();

	// Width/Height calculations
	virtual uint16	GetWidth ( )						{ return m_nWidth; }
	virtual uint16	GetHeight ( )						{ return m_nHeight; }

	// Commonly used keyboard messages
    virtual LTBOOL   OnEnter ( );
    virtual LTBOOL   OnLButtonUp(int x, int y) {return OnEnter();}


protected:
	void			SetRenderState();
	void			InitPolies();
	void			ScalePolies();

protected:

	eSkill      m_Skill;
	eSkillLevel m_Level;

	CUIFont*	m_pFont;				// The font for this control
	uint8		m_nFontSize;			// The scaled font size to use.
	uint8		m_nBaseFontSize;		// The font size before scaling

	uint16		m_nNameGap;
	uint16		m_nWidth;
	uint16		m_nHeight;

	LTPoly_GT4	m_Poly[kNumSkillLevels];

	static HTEXTURE	m_hChecked;
	static HTEXTURE	m_hUnchecked;

	CUIFormattedPolyString*		m_pLabel;

	// Receives a message when the "enter" key is pressed.
	CLTGUICommandHandler	*m_pCommandHandler;

};

#endif // _OBJ_CTRL_H_
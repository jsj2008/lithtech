// ----------------------------------------------------------------------- //
//
// MODULE  : BaseMenu.h
//
// PURPOSE : Base class for in-game menus
//
// (c) 2001 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#if !defined(_BASE_MENU_H_)
#define _BASE_MENU_H_

#include "LTGUIMgr.h"
#include "LTPoly.h"


enum eMenuID
{
#define INCLUDE_AS_ENUM
#include "MenuEnum.h"
#undef INCLUDE_AS_ENUM
};

class CSubMenu : public CLTGUIWindow
{
public:
	CSubMenu();
	virtual LTBOOL Init(HTEXTURE hFrame,HTEXTURE hFrameTip, LTIntPt size);
	
    virtual LTBOOL   HandleKeyUp(int key);
	// Render the control
	virtual void Render ();

	virtual void SetBasePos ( LTIntPt pos );
	virtual void SetScale(float fScale);

protected:

	virtual void UpdateFrame();

	HTEXTURE	m_hFrame;
	HTEXTURE	m_hFrameTip;
	LTPoly_GT4	m_Poly[2];

};

enum eMenuCmds
{
	MC_NONE = 0,
	MC_LEFT,
	MC_RIGHT,
	MC_CLOSE,
	MC_UPDATE,
	MC_CUSTOM
};

class CBaseMenu : public CLTGUIWindow
{
public:
	CBaseMenu();
	virtual ~CBaseMenu();

	virtual LTBOOL	Init();
	virtual void	Term();

	virtual void OnFocus(LTBOOL bFocus);

    virtual LTBOOL  OnUp ( );
    virtual LTBOOL  OnDown ( );
	virtual	LTBOOL	OnMouseMove(int x, int y);
    virtual LTBOOL	OnEscape () { return LTFALSE; }

	// Handle a command
    virtual uint32 OnCommand(uint32 nCommand, uint32 nParam1, uint32 nParam2);


	eMenuID	GetMenuID() {return m_MenuID;}

	// Render the control
	virtual void Render ();

	virtual void SetBasePos ( LTIntPt pos );
	virtual void SetScale(float fScale);

	virtual const char *GetTitle() {return m_Title.GetString()->GetText();}

protected:

	void		SetTitle(int stringID);

	uint16		AddControl (int stringID, uint32 commandID, LTBOOL bStatic = LTFALSE);
	uint16		AddControl (char *pString, uint32 commandID, LTBOOL bStatic = LTFALSE);

	void UpdateFrame();

protected:

    static	LTIntPt		s_Size;
    static	uint16		s_Pos;

	static	HTEXTURE	s_Frame;
	static	HTEXTURE	s_FrameTip;
	static	HTEXTURE	s_Up;
	static	HTEXTURE	s_UpH;
	static	HTEXTURE	s_Down;
	static	HTEXTURE	s_DownH;

	LTPoly_GT4	m_Poly[2];

    uint8		m_FontSize;
    uint8		m_FontFace;
    uint8		m_TitleFontSize;
    uint8		m_TitleFontFace;

	LTIntPt	m_Indent;
    uint32	m_SelectedColor;
    uint32	m_NonSelectedColor;
    uint32	m_DisabledColor;


	eMenuID			m_MenuID;
	LTIntPt			m_nextPos;
	CLTGUITextCtrl	m_Title;
	CLTGUITextCtrl	m_Resume;
	CLTGUIListCtrl	m_List;



};

#endif //!defined(_BASE_MENU_H_)

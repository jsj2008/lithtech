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

class CMenuMgr;

enum eMenuID
{
#define INCLUDE_AS_ENUM
#include "MenuEnum.h"
#undef INCLUDE_AS_ENUM
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

	virtual bool	Init( CMenuMgr& menuMgr );
	virtual void	Term();

	virtual void OnFocus(bool bFocus);

    virtual bool  OnUp ( );
    virtual bool  OnDown ( );
	virtual	bool	OnMouseMove(int x, int y);
    virtual bool	OnEscape () { return false; }

	void			ShowFrame( bool bValue ) { m_bShowFrame = bValue; }
	bool			IsShowFrame( ) const { return m_bShowFrame; }

	// Handle a command
    virtual uint32 OnCommand(uint32 nCommand, uint32 nParam1, uint32 nParam2);


	eMenuID	GetMenuID() {return m_MenuID;}

	// Render the control
	virtual void Render ();

	virtual void	SetBasePos( const LTVector2n& pos );
	virtual void	SetScale( const LTVector2& vfScale);

	virtual HRECORD	GetMenuRecord( );

	virtual void FreeSharedTextures();

	LTVector2n const&	GetDefaultPos( ) const { return m_vDefaultPos; }

protected:

	using		CLTGUIWindow::AddControl;
	uint16		AddControl (const char* szStringID, uint32 commandID, bool bStatic = false);
	uint16		AddControl (const wchar_t *pString, uint32 commandID, bool bStatic = false);

	void UpdateFrame();

protected:

    LTVector2n	m_vDefaultSize;
    LTVector2n	m_vDefaultPos;

	TextureReference	m_Frame;
	TextureReference	m_Up;
	TextureReference	m_UpH;
	TextureReference	m_Down;
	TextureReference	m_DownH;

	LTPoly_GT4	m_Poly;

    uint8		m_FontSize;
	std::string	m_FontFace;

	LTVector2n	m_Indent;
    uint32	m_SelectedColor;
    uint32	m_NonSelectedColor;
    uint32	m_DisabledColor;


	eMenuID			m_MenuID;
	LTVector2n			m_nextPos;
	CLTGUITextCtrl	m_Resume;
	CLTGUIListCtrl	m_List;

	CMenuMgr*		m_pMenuMgr;

	bool	m_bShowFrame;
};

#endif //!defined(_BASE_MENU_H_)

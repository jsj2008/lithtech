// ----------------------------------------------------------------------- //
//
// MODULE  : LTGUICtrl.h
//
// PURPOSE : Base clase for controls
//
// (c) 2001 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#if !defined(_LTGUICTRL_H_)
#define _LTGUICTRL_H_

#include "iltclient.h"
#include "ltbasedefs.h"


#define	LGCS_NORMAL		0	// control states
#define LGCS_SELECTED	1
#define LGCS_DISABLED	2

struct CLTGUICtrl_create
{
	CLTGUICtrl_create();
	uint32 nCommandID;
	const char* szHelpID;
	CLTGUICommandHandler *pCommandHandler;
	uint32 nParam1;
	uint32 nParam2;
	LTRect2n rnBaseRect;
	bool		bGlowEnable;
	float		fGlowAlpha;
	LTVector2	vGlowSize;

};

inline CLTGUICtrl_create::CLTGUICtrl_create() : 
	nCommandID(0), szHelpID(""), pCommandHandler(NULL), nParam1(0), nParam2(0),
	bGlowEnable(true), fGlowAlpha(0.25f), vGlowSize(1.2f,1.2f)
{
};

class CLTGUICtrl : public CLTGUICommandHandler
{
public:
	CLTGUICtrl();
	virtual ~CLTGUICtrl();

	// All controls should have a create function.
    virtual void    Create(const CLTGUICtrl_create& cs)
					{
						m_nCommandID = cs.nCommandID;
						m_szHelpID = cs.szHelpID;
						m_pCommandHandler = cs.pCommandHandler;
						m_nParam1 = cs.nParam1;
						m_nParam2 = cs.nParam2;
						m_rnBaseRect = cs.rnBaseRect;
						m_bCreated = true;
						m_vfScale.Init(1.0f,1.0f);
						SetBasePos(cs.rnBaseRect.m_vMin);
					}

	// Destruction
	virtual void	Destroy() {}

	// Render the control
	virtual void	Render ( ) = 0;
	virtual void	RenderTransition (float fTrans ) = 0;

	// free texture memory by flushing any texture strings owned by the control
	virtual void	FlushTextureStrings() = 0;

	// rebuild any texture strings owned by the control
	virtual void	RecreateTextureStrings() = 0;

	// Width/Height calculations
	virtual LTRect2f	GetRect ( ) const { return  m_rfRect; }
	virtual float		GetWidth ( ) const  { return  m_rfRect.GetWidth(); }
	virtual float		GetHeight ( ) const  { return  m_rfRect.GetHeight(); }

	virtual uint32	GetBaseWidth ( ) const  { return m_rnBaseRect.GetWidth(); }
	virtual uint32	GetBaseHeight ( ) const  { return m_rnBaseRect.GetHeight();  }
	virtual void	SetBaseWidth ( uint32 nWidth );
	
	virtual void	SetBasePos( const LTVector2n& pos );
    virtual LTVector2n GetBasePos ( ) const  { return m_rnBaseRect.m_vMin; }
    virtual LTVector2 GetPos ( ) const { return m_rfRect.m_vMin; }

	virtual void	SetSize( const LTVector2n& sz );

	virtual void		SetScale( const LTVector2& vfScale);
	virtual LTVector2	GetScale() const {return m_vfScale;}

	
	// Commonly used input messages
    virtual bool  OnLeft ( ) {return false;}
    virtual bool  OnRight ( ) {return false;}
    virtual bool  OnUp ( ) {return false;}
    virtual bool  OnDown ( ) {return false;}
    virtual bool  OnEnter ( ) {return false;}

	//check a point and see if it is over this control
	virtual bool  IsOnMe(int x, int y) { if (!m_bVisible) return false; return IsOnMe( LTVector2((float)x,(float)y) );}
	virtual bool  IsOnMe(LTVector2 pt) { if (!m_bVisible) return false; return m_rfRect.Contains(pt);}

	// Handles gamepad input
    virtual bool   HandleInterfaceCommand(int command) { LTUNREFERENCED_PARAMETER(command); return false;}

	// Handles a key press.  Returns FALSE if the key was not processed through this method.
	// Left, Up, Down, Right, and Enter are automatically passed through OnUp(), OnDown(), etc.
    virtual bool   HandleKeyDown(int key, int rep)
	{
		LTUNREFERENCED_PARAMETER(key);
		LTUNREFERENCED_PARAMETER(rep);
		return false;
	}
	virtual bool   HandleKeyUp(int key)
	{
		LTUNREFERENCED_PARAMETER(key);
		return false;
	}


	// Handles a character input.
    virtual bool   HandleChar(wchar_t c) { LTUNREFERENCED_PARAMETER(c); return false;}

	// Mouse messages
    virtual bool   OnLButtonDown(int x, int y) 
	{	
		LTUNREFERENCED_PARAMETER(x);
		LTUNREFERENCED_PARAMETER(y);
		return false;
	}
    virtual bool   OnLButtonUp(int x, int y)
	{	
		LTUNREFERENCED_PARAMETER(x);
		LTUNREFERENCED_PARAMETER(y);
		return false;
	}
    virtual bool   OnLButtonDblClick(int x, int y)
	{	
		LTUNREFERENCED_PARAMETER(x);
		LTUNREFERENCED_PARAMETER(y);
		return false;
	}
    virtual bool   OnRButtonDown(int x, int y)
	{	
		LTUNREFERENCED_PARAMETER(x);
		LTUNREFERENCED_PARAMETER(y);
		return false;
	}
    virtual bool   OnRButtonUp(int x, int y)
	{	
		LTUNREFERENCED_PARAMETER(x);
		LTUNREFERENCED_PARAMETER(y);
		return false;
	}
    virtual bool   OnRButtonDblClick(int x, int y)
	{	
		LTUNREFERENCED_PARAMETER(x);
		LTUNREFERENCED_PARAMETER(y);
		return false;
	}
    virtual bool   OnMouseMove(int x, int y)
	{	
		LTUNREFERENCED_PARAMETER(x);
		LTUNREFERENCED_PARAMETER(y);
		return false;
	}
	virtual bool   OnMouseWheel(int x, int y, int zDelta)
	{	
		LTUNREFERENCED_PARAMETER(x);
		LTUNREFERENCED_PARAMETER(y);
		LTUNREFERENCED_PARAMETER(zDelta);
		return false;
	}

	// Handle a command
    virtual void    OnCommand ( uint32 nCommandID ) { LTUNREFERENCED_PARAMETER(nCommandID); }

	// Select a control
    void            Select(bool bSelected);

	// Enable/Disable the control
    virtual void    Enable ( bool bEnabled );

	// Show/Hide the control
    virtual void    Show ( bool bShow )       { m_bVisible=bShow; }

	// Update data
    virtual void    UpdateData ( bool bSaveAndValidate = true ) { LTUNREFERENCED_PARAMETER(bSaveAndValidate); }

	virtual	int		GetState();
    virtual	uint32	GetCurrentColor();

	// Sets the colors to fade to and from
	virtual void    SetColors(uint32 argbSelected, uint32 argbNormal, uint32 argbDisabled) 
						{ m_argbSelected = argbSelected; m_argbNormal = argbNormal; m_argbDisabled = argbDisabled;}

	// if the control only needs one color
	virtual void    SetColor(uint32 argbColor) { SetColors(argbColor,argbColor,argbColor) ;}
	

	// Access to member variables
    bool          IsSelected()                    { return m_bSelected; }
    bool          IsEnabled()                     { return m_bEnabled && IsVisible(); }
    bool          IsNormal()                      { return(!IsSelected() && IsEnabled()); }
    bool          IsDisabled()                    { return(!IsSelected() && !IsEnabled()); }
    bool          IsVisible()						{ return m_bVisible; }

    virtual void    SetCommandID(uint32 id)            { m_nCommandID = id; }
    virtual uint32  GetCommandID()                     { return m_nCommandID; }

    void            SetParam1(uint32 n) { m_nParam1 = n; }
    uint32          GetParam1() const { return(m_nParam1); }

	virtual void	SetCommandHandler(CLTGUICommandHandler *pCommandHandler)
									{ m_pCommandHandler = pCommandHandler; }

    void            SetParam2(uint32 n) { m_nParam2 = n; }
    uint32          GetParam2() const { return(m_nParam2); }

    virtual void		SetHelpID(const char* id)       { m_szHelpID = id; }
    virtual const char*	GetHelpID()                     { return m_szHelpID; }

	virtual bool	IsCreated() const { return m_bCreated; }

	// this is used by parent controls that can indent child controls
	virtual bool	ShouldIndent() { return false; }

protected:
	// Called by Select().  The IsSelected() will return the newly selected state.
	virtual void	OnSelChange()					{}

protected:
    uint32          m_nCommandID;          // The command which is sent when "enter" is pressed
    const char*     m_szHelpID;
    uint32          m_nParam1;             // Extra params
    uint32          m_nParam2;             // Extra params

    bool			m_bCreated;

    LTRect2n		m_rnBaseRect;
	LTVector2		m_vfScale;

	LTRect2f		m_rfRect;

    bool          m_bSelected;
    bool          m_bEnabled;
    bool          m_bVisible;

    uint32	        m_argbSelected;        // The selected color
    uint32	        m_argbNormal;          // The non-selected color
    uint32	        m_argbDisabled;        // The disabled color

	CLTGUICommandHandler *m_pCommandHandler;
};

inline int CLTGUICtrl::GetState()
{
	if (IsSelected()) return(LGCS_SELECTED);
	if (IsDisabled()) return(LGCS_DISABLED);
	return(LGCS_NORMAL);
}

inline	uint32	CLTGUICtrl::GetCurrentColor()
{
	if (IsSelected()) return(m_argbSelected);
	if (IsDisabled()) return(m_argbDisabled);
	return(m_argbNormal);
}

typedef std::vector<CLTGUICtrl*, LTAllocator<CLTGUICtrl *, LT_MEM_TYPE_CLIENTSHELL> > ControlArray;

#endif // _LTGUICTRL_H_
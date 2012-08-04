// ----------------------------------------------------------------------- //
//
// MODULE  : LTGUICycleCtrl.h
//
// PURPOSE : Control which can cycle through a set of strings based on 
//				user input.
//
// (c) 2001 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#if !defined(_LTGUICYCLECTRL_H_)
#define _LTGUICYCLECTRL_H_

#include "LTGUICtrl.h"

struct CLTGUICycleCtrl_create : public CLTGUICtrl_create
{
	CLTGUICycleCtrl_create();
	uint32 nHeaderWidth;
	uint8 *pnValue;
};

inline CLTGUICycleCtrl_create::CLTGUICycleCtrl_create() : 
		nHeaderWidth(0), pnValue(NULL)
{
};


class CLTGUICycleCtrl;

//the cycle callback is used when some external code is needed to determine the next valid index
// returns the index of the next selection based on the current index and whether an increment
// or decrement is requested
typedef uint8 (*CycleCallBackFn)(CLTGUICycleCtrl* pCycle, uint8 nCurIndex, bool bIncrement);

class CLTGUICycleCtrl : public CLTGUICtrl
{
public:
	CLTGUICycleCtrl();
	virtual ~CLTGUICycleCtrl();

	// Create the control
    bool           Create ( const wchar_t *pHeader, const CFontInfo& Font, const CLTGUICycleCtrl_create& cs);

	// Destroys the control
	virtual void	Destroy ( ); 

	// Update data
    virtual void    UpdateData(bool bSaveAndValidate);

	// Add more strings to the control.  These are cycled when OnLeft() and OnRight() are called
	static const uint8	kMaxNumStrings;
	static const uint8	kNoSelection;
	virtual void	AddString(const wchar_t *pText);

	CLTGUIString* 	GetString(uint8 nIndex);		// Return a string at a specific index
	void			RemoveString(uint8 nIndex);		// Remove a string at a specific index
	void			RemoveAll();					// Remove all strings

	// Return the number of strings
	uint8			GetNumStrings()						{ ASSERT( m_StringArray.size() < 255 ); return ( uint8 )m_StringArray.size(); }

	// Sets/Get the currently selected index
	uint8			GetSelIndex()						{ return m_nSelIndex; }
	virtual void	SetSelIndex(uint8 nIndex);

	void	SetCycleCallback(CycleCallBackFn pCallback) {m_pCallback = pCallback;}

	// Render the control
	virtual void	Render();
	virtual void	RenderTransition(float fTrans);

    virtual bool   OnEnter() {return false;}
    virtual bool   OnLeft();
    virtual bool   OnRight();
    virtual bool   OnLButtonUp(int x, int y) 
	{
		LTUNREFERENCED_PARAMETER(x); LTUNREFERENCED_PARAMETER(y);
		return OnRight();
	}
    virtual bool   OnRButtonUp(int x, int y) 
	{
		LTUNREFERENCED_PARAMETER(x); LTUNREFERENCED_PARAMETER(y);
		return OnLeft();
	}

	virtual	void	SetBasePos( const LTVector2n& pos );
	virtual void	SetScale( const LTVector2& vfScale );

    virtual bool	SetFont ( const CFontInfo& Font );
	virtual bool	SetFontHeight (uint32 nFontHeight);

	// free texture memory by flushing any texture strings owned by the control
	virtual void	FlushTextureStrings();

	// rebuild any texture strings owned by the control
	virtual void	RecreateTextureStrings();

protected:
	CLTGUIString	m_Header;

	LTGUIStringArray	m_StringArray;

	CFontInfo		m_Font;					// The font for this control
	uint32			m_nBaseFontSize;		// The font size before scaling

	uint32		m_nHeaderWidth;
	uint8		m_nSelIndex;
	uint8*		m_pnValue;
	bool		m_bGlowEnable;
	float		m_fGlowAlpha;
	LTVector2	m_vGlowSize;
	
	CycleCallBackFn m_pCallback;


};


#endif // !defined(_LTGUICYCLECTRL_H_)
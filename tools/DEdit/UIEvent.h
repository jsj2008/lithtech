//////////////////////////////////////////////////////////////////////
// UIEvent.h - Events for the user interface

#ifndef __UIEVENT_H__
#define __UIEVENT_H__


// The actual tracker events
const int UIEVENT_NONE			= 0;
const int UIEVENT_KEYDOWN		= 1;
const int UIEVENT_KEYUP			= 2;
const int UIEVENT_MOUSEDOWN		= 3;
const int UIEVENT_MOUSEUP		= 4;
const int UIEVENT_MOUSEMOVE		= 5;
const int UIEVENT_MOUSEWHEEL	= 6;

//-------------------------------------------------------------------
// base UI event class
class CUIEvent
{
protected:
	// Name returning function to allow polymorphism.  IsEqual won't work correctly without it.
	virtual char *ClassName() const { return "CUIEvent"; };
	// Compares this event against another event.  Must be declared in descendant classes for proper comparisons
	virtual BOOL IsEqual(const CUIEvent &cOther) const { return (ClassName() == cOther.ClassName()) && (GetType() == cOther.GetType()); };

	int m_iType;

public:
	CUIEvent(int iEvent = UIEVENT_NONE) : m_iType(iEvent) {};
	virtual ~CUIEvent() {};
	virtual CUIEvent *Clone() const { return new CUIEvent(GetType()); };
	
	virtual BOOL operator == (const CUIEvent &cOther) const { return IsEqual(cOther); };
	virtual BOOL operator != (const CUIEvent &cOther) const { return !IsEqual(cOther); };

	// Returns whether or not the passed event is the opposite of this event
	virtual BOOL IsInverse(const CUIEvent &cOther) const { return FALSE; };

	// Member access functions
	virtual int GetType() const { return m_iType; };
	virtual void SetType(int iType) { m_iType = iType; };
};

//-------------------------------------------------------------------
// Keyboard event class
typedef CMoArray<CUIEvent *> CUIEventList;

class CUIKeyEvent : public CUIEvent
{
protected:
	virtual char *ClassName() const { return "CUIKeyEvent"; };
	virtual BOOL IsEqual(const CUIEvent &cOther) const;

	DWORD m_dwCode;
	BOOL m_bFirst;

public:
	CUIKeyEvent(int iEvent = UIEVENT_KEYDOWN, DWORD dwKeyCode = 0, BOOL bFirst = TRUE) : CUIEvent(iEvent), m_dwCode(dwKeyCode), m_bFirst(bFirst) {};
	virtual ~CUIKeyEvent() {};
	virtual CUIEvent *Clone() const { return new CUIKeyEvent(GetType(), GetCode(), GetFirst()); };

	virtual BOOL IsInverse(const CUIEvent &cOther) const;

	// Member access functions
	virtual DWORD GetCode() const { return m_dwCode; };
	virtual void SetCode(DWORD dwCode) { m_dwCode = dwCode; };

	virtual BOOL GetFirst() const { return m_bFirst; };
	virtual void SetFirst(BOOL bFirst) { m_bFirst = bFirst; };
};

//-------------------------------------------------------------------
// Mouse event class
const DWORD UI_MOUSENONE		= 0;
const DWORD UI_MOUSELEFT		= 1;
const DWORD UI_MOUSERIGHT		= 2;
const DWORD UI_MOUSEMIDDLE		= 4;

class CUIMouseEvent : public CUIEvent
{
protected:
	virtual char *ClassName() const { return "CUIMouseEvent"; };
	// Note that the position is not considered in mouse event comparisons
	virtual BOOL IsEqual(const CUIEvent &cOther) const;

	DWORD	m_dwButton;
	CPoint	m_cPos;
	int32	m_nWheelDelta;

public:

	CUIMouseEvent(int iEvent = UIEVENT_MOUSEDOWN, DWORD dwButton = UI_MOUSENONE, DWORD uX = 0, DWORD uY = 0, int32 nWheelDelta = 0) : 
			CUIEvent(iEvent), m_dwButton(dwButton), m_cPos(uX, uY), m_nWheelDelta(nWheelDelta) {};

	CUIMouseEvent(int iEvent, DWORD dwButton, CPoint cPos, int32 nWheelDelta = 0) : 
			CUIEvent(iEvent), m_dwButton(dwButton), m_cPos(cPos), m_nWheelDelta(nWheelDelta) {};

	virtual ~CUIMouseEvent() {};
	virtual CUIEvent *Clone() const { return new CUIMouseEvent(GetType(), GetButton(), GetPos()); };

	virtual BOOL IsInverse(const CUIEvent &cOther) const;

	// Member access functions
	virtual DWORD	GetButton() const			{ return m_dwButton; };
	virtual void	SetButton(DWORD dwButton)	{ m_dwButton = dwButton; };

	virtual CPoint	GetPos() const				{ return m_cPos; };
	virtual void	SetPos(CPoint cPos)			{ m_cPos = cPos; };

	virtual int32	GetWheelDelta() const		{ return m_nWheelDelta;	};
	virtual void	SetWheelDelta(int32 nDelta)	{ m_nWheelDelta = nDelta; };
};

#endif //__UIEVENT_H__
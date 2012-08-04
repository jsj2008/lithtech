//////////////////////////////////////////////////////////////////////
// UIEvent.cpp - Implementation code for the UI events

#include "bdefs.h"
#include "uievent.h"

//////////////////////////////////////////////////////////////////////
// CUIKeyEvent

BOOL CUIKeyEvent::IsEqual(const CUIEvent &cOther) const
{ 
	return CUIEvent::IsEqual(cOther) && (GetCode() == ((const CUIKeyEvent *)&cOther)->GetCode()) &&
		(GetFirst() == ((const CUIKeyEvent *)&cOther)->GetFirst()); 
}

BOOL CUIKeyEvent::IsInverse(const CUIEvent &cOther) const
{
	return (ClassName() == ((const CUIKeyEvent *)&cOther)->ClassName()) &&
		(GetCode() == ((const CUIKeyEvent *)&cOther)->GetCode()) && 
		(((GetType() == UIEVENT_KEYDOWN) && (cOther.GetType() == UIEVENT_KEYUP)) ||
		 ((GetType() == UIEVENT_KEYUP) && (cOther.GetType() == UIEVENT_KEYDOWN)));
}

//////////////////////////////////////////////////////////////////////
// CUIMouseEvent

BOOL CUIMouseEvent::IsEqual(const CUIEvent &cOther) const
{ 
	return	CUIEvent::IsEqual(cOther) && 
			(GetButton() == ((const CUIMouseEvent *)&cOther)->GetButton()); 
}

BOOL CUIMouseEvent::IsInverse(const CUIEvent &cOther) const
{
	return	(ClassName() == ((const CUIMouseEvent *)&cOther)->ClassName()) &&
			(GetButton() == ((const CUIMouseEvent *)&cOther)->GetButton()) && 
			(((GetType() == UIEVENT_MOUSEDOWN) && (cOther.GetType() == UIEVENT_MOUSEUP)) ||
			 ((GetType() == UIEVENT_MOUSEUP) && (cOther.GetType() == UIEVENT_MOUSEDOWN)));
}


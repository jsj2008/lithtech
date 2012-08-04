#ifndef __MAXKEYAGGREGATE_H__
#define __MAXKEYAGGREGATE_H__

#ifndef __KEYDEFAULTAGGREGATE_H__
#	include "KeyDefaultAggregate.h"
#endif

#include "eventnames.h"
#include "hotkey.h"

class CMaxKeyAggregate : 
	public CKeyDefaultAggregate
{
private:

	bool InternalDefineKey(CHotKey& HotKey)
	{
		const char* pszName = HotKey.GetEventName();

		//setup the drag navigation
		if(stricmp(pszName, UIE_NAV_DRAG) == 0)
		{
			HotKey.AddEvent(CUIMouseEvent(UIEVENT_MOUSEDOWN, UI_MOUSEMIDDLE));
		}
		else if(stricmp(pszName, UIE_NAV_DRAG_FAST) == 0)
		{
			HotKey.AddEvent(CUIKeyEvent(UIEVENT_KEYDOWN, VK_SHIFT));
		}

		//set up the arc rotation navigation
		else if(stricmp(pszName, UIE_NAV_ARC_ROTATE) == 0)
		{
			HotKey.AddEvent(CUIKeyEvent(UIEVENT_KEYDOWN, 'V'));
		}
		else if(stricmp(pszName, UIE_NAV_ARC_ROTATE_ROLL) == 0)
		{
			HotKey.AddEvent(CUIMouseEvent(UIEVENT_MOUSEDOWN, UI_MOUSELEFT));
		}

		//resolve conflicting keys
		else if(stricmp(pszName, UIE_VERT_SNAP_VERT) == 0)
		{
			HotKey.AddEvent(CUIKeyEvent(UIEVENT_KEYDOWN, VK_SHIFT));
			HotKey.AddEvent(CUIKeyEvent(UIEVENT_KEYDOWN, 'V'));
		}
		else
		{
			//didn't match any of the others, we want the default database
			//to set up the key configuration
			return false;
		}

		return true;
	}
};

#endif

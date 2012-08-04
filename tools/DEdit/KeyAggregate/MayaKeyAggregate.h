#ifndef __MAYAKEYAGGREGATE_H__
#define __MAYAKEYAGGREGATE_H__

#ifndef __KEYDEFAULTAGGREGATE_H__
#	include "KeyDefaultAggregate.h"
#endif

#include "eventnames.h"
#include "hotkey.h"

class CMayaKeyAggregate : 
	public CKeyDefaultAggregate
{
private:

	bool InternalDefineKey(CHotKey& HotKey)
	{
		const char* pszName = HotKey.GetEventName();

		//setup the drag navigation
		if(stricmp(pszName, UIE_NAV_DRAG) == 0)
		{
			HotKey.AddEvent(CUIKeyEvent(UIEVENT_KEYDOWN, VK_CONTROL));
			HotKey.AddEvent(CUIMouseEvent(UIEVENT_MOUSEDOWN, UI_MOUSEMIDDLE));
		}
		else if(stricmp(pszName, UIE_NAV_DRAG_FAST) == 0)
		{
			HotKey.AddEvent(CUIKeyEvent(UIEVENT_KEYDOWN, VK_SHIFT));
		}

		//set up the arc rotation navigation
		else if(stricmp(pszName, UIE_NAV_ORBIT) == 0)
		{
			HotKey.AddEvent(CUIKeyEvent(UIEVENT_KEYDOWN, VK_CONTROL));
			HotKey.AddEvent(CUIMouseEvent(UIEVENT_MOUSEDOWN, UI_MOUSELEFT));
		}
		else if(stricmp(pszName, UIE_NAV_ORBIT_MOVE) == 0)
		{
			HotKey.AddEvent(CUIMouseEvent(UIEVENT_MOUSEDOWN, UI_MOUSEMIDDLE));
		}

		else if(stricmp(pszName, UIE_NODE_ROTATE) == 0)
		{
			HotKey.AddEvent(CUIKeyEvent(UIEVENT_KEYDOWN, 'E'));
		}

		//resolve conflicting keys
		else if(stricmp(pszName, UIE_VERT_SNAP_EDGE) == 0)
		{
			HotKey.AddEvent(CUIKeyEvent(UIEVENT_KEYDOWN, 'N'));
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

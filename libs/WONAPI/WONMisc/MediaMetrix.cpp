#include "MediaMetrix.h"

#ifdef WIN32_NOT_XBOX

namespace WONAPI
{

static HWND gMediaMetrixHWND = NULL;

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void CreateMediaMetrixEditControl(HWND theParent)
{
     gMediaMetrixHWND = CreateWindow("EDIT",         // predefined class
                                     NULL,           // no window title
                                     WS_CHILD,       // not visible. not tabstop
                                     0, 0, 100, 50,  // arbitrary size
                                     theParent,		 // parent window
                                     (HMENU) NULL,   // edit control ID
                                     (HINSTANCE) GetWindowLongPtr(theParent, GWLP_HINSTANCE),
                                     NULL);          // pointer not needed
}
  
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void FillMediaMetrixEditControl(const char *theURL)
{

	if(gMediaMetrixHWND)
		SendMessage(gMediaMetrixHWND , WM_SETTEXT, 0, (LPARAM)theURL);
}
  
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void ClearMediaMetrixEditControl()
{
	if (gMediaMetrixHWND) 
		SendMessage(gMediaMetrixHWND , WM_SETTEXT, 0, (LPARAM) "");
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void DestroyMediaMetrixEditControl()
{
	if(gMediaMetrixHWND)
	{
		DestroyWindow(gMediaMetrixHWND);
		gMediaMetrixHWND = NULL;
	}
}

};

#endif

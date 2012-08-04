//-------------------------------------------------------------------
//
//   MODULE    : CUIBASE.H
//
//   PURPOSE   : defines the interface for The CUI Basic Widget
//               which all other widgets inherit from.    
//
//   CREATED   : 1/01
//
//   COPYRIGHT : (C) 2001 LithTech Inc
//
//-------------------------------------------------------------------


//	this class has some pure virtual functions and cannot be instantiated


#ifndef __CUIBASE_H__
#define __CUIBASE_H__


#ifndef __CUI_H__
#include "cui.h"
#endif

#ifndef __CUIMESSAGE_H__
#include "cuimessage.h"
#endif


#ifndef DOXYGEN_SHOULD_SKIP_THIS

// forward declaration for the CUIBase Implementation class 
class CUIBase_Impl;

#endif /* DOXYGEN_SHOULD_SKIP_THIS */


/*!  CUIBase class.
\see interface ILTWidgetManager

Used for: Text and UI.   */

class CUIBase
{
	public:

#ifndef DOXYGEN_SHOULD_SKIP_THIS
/*!	
Virtual Destructor.
*/
		virtual ~CUIBase();

#endif /* DOXYGEN_SHOULD_SKIP_THIS */


/*!
\return CUI Widget Type (as defined in cuitypes.h)

This function is used to query the type of a widget.

Used for: Text and UI.
*/
		virtual CUI_WIDGETTYPE	GetType();


/*!
\return Widget's name as a const char*

This function is used to query the class name of a widget.

Used for: Text and UI.
*/		
		virtual const char*		GetClassName();

		
/*!
\return Widget's CUIGUID

This function is used to query the CUIGUID of a widget.

Used for: Text and UI.
*/
		virtual CUIGUID			GetGUID();


/*!
\return CUIBase* address of parent widget

This function returns the address of a widget's parent.
If no parent exists, then this function will return NULL.

\see CUIBase::SetParent(CUIBase*)

Used for: Text and UI.
*/
		virtual CUIBase*		GetParent();


/*!
\return CUIBase* address of first child widget

This function returns the address of the first child widget.
If no child exists, then this function will return NULL.  To continue
iterating through the list of child widgets, call CUIBase::GetNextChild()
until a NULL pointer is returned.

\b NOTE: calling any function which changes the z-order of
widgets or changes their parent while iterating through a list
of children will cause unpredictable results! 
( i.e., CUIBase::BringToFront(), CUIBase::SendToBack(), or CUIBase::SetParent() )

Used for: Text and UI.
*/
		virtual CUIBase*		GetFirstChild();


/*!
\return CUIBase* address of next child widget

This function returns the address of the next child widget
each time it is called.  If no child exists, then this function will return NULL.  To continue
iterating through the list of child widgets, call this function again
until a NULL pointer is returned.  To retrieve the first child widget, use
CUIBase::GetFirstChild().

\b NOTE: calling any function which changes the z-order of
widgets or changes their parent while iterating through a list
of children will cause unpredictable results! 
( i.e., CUIBase::BringToFront(), CUIBase::SendToBack(), or CUIBase::SetParent() )

Used for: Text and UI.
*/
		virtual CUIBase*		GetNextChild();
		

/*!
\param pX (out) address of variable to receive the x-coordinate of the widget.
\param pY (out) address of variable to receive the y-coordinate of the widget.
\param pRelativeTo (in) specifies the widget that the retrieved (x,y) coords. are relative to.
\return CUIR_OK if successful.

This function retrieves the (x,y) coordinates of the upper left corner of a 
widget (in pixels), relative to the widget specified as \e pRelativeTo.  
If \e pRelativeTo is \b NULL, (x,y) will be relative
to the upper left corner of the screen (0,0). 

Used for: Text and UI.
*/
		virtual CUI_RESULTTYPE 		GetPosition(float *pX, 
												float *pY, 
												CUIBase* pRelativeTo);


/*!
\param pW (out) address of variable to receive the width of the widget.
\param pH (out) address of variable to receive the height of the widget.
\return CUIR_OK if successful.

This function retrieves the current width and hieght (in pixels) of a widget.

Used for: Text and UI.
*/
		virtual CUI_RESULTTYPE 		GetSize(float *pW, float *pH);


/*!
\param pRect (out) address of CUIRECT struct to receive the widget's bounding box.
\param pRelativeTo (in) specifies the widget that the retrieved (x,y) coords. are relative to.
\return CUIR_OK if successful.

This function retrieves the current bounding rectangle (in pixels) of a widget, relative to
the widget specified as \e pRelativeTo.  if \e pRelativeTo is \b NULL, (x,y) will be relative
to the upper left corner of the screen (0,0). 

Used for: Text and UI.
*/
		virtual CUI_RESULTTYPE 		GetRect(CUIRECT* pRect,
											CUIBase* pRelativeTo);


/*!
\param elm CUI_ELEMENTTYPE to query
\return HTEXTURE

This function retrieves the current HTEXTURE associated with a CUI_ELEMENTTYPE.
CUI_ELEMENTTYPE is defined in cuitypes.h, and breaks a widget down into texturable
components.  For example, a CUIWindow supports texturing the following CUI_ELEMENTTYPEs:

CUIE_BG			
CUIE_TOP		
CUIE_BOTTOM		
CUIE_LEFT		
CUIE_RIGHT		
CUIE_TR_CORNER	
CUIE_TL_CORNER	
CUIE_BR_CORNER	
CUIE_BL_CORNER	

\see HTEXTURE

Used for: Text and UI.
*/
		virtual HTEXTURE		GetTexture(CUI_ELEMENTTYPE elm);
		

/*!
\param x the new x-coordinate of the widget (in pixels).
\param y the new y-coordinate of the widget (in pixels).
\param pRelativeTo specifies the widget that the incoming (x,y) coords. are relative to
\param bMoveChildren specifies if the move should apply to child widgets also
\return CUIR_OK if successful.

This function moves a widget to new (x,y) coordinates.  If \e pRelativeTo is a valid
widget address, the (x,y) coordinates will be relative to that widget's screen position.
If \e pRelativeTo is \b NULL, the (x,y) coords. will be relative to the upper left corner
of the screen (0,0).

Used for: Text and UI.
*/
		virtual CUI_RESULTTYPE		SetPosition(float x, 
												float y,
												CUIBase* pRelativeTo,
												bool bMoveChildren);


/*!
\param w the new width of the widget (in pixels).
\param h the new height of the widget (in pixels).
\return CUIR_OK if successful.

This function re-sizes a widget to a new width and height.

Used for: Text and UI.
*/
		virtual CUI_RESULTTYPE		SetSize(float w, float h);


/*!
\param pRect the new bounding box of the widget.
\param pRelativeTo specifies the widget that the (x,y) coords. are relative to
\param bMoveChildren specifies if the move should apply to child widgets also
\return CUIR_OK if successful.

This function sets the (x,y) coordiantes of a widget, as well as the width and height.
All units are in pixels.

\see SetPosition()

Used for: Text and UI.
*/
		virtual CUI_RESULTTYPE		SetRect(CUIRECT* pRect,
											CUIBase* pRelativeTo,
											bool bMoveChildren);


/*!
\param pUserCallback the callback function pointer.
\param pUserCallbackObject
\param pUserCallbackData
\return CUI_RESULTTYPE

When a widget handles a message, the widget will attempt to pass the CUIMessage
on to any user-defined callback that exists.  If the user-defined callback handles the message,
it should return something other than CUIR_NOT_HANDLED.  If the callback funtion
returns CUIR_NOT_HANDLED, the message will continue to propagate.

For a further description of pUserCallbackObject and pUserCallbackData, see 
ILTWidgetManager::SetUserCallback().

\see CUI_CALLBACK
\see CUIMessage

Used for: Text and UI.
*/
		virtual CUI_RESULTTYPE		SetUserCallback(CUI_CALLBACKTYPE pUserCallback, 
													void* pUserCallbackObject,
													void* pUserCallbackData);

/*!
\param ppUserCallbackObject (out) address of user-defined callback object
\param ppUserCallbackData (out) address of user-defined callback data
\return address of user-defined callback function 

This function retrieves the user-defined callback, object, and data specified when the 
user callback was set.

\see SetUserCallback().
*/
		virtual CUI_CALLBACKTYPE GetUserCallback(void** ppUserCallbackObject,
												 void** ppUserCallbackData);

/*!
\return user-defined callback object

This function retrieves the user-defined callback object specified when the 
user callback was set.

\see SetUserCallback().
*/
		virtual void* GetUserCallbackObject();


/*!
\return user-defined callback data

This function retrieves the user-defined callback data specified when the 
user callback was set.

\see SetUserCallback().
*/
		virtual void* GetUserCallbackData();
		

/*!
\param pParent the address of the desired parent widget.
\return CUIR_OK if successful.

Use this function to set a widget's parent.  This will modify the widget's render
order and message handling.

\see GetParent()

Used for: Text and UI.
*/
		virtual CUI_RESULTTYPE		SetParent(CUIBase* pParent);


/*!
\param bShowChildren true or false.
\return CUIR_OK if successful.

Use this function to make a hidden widget visible.  If \e bShowChildren is
true, the widget's children will also be shown.

Used for: Text and UI.
*/
		virtual CUI_RESULTTYPE		Show(bool bShowChildren);

/*!
\param bHideChildren true or false.
\return CUIR_OK if successful.

Use this function to hide a widget.  If \e bHideChildren is
true, the widget's children will also be hidden.

Used for: Text and UI.
*/
		virtual CUI_RESULTTYPE		Hide(bool bHideChildren);


/*!
\param flags window state flags to set.
\return CUIR_OK if successful.

Used to set a state bitmask composed of CUI_STATETYPE.

Used for: Text and UI.
*/
		virtual CUI_RESULTTYPE		SetState(uint32 flags);

/*!
\param flags state flags to remove.
\return CUIR_OK if successful.

Used to remove a state bitmask composed of CUI_STATETYPE.

Used for: Text and UI.
*/
		virtual CUI_RESULTTYPE		UnsetState(uint32 flags);

/*!
\return current widget state flags.

Used to query widget's current state.  Returns a state bitmask composed of CUI_STATETYPE.

Used for: Text and UI.
*/
		virtual uint32 				GetState();

/*!
\param elm CUI Element whose color should be set.
\param argb 32-bit color value of the form 0xAARRGGBB.
\return CUI_RESULTTYPE

Use this function to set the background color of a CUI_ELEMENTTYPE.  CUI_ELEMENTTYPEs are
defined in cuitypes.h.

Used for: Text and UI.
*/		
		virtual CUI_RESULTTYPE		SetColor(CUI_ELEMENTTYPE elm, uint32 argb) = 0;


/*!
\param elm CUI Element to set the color of.
\param argb0 32-bit color value of the form 0xAARRGGBB.
\param argb1 32-bit color value of the form 0xAARRGGBB.
\param argb2 32-bit color value of the form 0xAARRGGBB.
\param argb3 32-bit color value of the form 0xAARRGGBB.
\return CUI_RESULTTYPE

Use this function to set separate vertex colors for a CUI_ELEMENTTYPE.  CUI_ELEMENTTYPEs are
defined in cuitypes.h.

Used for: Text and UI.
*/		
		virtual CUI_RESULTTYPE 	SetColors(CUI_ELEMENTTYPE elm, 
										  uint32 argb0,
										  uint32 argb1,
										  uint32 argb2,
										  uint32 argb3) = 0;


/*!
\param elm CUI Element whose texture should be set.
\param hTex Texture to use.
\param bTile Whether to tile or not
\return CUI_RESULTTYPE

Use this function to set the texture of a CUI_ELEMENTTYPE.  CUI_ELEMENTTYPEs are
defined in cuitypes.h.  If \e bTile is true, the texture will tile across the
element.  If false, the texture will stretch to fit.  Note that the amount of times
a texture will tile is limited by the system architecture, and further note that
not all CUI_ELEMENTTYPEs support tiled textures.

Used for: Text and UI.
*/		
		virtual CUI_RESULTTYPE		SetTexture(CUI_ELEMENTTYPE elm, 
							   		   		   HTEXTURE hTex, 
							   		   		   bool bTile) = 0;

/*!
\param left gutter distance from the left side.
\param right gutter distance from the right side.
\param top gutter distance from the top.
\param bottom gutter distance from the bottom.
\return CUI_RESULTTYPE.

Many widget types have sub-elements that can be spaced away from the edges by using a gutter.  
The gutter specifies how far (in pixels) from the edges any interior parts of the UI are drawn.

Used for:  Text and UI.
*/
		virtual CUI_RESULTTYPE	SetGutter(int16 left, int16 right, int16 top, int16 bottom);


/*!
\param pLeft address of variable to receive gutter distance from the left side.
\param pRight address of variable to receive gutter distance from the right side.
\param pTop address of variable to receive gutter distance from the top.
\param pBottom address of variable to receive gutter distance from the bottom.
\return CUI_RESULTTYPE.

Many widget types have sub-elements that can be spaced away from the edges by using a gutter.  
The gutter specifies how far (in pixels) from the edges any interior parts of the UI are drawn.

Used for:  Text and UI.
*/
		virtual CUI_RESULTTYPE	GetGutter(int16 *pLeft, int16 *pRight, int16 *pTop, int16 *pBottom);


/*!
\return CUI_RESULTTYPE

Brings a widget to the front (top) of its local display list.  A parent widget will never
draw above its children, but the child widgets can be re-ordered.

Used for: Text and UI.
*/	
		virtual CUI_RESULTTYPE		BringToFront();

		
/*!
\return CUI_RESULTTYPE

Sends a widget to the back (bottom) of its local display list.  A parent widget will never
draw above its children, but the child widgets can be re-ordered.

Used for: Text and UI.
*/	
		virtual CUI_RESULTTYPE	  	SendToBack();
		

/*!
\return CUI_RESULTTYPE

Causes a widget to draw immediately, and then render its children.  Widgets that are part 
of the main display list (i.e., can trace their lineage back to the root widget) will be 
rendered when ILTWidgetManager::Update() is called.  For widgets that are not part of the 
internal hierarchy, the widget's own render function must be called.

Used for: Text and UI.
*/	
		virtual void  			Render();


/*!
\return CUI_RESULTTYPE

Causes a widget be added to the destroy queue.  When the next ILTWidgetManager::Update() is 
called, the widget (and all its children) will be destroyed.

Used for: Text and UI.
*/	
		virtual void 			Destroy(); 
		

/*!
\param pMessage CUIMessage.
\return CUI_RESULTTYPE

This function is used to cause a widget to respond to a CUIMESSAGE, and
is generally used internally by the ILTWidgetManager.

\see ILTWidgetManager.

Used for: Text and UI.
*/
		virtual CUI_RESULTTYPE		HandleMessage(CUIMessage* pMessage);

/*!
\param pMessage CUIMessage.
\return CUI_RESULTTYPE

This function is used to pass a CUIMessage to all of a widget's
children.  This function is generally called from within a widget's
HandleMessage() function.

\see HandleMessage()
\see ILTWidgetManager

Used for: Text and UI.
*/
		CUI_RESULTTYPE				SendMessageToChildren(CUIMessage* pMessage);
								  
		
#ifndef DOXYGEN_SHOULD_SKIP_THIS
	
		// this code makes the bridge between the base interface and
		// its implementation.  This allows us to neatly cross the
		// EXE/DLL boundary and also keep utility code in the ui/src
		// directory where it belongs (i.e., not exposed in the
		// SDK header files).

		CUIBase_Impl*		GetImpl() { return m_pImpl; }
	
	protected:

		CUIBase_Impl*		m_pImpl;

#endif /* DOXYGEN_SHOULD_SKIP_THIS */
	

};

#endif //__CUIBASE_H__

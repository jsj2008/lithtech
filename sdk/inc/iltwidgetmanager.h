//-------------------------------------------------------------------
//
//   MODULE    : ILTWIDGETMANAGER.H
//
//   PURPOSE   : defines the ILTWidgetManager class
//
//   CREATED   : 1/01 
//
//   COPYRIGHT : (C) 2001 LithTech Inc
//
//-------------------------------------------------------------------


//	All widget creation is done through the manager
//	in a factory-style way.  This allows the manager to keep track of such
// 	things as creation of GUIDs and initial assignement of Parent-Child
//	relationships.
//
//	This interface is available through the new Interface Database!


#ifndef __ILTWIDGETMANAGER_H__
#define __ILTWIDGETMANAGER_H__


#ifndef __LTMODULE_H__
#include "ltmodule.h"
#endif

#ifndef __CUI_H__
#include "cui.h"
#endif

#ifndef __CUIMESSAGE_H__
#include "cuimessage.h"
#endif

	
/*!  ILTWidgetManager interface.  This interface is used to create
and destroy CUI Widgets, as well as pass CUIMessages around.

The widget manager maintains a hierarchy ui ui widgets which is used in rendering and
message handling.  When you create a widget via the ILTWidgetManager::CreateWidget()
function, you pass in an (optionally NULL) parent widget to secure the new widget's
place in the hierarchy. (parent-child relationships can be
changed at run-time, too).  The hierarchy has two basic rules: 

1.  Parent widgets render before (behind) their children.
2.	Messages are passed to children before the parent attempts to handle them.

If you want to use the ILTWidgetManager, you must call the ILTWidgetManager::Update() 
fuction once per frame.  This will trigger the following actions:

1.  Any messages waiting in the queue will be handled
2.  The UI system will render itself.
3.  Widgets scheduled for destruction ( ILTWidgetManager::DestroyWidget() ) will  
be released.

ILTWidgetManager is available from the Interface Database.  

\see class IBase
\see class CUIBase
\see class CUIMessage

Used for: Text and UI.
*/

class ILTWidgetManager : public IBase
{
	public:
		
#ifndef DOXYGEN_SHOULD_SKIP_THIS

		// add ILTWidgetManager to the interface database	
		interface_version(ILTWidgetManager, 0);

		// virtual destructor
		virtual ~ILTWidgetManager() {};

		// not supported/implemented for Sunner 2001
		virtual CUI_RESULTTYPE	SetCurrentSkin(HTEXTURE hTex) = 0;
		virtual HTEXTURE		GetCurrentSkin() = 0;

#endif /* DOXYGEN_SHOULD_SKIP_THIS */
			

/*!
\param scrWidth the LithTech window's width, in pixels
\param scrHeight the LithTech window's height, in pixels

\b Required \b Function.  Called to initialize the ILTWidgetManager's 
services.  You cannot call any of the ILTWidgetManager's functions until
you have initialized it.  The client is responsible for initializing
the ILTWidgetManager.  ILTClientShell::OnEngineInitialized() is a good place.
*/
	virtual void Init(uint16 scrWidth, uint16 scrHeight) = 0;


/*!
\b Required \b Function. Called to close down the ILTWidgetManager's 
services.  If you do not call Term() when the client shell is 
terminating, you risk memory leaks or worse.  The client is responsible for
terminating the ILTWidgetManager.  ILTClientShell::OnEngineTerm() is a good
place.
*/
	virtual void Term() = 0;


/*!
\return whether ILTWidgetManager was initialized.

Returns \b true if the ILTWidgetManager is initialized, or \b false if it has
not yet been initialized.

\see Init()
\see Term()

Used for: Text and UI.
*/
	virtual bool IsInitted() = 0;


/*!
\param pParent parent of the new widget (can be NULL).
\param type the type of widget to create.
\param pRect initial size and position of the widget.  
\param pRelativeTo specifies which widget (if any) the initial rectangle coordinates are 
relative to.
\param flags (optional) specifies initial status flags.
\return pointer to a CUIBase object

Creates a new widget.  If the \e parent is NULL, the widget will not be part of the
rendering or messaging hierarchy, and it will be left up to the programmer to ensure
that the widget is drawn.  If \e pRelativeTo is NULL, then \e pRect will be relative
to the upper left corner of the display (0,0).

To create a widget that is visible and will receive messages by default, the macro
\b CUI_DEFAULT_WIDGET_FLAGS is passed in for the \e flags parameter.  See cuitypes.h for the macro
definition and other flags.

\see CUI_WIDGETTYPE
\see CUI_STATETYPE

Used for: Text and UI.
*/
		virtual CUIBase*		CreateWidget(CUIBase*		  pParent,
											 CUI_WIDGETTYPE   type, 
								  			 CUIRECT*		  pRect, 
											 CUIBase*		  pRelativeTo,
				   				  			 uint32			  flags = CUI_DEFAULT_WIDGET_FLAGS) = 0;
/*!
\param pWidget the address of the widget to destroy.
\return CUIR_OK if successful.

Schedules a widget for destruction.  The widget is not immediately destroyed, so
that any last rendering calls or messages can be handled.  At the end of the 
ILTWidgetManager::Update() function, scheduled widgets are deleted.  Destroying a widget
recursively destroys all child widgets, so if you do not want to release all children,
you must remove them from the widget's child list.

Used for: Text and UI.
*/
		virtual CUI_RESULTTYPE	DestroyWidget(CUIBase* pWidget) = 0;

/*!
\return the address of the root widget.

The root widget is the parent of all top-level widgets. 

Used for: Text and UI.
*/
		virtual CUIBase*		GetRootWidget() = 0;


/*!
\return the address of the topmost widget at screen position (x,y).

Use this function to find the topmost widgets at screen position (x,y). 

Used for: Text and UI.
*/
		virtual CUIBase*		GetWidgetAtCoords(int16 x, int16 y) = 0;

		
/*!
\param pWidget the widget to focus.
\return CUIR_OK if successful.

Setting the focus widget has no visible effect, but it is a way to keep track of
a widget for whatever purpose you may have in mind.

\see GetFocusWidget().

Used for: Text and UI.
*/
		virtual CUI_RESULTTYPE	SetFocusWidget(CUIBase* pWidget) = 0;

/*!
\param pWidget the widget to focus.
\return CUIR_OK if successful.

Setting the focus widget has no visible effect, but it is a way to keep track of
a widget for whatever purpose you may have in mind.

\see GetFocusWidget().

Used for: Text and UI.
*/
		virtual CUIBase*		GetFocusWidget() = 0;

/*!
\param pWidget address of the widget to check
\return true or false

Function returns true if the widget's CUIGUID is found in the internal GUID tree.
If the function returns false, then the widget was previously destroyed and the 
pointer is no longer valid.

Used for: Text and UI.
*/
		virtual bool			IsWidgetValid(CUIBase* pWidget) = 0;

/*!
\return none.

Call this function once per frame to render the UI, process waiting messages, and handle
the deletion of destroyed widgets.

Used for: Text and UI.
*/
		virtual void			Update() = 0;	
		
/*!
\param pMessage address of a CUIMessage object
\return CUIR_OK if successful.

This function enqueues a message in the internal CUI message handler.  An internal 
copy of the message is created, so you do not have to keep this message around.

\see CUIMessage.
\see CUI_MESSAGETYPE.

Used for: Text and UI.
*/
		virtual CUI_RESULTTYPE	EnqueueMessage(CUIMessage* pMessage) = 0;
							
/*!
\param pMessage address of an empty CUIMessage object
\return CUIR_OK if successful.

This function dequeues (removes) a message from the internal CUI message handler.
You must provide a CUIMessage object to receive the data.

\see CUIMessage.
\see CUI_MESSAGETYPE.

Used for: Text and UI.
*/
		virtual CUI_RESULTTYPE	DequeueMessage(CUIMessage* pMessage) = 0;		
						
/*!
\param pMessage address of a CUIMessage object
\return CUIR_OK if successful.

This function looks at the next message in the queue, but does not delete it.
You must provide a CUIMessage object to receive the data.

\see CUIMessage.
\see CUI_MESSAGETYPE.

Used for: Text and UI.
*/
		virtual CUI_RESULTTYPE	PeekMessage(CUIMessage* pMessage) = 0;

/*!
\param pUserCallback address of a user-defined callback,
\param pUserCallbackObject a user-defined object.
\param pUserCallbackData user-specified data that gets added to every CUIMessage going to
this callback
\return CUIR_OK if successful.

This function installs a system-wide message callback if the user wishes to
intercept all messages before they are sent to any widgets.  the \e pUserCallbackObject
parameter is used to get around the limitation of not being able to use dynamic
member functions as callbacks.  For example:

Within foo:init_ui()

    Call the function ILTWidgetManager::SetUserCallback(foo::static_callback, \b this, pUserData).

WIthin static foo::static_callback(CUIMessage* pMessage)

    foo* theFoo = (foo*)pMessage->m_pUserObject;

And you can call foo's member functions.

\see CUI_CALLBACKTYPE.

Used for: Text and UI.
*/
		virtual CUI_RESULTTYPE	SetUserCallback(CUI_CALLBACKTYPE pUserCallback, 
												void* pUserCallbackObject,
												void* pUserCallbackData) = 0;

/*!
\ppUserCallbackObject (out) address of user-defined callback object
\ppUserCallbackData (out) address of user-defined callback data
\return address of user-defined callback function 

This function retrieves the user-defined callback, object, and data specified when the 
user callback was set.

\see SetUserCallback().
*/
		virtual CUI_CALLBACKTYPE GetUserCallback(void** ppUserCallbackObject,
												 void** ppUserCallbackData) = 0;

/*!
\return user-defined callback object

This function retrieves the user-defined callback object specified when the 
user callback was set.

\see SetUserCallback().
*/
		virtual void* GetUserCallbackObject() = 0;

/*!
\return user-defined callback data

This function retrieves the user-defined callback data specified when the 
user callback was set.

\see SetUserCallback().
*/
		virtual void* GetUserCallbackData() = 0;

/*!
\return CUIR_OK if successful.

This function sets the focus to the next sibling of the currently focused widget, if any.

Used for: Text and UI.
*/
		virtual CUI_RESULTTYPE FocusNext()	= 0;

/*!
\return CUIR_OK if successful.

This function sets the focus to the parent of the currently focused widget, if any.

Used for: Text and UI.
*/
		virtual CUI_RESULTTYPE FocusUp()	= 0;

/*!
\return CUIR_OK if successful.

This function sets the focus to the first child of the currently focused widget, if any.

Used for: Text and UI.
*/
		virtual CUI_RESULTTYPE FocusDown()	= 0;
};


#endif //__ILTWIDGETMANAGER_H__

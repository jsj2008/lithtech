//-------------------------------------------------------------------
//
//   MODULE    : CUITYPES.H
//
//   PURPOSE   : #define's, enums, structs for CUI system
//
//   CREATED   : 7/00 
//
//-------------------------------------------------------------------


#ifndef __CUITYPES_H__
#define __CUITYPES_H__


// a few platform-specificities
#ifdef WIN32

/*!	
On the PC, this is an alpha of 1.0 (opaque).
*/
	#define	CUI_SYSTEM_OPAQUE	0xFF000000
/*!	
On the PC, this is the default color of a CUIFont.
*/
	#define CUI_DEFAULT_FONT_COLOR		0x00FFFFFF
/*!	
On the PC, this is the default color of a CUIWidget.
*/
	#define CUI_DEFAULT_WIDGET_COLOR	0x00FFFFFF

#endif



//  --------------------------------------------------------------------------
// 	widget types
//	--------------------------------------------------------------------------

/*!
Enumerates the types of widgets that can be created and managed via the
CUIWidgetManager class.
*/
typedef enum {
/*!
The class from which all other widgets are derived.  This is an abstract
class.
*/
	CUIW_WIDGET,

/*!
A basic container class.  Has a background image as well as edges and corners
that can be textured.
*/
	CUIW_WINDOW,		

/*!
A basic image class.  Holds a single static image.
*/
	CUIW_STATICIMAGE,

/*!
A basic button class.  Holds images for up, down, highlighted, and disabled
states, and displays a string of text.
*/
	CUIW_BUTTON,		

/*!
A basic text container class.  Has a background image as well as a
CUIFormattedPolyString.
*/
	CUIW_STATICTEXT,	

/*!
A slider bar.  Has a background image as well as a slider.
*/
	CUIW_SLIDER,
	
/*!
A progress indicator.  Has a background image and a progress image.
*/
	CUIW_PROGRESS,

/*!
A checkbox.  Has a background image as well as check-on, check-off, and disabled.
*/
	CUIW_CHECK,

/*!
A option widget (a.k.a. Radio Button).  Has a background image as well as option-on, option-off, and disabled.
*/
	CUIW_OPTION,

/*!
A list-box widget.  Has a background image and contains a list of text items.
*/
	CUIW_LIST,
/*!
A drop-down list widget.  Has a background image, and contains a list of text items.
*/
	CUIW_DROPDOWNLIST,

#ifndef DOXYGEN_SHOULD_SKIP_THIS
	// not yet supported
	CUIW_VIEWPORT,	
	CUIW_CURSOR,		
	CUIW_GROUP,		
	CUIW_RADIOBUTTON,	
	CUIW_EDITTEXT,	
	CUIW_MENU,		
	CUIW_POPUP
#endif /* DOXYGEN_SHOULD_SKIP_THIS */
}	
CUI_WIDGETTYPE;



//  --------------------------------------------------------------------------
// 	widget messages
//	--------------------------------------------------------------------------

/*!
Enumerates the types of messages that the CUI widget toolkit can send and
receive.  A few of these messages are sent internally by the ILTWidgetManager,
but most are intended to be enqueued from (and responded to) in the Client 
Shell.  Since messages are largely the province of the client programmer,
they can be extended simply by adding values to this ennumerated type, 
recompiling the engine, and adding client code to handle them.

\see CUIMessage
\see CUIBase::SetUserCallbacke()
\see ILTWidgetManager::SetUserCallback()
\see ILTWidgetManager::EnqueueMessage()
\see ILTWidgetManager::DequeueMessage()
\see ILTWidgetManager::PeekMessage()
*/

typedef enum {
/*!
Indicates that a widget has been created.
*/
	CUIM_WIDGET_ON_CREATE = 1,
/*!
Indecates that a widget that is about to be destroyed.  Sent by the ILTWidgetManager.
*/
	CUIM_WIDGET_ON_DESTROY,
/*!
Used internally by the ILTWidgetManager to manage the destroy queue.
*/
	CUIM_WIDGET_SCHEDULE_FOR_DESTROY,
/*!
Indicates that a widget has been given the focus.  Sent by the ILTWidgetManager.
*/			
	CUIM_GOT_FOCUS,
/*!
Indicates that a widget has lost the focus.  Sent by the ILTWidgetManager.
*/
	CUIM_LOST_FOCUS,
/*!
Indicates that a widget has been hidden.
*/
	CUIM_ON_HIDE,
/*!
Indicates that a widget has been made visible.
*/
	CUIM_ON_SHOW,
/*!
Indicates that a widget has been resized.  Sent by the ILTWidgetManager.
The new height and width of the widget are packed into the message's m_Data1
member like so:  (int16)w | ((int16)h) << 16.
*/			
	CUIM_ON_RESIZE,
/*!
Indicates that a widget has been moved.  Sent by the ILTWidgetManager.
The new x and y position are packed into the message's m_Data1
member like so:  (int16)x | ((int16)y) << 16.
*/			
	CUIM_ON_MOVE,	

#ifndef DOXYGEN_SHOULD_SKIP_THIS
	// this allows the widgetmanager to prevent "system" messages from going to
	// widgets.
	CUIM_SYSTEM_THRESHOLD,
#endif /* DOXYGEN_SHOULD_SKIP_THIS */
			
/*!
Indicates that a mouse button has been pressed.  It is suggested that the (x,y) cursor
coordintates be packed into the CUIMessage::m_Data1 member, and that a button mask
be encoded in the CUIMessage::m_Data2 member.
*/			
	CUIM_ON_MOUSE_DOWN,
/*!
Indicates that a mouse button has been released.  It is suggested that the (x,y) cursor
coordintates be packed into the CUIMessage::m_Data1 member, and that a button mask
be encoded in the CUIMessage::m_Data2 member.
*/			
	CUIM_ON_MOUSE_UP,
/*!
Indicates that a mouse button has been pressed and released.  It is suggested that the
(x,y) cursor coordintates be packed into the CUIMessage::m_Data1 member, and that a 
button mask be encoded in the CUIMessage::m_Data2 member.
*/			
	CUIM_ON_MOUSE_CLICK,
	
/*!
Indicates that a key has been pressed.
*/
	CUIM_ON_KEY_DOWN,	
/*!
Indicates that a key has been released.
*/
	CUIM_ON_KEY_UP,		
/*!
Indicates that a key has been pressed and released.
*/
	CUIM_ON_KEY_PRESS,		

/*!
Indicates that the PC joystick or PS2 controller has been moved down.
*/
	CUIM_ON_PAD_DOWN,	
/*!
Indicates that the PC joystick or PS2 controller has been moved up.
*/
	CUIM_ON_PAD_UP,	
/*!
Indicates that the PC joystick or PS2 controller has been moved to the left.
*/
	CUIM_ON_PAD_LEFT,	
/*!
Indicates that the PC joystick or PS2 controller has been moved to the right.
*/
	CUIM_ON_PAD_RIGHT,	
	
/*!
Indicates that the 'X' button of the PS2 controller has been pressed.
*/
	CUIM_ON_PAD_X_DOWN,		
/*!
Indicates that the 'X' button of the PS2 controller has been released.
*/
	CUIM_ON_PAD_X_UP,		
/*!
Indicates that the 'X' button of the PS2 controller has been pressed and released.
*/
	CUIM_ON_PAD_X_PRESS,		

/*!
Indicates that the 'CIRCLE' button of the PS2 controller has been pressed.
*/
	CUIM_ON_PAD_CIRCLE_DOWN,
/*!
Indicates that the 'CIRCLE' button of the PS2 controller has been released.
*/
	CUIM_ON_PAD_CIRCLE_UP,
/*!
Indicates that the 'CIRCLE' button of the PS2 controller has been pressed and released.
*/
	CUIM_ON_PAD_CIRCLE_PRESS,

/*!
Indicates that the 'TRIANGLE' button of the PS2 controller has been pressed.
*/
	CUIM_ON_PAD_TRIANGLE_DOWN,
/*!
Indicates that the 'TRIANGLE' button of the PS2 controller has been released.
*/
	CUIM_ON_PAD_TRIANGLE_UP,
/*!
Indicates that the 'TRIANGLE' button of the PS2 controller has been pressed and released.
*/
	CUIM_ON_PAD_TRIANGLE_PRESS,

/*!
Indicates that the 'SQUARE' button of the PS2 controller has been pressed.
*/
	CUIM_ON_PAD_SQUARE_DOWN,
/*!
Indicates that the 'SQUARE' button of the PS2 controller has been released.
*/
	CUIM_ON_PAD_SQUARE_UP,
/*!
Indicates that the 'SQUARE' button of the PS2 controller has been pressed and released.
*/
	CUIM_ON_PAD_SQUARE_PRESS,
	
/*!
Indicates that the 'L1' button of the PS2 controller has been pressed.
*/
	CUIM_ON_PAD_L1_DOWN,	
/*!
Indicates that the 'L1' button of the PS2 controller has been released.
*/
	CUIM_ON_PAD_L1_UP,	
/*!
Indicates that the 'L1' button of the PS2 controller has been pressed and released.
*/
	CUIM_ON_PAD_L1_PRESSED,	

/*!
Indicates that the 'L2' button of the PS2 controller has been pressed.
*/
	CUIM_ON_PAD_L2_DOWN,	
/*!
Indicates that the 'L2' button of the PS2 controller has been released.
*/
	CUIM_ON_PAD_L2_UP,	
/*!
Indicates that the 'L2' button of the PS2 controller has been pressed and released.
*/
	CUIM_ON_PAD_L2_PRESSED,	

/*!
Indicates that the 'R1' button of the PS2 controller has been pressed.
*/
	CUIM_ON_PAD_R1_DOWN,	
/*!
Indicates that the 'R1' button of the PS2 controller has been released.
*/
	CUIM_ON_PAD_R1_UP,	
/*!
Indicates that the 'R1' button of the PS2 controller has been pressed and released.
*/
	CUIM_ON_PAD_R1_PRESSED,	

/*!
Indicates that the 'R2' button of the PS2 controller has been pressed.
*/
	CUIM_ON_PAD_R2_DOWN,	
/*!
Indicates that the 'R2' button of the PS2 controller has been released.
*/
	CUIM_ON_PAD_R2_UP,	
/*!
Indicates that the 'R2' button of the PS2 controller has been pressed and released.
*/
	CUIM_ON_PAD_R2_PRESSED,	
	
/*!
Indicates that the 'START' button of the PS2 controller has been pressed.
*/
	CUIM_ON_PAD_START_DOWN,	
/*!
Indicates that the 'START' button of the PS2 controller has been released.
*/
	CUIM_ON_PAD_START_UP,	
/*!
Indicates that the 'START' button of the PS2 controller has been pressed and released.
*/
	CUIM_ON_PAD_START_PRESSED,	

/*!
Indicates that the 'SELECT' button of the PS2 controller has been pressed.
*/
	CUIM_ON_PAD_SELECT_DOWN,	
/*!
Indicates that the 'SELECT' button of the PS2 controller has been released.
*/
	CUIM_ON_PAD_SELECT_UP,	
/*!
Indicates that the 'SELECT' button of the PS2 controller has been pressed and released.
*/
	CUIM_ON_PAD_SELECT_PRESSED,
	
/*!
Indicates that the 'TRIGGER1' button of the PC joystick has been pressed.
*/
	CUIM_ON_JOYSTICK_TRIGGER1_DOWN,
/*!
Indicates that the 'TRIGGER1' button of the PC joystick has been released.
*/
	CUIM_ON_JOYSTICK_TRIGGER1_UP,
/*!
Indicates that the 'TRIGGER1' button of the PC joystick has been pressed and released.
*/
	CUIM_ON_JOYSTICK_TRIGGER1_PRESSED,

/*!
Indicates that the 'TRIGGER2' button of the PC joystick has been pressed.
*/
	CUIM_ON_JOYSTICK_TRIGGER2_DOWN,
/*!
Indicates that the 'TRIGGER2' button of the PC joystick has been released.
*/
	CUIM_ON_JOYSTICK_TRIGGER2_UP,
/*!
Indicates that the 'TRIGGER2' button of the PC joystick has been pressed and released.
*/
	CUIM_ON_JOYSTICK_TRIGGER2_PRESSED,

/*!
Indicates that the 'TRIGGER3' button of the PC joystick has been pressed.
*/
	CUIM_ON_JOYSTICK_TRIGGER3_DOWN,
/*!
Indicates that the 'TRIGGER3' button of the PC joystick has been released.
*/
	CUIM_ON_JOYSTICK_TRIGGER3_UP,
/*!
Indicates that the 'TRIGGER3' button of the PC joystick has been pressed and released.
*/
	CUIM_ON_JOYSTICK_TRIGGER3_PRESSED,

/*!
Indicates that the 'TRIGGER4' button of the PC joystick has been pressed.
*/
	CUIM_ON_JOYSTICK_TRIGGER4_DOWN,
/*!
Indicates that the 'TRIGGER4' button of the PC joystick has been released.
*/
	CUIM_ON_JOYSTICK_TRIGGER4_UP,
/*!
Indicates that the 'TRIGGER4' button of the PC joystick has been pressed and released.
*/
	CUIM_ON_JOYSTICK_TRIGGER4_PRESSED,


#ifndef DOXYGEN_SHOULD_SKIP_THIS

	// keep a few of these around so headers don't get out of sync
	// every rootin-tootin' time you want to test something :-)

	CUIM_DUMMY_MESSAGE1 =	9999,
	CUIM_DUMMY_MESSAGE2 =	9998,
	CUIM_DUMMY_MESSAGE3 =	9997,
	CUIM_DUMMY_MESSAGE4 =	9996,
	CUIM_DUMMY_MESSAGE5 =	9995,

#endif /* DOXYGEN_SHOULD_SKIP_THIS */

/*!
Denotes an 'empty' message.
*/
	CUIM_NO_MESSAGE = 0

}
CUI_MESSAGETYPE;


//  --------------------------------------------------------------------------
// 	CUISTATESs
//  --------------------------------------------------------------------------

/*!
These are used to get/set state info bitmasks for ui widgets.  Many of these
are used to describe the visual appearance of a widget.  They can be used in
conjunction (i.e., CUIS_ENABLED | CUIS_VISIBLE).  When a widget is created,
it is assigned CUIS_ENABLED and CUIS_VISIBLE by default.
*/
typedef enum {
/*!
Whether a widget is drawn or not.  A widget that is invisible will still render
its child widgets, if they themselves are visible.  This is different from the 
CUIS_TREE_HIDDEN flag, which effects the current widget and all its descendants.
*/
	CUIS_VISIBLE		= 1,
/*!
A widget that is enabled will receive messages, a disabled widget will not.
*/
	CUIS_ENABLED		= 2,
/*!
Whether a widget is in its "pressed," "down," or "checked" state (as in a
CUIButton, CUICheck, or CUIOption widget).
*/
	CUIS_PRESSED		= 4,
/*!
Whether a widget is in its "highlighted" or "focused" state.
*/
	CUIS_HIGHLIGHTED	= 8,
/*!
A widget that is not tree-visible is not drawn, and its child widgets are not drawn.
This is different from the CUIS_VISIBLE flag which only affects the current
widget.  This state is useful when you want to hide a subtree of your widget hierarchy,
but want to preserve visibility states of child widgets when the subtree becomes
visible again.
*/
	CUIS_TREE_VISIBLE	= 16,
/*!
Whether a widget such as a drop-down list is in its "open" or "droppped" state.
*/
	CUIS_OPEN			= 32,

}
CUI_STATETYPE;


/*!
Defines a set of common widget creation flags.

\see CUIWidgetManager::CreateWidget()
*/
#define CUI_DEFAULT_WIDGET_FLAGS	CUIS_VISIBLE | CUIS_ENABLED | CUIS_TREE_VISIBLE


//  --------------------------------------------------------------------------
// 	CUIELEMENTs
//  --------------------------------------------------------------------------

/*!
CUI_ELEMENTTYPE is used to get or set colors and HTEXTURES for CUIWidgets.  
Depending on the type of widget and the type of element, textures can be tiled, or
stretched to fit their widget's size.

\b Note:  not all widgets have every CUI_ELEMENTTYPE.  Refer to the
documentation for each widget type.

\see CUIButton
\see CUICheck
\see CUIDropDownList
\see CUIList
\see CUIOption
\see CUIProgress
\see CUISlider
\see CUIStaticText
\see CUIStaticImage
\see CUIWindow
*/

typedef enum {
/*!
For CUIBase::SetColor() \e only, This will apply a color change to a relevant 
portion of the possilbe elements within a widget.  For example, calling 
CUIWindow::SetColor(CUIE_RELEVANT, 0xFFFFFFFF) will set the corners and edges of
the window to white, but not the main background.  Likewise, calling 
CUIButton::SetColor(CUIE_RELEVANT, 0xFFFFFFFF) will set all button backgrounds to
white, but will not change the color of the button's text.
*/
	CUIE_RELEVANT = 1,
/*!
The main background image.
*/
	CUIE_BG,
/*!
The top edge.
*/
	CUIE_TOP,		
/*!
The bottom edge.
*/
	CUIE_BOTTOM,	
/*!
The left edge.
*/
	CUIE_LEFT,		
/*!
The right edge.
*/
	CUIE_RIGHT,		
/*!
The top right corner.
*/
	CUIE_TR_CORNER,	
/*!
The top left corner.
*/
	CUIE_TL_CORNER,	
/*!
The bottom right corner.
*/
	CUIE_BR_CORNER,	
/*!
The bottom left corner.
*/
	CUIE_BL_CORNER,	
/*!
A button's "down" (pressed) image.
*/
	CUIE_BUTTON_PRESSED,
/*!
A button's highlighed image.
*/
	CUIE_BUTTON_HIGHLIGHTED,
/*!
A button's disabled image.
*/
	CUIE_BUTTON_DISABLED,
/*!
A widget's text.
*/
	CUIE_TEXT,
/*!
A widget's selected text.  This is for SetColor() and SetColors() only.  No texture
is associated with CUIE_SELECTEDTEXT.
*/
	CUIE_SELECTEDTEXT,
/*!
A slider's sliding thingie.
*/
	CUIE_SLIDER,
/*!
A progress indicator's indicator.
*/
	CUIE_PROGRESS_INDICATOR,
/*!
A check widget's "pressed" or "on" image.
*/
	CUIE_CHECK_ON,
/*!
A check widget's "up" or "off" image.
*/	
	CUIE_CHECK_OFF,
/*!
A check widget's disabled image.
*/
	CUIE_CHECK_DISABLED,
/*!
An option widget's "pressed" or "on" image.
*/
	CUIE_OPTION_ON,
/*!
An option widget's "up" or "off" image.
*/
	CUIE_OPTION_OFF,
/*!
An option widget's disabled image.
*/
	CUIE_OPTION_DISABLED,
/*!
A drop-down list's dropped background.
*/
	CUIE_DROPDOWN_BG
}
CUI_ELEMENTTYPE;


 
//  --------------------------------------------------------------------------
// 	CUI ALIGNMENTS
//	--------------------------------------------------------------------------

/*!
These are used to set the horizontal and vertical alignment of some widgets,
as well as CUIFormattedPolyString.  Be aware that not all CUIWidget classes 
support the concept of alignment.
*/
typedef enum {
/*!
Aligned horizontally to the left side.
*/
	CUI_HALIGN_LEFT = 1,
/*!
Aligned horizontally to the right side.
*/
	CUI_HALIGN_RIGHT,
/*!
Centered horizontally.
*/
	CUI_HALIGN_CENTER,
/*!
Justified horizontally.  In the case of text, this means that the words will
spread out to touch the left and right edges of a bounding rectangle.
*/
	CUI_HALIGN_JUSTIFY,
/*!
Aligned vertically to the top.
*/
	CUI_VALIGN_TOP,
/*!
Aligned vertically to the bottom.
*/
	CUI_VALIGN_BOTTOM,
/*!
Centered vertically.
*/
	CUI_VALIGN_CENTER,
}
CUI_ALIGNMENTTYPE;


//  --------------------------------------------------------------------------
// 	CUI ORIENTATIONS
//	--------------------------------------------------------------------------

/*!
These are used to set the horizontal and vertical orientation of some widgets.
Be aware that not all CUIWidget classes support the concept of orientation.
*/
typedef enum {
/*!
Oriented horizontally.
*/
	CUI_ORIENT_HORIZONTAL = 1,
/*!
Oriented vertically.
*/
	CUI_ORIENT_VERTICAL,
}
CUI_ORIENTATIONTYPE;


//  --------------------------------------------------------------------------
// 	CUI STRETCH MODES
//	--------------------------------------------------------------------------

/*!
These are used to set the stretch mode of CUIProgress.
*/
typedef enum {
/*!
CUI_STRETCH_FALSE means that the texture drawn on CUIE_PROGRESS will be partially
drawn.  How much is seen depends on the value of the CUIProgress widget.
*/
	CUI_STRETCH_FALSE = 0,
/*!
CUI_STRETCH_TRUE means that the texture drawn on CUIE_PROGRESS will draw in its
entirety, and will grow and shrink as the value of the CUIProgress widget changes.
*/
	CUI_STRETCH_TRUE = 1,
}
CUI_STRETCHMODE;


//  --------------------------------------------------------------------------
// 	CUI FILL MODES
//	--------------------------------------------------------------------------

/*!
These are used to set the fill mode of CUIProgress.
*/
typedef enum {
/*!
CUI_FILL_LEFTTORIGHT means that the CUIProgress widget will fill from left to
right when its orientation is CUI_ORIENT_HORIZONTAL.
*/
	CUI_FILL_LEFTTORIGHT = 0,
/*!
CUI_FILL_RIGHTTOLEFT means that the CUIProgress widget will fill from left to
right when its orientation is CUI_ORIENT_HORIZONTAL.
*/
	CUI_FILL_RIGHTTOLEFT = 1,
/*!
CUI_FILL_TOPTOBOTTOM means that the CUIProgress widget will fill from left to
right when its orientation is CUI_ORIENT_VERTICAL.
*/
	CUI_FILL_TOPTOBOTTOM = 0,
/*!
CUI_FILL_BOTTOMTOTOP means that the CUIProgress widget will fill from left to
right when its orientation is CUI_ORIENT_VERTICAL.
*/
	CUI_FILL_BOTTOMTOTOP = 1,
}
CUI_FILLMODE;


//  --------------------------------------------------------------------------
// CUIRESULT Definitions
//	--------------------------------------------------------------------------

/*! 
CUI_RESULTTYPE is retuned by most functions in the CUI toolkit.  They are used
to notify the caller of success or failure when more than just a TRUE or FALSE
value might be required.

Error codes are values less than or equal to zero, and success codes are
greater than zero.
*/

typedef enum {
/*!
Reached the end of the Message Queue.
*/
	CUIR_END_OF_MESSAGE_QUEUE = -1000,
/*!
Tried to access a null-pointer.
*/
	CUIR_INVALID_POINTER,
/*!
An widget's address does not match with the GUID tree.
*/
	CUIR_WIDGET_MISMATCH,
/*!
A widget does not exist in the GUID tree.
*/
	CUIR_NONEXISTENT_WIDGET,
/*!
An unknown CUI_ELEMENTTYPE was specified.
*/
	CUIR_UNKNOWN_ELEMENT,
/*!
An error occurred in ILTDrawPrim.
*/
	CUIR_ILT_DRAWPRIM_ERROR,
/*!
An error occured in ILTTexInterface.
*/
	CUIR_ILT_TEXINTERFACE_ERROR,
/*!
The ILTWidgetManager was never initialized.
*/
	CUIR_ILT_WIDGETMANAGER_ERROR,
/*!
The ILTFontManager was never initialized.
*/
	CUIR_ILT_FONTMANAGER_ERROR,
/*!
An error occured in the FreeType Library.
*/
	CUIR_FREETYPE_ERROR,
/*! 
A CUIFont was not properly created.  This is often the case when
the wrong texture type is used, or an incompatible texture depth
is set in the engine.
*/
	CUIR_INVALID_FONT,
/*!
A necessary CUIFont was not present.
*/
	CUIR_NO_FONT,
/*!
A memory allocation failed.
*/
	CUIR_OUT_OF_MEMORY,
/*!
An unknown position-relative specifier was encountered.
*/
	CUIR_UNKNOWN_CUIPOSITIONTYPE,
/*!
A widget was expected to have a parent, but did not.
*/
	CUIR_NO_PARENT,
/*!
An out-of-bounds index was specified.
*/
	CUIR_NONEXISTENT_INDEX,
/*!
A CUIPolyString or CUIFormattedPolystring did not initialize properly.
This could be the result of passing in a NULL font pointer.
*/
	CUIR_INVALID_POLYSTRING,
/*!
A widget did not initialize properly.
*/
	CUIR_INVALID_WIDGET,
/*!
A general error occured.
*/
	CUIR_ERROR = 0,
/*!
Success result.
*/
	CUIR_OK	= 1,
/*!
Indicates that a CUIMessage was not handled.  This is not an error, it just
means that the message was passed on for another widget (or the client) to
deal with.
*/
	CUIR_MESSAGE_NOT_HANDLED,
/*!
Indicates that a queried point lies in the \e minimal \e area of a CUISlider.
\see CUISlider::QueryPoint() 
*/
	CUIR_SLIDER_MIN,
/*!
Indicates that a queried point lies in the \e position \e marker (the bar) of a CUISlider.
\see CUISlider::QueryPoint() 
*/
	CUIR_SLIDER_BAR,
/*!
Indicates that a queried point lies in the \e maximal \e area of a CUISlider.
\see CUISlider::QueryPoint() 
*/
	CUIR_SLIDER_MAX,
/*!
Indicates that a queried point lies in the \e fill \e area (the bar) of a CUIProgress.
\see CUIProgress::QueryPoint() 
*/
	CUIR_PROGRESS_LESS,
/*!
Indicates that a queried point lies in the \e empty \e area of a CUIProgress.
\see CUIProgress::QueryPoint() 
*/
	CUIR_PROGRESS_GREATER

}
CUI_RESULTTYPE;



//  --------------------------------------------------------------------------
// CUI structs and typedefs
//	--------------------------------------------------------------------------

/*! 
A Globally Unique ID number among CUI classes.
*/
typedef	uint32 	CUIGUID;


#ifndef DOXYGEN_SHOULD_SKIP_THIS
// Forward declarations for the CUI_CALLBACKTYPE typedef
class CUIMessage;
class CUIBase;
#endif /* DOXYGEN_SHOULD_SKIP_THIS */


/*! 
The standard signature of a user-defined CUI callback.  User callbacks are intended
for the client shell to handle CUI messages.  It is important to remember that
CUIWidgets do not handle messages on their own.  It is up to the client shell to 
perform such actions as, for example, putting a CUIButton into its \e pressed 
state when it receives a CUIM_MOUSE_DOWN event (which, in turn, must be enqueued
by the client shell).  

\see ILTWidgetManager::SetUserCallback()
\see CUIBase::SetUserCallback()
*/
typedef CUI_RESULTTYPE (*CUI_CALLBACKTYPE) (CUIMessage* pMessage);


/*! 
CUI Rectangle struct.
*/
//typedef struct {
/*! 
x coordinate
*/
//	float x;
/*! 
y coordinate.
*/
//	float y;
/*! 
width.
*/
//	float width;
/*! 
height.
*/
//	float height;
//}
//CUIRECT;

/*! 
CUI Rectangle struct.
*/
class CUIRECT {

	public:

		// construction
		CUIRECT() { x = y = width = height = 0; }
		CUIRECT(float ix, float iy, float iwidth, float iheight) { 
			x = ix; 
			y = iy;
			width = iwidth;
			height = iheight;}

		// operator overload
		CUIRECT& operator = ( const CUIRECT & srcRect ) {
			x = srcRect.x;
			y = srcRect.y;
			width = srcRect.width;
			height = srcRect.height;
			return *this;
		}

	public:
/*! 
x coordinate
*/
		float x;
/*! 
y coordinate.
*/
		float y;
/*! 
width.
*/
		float width;
/*! 
height.
*/
		float height;
};


#endif //__CUITYPES_H__



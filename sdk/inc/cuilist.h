//-------------------------------------------------------------------
//
//   MODULE    : CUIList.H
//
//   PURPOSE   : defines the CUIList bridge class
//
//   CREATED   : 1/01 
//
//   COPYRIGHT : (C) 2001 LithTech Inc
//
//-------------------------------------------------------------------


#ifndef __CUILIST_H__
#define __CUILIST_H__


#ifndef __CUIWIDGET_H__
#include "cuiwidget.h"
#endif

#ifndef __CUIFONT_H__
#include "cuifont.h"
#endif


/*!  
CUIList class.  This class is uses a collection of CUIPolyStrings to display a
list of text items over a background.

CUIE_BG

\see class CUIPolyString

Used for: Text and UI.   */

class CUIList : public CUIWidget
{
	public:
			

#ifndef DOXYGEN_SHOULD_SKIP_THIS

		// CUIWidgets should not be created with new and delete, but should instead
		// be created via the ILTWindowManager's CreateWidget() and DestroyWidget()
		// management functions.
		CUIList();
		CUIList(CUIGUID guid);
		virtual ~CUIList();
		
#endif /* DOXYGEN_SHOULD_SKIP_THIS */

/*!  
\return current font

Use this function to retrieve the address of the CUIFont which is being used
by the CUIList widget.

Used for: Text and UI.   */
		virtual CUIFont*		GetFont();

/*!  
\param pFont address of font to set

Use this function to set the CUIFont used by the CUIList widget.

Used for: Text and UI.   */		
		virtual CUI_RESULTTYPE	SetFont(CUIFont* pFont);
		
/*!  
\param height height of text

Use this function to set the height of text items displayed by the CUIList
widget.

Used for: Text and UI.   */		
		virtual CUI_RESULTTYPE	SetCharHeight(uint8 height); 	
		
/*!  
\param pText text item to add
\param index (optional) position at which to add the new item.

Use this function to add a text item to the CUIList widget.  If \e index
is \b -1, the item will be added at the end of the list.  If \e index is
greater than the number of items currently in the list, the new item will
be added at the end.

Used for: Text and UI.   */	
		virtual CUI_RESULTTYPE	AddItem(const char* pText, int32 index = -1);

/*!  
\param index index of item to remove

Use this function to remove a list item.  \e Index is a zero-based (i.e., the
first item is index zero).

Used for: Text and UI.   */	
		virtual CUI_RESULTTYPE	RemoveItem(int32 index);

/*!  
\param pText text item to find

Use this function to locate a text item in the list.  The entire string is
matched and the match is case sensitive.

\return index of the matching item, or -1 if the search item does not exist
in the list.

Used for: Text and UI.   */	
		virtual int32			FindItem(const char* pText);

/*!  
\param index item to retrieve

Use this function to retrieve the text of an item in the list.

\return item text

Used for: Text and UI.   */	
		virtual const char*		GetItem(int32 index);

/*!  
\param index index of the item to select

Use this function to set the current selection in the list.  Only one item can
be selected at a time.

Used for: Text and UI.   */	
		virtual CUI_RESULTTYPE	SetSelection(int32 index);

/*!  
Use this function to get the current selection in the list.  Only one item can
be selected at a time.

\return index of the currently selected item.

Used for: Text and UI.   */	
		virtual int32			GetSelection();

/*!  
Use this function to get the total number of items in the list.

\return number of items

Used for: Text and UI.   */	
		virtual uint32			GetItemCount();

/*!  
\param x horizontal coordinate to test
\param y vertical coordinate to test

Use this function to test whether a point on the screen is within list items.

\return index of the item containing specified coords. or -1 if \e x and \e y
do not lie inside an item's bounding box.

Used for: Text and UI.   */	
		virtual int32			QueryPoint(float x, float y);

/*!  
\param number number of lines to scroll

Use this function to scroll the list up (\e number is negative) or down 
(\e number is positive).  You cannot scroll past the beginning or the end
of the list.

Used for: Text and UI.   */	
		virtual CUI_RESULTTYPE	Scroll(int32 number);

};

#endif //__CUILIST_H__

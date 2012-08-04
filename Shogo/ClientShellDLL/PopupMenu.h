#ifndef __POPUPMENU_H
#define __POPUPMENU_H

#include "clientheaders.h"
#include "DynArray.h"

class ILTClient;
class CRiotMenu;

enum PMIType
{
	NoType,
	KeyConfig,
	YesNo,
	Multiple,
	List,
	PopupMenu,
	Slider,
	Edit,
};

#define BORDERSIZE			6
#define CURSORBLINKTIME		1.0f

struct POPUPMENUITEM
{
	POPUPMENUITEM()	{ nHeight = 0; nSelectionPoint = 0; nSecondColumn = 0; hSurface = NULL; 
					  hSelected = NULL; nType = NoType; nID = 0; bEnabled = LTTRUE; nData = 0; pData = NULL; }
		
	int				nHeight;				// height to reserve for this item in the popup window
	int				nSelectionPoint;		// y coord (from top) where selection indicator should point
	int				nSecondColumn;			// x coord (from left) of second column (for editable items)
	HSURFACE		hSurface;				// surface containing image of item
	HSURFACE		hSelected;				// surface containing selected image of item
	
	PMIType			nType;					// Popup Menu Item Type (key config, yes/no toggle, etc.)
	int				nID;					// item ID - returned by SelectCurrentItem() when user presses enter

	LTBOOL			bEnabled;				// is the item enabled?

	uint32			nData;					// user-defined data
	void*			pData;					// user-defined data

		// for edit controls, pData is a pointer to the string, and nData is the number of bytes allocated for the string
		// for multiple option controls, pData is a pointer to a string delimeted with "|", and nData is the currently selected item
		// for keyconfig controls, pData is a pointer to the string that describes the key bound to that control and nData is
		//     the amount of string space allocated in pData
		// for slider controls, pData is a pointer to the slider object (see SLIDER.H)
		// for popup menu items, pData is a pointer to the popup menu object

};

class CPopupMenu
{
public:

	CPopupMenu (int nLeft, int nTop, int nWidth, int nHeight);
	~CPopupMenu();

	LTBOOL			Init (ILTClient* pClientDE, CRiotMenu* pRiotMenu);
	void			Draw (HSURFACE hScreen);

	void			AddItem (POPUPMENUITEM* ppmi);

	LTBOOL			ScrollUp();
	LTBOOL			ScrollDown();
	LTBOOL			PageUp();
	LTBOOL			PageDown();
	int				SelectCurrentItem();

	int				Width()								{ return m_nRight - m_nLeft + 1; }
	int				Height()							{ return m_nBottom - m_nTop + 1; }
	void			SetLeftMargin (int nMargin)			{ m_nLeftMargin = nMargin; }
	LTBOOL			IsEditing()							{ return m_bEditing; }
	LTBOOL			IsConfiguring()						{ return m_bConfiguring; }
	void			ResetSelection()					{ m_nSelection = 0; m_nFirstItem = 0; }
	CPopupMenu*		GetParentPopup()					{ return m_pParentPopup; }
	void			SetParentPopup (CPopupMenu* pMenu)	{ m_pParentPopup = pMenu; }
	uint32			GetMenuSelection()					{ return m_nSelection; }
	POPUPMENUITEM*	GetMenuItem (int nItem)				{ return &m_itemArray[nItem]; }
	void			SetTitleSurface (HSURFACE hTitle)	{ m_hTitle = hTitle; }

	void			ProcessKey (int nKey, int nRep);

public:

	uint32						m_nID;

protected:

	ILTClient*					m_pClientDE;
	CRiotMenu*					m_pRiotMenu;

	CPopupMenu*					m_pParentPopup;			// parent popup menu (if this is the child of a popup menu item)

	int							m_nLeft;
	int							m_nTop;
	int							m_nRight;
	int							m_nBottom;

	int							m_nLeftMargin;
	int							m_nTopMargin;

	CDynArray<POPUPMENUITEM>	m_itemArray;
	uint32						m_nItems;
	uint32						m_nFirstItem;
	uint32						m_nLastItem;

	uint32						m_nSelection;

	HSURFACE					m_hTitle;
	HSURFACE					m_hBackground;
	HSURFACE					m_hCursor;
	uint32						m_cxCursor;

	LTBOOL						m_bConfiguring;
	LTBOOL						m_bEditing;
	char*						m_strEdit;
};


#endif
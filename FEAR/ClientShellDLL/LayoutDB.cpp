// ----------------------------------------------------------------------- //
//
// MODULE  : LayoutDB.cpp
//
// PURPOSE : Layout Database Definition
//
// CREATED : 01/13/04
//
// (c) 1999-2004 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //


#include "stdafx.h"
#include "LayoutDB.h"

//
// Defines...
//
const char* const pszDefaultServerIcon = "Interface\\menu\\ServerIcon\\blank.dds";

//
// Globals...
//

CLayoutDB* g_pLayoutDB = NULL;

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLayoutDB::CLayoutDB()
//
//	PURPOSE:	Constructor...
//
// ----------------------------------------------------------------------- //

CLayoutDB::CLayoutDB()
:	CGameDatabaseMgr( ),
	m_hFontCat(NULL),
	m_hScreenCat(NULL),
	m_hMenuCat(NULL),
	m_hHUDCat(NULL)
{

}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLayoutDB::~CLayoutDB()
//
//	PURPOSE:	Destructor...
//
// ----------------------------------------------------------------------- //

CLayoutDB::~CLayoutDB()
{

}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLayoutDB::Init()
//
//	PURPOSE:	Initialize the database...
//
// ----------------------------------------------------------------------- //
bool CLayoutDB::Init( const char *szDatabaseFile /* = DB_Default_File  */ )
{
	if( !OpenDatabase( szDatabaseFile ))
		return false;

	// Set the global database pointer...
	g_pLayoutDB = this;

	// Get shared data record
	m_hShared = g_pLTDatabase->GetRecord(m_hDatabase,"Interface/Shared","Shared");


	// Get handles to all of the categories in the database...
	m_hFontCat = g_pLTDatabase->GetCategory(m_hDatabase,"Interface/Fonts");
	m_hScreenCat = g_pLTDatabase->GetCategory(m_hDatabase,"Interface/Screens");
	m_hMenuCat = g_pLTDatabase->GetCategory(m_hDatabase,"Interface/Menu");
	m_hHUDCat = g_pLTDatabase->GetCategory(m_hDatabase,"Interface/HUD");
	m_hLoadScreenCat = g_pLTDatabase->GetCategory(m_hDatabase,"Interface/LoadScreen");

	m_hServerIconCat = g_pLTDatabase->GetCategory(m_hDatabase,"Interface/ServerIcons");

	m_hXUIControlCat = g_pLTDatabase->GetCategory(m_hDatabase,"Interface/XUIControls");

	m_hXenonControlsCat = g_pLTDatabase->GetCategory(m_hDatabase,"Interface/XenonControlsConfig");

	m_hXenonControlsInteratorCat = g_pLTDatabase->GetCategory(m_hDatabase,"Interface/XenonControls");

	return true;
}

// ----------------------------------------------------------------------- //
//	ROUTINE:	CLayoutDB::GetPosition()
//	PURPOSE:	Get a position value from a record
// ----------------------------------------------------------------------- //
LTVector2n	CLayoutDB::GetPosition(HRECORD hRecord, const char* pszAttribute, uint32 nIndex /* = 0 */)
{
	LTVector2 v = GetVector2(hRecord,pszAttribute,nIndex);
	return LTVector2n(  int32(v.x),int32(v.y) );
}

// ----------------------------------------------------------------------- //
//	ROUTINE:	CLayoutDB::GetFont()
//	PURPOSE:	Get a font face name from a record
// ----------------------------------------------------------------------- //
const char* CLayoutDB::GetFont(HRECORD hRecord, const char* pszAttribute, uint32 nIndex /* = 0 */)
{
	HRECORD hFontRecord = GetRecordLink(hRecord,pszAttribute,nIndex);
	if (!hFontRecord)
		return "";
	return GetString(hFontRecord,"Face");
}

// ----------------------------------------------------------------------- //
//	ROUTINE:	CLayoutDB::GetRect()
//	PURPOSE:	Get a rectangle from a record
// ----------------------------------------------------------------------- //
LTRect2n	CLayoutDB::GetRect(HRECORD hRecord, const char* pszAttribute, uint32 nIndex /* = 0 */)
{
	LTVector4 v = GetVector4(hRecord,pszAttribute,nIndex);
	return LTRect2n(  int32(v.x),int32(v.y),int32(v.z),int32(v.w) );
}

// ----------------------------------------------------------------------- //
//	ROUTINE:	CLayoutDB::GetRect()
//	PURPOSE:	Get a rectangle from a record
// ----------------------------------------------------------------------- //
LTRect2f	CLayoutDB::GetRectF(HRECORD hRecord, const char* pszAttribute, uint32 nIndex /* = 0 */)
{
	LTVector4 v = GetVector4(hRecord,pszAttribute,nIndex);
	return LTRect2f(  v.x,v.y,v.z,v.w );
}


// ----------------------------------------------------------------------- //
//	ROUTINE:	CLayoutDB::GetColor()
//	PURPOSE:	Get a color value from a record
// ----------------------------------------------------------------------- //
uint32		CLayoutDB::GetColor(HRECORD hRecord, const char* pszAttribute, uint32 nIndex /* = 0 */)
{
	static uint32 argbDef = 0xFFFFFFFF;
	return uint32(GetInt32(hRecord,pszAttribute,nIndex,argbDef));
}


// ----------------------------------------------------------------------- //
//	ROUTINE:	CLayoutDB::GetListFont()
//	PURPOSE:	Get a font face name from a list attribute
// ----------------------------------------------------------------------- //
const char* CLayoutDB::GetListFont(HRECORD hRecord, uint32 nListIndex )
{
	HATTRIBUTE hLists = GetAttribute( hRecord, "Lists" );
	HATTRIBUTE hFont = GetStructAttribute( hLists, nListIndex, "Font" );
	HRECORD hFontRecord = g_pLTDatabase->GetRecordLink( hFont, 0, NULL );
	if (!hFontRecord)
		return "";
	return GetString(hFontRecord,"Face");
}

// ----------------------------------------------------------------------- //
//	ROUTINE:	CLayoutDB::GetListRect()
//	PURPOSE:	Get a rectangle from a list attribute
// ----------------------------------------------------------------------- //
LTRect2n	CLayoutDB::GetListRect(HRECORD hRecord, uint32 nListIndex )
{
	HATTRIBUTE hLists = GetAttribute( hRecord, "Lists" );
	HATTRIBUTE hRect = GetStructAttribute( hLists, nListIndex, "Rect" );
	LTVector4 v = g_pLTDatabase->GetVector4( hRect, 0, LTVector4( 0.0f, 0.0f, 0.0f, 0.0f ));
	return LTRect2n(  int32(v.x),int32(v.y),int32(v.z),int32(v.w) );
}

// ----------------------------------------------------------------------- //
//	ROUTINE:	CLayoutDB::GetListSize()
//	PURPOSE:	Get size for list font.
// ----------------------------------------------------------------------- //
int32 CLayoutDB::GetListSize(HRECORD hRecord, uint32 nListIndex )
{
	HATTRIBUTE hLists = GetAttribute( hRecord, "Lists" );
	HATTRIBUTE hSize = GetStructAttribute( hLists, nListIndex, "Size" );
	return g_pLTDatabase->GetInt32( hSize, 0, 0 );
}

// ----------------------------------------------------------------------- //
//	ROUTINE:	CLayoutDB::GetListColumnWidth()
//	PURPOSE:	Get horizontal size of columns in list.
// ----------------------------------------------------------------------- //
int32 CLayoutDB::GetListColumnWidth(HRECORD hRecord, uint32 nListIndex, uint32 nColumnIndex )
{
	HATTRIBUTE hLists = GetAttribute( hRecord, "Lists" );
	HATTRIBUTE hColumnWidths = GetStructAttribute( hLists, nListIndex, "ColumnWidths" );
	return g_pLTDatabase->GetInt32( hColumnWidths, nColumnIndex, 0 );
}

// ----------------------------------------------------------------------- //
//	ROUTINE:	CLayoutDB::GetListFrameTexture()
//	PURPOSE:	Get frame textures for list
// ----------------------------------------------------------------------- //
const char* CLayoutDB::GetListFrameTexture(HRECORD hRecord, uint32 nListIndex, uint32 nFrameIndex )
{
	HATTRIBUTE hLists = GetAttribute( hRecord, "Lists" );
	HATTRIBUTE hFrameTextures = GetStructAttribute( hLists, nListIndex, "FrameTexture" );
	return g_pLTDatabase->GetString( hFrameTextures, nFrameIndex, "" );
}

// ----------------------------------------------------------------------- //
//	ROUTINE:	CLayoutDB::GetListIndent()
//	PURPOSE:	Get how much items within the list should be indented from the upper left corner
// ----------------------------------------------------------------------- //
LTVector2n CLayoutDB::GetListIndent(HRECORD hRecord, uint32 nListIndex )
{
	HATTRIBUTE hLists = GetAttribute( hRecord, "Lists" );
	HATTRIBUTE hRect = GetStructAttribute( hLists, nListIndex, "Indent" );
	LTVector2 v = g_pLTDatabase->GetVector2( hRect, 0, LTVector2( 0.0f, 0.0f ));
	return LTVector2n(  int32(v.x),int32(v.y) );
}


// ----------------------------------------------------------------------- //
//	ROUTINE:	CLayoutDB::GetListFrameExpand()
//	PURPOSE:	Get how much the list's frame should be expanded.
// ----------------------------------------------------------------------- //
int32 CLayoutDB::GetListFrameExpand(HRECORD hRecord, uint32 nListIndex )
{
	HATTRIBUTE hLists = GetAttribute( hRecord, "Lists" );
	HATTRIBUTE hExpand = GetStructAttribute( hLists, nListIndex, "FrameExpand" );
	return g_pLTDatabase->GetInt32( hExpand, 0, 0 );
}

// ----------------------------------------------------------------------- //
//	ROUTINE:	CLayoutDB::GetListArrowSize()
//	PURPOSE:	Get size of scrolling arrows
// ----------------------------------------------------------------------- //
LTVector2n CLayoutDB::GetListArrowSize(HRECORD hRecord, uint32 nListIndex )
{
	HATTRIBUTE hLists = GetAttribute( hRecord, "Lists" );
	HATTRIBUTE hRect = GetStructAttribute( hLists, nListIndex, "ArrowSize" );
	LTVector2 v = g_pLTDatabase->GetVector2( hRect, 0, LTVector2( 0.0f, 0.0f ));
	return LTVector2n(  int32(v.x),int32(v.y) );
}

// ----------------------------------------------------------------------- //
//	ROUTINE:	CLayoutDB::GetScrollBarOffset()
//	PURPOSE:	Get the offset of the scrollbar from the top-right of the list
// ----------------------------------------------------------------------- //
LTVector2n CLayoutDB::GetScrollBarOffset(HRECORD hRecord, uint32 nListIndex )
{
	HATTRIBUTE hLists = GetAttribute( hRecord, "Lists" );
	HATTRIBUTE hOffset = GetStructAttribute( hLists, nListIndex, "ScrollBarOffset" );
	LTVector2 v = g_pLTDatabase->GetVector2( hOffset, 0, LTVector2( 0.0f, 0.0f ));
	return LTVector2n(  int32(v.x),int32(v.y) );
}

// ----------------------------------------------------------------------- //
//	ROUTINE:	CLayoutDB::GetHeaderCtrlOffset()
//	PURPOSE:	Get the offset of the header control from the top-left of the list
// ----------------------------------------------------------------------- //
LTVector2n CLayoutDB::GetHeaderCtrlOffset(HRECORD hRecord, uint32 nListIndex )
{
	HATTRIBUTE hLists = GetAttribute( hRecord, "Lists" );
	HATTRIBUTE hOffset = GetStructAttribute( hLists, nListIndex, "HeaderControlOffset" );
	LTVector2 v = g_pLTDatabase->GetVector2( hOffset, 0, LTVector2( 0.0f, 0.0f ));
	return LTVector2n(  int32(v.x),int32(v.y) );
}



// *********************************************************************** //
// Access to data from Shared category
// *********************************************************************** //

// ----------------------------------------------------------------------- //
//	ROUTINE:	CLayoutDB::GetHelpRect()
//	PURPOSE:	Get the rectangle used for help text
// ----------------------------------------------------------------------- //
LTRect2n CLayoutDB::GetHelpRect()
{
	LTVector4 v = GetVector4(m_hShared,"HelpRect");
	return LTRect2n(  int32(v.x),int32(v.y),int32(v.z),int32(v.w) );
}

// ----------------------------------------------------------------------- //
//	ROUTINE:	CLayoutDB::GetHelpFont()
//	PURPOSE:	Get the font used for help text
// ----------------------------------------------------------------------- //
const char*	CLayoutDB::GetHelpFont()
{
	HRECORD hFontRecord = GetRecordLink(m_hShared,"HelpFont");
	return GetString(hFontRecord,"Face");
}

// ----------------------------------------------------------------------- //
//	ROUTINE:	CLayoutDB::GetHelpSize()
//	PURPOSE:	Get the text size used for help text
// ----------------------------------------------------------------------- //
uint32 CLayoutDB::GetHelpSize()
{
	return GetInt32(m_hShared,"HelpSize");
}

// ----------------------------------------------------------------------- //
//	ROUTINE:	CLayoutDB::GetNumFontFiles()
//	PURPOSE:	Get the number of font files to load on start up
// ----------------------------------------------------------------------- //
uint8 CLayoutDB::GetNumFontFiles()
{
	uint32 nValue = GetNumValues(m_hShared,"FontFile");
	ASSERT( nValue == ( uint8 )nValue );
	return ( uint8 )nValue;
}

// ----------------------------------------------------------------------- //
//	ROUTINE:	CLayoutDB::GetFontFile()
//	PURPOSE:	Get a font file from its index
// ----------------------------------------------------------------------- //
const char* CLayoutDB::GetFontFile(uint8 nIndex)
{
	return GetString(m_hShared,"FontFile",nIndex);
}


// ----------------------------------------------------------------------- //
//	ROUTINE:	CLayoutDB::GetHUDFont()
//	PURPOSE:	Get the font used for HUD text
// ----------------------------------------------------------------------- //
const char*	CLayoutDB::GetHUDFont()
{
	HRECORD hFontRecord = GetRecordLink(m_hShared,"HUDFont");
	return GetString(hFontRecord,"Face");
}

// ----------------------------------------------------------------------- //
//	ROUTINE:	CLayoutDB::GetSliderTex()
//	PURPOSE:	Get texture used for building sliders
// ----------------------------------------------------------------------- //
const char*	CLayoutDB::GetSliderTex(uint8 nIndex)
{
	return GetString(m_hShared,"SliderTexture", nIndex);
}

// ----------------------------------------------------------------------- //
//	ROUTINE:	CLayoutDB::GetScrollBarTex()
//	PURPOSE:	Get texture used for building sliders
// ----------------------------------------------------------------------- //
const char*	CLayoutDB::GetScrollBarTex(uint8 nIndex)
{
	return GetString(m_hShared,"ScrollBarTexture", nIndex);
}

// ----------------------------------------------------------------------- //
//	ROUTINE:	CLayoutDB::GetScreenBackFont()
//	PURPOSE:	Get the font used for the back button
// ----------------------------------------------------------------------- //
const char* CLayoutDB::GetScreenBackFont()
{
	HRECORD hFontRecord = GetRecordLink(m_hShared,"ScreenBackFont");
	return GetString(hFontRecord,"Face");
}

// ----------------------------------------------------------------------- //
//	ROUTINE:	CLayoutDB::GetScreenBackSize()
//	PURPOSE:	Get the font size for the back button
// ----------------------------------------------------------------------- //
uint32		CLayoutDB::GetScreenBackSize()
{
	return uint32(GetInt32(m_hShared,"ScreenBackSize"));
}


// ----------------------------------------------------------------------- //
//	ROUTINE:	CLayoutDB::GetScreenBackRect()
//	PURPOSE:	Get the rectangle used for the back button
// ----------------------------------------------------------------------- //
LTRect2n	CLayoutDB::GetScreenBackRect()
{
	LTVector4 v = GetVector4(m_hShared,"ScreenBackRect");
	return LTRect2n(  int32(v.x),int32(v.y),int32(v.z),int32(v.w) );
}

// ----------------------------------------------------------------------- //
//	ROUTINE:	CLayoutDB::GetTeamColor()
//	PURPOSE:	Get the color associated with the specified team
// ----------------------------------------------------------------------- //
uint32		CLayoutDB::GetTeamColor(uint32 nTeam)
{
	return GetColor(m_hShared,"TeamColor",nTeam);
}


// ----------------------------------------------------------------------- //
//	ROUTINE:	CLayoutDB::GetDialogFrame()
//	PURPOSE:	Get texture used for dialog box backgrounds
// ----------------------------------------------------------------------- //
const char* CLayoutDB::GetDialogFrame()
{
	return GetString(m_hShared,"DialogFrame");
}

// ----------------------------------------------------------------------- //
//	ROUTINE:	CLayoutDB::GetDialogFontFace()
//	PURPOSE:	Get font used for dialog boxes
// ----------------------------------------------------------------------- //
const char* CLayoutDB::GetDialogFontFace()
{
	HRECORD hFontRecord = GetRecordLink(m_hShared,"DialogFont");
	return GetString(hFontRecord,"Face");
}

// ----------------------------------------------------------------------- //
//	ROUTINE:	CLayoutDB::GetDialogFontSize()
//	PURPOSE:	Get font size used for dialog boxes
// ----------------------------------------------------------------------- //
uint32		CLayoutDB::GetDialogFontSize()
{
	return uint32(GetInt32(m_hShared,"DialogSize"));
}

// ----------------------------------------------------------------------- //
//	ROUTINE:	CLayoutDB::GetHeaderCtrlSize()
//	PURPOSE:	The height of a header control
// ----------------------------------------------------------------------- //
uint32		CLayoutDB::GetHeaderCtrlSize()
{
	return uint32(GetInt32(m_hShared,"HeaderControlSize"));
}

// ----------------------------------------------------------------------- //
//	ROUTINE:	CLayoutDB::GetHeaderCtrlIndent()
//	PURPOSE:	The additional indentation used by the text displayed on header controls
// ----------------------------------------------------------------------- //
uint32		CLayoutDB::GetHeaderCtrlIndent()
{
	return uint32(GetInt32(m_hShared,"HeaderControlIndent"));
}

// ----------------------------------------------------------------------- //
//	ROUTINE:	CLayoutDB::GetScrollBarSize()
//	PURPOSE:	Get width of a vertical scrollbar
// ----------------------------------------------------------------------- //
uint32		CLayoutDB::GetScrollBarSize()
{
	return uint32(GetInt32(m_hShared,"ScrollBarSize"));
}

// ----------------------------------------------------------------------- //
//	ROUTINE:	CLayoutDB::GetHeaderCtrlBackgroundColor()
//	PURPOSE:	The default color for a header background
// ----------------------------------------------------------------------- //
uint32		CLayoutDB::GetHeaderCtrlBackgroundColor()
{
	return uint32(GetInt32(m_hShared,"HeaderBackgroundColor"));
}

// ----------------------------------------------------------------------- //
//	ROUTINE:	CLayoutDB::GetHeaderCtrlHighlightColor()
//	PURPOSE:	The default color for a header focus/highlight
// ----------------------------------------------------------------------- //
uint32		CLayoutDB::GetHeaderCtrlHighlightColor()
{
	return uint32(GetInt32(m_hShared,"HeaderHighlightColor"));
}

// ----------------------------------------------------------------------- //
//	ROUTINE:	CLayoutDB::GetHeaderCtrlSortedColor()
//	PURPOSE:	The default color for a header focus/highlight
// ----------------------------------------------------------------------- //
uint32		CLayoutDB::GetHeaderCtrlSortedColor()
{
	return uint32(GetInt32(m_hShared,"HeaderSortedColor"));
}

// ----------------------------------------------------------------------- //
//	ROUTINE:	CLayoutDB::GetScrollBarDelay()
//	PURPOSE:	Get the initial scrolling delay of a scrollbar
// ----------------------------------------------------------------------- //
float		CLayoutDB::GetScrollBarDelay()
{
	return GetFloat(m_hShared,"ScrollBarScrollSpeed",1);
}

// ----------------------------------------------------------------------- //
//	ROUTINE:	CLayoutDB::GetScrollBarSpeed()
//	PURPOSE:	Get the scrolling speed of a scrollbar
// ----------------------------------------------------------------------- //
float		CLayoutDB::GetScrollBarSpeed()
{
	return GetFloat(m_hShared,"ScrollBarScrollSpeed",0);
}

// ----------------------------------------------------------------------- //
//	ROUTINE:	CLayoutDB::GetHighlightGlowAlpha()
//	PURPOSE:	Get alpha value for glow effect on highlighted text
// ----------------------------------------------------------------------- //
float		CLayoutDB::GetHighlightGlowAlpha()
{
	return GetFloat(m_hShared,"HighlightGlowAlpha");
}

// ----------------------------------------------------------------------- //
//	ROUTINE:	CLayoutDB::GetHighlightGlowSize()
//	PURPOSE:	Get size of glow effect on highlighted text
// ----------------------------------------------------------------------- //
LTVector2	CLayoutDB::GetHighlightGlowSize()
{
	return GetVector2(m_hShared,"HighlightGlowSize");
}


// ----------------------------------------------------------------------- //
//	ROUTINE:	CLayoutDB::GetDefaultCursor()
//	PURPOSE:	The default cursor recordlink
// ----------------------------------------------------------------------- //
HRECORD		CLayoutDB::GetDefaultCursor()
{
	return GetRecordLink(m_hShared,"DefaultCursor");
}

// ----------------------------------------------------------------------- //
//	ROUTINE:	CLayoutDB::GetListSelectedColumnColor()
//	PURPOSE:	The default color for a selected column
// ----------------------------------------------------------------------- //
uint32		CLayoutDB::GetListSelectedColumnColor()
{
	return uint32(GetInt32(m_hShared,"ListSelectedColumnColor"));
}

// ----------------------------------------------------------------------- //
//	ROUTINE:	CLayoutDB::GetListHighlightColor()
//	PURPOSE:	The default color for a highlighted row in a list control
// ----------------------------------------------------------------------- //
uint32		CLayoutDB::GetListHighlightColor()
{
	return uint32(GetInt32(m_hShared,"ListHighlightColor"));
}

// ----------------------------------------------------------------------- //
//	ROUTINE:	CLayoutDB::GetListBackgroundColumnColor()
//	PURPOSE:	The default color for the background of a column
// ----------------------------------------------------------------------- //
uint32		CLayoutDB::GetListBackgroundColumnColor()
{
	return uint32(GetInt32(m_hShared,"ListBackgroundColumnColor"));
}

// *********************************************************************** //



// ----------------------------------------------------------------------- //
//	ROUTINE:	CLayoutDB::GetScreenRecord()
//	PURPOSE:	Get the record associated with a particular screen
// ----------------------------------------------------------------------- //
HRECORD		CLayoutDB::GetScreenRecord(eScreenID screenId)
{
	HRECORD hRec = g_pLTDatabase->GetRecord(m_hScreenCat,g_pInterfaceMgr->GetScreenMgr()->GetScreenName(screenId));

	//if no record exists for this screen, use the values in the default record
	if (!hRec)
		hRec = g_pLTDatabase->GetRecord(m_hScreenCat,"BaseScreen");


	LTASSERT(hRec,"Screen record does not exist.");

	return hRec;
}

// ----------------------------------------------------------------------- //
//	ROUTINE:	CLayoutDB::GetHUDRecord()
//	PURPOSE:	Get the record associated with a particular HUD item
// ----------------------------------------------------------------------- //
HRECORD		CLayoutDB::GetHUDRecord(const char* pszRecordName)
{
	HRECORD hRec = g_pLTDatabase->GetRecord(m_hHUDCat,pszRecordName);

	LTASSERT(hRec,"HUD record does not exist.");

	return hRec;
}

// ----------------------------------------------------------------------- //
//	ROUTINE:	CLayoutDB::GetMenuRecord()
//	PURPOSE:	Get the record associated with a particular menu
// ----------------------------------------------------------------------- //
HRECORD CLayoutDB::GetMenuRecord(eMenuID menuID) const
{

	HRECORD hRec = NULL;

	if (g_pInterfaceMgr->GetUserMenuMgr().GetMenu(menuID))
	{
		hRec = g_pLTDatabase->GetRecord(m_hMenuCat,g_pInterfaceMgr->GetUserMenuMgr().GetMenuName(menuID));
	}
	else
	{
		hRec = g_pLTDatabase->GetRecord(m_hMenuCat,g_pInterfaceMgr->GetSystemMenuMgr().GetMenuName(menuID));
	}
	

	//if no record exists for this menu, use the values in the default record
	if (!hRec)
		hRec = g_pLTDatabase->GetRecord(m_hMenuCat,"Menu");


	LTASSERT(hRec,"Menu record does not exist.");

	return hRec;
}



// ----------------------------------------------------------------------- //
//	ROUTINE:	CLayoutDB::GetXUIConfigRecord()
//	PURPOSE:	Get the XUI Configuration record
// ----------------------------------------------------------------------- //
HRECORD		CLayoutDB::GetXUIConfigRecord()
{
	return g_pLTDatabase->GetRecord(m_hDatabase,"Interface/XUIConfig","Config");
}

// ----------------------------------------------------------------------- //
//	ROUTINE:	CLayoutDB::GetXUIControlRecord()
//	PURPOSE:	Get the record associated with a particular Overlay
// ----------------------------------------------------------------------- //
HRECORD		CLayoutDB::GetXUIControlRecord(const char* pszRecordName)
{
	HRECORD hRec = g_pLTDatabase->GetRecord(m_hXUIControlCat,pszRecordName);

	LTASSERT(hRec,"Overlay record does not exist.");

	return hRec;
}

// ----------------------------------------------------------------------- //
//	ROUTINE:	CLayoutDB::GetXUIControlRecord()
//	PURPOSE:	Get the record associated with a particular Overlay
// ----------------------------------------------------------------------- //
HRECORD		CLayoutDB::GetXUIControlRecord(uint32 nIndex)
{
	HRECORD hRec = g_pLTDatabase->GetRecordByIndex(m_hXUIControlCat,nIndex);

	LTASSERT(hRec,"Overlay record does not exist.");

	return hRec;
}

// ----------------------------------------------------------------------- //
//	ROUTINE:	CLayoutDB::GetXUIControlRecord()
//	PURPOSE:	Get the record associated with a particular Overlay
// ----------------------------------------------------------------------- //
HRECORD		CLayoutDB::GetXenonControlsInteratorRecord(uint32 nIndex)
{
	HRECORD hRec = g_pLTDatabase->GetRecordByIndex(m_hXenonControlsInteratorCat,nIndex);

	LTASSERT(hRec,"Overlay record does not exist.");

	return hRec;
}

// ----------------------------------------------------------------------- //
//	ROUTINE:	CLayoutDB::GetNumXUIControlRecords()
//	PURPOSE:	Get the number of overlays
// ----------------------------------------------------------------------- //
uint32		CLayoutDB::GetNumXUIControlRecords()
{
	return g_pLTDatabase->GetNumRecords(m_hXUIControlCat);
}

// ----------------------------------------------------------------------- //
//	ROUTINE:	CLayoutDB::GetXUIControlRecord()
//	PURPOSE:	Get the record associated with a particular Overlay
// ----------------------------------------------------------------------- //
HRECORD		CLayoutDB::GetXenonControlsRecord(const char* pszRecordName)
{
	HRECORD hRec = g_pLTDatabase->GetRecord(m_hXenonControlsCat,pszRecordName);

	LTASSERT(hRec,"Overlay record does not exist.");

	return hRec;
}

// ----------------------------------------------------------------------- //
//	ROUTINE:	CLayoutDB::GetXUIControlRecord()
//	PURPOSE:	Get the record associated with a particular Overlay
// ----------------------------------------------------------------------- //
HRECORD		CLayoutDB::GetXenonControlsRecord(uint32 nIndex)
{
	HRECORD hRec = g_pLTDatabase->GetRecordByIndex(m_hXenonControlsCat,nIndex);

	LTASSERT(hRec,"Overlay record does not exist.");

	return hRec;
}

// ----------------------------------------------------------------------- //
//	ROUTINE:	CLayoutDB::GetNumXUIControlRecords()
//	PURPOSE:	Get the number of overlays
// ----------------------------------------------------------------------- //
uint32		CLayoutDB::GetNumXenonControlsRecords()
{
	return g_pLTDatabase->GetNumRecords(m_hXenonControlsCat);
}

// ----------------------------------------------------------------------- //
//	ROUTINE:	CLayoutDB::GetLoadScreenRecord()
//	PURPOSE:	Get the record associated with a particular LoadScreen
// ----------------------------------------------------------------------- //
HRECORD CLayoutDB::GetLoadScreenRecord(const char* pszRecordName)
{
	HRECORD hRec = g_pLTDatabase->GetRecord(m_hLoadScreenCat,pszRecordName);
	LTASSERT(hRec,"LoadScreen record does not exist.");

	return hRec;
}

// ----------------------------------------------------------------------- //
//	ROUTINE:	CLayoutDB::GetHeaderIcon()
//	PURPOSE:	Get the header icon for a particular option
// ----------------------------------------------------------------------- //
const char* CLayoutDB::GetHeaderIcon(const char* pszRecordName) const
{
	HRECORD hRec = g_pLTDatabase->GetRecord(m_hServerIconCat,pszRecordName);

	return GetString(hRec, "Header", 0, pszDefaultServerIcon);
}

const char* CLayoutDB::GetHighlightIcon(const char* pszRecordName) const
{
	HRECORD hRec = g_pLTDatabase->GetRecord(m_hServerIconCat,pszRecordName);

	return GetString(hRec, "Highlight", 0, pszDefaultServerIcon);
}

const char* CLayoutDB::GetServerIcon(const char* pszRecordName,uint32 nIndex) const
{
	HRECORD hRec = g_pLTDatabase->GetRecord(m_hServerIconCat,pszRecordName);

	return GetString(hRec, "Icons", nIndex, pszDefaultServerIcon);
}





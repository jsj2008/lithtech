// ----------------------------------------------------------------------- //
//
// MODULE  : LayoutDB.h
//
// PURPOSE : Layout Database Definition
//
// CREATED : 01/13/04
//
// (c) 1999-2004 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __LAYOUTDB_H__
#define __LAYOUTDB_H__


//
// Includes...
//

#include "GameDatabaseMgr.h"
#include "ResourceExtensions.h"
#include "ScreenMgr.h"

//
// Includes...
//

#include "GameDatabaseMgr.h"


//
// Defines...
//


//------------------------------------------------------
// Screen record attributes
const char* const LDB_ScreenTitlePos =			"TitlePos";
const char* const LDB_ScreenTitleFont =			"TitleFont";
const char* const LDB_ScreenTitleSize =			"TitleSize";
const char* const LDB_ScreenTitleColor =		"TitleColor";
const char* const LDB_ScreenRect =				"ScreenRect";
const char* const LDB_ScreenFontFace =			"FontFace";
const char* const LDB_ScreenFontSize =			"FontSize";
const char* const LDB_ScreenItemSpace =			"ItemSpace";
const char* const LDB_ScreenSelectedColor =		"SelectedColor";
const char* const LDB_ScreenNonSelectedColor =	"NonSelectedColor";
const char* const LDB_ScreenDisabledColor =		"DisabledColor";
const char* const LDB_ScreenColumnWidths =		"ColumnWidths";
const char* const LDB_ScreenFrameRect =			"FrameRect";
const char* const LDB_ScreenFrameTexture =		"FrameTexture";
const char* const LDB_ScreenDialogFont =		"DialogFont";
const char* const LDB_ScreenDialogSize =		"DialogSize";
const char* const LDB_ScreenDialogRect =		"DialogRect";
const char* const LDB_ScreenDialogFrame =		"DialogFrame";
const char* const LDB_ScreenAdditionalInt =		"AdditionalInt";
const char* const LDB_ScreenAdditionalPos =		"AdditionalPos";
const char* const LDB_ScreenAddColor =			"AdditionalColor";
const char* const LDB_ScreenAddTex =			"AdditionalTexture";

//------------------------------------------------------
// HUD record attributes
const char* const LDB_HUDBasePos =				"BasePos";
const char* const LDB_HUDTextOffset =			"TextOffset";
const char* const LDB_HUDTextSize =				"TextSize";
const char* const LDB_HUDTextColor =			"TextColor";
const char* const LDB_HUDTextAlignment =		"TextAlignment";
const char* const LDB_HUDIconOffset =			"IconOffset";
const char* const LDB_HUDIconSize =				"IconSize";
const char* const LDB_HUDIconColor =			"IconColor";
const char* const LDB_HUDIconTexture =			"IconTexture";
const char* const LDB_HUDFadeDelay =			"FadeDelay";
const char* const LDB_HUDFadeTime =				"FadeTime";
const char* const LDB_bHUDSinglePlayerFade =	"SinglePlayerFade";
const char* const LDB_bHUDMultiplayerFade =		"MultiplayerFade";
const char* const LDB_HUDAddPoint =				"AdditionalPoint";
const char* const LDB_HUDAddRect =				"AdditionalRect";
const char* const LDB_HUDAddInt =				"AdditionalInt";
const char* const LDB_HUDAddFloat =				"AdditionalFloat";
const char* const LDB_HUDAddColor =				"AdditionalColor";
const char* const LDB_HUDAddTex =				"AdditionalTexture";
const char* const LDB_vMenuAddPoint =				"AdditionalPoint";
const char* const LDB_rMenuAddRect =				"AdditionalRect";
const char* const LDB_nMenuAddInt =				"AdditionalInt";
const char* const LDB_fMenuAddFloat =				"AdditionalFloat";
const char* const LDB_cMenuAddColor =				"AdditionalColor";
const char* const LDB_sMenuAddTex =				"AdditionalTexture";


class CLayoutDB;
extern CLayoutDB* g_pLayoutDB;

class CLayoutDB : public CGameDatabaseMgr
{
	DECLARE_SINGLETON( CLayoutDB );

public :	// Methods...

	bool	Init( const char *szDatabaseFile = DB_Default_File );
	void	Term() {};

	//------------------------------------------------------
	// General access help functions
	LTRect2n	GetRect(HRECORD hRecord, const char* pszAttribute, uint32 nIndex = 0);
	LTRect2f	GetRectF(HRECORD hRecord, const char* pszAttribute, uint32 nIndex = 0);
	LTVector2n	GetPosition(HRECORD hRecord, const char* pszAttribute, uint32 nIndex = 0);
	const char*	GetFont(HRECORD hRecord, const char* pszAttribute, uint32 nIndex = 0);
	uint32		GetColor(HRECORD hRecord, const char* pszAttribute, uint32 nIndex = 0);

	//------------------------------------------------------
	// access functions for lists.
	LTRect2n	GetListRect(HRECORD hRecord, uint32 nListIndex );
	const char*	GetListFont(HRECORD hRecord, uint32 nListIndex );
	int32		GetListSize(HRECORD hRecord, uint32 nListIndex );
	int32		GetListColumnWidth(HRECORD hRecord, uint32 nListIndex, uint32 nColumnIndex );
	const char* GetListFrameTexture(HRECORD hRecord, uint32 nListIndex, uint32 nFrameIndex );
	LTVector2n	GetListIndent(HRECORD hRecord, uint32 nListIndex );
	int32		GetListFrameExpand(HRECORD hRecord, uint32 nListIndex );
	LTVector2n	GetListArrowSize(HRECORD hRecord, uint32 nListIndex );

	//------------------------------------------------------
	// access functions for scrollbars.
	LTVector2n	GetScrollBarOffset(HRECORD hRecord, uint32 nListIndex );


	//------------------------------------------------------
	// access functions for headers.
	LTVector2n	GetHeaderCtrlOffset(HRECORD hRecord, uint32 nListIndex );


	//------------------------------------------------------
	// Access to data from Shared category
	LTRect2n	GetHelpRect();
	const char*	GetHelpFont();
	uint32		GetHelpSize();
	uint8		GetNumFontFiles();
	const char* GetFontFile(uint8 nIndex);
	const char*	GetHUDFont();
	const char*	GetSliderTex(uint8 nIndex);
	const char*	GetScrollBarTex(uint8 nIndex);
	const char* GetScreenBackFont();
	uint32		GetScreenBackSize();
	LTRect2n	GetScreenBackRect();
	uint32		GetTeamColor(uint32 nTeam);
	const char* GetDialogFrame();
	const char* GetDialogFontFace();
	uint32		GetDialogFontSize();
	float		GetHighlightGlowAlpha();
	LTVector2	GetHighlightGlowSize();
	uint32		GetScrollBarSize();
	float		GetScrollBarDelay();
	float		GetScrollBarSpeed();
	uint32		GetHeaderCtrlSize();
	uint32		GetHeaderCtrlIndent();
	uint32		GetHeaderCtrlBackgroundColor();
	uint32		GetHeaderCtrlHighlightColor();
	uint32		GetHeaderCtrlSortedColor();
	HRECORD		GetDefaultCursor();
	uint32		GetListBackgroundColumnColor();
	uint32		GetListSelectedColumnColor();
	uint32		GetListHighlightColor();

	//------------------------------------------------------
	// Access to records from Screen category
	HRECORD		GetScreenRecord(eScreenID screenId);

	//------------------------------------------------------
	// Access to records from HUD Category
	HRECORD		GetHUDRecord(const char* pszRecordName);

	//------------------------------------------------------
	// Access to the default menu record
	HRECORD		GetMenuRecord(eMenuID menuID) const;

	//------------------------------------------------------
	// Access to records from LoadScreen Category
	HRECORD		GetLoadScreenRecord(const char* pszRecordName);

	//------------------------------------------------------
	// Access to record from Shared Category
	HRECORD		GetSharedRecord() const { return m_hShared; }

	//------------------------------------------------------
	// Access to record from Cursors Category
	HRECORD		GetCursorRecord( const char* pszRecordName );

	//------------------------------------------------------
	// Access to record from XUIConfig Category
	HRECORD		GetXUIConfigRecord();

	//------------------------------------------------------
	// Access to records from XUIControls Category
	HRECORD		GetXUIControlRecord(const char* pszRecordName);
	HRECORD		GetXUIControlRecord(uint32 nIndex);
	uint32		GetNumXUIControlRecords();

	//------------------------------------------------------
	// Access to records from XenonControls Category
	HRECORD		GetXenonControlsRecord(const char* pszRecordName);
	HRECORD		GetXenonControlsRecord(uint32 nIndex);
	uint32		GetNumXenonControlsRecords();

	//------------------------------------------------------
	// Access to records from XenonControlsIterator Category
	HRECORD		CLayoutDB::GetXenonControlsInteratorRecord(uint32 nIndex);

	//------------------------------------------------------
	// Access to data from ServerIcon Category
	const char* GetHeaderIcon(const char* pszRecordName) const;
	const char* GetHighlightIcon(const char* pszRecordName) const;
	const char* GetServerIcon(const char* pszRecordName,uint32 nIndex) const;


private	:	// Members...

	HRECORD		m_hShared;

	HCATEGORY	m_hFontCat;
	HCATEGORY	m_hScreenCat;
	HCATEGORY	m_hMenuCat;
	HCATEGORY	m_hHUDCat;
	HCATEGORY	m_hLoadScreenCat;
	HCATEGORY	m_hServerIconCat;
	HCATEGORY	m_hXUIControlCat;
	HCATEGORY	m_hXenonControlsCat;
	HCATEGORY	m_hXenonControlsInteratorCat;
};


#endif //__LAYOUTDB_H__

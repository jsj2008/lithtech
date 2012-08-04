#include "bdefs.h"
#include "optionsmisc.h"

COptionsMisc::COptionsMisc() :
	m_bParentFolder(true)
{
}

COptionsMisc::~COptionsMisc()
{
}

// Load/Save
BOOL COptionsMisc::Load()
{
	m_bParentFolder = GetBoolValue	("ParentFolder", TRUE) ? true : false;

	// Load Icons Info
	m_bShowIcons = GetBoolValue ("ShowIcons", TRUE) ? true : false;

	// Load Thumbnails Info
	m_bShowThumbnails = GetBoolValue ("ShowThumbnails", TRUE) ? true : false;

	SetAutoExtractIcons(GetBoolValue("AutoExtractIcons", TRUE) ? true : false);
	SetAutoExtractClassHelp(GetBoolValue("AutoExtractClassHelp", TRUE) ? true : false);

	m_bShowUndoWarnings = GetBoolValue ("ShowUndoWarnings", TRUE) ? true : false;

	m_bShowFullPath = GetBoolValue("ShowFullPathInTitle", FALSE) ? true : false;

	m_bAutoLoadProj = GetBoolValue("AutoLoadProj", TRUE) ? true : false;

	m_bDefaultCompressed = GetBoolValue("DefaultCompressed", TRUE) ? true : false;
	m_bLoadLYTFile = GetBoolValue("LoadLYTFile", TRUE) ? true : false;
	m_bUndoFreezeHide = GetBoolValue("UndoFreezeHide", FALSE) ? true : false;

	// Load undo data
	m_dwUndos			= GetDWordValue("NumUndos", 40);

	SetVectorEditIncrement(atof(GetStringValue("VectorEditIncrement", "0.1")));
	SetVectorEditAutoApply(GetBoolValue("VectorEditAutoApply", TRUE) ? true : false);
	SetRotationEditAutoApply(GetBoolValue("RotationEditAutoApply", TRUE) ? true : false);

	return TRUE;
}

BOOL COptionsMisc::Save()
{
	SetBoolValue("ParentFolder",	m_bParentFolder ? TRUE : FALSE);

	// Save Thumbnails Info
	SetBoolValue("ShowIcons", m_bShowIcons ? TRUE : FALSE);

	// Save Thumbnails Info
	SetBoolValue("ShowThumbnails", m_bShowThumbnails ? TRUE : FALSE);

	SetBoolValue("AutoExtractIcons", IsAutoExtractIcons() ? TRUE : FALSE);
	SetBoolValue("AutoExtractClassHelp", IsAutoExtractClassHelp() ? TRUE : FALSE);

	SetBoolValue("ShowUndoWarnings", GetShowUndoWarnings() ? TRUE : FALSE);

	SetBoolValue("ShowFullPathInTitle", IsShowFullPathTitle() ? TRUE : FALSE);

	SetBoolValue("AutoLoadProj", IsAutoLoadProj() ? TRUE : FALSE);

	SetBoolValue("DefaultCompressed", IsDefaultCompressed() ? TRUE : FALSE);

	SetBoolValue("LoadLYTFile", IsLoadLYTFile() ? TRUE : FALSE);
	SetBoolValue("UndoFreezeHide", IsUndoFreezeHide() ? TRUE : FALSE);

	//for formatting the floats
	CString sVal;
	sVal.Format("%f", GetVectorEditIncrement());
	SetStringValue("VectorEditIncrement", sVal);
	SetBoolValue("VectorEditAutoApply", IsVectorEditAutoApply() ? TRUE : FALSE);
	SetBoolValue("RotationEditAutoApply", IsRotationEditAutoApply() ? TRUE : FALSE);

	// Save undo data
	SetDWordValue("NumUndos", m_dwUndos);

	return TRUE;
}


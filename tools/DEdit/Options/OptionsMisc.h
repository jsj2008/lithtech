//--------------------------------------------------------------
//OptionsMisc.h
//
// Contains the definition for COptionsMisc which holds the
// miscellaneous user options
//
// Author: David Carlton
// Created: unknown
// Modification History:
//
//---------------------------------------------------------------
#ifndef __OPTIONSMISC_H__
#define __OPTIONSMISC_H__

#include "optionsbase.h"

class COptionsMisc : public COptionsBase  
{
public:

	COptionsMisc();
	virtual ~COptionsMisc();

	// Load/Save
	BOOL	Load();
	BOOL	Save();

	// Access to the options
	bool		IsParentFolder() const				{return m_bParentFolder;}
	void		SetParentFolder(bool bVal)			{m_bParentFolder = bVal;}

	// Tab icons in Project Window
	bool		GetShowIcons() const				{ return m_bShowIcons; }
	void		SetShowIcons(bool bShow)			{ m_bShowIcons = bShow; }

	// Thumbnails in Textures tab
	bool		GetShowThumbnails() const			{ return m_bShowThumbnails; }
	void		SetShowThumbnails(bool bShow)		{ m_bShowThumbnails = bShow; }

	//determine if the user wants the class icons extracted
	bool		IsAutoExtractIcons() const			{ return m_bAutoExtractIcons; }
	void		SetAutoExtractIcons(bool bVal)		{ m_bAutoExtractIcons = bVal; }

	//determine if Classhlp.but should be extracted
	bool		IsAutoExtractClassHelp() const		{ return m_bAutoExtractHelp; }
	void		SetAutoExtractClassHelp(bool bVal)	{ m_bAutoExtractHelp = bVal; }

	// Undo warnings
	bool		GetShowUndoWarnings() const			{ return m_bShowUndoWarnings; }
	void		SetShowUndoWarnings(bool bShow)		{ m_bShowUndoWarnings = bShow; }

	//Full path in title
	bool		IsShowFullPathTitle() const			{ return m_bShowFullPath; }
	void		SetShowFullPathTitle(bool bShow)	{ m_bShowFullPath = bShow; }

	//size of undo buffer
	DWORD		GetNumUndos() const					{ return m_dwUndos; }
	void		SetNumUndos(DWORD dwUndos)			{ m_dwUndos = dwUndos; }

	//Auto load previously open project
	bool		IsAutoLoadProj() const				{ return m_bAutoLoadProj; }
	void		SetAutoLoadProj(bool bVal)			{ m_bAutoLoadProj = bVal; }

	//Whether or not to use compressed files for default (used when creating
	// LTA files)
	bool		IsDefaultCompressed() const			{ return m_bDefaultCompressed; }
	void		SetDefaultCompressed(bool bVal)		{ m_bDefaultCompressed = bVal; }

	//Whether or not the LYT file should be opened up when a project is loaded
	bool		IsLoadLYTFile() const				{ return m_bLoadLYTFile; }
	void		SetLoadLYTFile(bool bVal)			{ m_bLoadLYTFile = bVal; }

	//Whether or not freeze/hide operations should create undos
	bool		IsUndoFreezeHide() const			{ return m_bUndoFreezeHide; }
	void		SetUndoFreezeHide(bool bVal)		{ m_bUndoFreezeHide = bVal; }

	//Increment value for the vector edit dialog
	float		GetVectorEditIncrement() const		{ return m_fVectorEditIncrement; }
	void		SetVectorEditIncrement(float fVal)	{ m_fVectorEditIncrement = fVal; }

	//Auto apply value for the vector edit dialog
	bool		IsVectorEditAutoApply() const		{ return m_bVectorEditAutoApply; }
	void		SetVectorEditAutoApply(bool bVal)	{ m_bVectorEditAutoApply = bVal; }

	//Auto apply value for the rotation edit dialog
	bool		IsRotationEditAutoApply() const		{ return m_bRotationEditAutoApply; }
	void		SetRotationEditAutoApply(bool bVal)	{ m_bRotationEditAutoApply = bVal; }



protected:

	//do we show the full path in the title
	bool		m_bShowFullPath;

	//do we want to auto extract the icons
	bool		m_bAutoExtractIcons;

	//determine if Classhlp.but should be extracted
	bool		m_bAutoExtractHelp;

	// Do we expand parent folders when children are selected?
	bool		m_bParentFolder;

	// Tab icons in the Project Window?
	bool		m_bShowIcons;

	// Thumbnails in the Textures tab?
	bool		m_bShowThumbnails;

	// Show warnings when the undo buffer becomes large?
	bool		m_bShowUndoWarnings;

	//Auto load previously open project
	bool		m_bAutoLoadProj;

	//Use LTC by default?
	bool		m_bDefaultCompressed;

	//Load LYT file when opening project
	bool		m_bLoadLYTFile;

	//Create an undo for freeze/hide
	bool		m_bUndoFreezeHide;

	//Increment value for the vector edit dialog
	float		m_fVectorEditIncrement;

	//whehter or not to auto apply the vector edit changes
	bool		m_bVectorEditAutoApply;

	//whehter or not to auto apply the rotation edit changes
	bool		m_bRotationEditAutoApply;

	// Size of undo buffer
	DWORD		m_dwUndos;
};

#endif 

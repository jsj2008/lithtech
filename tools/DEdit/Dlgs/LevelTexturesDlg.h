////////////////////////////////////////////////////////////////
//
// leveltextures.h
//
// The implementation for the dialog that handles reporting the 
// level textures for artist information gathering
//
// Author: John O'Rorke
// Created: 6/17/02
// Modification History:
//
////////////////////////////////////////////////////////////////
#ifndef __LEVELTEXTURESDLG_H__
#define __LEVELTEXTURESDLG_H__

#include "stdafx.h"
#include "resource.h"

#ifndef __LEVELTEXTURESOPTIONSDLG_H__
#	include "LevelTexturesOptionsDlg.h"
#endif

class CWorldNode;
class CEditRegion;

class CLevelTexturesDlg : 
	public CDialog
{
public:

	CLevelTexturesDlg();
	~CLevelTexturesDlg();

	//given the name of a file, it will add it to the list appropriately
	void			AddNewFile(const char* pszFile);

	//initializes our icon bitmap. Should be called after new files have been added
	void			UpdateIcons();

private:

	//texture information
	struct STexture
	{
		//texture filename
		CString		m_sFilename;

		//number of times it is used in this level
		uint32		m_nRefCount;

		//other fields from the texture
		uint32		m_nWidth;
		uint32		m_nHeight;
		uint32		m_nMemory;
		uint32		m_nUncompressedMemory;
		uint32		m_nMipMaps;
		uint32		m_nFlags;
		uint32		m_nGroup;
		uint32		m_nNonS3TCOffset;
		uint32		m_nUIOffset;

		bool		m_bPrefer16Bit;
		bool		m_bNoSysCache;
		bool		m_bFullBrite;
		bool		m_bPreserve32Bit;

		float		m_fDetailTexScale;
		float		m_fDetailTexAngle;

		CString		m_sCompression;
		CString		m_s16BitFormat;
		CString		m_sTextureType;
		CString		m_sCommandString;
	};

	enum{	MIN_TEXTURE_SIZE_BITS	= 3,
			MAX_TEXTURE_SIZE_BITS	= 11,
			MIN_TEXTURE_SIZE		= (1<<MIN_TEXTURE_SIZE_BITS),
			MAX_TEXTURE_SIZE		= (1<<MAX_TEXTURE_SIZE_BITS),
			NUM_TEXTURE_SIZES		= MAX_TEXTURE_SIZE_BITS - MIN_TEXTURE_SIZE_BITS
		};

	enum {	COMPRESSION_NONE,
			COMPRESSION_DXT1,
			COMPRESSION_DXT3,
			COMPRESSION_DXT5,
			NUM_COMPRESSION_TYPES
		};

	enum {	COL_WIDTH,
			COL_HEIGHT,
			COL_REFCOUNT,
			COL_MEMORY,
			COL_UNCOMPRESSEDMEMORY,
			COL_MIPMAPS,
			COL_COMPRESSION,
			COL_FLAGS,
			COL_GROUP,
			COL_NONS3TCMIPOFFSET,
			COL_UIMIPOFFSET,
			COL_DETAILTEXSCALE,
			COL_DETAILTEXANGLE,
			COL_16BITFORMAT,
			COL_PREFER16BIT,
			COL_NOSYSCACHE,
			COL_FULLBRITES,
			COL_PRESERVE32BIT,
			COL_TEXTURETYPE,
			COL_COMMANDSTRING,
			
			NUM_COLS
	};

	//fills out the stats list
	void			AddStat(const char* pszName, uint32 nVal);
	void			FillStatsList();

	//This will add a new texture to the list loaded from the specified file
	bool			AddNewTexture(const char* pszFile);

	//this will initialize all the columns
	void			InitColumns();

	//this will put the data into the table for the appropriate item if it is enabled
	void			AddTableData(uint32 nItem, uint32 nCol, const char* pszData);
	void			AddTableData(uint32 nItem, uint32 nCol, uint32 nData);
	void			AddTableData(uint32 nItem, uint32 nCol, float fData);

	//given a texture size it will convert it into an index into the count array
	uint32			TextureSizeToIndex(uint32 nTexSize);

	//the reference count of textures of each size
	uint32			m_nTexSizeRefCount[NUM_TEXTURE_SIZES][NUM_TEXTURE_SIZES];

	//reference counts for compression
	uint32			m_nCompressionRefCount[NUM_COMPRESSION_TYPES];

	//the column information
	CLevelTexturesColumn	m_Columns[NUM_COLS];

	//the number of columns installed on the control
	uint32			m_nNumControlColumns;

	//misc texture counts
	uint32			m_nTotalTextureCount;
	uint32			m_nTotalMemory;
	uint32			m_nTotalUncompressedMemory;

	CMoArray<STexture>	m_Textures;

	//The bitmap that holds the icon images
	CBitmap			*m_pIconBitmap;

	//Image lists for the items in the list
	CImageList		m_IconList;

	//accessor for obtaining the list control that holds the list of textures
	CListCtrl*		GetTextureList()		{ return (CListCtrl*)GetDlgItem(IDC_LIST_TEXTURE_INFO); }

	//accessor for obtaining the list control that holds the texture stats
	CListCtrl*		GetStatsList()			{ return (CListCtrl*)GetDlgItem(IDC_LIST_TEXTURE_STATS); }

	//determines the number of items selected
	uint32			GetNumSelected();

	//handles updating the various controls
	void			UpdateEnabled();

	//standard button handlers
	void			OnOK();
	void			OnCancel();

	//handle initialization and loading of icons
	BOOL			OnInitDialog();

	//clears out the old lists and controls
	void			ClearTextureInfo();

	//clears out the old lists and controls
	void			BuildTextureInfoR(const CWorldNode* pNode);

	//adds a texture to the list
	void			AddTexture(const char* pszTextureName);

	//recursively selects brushes matching the selected texture
	void			SelectBrushesR(CEditRegion* pRegion, const CWorldNode* pNode, const CWorldNode* pSelectNode, const char* pszTex);

	//the callbackf for sorting textures
	static int CALLBACK SortTextureList(LPARAM lParam1, LPARAM lParam2, LPARAM nCol);

	//handle the button for saving the list to a csv
	afx_msg void	OnButtonSave();

	//handle the button for editing the options
	afx_msg void	OnButtonOptions();

	//handle the button for updating the texture list
	afx_msg void	OnButtonUpdate();

	//handle the button for updating the texture list
	afx_msg void	OnButtonSelectBrushes();

	//handle when the selection changes
	afx_msg void	OnSelectionChange(NMHDR* pmnh, LRESULT* pResult);

	//handle a column click on the texture list
	afx_msg void	OnSortItems(NMHDR * pNotifyStruct, LRESULT * pResult);

	//handle a double click on a texture
	afx_msg void	OnActivateItem(NMHDR * pNotifyStruct, LRESULT * pResult);


	DECLARE_MESSAGE_MAP()

};

#endif


#include "bdefs.h"
#include "leveltexturesdlg.h"
#include "resource.h"
#include "edithelpers.h"
#include "editprojectmgr.h"
#include "dirdialog.h"
#include "filepalette.h"
#include "texture.h"
#include "regiondoc.h"
#include "worldnode.h"
#include "ProjectBar.h"
#include "streamsim.h"

#if _MSC_VER >= 1300
#include <fstream>
#include <iostream>
#else
#include <fstream.h>
#include <iostream.h>
#endif

#define IMAGE_SIZE			32

//----------------------------------------------------------------------------
// CLevelTexturesDlg


BEGIN_MESSAGE_MAP (CLevelTexturesDlg, CDialog)
	ON_BN_CLICKED(IDC_BUTTON_SELECT_BRUSHES, OnButtonSelectBrushes)	
	ON_BN_CLICKED(IDC_BUTTON_UPDATE, OnButtonUpdate)	
	ON_BN_CLICKED(IDC_BUTTON_SAVE, OnButtonSave)
	ON_BN_CLICKED(IDC_BUTTON_OPTIONS, OnButtonOptions)

	ON_NOTIFY(LVN_COLUMNCLICK, IDC_LIST_TEXTURE_INFO, OnSortItems)
	ON_NOTIFY(LVN_ITEMACTIVATE, IDC_LIST_TEXTURE_INFO, OnActivateItem)
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_LIST_TEXTURE_INFO, OnSelectionChange)

END_MESSAGE_MAP()

CLevelTexturesDlg::CLevelTexturesDlg() :	
	CDialog(IDD_LEVEL_TEXTURES),
	m_pIconBitmap(NULL),
	m_nNumControlColumns(0)
{
	InitColumns();
}

CLevelTexturesDlg::~CLevelTexturesDlg()
{
	//free up the bitmap
	delete m_pIconBitmap;
}



//---------------------------------------------------------------------------------------
// User Interface
//
// Functions for handling the user interface of the dialog
//
//---------------------------------------------------------------------------------------


//this will initialize all the columns
void CLevelTexturesDlg::InitColumns()
{
	m_Columns[COL_WIDTH].Init("Width", true, 50);
	m_Columns[COL_HEIGHT].Init("Height", true, 50);
	m_Columns[COL_FLAGS].Init("Flags", true, 30);
	m_Columns[COL_GROUP].Init("Group", true, 40);
	m_Columns[COL_MEMORY].Init("Memory", true, 50);
	m_Columns[COL_UNCOMPRESSEDMEMORY].Init("Uncompressed Memory", false, 90);
	m_Columns[COL_REFCOUNT].Init("Uses", true, 40);
	m_Columns[COL_MIPMAPS].Init("MipMaps", true, 50);
	m_Columns[COL_COMMANDSTRING].Init("Command String", true, 200);
	m_Columns[COL_NONS3TCMIPOFFSET].Init("Non S3TC Mip Offset", false, 150);
	m_Columns[COL_UIMIPOFFSET].Init("UI Mip Offset", false, 100);
	m_Columns[COL_DETAILTEXSCALE].Init("Detail Tex Scale", false, 100);
	m_Columns[COL_DETAILTEXANGLE].Init("Detail Tex Angle", false, 150);
	m_Columns[COL_COMPRESSION].Init("Compression", false, 70);
	m_Columns[COL_16BITFORMAT].Init("16 bit Format", false, 80);
	m_Columns[COL_PREFER16BIT].Init("Prefer 16 bit", false, 80);
	m_Columns[COL_NOSYSCACHE].Init("No Sys Cache", false, 80);
	m_Columns[COL_FULLBRITES].Init("Fullbrites", false, 70);
	m_Columns[COL_PRESERVE32BIT].Init("Preserve 32 bit", false, 100);
	m_Columns[COL_TEXTURETYPE].Init("Texture Type", true, 80);
}

//standard button handlers
void CLevelTexturesDlg::OnOK()
{
	//don't call the base OK....
	ShowWindow(SW_HIDE);
}

void CLevelTexturesDlg::OnCancel()
{
	//don't call the base cancel, but hide ourselves...
	ShowWindow(SW_HIDE);
}

//handle initialization and loading of icons
BOOL CLevelTexturesDlg::OnInitDialog()
{
	if(!CDialog::OnInitDialog())
		return FALSE;

	//make it so the user can select any part of that row. Makes it much easier to use
	GetTextureList()->SetExtendedStyle( LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | LVS_EX_HEADERDRAGDROP );
	GetStatsList()->SetExtendedStyle( LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | LVS_EX_HEADERDRAGDROP );

	//update the enabled status
	UpdateEnabled();

	//setup the icon list
	m_IconList.Create(IMAGE_SIZE,IMAGE_SIZE,ILC_COLOR32,1,1);
	m_IconList.SetBkColor(RGB(0,128,128));
	m_IconList.Add(AfxGetApp()->LoadIcon(IDI_TEXTURE_TAB_ICON));
	GetTextureList()->SetImageList(&m_IconList, LVSIL_SMALL);

	//add our columns to the stat list
	GetStatsList()->InsertColumn(0, "Stat", LVCFMT_LEFT, 250);
	GetStatsList()->InsertColumn(1, "Value", LVCFMT_LEFT, 75);

	return TRUE;
}

//determines the number of items selected
uint32 CLevelTexturesDlg::GetNumSelected()
{
	return (uint32)GetTextureList()->GetSelectedCount();
}

//handles updating the various controls
void CLevelTexturesDlg::UpdateEnabled()
{
	//get the number of items selected in the list
	DWORD nNumSelected = GetNumSelected();

	GetDlgItem(IDC_BUTTON_SELECT_BRUSHES)->EnableWindow(nNumSelected > 0);
}

//handle when the selection changes
void CLevelTexturesDlg::OnSelectionChange(NMHDR* pmnh, LRESULT* pResult)
{
	//see if this is actually a change
	NMLISTVIEW* pLVHdr = (NMLISTVIEW*)pmnh;

	if(pLVHdr->uChanged & LVIF_STATE)
	{
		UpdateEnabled();
	}
}

//clears out the old lists and controls
void CLevelTexturesDlg::ClearTextureInfo()
{
	//we need to clear everything out of the list
	GetTextureList()->DeleteAllItems();
	GetStatsList()->DeleteAllItems();

	//reset all the counts
	memset(m_nTexSizeRefCount, 0, sizeof(m_nTexSizeRefCount));
	memset(m_nCompressionRefCount, 0, sizeof(m_nCompressionRefCount));

	m_nTotalTextureCount = 0;
	m_nTotalMemory = 0;
	m_nTotalUncompressedMemory = 0;

	//remove all the columns
	for(int32 nCol = m_nNumControlColumns - 1; nCol >= 0; nCol--)
	{
		GetTextureList()->DeleteColumn(nCol);
	}
	m_nNumControlColumns = 0;

	m_Textures.SetSize(0);
}

//adds a texture to the list
void CLevelTexturesDlg::AddTexture(const char* pszTextureName)
{
	//first off see if this is already in the list, if so, all we need to do is increment the ref
	//count
	for(uint32 nCurrTex = 0; nCurrTex < m_Textures.GetSize(); nCurrTex++)
	{
		if(m_Textures[nCurrTex].m_sFilename.CompareNoCase(pszTextureName) == 0)
		{
			//we have a match
			m_Textures[nCurrTex].m_nRefCount++;
			return;
		}
	}

	//no match, we need to add the new texture
	AddNewTexture(pszTextureName);
}

//clears out the old lists and controls
void CLevelTexturesDlg::BuildTextureInfoR(const CWorldNode* pNode)
{
	//see if this is a brush
	if(pNode->GetType() == Node_Brush)
	{
		//this is a brush, add its texture info
		CEditBrush* pBrush = (CEditBrush*)pNode;

		for(uint32 nCurrPoly = 0; nCurrPoly < pBrush->m_Polies.GetSize(); nCurrPoly++)
		{
			for(uint32 nCurrTex = 0; nCurrTex < CEditPoly::NUM_TEXTURES; nCurrTex++)
			{
				const char* pszTexture = pBrush->m_Polies[nCurrPoly]->GetTexture(nCurrTex).m_pTextureName;

				if(pszTexture && strlen(pszTexture) && !CHelpers::UpperStrcmp(pszTexture, "Default"))
				{
					AddTexture(pszTexture);
				}
			} 
		}
	}
	else if(pNode->GetType() == Node_PrefabRef)
	{
		//this is a prefab, recurse into its tree
		BuildTextureInfoR(((CPrefabRef*)pNode)->GetPrefabTree());
	}

	//now we need to recurse into the children of this node
	GPOS Pos;
	for( Pos = const_cast<CWorldNode*>(pNode)->m_Children; Pos; )
	{
		CWorldNode* pChild = pNode->m_Children.GetNext(Pos);
		BuildTextureInfoR(pChild);
	}
}


//handle the button for saving the list to a csv
void CLevelTexturesDlg::OnButtonSave()
{
	//alright, first prompt for a filename
	CFileDialog Dlg(FALSE, ".csv", NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, "CSV Files(*.csv)|*.csv|All Files(*.*)|*.*||");

	if(Dlg.DoModal() != IDOK)
		return;

#if _MSC_VER >= 1300
	std::ofstream OutFile(Dlg.GetPathName());
#else
	ofstream OutFile(Dlg.GetPathName());
#endif

	if(!OutFile.good())
	{
		CString sError;
		sError.Format("Error opening file %s for writing", Dlg.GetPathName());
		MessageBox(sError, "Error opening file", MB_ICONEXCLAMATION | MB_OK);
		return;
	}

	//now we begin by getting the column ordering
	int nColOrder[NUM_COLS + 1];
	GetTextureList()->GetColumnOrderArray(nColOrder, -1);

	//now save out the texture header
	uint32 nCurrCol;
	for(nCurrCol = 0; nCurrCol < m_nNumControlColumns; nCurrCol++)
	{
		//if it is the first, it is texture, otherwise we need to search and find it
		const char* pszColName = "Texture";

		for(uint32 nTestCol = 0; nTestCol < NUM_COLS; nTestCol++)
		{
			if(nColOrder[nCurrCol] == m_Columns[nTestCol].m_nColID)
			{
				pszColName = m_Columns[nTestCol].m_sName;
				break;
			}
		}

		OutFile << pszColName << ", ";
	}
#if _MSC_VER >= 1300
	OutFile << std::endl;
#else
	OutFile << endl;
#endif
	//now we need to save out each row and each field
	uint32 nCurrItem;
	for(nCurrItem = 0; nCurrItem < GetTextureList()->GetItemCount(); nCurrItem++)
	{
		for(nCurrCol = 0; nCurrCol < m_nNumControlColumns; nCurrCol++)
		{
			OutFile << GetTextureList()->GetItemText(nCurrItem, nColOrder[nCurrCol]) << ", ";
		}
#if _MSC_VER >= 1300
		OutFile << std::endl;
#else
		OutFile << endl;
#endif
	}

	//now save out the stats
#if _MSC_VER >= 1300
	OutFile << std::endl;
	OutFile << "Stat, Value" << std::endl;
#else
	OutFile << endl;
	OutFile << "Stat, Value" << endl;
#endif

	for(nCurrItem = 0; nCurrItem < GetStatsList()->GetItemCount(); nCurrItem++)
	{
		for(nCurrCol = 0; nCurrCol < 2; nCurrCol++)
		{
			OutFile << GetStatsList()->GetItemText(nCurrItem, nCurrCol) << ", ";
		}
#if _MSC_VER >= 1300
		OutFile << std::endl;
#else
		OutFile << endl;
#endif
	}

	//and we are done...	
}

//handle the button for updating the texture list
void CLevelTexturesDlg::OnButtonUpdate()
{
	//get the active document
	CRegionDoc* pDoc = ::GetActiveRegionDoc();

	//Bail if we aren't setup with an active level
	if(pDoc == NULL)
	{
		MessageBox("Unable to update level textures since no level is open", "Error updating textures", MB_ICONEXCLAMATION | MB_OK);
		return;
	}

	//out with the old
	ClearTextureInfo();

	//now we need to run through the level and accumulate the new texture info
	BuildTextureInfoR(pDoc->GetRegion()->GetRootNode());

	//setup our new columns
	GetTextureList()->InsertColumn(0, "Texture", LVCFMT_LEFT, 200);
	m_nNumControlColumns++;

	for(uint32 nCurrCol = 0; nCurrCol < NUM_COLS; nCurrCol++)
	{
		if(m_Columns[nCurrCol].m_bEnabled)
		{
			m_Columns[nCurrCol].m_nColID = GetTextureList()->InsertColumn(m_nNumControlColumns, m_Columns[nCurrCol].m_sName, LVCFMT_LEFT, m_Columns[nCurrCol].m_nDefaultWidth);
			m_nNumControlColumns++;
		}
		else
		{
			m_Columns[nCurrCol].m_nColID = 0xFFFFFFFF;
		}

	}

	//alright, we now have all the textures and how many times each is used. We need to setup
	//the columns in the texture list, and then load up each texture, grab its information
	//and put it into the table
	for(uint32 nCurrTex = 0; nCurrTex < m_Textures.GetSize(); nCurrTex++)
	{
		STexture* pTexture = &m_Textures[nCurrTex];

		//add the actual item to the table
		uint32 nItem = GetTextureList()->InsertItem(0, pTexture->m_sFilename);
		GetTextureList()->SetItemData(nItem, (DWORD)pTexture);

		//we have the texture loaded now, so put its data into the appropriate fields
		AddTableData(nItem, COL_WIDTH, pTexture->m_nWidth);
		AddTableData(nItem, COL_HEIGHT, pTexture->m_nHeight);
		AddTableData(nItem, COL_FLAGS, pTexture->m_nFlags);
		AddTableData(nItem, COL_GROUP, pTexture->m_nGroup);
		AddTableData(nItem, COL_MEMORY, pTexture->m_nMemory / 1024);
		AddTableData(nItem, COL_UNCOMPRESSEDMEMORY, pTexture->m_nUncompressedMemory / 1024);
		AddTableData(nItem, COL_REFCOUNT, pTexture->m_nRefCount);
		AddTableData(nItem, COL_MIPMAPS, pTexture->m_nMipMaps);
		AddTableData(nItem, COL_COMMANDSTRING, pTexture->m_sCommandString);
		AddTableData(nItem, COL_NONS3TCMIPOFFSET, pTexture->m_nNonS3TCOffset);
		AddTableData(nItem, COL_UIMIPOFFSET, pTexture->m_nUIOffset);
		AddTableData(nItem, COL_DETAILTEXSCALE, pTexture->m_fDetailTexScale);
		AddTableData(nItem, COL_DETAILTEXANGLE, pTexture->m_fDetailTexAngle);
		AddTableData(nItem, COL_COMPRESSION, pTexture->m_sCompression);
		AddTableData(nItem, COL_16BITFORMAT, pTexture->m_s16BitFormat);
		AddTableData(nItem, COL_PREFER16BIT, (pTexture->m_bPrefer16Bit) ? "Yes" : "No");
		AddTableData(nItem, COL_NOSYSCACHE, (pTexture->m_bNoSysCache) ? "Yes" : "No");
		AddTableData(nItem, COL_FULLBRITES, (pTexture->m_bFullBrite) ? "Yes" : "No");
		AddTableData(nItem, COL_PRESERVE32BIT, (pTexture->m_bPreserve32Bit) ? "Yes" : "No");
		AddTableData(nItem, COL_TEXTURETYPE, pTexture->m_sTextureType);
	}

	UpdateIcons();
	UpdateEnabled();

	FillStatsList();
}

//This will add a new texture to the list loaded from the specified file
bool CLevelTexturesDlg::AddNewTexture(const char* pszFile)
{
	STexture NewTexture;

	//fill out the standard fields
	NewTexture.m_sFilename = pszFile;
	NewTexture.m_nRefCount = 1;

	//first off, we need to load up this texture to get all of its information
	CString sFilename = GetProject()->m_BaseProjectDir + "\\" + NewTexture.m_sFilename;

	//try and open up the file
	ILTStream* pStream = streamsim_Open(sFilename, "rb");
	if(!pStream)
	{
		return false;
	}

	TextureData* pDTX;
	if(dtx_Create(pStream, &pDTX, FALSE, FALSE) != LT_OK)
		return false;

	//free up the stream
	pStream->Release();

	//figure out the compression type
	switch(pDTX->m_Header.GetBPPIdent())
	{
	case BPP_S3TC_DXT1:
		NewTexture.m_sCompression = "DXT1";
		m_nCompressionRefCount[COMPRESSION_DXT1]++;
		break;
	case BPP_S3TC_DXT3:
		NewTexture.m_sCompression = "DXT3";
		m_nCompressionRefCount[COMPRESSION_DXT3]++;
		break;
	case BPP_S3TC_DXT5:
		NewTexture.m_sCompression = "DXT5";
		m_nCompressionRefCount[COMPRESSION_DXT5]++;
		break;
	default:
		NewTexture.m_sCompression = "None";
		m_nCompressionRefCount[COMPRESSION_NONE]++;
		break;
	};

	if(pDTX->m_Header.m_IFlags & DTX_CUBEMAP)
		NewTexture.m_sTextureType = "Cube Map";
	else if(pDTX->m_Header.m_IFlags & DTX_BUMPMAP)
		NewTexture.m_sTextureType = "Bump Map";
	else if(pDTX->m_Header.m_IFlags & DTX_LUMBUMPMAP)
		NewTexture.m_sTextureType = "Lum Bump Map";
	else
		NewTexture.m_sTextureType = "Image";

	if(pDTX->m_Header.m_IFlags & DTX_PREFER4444)
		NewTexture.m_s16BitFormat = "4444";
	else if(pDTX->m_Header.m_IFlags & DTX_PREFER5551)
		NewTexture.m_s16BitFormat = "5551";
	else 
		NewTexture.m_s16BitFormat = "Default";


	NewTexture.m_nMemory = 0;
	NewTexture.m_nUncompressedMemory = 0;
	NewTexture.m_nMipMaps = pDTX->m_Header.m_nMipmaps;

	for(uint32 nCurrMip = 0; nCurrMip < NewTexture.m_nMipMaps; nCurrMip++)
	{
		NewTexture.m_nMemory += CalcImageSize(pDTX->m_Header.GetBPPIdent(), pDTX->m_Mips[nCurrMip].m_Width, pDTX->m_Mips[nCurrMip].m_Height);
		NewTexture.m_nUncompressedMemory += CalcImageSize(BPP_32, pDTX->m_Mips[nCurrMip].m_Width, pDTX->m_Mips[nCurrMip].m_Height);
	}

	//increment our size reference counter as well
	m_nTexSizeRefCount[TextureSizeToIndex(pDTX->m_Header.m_BaseWidth)][TextureSizeToIndex(pDTX->m_Header.m_BaseHeight)]++;

	//keep our other counts up to date..
	m_nTotalTextureCount++;
	m_nTotalMemory += NewTexture.m_nMemory;
	m_nTotalUncompressedMemory += NewTexture.m_nUncompressedMemory;

	NewTexture.m_nWidth = pDTX->m_Header.m_BaseWidth;
	NewTexture.m_nHeight = pDTX->m_Header.m_BaseHeight;
	NewTexture.m_nFlags = pDTX->m_Header.m_UserFlags;
	NewTexture.m_nGroup = pDTX->m_Header.GetTextureGroup();
	NewTexture.m_sCommandString = pDTX->m_Header.m_CommandString;
	NewTexture.m_nNonS3TCOffset = pDTX->m_Header.GetNonS3TCMipmapOffset();
	NewTexture.m_nUIOffset = pDTX->m_Header.GetUIMipmapOffset();
	NewTexture.m_fDetailTexScale = pDTX->m_Header.GetDetailTextureScale();
	NewTexture.m_fDetailTexAngle = (float)pDTX->m_Header.GetDetailTextureAngle();
	NewTexture.m_bPrefer16Bit = !!(pDTX->m_Header.m_IFlags & DTX_PREFER16BIT);
	NewTexture.m_bNoSysCache = !!(pDTX->m_Header.m_IFlags & DTX_NOSYSCACHE);
	NewTexture.m_bFullBrite = !!(pDTX->m_Header.m_IFlags & DTX_FULLBRITE);
	NewTexture.m_bPreserve32Bit = !!(pDTX->m_Header.m_IFlags & DTX_32BITSYSCOPY);

	dtx_Destroy(pDTX);

	//now add this texture
	m_Textures.Append(NewTexture);

	return true;
}

//recursively selects brushes matching the selected texture
void CLevelTexturesDlg::SelectBrushesR(CEditRegion* pRegion, const CWorldNode* pNode, const CWorldNode* pSelectNode, const char* pszTex)
{
	//see if this is a brush
	if(pNode->GetType() == Node_Brush)
	{
		//this is a brush, add its texture info
		CEditBrush* pBrush = (CEditBrush*)pNode;

		for(uint32 nCurrPoly = 0; nCurrPoly < pBrush->m_Polies.GetSize(); nCurrPoly++)
		{
			for(uint32 nCurrTex = 0; nCurrTex < CEditPoly::NUM_TEXTURES; nCurrTex++)
			{
				const char* pszTexture = pBrush->m_Polies[nCurrPoly]->GetTexture(nCurrTex).m_pTextureName;

				if(pszTexture && strlen(pszTexture) && CHelpers::UpperStrcmp(pszTexture, pszTex))
				{
					//we have a match
					pRegion->SelectNode(const_cast<CWorldNode*>(pSelectNode ? pSelectNode : pNode));
					break;
				}
			} 
		}
	}
	else if(pNode->GetType() == Node_PrefabRef)
	{
		//this is a prefab, recurse into its tree
		SelectBrushesR(pRegion, ((CPrefabRef*)pNode)->GetPrefabTree(), pSelectNode ? pSelectNode : pNode, pszTex);
	}

	//now we need to recurse into the children of this node
	GPOS Pos;
	for( Pos = const_cast<CWorldNode*>(pNode)->m_Children; Pos; )
	{
		CWorldNode* pChild = pNode->m_Children.GetNext(Pos);
		SelectBrushesR(pRegion, pChild, pSelectNode, pszTex);
	}
}

//handle the button for updating the texture list
void CLevelTexturesDlg::OnButtonSelectBrushes()
{
	//get the active document
	CRegionDoc* pDoc = ::GetActiveRegionDoc();

	if(!pDoc)
	{
		MessageBox("Unable to select brushes since no level is open", "Error updating textures", MB_ICONEXCLAMATION | MB_OK);
		return;
	}

	//get the current selection
	int nSel = GetTextureList()->GetSelectionMark();

	if(nSel == -1)
		return;

	//remove any old selections
	pDoc->GetRegion()->ClearSelections();

	//now select the selected texture
	STexture* pTex = (STexture*)GetTextureList()->GetItemData(nSel);
	SelectBrushesR(pDoc->GetRegion(), pDoc->GetRegion()->GetRootNode(), NULL, pTex->m_sFilename);

	//make the change show up
	pDoc->UpdateSelectionBox();
	pDoc->RedrawAllViews();
}

//given a texture size it will convert it into an index into the count array
uint32 CLevelTexturesDlg::TextureSizeToIndex(uint32 nTexSize)
{
	if(nTexSize == 0)
		return 0;

	//find out the highest bit of the texture
	uint32 nCount	= 1;

	while((1<<nCount) < nTexSize)
	{
		nCount++;
	}

	if(nCount < MIN_TEXTURE_SIZE_BITS)
		return 0;

	if(nCount > MAX_TEXTURE_SIZE_BITS)
		return NUM_TEXTURE_SIZES - 1;

	return nCount - MIN_TEXTURE_SIZE_BITS;
}



void CLevelTexturesDlg::UpdateIcons()
{
	CClientDC	dcScreen(this);
	CDC			dcMem;
	CTexture	*pTexture;
	CBitmap		*pOldBitmap;

	CWaitCursor WaitCursor;

	m_IconList.SetImageCount(0);  // Empty list out

	dcMem.CreateCompatibleDC(&dcScreen);
	SetStretchBltMode( dcMem.m_hDC, COLORONCOLOR );

	CDC dcPicture;
	dcPicture.CreateCompatibleDC(&dcMem);
	SetStretchBltMode( dcPicture.m_hDC, COLORONCOLOR );

	//clear out the old bitmap
	delete m_pIconBitmap;
	m_pIconBitmap = new CBitmap;

	if(m_pIconBitmap == NULL)
		return;

	//get the number of items in the list
	uint32 nNumItems = GetTextureList()->GetItemCount();

	//create the bitmap we will be using
	if (!m_pIconBitmap->CreateCompatibleBitmap(&dcScreen, IMAGE_SIZE * nNumItems, IMAGE_SIZE))  
		return;

	pOldBitmap = dcMem.SelectObject(m_pIconBitmap);

	//run through and add our images to it
	uint32 nCurrTex;
	for (nCurrTex = 0; nCurrTex < nNumItems; nCurrTex++)
	{
		//get the associated data
		STexture* pInfo = (STexture*)GetTextureList()->GetItemData(nCurrTex);

		//we have a texture, add it to the image list
		DFileIdent* pIdent;
		dfm_GetFileIdentifier(GetFileMgr(), pInfo->m_sFilename, &pIdent);
		
		pTexture = dib_GetDibTexture(pIdent);

		if (pTexture != NULL)  
			pTexture->m_pDib->Blt(dcMem.m_hDC, 0 + IMAGE_SIZE * nCurrTex, 0, IMAGE_SIZE, IMAGE_SIZE);
	}


	dcMem.SelectObject(pOldBitmap);

	m_IconList.Add(m_pIconBitmap, RGB(0,0,0));

	for (nCurrTex = 0; nCurrTex < nNumItems; nCurrTex++)
		GetTextureList()->SetItem( nCurrTex, 0, LVIF_IMAGE, NULL, nCurrTex, 0, 0, 0 );
}

//handle the button for editing the options
void CLevelTexturesDlg::OnButtonOptions()
{
	CLevelTexturesOptionsDlg Dlg(m_Columns, NUM_COLS);
	Dlg.DoModal();
}

void CLevelTexturesDlg::AddTableData(uint32 nItem, uint32 nCol, const char* pszData)
{
	//see if the column is even enabled
	if(!m_Columns[nCol].m_bEnabled)
		return;

	//alright it is, add it
	GetTextureList()->SetItemText(nItem, m_Columns[nCol].m_nColID, pszData);
}

void CLevelTexturesDlg::AddTableData(uint32 nItem, uint32 nCol, uint32 nData)
{
	CString sFormat;
	sFormat.Format("%d", nData);
	AddTableData(nItem, nCol, sFormat);
}

void CLevelTexturesDlg::AddTableData(uint32 nItem, uint32 nCol, float fData)
{
	CString sFormat;
	sFormat.Format("%.2f", fData);
	AddTableData(nItem, nCol, sFormat);
}

//fills out the stats list
void CLevelTexturesDlg::FillStatsList()
{
	//clear out any old data
	GetStatsList()->DeleteAllItems();

	//add in our stats
	AddStat("Num Textures", m_nTotalTextureCount);
	AddStat("Total Memory (k)", m_nTotalMemory / 1024);
	AddStat("Total Uncompressed Memory (k)", m_nTotalUncompressedMemory / 1024);
	AddStat("Uncompressed Textures", m_nCompressionRefCount[COMPRESSION_NONE]);
	AddStat("DXT1 Compressed Textures", m_nCompressionRefCount[COMPRESSION_DXT1]);
	AddStat("DXT3 Compressed Textures", m_nCompressionRefCount[COMPRESSION_DXT3]);
	AddStat("DXT5 Compressed Textures", m_nCompressionRefCount[COMPRESSION_DXT5]);
	
	CString sFormat;
	for(uint32 x = 0; x < NUM_TEXTURE_SIZES; x++)
	{
		for(uint32 y = 0; y < NUM_TEXTURE_SIZES; y++)
		{
			if(m_nTexSizeRefCount[x][y])
			{
				sFormat.Format("%dx%d Textures", (1<<(x + MIN_TEXTURE_SIZE_BITS)), (1<<(y + MIN_TEXTURE_SIZE_BITS)));
				AddStat(sFormat, m_nTexSizeRefCount[x][y]);
			}
		}
	}
}

void CLevelTexturesDlg::AddStat(const char* pszName, uint32 nVal)
{
	CString sFormat;
	sFormat.Format("%d", nVal);

	int nItem = GetStatsList()->InsertItem(GetStatsList()->GetItemCount(), pszName);
	GetStatsList()->SetItemText(nItem, 1, sFormat);
}

int CALLBACK CLevelTexturesDlg::SortTextureList(LPARAM lParam1, LPARAM lParam2, LPARAM nCol)
{
	STexture* pTex1 = (STexture*)lParam1;
	STexture* pTex2 = (STexture*)lParam2;

	//see what we are sorting upon
	switch(nCol)
	{
	case COL_WIDTH:					return pTex2->m_nWidth - pTex1->m_nWidth;	
	case COL_HEIGHT:				return pTex2->m_nHeight - pTex1->m_nHeight;	
	case COL_REFCOUNT:				return pTex2->m_nRefCount - pTex1->m_nRefCount;	
	case COL_MEMORY:				return pTex2->m_nMemory - pTex1->m_nMemory;	
	case COL_UNCOMPRESSEDMEMORY:	return pTex2->m_nUncompressedMemory - pTex1->m_nUncompressedMemory;
	case COL_MIPMAPS:				return pTex2->m_nMipMaps - pTex1->m_nMipMaps;	
	case COL_COMPRESSION:			return strcmp(pTex1->m_sCompression, pTex2->m_sCompression);
	case COL_FLAGS:					return pTex2->m_nFlags - pTex1->m_nFlags;	
	case COL_GROUP:					return pTex2->m_nGroup - pTex1->m_nGroup;	
	case COL_NONS3TCMIPOFFSET:		return pTex2->m_nNonS3TCOffset - pTex1->m_nNonS3TCOffset;	
	case COL_UIMIPOFFSET:			return pTex2->m_nUIOffset - pTex1->m_nUIOffset;
	case COL_DETAILTEXSCALE:		return (int)((pTex2->m_fDetailTexScale - pTex1->m_fDetailTexScale) * 1000.0f);
	case COL_DETAILTEXANGLE:		return (int)((pTex2->m_fDetailTexAngle - pTex1->m_fDetailTexAngle) * 1000.0f);
	case COL_16BITFORMAT:			return strcmp(pTex1->m_s16BitFormat, pTex2->m_s16BitFormat);
	case COL_PREFER16BIT:			return (int)pTex2->m_bPrefer16Bit - (int)pTex1->m_bPrefer16Bit;
	case COL_NOSYSCACHE:			return (int)pTex2->m_bNoSysCache - (int)pTex1->m_bNoSysCache;
	case COL_FULLBRITES:			return (int)pTex2->m_bFullBrite - (int)pTex1->m_bFullBrite;
	case COL_PRESERVE32BIT:			return (int)pTex2->m_bPreserve32Bit - (int)pTex1->m_bPreserve32Bit;
	case COL_TEXTURETYPE:			return strcmp(pTex1->m_sTextureType, pTex2->m_sTextureType);
	case COL_COMMANDSTRING:			return strcmp(pTex1->m_sCommandString, pTex2->m_sCommandString);
	default:						return strcmp(pTex1->m_sFilename, pTex2->m_sFilename);
	}

	return 0;
}

//handle a column click on the texture list
void CLevelTexturesDlg::OnSortItems(NMHDR * pNotifyStruct, LRESULT * pResult)
{
	LPNMLISTVIEW pnmv = (LPNMLISTVIEW)pNotifyStruct; 

	//figure out which column was clicked
	DWORD nCol = pnmv->iSubItem;

	//map that column to one of ours
	uint32 nOurCol = NUM_COLS;		//default to an invalid one (invalid means filename)
	for(uint32 nCurrCol = 0; nCurrCol < NUM_COLS; nCurrCol++)
	{
		if(m_Columns[nCurrCol].m_nColID == nCol)
		{
			nOurCol = nCurrCol;
			break;
		}
	}

	//now we need to sort the list based upon that column
	GetTextureList()->SortItems(SortTextureList, nOurCol);
}

//handle a double click on a texture
void CLevelTexturesDlg::OnActivateItem(NMHDR * pNotifyStruct, LRESULT * pResult)
{
	CListCtrl* pList = GetTextureList();
	CMoArray<DFileIdent*> Selections;

	//find the first item
	POSITION Pos = pList->GetFirstSelectedItemPosition();
	while(Pos)
	{
		//get the error object associated with this item
		int nIndex = pList->GetNextSelectedItem(Pos);

		//get the texture so we can grab the name
		STexture* pTex = (STexture*)pList->GetItemData(nIndex);

		//now create a file identifier from it
		DFileIdent* pIdent;		
		dfm_GetFileIdentifier(GetFileMgr(), pTex->m_sFilename, &pIdent);

		//stick it in a CMoArray
		Selections.Append(pIdent);
	}

	//now do the texture properties on it
	GetTextureDlg()->DoTextureProperties(Selections);
}


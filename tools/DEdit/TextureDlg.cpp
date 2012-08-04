// TextureDlg.cpp : implementation file
//

#include "bdefs.h"
#include "dedit.h"
#include "butemgr.h"
#include "texturedlg.h"
#include "editprojectmgr.h"
#include "edithelpers.h"
#include "mainfrm.h"
#include "projectbar.h"
#include "resnewdir.h"
#include "texture.h"
#include "textureprop.h"
#include "spriteeditdlg.h"
#include "sysstreamsim.h"
#include "stringdlg.h"
#include "alphafromcolordlg.h"
#include "ImportCubeMapDlg.h"
#include "solidalphadlg.h"
#include "dtxmgr.h"
#include "s3tc_compress.h"
#include "RenameResourceDlg.h"
#include "ImportBumpMapDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


extern FormatMgr g_FormatMgr;



/////////////////////////////////////////////////////////////////////////////
// CTextureDlg dialog

BOOL GetTextureDimensions(DFileIdent *pIdent, DWORD *pWidth, DWORD *pHeight)
{
	CMoFileIO file;
	DtxHeader hdr;
	
	*pWidth = *pHeight = 0;

	if(!pIdent)
		return FALSE;

	if(!dfm_OpenFileRelative(GetFileMgr(), pIdent->m_Filename, file))
		return FALSE;

	LithTry
	{				
		file.Read(&hdr, sizeof(hdr));
		if(hdr.m_Version == CURRENT_DTX_VERSION)
		{
			*pWidth = hdr.m_BaseWidth >> hdr.GetUIMipmapOffset();
			*pHeight = hdr.m_BaseHeight >> hdr.GetUIMipmapOffset();
		}
	}
	LithCatch(CLithIOException &exc)
	{
		exc = exc;
		*pWidth = *pHeight = 0;
		return FALSE;
	}

	return TRUE;
}



// --------------------------------------------------------------------------------- //
// The TextureUtils class makes it easier to modify textures.
// Note: This should get its own file but doesn't have one yet due to logistical puff.
// --------------------------------------------------------------------------------- //
class TextureUtils
{
public:

					TextureUtils();

	// Create backups and setup m_PValueMipmaps.
	BOOL			FromTextureData(TextureData *pData);

	// Moves the data from m_PValueMipmaps into the TextureData.
	// If bRestoreColorData is TRUE, it calls RestoreColorData.
	BOOL			ToTextureData(TextureData *pData, BOOL bRestoreColorData);

	// This restores the color data into the texture (ie: if you only wanted to modify the
	// alpha data, you would replace it, recompress, and put the color back the way it was..)
	// If you didn't restore the color, it would have gotten re-compressed and lost quality.
	// Note: this only restores color if it's a compressed format.
	BOOL			RestoreColorData(TextureData *pData);

public:

	// What format m_MipmapBackups are in.
	BPPIdent		m_BPPIdent;

	// How many mipmaps we have setup.
	DWORD			m_nMipmaps;

	// The mipmaps in PValue format.
	CMoDWordArray	m_PValueMipmaps[MAX_DTX_MIPMAPS];
	
	// Backups of the mipmap data.
	CMoByteArray	m_MipmapBackups[MAX_DTX_MIPMAPS];
};
	

TextureUtils::TextureUtils()
{
	m_BPPIdent = BPP_32;
	m_nMipmaps = 0;
}


BOOL TextureUtils::FromTextureData(TextureData *pData)
{
	DWORD i, y, size;
	TextureMipData *pMip;
	FormatMgr formatMgr;
	FMConvertRequest cRequest;
	DRESULT dResult;
	BYTE *pInLine, *pOutLine;


	if(pData->m_Header.m_nMipmaps > MAX_DTX_MIPMAPS)
		return FALSE;

	m_BPPIdent = pData->m_Header.GetBPPIdent();
	m_nMipmaps = pData->m_Header.m_nMipmaps;

	for(i=0; i < pData->m_Header.m_nMipmaps; i++)
	{
		pMip = &pData->m_Mips[i];

		// Setup the PValue version of the mipmap.
		if(!m_PValueMipmaps[i].SetSize(pMip->m_Width * pMip->m_Height))
			return FALSE;

		dtx_SetupDTXFormat(pData, cRequest.m_pSrcFormat);
		cRequest.m_pSrc = pMip->m_Data;
		cRequest.m_SrcPitch = pMip->m_Pitch;
		cRequest.m_pDestFormat->InitPValueFormat();
		cRequest.m_pDest = (BYTE*)m_PValueMipmaps[i].GetArray();
		cRequest.m_DestPitch = pMip->m_Width * sizeof(DWORD);
		cRequest.m_Width = pMip->m_Width;
		cRequest.m_Height = pMip->m_Height;
		cRequest.m_Flags = 0;
		dResult = formatMgr.ConvertPixels(&cRequest);
		if(dResult != LT_OK)
			return FALSE;

		// Backup the mipmaps.
		size = CalcImageSize(m_BPPIdent, pMip->m_Width, pMip->m_Height);
		if(!m_MipmapBackups[i].SetSize(size))
			return FALSE;
		
		if(m_BPPIdent == BPP_32)
		{
			pInLine = pMip->m_Data;
			pOutLine = (BYTE*)m_MipmapBackups[i].GetArray();
			for(y=0; y < pMip->m_Height; y++)
			{
				memcpy(pOutLine, pInLine, pMip->m_Width * sizeof(DWORD));
				pInLine += pMip->m_Pitch;
				pOutLine += pMip->m_Width * sizeof(DWORD);
			}
		}
		else
		{
			memcpy(m_MipmapBackups[i].GetArray(), pMip->m_Data, size);
		}
	}

	return TRUE;
}


BOOL TextureUtils::ToTextureData(TextureData *pData, BOOL bRestoreColorData)
{
	DWORD i, y, size;
	TextureMipData *pMip;
	BYTE *pInLine, *pOutLine;
	S3TC_Compressor cS3TC;
	DRESULT dResult;


	if(m_BPPIdent != pData->m_Header.GetBPPIdent() || 
		m_nMipmaps != pData->m_Header.m_nMipmaps)
	{
		return FALSE;
	}

	// Either just copy the data in or compress it.
	if(m_BPPIdent == BPP_32)
	{
		for(i=0; i < m_nMipmaps; i++)
		{
			pMip = &pData->m_Mips[i];

			pInLine = (BYTE*)m_PValueMipmaps[i].GetArray();
			pOutLine = pMip->m_Data;
			for(y=0; y < pMip->m_Height; y++)
			{
				memcpy(pOutLine, pInLine, pMip->m_Width * sizeof(DWORD));
				pInLine += pMip->m_Width * sizeof(DWORD);
				pOutLine += pMip->m_Pitch;
			}
		}
	}
	else
	{
		for(i=0; i < m_nMipmaps; i++)
		{
			pMip = &pData->m_Mips[i];

			cS3TC.m_Format = m_BPPIdent;
			cS3TC.m_pData = m_PValueMipmaps[i].GetArray();
			cS3TC.m_Width = pMip->m_Width;
			cS3TC.m_Height = pMip->m_Height;
			cS3TC.m_Pitch = pMip->m_Width * sizeof(DWORD);
			cS3TC.m_DataFormat.InitPValueFormat();

			dResult = cS3TC.CompressUsingLibrary();
			if(dResult != LT_OK)
				return FALSE;

			size = CalcImageSize(m_BPPIdent, pMip->m_Width, pMip->m_Height);
			if(size != cS3TC.m_OutDataSize)
				return FALSE;

			memcpy(pMip->m_Data, cS3TC.m_pOutData, size);
		}
	}
		
	return bRestoreColorData ? RestoreColorData(pData) : TRUE;
}


BOOL TextureUtils::RestoreColorData(TextureData *pData)
{
	DWORD iMipmap, nBlocks, iBlock, blockSize, colorBlockSize, alphaBlockSize;
	TextureMipData *pMip;
	BYTE *pSrcPos, *pDestPos;

	// No need....
	if(!IsBPPCompressed(m_BPPIdent))
		return TRUE;
	
	if(m_BPPIdent != pData->m_Header.GetBPPIdent() ||
		m_nMipmaps != pData->m_Header.m_nMipmaps)
	{
		return FALSE;
	}

	for(iMipmap=0; iMipmap < m_nMipmaps; iMipmap++)
	{
		pMip = &pData->m_Mips[iMipmap];

		nBlocks = (pMip->m_Width/4) * (pMip->m_Height/4);
		blockSize = GetBlockSize(m_BPPIdent);
		alphaBlockSize = GetAlphaBlockSize(m_BPPIdent);
		colorBlockSize = GetColorBlockSize(m_BPPIdent);

		pSrcPos = m_MipmapBackups[iMipmap].GetArray();
		pDestPos = pMip->m_Data;

		// Alpha block comes first.. skip past it.
		pSrcPos += alphaBlockSize;
		pDestPos += alphaBlockSize;

		for(iBlock=0; iBlock < nBlocks; iBlock++)
		{
			memcpy(pDestPos, pSrcPos, colorBlockSize);
			pSrcPos += blockSize;
			pDestPos += blockSize;
		}
	}

	return TRUE;
}

// MaxMipsForSize, given a size, it will determine how many additional mips can be generated
// including the starting size. It will always return at least 1 for the base image
uint32 MaxMipsForSize(uint32 nSize)
{
	static const uint32 knMinMipSize = 8;

	uint32 nNumMips = 0;

	do
	{
		nSize /= 2;
		nNumMips++;
	}
	while(nSize >= knMinMipSize);

	return nNumMips;
}

//Utility function for creating a new texture, creating the number of specified mip maps
//and returning it
TextureData* CreateNewMipTexture(uint8* pSrcData, uint32 nSrcPitch, uint32 nWidth, uint32 nHeight, BPPIdent Format, uint32 nNumMips)
{
	//alright, so now we need to discard all the previous mips by taking the source images and
	//recreting N mips.
	TextureData* pNewTexture = dtx_Alloc(	BPP_32, 
											nWidth, nHeight,
											nNumMips, NULL, NULL);

	if(!pNewTexture)
		return NULL;

	// convert the data into this new texture
	FMConvertRequest convert;
	convert.m_pSrc = pSrcData;
	convert.m_pSrcPalette = NULL;
	dtx_SetupDTXFormat2(Format, convert.m_pSrcFormat);
	convert.m_SrcPitch = nSrcPitch;

	convert.m_pDest = pNewTexture->m_Mips[0].m_Data;
	convert.m_pDestFormat->InitPValueFormat();
	convert.m_DestPitch = pNewTexture->m_Mips[0].m_Pitch;

	convert.m_Width = nWidth;
	convert.m_Height = nHeight;
	convert.m_Flags = 0;

	//now generate the mipmaps for this texture
	if(g_FormatMgr.ConvertPixels( &convert ) != LT_OK)
	{
		dtx_Destroy(pNewTexture);
		return NULL;
	}

	// generate mipmaps
	dtx_BuildMipmaps( pNewTexture );

	return pNewTexture;
}

//--------------------------------------------------------------------------
// RegenerateMipMaps
// Handles regenerating mipmaps from a texture. It will preserver the flags
// and compression of the original image, grab the first image of each surface
// and then regenerate N mips. This mip number includes the base image as
// well.
TextureData* RegenerateMipMaps(TextureData* pTexture, uint32 nNumMips)
{
	//sanity check
	if(!pTexture)
		return NULL;

	//make sure our mip count is reasonable
	if(nNumMips == 0)
		nNumMips = 1;

	//only generate mips up to the a certain limit
	uint32 nWidth = pTexture->m_Header.m_BaseWidth;
	uint32 nHeight = pTexture->m_Header.m_BaseHeight;
	uint32 nPitch = pTexture->m_Mips[0].m_Pitch;

	nNumMips = LTMIN(nNumMips, LTMIN(MaxMipsForSize(nWidth), MaxMipsForSize(nHeight)));

	//create our main texture
	TextureData* pNewTexture = CreateNewMipTexture(pTexture->m_Mips[0].m_Data, nPitch, nWidth, nHeight, pTexture->m_Header.GetBPPIdent(), nNumMips);

	//see if that worked
	if(!pNewTexture)
		return NULL;

	//copy over any non-cube map data sections
	DtxSection* pCurr = pTexture->m_pSections;
	while(pCurr)
	{
		if(stricmp(pCurr->m_Header.m_Type, "CUBEMAPDATA") != 0)
		{
			//copy over this section
			uint32 nSectionSize = (uint32)((sizeof(DtxSection)-1) + pCurr->m_Header.m_DataLen );
			DtxSection* pNewSection = (DtxSection*)dalloc( (unsigned long)((sizeof(DtxSection)-1) + pCurr->m_Header.m_DataLen ) );

			if(!pNewSection)
			{
				dtx_Destroy(pNewTexture);
				return NULL;
			}

			memcpy(pNewSection, pCurr, nSectionSize);

			//add the section onto the head of the list
			pNewSection->m_pNext = pNewTexture->m_pSections;
			pNewTexture->m_pSections = pNewSection;
			pNewTexture->m_Header.m_nSections++;
		}

		pCurr = pCurr->m_pNext;
	}


	//see if this is a cube map and we need to generate each cube side
	if(pTexture->m_Header.m_IFlags & DTX_CUBEMAP)
	{
		DtxSection* pCubeData = dtx_FindSection(pTexture, "CUBEMAPDATA");

		if(!pCubeData)
		{
			dtx_Destroy(pNewTexture);
			return NULL;
		}

		//for each face create a texture, build the mips, and skip over the other data

		//calculate how large each cube map chunk is
		uint32 nSrcCubeMapChunkSize = 0;
		uint32 nDestCubeMapChunkSize = 0;

		uint32 nCurrMip;
		for(nCurrMip = 0; nCurrMip < pTexture->m_Header.m_nMipmaps; nCurrMip++)
		{
			nSrcCubeMapChunkSize += CalcImageSize(pTexture->m_Header.GetBPPIdent(), pTexture->m_Mips[nCurrMip].m_Width, pTexture->m_Mips[nCurrMip].m_Height);
		}

		for(nCurrMip = 0; nCurrMip < nNumMips; nCurrMip++)
		{
			nDestCubeMapChunkSize += CalcImageSize(pNewTexture->m_Header.GetBPPIdent(), pNewTexture->m_Mips[nCurrMip].m_Width, pNewTexture->m_Mips[nCurrMip].m_Height);
		}

		//now we need to copy this all into a new section for the DTX
		DtxSection* pNewSection = (DtxSection*)dalloc( (unsigned long)((sizeof(DtxSection)-1) + nDestCubeMapChunkSize * 5 ) );

		if(!pNewSection)
		{
			dtx_Destroy(pNewTexture);
			return NULL;
		}

		pNewSection->m_Header.m_DataLen = nDestCubeMapChunkSize * 5;
		strcpy( pNewSection->m_Header.m_Type, "CUBEMAPDATA" );
		strcpy( pNewSection->m_Header.m_Name, "" );
		pNewSection->m_pNext = pNewTexture->m_pSections;
		pNewTexture->m_pSections = pNewSection;
		pNewTexture->m_Header.m_nSections++;

		//alright, lets proceed
		uint8* pCurrCubeData = (uint8*)pCubeData->m_Data;
		for(uint32 nCurrFace = 0; nCurrFace < 5; nCurrFace++)
		{
			//build up the face texture
			TextureData* pFace = CreateNewMipTexture(pCurrCubeData, nPitch, nWidth, nHeight, pTexture->m_Header.GetBPPIdent(), nNumMips);

			//see if we failed
			if(!pFace)
			{
				//clean up any existing faces
				dtx_Destroy(pNewTexture);
				dtx_Destroy(pFace);
				dfree(pNewSection);
				return NULL;
			}

			pCurrCubeData += nSrcCubeMapChunkSize;

			// copy the texture data into the section data
			memcpy( pNewSection->m_Data + nCurrFace * nDestCubeMapChunkSize, pFace->m_pDataBuffer, nDestCubeMapChunkSize );

			//we don't need this face any more
			dtx_Destroy(pFace);
		}
	}

	//now we need to preserve all settings from the original texture in this new texture
	strcpy(pNewTexture->m_Header.m_CommandString, pTexture->m_Header.m_CommandString);
	pNewTexture->m_Header.m_IFlags = pTexture->m_Header.m_IFlags;
	pNewTexture->m_Header.m_UserFlags = pTexture->m_Header.m_UserFlags;

	//copy over the 'extra' fields. Wow was that a good design decision.
	pNewTexture->m_Header.m_Extra[0] = pTexture->m_Header.m_Extra[0];
	pNewTexture->m_Header.m_Extra[1] = nNumMips;
	pNewTexture->m_Header.m_Extra[4] = pTexture->m_Header.m_Extra[4];

	pNewTexture->m_Header.SetTexturePriority(pTexture->m_Header.GetTexturePriority());
	pNewTexture->m_Header.SetNonS3TCMipmapOffset(pTexture->m_Header.GetNonS3TCMipmapOffset());
	pNewTexture->m_Header.SetDetailTextureScale(pTexture->m_Header.GetDetailTextureScale());
	pNewTexture->m_Header.SetDetailTextureAngle(pTexture->m_Header.GetDetailTextureAngle());

	//we now have a copy of the texture with our number of mip maps. We now need to make sure
	//that the formats match
	if(pTexture->m_Header.GetBPPIdent() != pNewTexture->m_Header.GetBPPIdent())
	{
		if(!ConvertTextureData(pNewTexture, pTexture->m_Header.GetBPPIdent()))
		{
			//we failed
			dtx_Destroy(pNewTexture);
			return NULL;
		}
	}

	//success
	return pNewTexture;
}

/////////////////////////////////////////////////////////////////////////////
// CTextureDlg dialog


CTextureDlg::CTextureDlg()
{
	//{{AFX_DATA_INIT(CTextureDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT

	m_hCurrentItem		= NULL;
	m_hCurrentTextureDir	= 0;
	m_nCurrentTextureIndex	= -1;
	InitBaseRezDlg("dtx", &m_TextureTree, &m_TextureList, RESTYPE_TEXTURE);
}

CTextureDlg::~CTextureDlg()
{
}

void CTextureDlg::DoDataExchange(CDataExchange* pDX)
{
	CMRCSizeDialogBar::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CTextureDlg)
	DDX_Control(pDX, IDC_BASEREZ_TREE, m_TextureTree);
	DDX_Control(pDX, IDC_BASEREZ_LIST, m_TextureList);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CTextureDlg, CBaseImgDlg)
	//{{AFX_MSG_MAP(CTextureDlg)
	ON_WM_CONTEXTMENU()
	ON_WM_SIZE()
	ON_NOTIFY( NM_CLICK, IDC_BASEREZ_LIST, OnSelTexture )
	ON_NOTIFY( NM_DBLCLK, IDC_BASEREZ_LIST, OnOpenTexture )
	ON_NOTIFY(LVN_KEYDOWN, IDC_BASEREZ_LIST, OnKeySelTexture)
	ON_NOTIFY(TVN_SELCHANGED, IDC_BASEREZ_TREE, OnSelchangedDirectory)
	ON_WM_DESTROY()
	ON_NOTIFY(TVN_ITEMEXPANDING, IDC_BASEREZ_TREE, OnItemexpandingBaserezTree)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CTextureDlg message handlers

BOOL CTextureDlg::OnInitDialogBar() 
{
	CMRCSizeDialogBar::OnInitDialogBar();
	
	InitBaseImgDlg((CListCtrl*)GetDlgItem(IDC_BASEREZ_LIST), (CTreeCtrl*)GetDlgItem(IDC_BASEREZ_TREE), IDI_TEXTURE_TAB_ICON);
	
	CRect rect;
	m_TextureList.GetClientRect( &rect );

	m_TextureList.InsertColumn(0,"Name",LVCFMT_LEFT,rect.Width()/2,-1);
	m_TextureList.InsertColumn(1,"Size",LVCFMT_RIGHT,rect.Width()/2,-1);


	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CTextureDlg::OnContextMenu(CWnd* pWnd, CPoint point) 
{
	// TODO: Add your message handler code here
	CMenu menu;
	
	if(!GetProjectBar()->IsProjectOpen())
		return;

	//also don't allow the menu to come up if no directory is selected
	if(m_TextureTree.GetSelectedItem() == NULL)
	{
		MessageBox("You must select a directory above to perform texture operations in", "Please select a texture directory", MB_ICONEXCLAMATION | MB_OK);
		return;
	}

	if(pWnd->m_hWnd == m_TextureTree.m_hWnd)
		VERIFY(menu.LoadMenu(CG_IDR_POPUP_TEXTURETREE));
	else if(pWnd->m_hWnd == m_TextureList.m_hWnd)
	{
		VERIFY(menu.LoadMenu(CG_IDR_POPUP_TEXTURELIST));
		menu.CheckMenuItem(ID_POPUP_SHOWTHUMBNAILS, GetProjectBar()->m_bShowThumbnails ? MF_CHECKED : MF_UNCHECKED );
	}
	else return;

	CMenu* pPopup = menu.GetSubMenu(0);
	ASSERT(pPopup != NULL);

	CWnd* pWndPopupOwner = GetProjectBar();

	pPopup->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, point.x, point.y,
		pWndPopupOwner);	
}

void CTextureDlg::OnSize(UINT nType, int cx, int cy) 
{
	CMRCSizeDialogBar::OnSize(nType, cx, cy);
	
	// Reposition the controls
	RepositionControls();
}

/************************************************************************/
// This is called to reposition the controls
void CTextureDlg::RepositionControls()
{
	if( ::IsWindow(m_hWnd))
	{
		CBaseImgDlg::RepositionControls();

		// Adjust columns
		if (m_TextureList)
		{
			CRect rect;
			m_TextureList.GetClientRect( &rect );

			m_TextureList.SetColumnWidth(0, rect.Width()/2);
			m_TextureList.SetColumnWidth(1, rect.Width()/2);
		}
	}		
}

void CTextureDlg::OnSelchangedDirectory(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_TREEVIEW* pNMTreeView = (NM_TREEVIEW*)pNMHDR;
	// TODO: Add your control notification handler code here
//	LPPOINT lpPoint=new POINT;
//	UINT* pnFlags=new UINT;
	DDirIdent *pIdent;

//	GetCursorPos(lpPoint);
//	m_TextureTree.ScreenToClient(lpPoint);
//	HTREEITEM hItem=m_TextureTree.HitTest(*lpPoint,pnFlags);
	HTREEITEM hItem=pNMTreeView->itemNew.hItem;

	if(hItem)
	{
		pIdent = (DDirIdent*)m_TextureTree.GetItemData(hItem);
		if(pIdent)
		{
			m_csCurrentDir = pIdent->m_Filename;
			m_hCurrentItem = hItem;

			PopulateList();

			// Make sure only the current directory has the selected list item set
			if (m_hCurrentItem == m_hCurrentTextureDir)
				m_TextureList.SetItemState(m_nCurrentTextureIndex, LVIS_SELECTED, LVIS_SELECTED);
		}

		m_AllTextureDlg.NotifyDirChange();
	}

	// If we're showing thumbnails, load up the images for this directory
	if (GetProjectBar()->m_bShowThumbnails)  
	{
		UpdateThumbnails();
		m_TextureList.Update(0); // Update the first item, since its image data doesn't change
	}

	
	RepositionControls(); // Update sizes of things, such as columns in the texture list

	//cleanup
//	delete lpPoint;
//	delete pnFlags;

	*pResult = 0;
}

void CTextureDlg::OnSelTexture( NMHDR * pNMHDR, LRESULT * pResult )
{
	POINT point;
	UINT nFlags;

	GetCursorPos(&point);
	m_TextureList.ScreenToClient(&point);

	int nItem=m_TextureList.HitTest(point,&nFlags);

	if(nItem >= 0)
	{
		ChangeSelection(nItem, FALSE);
	}

	*pResult = 0;
}

void CTextureDlg::OnOpenTexture( NMHDR * pNMHDR, LRESULT * pResult )
{
	POINT point;
	UINT nFlags;
	DFileIdent *pIdent;

	GetCursorPos(&point);
	m_TextureList.ScreenToClient(&point);

	int nItem=m_TextureList.HitTest(point,&nFlags);

	if(nItem >= 0)
	{
		pIdent = (DFileIdent*)m_TextureList.GetItemData(nItem);
		GetProjectBar()->SetCurTextureSel(pIdent);
		RenderLargeImage();

		if( IsWindow( GetProjectBar( )->m_SpriteEditDlg.m_hWnd ))
		{
			GetProjectBar( )->m_SpriteEditDlg.AddTexture( pIdent );
		}
		else
		{
			DoTextureProperties();
		}
	}

	*pResult = 0;
}

void CTextureDlg::OnKeySelTexture( NMHDR * pnkd, LRESULT * pResult )
{
	int nIndex = m_TextureList.GetNextItem(-1,LVNI_SELECTED);
	int nOldIndex = nIndex;
	*pResult = 0;
	
	LV_KEYDOWN *pPnkd = reinterpret_cast<LV_KEYDOWN FAR*>(pnkd);

	switch(pPnkd->wVKey)
	{
		// Up arrow
		case VK_UP:	
			nIndex--; break;
		// Down Arrow
		case VK_DOWN: 
			nIndex++; break;
		// Page up
		case VK_PRIOR: 
			nIndex -= m_TextureList.GetCountPerPage(); break;
		// Page down
		case VK_NEXT: 
			nIndex += m_TextureList.GetCountPerPage(); break;
		// Anything else
		default:	return;
	}

	// Keep the index inside the bounds of the list
	if (nIndex < 0)
		nIndex = 0;
	else if (nIndex >= m_TextureList.GetItemCount())
		nIndex = m_TextureList.GetItemCount() - 1;

	// Re-display the texture if it's changed
	if (nOldIndex != nIndex)
		ChangeSelection(nIndex);

	return;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTextureDlg::ChangeSelection
//
//	PURPOSE:
//
// ----------------------------------------------------------------------- //

void CTextureDlg::ChangeSelection(int nIndex, BOOL bClearSelect)
{
	CMainFrame* pFrame = (CMainFrame*)AfxGetMainWnd();
	pFrame->UpdateStatusText(ID_INDICATOR_TEXTURE,((DFileIdent*)m_TextureList.GetItemData(nIndex))->m_Filename);

	//make sure that no other textures are selected
	if (bClearSelect)
	{
		for (int iLoop = 0; iLoop < m_TextureList.GetItemCount(); iLoop++)
			m_TextureList.SetItemState(iLoop, 0, LVIS_SELECTED);
	}

	//set up information about the texture
	m_hCurrentTextureDir = m_hCurrentItem;
	m_nCurrentTextureIndex = nIndex;
	m_TextureList.SetItemState(nIndex, LVIS_SELECTED, LVIS_SELECTED);
	m_TextureList.SetSelectionMark(nIndex);
	m_TextureList.EnsureVisible(nIndex, FALSE);

	GetProjectBar()->SetCurTextureSel((DFileIdent*)m_TextureList.GetItemData(nIndex));
	RenderLargeImage();	
}

void CTextureDlg::SetListItemText(int nItem, CString &relativeFilename)
{
	DFileIdent *pIdent;
	char szTemp[MAX_PATH];
	DWORD width, height;

	// insert the texture dimensions
	pIdent = (DFileIdent*)m_TextureList.GetItemData(nItem);
	if(pIdent)
	{
		GetTextureDimensions(pIdent, &width, &height);

		sprintf(szTemp, "%d x %d", width, height);
		m_TextureList.SetItemText(nItem, 1, szTemp);
	}
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTextureDlg::DoImportOperation
//
//	PURPOSE:
//
// ----------------------------------------------------------------------- //

BOOL CTextureDlg::DoImportOperation(DWORD flags)
{
	CString			importExt, fileMask, newExtension, sFileName;
	CString			pathName, targetFilename, tempStr, relativeTargetFilename;

	POSITION		pos;
	char			fileTitle[256];
	char			fileName[256], fileExt[256];
	
	int				status, item;
	bool			success;
	CMoFileIO		file;
	CStringDlg stringDlg;
	DWORD textureFlags;

	CAbstractIO* pFile = NULL;
	DWORD width, height; 

	
	// If no directory is selected, then we can't import anything.
	if(!IsDirectorySelected())
		return FALSE;


	if(flags & PCX_TO_TEXTURE || flags & TGA_TO_TEXTURE)
	{
		if(flags & PCX_TO_TEXTURE)
		{
			importExt.LoadString( IDS_PCX_EXTENSION );
			fileMask.LoadString( IDS_PCX_FILEMASK );
		}
		else
		{
			importExt.LoadString( IDS_TGA_EXTENSION );
			fileMask.LoadString( IDS_TGA_FILEMASK );
		}

		textureFlags = 0;
	}
	else
	{
		importExt.LoadString( IDS_TEXTURE_EXTENSION );
		fileMask.LoadString( IDS_TEXTURE_FILEMASK );
	}
	
	newExtension.LoadString( IDS_TEXTURE_EXTENSION );
	sFileName = GetProject()->m_BaseProjectDir + "\\Textures\\*" + importExt;

	CHelperFileDlg	dlg( TRUE, importExt, (LPCTSTR)sFileName, OFN_ALLOWMULTISELECT, fileMask, this );
	if( dlg.DoModal() == IDOK )
	{
		BeginWaitCursor( );

		// Note: pPos is NOT used as the filename here!!!
		for( pos=dlg.GetStartPosition(); pos != NULL; )
		{		
			pathName = dlg.GetNextPathName( pos );
			
			GetFileTitle( (LPCTSTR)pathName, fileTitle, 256 );
			CHelpers::ExtractFileNameAndExtension( fileTitle, fileName, fileExt );
		
			// Does the file already exist?
			relativeTargetFilename = dfm_BuildName(m_csCurrentDir, CString(fileName) + newExtension);
			targetFilename = dfm_GetFullFilename(GetFileMgr(), relativeTargetFilename);
			
			if( file.Open(targetFilename, "rb") )
			{
				file.Close();

				tempStr.FormatMessage( IDS_TEXTURE_ALREADY_EXISTS, fileName );
				status = MessageBox( tempStr, AfxGetAppName(), MB_YESNOCANCEL );
			
				if( status == IDYES )
				{
					// Copy the file.
					if( flags & PCX_TO_TEXTURE )
					{
						success = CopyImageToTexture( ITYPE_PCX, pathName, targetFilename, textureFlags );
					}
					else if(flags & TGA_TO_TEXTURE)
					{
						success = CopyImageToTexture( ITYPE_TGA, pathName, targetFilename, textureFlags );
					}
					else
					{
						success = CopyFile( pathName, targetFilename, FALSE );
					}

					if(success) 
					{
						// Update the listbox and select the new texture.
						if( GetProjectBar()->m_pCurTextureSel )
							SelectFileInList( GetProjectBar()->m_pCurTextureSel, FALSE );
						item = AddFileToList(relativeTargetFilename);
						GetProjectBar()->SetCurTextureSel((DFileIdent*)m_TextureList.GetItemData(item));
						SelectFileInList( GetProjectBar()->m_pCurTextureSel, TRUE );
						DoShowThumbnails(GetProjectBar()->m_bShowThumbnails);
					}
				}
				else if( status == IDCANCEL )
				{
					break;
				}
			}
			else
			{
				// Copy the file.
				if( flags & PCX_TO_TEXTURE )
				{
					success = CopyImageToTexture( ITYPE_PCX, pathName, targetFilename, textureFlags );
				}
				else if(flags & TGA_TO_TEXTURE)
				{
					success = CopyImageToTexture( ITYPE_TGA, pathName, targetFilename, textureFlags );
				}
				else
				{
					success = CopyFile( pathName, targetFilename, FALSE );
				}

				if(success) 
				{
					// Update the listbox and select the new texture.
					if( GetProjectBar()->m_pCurTextureSel )
						SelectFileInList( GetProjectBar()->m_pCurTextureSel, FALSE );
					item = AddFileToList(relativeTargetFilename);
					GetProjectBar()->SetCurTextureSel((DFileIdent*)m_TextureList.GetItemData(item));
					SelectFileInList( GetProjectBar()->m_pCurTextureSel, TRUE );
					DoShowThumbnails(GetProjectBar()->m_bShowThumbnails);
				}
			}
		}

//		GetProjectBar( )->SetCurTextureSel( NULL );
//		UpdateDirectories( );
//		PopulateList();
//		RenderLargeImage( );

		EndWaitCursor( );
		return TRUE;
	}

	return FALSE;
}

// opens the dialog to allow the user to enter data for the normal map
bool CTextureDlg::DoImportNormalMapOperation()
{
	CImportBumpMapDlg Dlg(this);
	Dlg.m_sLuminanceText = "Alpha Channel";

	if(Dlg.DoModal() != IDOK)
	{
		//the user cancelled
		return false;
	}

	//make sure that we can open up the file
	BeginWaitCursor();

	//break the list on the semicolon boundaries and process each file
	CString sFilesLeft = Dlg.m_sImageFile;
	
	while(1)
	{
		CString sCurrFile = sFilesLeft;
		int nSemiColonPos = sFilesLeft.Find(';');

		if(nSemiColonPos != -1)
		{
			sCurrFile = sFilesLeft.Left(nSemiColonPos);
			sFilesLeft = sFilesLeft.Mid(nSemiColonPos + 1);
		}
		else
			sFilesLeft = "";

		//cleanup
		sFilesLeft.TrimLeft();
		sFilesLeft.TrimRight();
		sCurrFile.TrimLeft();
		sCurrFile.TrimRight();

		//see if we are done
		if(sCurrFile.IsEmpty())
		{
			//see if this was just two semicolons in a row, creating an empty file
			if(sFilesLeft.IsEmpty())
			{
				//nope, end of list
				break;
			}			
			continue;
		}

		//determines if the file already existed
		bool bFileExisted = false;

		// build the full path of the output dtx file
		char fileName[MAX_PATH], fileExt[MAX_PATH], fileTitle[MAX_PATH];
		GetFileTitle( (LPCTSTR)sCurrFile, fileTitle, MAX_PATH );
		CHelpers::ExtractFileNameAndExtension( fileTitle, fileName, fileExt );

		CString newExtension( (LPCSTR)IDS_TEXTURE_EXTENSION );
		CString relativeTargetFilename = dfm_BuildName( m_csCurrentDir, CString(fileName) + newExtension );
		CString targetFilename = dfm_GetFullFilename( GetFileMgr(), relativeTargetFilename );

		// check if the file exists already
		CMoFileIO file;
		if( file.Open( targetFilename, "rb" ) )
		{
			file.Close();

			CString msg;
			msg.FormatMessage( IDS_TEXTURE_ALREADY_EXISTS, fileName );
			int status = MessageBox( msg, AfxGetAppName(), MB_YESNO );
			if( status == IDNO )
			{
				continue;
			}

			//the file existed
			bFileExisted = true;
		}

		// load the source tga
		LoadedBitmap bitmap;
		DStream* sourceFile;
		bool success = true;
		CString msg;

		if( sourceFile = streamsim_Open( sCurrFile, "rb" ) )
		{
			if( tga_Create2( sourceFile, &bitmap ) )
			{
				if(!dtx_IsTextureSizeValid( bitmap.m_Width ) || !dtx_IsTextureSizeValid( bitmap.m_Height ))
				{
					// failed due to bad texture dimensions
					msg.FormatMessage( IDS_INVALIDTEXTURESIZE, sCurrFile, bitmap.m_Width, bitmap.m_Height );
					AppMessageBox( msg, MB_OK );
					success = false;
				}
			}
			else
			{
				// failed due to bad tga file
				msg.FormatMessage( IDS_CANTLOADTGA, sCurrFile );
				AppMessageBox( msg, MB_OK );
				success = false;
			}

			sourceFile->Release();
		}
		else
		{
			// failed due to nonexistent file
			msg.FormatMessage( IDS_CANTLOADTGA, sCurrFile );
			AppMessageBox( msg, MB_OK );
			success = false;
		}

		// create the normal map file
		if( success )
		{
			DStream* destFile = streamsim_Open( targetFilename, "wb" );
			if( destFile )
			{
				bool bAlpha = (Dlg.m_nLuminanceChannel != CImportBumpMapDlg::NONE);

				SaveNormalMap( &bitmap, destFile, Dlg.m_nHeightChannel, bAlpha, Dlg.m_nLuminanceChannel, Dlg.m_fHeight );
				destFile->Release();
			}
			else
			{
				msg.FormatMessage( IDS_ERRORSAVEDTX, targetFilename );
				AppMessageBox( msg, MB_OK );
			}
		}

		if( success )
		{
			// update the listbox and select the new texture if it didn't exist already
			if(!bFileExisted)
			{
				if( GetProjectBar()->m_pCurTextureSel )
					SelectFileInList( GetProjectBar()->m_pCurTextureSel, FALSE );
				int item = AddFileToList( relativeTargetFilename );
				GetProjectBar()->SetCurTextureSel( (DFileIdent*)m_TextureList.GetItemData( item ) );
				SelectFileInList( GetProjectBar()->m_pCurTextureSel, TRUE );
			}
			
		}
	}

	DoShowThumbnails(GetProjectBar()->m_bShowThumbnails);
	EndWaitCursor();

	return true;
}


// opens the dialog to allow the user to enter data for the bump map
bool CTextureDlg::DoImportBumpMapOperation()
{
	CImportBumpMapDlg Dlg(this);

	if(Dlg.DoModal() != IDOK)
	{
		//the user cancelled
		return false;
	}


	//make sure that we can open up the file
	BeginWaitCursor();

	//break the list on the semicolon boundaries and process each file
	CString sFilesLeft = Dlg.m_sImageFile;
	
	while(1)
	{
		CString sCurrFile = sFilesLeft;
		int nSemiColonPos = sFilesLeft.Find(';');

		if(nSemiColonPos != -1)
		{
			sCurrFile = sFilesLeft.Left(nSemiColonPos);
			sFilesLeft = sFilesLeft.Mid(nSemiColonPos + 1);
		}
		else
			sFilesLeft = "";

		//cleanup
		sFilesLeft.TrimLeft();
		sFilesLeft.TrimRight();
		sCurrFile.TrimLeft();
		sCurrFile.TrimRight();

		//see if we are done
		if(sCurrFile.IsEmpty())
		{
			//see if this was just two semicolons in a row, creating an empty file
			if(sFilesLeft.IsEmpty())
			{
				//nope, end of list
				break;
			}			
			continue;
		}

		//determines if the file already existed
		bool bFileExisted = false;

		// build the full path of the output dtx file
		char fileName[MAX_PATH], fileExt[MAX_PATH], fileTitle[MAX_PATH];
		GetFileTitle( (LPCTSTR)sCurrFile, fileTitle, MAX_PATH );
		CHelpers::ExtractFileNameAndExtension( fileTitle, fileName, fileExt );

		CString newExtension( (LPCSTR)IDS_TEXTURE_EXTENSION );
		CString relativeTargetFilename = dfm_BuildName( m_csCurrentDir, CString(fileName) + newExtension );
		CString targetFilename = dfm_GetFullFilename( GetFileMgr(), relativeTargetFilename );

		// check if the file exists already
		CMoFileIO file;
		if( file.Open( targetFilename, "rb" ) )
		{
			file.Close();

			CString msg;
			msg.FormatMessage( IDS_TEXTURE_ALREADY_EXISTS, fileName );
			int status = MessageBox( msg, AfxGetAppName(), MB_YESNO );
			if( status == IDNO )
			{
				continue;
			}

			//the file existed
			bFileExisted = true;
		}

		// load the source tga
		LoadedBitmap bitmap;
		DStream* sourceFile;
		bool success = true;
		CString msg;

		if( sourceFile = streamsim_Open( sCurrFile, "rb" ) )
		{
			if( tga_Create2( sourceFile, &bitmap ) )
			{
				if(!dtx_IsTextureSizeValid( bitmap.m_Width ) || !dtx_IsTextureSizeValid( bitmap.m_Height ))
				{
					// failed due to bad texture dimensions
					msg.FormatMessage( IDS_INVALIDTEXTURESIZE, sCurrFile, bitmap.m_Width, bitmap.m_Height );
					AppMessageBox( msg, MB_OK );
					success = false;
				}
			}
			else
			{
				// failed due to bad tga file
				msg.FormatMessage( IDS_CANTLOADTGA, sCurrFile );
				AppMessageBox( msg, MB_OK );
				success = false;
			}

			sourceFile->Release();
		}
		else
		{
			// failed due to nonexistent file
			msg.FormatMessage( IDS_CANTLOADTGA, sCurrFile );
			AppMessageBox( msg, MB_OK );
			success = false;
		}

		// create the bump map file
		if( success )
		{
			DStream* destFile = streamsim_Open( targetFilename, "wb" );
			if( destFile )
			{
				bool bLuminance = (Dlg.m_nLuminanceChannel != CImportBumpMapDlg::NONE);

				SaveBumpMap( &bitmap, destFile, Dlg.m_nHeightChannel, bLuminance, Dlg.m_nLuminanceChannel, Dlg.m_fHeight );
				destFile->Release();
			}
			else
			{
				msg.FormatMessage( IDS_ERRORSAVEDTX, targetFilename );
				AppMessageBox( msg, MB_OK );
			}
		}

		if( success )
		{
			// update the listbox and select the new texture if it didn't exist already
			if(!bFileExisted)
			{
				if( GetProjectBar()->m_pCurTextureSel )
					SelectFileInList( GetProjectBar()->m_pCurTextureSel, FALSE );
				int item = AddFileToList( relativeTargetFilename );
				GetProjectBar()->SetCurTextureSel( (DFileIdent*)m_TextureList.GetItemData( item ) );
				SelectFileInList( GetProjectBar()->m_pCurTextureSel, TRUE );
			}
		}
	}

	DoShowThumbnails(GetProjectBar()->m_bShowThumbnails);
	EndWaitCursor();

	return true;
}


// brings up a dialog requesting 6 tga files and a resulting filename for a cube map
bool CTextureDlg::DoImportCubeMapOperation( void )
{
	// can't import if no directory selected
	if( !IsDirectorySelected() )
		return false;

	bool bFileExisted = false;

	CImportCubeMapDlg dlg( this );

	if( dlg.DoModal() == IDOK )
	{
		BeginWaitCursor();

		// build the full path of the output dtx file
		char fileName[MAX_PATH], fileExt[MAX_PATH], fileTitle[MAX_PATH];
		GetFileTitle( (LPCTSTR)dlg.m_OutputName, fileTitle, MAX_PATH );
		CHelpers::ExtractFileNameAndExtension( fileTitle, fileName, fileExt );
		CString newExtension( (LPCSTR)IDS_TEXTURE_EXTENSION );
		CString relativeTargetFilename = dfm_BuildName( m_csCurrentDir, CString(fileName) + newExtension );
		CString targetFilename = dfm_GetFullFilename( GetFileMgr(), relativeTargetFilename );

		// check if the file exists already
		CMoFileIO file;
		if( file.Open( targetFilename, "rb" ) )
		{
			file.Close();

			CString msg;
			msg.FormatMessage( IDS_TEXTURE_ALREADY_EXISTS, fileName );
			int status = MessageBox( msg, AfxGetAppName(), MB_YESNO );
			if( status == IDNO )
			{
				EndWaitCursor();
				return false;
			}
			bFileExisted = true;
		}

		// load the source tga files
		LoadedBitmap* bitmaps = new LoadedBitmap[6];
		DStream* sourceFile;
		bool success = true;
		int mapWidth;
		CString msg;

		for( int i = 0; i < 6; i++ )
		{
			if( sourceFile = streamsim_Open( dlg.m_InputName[i], "rb" ) )
			{
				if( tga_Create2( sourceFile, &bitmaps[i] ) )
				{
					if( dtx_IsTextureSizeValid( bitmaps[i].m_Width ) && (bitmaps[i].m_Width == bitmaps[i].m_Height) )
					{
						// remember the dimensions of the PosX texture so we can test the rest
						if( i == 0 )
						{
							mapWidth = bitmaps[i].m_Width;
						}
						else if( mapWidth != bitmaps[i].m_Width )
						{
							// failed due to a cube face being a different size
							msg.FormatMessage( IDS_INVALIDTEXTURESIZE, dlg.m_InputName[i], bitmaps[i].m_Width, bitmaps[i].m_Height );
							AppMessageBox( msg, MB_OK );
							success = false;
							break;
						}
					}
					else
					{
						// failed due to bad texture dimensions
						msg.FormatMessage( IDS_INVALIDTEXTURESIZE, dlg.m_InputName[i], bitmaps[i].m_Width, bitmaps[i].m_Height );
						AppMessageBox( msg, MB_OK );
						success = false;
						break;
					}
				}
				else
				{
					// failed due to bad tga file
					msg.FormatMessage( IDS_CANTLOADTGA, dlg.m_InputName[i] );
					AppMessageBox( msg, MB_OK );
					success = false;
					break;
				}

				sourceFile->Release();
			}
			else
			{
				// failed due to nonexistent file
				msg.FormatMessage( IDS_CANTLOADTGA, dlg.m_InputName[i] );
				AppMessageBox( msg, MB_OK );
				success = false;
				break;
			}
		}

		// create the cube map file
		if( success )
		{
			DStream* destFile = streamsim_Open( targetFilename, "wb" );
			if( destFile )
			{
				SaveCubeMap( bitmaps, destFile );
				destFile->Release();
			}
			else
			{
				msg.FormatMessage( IDS_ERRORSAVEDTX, targetFilename );
				AppMessageBox( msg, MB_OK );
			}
		}

		delete [] bitmaps;

		if( success )
		{
			// update the listbox and select the new texture if it didn't already exist
			if(!bFileExisted)
			{
				if( GetProjectBar()->m_pCurTextureSel )
					SelectFileInList( GetProjectBar()->m_pCurTextureSel, FALSE );
				int item = AddFileToList( relativeTargetFilename );
				GetProjectBar()->SetCurTextureSel( (DFileIdent*)m_TextureList.GetItemData( item ) );
				SelectFileInList( GetProjectBar()->m_pCurTextureSel, TRUE );
			}
			DoShowThumbnails(GetProjectBar()->m_bShowThumbnails);
		}

		EndWaitCursor();
		return true;
	}

	return false;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTextureDlg::DoImportMipPcxOperation
//
//	PURPOSE:
//
// ----------------------------------------------------------------------- //

BOOL CTextureDlg::DoImportMipPcxOperation( )
{
	static const uint32 knMaxMipMaps = 16;

	CString			importExt, fileMask, newExtension, newFileMask, sFileName;
	CString			pathName[knMaxMipMaps], targetFilename, targetName, tempStr, relativeTargetFilename;

	POSITION		pos;
		
	int				status;
	int				nFileCount;

	char			fileTitle[knMaxMipMaps][_MAX_PATH+1];
	char			fileName[knMaxMipMaps][_MAX_FNAME+1];
	char			fileExt[knMaxMipMaps][_MAX_EXT+1];

	CAbstractIO* pFile = NULL;
	DWORD width, height;


	importExt.LoadString( IDS_TGA_EXTENSION );
	fileMask.LoadString( IDS_TGA_FILEMASK );
	
	newExtension.LoadString( IDS_TEXTURE_EXTENSION );
	newFileMask.LoadString( IDS_TEXTURE_FILEMASK );

	// Get the mip tga files...
	nFileCount = 0;
	while( nFileCount < knMaxMipMaps )
	{
		tempStr.FormatMessage( IDS_OPENMIPFILE, nFileCount + 1 );
		sFileName = GetProject()->m_BaseProjectDir + "\\Tex\\*" + importExt;
		CHelperFileDlg dlg( TRUE, importExt, (LPCTSTR)sFileName, OFN_FILEMUSTEXIST, fileMask, this );
		dlg.m_ofn.lpstrTitle = LPCTSTR( tempStr );

		if( dlg.DoModal() == IDOK )
		{

			// Note: pPos is NOT used as the filename here!!!
			pathName[nFileCount] = dlg.GetPathName();
			GetFileTitle( (LPCTSTR)pathName[nFileCount], fileTitle[nFileCount], _MAX_PATH );
			CHelpers::ExtractFileNameAndExtension( fileTitle[nFileCount], fileName[nFileCount], fileExt[nFileCount] );

			nFileCount++;
		}
		else
		{
			break;
		}
	}

	//make sure we got at least one file
	if(nFileCount == 0)
		return FALSE;
	
	// Get the dtx file outputname...
	targetName = CString(fileName[0]) + newExtension;
	targetFilename = dfm_BuildName(m_csCurrentDir, targetName);
	
	CHelperFileDlg dlgSave( FALSE, newExtension, LPCTSTR( targetName ), OFN_OVERWRITEPROMPT, newFileMask, this );
	tempStr.LoadString( IDS_SAVEDTXFILE );
	dlgSave.m_ofn.lpstrTitle = LPCTSTR( tempStr );
	dlgSave.m_ofn.lpstrInitialDir = GetCurDirPath( );
	if( dlgSave.DoModal( ) == IDOK )
	{
		targetFilename = dlgSave.GetPathName( );
	}
	else
	{
		return FALSE;
	}

	BeginWaitCursor( );

	// Copy the file.
	if( !CopyMipTGAToTexture( pathName, nFileCount, targetFilename ))
	{
		EndWaitCursor( );
		return FALSE;
	}

	//AddFileToList(relativeTargetFilename);
	PopulateList(); // {MD 4/14/98} Do this instead because the file may not be in resource dirs.
	DoShowThumbnails(GetProjectBar()->m_bShowThumbnails);

	EndWaitCursor( );
	return TRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTextureDlg::DoExportAllAsBPP_32POperation
//
//	PURPOSE:
//
// ----------------------------------------------------------------------- //

//! todo
BOOL CTextureDlg::DoExportAllAsBPP_32POperation( )
{
	CMoFileIO	outFile;
	int			palNumber, temp;
	WORD		wWidth, wHeight;
	DStream *pStream;

	CString pcxExt, fileMask, pcxFilename, pcxName, str;
	char dtxName[_MAX_FNAME], dtxTitle[_MAX_FNAME], dtxExt[_MAX_EXT];
	CString dtxFilename;
	DFileIdent *pRez;
	CMoArray<DFileIdent*> files;
	int iItem;
	
	pcxExt = ".dt8";

	//! get all the filenames
	for(int iIndex = 0; iIndex < m_TextureList.GetItemCount(); iIndex++)
	{
		// OutputDebugString("\n"); OutputDebugString(pIdent->m_Filename);
		files.Append((DFileIdent*) m_TextureList.GetItemData(iIndex));
	}

	// create a subdirectory under Textures\..\palettized
	// convert into BPP_32P textures
	for(int i=0; i < files; i++)
	{
		dtxFilename = dfm_GetFullFilename(GetFileMgr(), files[i]->m_Filename);
		if(dtxFilename.GetLength() > 4)
		{
			pcxFilename = dtxFilename.Left(dtxFilename.GetLength()-4) + pcxExt;

			if(!outFile.Open(pcxFilename, "wb"))
			{
				str.FormatMessage( IDS_ERRORSAVEPCX, pcxFilename );
				AppMessageBox( str, MB_OK );
				continue;
			}

			if(!(pStream = streamsim_Open((LPCTSTR)dtxFilename, "rb")))
			{
				outFile.Close();
				str.LoadString( IDS_ERR_OPENFILE );
				AppMessageBox( str, MB_OK );
				continue;
			}

			if(!SaveDtxAs8Bit(pStream, outFile, &pcxFilename))
			{
				pStream->Release();
				EndWaitCursor( );
				str.FormatMessage( IDS_ERRORSAVEDTX, pcxFilename );
				AppMessageBox( str, MB_OK );
				continue;
			}

			pStream->Release();
		}
	}

	return TRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTextureDlg::DoConvertTo32P
//
//	PURPOSE:	Convert the currently selected .dtx files to 32P
//				This operation overwrites the existing .dtx file.
//				Applicable header information is retained.
//
// ----------------------------------------------------------------------- //

BOOL CTextureDlg::DoConvertTo32P( )
{
	CMoArray<DFileIdent*> files;		// list of selected textures
	DStream* openFile;					// current file being read
	TextureData* texData;				// image data of the existing file
	uint8 bogusAlpha;					// unused parameter
	CMoFileIO bogusFile;				// unused parameter

	CString messageBoxTitle( "Convert to BPP_32P" );
	CString error;

	// get the selected textures
	int i = -1;
	while( -1 != (i = m_TextureList.GetNextItem( i, LVNI_ALL|LVNI_SELECTED )) )
	{
		files.Append( (DFileIdent*)m_TextureList.GetItemData( i ) );
	}

	if( files <= 0 )
	{
		MessageBox( "At least one texture must be selected to perform this operation.", messageBoxTitle, MB_OK|MB_ICONINFORMATION );
		return TRUE;
	}

	if( MessageBox( "This operation will overwrite the selected textures.\n"
					"Please make sure you have a backup before continuing.\n"
					"Textures must have <= 256 RGBA color combinations\n"
					"in order to convert correctly.",
					 messageBoxTitle, MB_OKCANCEL|MB_ICONINFORMATION ) == IDCANCEL )
	{
		return TRUE;
	}

	BeginWaitCursor();

	// convert each of the files
	for( i = 0; i < files; i++ )
	{
		CString filename = dfm_GetFullFilename( GetFileMgr(), files[i]->m_Filename );

		// open the original file
		if( !(openFile = streamsim_Open( (LPCTSTR)filename, "rb" )) )
		{
			error.Format( "Couldn't open \"%s\" for reading.", filename );
			if( MessageBox( error, messageBoxTitle, MB_OKCANCEL|MB_ICONEXCLAMATION ) == IDCANCEL )
				break;
			continue;
		}

		// get the image data out of the file
		if( dtx_Create( openFile, &texData, FALSE ) != DE_OK )
		{
			openFile->Release();
			error.Format( "Error reading \"%s\".", filename );
			if( MessageBox( error, messageBoxTitle, MB_OKCANCEL|MB_ICONEXCLAMATION ) == IDCANCEL )
				break;
			continue;
		}

		openFile->Release();

		// write to the existing file, keeping the header info
		bool success = Write8BitDTX( bogusFile, texData, &filename, true );
		dtx_Destroy( texData );

		if( !success )
		{
			error.Format( "Error converting \"%s\".\nFile may be read only.", filename );
			if( MessageBox( error, messageBoxTitle, MB_OKCANCEL|MB_ICONEXCLAMATION ) == IDCANCEL )
				break;
			continue;
		}
	}

	DoShowThumbnails(GetProjectBar()->m_bShowThumbnails);
	EndWaitCursor();

	return TRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTextureDlg::DoRenameTexture
//
//	PURPOSE:	Brings up the rename resource dialog with this the currently
//				selected texture filled in the dialog
//
// ----------------------------------------------------------------------- //

BOOL CTextureDlg::DoRenameTexture( )
{
	int iItem = -1;
	while((iItem = m_TextureList.GetNextItem(iItem, LVNI_ALL|LVNI_SELECTED)) != -1)
	{
		DFileIdent *pItemData = (DFileIdent *)m_TextureList.GetItemData(iItem);
		GetRenameResourceDlg()->AddNewFile(pItemData->m_Filename);
	}

	GetRenameResourceDlg()->UpdateIcons();
	GetRenameResourceDlg()->ShowWindow(SW_SHOW);

	return TRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTextureDlg::DoMakeTextureWritable
//
//	PURPOSE:	Makes the currently selected textures writable
//
// ----------------------------------------------------------------------- //

BOOL CTextureDlg::DoMakeTextureWritable( )
{
	int iItem = -1;
	while((iItem = m_TextureList.GetNextItem(iItem, LVNI_ALL|LVNI_SELECTED)) != -1)
	{
		DFileIdent *pItemData = (DFileIdent *)m_TextureList.GetItemData(iItem);

		CString sFilename = dfm_GetFullFilename(GetFileMgr(), pItemData->m_Filename);

		CFileStatus Status;
		if(CFile::GetStatus(sFilename, Status))
		{
			if(Status.m_attribute & CFile::readOnly)
			{
				//ok, make the file writable
				Status.m_attribute &= ~CFile::readOnly;
				CFile::SetStatus(sFilename, Status);
			}
		}
	}

	return TRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTextureDlg::DoExport8BitDTXOperation
//
//	PURPOSE:
//
// ----------------------------------------------------------------------- //

BOOL CTextureDlg::DoExport8BitDTXOperation( )
{
	CMoFileIO	outFile;
	int			palNumber, temp;
	WORD		wWidth, wHeight;
	DStream *pStream;

	CString pcxExt, fileMask, pcxFilename, pcxName, str;
	char dtxName[_MAX_FNAME], dtxTitle[_MAX_FNAME], dtxExt[_MAX_EXT];
	CString dtxFilename;
	DFileIdent *pRez;
	CMoArray<DFileIdent*> files;
	int iItem;
	DWORD i;
	
	if(!GetProjectBar()->IsProjectOpen())
		return false;

	if(MessageBox("Are you sure you want to save\nall textures as 8-bit .dt8 files?",
					"Convert Textures?", MB_ICONQUESTION | MB_YESNO) == IDNO)		return false;

	iItem = -1;
	while((iItem = m_TextureList.GetNextItem(iItem, LVNI_ALL|LVNI_SELECTED)) != -1)
	{
		files.Append((DFileIdent*)m_TextureList.GetItemData(iItem));
	}

	// pcxExt.LoadString( IDS_TEXTURE_EXTENSION );
	// fileMask.LoadString( IDS_TEXTURE_FILEMASK );

	pcxExt = ".dtx";
	fileMask = "DTX Files (*.dtx)|*.dtx||";

	if(files > 1)
	{
		if(AppMessageBox(IDS_MULTIPLETEXTURESSELECTED, MB_YESNO) == IDNO)
			return FALSE;

		BeginWaitCursor( );
		
		for(i=0; i < files; i++)
		{
			dtxFilename = dfm_GetFullFilename(GetFileMgr(), files[i]->m_Filename);
			if(dtxFilename.GetLength() > 4)
			{
				pcxFilename = dtxFilename.Left(dtxFilename.GetLength()-4) + pcxExt;

				if(!outFile.Open(pcxFilename, "wb"))
				{
					str.FormatMessage( IDS_ERRORSAVEPCX, pcxFilename );
					AppMessageBox( str, MB_OK );
					continue;
				}

				if(!(pStream = streamsim_Open((LPCTSTR)dtxFilename, "rb")))
				{
					outFile.Close();
					str.LoadString( IDS_ERR_OPENFILE );
					AppMessageBox( str, MB_OK );
					continue;
				}

				if(!SaveDtxAs8Bit(pStream, outFile, &pcxFilename))
				{
					pStream->Release();
					EndWaitCursor( );
					str.FormatMessage( IDS_ERRORSAVEDTX, pcxFilename );
					AppMessageBox( str, MB_OK );
					continue;
				}

				pStream->Release();
			}
		}

		EndWaitCursor( );
		return TRUE;
	}
	else
	{
		pRez = GetProjectBar()->m_pCurTextureSel;
		if(!pRez)
			return FALSE;

		dtxFilename = dfm_GetFullFilename(GetFileMgr(), pRez->m_Filename);

		GetFileTitle( (LPCTSTR)dtxFilename, dtxTitle, _MAX_PATH );
		CHelpers::ExtractFileNameAndExtension( dtxTitle, dtxName, dtxExt );

		pcxFilename = dfm_BuildName(m_csCurrentDir, CString(dtxName) + pcxExt);
		pcxName = dtxName + pcxExt;
		CHelperFileDlg dlgSave( FALSE, pcxExt, LPCTSTR( pcxName ), OFN_OVERWRITEPROMPT, fileMask, this );
		dlgSave.m_ofn.lpstrInitialDir = GetCurDirPath( );
		if( dlgSave.DoModal( ) == IDOK )
		{
			pcxFilename = dlgSave.GetPathName( );
		}
		else
		{
			return FALSE;
		}

		if(!outFile.Open(pcxFilename, "wb"))
		{
			str.FormatMessage( IDS_ERRORSAVEDTX, pcxFilename );
			AppMessageBox( str, MB_OK );
			return FALSE;
		}

		if(!(pStream = streamsim_Open((LPCTSTR)dtxFilename, "rb")))
		{
			outFile.Close();
			str.LoadString( IDS_ERR_OPENFILE );
			AppMessageBox( str, MB_OK );
			return FALSE;
		}

		BeginWaitCursor( );

		if(!SaveDtxAs8Bit(pStream, outFile, &pcxFilename ))
		{
			pStream->Release();
			EndWaitCursor( );
			str.FormatMessage( IDS_ERRORSAVEDTX, pcxFilename );
			AppMessageBox( str, MB_OK );
			return FALSE;
		}

		pStream->Release();
		EndWaitCursor( );
		return TRUE;
	}
}


//----------------------------
// DoBatchReload stuff
//----------------------------

// dialog options for DoBatchReload
struct BatchReloadOptions
{
	bool adjustMappings;		// should we adjust mappings in all the worlds in the project
	bool noCompression;			// should new textures be compressed the same as the originals
	char basePath[_MAX_PATH];	// base path for the new texture tree
	int resourceBaseLen;		// length of the base path of the resources directory

	BatchReloadOptions()
	{
		adjustMappings = true;
		noCompression = false;
		basePath[0] = 0;
		resourceBaseLen = 0;
	}
};

// process options from the dialog box for DoBatchReload
void BatchReloadDialogAccepted( HWND hWnd, BatchReloadOptions* options )
{
	options->adjustMappings = SendDlgItemMessage( hWnd, IDC_ADJUSTMAPPINGS, BM_GETSTATE, 0, 0 ) > 0;
	options->noCompression = SendDlgItemMessage( hWnd, IDC_NOCOMPRESSION, BM_GETSTATE, 0, 0 ) > 0;
	GetWindowText( GetDlgItem( hWnd, IDC_BASEPATH ), options->basePath, _MAX_PATH );
	int basePathLen = strlen( options->basePath );

	// make sure path ends in a backslash
	if( basePathLen && options->basePath[basePathLen-1] != '\\' )
	{
		options->basePath[basePathLen] = '\\';
		options->basePath[basePathLen+1] = 0;
		basePathLen++;
	}

	// make path lowercase
	for( int i = 0; i < basePathLen; i++ )
	{
		if( isupper( options->basePath[i] ) )
			options->basePath[i] = tolower( options->basePath[i] );
	}
}


// dialog stuff for DoBatchReload
static BOOL CALLBACK BatchReloadDialogProc( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam )
{
	BatchReloadOptions* options = (BatchReloadOptions*)GetWindowLong( hWnd, GWL_USERDATA );
	if( !options && (msg != WM_INITDIALOG) )
		return FALSE;

	switch( msg )
	{
	case WM_INITDIALOG:
		options = (BatchReloadOptions*)lParam;
		// setup dialog controls to match settings
		SendDlgItemMessage( hWnd, IDC_ADJUSTMAPPINGS, BM_SETCHECK, options->adjustMappings ? BST_CHECKED : BST_UNCHECKED, 0 );
		SendDlgItemMessage( hWnd, IDC_NOCOMPRESSION, BM_SETCHECK, options->noCompression ? BST_CHECKED : BST_UNCHECKED, 0 );
		SetWindowText( GetDlgItem( hWnd, IDC_BASEPATH ), options->basePath );
		// put a pointer to the options data in the user data
		SetWindowLong( hWnd, GWL_USERDATA, lParam );
		break;

	case WM_DESTROY:
		break;

	case WM_COMMAND:
		switch( LOWORD(wParam) )
		{
		case IDC_CANCEL:
			EndDialog( hWnd, 1 );
			break;

		case IDC_OK:
			BatchReloadDialogAccepted( hWnd, options );
			EndDialog( hWnd, 0 );
			break;
		}
		break;

	default:
		return FALSE;
	}

	return TRUE;
}

bool CTextureDlg::ProcessBatchReload( CMoArray<BatchTextureScaleInfo*>& textureInfo, const char* input, const char* output, const BatchReloadOptions& options )
{
	CString str;	// error messages

	char inputDir[_MAX_PATH];
	char outputDir[_MAX_PATH];

	strcpy( inputDir, input );
	strcpy( outputDir, output );

	int inputLen = strlen( inputDir );
	int outputLen = strlen( outputDir );

	if( !inputLen || !outputLen )
	{
		AppMessageBox( IDS_TEXRELOAD_NODIRECTORIES, MB_OK );
		return false;
	}

	if( inputDir[inputLen-1] != '\\' )
	{
		inputDir[inputLen] = '\\';
		inputDir[inputLen+1] = 0;
		inputLen++;
	}

	if( outputDir[outputLen-1] != '\\' )
	{
		outputDir[outputLen] = '\\';
		outputDir[outputLen+1] = 0;
		outputLen++;
	}

	// loop over all .tga and .pcx files in input directory and child directories
	WIN32_FIND_DATA findData;
	HANDLE findHandle;
	CString startDir;

	startDir = inputDir;
	startDir += "*.*";
	findHandle = FindFirstFile( LPCTSTR(startDir), &findData );

	if( findHandle != INVALID_HANDLE_VALUE )
	{
		do
		{
			if( findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY )
			{
				// ignore directories starting with .
				if( findData.cFileName[0] != '.' )
				{
					// found a directory, recurse into it
					CString newInDir = inputDir;
					newInDir += findData.cFileName;
					CString newOutDir = outputDir;
					newOutDir += findData.cFileName;

					ProcessBatchReload( textureInfo, newInDir, newOutDir, options );
				}
			}
			else
			{
				// it's a file, check the file type
				char ext[_MAX_EXT];
				char file[_MAX_FNAME];
				bool isPcx;

				_splitpath( findData.cFileName, NULL, NULL, file, ext );

				// make sure the extension is lower case
				for( unsigned i = 0; i < strlen( ext ); i++ )
				{
					if( isupper( ext[i] ) )
						ext[i] = tolower( ext[i] );
				}

				// found a legit filetype, process it
				if( (isPcx = !strcmp( ext, ".pcx" )) || !strcmp( ext, ".tga" ) )
				{
					CString inFile = inputDir;
					inFile += file;
					inFile += ext;
					CString outFile = outputDir;
					outFile += file;
					outFile += ".dtx";

					// texture rescaling info
					BatchTextureScaleInfo* newTextureInfo = new BatchTextureScaleInfo;

					if( !ProcessTextureReload( newTextureInfo, inFile, outFile, isPcx ) )
					{
						str.FormatMessage( IDS_TEXRELOAD_ERRORPROCESSING, inFile );
						AppMessageBox( str, MB_OK );
						FindClose( findHandle );
						delete newTextureInfo;
						return false;
					}

					// the texture name based off of the resource base path
					newTextureInfo->texName = outFile.Right( outFile.GetLength() - options.resourceBaseLen );

					// append the rescaling info
					textureInfo.Append( newTextureInfo );
				}
			}
		}
		while( FindNextFile( findHandle, &findData ) );

		FindClose( findHandle );
	}

	return true;
}

// replace the image data of out with in.  fills in textureInfo with the texture name
// and new and old dimensions for use in rescaling mapping coordinates in the worlds
bool CTextureDlg::ProcessTextureReload( BatchTextureScaleInfo* textureInfo, const CString& in, const CString& out, bool isPcx )
{
	DStream* outFile;
	DStream* inFile;
	CString str;	// error string

	// open the destination file
	outFile = streamsim_Open( out, "rb" );
	if( !outFile )
	{
		str.FormatMessage( IDS_TEXRELOAD_NOTFOUND, out );
		AppMessageBox( str, MB_OK );
		return false;
	}

	// read the original dtx
	TextureData* dtxData = NULL;
	if( dtx_Create( outFile, &dtxData, true, false ) != LT_OK )
	{
		str.FormatMessage( IDS_TEXRELOAD_OPENERROR, out );
		AppMessageBox( str, MB_OK );
		outFile->Release();
		return false;
	}

	// store the old texture size
	textureInfo->oldX = dtxData->m_Header.m_BaseWidth;
	textureInfo->oldY = dtxData->m_Header.m_BaseHeight;

	outFile->Release();

	// open the input file
	inFile = streamsim_Open( in, "rb" );
	if( !inFile )
	{
		str.FormatMessage( IDS_TEXRELOAD_OPENERROR, in );
		AppMessageBox( str, MB_OK );
		dtx_Destroy( dtxData );
		return false;
	}

	// load the source file
	LoadedBitmap bitmap;
	bool createdBitmap;
	if( isPcx )
	{
		// load the pcx file
		createdBitmap = pcx_Create2( inFile, &bitmap );
	}
	else
	{
		// load the tga file
		createdBitmap = tga_Create2( inFile, &bitmap );
	}
	inFile->Release();

	if( !createdBitmap )
	{
		str.FormatMessage( IDS_TEXRELOAD_BADFORMAT, in );
		AppMessageBox( str, MB_OK );
		dtx_Destroy( dtxData );
		return false;
	}

	bool success = true;

	// store the new texture size
	textureInfo->newX = bitmap.m_Width;
	textureInfo->newY = bitmap.m_Height;

	if( dtx_IsTextureSizeValid( bitmap.m_Width ) && dtx_IsTextureSizeValid( bitmap.m_Height ) )
	{
		outFile = streamsim_Open( out, "wb" );
		if( outFile )
		{
			if( FillTextureWithPcx( &bitmap, dtxData ) )
			{
				if( LT_OK != dtx_Save( dtxData, outFile ) )
				{
					str.FormatMessage( IDS_TEXRELOAD_SAVEERROR, out );
					AppMessageBox( str, MB_OK );
					success = false;
				}
			}
			else
			{
				str.FormatMessage( IDS_TEXRELOAD_CANTCONVERT, in );
				AppMessageBox( str, MB_OK );
				success = false;
			}

			outFile->Release();
		}
		else
		{
			str.FormatMessage( IDS_TEXRELOAD_CANTWRITE, out );
			AppMessageBox( str, MB_OK );
			success = false;
		}
	}
	else
	{
		str.FormatMessage( IDS_TEXRELOAD_BADDIMS, in );
		AppMessageBox( str, MB_OK );
		success = false;
	}

	dtx_Destroy( dtxData );

	return success;
}

// stuff new texture image data into a dtx, then go through all the worlds in the
// project and resize the mapping to fit the new texture size
bool CTextureDlg::DoBatchReload( void )
{
	BatchReloadOptions options;						// dialog box options
	CMoArray<BatchTextureScaleInfo*> textureInfo;	// texture resizing info

	// show the reload dialog
	int res = DialogBoxParam( GetApp()->m_hInstance, MAKEINTRESOURCE(IDD_TEXTURE_BATCHRELOAD), m_hWnd, BatchReloadDialogProc, (LPARAM)&options );

	// they canceled
	if( res != 0 ) return false;

	// get the base resource directory and it's length for stripping off of texture names
	CString resourceBase = GetProject()->m_BaseProjectDir;
	options.resourceBaseLen = resourceBase.GetLength();
	if( options.resourceBaseLen && (resourceBase[options.resourceBaseLen - 1] != '\\') )
		options.resourceBaseLen++;

	BeginWaitCursor();

	// reload any new texture image data that is available
	if( !ProcessBatchReload( textureInfo, options.basePath, resourceBase, options ) )
	{
		EndWaitCursor();
		return false;
	}

	// load each world in the project and rescale the mappings as appropriate
	if( options.adjustMappings )
	{
		if( !GetProjectBar()->BatchTextureScale( textureInfo, resourceBase ) )
		{
			EndWaitCursor();
			return false;
		}
	}

	EndWaitCursor();

	// delete all the texture info
	for( int i = 0; i < textureInfo; i++ )
	{
		delete textureInfo[i];
	}

	// update the texture dialog
	GetProjectBar()->SetCurTextureSel( NULL );
	UpdateDirectories();
	PopulateList();
	RenderLargeImage();

	return true;
}


//------------------------------
// ScaleTextureCoords stuff
//------------------------------

// dialog options for DoScaleTextureCoords
struct ScaleTextureCoordsOptions
{
	char basePath[_MAX_PATH];	// base path for the new texture tree
	int resourceBaseLen;		// length of the base path of the resources directory

	ScaleTextureCoordsOptions()
	{
		basePath[0] = 0;
		resourceBaseLen = 0;
	}
};

// process options from the dialog box for DoScaleTextureCoords
void ScaleTextureCoordsDialogAccepted( HWND hWnd, ScaleTextureCoordsOptions* options )
{
	GetWindowText( GetDlgItem( hWnd, IDC_BASEPATH ), options->basePath, _MAX_PATH );
	int basePathLen = strlen( options->basePath );

	// make sure path ends in a backslash
	if( basePathLen && options->basePath[basePathLen-1] != '\\' )
	{
		options->basePath[basePathLen] = '\\';
		options->basePath[basePathLen+1] = 0;
		basePathLen++;
	}

	// make path lowercase
	for( int i = 0; i < basePathLen; i++ )
	{
		if( isupper( options->basePath[i] ) )
			options->basePath[i] = tolower( options->basePath[i] );
	}

	options->resourceBaseLen = basePathLen;
}


// dialog stuff for DoScaleTextureCoords
static BOOL CALLBACK ScaleTextureCoordsDialogProc( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam )
{
	ScaleTextureCoordsOptions* options = (ScaleTextureCoordsOptions*)GetWindowLong( hWnd, GWL_USERDATA );
	if( !options && (msg != WM_INITDIALOG) )
		return FALSE;

	switch( msg )
	{
	case WM_INITDIALOG:
		options = (ScaleTextureCoordsOptions*)lParam;
		// setup dialog controls to match settings
		SetWindowText( GetDlgItem( hWnd, IDC_BASEPATH ), options->basePath );
		// put a pointer to the options data in the user data
		SetWindowLong( hWnd, GWL_USERDATA, lParam );
		break;

	case WM_DESTROY:
		break;

	case WM_COMMAND:
		switch( LOWORD(wParam) )
		{
		case IDC_CANCEL:
			EndDialog( hWnd, 1 );
			break;

		case IDC_OK:
			ScaleTextureCoordsDialogAccepted( hWnd, options );
			EndDialog( hWnd, 0 );
			break;
		}
		break;

	default:
		return FALSE;
	}

	return TRUE;
}

// go through all the worlds in the project and resize the mappings to fit the new texture size
bool CTextureDlg::DoScaleTextureCoords( void )
{
	ScaleTextureCoordsOptions options;				// dialog box options
	CMoArray<BatchTextureScaleInfo*> textureInfo;	// texture resizing info

	// get the filename of the bute file containing scale information
	CString buteFileName;
	buteFileName = GetProject()->m_BaseProjectDir + "\\*.dcf";
	CFileDialog fileDlg( TRUE, "dcf", (LPCTSTR)buteFileName, 0, "DCF File (*.dcf)|*.dcf||" );
	if( fileDlg.DoModal() != IDOK )
		return false;
	buteFileName = fileDlg.GetFileName();

	// show the scale dialog
	int res = DialogBoxParam( GetApp()->m_hInstance, MAKEINTRESOURCE(IDD_TEXTURE_SCALETEXTURECOORDS), m_hWnd, ScaleTextureCoordsDialogProc, (LPARAM)&options );

	// they canceled
	if( res != 0 )
		return false;

	BeginWaitCursor();

	// load the rescale info from the bute file
	CButeMgr buteMgr;
	CString curFileName;
	CString curFileIndex;
	CPoint curOldSize, curNewSize;
	buteMgr.Init();

	if( !buteMgr.Parse( buteFileName ) )
	{
		EndWaitCursor();
		return false;
	}

	int numScaledFiles = buteMgr.GetInt( "General", "NumFiles", 0 );

	// loop over the files in the bute and add the scaling info to textureInfo
	for( int i = 0; i < numScaledFiles; i++ )
	{
		// load the scale info for this file
		curFileIndex.Format( "File%d", i+1 );
		curFileName = buteMgr.GetString( curFileIndex, "Path", CString("") );
		curOldSize = buteMgr.GetPoint( curFileIndex, "OldSize", CPoint(256,256) );
		curNewSize = buteMgr.GetPoint( curFileIndex, "NewSize", CPoint(256,256) );

		// create texture rescaling info structure and add it to textureInfo
		BatchTextureScaleInfo* newTextureInfo = new BatchTextureScaleInfo;
		newTextureInfo->newX = curNewSize.x;
		newTextureInfo->newY = curNewSize.y;
		newTextureInfo->oldX = curOldSize.x;
		newTextureInfo->oldY = curOldSize.y;
		newTextureInfo->texName = curFileName.Right( curFileName.GetLength() - options.resourceBaseLen );
		textureInfo.Append( newTextureInfo );
	}

	// load each world in the project and rescale the mappings as appropriate
	if( !GetProjectBar()->BatchTextureScale( textureInfo, GetProject()->m_BaseProjectDir ) )
	{
		EndWaitCursor();
		return false;
	}

	EndWaitCursor();

	// delete all the texture info
	for( i = 0; i < textureInfo; i++ )
	{
		delete textureInfo[i];
	}

	return true;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTextureDlg::DoExportPcxOperation
//
//	PURPOSE:
//
// ----------------------------------------------------------------------- //

BOOL CTextureDlg::DoExportPcxOperation( )
{
	CMoFileIO	outFile;
	int			palNumber, temp;
	WORD		wWidth, wHeight;
	DStream *pStream;

	CString pcxExt, fileMask, pcxFilename, pcxName, str;
	char dtxName[_MAX_FNAME], dtxTitle[_MAX_FNAME], dtxExt[_MAX_EXT];
	CString dtxFilename;
	DFileIdent *pRez;
	CMoArray<DFileIdent*> files;
	int iItem;
	DWORD i;

	
	if(!GetProjectBar()->IsProjectOpen())
		return FALSE;

	iItem = -1;
	while((iItem = m_TextureList.GetNextItem(iItem, LVNI_ALL|LVNI_SELECTED)) != -1)
	{
		files.Append((DFileIdent*)m_TextureList.GetItemData(iItem));
	}

	pcxExt.LoadString( IDS_TGA_EXTENSION );
	fileMask.LoadString( IDS_TGA_FILEMASK );

	if(files > 1)
	{
		if(AppMessageBox(IDS_MULTIPLETEXTURESSELECTED, MB_YESNO) == IDNO)
			return FALSE;

		BeginWaitCursor( );
		
		for(i=0; i < files; i++)
		{
			dtxFilename = dfm_GetFullFilename(GetFileMgr(), files[i]->m_Filename);
			if(dtxFilename.GetLength() > 4)
			{
				pcxFilename = dtxFilename.Left(dtxFilename.GetLength()-4) + pcxExt;

				if(!outFile.Open(pcxFilename, "wb"))
				{
					str.FormatMessage( IDS_ERRORSAVEPCX, pcxFilename );
					AppMessageBox( str, MB_OK );
					continue;
				}

				if(!(pStream = streamsim_Open((LPCTSTR)dtxFilename, "rb")))
				{
					outFile.Close();
					str.LoadString( IDS_ERR_OPENFILE );
					AppMessageBox( str, MB_OK );
					continue;
				}

				if(!SaveDtxAsTga(pStream, outFile))
				{
					pStream->Release();
					EndWaitCursor( );
					str.FormatMessage( IDS_ERRORSAVEPCX, pcxFilename );
					AppMessageBox( str, MB_OK );
					continue;
				}

				pStream->Release();
			}
		}

		EndWaitCursor( );
		return TRUE;
	}
	else
	{
		pRez = GetProjectBar()->m_pCurTextureSel;
		if(!pRez)
			return FALSE;

		dtxFilename = dfm_GetFullFilename(GetFileMgr(), pRez->m_Filename);

		GetFileTitle( (LPCTSTR)dtxFilename, dtxTitle, _MAX_PATH );
		CHelpers::ExtractFileNameAndExtension( dtxTitle, dtxName, dtxExt );

		pcxFilename = dfm_BuildName(m_csCurrentDir, CString(dtxName) + pcxExt);
		pcxName = dtxName + pcxExt;
		CHelperFileDlg dlgSave( FALSE, pcxExt, LPCTSTR( pcxName ), OFN_OVERWRITEPROMPT, fileMask, this );
		dlgSave.m_ofn.lpstrInitialDir = GetCurDirPath( );
		if( dlgSave.DoModal( ) == IDOK )
		{
			pcxFilename = dlgSave.GetPathName( );
		}
		else
		{
			return FALSE;
		}

		if(!outFile.Open(pcxFilename, "wb"))
		{
			str.FormatMessage( IDS_ERRORSAVEPCX, pcxFilename );
			AppMessageBox( str, MB_OK );
			return FALSE;
		}

		if(!(pStream = streamsim_Open((LPCTSTR)dtxFilename, "rb")))
		{
			outFile.Close();
			str.LoadString( IDS_ERR_OPENFILE );
			AppMessageBox( str, MB_OK );
			return FALSE;
		}

		BeginWaitCursor( );

		if(!SaveDtxAsTga(pStream, outFile))
		{
			pStream->Release();
			EndWaitCursor( );
			str.FormatMessage( IDS_ERRORSAVEPCX, pcxFilename );
			AppMessageBox( str, MB_OK );
			return FALSE;
		}

		pStream->Release();
		EndWaitCursor( );
		return TRUE;
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTextureDlg::DoTextureProperties
//
//	PURPOSE:
//
// ----------------------------------------------------------------------- //

BOOL CTextureDlg::DoTextureProperties( )
{
	if(!GetProjectBar()->IsProjectOpen())
		return FALSE;

	CMoArray<DFileIdent*> selections;

	// Get the selections into an array.
	int nIndex = -1;
	while((nIndex = m_TextureList.GetNextItem(nIndex, LVNI_SELECTED)) != -1)
	{
		DFileIdent *pRez = (DFileIdent*)m_TextureList.GetItemData(nIndex);
		if(pRez)
			selections.Append(pRez);
	}

	return DoTextureProperties( selections );
}


BOOL CTextureDlg::DoTextureProperties( CMoArray<DFileIdent*>& selections )
{
	DFileIdent *pRez;
	CString fullName, str;
	int tempInt;
	DStream *pStream;
	TextureData *pTexture;
	DRESULT dResult;
	DtxSection *pSection;
	int nIndex;
	DWORD i;
	CTextureProp dlgProp;


	if(!GetProjectBar()->IsProjectOpen())
		return FALSE;

	if(selections.GetSize() == 0)
		return FALSE;

	// Setup the dialog from the properties in the first texture.
	BeginWaitCursor();

		fullName = dfm_GetFullFilename(GetFileMgr(), selections[0]->m_Filename);
		pStream = streamsim_Open((LPCTSTR)fullName, "rb");
		if(!pStream)
		{
			AppMessageBox( IDS_ERR_OPENFILE, MB_OK );
			return FALSE;
		}

		dResult = dtx_Create(pStream, &pTexture, TRUE);
		pStream->Release();

		if(dResult != DE_OK)
		{
			str.FormatMessage(IDS_ERRORLOADINGDTX, fullName);
			AppMessageBox(str, MB_OK);
			return FALSE;
		}

		if(pTexture->m_Header.m_Extra[1] == 0)
			dlgProp.m_nMipmaps = 4;
		else
			dlgProp.m_nMipmaps = pTexture->m_Header.m_Extra[1];

		if(pTexture->m_Header.m_Extra[2] & 0x80)
			dlgProp.m_AlphaCutoff = pTexture->m_Header.m_Extra[2] & ~0x80;
		else
			dlgProp.m_AlphaCutoff = 8;

		dlgProp.m_AverageAlpha = pTexture->m_Header.m_Extra[3];

		dlgProp.m_bFullBrights = !!(pTexture->m_Header.m_IFlags&DTX_FULLBRITE);
		dlgProp.m_b32BitSysCopy = !!(pTexture->m_Header.m_IFlags&DTX_32BITSYSCOPY);
		dlgProp.m_bPrefer4444 = !!(pTexture->m_Header.m_IFlags & DTX_PREFER4444);
		dlgProp.m_bPrefer5551 = !!(pTexture->m_Header.m_IFlags & DTX_PREFER5551);
		dlgProp.m_bPrefer16Bit = !!(pTexture->m_Header.m_IFlags & DTX_PREFER16BIT);
		dlgProp.m_bNoSysCache = !!(pTexture->m_Header.m_IFlags & DTX_NOSYSCACHE);
		dlgProp.m_TextureFlags = pTexture->m_Header.m_UserFlags;
		dlgProp.m_TextureGroup = pTexture->m_Header.m_Extra[0];
		dlgProp.m_BPPIdent = pTexture->m_Header.GetBPPIdent();
		dlgProp.m_NonS3TCMipmapOffset = pTexture->m_Header.GetNonS3TCMipmapOffset();
		dlgProp.m_UIMipmapOffset = pTexture->m_Header.GetUIMipmapOffset();
		SAFE_STRCPY(dlgProp.m_CommandString, pTexture->m_Header.m_CommandString);
		dlgProp.m_TexturePriority = pTexture->m_Header.GetTexturePriority();
		dlgProp.m_DetailTextureScale = pTexture->m_Header.GetDetailTextureScale();
		dlgProp.m_DetailTextureAngle = pTexture->m_Header.GetDetailTextureAngle();

		dlgProp.m_pPreviewTexture = pTexture;

	EndWaitCursor( );
	

	DO_DIALOG_AGAIN:;

	if(dlgProp.DoModal() == IDOK)
	{
		for(i=0; i < selections; i++)
		{
			fullName = dfm_GetFullFilename(GetFileMgr(), selections[i]->m_Filename);
			pStream = streamsim_Open((LPCTSTR)fullName, "rb");
			if(!pStream)
			{
				str.FormatMessage(IDS_ERRORLOADINGDTX, fullName);
				AppMessageBox(str, MB_OK);
				continue;
			}

			dResult = dtx_Create(pStream, &pTexture, TRUE);
			pStream->Release();

			if(dResult != DE_OK)
			{
				str.FormatMessage(IDS_ERRORLOADINGDTX, fullName);
				AppMessageBox(str, MB_OK);
				continue;
			}

			// Replace flags.
			if(dlgProp.m_ChangeFlags & TPROP_FLAGS)
			{
				pTexture->m_Header.m_UserFlags = dlgProp.m_TextureFlags;
			}
			
			if(dlgProp.m_ChangeFlags & TPROP_GROUP)
			{
				pTexture->m_Header.m_Extra[0] = (BYTE)dlgProp.m_TextureGroup;
			}

			if(dlgProp.m_ChangeFlags & TPROP_NUMMIPMAPS)
			{
				TextureData* pNewTexture = RegenerateMipMaps(pTexture, (BYTE)DCLAMP(dlgProp.m_nMipmaps, 1, MAX_DTX_MIPMAPS));

				if(pNewTexture)
				{
					dtx_Destroy(pTexture);
					pTexture = pNewTexture;
				}
			}

			if(dlgProp.m_ChangeFlags & TPROP_FULLBRITES)
			{
				pTexture->m_Header.m_IFlags &= ~DTX_FULLBRITE;
				if(dlgProp.m_bFullBrights)
					pTexture->m_Header.m_IFlags |= DTX_FULLBRITE;
			}

			if(dlgProp.m_ChangeFlags & TPROP_32BITSYSCOPY)
			{
				pTexture->m_Header.m_IFlags &= ~DTX_32BITSYSCOPY;
				if(dlgProp.m_b32BitSysCopy)
					pTexture->m_Header.m_IFlags |= DTX_32BITSYSCOPY;
			}

			if(dlgProp.m_ChangeFlags & TPROP_PREFER4444)
			{
				pTexture->m_Header.m_IFlags &= ~DTX_PREFER4444;
				if(dlgProp.m_bPrefer4444)
					pTexture->m_Header.m_IFlags |= DTX_PREFER4444;
			}

			if(dlgProp.m_ChangeFlags & TPROP_PREFER5551)
			{
				pTexture->m_Header.m_IFlags &= ~DTX_PREFER5551;
				if(dlgProp.m_bPrefer5551)
					pTexture->m_Header.m_IFlags |= DTX_PREFER5551;
			}

			if(dlgProp.m_ChangeFlags & TPROP_PREFER16BIT)
			{
				pTexture->m_Header.m_IFlags &= ~DTX_PREFER16BIT;
				if(dlgProp.m_bPrefer16Bit)
					pTexture->m_Header.m_IFlags |= DTX_PREFER16BIT;
			}

			if(dlgProp.m_ChangeFlags & TPROP_NOSYSCACHE)
			{
				pTexture->m_Header.m_IFlags &= ~DTX_NOSYSCACHE;
				if(dlgProp.m_bNoSysCache)
					pTexture->m_Header.m_IFlags |= DTX_NOSYSCACHE;
			}

			if(dlgProp.m_ChangeFlags & TPROP_COMMANDSTRING)
			{
				SAFE_STRCPY(pTexture->m_Header.m_CommandString, dlgProp.m_CommandString);
			}

			if(dlgProp.m_ChangeFlags & TPROP_DATAFORMAT)
			{
				if(pTexture->m_Header.GetBPPIdent() != dlgProp.m_BPPIdent)
				{
					if(!ConvertTextureData(pTexture, dlgProp.m_BPPIdent))
					{
						dtx_Destroy(pTexture);
						str.FormatMessage(IDS_ERRORCONVERTINGTEXTURE, fullName);
						AppMessageBox(str, MB_OK);
						if(selections.GetSize() == 1)
							goto DO_DIALOG_AGAIN;
						else
							continue;
					}
				}
			}

			if(dlgProp.m_ChangeFlags & TPROP_NONS3TCMIPMAPOFFSET)
			{
				pTexture->m_Header.SetNonS3TCMipmapOffset(dlgProp.m_NonS3TCMipmapOffset);
			}

			if(dlgProp.m_ChangeFlags & TPROP_UIMIPMAPOFFSET)
			{
				pTexture->m_Header.SetUIMipmapOffset(dlgProp.m_UIMipmapOffset);
			}

			if(dlgProp.m_ChangeFlags & TPROP_TEXTUREPRIORITY)
			{
				pTexture->m_Header.SetTexturePriority(dlgProp.m_TexturePriority);
			}

			if(dlgProp.m_ChangeFlags & TPROP_DETAILTEXTURESCALE)
			{
				pTexture->m_Header.SetDetailTextureScale(dlgProp.m_DetailTextureScale);
			}

			if(dlgProp.m_ChangeFlags & TPROP_DETAILTEXTUREANGLE)
			{
				pTexture->m_Header.SetDetailTextureAngle(dlgProp.m_DetailTextureAngle);
			}

			// Save it out.
			pStream = streamsim_Open((LPCTSTR)fullName, "wb");
			if(!pStream)
			{
				dtx_Destroy(pTexture);
				str.FormatMessage(IDS_ERRORSAVEDTX, fullName);
				AppMessageBox(str, MB_OK);
				if(selections.GetSize() == 1)
					goto DO_DIALOG_AGAIN;
				else
					continue;
			}

			dtx_Save(pTexture, pStream);
			pStream->Release();
			dtx_Destroy(pTexture);
		}
	}

	// Free the textures so changes take effect.
	dib_FreeAllTextures();
		
	return TRUE;
}


BOOL CTextureDlg::DoCreateAlphaMask(CAMType camType)
{
	DFileIdent *pRez;
	CString fullName;
	int tempInt;
	DStream *pStream;
	TextureData *pTexture;
	DRESULT dResult;
	CString importExt, fileMask, str;
	LoadedBitmap pcxTexture;
	DWORD i, curWidth, curHeight, x, y;
	WORD wVal, iColor;
	TextureMipData *pMip;
	RPaletteColor *pColors;
	AlphaFromColorDlg afcDlg;
	SolidAlphaDlg saDlg;
	float alphaScale, fVal;
	int alphaOffset, sum, theValue;
	DWORD aVal, rVal, gVal, bVal;
	BOOL bLoaded;
	BYTE *pCurLine, *pPos, fillValue;
	TextureUtils tUtils;


	// Load the DTX file.
	pRez = GetProjectBar()->m_pCurTextureSel;
	if(!pRez)
		return FALSE;

	if(camType == 0)
	{
		saDlg.m_Scale = "50";
		if(saDlg.DoModal() != IDOK)
			return FALSE;

		alphaScale = (float)atoi(saDlg.m_Scale) / 100.0f;
		alphaOffset = 0;
	}
	else if(camType == 1)
	{
		if(afcDlg.DoModal() != IDOK)
			return FALSE;

		alphaScale = (float)atoi(afcDlg.m_Scale) / 100.0f;
		alphaOffset = atoi(afcDlg.m_Offset);
	}

	fullName = dfm_GetFullFilename(GetFileMgr(), pRez->m_Filename);
	pStream = streamsim_Open((LPCTSTR)fullName, "rb");
	if(!pStream)
	{
		AppMessageBox( IDS_ERR_OPENFILE, MB_OK );
		return FALSE;
	}

	dResult = dtx_Create(pStream, &pTexture, TRUE);
	pStream->Release();

	if(dResult != DE_OK)
	{
		str.FormatMessage(IDS_ERRORLOADINGDTX, fullName);
		AppMessageBox(str, MB_OK);
		return FALSE;
	}


	// Convert to RGBA..
	if(!tUtils.FromTextureData(pTexture))
	{
		str.FormatMessage(IDS_ERRORLOADINGDTX, fullName);
		AppMessageBox(str, MB_OK);
		return FALSE;
	}


	// Load the PCX file.

	if(camType == CAM_Solid)
	{
		// Setup solid alpha.
		theValue = (int)(255.0f * alphaScale) + alphaOffset;
		theValue = DCLAMP(theValue, 0, 255);
		fillValue = (BYTE)theValue;
		for(i=0; i < pTexture->m_Header.m_nMipmaps; i++)
		{
			pMip = &pTexture->m_Mips[i];
			
			pCurLine = (BYTE*)tUtils.m_PValueMipmaps[i].GetArray();
			for(y=0; y < pMip->m_Height; y++)
			{
				pPos = pCurLine;

				for(x=0; x < pMip->m_Width; x++)
				{
					PValue &pValue = *((PValue*)pPos);
					pValue = PValue_Set(fillValue, PValue_GetR(pValue), PValue_GetG(pValue), PValue_GetB(pValue));

					pPos += sizeof(DWORD);
				}

				pCurLine += pMip->m_Width * sizeof(DWORD);
			}
		}
	}
	else if(camType == CAM_FromColor)
	{
		// Just get alpha from color.
		for(i=0; i < pTexture->m_Header.m_nMipmaps; i++)
		{
			pMip = &pTexture->m_Mips[i];
			
			pCurLine = (BYTE*)tUtils.m_PValueMipmaps[i].GetArray();
			for(y=0; y < pMip->m_Height; y++)
			{
				pPos = pCurLine;

				for(x=0; x < pMip->m_Width; x++)
				{
					PValue &pValue = *((PValue*)pPos);
					
					PValue_Get(pValue, aVal, rVal, gVal, bVal);
					aVal = (rVal + gVal + bVal) / 3;
					pValue = PValue_Set(aVal, rVal, gVal, bVal);

					pPos += sizeof(DWORD);
				}

				pCurLine += pMip->m_Width * sizeof(DWORD);
			}
		}
	}
	else
	{
		importExt.LoadString( IDS_PCX_EXTENSION );
		fileMask.LoadString( IDS_PCX_FILEMASK );
		str = GetProject()->m_BaseProjectDir + "\\Textures\\*" + importExt;
		CFileDialog dlg(TRUE, importExt, (LPCTSTR)str, 0, fileMask, this);

		if(dlg.DoModal() == IDOK)
		{
			bLoaded = FALSE;
			if(pStream = streamsim_Open(dlg.GetPathName(), "rb"))
			{
				if(pcx_Create2(pStream, &pcxTexture))
				{
					bLoaded = TRUE;
				}
				pStream->Release();
			}

			if(!bLoaded)
			{
				dtx_Destroy(pTexture);
				str.FormatMessage(IDS_CANTLOADPCX, dlg.GetPathName());
				AppMessageBox( str, MB_OK );
				return FALSE;
			}

			if(pcxTexture.m_Width != pTexture->m_Header.m_BaseWidth || 
				pcxTexture.m_Height != pTexture->m_Header.m_BaseHeight ||
				pcxTexture.m_Format.m_BPP != BPP_8P)
			{
				dtx_Destroy(pTexture);
				str.FormatMessage(IDS_SIZEDOESNTMATCH);
				AppMessageBox( str, MB_OK );
				return FALSE;
			}
		}
		else
		{
			dtx_Destroy(pTexture);
			return FALSE;
		}


		// This routine wants it in 8-bit form.
		if(camType == CAM_From4BitPCX)
		{
			for(y=0; y < pcxTexture.m_Height; y++)
			{
				for(x=0; x < pcxTexture.m_Width; x++)
				{
					fVal = pcxTexture.m_Data[y*pcxTexture.m_Pitch+x];
					fVal = (fVal * 255.0f) / 15.0f;
					pcxTexture.m_Data[y*pcxTexture.m_Pitch+x] = (BYTE)fVal;
				}
			}
		}


		// Fill in the alpha part of the texture.
		for(i=0; i < pTexture->m_Header.m_nMipmaps; i++)
		{
			pMip = &pTexture->m_Mips[i];
			
			pCurLine = (BYTE*)tUtils.m_PValueMipmaps[i].GetArray();
			for(y=0; y < pMip->m_Height; y++)
			{
				pPos = pCurLine;

				for(x=0; x < pMip->m_Width; x++)
				{
					PValue &pValue = *((PValue*)pPos);
					
					PValue_Get(pValue, aVal, rVal, gVal, bVal);
					pValue = PValue_Set(pcxTexture.Pixel(x, y), rVal, gVal, bVal);

					pPos += sizeof(DWORD);
				}

				pCurLine += pMip->m_Width * sizeof(DWORD);
			}

			// Reduce (for the next mipmap).
			for(y=0; y < pMip->m_Height; y+=2)
			{
				for(x=0; x < pMip->m_Width; x+=2)
				{
					wVal = (WORD)pcxTexture.m_Data[y*pcxTexture.m_Pitch+x] + (WORD)pcxTexture.m_Data[y*pcxTexture.m_Pitch+x+1]
						+ (WORD)pcxTexture.m_Data[(y+1)*pcxTexture.m_Pitch+(x+1)] + (WORD)pcxTexture.m_Data[(y+1)*pcxTexture.m_Pitch+x];

					pcxTexture.m_Data[(y>>1)*pcxTexture.m_Pitch+(x>>1)] = (BYTE)(wVal >> 2);
				}
			}
		}
	}


	// Reconvert into the TextureData.
	if(!tUtils.ToTextureData(pTexture, TRUE))
	{
		str.FormatMessage(IDS_ERRORSAVEDTX, fullName);
		AppMessageBox(str, MB_OK);
		dtx_Destroy(pTexture);	
		return FALSE;		
	}


	// Save the DTX back out.
	pStream = streamsim_Open(fullName, "wb");
	if(!pStream)
	{
		str.FormatMessage(IDS_ERRORSAVEDTX, fullName);
		AppMessageBox( str, MB_OK );
		dtx_Destroy(pTexture);	
		return FALSE;		
   	}
	
	dResult = dtx_Save(pTexture, pStream);
	if(dResult != DE_OK)
	{
		str.FormatMessage(IDS_ERRORSAVEDTX, fullName);
		AppMessageBox( str, MB_OK );
	}

	pStream->Release();
	dtx_Destroy(pTexture);	

	AppMessageBox(IDS_ALPHA_SET_SUCCESSFUL, MB_OK);
	return TRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTextureDlg::DoViewAllTextures
//
//	PURPOSE:
//
// ----------------------------------------------------------------------- //

BOOL CTextureDlg::DoViewAllTextures()
{
	if( IsWindow( m_AllTextureDlg.m_hWnd ))
		return FALSE;

	m_AllTextureDlg.Create(IDD_ALLTEXTUREDLG, this );
	m_AllTextureDlg.ShowWindow( SW_SHOW );

	return TRUE;
}

BOOL CTextureDlg::DoAddToPalette()
{
	// Make sure the texture dialog is open
	if (!IsWindow(m_AllTextureDlg.m_hWnd))
		return FALSE;

	int iItem;

	iItem = -1;
	while((iItem = m_TextureList.GetNextItem(iItem, LVNI_ALL|LVNI_SELECTED)) != -1)
	{
		DFileIdent *pItemData = (DFileIdent *)m_TextureList.GetItemData(iItem);
		m_AllTextureDlg.AddTextureToPalette(pItemData->m_Filename);
	}

	return TRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTextureDlg::CopyPcxToTexture
//
//	PURPOSE:
//
// ----------------------------------------------------------------------- //

bool CTextureDlg::CopyImageToTexture( 
	ImageType iType, const char *pPcxFilename, const char *pTextureFilename, DWORD textureFlags )
{
	LoadedBitmap bitmap;
	CString str;
	DStream *pStream, *pInFile;
	bool rval = false;


	if( pInFile = streamsim_Open(pPcxFilename, "rb") )
	{
		if( (iType == ITYPE_PCX ? pcx_Create2(pInFile, &bitmap) : tga_Create2(pInFile, &bitmap)) )
		{
			if( dtx_IsTextureSizeValid(bitmap.m_Width) && dtx_IsTextureSizeValid(bitmap.m_Height) )
			{
				pStream = streamsim_Open(pTextureFilename, "wb");
				if(pStream)
				{
					SavePcxAsTexture(&bitmap, pStream, textureFlags);
					pStream->Release();
					rval = true;
				}
			}
			else
			{
				str.FormatMessage( IDS_INVALIDTEXTURESIZE, pPcxFilename, bitmap.m_Width, bitmap.m_Height );
				AppMessageBox( str, MB_OK );
			}
		}
		else
		{
			if (iType == ITYPE_PCX)
				str.FormatMessage( IDS_CANTLOADPCX, pPcxFilename );
			else
				str.FormatMessage( IDS_CANTLOADTGA, pPcxFilename );

			AppMessageBox( str, MB_OK );
		}
	
		pInFile->Release();
	}

	return rval;
}


// ----------------------------------------------------------------------- //
//	ROUTINE:	CTextureDlg::CopyMipTGAToTexture
//	PURPOSE:	Builds a DTX from the 4 input TGA files.
// ----------------------------------------------------------------------- //
BOOL CTextureDlg::CopyMipTGAToTexture( const CString *pFileNames, uint32 nNumImages, const char *outFilename )
{
	CMoFileIO outFile;
	DStream *pInFile;
	LoadedBitmap theTexture;
	CString str;
	int	i, prevWidth, prevHeight, x, y;
	TextureData *pDtx;
	TextureMipData *pMip;
	DRESULT dResult;
	BYTE *pInLine, *pOutLine;
	DStream *pStream;
	FMConvertRequest cRequest;
	BOOL bRet;

	
	// Figure out how large the texture will be.
	if(pInFile = streamsim_Open((LPCTSTR)pFileNames[0], "rb"))
	{
		bRet = tga_Create2(pInFile, &theTexture);
		pInFile->Release();
		
		if(!bRet)
		{
			str.FormatMessage( IDS_CANTLOADTGA, pFileNames[0] );
			AppMessageBox( str, MB_OK );
			return FALSE;
		}
	}
	else
	{
		str.FormatMessage( IDS_CANTLOADTGA, pFileNames[0] );
		AppMessageBox( str, MB_OK );
		return FALSE;
	}

	// Create the DTX.
	pDtx = dtx_Alloc(BPP_32, theTexture.m_Width, theTexture.m_Height, nNumImages, NULL, NULL);
	if(!pDtx)
		return FALSE;

	// Convert the mipmaps.
	for(i=0; i < nNumImages; i++)
	{
		pMip = &pDtx->m_Mips[i];
		
		pInFile = streamsim_Open(LPCTSTR( pFileNames[i] ), "rb");
		if(!pInFile)
		{
			dtx_Destroy(pDtx);	
			str.FormatMessage( IDS_CANTLOADTGA, pFileNames[i] );
			AppMessageBox( str, MB_OK );
			return FALSE;
		}

		// Read in image data.
		if(!tga_Create2(pInFile, &theTexture))
		{
			pInFile->Release();
			dtx_Destroy(pDtx);	
			str.FormatMessage( IDS_CANTLOADTGA, pFileNames[i] );
			AppMessageBox( str, MB_OK );
			return FALSE;
		}
		pInFile->Release();

		
		// Make sure sizes are valid.
		if(theTexture.m_Width != pMip->m_Width || theTexture.m_Height != pMip->m_Height)
		{
			dtx_Destroy(pDtx);	
			str.FormatMessage( IDS_INVALIDMIPTGA, i + 1, i );
			AppMessageBox( str, MB_OK );
			return FALSE;
		}

		// Convert the data.
		cRequest.m_pSrc = theTexture.m_Data.GetArray();
		cRequest.m_pSrcPalette = theTexture.m_Palette;
		cRequest.m_SrcPitch = theTexture.m_Pitch;
		*cRequest.m_pSrcFormat = theTexture.m_Format;
		cRequest.m_pDest = pMip->m_Data;
		cRequest.m_DestPitch = pMip->m_Pitch;
		cRequest.m_pDestFormat->InitPValueFormat();
		cRequest.m_Width = pMip->m_Width;
		cRequest.m_Height = pMip->m_Height;
		cRequest.m_Flags = 0;
		
		dResult = g_FormatMgr.ConvertPixels(&cRequest);
		if(dResult != LT_OK)
		{
			dtx_Destroy(pDtx);	
			str.FormatMessage( IDS_INVALIDMIPTGA, i + 1, i );
			AppMessageBox( str, MB_OK );
			return FALSE;			
		}
	}

	// Save the data!
	pStream = streamsim_Open(outFilename, "wb");
	if(!pStream)
	{
		dtx_Destroy(pDtx);	
		str.FormatMessage( IDS_ERRORSAVEDTX, outFilename );
		AppMessageBox( str, MB_OK );
		return FALSE;
	}

	dResult = dtx_Save(pDtx, pStream);
	pStream->Release();

	dtx_Destroy(pDtx);	
	return dResult == DE_OK;
}


void CTextureDlg::OnDestroy() 
{
	CMRCSizeDialogBar::OnDestroy();
	
	// TODO: Add your message handler code here
	m_AllTextureDlg.DestroyWindow();	
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTextureDlg::FindTexture
//
//	PURPOSE:	Finds a texture in the tree & selects it
//
// ----------------------------------------------------------------------- //

BOOL CTextureDlg::FindTexture(DFileIdent *pTexture, BOOL bRecurse, HTREEITEM hBaseTree)
{
	if (!pTexture || !pTexture->m_Filename)
		return FALSE;

	// Handle the entry point
	if (!hBaseTree)
	{
		// Try to find it the fast way...
		if (FastFindTexture(pTexture))
			return TRUE;
		if (!bRecurse)
			return FALSE;
		// Otherwise, use the root item and try to find it by recursing through the directories
		hBaseTree = m_TextureTree.GetRootItem();
	}

	// Check the list at this level
	DDirIdent *pIdent = (DDirIdent*)m_TextureTree.GetItemData(hBaseTree);
	if(pIdent)
	{
		m_csCurrentDir = pIdent->m_Filename;
		m_hCurrentItem = hBaseTree;

		// Note : I hate visibly populating the list like this, but it's the only clean way to get
		//		  the list of files at this level that I'm aware of
		PopulateList();
		
		int iListIndex = FindFileInList(pTexture);
		if (iListIndex >= 0)
		{
			// If we found it, select this item and update the All Textures dialog
			m_TextureTree.Select(hBaseTree, TVGN_CARET);
			ChangeSelection(iListIndex);
			m_AllTextureDlg.NotifyDirChange();
			// And return that we found it
			return TRUE;
		}
	}

	// Check the first child
	HTREEITEM hChild = m_TextureTree.GetChildItem(hBaseTree);
	if (hChild)
	{
		if (FindTexture(pTexture, bRecurse, hChild))
			return TRUE;
	}

	// Check the first sibling
	hBaseTree = m_TextureTree.GetNextSiblingItem(hBaseTree);
	if (hBaseTree)
	{
		if (FindTexture(pTexture, bRecurse, hBaseTree))
			return TRUE;
	}

	// It wasn't in this branch, so return false
	return FALSE;
}

BOOL CTextureDlg::FastFindTexture(DFileIdent *pTexture)
{
	if (!pTexture || !pTexture->m_Filename)
		return FALSE;

	// Make a copy of the name
	CString csName(pTexture->m_Filename);
	// Find the last slash
	int iFinger = csName.ReverseFind('\\');
	if (iFinger < 0)
		return FALSE;

	// Cut off the string at the last slash
	csName = csName.Left(iFinger);

	// Structure for searching the tree
	char cBuffer[256];
	TVITEM sTreeItem;
	memset(&sTreeItem, 0, sizeof(sTreeItem));
	sTreeItem.pszText = cBuffer;
	sTreeItem.cchTextMax = 256;
	sTreeItem.mask = TVIF_TEXT | TVIF_HANDLE;

	// Find the path in the tree
	HTREEITEM hTreeFinger = m_TextureTree.GetRootItem();
	while ((hTreeFinger) && (!csName.IsEmpty()))
	{
		CString csNodeName;

		iFinger = csName.Find('\\');
		// Strip off leading slashes..
		if (!iFinger)
		{
			csName.Delete(0);
			continue;
		}
		// Handle a mid-level node
		else if (iFinger > 0)
		{
			csNodeName = csName.Left(iFinger);
			csName.Delete(0, iFinger + 1);
		}
		// handle the final node
		else
		{
			csNodeName = csName;
			csName.Empty();
		}
		// Find the node at this level of the tree
		while (hTreeFinger) 
		{
			sTreeItem.hItem = hTreeFinger;
			m_TextureTree.GetItem(&sTreeItem);
			// If we found a tree item with the right name, go back through the main loop
			if (!stricmp(cBuffer, (LPCTSTR)csNodeName))
				break;
			// Move to the next sibling
			hTreeFinger = m_TextureTree.GetNextSiblingItem(hTreeFinger);
		}
		// Go down a level if we found a match for this name
		if ((hTreeFinger) && (!csName.IsEmpty()))
			hTreeFinger = m_TextureTree.GetChildItem(hTreeFinger);
	}

	// If we found the folder, try to find it in that directory..
	if (hTreeFinger)
		return FindTexture(pTexture, FALSE, hTreeFinger);

	// Otherwise, it wasn't found...
	return FALSE;
}

// Function to distribute an expansion action to a tree item's children because MFC is too lame
//		to provide this functionality on its own.
static void ExpandTreeChildren(CTreeCtrl &cTree, HTREEITEM hItem, UINT action)
{
	HTREEITEM hFinger = cTree.GetChildItem(hItem);
	while (hFinger)
	{
		// If this node has children, recurse into them..
		if (cTree.GetChildItem(hFinger))
			ExpandTreeChildren(cTree, hFinger, action);
		// Move to the next sibling
		hFinger = cTree.GetNextSiblingItem(hFinger);
	}
	// Visit this node
	if (cTree.GetChildItem(hItem))
		cTree.Expand(hItem, action);
}

void CTextureDlg::OnItemexpandingBaserezTree(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_TREEVIEW* pNMTreeView = (NM_TREEVIEW*)pNMHDR;

	if (pNMTreeView->action == TVE_COLLAPSE)
		ExpandTreeChildren(m_TextureTree, pNMTreeView->itemNew.hItem, TVE_COLLAPSE);
	
	*pResult = 0;
}




//this must be overridden by a derived class to render the icon for the appropriate
//list item
bool CTextureDlg::RenderIcon(HDC BlitTo, uint32 nXOff, uint32 nImgSize, uint32 nItem)
{
	CTexture* pTexture = dib_GetDibTexture((DFileIdent*)m_TextureList.GetItemData(nItem));

	if (!pTexture)  
		return false;

	pTexture->m_pDib->Blt(BlitTo, nXOff, 0, nImgSize, nImgSize);
	return true;
}

//this must be overridden by a derived class to render the large selected image
void CTextureDlg::RenderLargeImage()
{
	//setup the rectangle we are going to blit to
	CRect InitialRect = CBaseImgDlg::InitLargeImageRect();
	CRect DrawRect = InitialRect;

	//get a DC
	CDC *pDC = GetDC();
	if(!pDC)
		return;

	DFileIdent	*pRez = NULL;

	if(GetProjectBar()->IsProjectOpen())
		pRez = GetProjectBar()->m_pCurTextureSel;

	if(pRez == NULL)
	{
		ReleaseDC(pDC);
		return;
	}

	CTexture *pTexture = dib_GetDibTexture(pRez);
	if(!pTexture)
	{
		ReleaseDC(pDC);
		return;
	}

	CDib *pDib = pTexture->m_pDib;
	CRect texRect(0,0,pDib->GetWidth(),pDib->GetHeight());
	texRect.right >>= pTexture->m_UIMipmapOffset;
	texRect.bottom >>= pTexture->m_UIMipmapOffset;


	//adjust the origin of the texture
	int nTmp = (DrawRect.top + (DrawRect.bottom/6)) - (texRect.bottom/2);
	if(nTmp > DrawRect.top)
		DrawRect.top = nTmp;

	DrawRect.left = (DrawRect.right/2) - (texRect.right/2);
	
	nTmp = DrawRect.top + texRect.bottom;
	if(nTmp < DrawRect.bottom)
		DrawRect.bottom = nTmp;

	DrawRect.right = DrawRect.left + texRect.right;

	// Blit the texture in.
	SetStretchBltMode( pDC->m_hDC, COLORONCOLOR );
	pDib->Blt( pDC->m_hDC, DrawRect.left, DrawRect.top, DrawRect.Width()+1, DrawRect.Height()+1 );
	
	// Draw the texture name.
	char pszToPrint[MAX_PATH];
	CHelpers::ExtractNames( pRez->m_Filename, NULL, NULL, pszToPrint, NULL );

	CBaseImgDlg::PrintLargeImageText(pDC->m_hDC, InitialRect, pszToPrint);

	ReleaseDC(pDC);
}

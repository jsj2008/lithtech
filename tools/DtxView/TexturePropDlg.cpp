//------------------------------------------------------------------
//
//  FILE      : TexturePropDlg.cpp
//
//  PURPOSE   :	Texture properties dialog
//
//  COPYRIGHT : (c) 2003 Touchdown Entertainment, Inc. All rights reserved.
//
//------------------------------------------------------------------

#include "stdafx.h"
#include "bdefs.h"
#include "resource.h"
#include "TexturePropDlg.h"
#include "s3tc_compress.h"
#include "oldtypes.h"

#ifdef _DEBUG
#	define new DEBUG_NEW
#	undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif



CTexturePropDlg *g_pTexturePropDlg=NULL;


/////////////////////////////////////////////////////////////////////////////
// Preview window functionality.
/////////////////////////////////////////////////////////////////////////////

static void PaintPreviewWnd(CTexturePropDlg *pDlg, HWND hWnd, HDC hDC)
{
	CMoDWordArray rgbData;
	CMoWordArray rgb4444Data;
	TextureData *pTexture;
	FMConvertRequest cRequest;
	FormatMgr formatMgr;
	DRESULT dResult;
	DWORD x, y, *pInLine, r, g, b, a, outYCoord, iMipmap;
	S3TC_Compressor compressor;
	HPEN hPen, hOldPen;
	int oldROP;
	BPPIdent bppIdent;
	char oldText[256];
	TextureMipData *pMip;


	if(!pDlg || !pDlg->m_pPreviewTexture)
		return;

	pTexture = pDlg->m_pPreviewTexture;
	if(!rgbData.SetSize(pTexture->m_Header.m_BaseWidth * pTexture->m_Header.m_BaseHeight))
		return;

	GetWindowText(hWnd, oldText, sizeof(oldText));
	outYCoord = 0;

	for(iMipmap=0; iMipmap < pTexture->m_Header.m_nMipmaps; iMipmap++)
	{
		pMip = &pTexture->m_Mips[iMipmap];

		// Get us into PValue format.
		SetWindowText(hWnd, "Converting...");
		dtx_SetupDTXFormat(pTexture, cRequest.m_pSrcFormat);
		cRequest.m_pSrc = pMip->m_Data;
		cRequest.m_SrcPitch = pMip->m_Pitch;
		cRequest.m_pDestFormat->InitPValueFormat();
		cRequest.m_pDest = (BYTE*)rgbData.GetArray();
		cRequest.m_DestPitch = pMip->m_Width * sizeof(DWORD);
		cRequest.m_Width = pMip->m_Width;
		cRequest.m_Height = pMip->m_Height;

		if( pTexture->m_Header.GetBPPIdent() == BPP_32P )
		{
			DtxSection* pSection = dtx_FindSection(pTexture, "PALLETE32");
			if( pSection )
			{
				cRequest.m_pSrcPalette = (RPaletteColor*)pSection->m_Data;
			}
		}

		dResult = formatMgr.ConvertPixels(&cRequest);
		if(dResult != LT_OK)
		{
			SetWindowText(hWnd, oldText);
			return;
		}

		// Store this in the stack so it doesn't crash if they change it in the other thread.
		bppIdent = pDlg->m_BPPIdent;

		// Possibly compress.
		if(IsBPPCompressed(bppIdent) && bppIdent != pTexture->m_Header.GetBPPIdent())
		{
			compressor.m_Format = bppIdent;
			compressor.m_Width = pMip->m_Width;
			compressor.m_Height = pMip->m_Height;

			compressor.m_pData = rgbData.GetArray();
			compressor.m_Pitch = pMip->m_Width * sizeof(DWORD);
			compressor.m_DataFormat.InitPValueFormat();

			SetWindowText(hWnd, "Compressing...");
			dResult = compressor.CompressUsingLibrary();
			if(dResult == LT_OK)
			{
				// Now decompress into the PValue format.
				cRequest.m_pSrcFormat->Init(bppIdent, 0, 0, 0, 0);
				cRequest.m_pSrc = (BYTE*)compressor.m_pOutData;
				cRequest.m_pDestFormat->InitPValueFormat();
				cRequest.m_pDest = (BYTE*)rgbData.GetArray();
				cRequest.m_DestPitch = pMip->m_Width * sizeof(DWORD);
				cRequest.m_Width = pMip->m_Width;
				cRequest.m_Height = pMip->m_Height;

				SetWindowText(hWnd, "Decompressing...");
				dResult = formatMgr.ConvertPixels(&cRequest);

				delete compressor.m_pOutData;
			}
		}


		SetWindowText(hWnd, "Drawing...");
		hPen = CreatePen(PS_SOLID, 1, RGB(255,255,255));
		hOldPen = (HPEN)SelectObject(hDC, hPen);
			pInLine = rgbData.GetArray();
			for(y=0; y < pMip->m_Height; y++)
			{
				oldROP = SetROP2(hDC, R2_XORPEN);
				MoveToEx(hDC, 0, outYCoord+1, NULL);
				LineTo(hDC, pMip->m_Width*2, outYCoord+1);
				SetROP2(hDC, oldROP);

				for(x=0; x < pMip->m_Width; x++)
				{
					PValue_Get(pInLine[x], a, r, g, b);
					SetPixel(hDC, x, outYCoord, RGB(r, g, b));
					SetPixel(hDC, x+pMip->m_Width, outYCoord, RGB(a, a, a));
				}

				pInLine += pMip->m_Width;
				++outYCoord;
			}
		SelectObject(hDC, hOldPen);
		DeleteObject(hPen);
	}

	SetWindowText(hWnd, oldText);
}

static long WINAPI PreviewWndProc( HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam )
{
	PAINTSTRUCT ps;
	HDC hDC;

	switch(message)
	{
		case WM_PAINT:
		{
			hDC = BeginPaint(hwnd, &ps);
			PaintPreviewWnd(g_pTexturePropDlg, hwnd, hDC);
			EndPaint(hwnd, &ps);
		}
		break;
	}

	return DefWindowProc( hwnd, message, wParam, lParam );
}

unsigned long WINAPI PreviewThreadFn(LPVOID pParam)
{
	WNDCLASS wndclass;
	RECT rect, wndRect, clientRect;
	int xExtra, yExtra;
	CTexturePropDlg *pDlg;
	MSG msg;
	DWORD i, totalHeight;


	pDlg = (CTexturePropDlg*)pParam;

	pDlg->GetWindowRect(&rect);

	wndclass.style         = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
	wndclass.lpfnWndProc   = PreviewWndProc;
	wndclass.cbClsExtra    = 0;
	wndclass.cbWndExtra    = 0;
	wndclass.hInstance     = AfxGetInstanceHandle();
	wndclass.hIcon         = LoadIcon(NULL, IDI_APPLICATION);
	wndclass.hCursor       = LoadCursor(NULL, IDC_ARROW);
	wndclass.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
	wndclass.lpszMenuName  = NULL;
	wndclass.lpszClassName = "TexturePreview";
	RegisterClass(&wndclass);

	// Figure out the height.
	totalHeight = 0;
	for(i=0; i < pDlg->m_pPreviewTexture->m_Header.m_nMipmaps; i++)
	{
		totalHeight += pDlg->m_pPreviewTexture->m_Mips[i].m_Height;
	}

	// Make copies of everything we need in the main loop so we don't have to access pDlg
	HANDLE heventSync = pDlg->m_heventPreviewSync;
	HWND hPreviewWnd = pDlg->m_hPreviewWnd;

	// Let the main thread know that we're done using the dialog pointer  (Note : This needs to
	//		be here instead of right before the main loop because CreateWindow needs to use the
	//		main thread.)
	SetEvent(heventSync);
	while (WaitForSingleObject(heventSync, 0) == WAIT_OBJECT_0)
		Sleep(15);

	// Note: it assumes we're centered here.  The GetWindowRect call above doesn't seem to
	// return the correct top and left coordinates.
	pDlg->m_hPreviewWnd = CreateWindow("TexturePreview",      // window class name
				"Preview",			// window caption
				WS_OVERLAPPED|WS_VISIBLE,    // window style
				rect.right - rect.left,		// initial x position
				0,	// initial y position
				pDlg->m_pPreviewTexture->m_Header.m_BaseWidth,		// initial x size
				totalHeight,   	// initial y size
				pDlg->m_hWnd,                   // parent window handle
				NULL,                   // window menu handle
				AfxGetInstanceHandle(),            // program instance handle
				NULL);					// creation parameters

	// Size it correctly (have to do this after creating it because we don't know the size of the
	// borders and stuff).
	if(pDlg->m_hPreviewWnd)
	{
		::GetWindowRect(pDlg->m_hPreviewWnd, &wndRect);
		::GetClientRect(pDlg->m_hPreviewWnd, &clientRect);
		xExtra = (wndRect.right - wndRect.left) - (clientRect.right - clientRect.left);
		yExtra = (wndRect.bottom - wndRect.top) - (clientRect.bottom - clientRect.top);
		::MoveWindow(pDlg->m_hPreviewWnd,
			rect.right-rect.left, 0,
			(pDlg->m_pPreviewTexture->m_Header.m_BaseWidth*2) + xExtra,
			totalHeight + yExtra, TRUE);
	}

	SetFocus(pDlg->m_hWnd);

	while(WaitForSingleObject(heventSync, 15) == WAIT_TIMEOUT)
	{
		if(PeekMessage(&msg, hPreviewWnd, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	::DestroyWindow(pDlg->m_hPreviewWnd);
	pDlg->m_hPreviewWnd = NULL;

	ExitThread(0);

	return 0;
}



/////////////////////////////////////////////////////////////////////////////
// CTexturePropDlg dialog

CTexturePropDlg::CTexturePropDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CTexturePropDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CTexturePropDlg)
	//}}AFX_DATA_INIT
	m_ChangeFlags = 0;
	m_bAllowChange = FALSE;
	m_hPreviewWnd = NULL;
	m_hPreviewThread = NULL;
	m_PreviewThreadID = 0;
	m_heventPreviewSync = CreateEvent(NULL, TRUE, FALSE, NULL);
	m_pPreviewTexture = NULL;
	m_bTFormatChanges = TRUE;
}

CTexturePropDlg::~CTexturePropDlg()
{
	// Shutdown the thread.
	if(m_hPreviewThread)
	{
		SetEvent(m_heventPreviewSync);
		WaitForSingleObject(m_hPreviewThread, INFINITE);
		CloseHandle(m_heventPreviewSync);
		CloseHandle(m_hPreviewThread);
	}

	if(m_pPreviewTexture)
	{
		dtx_Destroy(m_pPreviewTexture);
		m_pPreviewTexture = NULL;
	}

	g_pTexturePropDlg = NULL;
}


void CTexturePropDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CTexturePropDlg)
	DDX_Control(pDX, IDC_PREFER5551, m_Prefer5551);
	DDX_Control(pDX, IDC_PREFER4444, m_Prefer4444);
	DDX_Control(pDX, IDC_FORMAT_DXT5, m_Format_DXT5);
	DDX_Control(pDX, IDC_DATAFORMAT, m_DataFormat);
	DDX_Control(pDX, IDC_FORMAT_DXT3, m_Format_DXT3);
	DDX_Control(pDX, IDC_FORMAT_DXT1, m_Format_DXT1);
	DDX_Control(pDX, IDC_FORMAT_32BIT, m_Format_32Bit);
	DDX_Control(pDX, IDC_FORMAT_32P, m_Format_32P);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CTexturePropDlg, CDialog)
	//{{AFX_MSG_MAP(CTexturePropDlg)
	ON_EN_CHANGE(IDC_TEXTUREPROP_FLAGS, OnTexturePropChange)
	ON_EN_CHANGE(IDC_TEXTURE_GROUP, OnGroupChange)
	ON_EN_CHANGE(IDC_NUMBER_OF_MIPMAPS, OnNumMipmapsChange)
	ON_EN_CHANGE(IDC_DTX_NONS3TCMIPMAPOFFSET, OnNonS3TCMipmapOffset)
	ON_EN_CHANGE(IDC_UIMIPMAPOFFSET, OnUIMipmapOffset)
	ON_EN_CHANGE(IDC_DETAILTEXTURESCALE, OnChangeDetailTextureScale)
	ON_EN_CHANGE(IDC_DETAILTEXTUREANGLE, OnChangeDetailTextureAngle)
	ON_COMMAND(IDOK, OnOk)
	ON_BN_CLICKED(IDC_FULLBRITE, OnFullbrite)
	ON_BN_CLICKED(IDC_32BITSYSCOPY, On32BitSysCopy)
	ON_BN_CLICKED(IDC_PREFER4444, OnPrefer4444)
	ON_BN_CLICKED(IDC_PREFER5551, OnPrefer5551)
	ON_BN_CLICKED(IDC_PREFER16BIT, OnPrefer16Bit)
	ON_BN_CLICKED(IDC_NOSYSCACHE, OnNoSysCache)
	ON_EN_CHANGE(IDC_DTX_COMMAND_STRING2, OnChangeDtxCommandString)
	ON_EN_CHANGE(IDC_TEXTUREPRIORITY, OnChangeTexturePriority)
	ON_BN_CLICKED(IDC_FORMAT_32BIT, OnFormat32bit)
	ON_BN_CLICKED(IDC_FORMAT_DXT1, OnFormatDxt1)
	ON_BN_CLICKED(IDC_FORMAT_DXT3, OnFormatDxt3)
	ON_BN_CLICKED(IDC_FORMAT_DXT5, OnFormatDxt5)
	ON_BN_CLICKED(IDC_FORMAT_32P, OnFormat32P)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


void CTexturePropDlg::SetCheckBox(UINT id, BOOL bCheck)
{
	CButton *pButton;

	pButton = (CButton*)GetDlgItem(id);
	if(pButton)
		pButton->SetCheck(bCheck);
}


void CTexturePropDlg::EnableCheckBox(UINT id, BOOL bEnable)
{
	CButton *pButton;

	pButton = (CButton*)GetDlgItem(id);
	if( pButton )
		pButton->EnableWindow( bEnable );
}


BOOL CTexturePropDlg::GetCheckBox(UINT id)
{
	CButton *pButton;

	pButton = (CButton*)GetDlgItem(id);
	if(pButton)
		return pButton->GetCheck();
	else
		return FALSE;
}


void CTexturePropDlg::SetupBPPIdent()
{
	// disable checkboxes that can't be converted to from the current format
	EnableCheckBox(IDC_FORMAT_32BIT, m_BPPIdent != BPP_32P);
	EnableCheckBox(IDC_FORMAT_DXT1, m_BPPIdent != BPP_32P);
	EnableCheckBox(IDC_FORMAT_DXT3, m_BPPIdent != BPP_32P);
	EnableCheckBox(IDC_FORMAT_DXT5, m_BPPIdent != BPP_32P);
	EnableCheckBox(IDC_FORMAT_32P, m_BPPIdent == BPP_32P);

	SetCheckBox(IDC_FORMAT_32BIT, m_BPPIdent == BPP_32);
	SetCheckBox(IDC_FORMAT_DXT1, m_BPPIdent == BPP_S3TC_DXT1);
	SetCheckBox(IDC_FORMAT_DXT3, m_BPPIdent == BPP_S3TC_DXT3);
	SetCheckBox(IDC_FORMAT_DXT5, m_BPPIdent == BPP_S3TC_DXT5);
	SetCheckBox(IDC_FORMAT_32P, m_BPPIdent == BPP_32P);
}


void CTexturePropDlg::RedrawPreviewWindow()
{
	if(m_hPreviewWnd)
	{
		::InvalidateRect(m_hPreviewWnd, NULL, FALSE);
	}
}


/////////////////////////////////////////////////////////////////////////////
// CTexturePropDlg message handlers

BOOL CTexturePropDlg::OnInitDialog()
{
	char str[32];
	RECT rect;


	CDialog::OnInitDialog();

	m_bAllowChange = FALSE;

	SetDlgItemInt(IDC_TEXTUREPROP_FLAGS, m_TextureFlags);

	SetCheckBox(IDC_PREFER16BIT,	m_bPrefer16Bit);
	SetCheckBox(IDC_NOSYSCACHE,		m_bNoSysCache);
	SetCheckBox(IDC_FULLBRITE,		m_bFullBrights);
	SetCheckBox(IDC_32BITSYSCOPY,	m_b32BitSysCopy);
	SetCheckBox(IDC_PREFER4444,		m_bPrefer4444);
	SetCheckBox(IDC_PREFER5551,		m_bPrefer5551);

	SetDlgItemInt(IDC_TEXTURE_GROUP, m_TextureGroup);

	SetDlgItemText(IDC_DTX_COMMAND_STRING2, m_CommandString);
	SetDlgItemInt(IDC_NUMBER_OF_MIPMAPS, (int)m_nMipmaps);
	SetDlgItemInt(IDC_DTX_NONS3TCMIPMAPOFFSET, (int)m_NonS3TCMipmapOffset);
	SetDlgItemInt(IDC_UIMIPMAPOFFSET, m_UIMipmapOffset);
	SetDlgItemInt(IDC_TEXTUREPRIORITY, m_TexturePriority);
	SetDlgItemInt(IDC_DETAILTEXTUREANGLE, m_DetailTextureAngle);

	sprintf(str, "%f", m_DetailTextureScale);
	SetDlgItemText(IDC_DETAILTEXTURESCALE, str);


	m_bAllowChange = TRUE;

	// Move us into the upper-left corner.
	GetWindowRect(&rect);
	MoveWindow(0, 0, rect.right-rect.left, rect.bottom-rect.top, TRUE);

	SetupBPPIdent();

	// Create the preview window?
	if(m_pPreviewTexture)
	{
		g_pTexturePropDlg = this;

		// Close the preview thread if it's previously been opened
		if (m_hPreviewThread)
		{
			SetEvent(m_heventPreviewSync);
			WaitForSingleObject(m_hPreviewThread, INFINITE);
			CloseHandle(m_hPreviewThread);
			ResetEvent(m_heventPreviewSync);
		}

		m_hPreviewThread = CreateThread(NULL, 0, PreviewThreadFn, this, 0, (unsigned long *)&m_PreviewThreadID);
		// Wait until the thread says it's done with the dialog pointer
		WaitForSingleObject(m_heventPreviewSync, INFINITE);
		ResetEvent(m_heventPreviewSync);
		// Cut the thread's priority
		SetThreadPriority(m_hPreviewThread, THREAD_PRIORITY_LOWEST);
		SetFocus();
	}

	return TRUE;
}


void CTexturePropDlg::OnOk()
{
	TCHAR theText[200];

	if(GetDlgItemText(IDC_TEXTUREPROP_FLAGS, theText, 200) != 0)
	{
		m_TextureFlags = atoi(theText);
	}

	if(GetDlgItemText(IDC_TEXTURE_GROUP, theText, 200) != 0)
	{
		m_TextureGroup = atoi(theText);
	}

	if(GetDlgItemText(IDC_DTX_NONS3TCMIPMAPOFFSET, theText, 200) != 0)
	{
		m_NonS3TCMipmapOffset = atoi(theText);
	}

	if(GetDlgItemText(IDC_UIMIPMAPOFFSET, theText, 200) != 0)
	{
		m_UIMipmapOffset = atoi(theText);
	}

	if(GetDlgItemText(IDC_TEXTUREPRIORITY, theText, 200) != 0)
	{
		m_TexturePriority = atoi(theText);
	}

	if(GetDlgItemText(IDC_DETAILTEXTURESCALE, theText, 200) != 0)
	{
		m_DetailTextureScale = (float)atof(theText);
	}

	if(GetDlgItemText(IDC_DETAILTEXTUREANGLE, theText, 200) != 0)
	{
		m_DetailTextureAngle = atoi(theText);
	}

	if(GetDlgItemText(IDC_NUMBER_OF_MIPMAPS, theText, 200) != 0)
	{
		m_nMipmaps = atoi(theText);

		// Make sure they enter a valid range.
		if(m_nMipmaps <= 0 || m_nMipmaps > MAX_DTX_MIPMAPS)
		{
			AppMessageBox(IDS_INVALID_MIPMAP_COUNT, MB_OK);
			return;
		}
	}

	m_bFullBrights = GetCheckBox(IDC_FULLBRITE);
	m_b32BitSysCopy= GetCheckBox(IDC_32BITSYSCOPY);
	m_bNoSysCache = GetCheckBox(IDC_NOSYSCACHE);
	m_bPrefer4444 = GetCheckBox(IDC_PREFER4444);
	m_bPrefer5551 = GetCheckBox(IDC_PREFER5551);
	m_bPrefer16Bit = GetCheckBox(IDC_PREFER16BIT);

	GetDlgItemText(IDC_DTX_COMMAND_STRING2, m_CommandString, sizeof(m_CommandString));

	EndDialog(IDOK);
}


void CTexturePropDlg::SetChange(uint32 id, uint32 flag)
{
	CWnd *pWnd;
	char curText[256], newText[256];

	if(m_bAllowChange)
	{
		if(!(m_ChangeFlags & flag))
		{
			m_ChangeFlags |= flag;
			if(pWnd = GetDlgItem(id))
			{
				pWnd->GetWindowText(curText, sizeof(curText));
				sprintf(newText, "* %s", curText);
				pWnd->SetWindowText(newText);
			}
		}
	}
}

void CTexturePropDlg::OnTexturePropChange()
{
	SetChange(IDC_TEXTUREPROP_FLAGS_TEXT, TPROP_FLAGS);
}

void CTexturePropDlg::OnGroupChange()
{
	SetChange(IDC_TEXTURE_GROUP_TEXT, TPROP_GROUP);
}

void CTexturePropDlg::OnNumMipmapsChange()
{
	SetChange(IDC_NUMBER_OF_MIPMAPS_TEXT, TPROP_NUMMIPMAPS);
}

void CTexturePropDlg::OnFullbrite()
{
	SetChange(IDC_FULLBRITE, TPROP_FULLBRITES);
}

void CTexturePropDlg::On32BitSysCopy()
{
	SetChange(IDC_32BITSYSCOPY, TPROP_32BITSYSCOPY);
}

void CTexturePropDlg::OnPrefer4444()
{
	if(m_bTFormatChanges)
	{
		SetChange(IDC_PREFER4444, TPROP_PREFER4444);

		// Force 5551 off.
		m_bTFormatChanges = FALSE;
			m_Prefer5551.SetCheck(FALSE);
			SetChange(IDC_PREFER5551, TPROP_PREFER5551);
		m_bTFormatChanges = TRUE;
	}
}

void CTexturePropDlg::OnPrefer5551()
{
	if(m_bTFormatChanges)
	{
		SetChange(IDC_PREFER5551, TPROP_PREFER5551);

		// Force 4444 off.
		m_bTFormatChanges = FALSE;
			m_Prefer4444.SetCheck(FALSE);
			SetChange(IDC_PREFER4444, TPROP_PREFER4444);
		m_bTFormatChanges = TRUE;
	}
}

void CTexturePropDlg::OnPrefer16Bit()
{
	SetChange(IDC_PREFER16BIT, TPROP_PREFER16BIT);
}

void CTexturePropDlg::OnNoSysCache()
{
	SetChange(IDC_NOSYSCACHE, TPROP_NOSYSCACHE);
}

void CTexturePropDlg::OnChangeDtxCommandString()
{
	SetChange(IDC_DTX_COMMAND_STRING_LABEL2, TPROP_COMMANDSTRING);
}

void CTexturePropDlg::OnFormat32bit()
{
	SetChange(IDC_DATAFORMAT, TPROP_DATAFORMAT);
	m_BPPIdent = BPP_32;
	SetupBPPIdent();
	RedrawPreviewWindow();
}

void CTexturePropDlg::OnFormatDxt1()
{
	SetChange(IDC_DATAFORMAT, TPROP_DATAFORMAT);
	m_BPPIdent = BPP_S3TC_DXT1;
	SetupBPPIdent();
	RedrawPreviewWindow();
}

void CTexturePropDlg::OnFormatDxt3()
{
	SetChange(IDC_DATAFORMAT, TPROP_DATAFORMAT);
	m_BPPIdent = BPP_S3TC_DXT3;
	SetupBPPIdent();
	RedrawPreviewWindow();
}

void CTexturePropDlg::OnFormatDxt5()
{
	SetChange(IDC_DATAFORMAT, TPROP_DATAFORMAT);
	m_BPPIdent = BPP_S3TC_DXT5;
	SetupBPPIdent();
	RedrawPreviewWindow();
}

void CTexturePropDlg::OnFormat32P()
{
	SetupBPPIdent();
}

void CTexturePropDlg::OnNonS3TCMipmapOffset()
{
	SetChange(IDC_DTX_NONS3TCMIPMAPOFFSET_LABEL, TPROP_NONS3TCMIPMAPOFFSET);
}

void CTexturePropDlg::OnUIMipmapOffset()
{
	SetChange(IDC_UIMIPMAPOFFSET_LABEL, TPROP_UIMIPMAPOFFSET);
}


void CTexturePropDlg::OnChangeTexturePriority()
{
	SetChange(IDC_PRIORITY_LABEL, TPROP_TEXTUREPRIORITY);
}


void CTexturePropDlg::OnChangeDetailTextureScale()
{
	SetChange(IDC_DETAILTEXTURESCALE_LABEL, TPROP_DETAILTEXTURESCALE);
}

void CTexturePropDlg::OnChangeDetailTextureAngle()
{
	SetChange(IDC_DETAILTEXTUREANGLE_LABEL, TPROP_DETAILTEXTUREANGLE);
}



int AppMessageBox( UINT idString, UINT nType )
{
	CString		str;

	str.LoadString( idString );
	return AfxGetMainWnd()->MessageBox( str, AfxGetAppName(), nType );
}


int AppMessageBox( const char *pStr, UINT nType )
{
	return AfxGetMainWnd()->MessageBox( pStr, AfxGetAppName(), nType );
}





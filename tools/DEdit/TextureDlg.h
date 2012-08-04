#ifndef __TEXTUREDLG_H__
#define __TEXTUREDLG_H__

#include "resourcemgr.h"
#include "baserezdlg.h"
#include "alltexturedlg.h"
#include "baseimgdlg.h"


// (Create Alpha Mask type).
typedef enum
{
	CAM_Solid=0,
	CAM_FromColor,
	CAM_From4BitPCX,
	CAM_From8BitPCX
} CAMType;


typedef enum
{
	ITYPE_PCX=0,
	ITYPE_TGA
} ImageType;


// info used during a batch scale of texture data for rescaling mappings in worlds
struct BatchTextureScaleInfo
{
	CString texName;	// name of the texture being rescaled
	int oldX, oldY;		// dimensions of the old texture
	int newX, newY;		// dimensions of the new texture
};

struct BatchReloadOptions;



/////////////////////////////////////////////////////////////////////////////
// CTextureDlg dialog

class CTextureDlg : public CBaseImgDlg
{
// Construction
public:
	CTextureDlg();			// standard constructor
	virtual ~CTextureDlg();	// destructor

// Dialog Data
	//{{AFX_DATA(CTextureDlg)
	enum { IDD = IDD_TEXTURE_TABDLG };
	//}}AFX_DATA

	void	SetListItemText(int nItem, CString &relativeFilename);

	CTreeCtrl	m_TextureTree;
	CListCtrl	m_TextureList;

	CImageList	*m_pIconList;
	CImageList	*m_pIconList2;

	HFONT		m_hFont;

	CAllTextureDlg	m_AllTextureDlg;

	HTREEITEM	m_hCurrentTextureDir;
	int			m_nCurrentTextureIndex;
	void		ChangeSelection(int nIndex, BOOL bClearSelect = TRUE);
	BOOL		DoImportOperation(DWORD flags );
	bool		DoImportCubeMapOperation( );
	bool		DoImportBumpMapOperation( );
	bool		DoImportNormalMapOperation( );
	BOOL		DoImportMipPcxOperation( );
	BOOL		DoExportPcxOperation( );
	BOOL		DoExport8BitDTXOperation( );
	BOOL		DoExportAllAsBPP_32POperation( );
	BOOL		DoConvertTo32P( );
	BOOL		DoRenameTexture( );
	BOOL		DoMakeTextureWritable( );
	BOOL		DoTextureProperties( );
	BOOL		DoTextureProperties( CMoArray<DFileIdent*>& selections );

	// batch texture reload stuff.  these replace image data in a texture
	bool		ProcessTextureReload( BatchTextureScaleInfo* textureInfo, const CString& in, const CString& out, bool isPcx );
	bool		ProcessBatchReload( CMoArray<BatchTextureScaleInfo*>& textureInfo, const char* input, const char* output, const BatchReloadOptions& options );
	bool		DoBatchReload( );

	// batch texture scale stuff.  this scales texture coordinates in the world based on a bute file
	bool		DoScaleTextureCoords( );
	
	BOOL		DoCreateAlphaMask(CAMType type);
	
	BOOL		DoViewAllTextures();
	BOOL		DoAddToPalette();
	bool		CopyImageToTexture( ImageType iType,
		const char *pPcxFilename, const char *pTextureFilename, DWORD textureFlags );
	BOOL		CopyMipTGAToTexture( const CString *pTGAFilename, uint32 nNumImages, const char *pTextureFilename );

	// Finds a specified texture in the list
	BOOL		FindTexture(DFileIdent *pTexture, BOOL bRecurse = FALSE, HTREEITEM hBaseTree = 0);
	// Tries to find a specified texture based on the filename instead of searching recursively
	BOOL		FastFindTexture(DFileIdent *pTexture);

	//this must be overridden by a derived class to render the icon for the appropriate
	//list item
	virtual bool	RenderIcon(HDC BlitTo, uint32 nXOff, uint32 nImgSize, uint32 nItem); 

	//this must be overridden by a derived class to render the large selected image
	virtual void	RenderLargeImage();


// Overrides
	//{{AFX_VIRTUAL(CTextureDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	CBitmap	*m_pThumbnails;

	BOOL OnInitDialogBar();

	// This is called to reposition the controls
	virtual void	RepositionControls();

	// Generated message map functions
	//{{AFX_MSG(CTextureDlg)	
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnSelTexture( NMHDR * pNMHDR, LRESULT * pResult );
	afx_msg void OnOpenTexture( NMHDR * pNMHDR, LRESULT * pResult );
	afx_msg void OnKeySelTexture(NMHDR * pnkd, LRESULT * pResult );
	afx_msg void OnSelchangedDirectory(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDestroy();
	afx_msg void OnItemexpandingBaserezTree(NMHDR* pNMHDR, LRESULT* pResult);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}


#endif 

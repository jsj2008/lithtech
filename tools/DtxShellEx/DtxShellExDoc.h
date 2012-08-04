// DtxShellExDoc.h : interface of the CDtxShellExDoc class
//
#include "TextureProp.h"

class DtxImageBuffer
{
public:

	DtxImageBuffer()
		: m_Image(NULL),
		  m_Alpha(NULL),
	      m_Width(0),
		  m_Height(0),
		  m_Bytes(0)
	{
	}

	~DtxImageBuffer()
	{
		Term();
	}

	void		Init(unsigned Width, unsigned Height);
	void		Term();

public:

	uint32*		m_Image;
	uint32*		m_Alpha;
	unsigned	m_Width;
	unsigned	m_Height;
	unsigned	m_Bytes;
};




class CDtxShellExDoc : public CDocument
{
protected: // create from serialization only
	CDtxShellExDoc();
	DECLARE_DYNCREATE(CDtxShellExDoc)


protected:
	CSize           m_sizeDoc;
public:
	virtual CSize GetDocSize() { return m_sizeDoc; }
	virtual void OnDraw(CDC* pDC);


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDtxShellExDoc)
	public:
	virtual BOOL OnNewDocument();
	virtual void Serialize(CArchive& ar);
	virtual void LoadDTX(const char* szFullFilePath);
	virtual bool CreateBuffers();
	virtual BOOL OnOpenDocument(LPCTSTR lpszPathName);
	virtual void DeleteContents();
	//}}AFX_VIRTUAL

// Implementation
protected:
	bool m_bBuffersCreated;

public:
	virtual ~CDtxShellExDoc();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif


// Generated message map functions
protected:
	//{{AFX_MSG(CDtxShellExDoc)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

	TextureProp			m_TextureProp; // texture data and settings
	DtxImageBuffer		m_ImageBuffer;
};

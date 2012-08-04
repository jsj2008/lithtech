#ifndef USE_COLUMNCTRL
#define USE_COLUMNCTRL

#define SORT_MIN				0x00000000	// Used to validate sort types
#define SORT_NONE				SORT_MIN+1	// no sorting
#define SORT_ALPHA_ASCENDING	SORT_MIN+2	// Ascending alpha sort
#define SORT_ALPHA_DESCENDING	SORT_MIN+3	// Descending alpha sort
#define SORT_NUMERIC_ASCENDING	SORT_MIN+4	// Ascending numeric sort
#define SORT_NUMERIC_DESCENDING	SORT_MIN+5	// Descending numeric sort
#define SORT_MAX				SORT_MIN+6	// Used to validate sort types

/////////////////////////////////////////////////////
// CColumnCtrl
//
// Layer around the CListCtrl for multi-column list boxes making it
// easier to deal with a multi-column listboxes because you don't have
// to deal with CListCtrl.

class CColumnCtrl : public CListCtrl
{
public:
	CColumnCtrl();

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CColumnCtrl)
	//}}AFX_VIRTUAL

public:
	// Creates a column control
	BOOL Create ( const RECT &rect, CWnd *pParentWnd, HICON hDefaultIcon=NULL, CSize csImage=CSize(0,0), HINSTANCE hAppInstance=NULL);
	BOOL Create ( const RECT &rect, HWND hParentWnd, HICON hDefaultIcon=NULL, CSize csImage=CSize(0,0), HINSTANCE hAppInstance=NULL);

	// Add columns to the control
	// note: call CListCtrl::DeleteColumn to remove a column
	BOOL AddColumn ( LPCTSTR lpszColumnHeading, unsigned int dwSortMethod=SORT_ALPHA_ASCENDING, int nPercentWidth = -1 );
	
	// Add/Delete lines of text to/from the control	
	BOOL AddRow ( CString sRowText, HICON hIcon=NULL );		// Separate the columns with \t
	BOOL DeleteRow ( int nIndex );
	BOOL DeleteAllRows();

	// Sort the control based on a column and method (SORT_XXXX see above)
	void Sort ( int nColumnIndex, DWORD dwSortMethod );

protected:
	// Swap two rows in the control
	void SwapRows ( int nRow1, int Row2 );	

protected:
	int nColumns;	// Number of columns in the control
	int nRows;		// Number of rows in the control

	CImageList ImageList;		// Image list used for the control
	CDWordArray dwSortArray;	// Array or sort types (one for each column)
	CDWordArray dwImageArray;	// Which image is with which icon

	HINSTANCE hInstance;		// HINSTANCE for loading icons

	// Generated message map functions
protected:
	//{{AFX_MSG(CColumnCtrl)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnColumnclick(NMHDR* pNMHDR, LRESULT* pResult);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

#endif
